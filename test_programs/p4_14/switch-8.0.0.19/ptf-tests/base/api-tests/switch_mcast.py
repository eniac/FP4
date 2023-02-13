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
IP multicast tests
"""

import switchapi_thrift

import os
import time
import sys
import logging

import unittest
import random

import api_base_tests

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *
import pd_base_tests
try:
    import pltfm_pm_rpc
    from pltfm_pm_rpc.ttypes import *
except ImportError:
    pass

from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '..'))
from common.utils import *
from common.api_utils import *

device=0
cpu_port=64

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)

if swports == []:
    swports = [x for x in range(65)]

###############################################################################
@group('l3')
@group('mcast')
@group('maxsizes')
class L3Multicast(pd_base_tests.ThriftInterfaceDataPlane,
                  api_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def setUp(self):
        print
        print 'Configuring devices for L3 multicast test cases'

        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.devport = []
        for i in range(0,8):
            self.devport.append(swport_to_devport(self, swports[i]))

        self.cpu_port = get_cpu_port(self)

        # initialize the variables
        self.mch3 = ""
        self.rpf4 = ""
        self.rpf5 = ""
        self.rpf6 = ""

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.client.switch_api_init(device)

        self.vrf = self.client.switch_api_vrf_create(device, 2)
        self.rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, self.rmac,
                                              '00:77:66:55:44:33')

        self.port0 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[0])
        self.port1 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[1])
        self.port2 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[2])
        self.port3 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[3])
        self.port4 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[4])
        self.port5 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[5])
        self.port6 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[6])
        self.port7 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[7])
        self.port8 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[8])

        # vlans: 10, 100, 200, 300
        self.vlan1 = self.client.switch_api_vlan_create(device, 10)
        self.vlan2 = self.client.switch_api_vlan_create(device, 100)
        self.vlan3 = self.client.switch_api_vlan_create(device, 200)
        self.vlan4 = self.client.switch_api_vlan_create(device, 300)

        # disable learning
        self.client.switch_api_vlan_learning_set(device, self.vlan1, 0)
        self.client.switch_api_vlan_learning_set(device, self.vlan2, 0)
        self.client.switch_api_vlan_learning_set(device, self.vlan3, 0)
        self.client.switch_api_vlan_learning_set(device, self.vlan4, 0)

        # enable igmp snooping on vlan3 and vlan4
        self.client.switch_api_vlan_igmp_snooping_set(device, self.vlan3, 1)
        self.client.switch_api_vlan_igmp_snooping_set(device, self.vlan4, 1)

        # port 0: access port in vlan 10
        i_info1 = switcht_interface_info_t(
            handle=self.port0, type=SWITCH_INTERFACE_TYPE_ACCESS)
        self.if1 = self.client.switch_api_interface_create(device, i_info1)
        self.client.switch_api_vlan_member_add(device, self.vlan1, self.if1)

        # port 1: trunk port; allowed vlans: 10, 100, 200
        i_info2 = switcht_interface_info_t(
            handle=self.port1, type=SWITCH_INTERFACE_TYPE_TRUNK)
        self.if2 = self.client.switch_api_interface_create(device, i_info2)
        self.client.switch_api_vlan_member_add(device, self.vlan1, self.if2)
        self.client.switch_api_vlan_member_add(device, self.vlan2, self.if2)
        self.client.switch_api_vlan_member_add(device, self.vlan3, self.if2)

        # port 2: access port in vlan 100
        i_info3 = switcht_interface_info_t(
            handle=self.port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        self.if3 = self.client.switch_api_interface_create(device, i_info3)
        self.client.switch_api_vlan_member_add(device, self.vlan2, self.if3)

        rif_info4 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_multicast_enabled=1,
            v6_multicast_enabled=1)
        rif_info5 = rif_info4
        self.rif4 = self.client.switch_api_rif_create(0, rif_info4)
        self.rif5 = self.client.switch_api_rif_create(0, rif_info5)
        # port 3: routed port
        i_info4 = switcht_interface_info_t(
            handle=self.port3,
            type=SWITCH_INTERFACE_TYPE_PORT,
            rif_handle=self.rif4)
        self.if4 = self.client.switch_api_interface_create(device, i_info4)

        self.ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.250.1',
            prefix_length=24)
        self.client.switch_api_l3_interface_address_add(device, self.rif4,
                                                        self.vrf, self.ip4)

        # port 4: routed port
        i_info5 = switcht_interface_info_t(
            handle=self.port4,
            type=SWITCH_INTERFACE_TYPE_PORT,
            rif_handle=self.rif5)
        self.if5 = self.client.switch_api_interface_create(device, i_info5)
        self.ip5 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.251.1',
            prefix_length=24)
        self.client.switch_api_l3_interface_address_add(device, self.rif5,
                                                        self.vrf, self.ip5)

        # port 5: trunk port; allowed vlans: 10, 100, 200, 300
        i_info6 = switcht_interface_info_t(
            handle=self.port5, type=SWITCH_INTERFACE_TYPE_TRUNK)
        self.if6 = self.client.switch_api_interface_create(device, i_info6)
        self.client.switch_api_vlan_member_add(device, self.vlan1, self.if6)
        self.client.switch_api_vlan_member_add(device, self.vlan2, self.if6)
        self.client.switch_api_vlan_member_add(device, self.vlan3, self.if6)
        self.client.switch_api_vlan_member_add(device, self.vlan4, self.if6)

        # port 6: trunk port; allowed vlans: 10, 100, 200, 300
        i_info7 = switcht_interface_info_t(
            handle=self.port6, type=SWITCH_INTERFACE_TYPE_TRUNK)
        self.if7 = self.client.switch_api_interface_create(device, i_info7)
        self.client.switch_api_vlan_member_add(device, self.vlan1, self.if7)
        self.client.switch_api_vlan_member_add(device, self.vlan2, self.if7)
        self.client.switch_api_vlan_member_add(device, self.vlan3, self.if7)
        self.client.switch_api_vlan_member_add(device, self.vlan4, self.if7)

        # port 7: trunk port; allowed vlans: 10, 100, 200, 300
        i_info8 = switcht_interface_info_t(
            handle=self.port7, type=SWITCH_INTERFACE_TYPE_TRUNK)
        self.if8 = self.client.switch_api_interface_create(device, i_info8)
        self.client.switch_api_vlan_member_add(device, self.vlan1, self.if8)
        self.client.switch_api_vlan_member_add(device, self.vlan2, self.if8)
        self.client.switch_api_vlan_member_add(device, self.vlan3, self.if8)
        self.client.switch_api_vlan_member_add(device, self.vlan4, self.if8)

        # port 8: trunk port; allowed vlans: 300
        i_info9 = switcht_interface_info_t(
            handle=self.port8, type=SWITCH_INTERFACE_TYPE_TRUNK)
        self.if9 = self.client.switch_api_interface_create(device, i_info9)
        self.client.switch_api_vlan_member_add(device, self.vlan4, self.if9)

        rif_info20 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1,
            v4_multicast_enabled=1,
            v6_multicast_enabled=1)
        self.rif20 = self.client.switch_api_rif_create(0, rif_info20)
        rif_info21 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=100,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1,
            v4_multicast_enabled=1,
            v6_multicast_enabled=1)
        self.rif21 = self.client.switch_api_rif_create(0, rif_info21)
        rif_info22 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=200,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1,
            v4_multicast_enabled=1,
            v6_multicast_enabled=1)
        self.rif22 = self.client.switch_api_rif_create(0, rif_info22)

        # Create L3 virtual interface for vlan 10
        self.ip20 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.10.1',
            prefix_length=24)
        self.client.switch_api_l3_interface_address_add(device, self.rif20,
                                                        self.vrf, self.ip20)

        # Create L3 virtual interface for vlan 100
        self.ip21 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.100.1',
            prefix_length=24)
        self.client.switch_api_l3_interface_address_add(device, self.rif21,
                                                        self.vrf, self.ip21)

        # Create L3 virtual interface for vlan 200
        self.ip22 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.200.1',
            prefix_length=24)
        self.client.switch_api_l3_interface_address_add(device, self.rif22,
                                                        self.vrf, self.ip22)

        # create inner multicast tree
        self.mch1 = self.client.switch_api_multicast_tree_create(device)

        # vlan ports
        self.route_ports = [
            switcht_mcast_member_t(network_handle=self.vlan1, intf_handle=self.if1),
            switcht_mcast_member_t(network_handle=self.vlan1, intf_handle=self.if2),
            switcht_mcast_member_t(network_handle=self.vlan1, intf_handle=self.if6),
            switcht_mcast_member_t(network_handle=self.vlan1, intf_handle=self.if7),
            switcht_mcast_member_t(network_handle=self.vlan1, intf_handle=self.if8),
            switcht_mcast_member_t(network_handle=self.vlan2, intf_handle=self.if2),
            switcht_mcast_member_t(network_handle=self.vlan2, intf_handle=self.if3),
            switcht_mcast_member_t(network_handle=self.vlan2, intf_handle=self.if6),
            switcht_mcast_member_t(network_handle=self.vlan2, intf_handle=self.if7),
            switcht_mcast_member_t(network_handle=self.vlan2, intf_handle=self.if8),
            switcht_mcast_member_t(network_handle=self.vlan3, intf_handle=self.if6),
            switcht_mcast_member_t(network_handle=self.vlan3, intf_handle=self.if7),
            switcht_mcast_member_t(network_handle=self.vlan3, intf_handle=self.if8),
            switcht_mcast_member_t(network_handle=0, intf_handle=self.rif4),
            switcht_mcast_member_t(network_handle=0, intf_handle=self.rif5),
        ]
        self.client.switch_api_multicast_member_add(device, self.mch1,
                                                    self.route_ports)

        # create a ip multicast route (10.0.10.5,230.1.1.5)
        self.msrc_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.10.5',
            prefix_length=32)
        self.mgrp_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='230.1.1.5',
            prefix_length=32)
        self.rpf1 = self.client.switch_api_rpf_create(device, 2, 1)
        self.client.switch_api_rpf_member_add(device, self.rpf1, self.vlan1)
        self.client.switch_api_multicast_mroute_add(device, self.mch1,
                                                    self.rpf1, self.vrf,
                                                    self.msrc_ip1,
                                                    self.mgrp_ip1, 1)

        # create a snooping entry for vlan 200
        self.mch2 = self.client.switch_api_multicast_tree_create(device)
        # vlan 200 ports
        self.snoop_ports = [
            switcht_mcast_member_t(
                network_handle=self.vlan3,
                intf_handle=self.if6), switcht_mcast_member_t(
                    network_handle=self.vlan3, intf_handle=self.if7),
            switcht_mcast_member_t(
                network_handle=self.vlan3, intf_handle=self.if8)
        ]
        self.client.switch_api_multicast_member_add(device, self.mch2,
                                                    self.snoop_ports)
        self.client.switch_api_multicast_l2route_add(
            device, self.mch2, self.vlan3, self.msrc_ip1, self.mgrp_ip1)

        # vlan 400 ports
        self.miss_ports = [
            switcht_mcast_member_t(
                network_handle=self.vlan4, intf_handle=self.if7),
            switcht_mcast_member_t(
                network_handle=self.vlan4, intf_handle=self.if8)
        ]
        # create mroute miss tree
        self.mch0 = self.client.switch_api_multicast_tree_create(device)
        self.client.switch_api_multicast_member_add(device, self.mch0,
                                                    self.miss_ports)
        self.client.switch_api_multicast_mroute_miss_mgid_set(device, self.mch0,
                                                    self.vlan4)

    def updateMulticastMroute(self):
        print
        print 'Configuring devices for L3 multicast update test cases'

        # create new inner multicast tree
        self.mch3 = self.client.switch_api_multicast_tree_create(device)
        self.client.switch_api_multicast_member_add(device, self.mch3,
                                                    self.route_ports)

        # update a ip multicast route (10.0.10.5,230.1.1.5)
        self.client.switch_api_multicast_mroute_mgid_set(device,
                                                    self.mch3, self.vrf,
                                                    self.msrc_ip1,
                                                    self.mgrp_ip1, 1)
        self.rpf4 = self.client.switch_api_rpf_create(device, 2, 1)
        self.client.switch_api_rpf_member_add(device, self.rpf4, self.vlan1)
        self.client.switch_api_multicast_mroute_rpf_set(device,
                                                    self.rpf4, self.vrf,
                                                    self.msrc_ip1,
                                                    self.mgrp_ip1, 1)

    def allTestCases(self):
        print "IPv4 multicast hit (RPF pass)"
        pkt = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=64,
            pktlen=100)
        pkt1 = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=64,
            pktlen=104)
        pkt2 = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:77:66:55:44:33',
            dl_vlan_enable=True,
            vlan_vid=100,
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=63,
            pktlen=104)
        pkt3 = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:77:66:55:44:33',
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=63,
            pktlen=100)
        pkt4 = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:77:66:55:44:33',
            dl_vlan_enable=True,
            vlan_vid=200,
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=63,
            pktlen=104)
        send_packet(self, swports[0], str(pkt))
        p1 = [swports[1], [pkt1, pkt2]]
        p2 = [swports[2], [pkt3]]
        p3 = [swports[3], [pkt3]]
        p4 = [swports[4], [pkt3]]
        p5 = [swports[5], [pkt1, pkt2, pkt4]]
        p6 = [swports[6], [pkt1, pkt2, pkt4]]
        p7 = [swports[7], [pkt1, pkt2, pkt4]]
        verify_multiple_packets_on_ports(self, [p1, p2, p3, p4, p5, p6, p7])

        print "IPv4 multicast hit (RPF fail - flood in ingress vlan)"
        pkt = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=64,
            pktlen=100)
        pkt1 = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            dl_vlan_enable=True,
            vlan_vid=100,
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=64,
            pktlen=104)
        send_packet(self, swports[2], str(pkt))
        p1 = [swports[1], [pkt1]]
        p5 = [swports[5], [pkt1]]
        p6 = [swports[6], [pkt1]]
        p7 = [swports[7], [pkt1]]
        verify_multiple_packets_on_ports(self, [p1, p5, p6, p7])

        print "IPv4 multicast hit (Bridge hit, RPF fail, snooping enabled, mroute no)"
        pkt = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            dl_vlan_enable=True,
            vlan_vid=200,
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=64,
            pktlen=100)
        send_packet(self, swports[6], str(pkt))
        p5 = [swports[5], [pkt]]
        p7 = [swports[7], [pkt]]
        verify_multiple_packets_on_ports(self, [p5, p7])

        print "IPv4 multicast hit (Bridge miss, RPF fail, snooping enabled, mroute yes)"
        pkt = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            dl_vlan_enable=True,
            vlan_vid=300,
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=64,
            pktlen=100)
        send_packet(self, swports[8], str(pkt))
        p6 = [swports[6], [pkt]]
        p7 = [swports[7], [pkt]]
        verify_multiple_packets_on_ports(self, [p6, p7])
        verify_no_other_packets(self)

        print "IPv4 multicast miss (Bridge miss, snooping enabled, mroute no)"
        pkt = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            dl_vlan_enable=True,
            vlan_vid=200,
            ip_src='10.0.10.5',
            ip_dst='231.1.1.5',
            ip_ttl=64,
            pktlen=100)
        send_packet(self, swports[6], str(pkt))
        verify_multiple_packets_on_ports(self, [])

        print "IPv4 multicast miss (Bridge miss - snooping enabled, mroute yes)"
        pkt = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            dl_vlan_enable=True,
            vlan_vid=300,
            ip_src='10.0.10.5',
            ip_dst='231.1.1.5',
            ip_ttl=64,
            pktlen=100)
        send_packet(self, swports[8], str(pkt))
        p6 = [swports[6], [pkt]]
        p7 = [swports[7], [pkt]]
        verify_multiple_packets_on_ports(self, [p6, p7])
        verify_no_other_packets(self)

        print "IPv4 multicast miss (snooping disabled)"
        pkt = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            dl_vlan_enable=True,
            vlan_vid=100,
            ip_src='10.0.10.5',
            ip_dst='231.1.1.5',
            ip_ttl=64,
            pktlen=100)
        pkt1 = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            ip_src='10.0.10.5',
            ip_dst='231.1.1.5',
            ip_ttl=64,
            pktlen=96)
        ingress_ifindex = self.client.switch_api_interface_ifindex_get(device,
                                                                       self.if7)
        cpu_pkt = simple_cpu_packet(
            ingress_port=self.devport[6],
            ingress_ifindex=ingress_ifindex,
            reason_code=0,
            ingress_bd=3,
            inner_pkt=pkt)
        send_packet(self, swports[6], str(pkt))
        p1 = [swports[1], [pkt]]
        p2 = [swports[2], [pkt1]]
        p5 = [swports[5], [pkt]]
        p7 = [swports[7], [pkt]]
        p64 = [self.cpu_port, [cpu_pkt]]
        verify_multiple_packets_on_ports(self, [p1, p2, p5, p7])

    def runTest(self):
        self.allTestCases()

        # Update the MGID and retry
        self.updateMulticastMroute()

        self.allTestCases()

    def tearDown(self):
        self.client.switch_api_mac_table_entry_flush(
            device, SWITCH_MAC_FLUSH_TYPE_ALL, 0x0, 0x0)

        # delete mroute and its tree
        self.client.switch_api_multicast_mroute_delete(
            device, self.vrf, self.msrc_ip1, self.mgrp_ip1)

        self.client.switch_api_rpf_member_delete(device, self.rpf1, self.vlan1)
        self.client.switch_api_rpf_delete(device, self.rpf1)
        if self.rpf4:
            self.client.switch_api_rpf_member_delete(device, self.rpf4, self.vlan1)
            self.client.switch_api_rpf_delete(device, self.rpf4)

        self.client.switch_api_multicast_member_delete(device, self.mch1,
                                                       self.route_ports)
        self.client.switch_api_multicast_tree_delete(device, self.mch1)

        if self.mch3:
            self.client.switch_api_multicast_member_delete(device, self.mch3,
                                                           self.route_ports)
            self.client.switch_api_multicast_tree_delete(device, self.mch3)

        # delete mroute miss tree and ports on vlan 300
        self.client.switch_api_multicast_member_delete(device, self.mch0,
                                                       self.miss_ports)
        self.client.switch_api_multicast_tree_delete(device, self.mch0)

        # delete snooping entry in vlan 200
        self.client.switch_api_multicast_l2route_delete(
            device, self.vlan3, self.msrc_ip1, self.mgrp_ip1)
        self.client.switch_api_multicast_member_delete(device, self.mch2,
                                                       self.snoop_ports)
        self.client.switch_api_multicast_tree_delete(device, self.mch2)

        self.client.switch_api_l3_interface_address_delete(device, self.rif4,
                                                           self.vrf, self.ip4)
        self.client.switch_api_l3_interface_address_delete(device, self.rif5,
                                                           self.vrf, self.ip5)
        self.client.switch_api_l3_interface_address_delete(device, self.rif20,
                                                           self.vrf, self.ip20)
        self.client.switch_api_l3_interface_address_delete(device, self.rif21,
                                                           self.vrf, self.ip21)
        self.client.switch_api_l3_interface_address_delete(device, self.rif22,
                                                           self.vrf, self.ip22)

        self.client.switch_api_vlan_member_remove(device, self.vlan1, self.if1)
        self.client.switch_api_vlan_member_remove(device, self.vlan1, self.if2)
        self.client.switch_api_vlan_member_remove(device, self.vlan2, self.if2)
        self.client.switch_api_vlan_member_remove(device, self.vlan3, self.if2)
        self.client.switch_api_vlan_member_remove(device, self.vlan2, self.if3)
        self.client.switch_api_vlan_member_remove(device, self.vlan1, self.if6)
        self.client.switch_api_vlan_member_remove(device, self.vlan2, self.if6)
        self.client.switch_api_vlan_member_remove(device, self.vlan3, self.if6)
        self.client.switch_api_vlan_member_remove(device, self.vlan4, self.if6)
        self.client.switch_api_vlan_member_remove(device, self.vlan1, self.if7)
        self.client.switch_api_vlan_member_remove(device, self.vlan2, self.if7)
        self.client.switch_api_vlan_member_remove(device, self.vlan3, self.if7)
        self.client.switch_api_vlan_member_remove(device, self.vlan4, self.if7)
        self.client.switch_api_vlan_member_remove(device, self.vlan1, self.if8)
        self.client.switch_api_vlan_member_remove(device, self.vlan2, self.if8)
        self.client.switch_api_vlan_member_remove(device, self.vlan3, self.if8)
        self.client.switch_api_vlan_member_remove(device, self.vlan4, self.if8)
        self.client.switch_api_vlan_member_remove(device, self.vlan4, self.if9)

        self.client.switch_api_interface_delete(device, self.if1)
        self.client.switch_api_interface_delete(device, self.if2)
        self.client.switch_api_interface_delete(device, self.if3)
        self.client.switch_api_interface_delete(device, self.if4)
        self.client.switch_api_interface_delete(device, self.if5)
        self.client.switch_api_interface_delete(device, self.if6)
        self.client.switch_api_interface_delete(device, self.if7)
        self.client.switch_api_interface_delete(device, self.if8)
        self.client.switch_api_interface_delete(device, self.if9)

        self.client.switch_api_rif_delete(device, self.rif4)
        self.client.switch_api_rif_delete(device, self.rif5)
        self.client.switch_api_rif_delete(device, self.rif20)
        self.client.switch_api_rif_delete(device, self.rif21)
        self.client.switch_api_rif_delete(device, self.rif22)

        self.client.switch_api_vlan_delete(device, self.vlan1)
        self.client.switch_api_vlan_delete(device, self.vlan2)
        self.client.switch_api_vlan_delete(device, self.vlan3)
        self.client.switch_api_vlan_delete(device, self.vlan4)

        self.client.switch_api_router_mac_delete(device, self.rmac,
                                                 '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(device, self.rmac)
        self.client.switch_api_vrf_delete(device, self.vrf)

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('l3')
@group('mcast')
@group('tunnel')
@group('maxsizes')
class L3Multicast_with_tunnel(pd_base_tests.ThriftInterfaceDataPlane,
                  api_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def setUp(self):
        print
        print 'Configuring devices for L3 multicast test cases'

        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.devport = []
        for i in range(0,8):
            self.devport.append(swport_to_devport(self, swports[i]))

        self.cpu_port = get_cpu_port(self)

        # initialize the variables
        self.mch3 = ""
        self.rpf4 = ""
        self.rpf5 = ""
        self.rpf6 = ""

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.client.switch_api_init(device)

        self.vrf = self.client.switch_api_vrf_create(device, 2)
        self.rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, self.rmac,
                                              '00:77:66:55:44:33')

        self.port0 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[0])
        self.port1 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[1])
        self.port2 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[2])
        self.port3 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[3])
        self.port4 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[4])
        self.port5 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[5])
        self.port6 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[6])
        self.port7 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[7])
        self.port8 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[8])

        # vlans: 10, 100, 200, 300
        self.vlan1 = self.client.switch_api_vlan_create(device, 10)
        self.vlan2 = self.client.switch_api_vlan_create(device, 100)
        self.vlan3 = self.client.switch_api_vlan_create(device, 200)
        self.vlan4 = self.client.switch_api_vlan_create(device, 300)

        # disable learning
        self.client.switch_api_vlan_learning_set(device, self.vlan1, 0)
        self.client.switch_api_vlan_learning_set(device, self.vlan2, 0)
        self.client.switch_api_vlan_learning_set(device, self.vlan3, 0)
        self.client.switch_api_vlan_learning_set(device, self.vlan4, 0)

        # enable igmp snooping on vlan3 and vlan4
        self.client.switch_api_vlan_igmp_snooping_set(device, self.vlan3, 1)
        self.client.switch_api_vlan_igmp_snooping_set(device, self.vlan4, 1)

        # port 0: access port in vlan 10
        i_info1 = switcht_interface_info_t(
            handle=self.port0, type=SWITCH_INTERFACE_TYPE_ACCESS)
        self.if1 = self.client.switch_api_interface_create(device, i_info1)
        self.client.switch_api_vlan_member_add(device, self.vlan1, self.if1)

        # port 1: trunk port; allowed vlans: 10, 100, 200
        i_info2 = switcht_interface_info_t(
            handle=self.port1, type=SWITCH_INTERFACE_TYPE_TRUNK)
        self.if2 = self.client.switch_api_interface_create(device, i_info2)
        self.client.switch_api_vlan_member_add(device, self.vlan1, self.if2)
        self.client.switch_api_vlan_member_add(device, self.vlan2, self.if2)
        self.client.switch_api_vlan_member_add(device, self.vlan3, self.if2)

        # port 2: access port in vlan 100
        i_info3 = switcht_interface_info_t(
            handle=self.port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        self.if3 = self.client.switch_api_interface_create(device, i_info3)
        self.client.switch_api_vlan_member_add(device, self.vlan2, self.if3)

        rif_info4 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_multicast_enabled=1,
            v6_multicast_enabled=1)
        rif_info5 = rif_info4
        self.rif4 = self.client.switch_api_rif_create(0, rif_info4)
        self.rif5 = self.client.switch_api_rif_create(0, rif_info5)
        # port 3: routed port
        i_info4 = switcht_interface_info_t(
            handle=self.port3,
            type=SWITCH_INTERFACE_TYPE_PORT,
            rif_handle=self.rif4)
        self.if4 = self.client.switch_api_interface_create(device, i_info4)

        self.ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.250.1',
            prefix_length=24)
        self.client.switch_api_l3_interface_address_add(device, self.rif4,
                                                        self.vrf, self.ip4)

        # port 4: routed port
        i_info5 = switcht_interface_info_t(
            handle=self.port4,
            type=SWITCH_INTERFACE_TYPE_PORT,
            rif_handle=self.rif5)
        self.if5 = self.client.switch_api_interface_create(device, i_info5)
        self.ip5 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.251.1',
            prefix_length=24)
        self.client.switch_api_l3_interface_address_add(device, self.rif5,
                                                        self.vrf, self.ip5)

        # port 5: trunk port; allowed vlans: 10, 100, 200, 300
        i_info6 = switcht_interface_info_t(
            handle=self.port5, type=SWITCH_INTERFACE_TYPE_TRUNK)
        self.if6 = self.client.switch_api_interface_create(device, i_info6)
        self.client.switch_api_vlan_member_add(device, self.vlan1, self.if6)
        self.client.switch_api_vlan_member_add(device, self.vlan2, self.if6)
        self.client.switch_api_vlan_member_add(device, self.vlan3, self.if6)
        self.client.switch_api_vlan_member_add(device, self.vlan4, self.if6)

        # port 6: trunk port; allowed vlans: 10, 100, 200, 300
        i_info7 = switcht_interface_info_t(
            handle=self.port6, type=SWITCH_INTERFACE_TYPE_TRUNK)
        self.if7 = self.client.switch_api_interface_create(device, i_info7)
        self.client.switch_api_vlan_member_add(device, self.vlan1, self.if7)
        self.client.switch_api_vlan_member_add(device, self.vlan2, self.if7)
        self.client.switch_api_vlan_member_add(device, self.vlan3, self.if7)
        self.client.switch_api_vlan_member_add(device, self.vlan4, self.if7)

        # port 7: trunk port; allowed vlans: 10, 100, 200, 300
        i_info8 = switcht_interface_info_t(
            handle=self.port7, type=SWITCH_INTERFACE_TYPE_TRUNK)
        self.if8 = self.client.switch_api_interface_create(device, i_info8)
        self.client.switch_api_vlan_member_add(device, self.vlan1, self.if8)
        self.client.switch_api_vlan_member_add(device, self.vlan2, self.if8)
        self.client.switch_api_vlan_member_add(device, self.vlan3, self.if8)
        self.client.switch_api_vlan_member_add(device, self.vlan4, self.if8)

        # port 8: trunk port; allowed vlans: 300
        i_info9 = switcht_interface_info_t(
            handle=self.port8, type=SWITCH_INTERFACE_TYPE_TRUNK)
        self.if9 = self.client.switch_api_interface_create(device, i_info9)
        self.client.switch_api_vlan_member_add(device, self.vlan4, self.if9)

        rif_info20 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1,
            v4_multicast_enabled=1,
            v6_multicast_enabled=1)
        self.rif20 = self.client.switch_api_rif_create(0, rif_info20)
        rif_info21 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=100,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1,
            v4_multicast_enabled=1,
            v6_multicast_enabled=1)
        self.rif21 = self.client.switch_api_rif_create(0, rif_info21)
        rif_info22 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=200,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1,
            v4_multicast_enabled=1,
            v6_multicast_enabled=1)
        self.rif22 = self.client.switch_api_rif_create(0, rif_info22)

        # Create L3 virtual interface for vlan 10
        self.ip20 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.10.1',
            prefix_length=24)
        self.client.switch_api_l3_interface_address_add(device, self.rif20,
                                                        self.vrf, self.ip20)

        # Create L3 virtual interface for vlan 100
        self.ip21 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.100.1',
            prefix_length=24)
        self.client.switch_api_l3_interface_address_add(device, self.rif21,
                                                        self.vrf, self.ip21)

        # Create L3 virtual interface for vlan 200
        self.ip22 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.200.1',
            prefix_length=24)
        self.client.switch_api_l3_interface_address_add(device, self.rif22,
                                                        self.vrf, self.ip22)

        # create inner multicast tree
        self.mch1 = self.client.switch_api_multicast_tree_create(device)

        # vlan ports
        self.route_ports = [
            switcht_mcast_member_t(network_handle=self.vlan1, intf_handle=self.if1),
            switcht_mcast_member_t(network_handle=self.vlan1, intf_handle=self.if2),
            switcht_mcast_member_t(network_handle=self.vlan1, intf_handle=self.if6),
            switcht_mcast_member_t(network_handle=self.vlan1, intf_handle=self.if7),
            switcht_mcast_member_t(network_handle=self.vlan1, intf_handle=self.if8),
            switcht_mcast_member_t(network_handle=self.vlan2, intf_handle=self.if2),
            switcht_mcast_member_t(network_handle=self.vlan2, intf_handle=self.if3),
            switcht_mcast_member_t(network_handle=self.vlan2, intf_handle=self.if6),
            switcht_mcast_member_t(network_handle=self.vlan2, intf_handle=self.if7),
            switcht_mcast_member_t(network_handle=self.vlan2, intf_handle=self.if8),
            switcht_mcast_member_t(network_handle=self.vlan3, intf_handle=self.if6),
            switcht_mcast_member_t(network_handle=self.vlan3, intf_handle=self.if7),
            switcht_mcast_member_t(network_handle=self.vlan3, intf_handle=self.if8),
            switcht_mcast_member_t(network_handle=0, intf_handle=self.rif4),
            switcht_mcast_member_t(network_handle=0, intf_handle=self.rif5),
        ]
        self.client.switch_api_multicast_member_add(device, self.mch1,
                                                    self.route_ports)

        # create a ip multicast route (10.0.10.5,230.1.1.5)
        self.msrc_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.10.5',
            prefix_length=32)
        self.mgrp_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='230.1.1.5',
            prefix_length=32)
        self.rpf1 = self.client.switch_api_rpf_create(device, 2, 1)
        self.client.switch_api_rpf_member_add(device, self.rpf1, self.vlan1)
        self.client.switch_api_multicast_mroute_add(device, self.mch1,
                                                    self.rpf1, self.vrf,
                                                    self.msrc_ip1,
                                                    self.mgrp_ip1, 1)

        # create a snooping entry for vlan 200
        self.mch2 = self.client.switch_api_multicast_tree_create(device)
        # vlan 200 ports
        self.snoop_ports = [
            switcht_mcast_member_t(network_handle=self.vlan3, intf_handle=self.if6),
            switcht_mcast_member_t(network_handle=self.vlan3, intf_handle=self.if7),
            switcht_mcast_member_t(network_handle=self.vlan3, intf_handle=self.if8)
        ]
        self.client.switch_api_multicast_member_add(device, self.mch2,
                                                    self.snoop_ports)
        self.client.switch_api_multicast_l2route_add(
            device, self.mch2, self.vlan3, self.msrc_ip1, self.mgrp_ip1)

        # vlan 400 ports
        self.miss_ports = [
            switcht_mcast_member_t(network_handle=self.vlan4, intf_handle=self.if7),
            switcht_mcast_member_t(network_handle=self.vlan4, intf_handle=self.if8)
        ]
        # create mroute miss tree
        self.mch0 = self.client.switch_api_multicast_tree_create(device)
        self.client.switch_api_multicast_member_add(device, self.mch0,
                                                    self.miss_ports)
        self.client.switch_api_multicast_mroute_miss_mgid_set(device, self.mch0,
                                                    self.vlan4)

    def updateMulticastMroute(self):
        print
        print 'Configuring devices for L3 multicast update test cases'

        # create new inner multicast tree
        self.mch3 = self.client.switch_api_multicast_tree_create(device)
        self.client.switch_api_multicast_member_add(device, self.mch3,
                                                    self.route_ports)

        # update a ip multicast route (10.0.10.5,230.1.1.5)
        self.client.switch_api_multicast_mroute_mgid_set(device,
                                                    self.mch3, self.vrf,
                                                    self.msrc_ip1,
                                                    self.mgrp_ip1, 1)
        self.rpf4 = self.client.switch_api_rpf_create(device, 2, 1)
        self.client.switch_api_rpf_member_add(device, self.rpf4, self.vlan1)
        self.client.switch_api_multicast_mroute_rpf_set(device,
                                                    self.rpf4, self.vrf,
                                                    self.msrc_ip1,
                                                    self.mgrp_ip1, 1)

    def allTestCases(self):
        print "IPv4 multicast hit (RPF pass)"
        pkt = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=64,
            pktlen=100)
        pkt1 = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=64,
            pktlen=104)
        pkt2 = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:77:66:55:44:33',
            dl_vlan_enable=True,
            vlan_vid=100,
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=63,
            pktlen=104)
        pkt3 = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:77:66:55:44:33',
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=63,
            pktlen=100)
        pkt4 = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:77:66:55:44:33',
            dl_vlan_enable=True,
            vlan_vid=200,
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=63,
            pktlen=104)
        send_packet(self, swports[0], str(pkt))
        p1 = [swports[1], [pkt1, pkt2]]
        p2 = [swports[2], [pkt3]]
        p3 = [swports[3], [pkt3]]
        p4 = [swports[4], [pkt3]]
        p5 = [swports[5], [pkt1, pkt2, pkt4]]
        p6 = [swports[6], [pkt1, pkt2, pkt4]]
        p7 = [swports[7], [pkt1, pkt2, pkt4]]
        verify_multiple_packets_on_ports(self, [p1, p2, p3, p4, p5, p6, p7])

        print "IPv4 multicast hit (RPF fail - flood in ingress vlan)"
        pkt = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=64,
            pktlen=100)
        pkt1 = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            dl_vlan_enable=True,
            vlan_vid=100,
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=64,
            pktlen=104)
        send_packet(self, swports[2], str(pkt))
        p1 = [swports[1], [pkt1]]
        p5 = [swports[5], [pkt1]]
        p6 = [swports[6], [pkt1]]
        p7 = [swports[7], [pkt1]]
        verify_multiple_packets_on_ports(self, [p1, p5, p6, p7])

        print "IPv4 multicast hit (Bridge hit, RPF fail, snooping enabled, mroute no)"
        pkt = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            dl_vlan_enable=True,
            vlan_vid=200,
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=64,
            pktlen=100)
        send_packet(self, swports[6], str(pkt))
        p5 = [swports[5], [pkt]]
        p7 = [swports[7], [pkt]]
        verify_multiple_packets_on_ports(self, [p5, p7])

        print "IPv4 multicast hit (Bridge miss, RPF fail, snooping enabled, mroute yes)"
        pkt = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            dl_vlan_enable=True,
            vlan_vid=300,
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=64,
            pktlen=100)
        send_packet(self, swports[8], str(pkt))
        p6 = [swports[6], [pkt]]
        p7 = [swports[7], [pkt]]
        verify_multiple_packets_on_ports(self, [p6, p7])
        verify_no_other_packets(self)

        print "IPv4 multicast miss (Bridge miss, snooping enabled, mroute no)"
        pkt = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            dl_vlan_enable=True,
            vlan_vid=200,
            ip_src='10.0.10.5',
            ip_dst='231.1.1.5',
            ip_ttl=64,
            pktlen=100)
        send_packet(self, swports[6], str(pkt))
        verify_multiple_packets_on_ports(self, [])

        print "IPv4 multicast miss (Bridge miss - snooping enabled, mroute yes)"
        pkt = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            dl_vlan_enable=True,
            vlan_vid=300,
            ip_src='10.0.10.5',
            ip_dst='231.1.1.5',
            ip_ttl=64,
            pktlen=100)
        send_packet(self, swports[8], str(pkt))
        p6 = [swports[6], [pkt]]
        p7 = [swports[7], [pkt]]
        verify_multiple_packets_on_ports(self, [p6, p7])
        verify_no_other_packets(self)

        print "IPv4 multicast miss (snooping disabled)"
        pkt = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            dl_vlan_enable=True,
            vlan_vid=100,
            ip_src='10.0.10.5',
            ip_dst='231.1.1.5',
            ip_ttl=64,
            pktlen=100)
        pkt1 = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            ip_src='10.0.10.5',
            ip_dst='231.1.1.5',
            ip_ttl=64,
            pktlen=96)
        ingress_ifindex = self.client.switch_api_interface_ifindex_get(device,
                                                                       self.if7)
        cpu_pkt = simple_cpu_packet(
            ingress_port=self.devport[6],
            ingress_ifindex=ingress_ifindex,
            reason_code=0,
            ingress_bd=3,
            inner_pkt=pkt)
        send_packet(self, swports[6], str(pkt))
        p1 = [swports[1], [pkt]]
        p2 = [swports[2], [pkt1]]
        p5 = [swports[5], [pkt]]
        p7 = [swports[7], [pkt]]
        p64 = [self.cpu_port, [cpu_pkt]]
        verify_multiple_packets_on_ports(self, [p1, p2, p5, p7])

    def runTest(self):
        self.allTestCases()

        # Update the MGID and retry
        self.updateMulticastMroute()

        self.allTestCases()

    def tearDown(self):
        self.client.switch_api_mac_table_entry_flush(
            device, SWITCH_MAC_FLUSH_TYPE_ALL, 0x0, 0x0)

        # delete mroute and its tree
        self.client.switch_api_multicast_mroute_delete(
            device, self.vrf, self.msrc_ip1, self.mgrp_ip1)

        self.client.switch_api_rpf_member_delete(device, self.rpf1, self.vlan1)
        self.client.switch_api_rpf_delete(device, self.rpf1)
        if self.rpf4:
            self.client.switch_api_rpf_member_delete(device, self.rpf4, self.vlan1)
            self.client.switch_api_rpf_delete(device, self.rpf4)

        self.client.switch_api_multicast_member_delete(device, self.mch1,
                                                       self.route_ports)
        self.client.switch_api_multicast_tree_delete(device, self.mch1)
        if self.mch3:
            self.client.switch_api_multicast_member_delete(device, self.mch3,
                                                           self.route_ports)
            self.client.switch_api_multicast_tree_delete(device, self.mch3)

        # delete mroute miss tree and ports on vlan 300
        self.client.switch_api_multicast_member_delete(device, self.mch0,
                                                       self.miss_ports)
        self.client.switch_api_multicast_tree_delete(device, self.mch0)

        # delete snooping entry in vlan 200
        self.client.switch_api_multicast_l2route_delete(
            device, self.vlan3, self.msrc_ip1, self.mgrp_ip1)
        self.client.switch_api_multicast_member_delete(device, self.mch2,
                                                       self.snoop_ports)
        self.client.switch_api_multicast_tree_delete(device, self.mch2)

        self.client.switch_api_l3_interface_address_delete(device, self.rif4,
                                                           self.vrf, self.ip4)
        self.client.switch_api_l3_interface_address_delete(device, self.rif5,
                                                           self.vrf, self.ip5)
        self.client.switch_api_l3_interface_address_delete(device, self.rif20,
                                                           self.vrf, self.ip20)
        self.client.switch_api_l3_interface_address_delete(device, self.rif21,
                                                           self.vrf, self.ip21)
        self.client.switch_api_l3_interface_address_delete(device, self.rif22,
                                                           self.vrf, self.ip22)


        self.client.switch_api_vlan_member_remove(device, self.vlan1, self.if1)
        self.client.switch_api_vlan_member_remove(device, self.vlan1, self.if2)
        self.client.switch_api_vlan_member_remove(device, self.vlan2, self.if2)
        self.client.switch_api_vlan_member_remove(device, self.vlan3, self.if2)
        self.client.switch_api_vlan_member_remove(device, self.vlan2, self.if3)
        self.client.switch_api_vlan_member_remove(device, self.vlan1, self.if6)
        self.client.switch_api_vlan_member_remove(device, self.vlan2, self.if6)
        self.client.switch_api_vlan_member_remove(device, self.vlan3, self.if6)
        self.client.switch_api_vlan_member_remove(device, self.vlan4, self.if6)
        self.client.switch_api_vlan_member_remove(device, self.vlan1, self.if7)
        self.client.switch_api_vlan_member_remove(device, self.vlan2, self.if7)
        self.client.switch_api_vlan_member_remove(device, self.vlan3, self.if7)
        self.client.switch_api_vlan_member_remove(device, self.vlan4, self.if7)
        self.client.switch_api_vlan_member_remove(device, self.vlan1, self.if8)
        self.client.switch_api_vlan_member_remove(device, self.vlan2, self.if8)
        self.client.switch_api_vlan_member_remove(device, self.vlan3, self.if8)
        self.client.switch_api_vlan_member_remove(device, self.vlan4, self.if8)
        self.client.switch_api_vlan_member_remove(device, self.vlan4, self.if9)

        self.client.switch_api_interface_delete(device, self.if1)
        self.client.switch_api_interface_delete(device, self.if2)
        self.client.switch_api_interface_delete(device, self.if3)
        self.client.switch_api_interface_delete(device, self.if4)
        self.client.switch_api_interface_delete(device, self.if5)
        self.client.switch_api_interface_delete(device, self.if6)
        self.client.switch_api_interface_delete(device, self.if7)
        self.client.switch_api_interface_delete(device, self.if8)
        self.client.switch_api_interface_delete(device, self.if9)

        self.client.switch_api_rif_delete(device, self.rif4)
        self.client.switch_api_rif_delete(device, self.rif5)
        self.client.switch_api_rif_delete(device, self.rif20)
        self.client.switch_api_rif_delete(device, self.rif21)
        self.client.switch_api_rif_delete(device, self.rif22)

        self.client.switch_api_vlan_delete(device, self.vlan1)
        self.client.switch_api_vlan_delete(device, self.vlan2)
        self.client.switch_api_vlan_delete(device, self.vlan3)
        self.client.switch_api_vlan_delete(device, self.vlan4)

        self.client.switch_api_router_mac_delete(device, self.rmac,
                                                 '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(device, self.rmac)
        self.client.switch_api_vrf_delete(device, self.vrf)

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('l3')
@group('mcast')
@group('maxsizes')
class L3MulticastBidir(api_base_tests.ThriftInterfaceDataPlane):
    def setUp(self):
        print
        print 'Configuring devices for L3 multicast (Bidir) test cases'

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.vrf = self.client.switch_api_vrf_create(device, 2)
        self.rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, self.rmac,
                                              '00:77:66:55:44:33')

        self.port0 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[0])
        self.port1 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[1])
        self.port2 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[2])
        self.port3 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[3])
        self.port4 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[4])
        self.port5 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[5])
        self.port6 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[6])
        self.port7 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[7])

        # vlans: 10, 100, 200
        self.vlan1 = self.client.switch_api_vlan_create(device, 10)
        self.vlan2 = self.client.switch_api_vlan_create(device, 100)
        self.vlan3 = self.client.switch_api_vlan_create(device, 200)

        # disable learning
        self.client.switch_api_vlan_learning_set(device, self.vlan1, 0)
        self.client.switch_api_vlan_learning_set(device, self.vlan2, 0)
        self.client.switch_api_vlan_learning_set(device, self.vlan3, 0)

        # set RPF group
        self.client.switch_api_vlan_mrpf_group_set(device, self.vlan1, 0x0003)
        self.client.switch_api_vlan_mrpf_group_set(device, self.vlan2, 0x000A)
        self.client.switch_api_vlan_mrpf_group_set(device, self.vlan3, 0x0010)

        # enable igmp snooping on vlan3
        self.client.switch_api_vlan_igmp_snooping_set(device, self.vlan3, 1)

        # port 0: access port in vlan 10
        i_info1 = switcht_interface_info_t(
            handle=self.port0, type=SWITCH_INTERFACE_TYPE_ACCESS)
        self.if1 = self.client.switch_api_interface_create(device, i_info1)
        self.client.switch_api_vlan_member_add(device, self.vlan1, self.if1)

        # port 1: trunk port; allowed vlans: 10, 100, 200
        i_info2 = switcht_interface_info_t(
            handle=self.port1, type=SWITCH_INTERFACE_TYPE_TRUNK)
        self.if2 = self.client.switch_api_interface_create(device, i_info2)
        self.client.switch_api_vlan_member_add(device, self.vlan1, self.if2)
        self.client.switch_api_vlan_member_add(device, self.vlan2, self.if2)
        self.client.switch_api_vlan_member_add(device, self.vlan3, self.if2)

        # port 2: access port in vlan 100
        i_info3 = switcht_interface_info_t(
            handle=self.port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        self.if3 = self.client.switch_api_interface_create(device, i_info3)
        self.client.switch_api_vlan_member_add(device, self.vlan2, self.if3)

        # port 5: trunk port; allowed vlans: 10, 100, 200
        i_info6 = switcht_interface_info_t(
            handle=self.port5, type=SWITCH_INTERFACE_TYPE_TRUNK)
        self.if6 = self.client.switch_api_interface_create(device, i_info6)
        self.client.switch_api_vlan_member_add(device, self.vlan1, self.if6)
        self.client.switch_api_vlan_member_add(device, self.vlan2, self.if6)
        self.client.switch_api_vlan_member_add(device, self.vlan3, self.if6)

        # port 6: trunk port; allowed vlans: 10, 100, 200
        i_info7 = switcht_interface_info_t(
            handle=self.port6, type=SWITCH_INTERFACE_TYPE_TRUNK)
        self.if7 = self.client.switch_api_interface_create(device, i_info7)
        self.client.switch_api_vlan_member_add(device, self.vlan1, self.if7)
        self.client.switch_api_vlan_member_add(device, self.vlan2, self.if7)
        self.client.switch_api_vlan_member_add(device, self.vlan3, self.if7)

        # port 7: trunk port; allowed vlans: 10, 100, 200
        i_info8 = switcht_interface_info_t(
            handle=self.port7, type=SWITCH_INTERFACE_TYPE_TRUNK)
        self.if8 = self.client.switch_api_interface_create(device, i_info8)
        self.client.switch_api_vlan_member_add(device, self.vlan1, self.if8)
        self.client.switch_api_vlan_member_add(device, self.vlan2, self.if8)
        self.client.switch_api_vlan_member_add(device, self.vlan3, self.if8)

        rif_info20 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1,
            v4_multicast_enabled=1,
            v6_multicast_enabled=1)
        self.rif20 = self.client.switch_api_rif_create(0, rif_info20)
        rif_info21 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=100,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1,
            v4_multicast_enabled=1,
            v6_multicast_enabled=1)
        self.rif21 = self.client.switch_api_rif_create(0, rif_info21)
        rif_info22 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=200,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1,
            v4_multicast_enabled=1,
            v6_multicast_enabled=1)
        self.rif22 = self.client.switch_api_rif_create(0, rif_info22)

        # Create L3 virtual interface for vlan 10
        self.ip20 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.10.1',
            prefix_length=24)
        self.client.switch_api_l3_interface_address_add(device, self.rif20,
                                                        self.vrf, self.ip20)

        # Create L3 virtual interface for vlan 100
        self.ip21 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.100.1',
            prefix_length=24)
        self.client.switch_api_l3_interface_address_add(device, self.rif21,
                                                        self.vrf, self.ip21)

        # Create L3 virtual interface for vlan 200
        self.ip22 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.200.1',
            prefix_length=24)
        self.client.switch_api_l3_interface_address_add(device, self.rif22,
                                                        self.vrf, self.ip22)

        # create inner multicast tree
        self.mch1 = self.client.switch_api_multicast_tree_create(device)

        # vlan ports
        self.route_ports = [
            switcht_mcast_member_t(
                network_handle=self.vlan1,
                intf_handle=self.if1), switcht_mcast_member_t(
                    network_handle=self.vlan1,
                    intf_handle=self.if2), switcht_mcast_member_t(
                        network_handle=self.vlan1,
                        intf_handle=self.if6), switcht_mcast_member_t(
                            network_handle=self.vlan1,
                            intf_handle=self.if7), switcht_mcast_member_t(
                                network_handle=self.vlan1,
                                intf_handle=self.if8), switcht_mcast_member_t(
                                    network_handle=self.vlan2,
                                    intf_handle=self.if2),
            switcht_mcast_member_t(
                network_handle=self.vlan2,
                intf_handle=self.if3), switcht_mcast_member_t(
                    network_handle=self.vlan2,
                    intf_handle=self.if6), switcht_mcast_member_t(
                        network_handle=self.vlan2,
                        intf_handle=self.if7), switcht_mcast_member_t(
                            network_handle=self.vlan2,
                            intf_handle=self.if8), switcht_mcast_member_t(
                                network_handle=self.vlan3,
                                intf_handle=self.if6), switcht_mcast_member_t(
                                    network_handle=self.vlan3,
                                    intf_handle=self.if7),
            switcht_mcast_member_t(
                network_handle=self.vlan3, intf_handle=self.if8)
        ]
        self.client.switch_api_multicast_member_add(device, self.mch1,
                                                    self.route_ports)

        # create a ip multicast route (*,230.1.1.5)
        self.msrc_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='0.0.0.0', prefix_length=0)
        self.mgrp_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='230.1.1.5',
            prefix_length=32)
        self.rpid = ~0x0002 & 0xFFFF
        self.rpf = self.client.switch_api_rpf_create(device, 0, 2)
        self.client.switch_api_rpf_member_add(device, self.rpf, self.rpid)
        self.client.switch_api_multicast_mroute_add(device, self.mch1,
                                                    self.rpf, self.vrf,
                                                    self.msrc_ip1,
                                                    self.mgrp_ip1, 2)

        # create a snooping entry for vlan 200
        self.mch2 = self.client.switch_api_multicast_tree_create(device)
        # vlan 200 ports
        self.snoop_ports = [
            switcht_mcast_member_t(
                network_handle=self.vlan3,
                intf_handle=self.if6), switcht_mcast_member_t(
                    network_handle=self.vlan3, intf_handle=self.if7),
            switcht_mcast_member_t(
                network_handle=self.vlan3, intf_handle=self.if8)
        ]
        self.client.switch_api_multicast_member_add(device, self.mch2,
                                                    self.snoop_ports)
        self.client.switch_api_multicast_l2route_add(
            device, self.mch2, self.vlan3, self.msrc_ip1, self.mgrp_ip1)

    def runTest(self):
        print "IPv4 multicast (bidir) hit (RPF pass)"
        pkt = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=64,
            pktlen=100)
        pkt1 = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=64,
            pktlen=104)
        pkt2 = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:77:66:55:44:33',
            dl_vlan_enable=True,
            vlan_vid=100,
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=63,
            pktlen=104)
        pkt3 = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:77:66:55:44:33',
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=63,
            pktlen=100)
        pkt4 = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:77:66:55:44:33',
            dl_vlan_enable=True,
            vlan_vid=200,
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=63,
            pktlen=104)

        send_packet(self, swports[0], str(pkt))
        p1 = [swports[1], [pkt1, pkt2]]
        p2 = [swports[2], [pkt3]]
        p5 = [swports[5], [pkt1, pkt2, pkt4]]
        p6 = [swports[6], [pkt1, pkt2, pkt4]]
        p7 = [swports[7], [pkt1, pkt2, pkt4]]
        verify_multiple_packets_on_ports(self, [p1, p2, p5, p6, p7])

        print "IPv4 multicast (bidir) hit (RPF pass)"
        pkt5 = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            dl_vlan_enable=True,
            vlan_vid=100,
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=64,
            pktlen=104)
        pkt6 = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:77:66:55:44:33',
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=63,
            pktlen=104)
        send_packet(self, swports[1], str(pkt5))
        p0 = [swports[0], [pkt3]]
        p1 = [swports[1], [pkt6]]
        p2 = [swports[2], [pkt]]
        p5 = [swports[5], [pkt6, pkt5, pkt4]]
        p6 = [swports[6], [pkt6, pkt5, pkt4]]
        p7 = [swports[7], [pkt6, pkt5, pkt4]]
        verify_multiple_packets_on_ports(self, [p0, p1, p2, p5, p6, p7])

        print "IPv4 multicast (bidir) hit (RPF fail - snooping enabled)"
        pkt = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            dl_vlan_enable=True,
            vlan_vid=200,
            ip_src='10.0.10.5',
            ip_dst='230.1.1.5',
            ip_ttl=64,
            pktlen=100)
        send_packet(self, swports[6], str(pkt))
        p5 = [swports[5], [pkt]]
        p7 = [swports[7], [pkt]]
        verify_multiple_packets_on_ports(self, [p5, p7])

        print "IPv4 multicast (bidir) miss (snooping enabled)"
        pkt = simple_udp_packet(
            eth_dst='01:00:5e:01:01:05',
            eth_src='00:22:22:22:22:22',
            dl_vlan_enable=True,
            vlan_vid=200,
            ip_src='10.0.10.5',
            ip_dst='231.1.1.5',
            ip_ttl=64,
            pktlen=100)
        send_packet(self, swports[6], str(pkt))
        verify_multiple_packets_on_ports(self, [])

    def tearDown(self):
        self.client.switch_api_mac_table_entry_flush(
            device, SWITCH_MAC_FLUSH_TYPE_ALL, 0x0, 0x0)

        # delete mroute and its tree
        self.client.switch_api_multicast_mroute_delete(
            device, self.vrf, self.msrc_ip1, self.mgrp_ip1)

        self.client.switch_api_multicast_member_delete(device, self.mch1,
                                                       self.route_ports)
        self.client.switch_api_multicast_tree_delete(device, self.mch1)

        self.client.switch_api_rpf_member_delete(device, self.rpf, self.rpid)
        self.client.switch_api_rpf_delete(device, self.rpf)

        # delete snooping entry in vlan 200
        self.client.switch_api_multicast_l2route_delete(
            device, self.vlan3, self.msrc_ip1, self.mgrp_ip1)
        self.client.switch_api_multicast_member_delete(device, self.mch2,
                                                       self.snoop_ports)
        self.client.switch_api_multicast_tree_delete(device, self.mch2)

        self.client.switch_api_vlan_member_remove(device, self.vlan1, self.if1)
        self.client.switch_api_vlan_member_remove(device, self.vlan1, self.if2)
        self.client.switch_api_vlan_member_remove(device, self.vlan2, self.if2)
        self.client.switch_api_vlan_member_remove(device, self.vlan3, self.if2)
        self.client.switch_api_vlan_member_remove(device, self.vlan2, self.if3)
        self.client.switch_api_vlan_member_remove(device, self.vlan1, self.if6)
        self.client.switch_api_vlan_member_remove(device, self.vlan2, self.if6)
        self.client.switch_api_vlan_member_remove(device, self.vlan3, self.if6)
        self.client.switch_api_vlan_member_remove(device, self.vlan1, self.if7)
        self.client.switch_api_vlan_member_remove(device, self.vlan2, self.if7)
        self.client.switch_api_vlan_member_remove(device, self.vlan3, self.if7)
        self.client.switch_api_vlan_member_remove(device, self.vlan1, self.if8)
        self.client.switch_api_vlan_member_remove(device, self.vlan2, self.if8)
        self.client.switch_api_vlan_member_remove(device, self.vlan3, self.if8)

        self.client.switch_api_l3_interface_address_delete(device, self.rif20,
                                                           self.vrf, self.ip20)
        self.client.switch_api_l3_interface_address_delete(device, self.rif21,
                                                           self.vrf, self.ip21)
        self.client.switch_api_l3_interface_address_delete(device, self.rif22,
                                                           self.vrf, self.ip22)

        self.client.switch_api_interface_delete(device, self.if1)
        self.client.switch_api_interface_delete(device, self.if2)
        self.client.switch_api_interface_delete(device, self.if3)
        self.client.switch_api_interface_delete(device, self.if6)
        self.client.switch_api_interface_delete(device, self.if7)
        self.client.switch_api_interface_delete(device, self.if8)

        self.client.switch_api_rif_delete(device, self.rif20)
        self.client.switch_api_rif_delete(device, self.rif21)
        self.client.switch_api_rif_delete(device, self.rif22)

        self.client.switch_api_vlan_delete(device, self.vlan1)
        self.client.switch_api_vlan_delete(device, self.vlan2)
        self.client.switch_api_vlan_delete(device, self.vlan3)

        self.client.switch_api_router_mac_delete(device, self.rmac,
                                                 '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(device, self.rmac)
        self.client.switch_api_vrf_delete(device, self.vrf)

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


@group('l3')
@group('mcast')
class L3MulticastToEcmp(api_base_tests.ThriftInterfaceDataPlane):
    def setUp(self):
        print
        print 'Configuring devices for L3 multicast test cases'

	if (test_param_get('target') == 'bmv2' and
             test_param_get('arch') != 'Tofino'):
            return

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.client.switch_api_init(device)

        self.vrf = self.client.switch_api_vrf_create(device, 2)
        self.rmac = self.client.switch_api_router_mac_group_create(device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, self.rmac, '00:77:66:55:44:33')

        self.port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        self.port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        self.port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_multicast_enabled=1,
            v6_multicast_enabled=1)

        self.rif1 = self.client.switch_api_rif_create(0, rif_info)
        self.rif2 = self.client.switch_api_rif_create(0, rif_info)
        self.rif3 = self.client.switch_api_rif_create(0, rif_info)

        self.i_info1 = switcht_interface_info_t(
            handle=self.port1,
            type=SWITCH_INTERFACE_TYPE_PORT,
            rif_handle=self.rif1
        )
        self.if1 = self.client.switch_api_interface_create(device, self.i_info1)
        self.i_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='192.168.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device,
                                self.rif1, self.vrf, self.i_ip1)

        self.i_info2 = switcht_interface_info_t(
            handle=self.port2,
            type=SWITCH_INTERFACE_TYPE_PORT,
            rif_handle=self.rif2
        )
        self.if2 = self.client.switch_api_interface_create(device, self.i_info2)
        self.i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device,
                                self.rif2, self.vrf, self.i_ip2)

        self.i_info3 = switcht_interface_info_t(
            handle=self.port3,
            type=SWITCH_INTERFACE_TYPE_PORT,
            rif_handle=self.rif3
        )

        self.if3 = self.client.switch_api_interface_create(device, self.i_info3)
        self.i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='11.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device,
                                self.rif3, self.vrf, self.i_ip3)

        self.i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.1',
            prefix_length=32)
        self.nhop1, self.neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, self.rif2, self.i_ip4, '00:11:22:33:44:55')
        self.nhop2, self.neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, self.rif3, self.i_ip4, '00:11:22:33:44:56')

        self.ecmp = self.client.switch_api_ecmp_create(device)
        self.client.switch_api_ecmp_member_add(device, self.ecmp, 2,
                                               [self.nhop1, self.nhop2])

        # create multicast tree
        self.mch = self.client.switch_api_multicast_tree_create(device)

        # Add ECMP group to the tree
        self.client.switch_api_multicast_ecmp_nhop_add(device, self.mch,
                                                         self.ecmp)

        # create a ip multicast route (10.0.10.5,230.1.1.5)
        self.msrc_ip1 = switcht_ip_addr_t(addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='10.0.10.5',
                                         prefix_length=32)
        self.mgrp_ip1 = switcht_ip_addr_t(addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='230.1.1.5',
                                         prefix_length=32)

        rpflist = [ self.rif1 ]
        self.rpf = self.client.switch_api_rpf_create(device, 2, 1)
        self.client.switch_api_rpf_member_add(device, self.rpf, self.rif1)
        self.client.switch_api_multicast_mroute_add(device, self.mch,
                                                     self.rpf, self.vrf,
                                                     self.msrc_ip1,
                                                     self.mgrp_ip1, 1)

    def runTest(self):
	if (test_param_get('target') == 'bmv2' and
             test_param_get('arch') != 'Tofino'):
            return
        pkt = simple_udp_packet(eth_dst='01:00:5e:01:01:05',
                                eth_src='00:22:22:22:22:22',
                                ip_src='10.0.10.5',
                                ip_dst='230.1.1.5',
                                ip_ttl=64,
                                pktlen=100)

        exp_pkt = simple_udp_packet(eth_dst='01:00:5e:01:01:05',
                                    eth_src='00:77:66:55:44:33',
                                    ip_src='10.0.10.5',
                                    ip_dst='230.1.1.5',
                                    ip_ttl=63)

        send_packet(self, swports[1], str(pkt))
        verify_any_packet_any_port(self, [exp_pkt],
                                   [swports[2], swports[3]])

    def tearDown(self):
	if (test_param_get('target') == 'bmv2' and
             test_param_get('arch') != 'Tofino'):
            return
        self.client.switch_api_mac_table_entry_flush(device, SWITCH_MAC_FLUSH_TYPE_ALL, 0x0, 0x0)
        self.client.switch_api_multicast_mroute_delete(device,
                                                       self.vrf,
                                                       self.msrc_ip1,
                                                       self.mgrp_ip1)

        self.client.switch_api_rpf_member_delete(device, self.rpf, self.rif1)
        self.client.switch_api_rpf_delete(device, self.rpf)

        self.client.switch_api_multicast_ecmp_nhop_delete(device, self.mch,
                                                  self.ecmp)
        self.client.switch_api_multicast_tree_delete(device, self.mch)

        self.client.switch_api_ecmp_member_delete(device, self.ecmp, 2,
                                                 [self.nhop1, self.nhop2])

        self.client.switch_api_ecmp_delete(device, self.ecmp)

        self.client.switch_api_neighbor_delete(device, self.neighbor1)
        self.client.switch_api_nhop_delete(device, self.nhop1)

        self.client.switch_api_neighbor_delete(device, self.neighbor2)
        self.client.switch_api_nhop_delete(device, self.nhop2)

        self.client.switch_api_l3_interface_address_delete(device, self.rif1, self.vrf,
                                                           self.i_ip1)
        self.client.switch_api_l3_interface_address_delete(device, self.rif2, self.vrf,
                                                           self.i_ip2)
        self.client.switch_api_l3_interface_address_delete(device, self.rif3, self.vrf,
                                                           self.i_ip3)

        self.client.switch_api_interface_delete(device, self.if1)
        self.client.switch_api_interface_delete(device, self.if2)
        self.client.switch_api_interface_delete(device, self.if3)

        self.client.switch_api_rif_delete(device, self.rif3)
        self.client.switch_api_rif_delete(device, self.rif2)
        self.client.switch_api_rif_delete(device, self.rif1)

        self.client.switch_api_router_mac_delete(device, self.rmac,
                                                 '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(device, self.rmac)
        self.client.switch_api_vrf_delete(device, self.vrf)

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)

###############################################################################
@group('l3')
@group('mcast')
class L3MulticastStatsTest(pd_base_tests.ThriftInterfaceDataPlane,
                  api_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def setUp(self):
        print
        print 'Configuring devices for L3 multicast test cases'

        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.devport = []
        for i in range(0,8):
            self.devport.append(swport_to_devport(self, swports[i]))

        self.cpu_port = get_cpu_port(self)

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.client.switch_api_init(device)

        self.vrf = self.client.switch_api_vrf_create(device, 2)
        self.rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, self.rmac,
                                              '00:77:66:55:44:33')

        self.port0 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[0])
        self.port1 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[1])
        self.port2 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[2])

        # vlans: 10, 100
        self.vlan1 = self.client.switch_api_vlan_create(device, 10)
        self.vlan2 = self.client.switch_api_vlan_create(device, 100)

        # disable learning
        self.client.switch_api_vlan_learning_set(device, self.vlan1, 0)
        self.client.switch_api_vlan_learning_set(device, self.vlan2, 0)

        # port 0: access port in vlan 10
        i_info0 = switcht_interface_info_t(
            handle=self.port0, type=SWITCH_INTERFACE_TYPE_ACCESS)
        self.if0 = self.client.switch_api_interface_create(device, i_info0)
        self.client.switch_api_vlan_member_add(device, self.vlan1, self.if0)

        # port 1: trunk port; allowed vlans: 10, 100
        i_info1 = switcht_interface_info_t(
            handle=self.port1, type=SWITCH_INTERFACE_TYPE_TRUNK)
        self.if1 = self.client.switch_api_interface_create(device, i_info1)
        self.client.switch_api_vlan_member_add(device, self.vlan1, self.if1)
        self.client.switch_api_vlan_member_add(device, self.vlan2, self.if1)

        # port 5: trunk port; allowed vlans: 10, 100
        i_info2 = switcht_interface_info_t(
            handle=self.port2, type=SWITCH_INTERFACE_TYPE_TRUNK)
        self.if2 = self.client.switch_api_interface_create(device, i_info2)
        self.client.switch_api_vlan_member_add(device, self.vlan1, self.if2)
        self.client.switch_api_vlan_member_add(device, self.vlan2, self.if2)

        rif_info20 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1,
            v4_multicast_enabled=1,
            v6_multicast_enabled=1)
        self.rif20 = self.client.switch_api_rif_create(0, rif_info20)
        rif_info21 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=100,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1,
            v4_multicast_enabled=1,
            v6_multicast_enabled=1)
        self.rif21 = self.client.switch_api_rif_create(0, rif_info21)

        # Create L3 virtual interface for vlan 10
        self.ip20 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.10.1',
            prefix_length=24)
        self.client.switch_api_l3_interface_address_add(device, self.rif20,
                                                        self.vrf, self.ip20)

        # create inner multicast tree
        self.mch1 = self.client.switch_api_multicast_tree_create(device)

        # vlan ports
        self.route_ports = [
            switcht_mcast_member_t(
                network_handle=self.vlan1, intf_handle=self.if1),
            switcht_mcast_member_t(
                network_handle=self.vlan1, intf_handle=self.if2),
            switcht_mcast_member_t(
                network_handle=self.vlan2, intf_handle=self.if1),
            switcht_mcast_member_t(
                network_handle=self.vlan2, intf_handle=self.if2)
        ]
        self.client.switch_api_multicast_member_add(device, self.mch1,
                                                    self.route_ports)

        # create a ip multicast route (10.0.10.5,230.1.1.5)
        self.msrc_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.10.5',
            prefix_length=32)
        self.mgrp_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='230.1.1.5',
            prefix_length=32)
        self.rpf1 = self.client.switch_api_rpf_create(device, 2, 1)
        self.client.switch_api_rpf_member_add(device, self.rpf1, self.vlan1)
        self.client.switch_api_multicast_mroute_add(device, self.mch1,
                                                    self.rpf1, self.vrf,
                                                    self.msrc_ip1,
                                                    self.mgrp_ip1, 1)

    def runTest(self):
        print "L3 Multicast Stats test"
        # send the test packet(s)
        try:
            num_packets = 0
            stats0 = self.client.switch_api_multicast_mroute_stats_get(
                             0, self.vrf, self.msrc_ip1, self.mgrp_ip1)
            for i in range(0, 10):
                pkt = simple_udp_packet(
                    eth_dst='01:00:5e:01:01:05',
                    eth_src='00:22:22:22:22:22',
                    ip_src='10.0.10.5',
                    ip_dst='230.1.1.5',
                    ip_ttl=64,
                    pktlen=100)
                pkt1 = simple_udp_packet(
                    eth_dst='01:00:5e:01:01:05',
                    eth_src='00:22:22:22:22:22',
                    dl_vlan_enable=True,
                    vlan_vid=10,
                    ip_src='10.0.10.5',
                    ip_dst='230.1.1.5',
                    ip_ttl=64,
                    pktlen=104)
                pkt2 = simple_udp_packet(
                    eth_dst='01:00:5e:01:01:05',
                    eth_src='00:77:66:55:44:33',
                    dl_vlan_enable=True,
                    vlan_vid=100,
                    ip_src='10.0.10.5',
                    ip_dst='230.1.1.5',
                    ip_ttl=63,
                    pktlen=104)
                send_packet(self, swports[0], str(pkt))
                num_packets += 1
                p1 = [swports[1], [pkt1, pkt2]]
                p2 = [swports[2], [pkt1, pkt2]]
                verify_multiple_packets_on_ports(self, [p1, p2])
            time.sleep(2)
            stats = self.client.switch_api_multicast_mroute_stats_get(
                             0, self.vrf, self.msrc_ip1, self.mgrp_ip1)
            stats.num_packets = stats.num_packets - stats0.num_packets
            self.assertEqual(stats.num_packets, num_packets)

        finally:
            pass

    def tearDown(self):
        self.client.switch_api_mac_table_entry_flush(
            device, SWITCH_MAC_FLUSH_TYPE_ALL, 0x0, 0x0)

        # delete mroute and its tree
        self.client.switch_api_multicast_mroute_delete(
            device, self.vrf, self.msrc_ip1, self.mgrp_ip1)

        self.client.switch_api_rpf_member_delete(device, self.rpf1, self.vlan1)
        self.client.switch_api_rpf_delete(device, self.rpf1)

        self.client.switch_api_multicast_member_delete(device, self.mch1,
                                                       self.route_ports)
        self.client.switch_api_multicast_tree_delete(device, self.mch1)

        self.client.switch_api_l3_interface_address_delete(device, self.rif20,
                                                           self.vrf, self.ip20)

        self.client.switch_api_vlan_member_remove(device, self.vlan1, self.if0)
        self.client.switch_api_vlan_member_remove(device, self.vlan1, self.if1)
        self.client.switch_api_vlan_member_remove(device, self.vlan1, self.if2)
        self.client.switch_api_vlan_member_remove(device, self.vlan2, self.if1)
        self.client.switch_api_vlan_member_remove(device, self.vlan2, self.if2)

        self.client.switch_api_interface_delete(device, self.if0)
        self.client.switch_api_interface_delete(device, self.if1)
        self.client.switch_api_interface_delete(device, self.if2)

        self.client.switch_api_rif_delete(device, self.rif20)
        self.client.switch_api_rif_delete(device, self.rif21)

        self.client.switch_api_vlan_delete(device, self.vlan1)
        self.client.switch_api_vlan_delete(device, self.vlan2)

        self.client.switch_api_router_mac_delete(device, self.rmac,
                                                 '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(device, self.rmac)
        self.client.switch_api_vrf_delete(device, self.vrf)

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)
