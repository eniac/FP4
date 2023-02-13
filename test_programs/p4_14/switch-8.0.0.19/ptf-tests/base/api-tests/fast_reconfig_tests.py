################################################################################
# BAREFOOT NETWORKS CONFIDENTIAL & PROPRIETARY
#
# Copyright (c) 2015-2016 Barefoot Networks, Inc.

# All Rights Reserved.
#
# NOTICE: All information contained herein is, and remains the property of
# Barefoot Networks, Inc. and its suppliers, if any. The intellectual and
# technical concepts contained herein are proprietary to Barefoot Networks, # Inc.
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
Thrift API interface basic tests
"""

import switchapi_thrift

import time
import sys
import logging

import unittest
import random

import ptf.dataplane as dataplane
import api_base_tests
import pd_base_tests
try:
    import pltfm_pm_rpc
    from pltfm_pm_rpc.ttypes import *
except ImportError:
    pass

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *

import os
import ptf.mask

from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *
from devport_mgr_pd_rpc.ttypes import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '..'))
from common.utils import *
from common.api_utils import *
from common.api_adapter import ApiAdapter

device = 0
cpu_port = 64

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)

if swports == []:
    swports = [x for x in range(65)]

###############################################################################
@group('bfd')
@group('l2')
@group('maxsizes')
@group('2porttests')
@group('warminit')
class RegularWarmInitTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        if test_param_get('target') == "bmv2":
            return
        self.warm_init_begin(device)
	vlan = self.client.switch_api_vlan_create(device, 10)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)

        switch_api_mac_table_entry_create(
            self, device, vlan, '00:11:11:11:11:11', 2, if2)
        switch_api_mac_table_entry_create(
            self, device, vlan, '00:22:22:22:22:22', 2, if1)

        pkt = simple_udp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_ttl=64)
        exp_pkt = simple_udp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_ttl=64)
	self.devport_mgr.devport_mgr_warm_init_end(device)
        try:
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])
        finally:
            switch_api_mac_table_entry_delete(self, device, vlan,
                                                          '00:11:11:11:11:11')
            switch_api_mac_table_entry_delete(self, device, vlan,
                                                          '00:22:22:22:22:22')

            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_vlan_delete(device, vlan)

###############################################################################
@group('bfd')
@group('l2')
@group('maxsizes')
@group('2porttests')
@group('warminit')
class MultipleWarmInitTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        if test_param_get('target') == "bmv2":
            return
        print
	return
        print "Sending L2 packet port %d" % swports[0], "-> port %d" % swports[
            1], "[access vlan=10])"
        self.warm_init_begin(device)
	print "teaches after the first warm init begin"
	vlan = self.client.switch_api_vlan_create(device, 10)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)

        switch_api_mac_table_entry_create(
            self, device, vlan, '00:11:11:11:11:11', 2, if2)
        switch_api_mac_table_entry_create(
            self, device, vlan, '00:22:22:22:22:22', 2, if1)

        pkt = simple_udp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_ttl=64)
        exp_pkt = simple_udp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_ttl=64)
	self.devport_mgr.devport_mgr_warm_init_end(device)
        try:
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])
        finally:
            switch_api_mac_table_entry_delete(self, device, vlan,
                                                          '00:11:11:11:11:11')
            switch_api_mac_table_entry_delete(self, device, vlan,
                                                          '00:22:22:22:22:22')

            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_vlan_delete(device, vlan)
	self.warm_init_begin(device)
	vlan = self.client.switch_api_vlan_create(device, 10)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)

        switch_api_mac_table_entry_create(
            self, device, vlan, '00:11:11:11:11:11', 2, if2)
        switch_api_mac_table_entry_create(
            self, device, vlan, '00:22:22:22:22:22', 2, if1)

        pkt = simple_udp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_ttl=64)
        exp_pkt = simple_udp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_ttl=64)
	self.devport_mgr.devport_mgr_warm_init_end(device)
        try:
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])
        finally:
            switch_api_mac_table_entry_delete(self, device, vlan,
                                                          '00:11:11:11:11:11')
            switch_api_mac_table_entry_delete(self, device, vlan,
                                                          '00:22:22:22:22:22')

            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_vlan_delete(device, vlan)

###############################################################################
@group('bfd')
@group('l2')
@group('maxsizes')
@group('2porttests')
@group('warminit')
class NormalthanWarmInitTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        if test_param_get('target') == "bmv2":
            return
        print
	return
        print "Sending L2 packet port %d" % swports[0], "-> port %d" % swports[
            1], "[access vlan=10])"
	vlan = self.client.switch_api_vlan_create(device, 10)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)

        switch_api_mac_table_entry_create(
            self, device, vlan, '00:11:11:11:11:11', 2, if2)
        switch_api_mac_table_entry_create(
            self, device, vlan, '00:22:22:22:22:22', 2, if1)

        pkt = simple_udp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_ttl=64)
        exp_pkt = simple_udp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_ttl=64)
        send_packet(self, swports[0], str(pkt))
        verify_packets(self, exp_pkt, [swports[1]])
	self.warm_init_begin(device)
	vlan = self.client.switch_api_vlan_create(device, 10)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)

        switch_api_mac_table_entry_create(
            self, device, vlan, '00:11:11:11:11:11', 2, if2)
        switch_api_mac_table_entry_create(
            self, device, vlan, '00:22:22:22:22:22', 2, if1)

        pkt = simple_udp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_ttl=64)
        exp_pkt = simple_udp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_ttl=64)
	self.devport_mgr.devport_mgr_warm_init_end(device)
        try:
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])
        finally:
            switch_api_mac_table_entry_delete(self, device, vlan,
                                                          '00:11:11:11:11:11')
            switch_api_mac_table_entry_delete(self, device, vlan,
                                                          '00:22:22:22:22:22')

            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_vlan_delete(device, vlan)

###############################################################################


@group('bfd')
@group('l2')
@group('maxsizes')
@group('2porttests')
@group('warminit')
class FastReconfigBigTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        if test_param_get('target') == "bmv2":
            return
        print
	return

        print "Sending L2 packet port %d" % swports[0], "-> port %d" % swports[
            1], "[access vlan=10])"
        self.warm_init_begin(device)
        vlan = self.client.switch_api_vlan_create(device, 10)


        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)

        switch_api_mac_table_entry_create(
            self, device, vlan, '00:11:11:11:11:11', 2, if2)
        switch_api_mac_table_entry_create(
            self, device, vlan, '00:22:22:22:22:22', 2, if1)

        pkt = simple_udp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_ttl=64)
        exp_pkt = simple_udp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_ttl=64)
	self.devport_mgr.devport_mgr_warm_init_end(device)
        send_packet(self, swports[0], str(pkt))
        verify_packets(self, exp_pkt, [swports[1]])
        #Starting the Ecmp lag test
        self.warm_init_begin(device)
	vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])
        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[4])
        port5 = self.client.switch_api_port_id_to_handle_get(device, swports[5])
        port6 = self.client.switch_api_port_id_to_handle_get(device, swports[6])
        port7 = self.client.switch_api_port_id_to_handle_get(device, swports[7])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v4_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)
        i_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='192.168.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif1, vrf,
                                                        i_ip1)

        lag1 = self.client.switch_api_lag_create(device)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag1, side=SWITCH_API_DIRECTION_BOTH, port=port2)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag1, side=SWITCH_API_DIRECTION_BOTH, port=port3)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag1, side=SWITCH_API_DIRECTION_BOTH, port=port4)
        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v4_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=lag1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.2.2',
            prefix_length=24)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf,
                                                        i_ip2)

        lag2 = self.client.switch_api_lag_create(device)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag2, side=SWITCH_API_DIRECTION_BOTH, port=port5)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag2, side=SWITCH_API_DIRECTION_BOTH, port=port6)
        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v4_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif3 = self.client.switch_api_rif_create(0, rif_info3)
        i_info3 = switcht_interface_info_t(
            handle=lag2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.3.2',
            prefix_length=24)
        self.client.switch_api_l3_interface_address_add(device, rif3, vrf,
                                                        i_ip3)

        rif_info4 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v4_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif4 = self.client.switch_api_rif_create(0, rif_info4)
        i_info4 = switcht_interface_info_t(
            handle=port7, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif4)
        if4 = self.client.switch_api_interface_create(device, i_info4)
        i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.4.2',
            prefix_length=24)
        self.client.switch_api_l3_interface_address_add(device, rif4, vrf,
                                                        i_ip4)

        i_ip5 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.100.0.0',
            prefix_length=16)

        nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip5, '00:11:22:33:44:55')
        nhop2, neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, rif3, i_ip5, '00:11:22:33:44:56')
        nhop3, neighbor3 = switch_api_l3_nhop_neighbor_create(self, device, rif4, i_ip5, '00:11:22:33:44:57')

        ecmp = self.client.switch_api_ecmp_create(device)
        self.client.switch_api_ecmp_member_add(device, ecmp, 3,
                                               [nhop1, nhop2, nhop3])

        self.client.switch_api_l3_route_add(device, vrf, i_ip5, ecmp)
	#Resending Packet from Vlan Acces Test
	send_packet(self, swports[0], str(pkt))
        verify_packets(self, exp_pkt, [swports[1]])
	self.devport_mgr.devport_mgr_warm_init_end(device)


        count = [0, 0, 0, 0, 0, 0]
	dst_ip = int(socket.inet_aton('10.100.10.1').encode('hex'), 16)
	src_ip = int(socket.inet_aton('192.168.8.1').encode('hex'), 16)
	max_itrs = 500
	for i in range(0, max_itrs):
	       dst_ip_addr = socket.inet_ntoa(
		   format(dst_ip, 'x').zfill(8).decode('hex'))
	       src_ip_addr = socket.inet_ntoa(
		   format(src_ip, 'x').zfill(8).decode('hex'))
	       pkt = simple_tcp_packet(
		   eth_dst='00:77:66:55:44:33',
		   eth_src='00:22:22:22:22:22',
		   ip_dst=dst_ip_addr,
		   ip_src=src_ip_addr,
		   ip_id=106,
		   ip_ttl=64)

	       exp_pkt1 = simple_tcp_packet(
		   eth_dst='00:11:22:33:44:55',
		   eth_src='00:77:66:55:44:33',
		   ip_dst=dst_ip_addr,
		   ip_src=src_ip_addr,
		   ip_id=106,
		   ip_ttl=63)
	       exp_pkt2 = simple_tcp_packet(
		   eth_dst='00:11:22:33:44:56',
		   eth_src='00:77:66:55:44:33',
		   ip_dst=dst_ip_addr,
		   ip_src=src_ip_addr,
		   ip_id=106,
		   ip_ttl=63)
	       exp_pkt3 = simple_tcp_packet(
		   eth_dst='00:11:22:33:44:57',
		   eth_src='00:77:66:55:44:33',
		   ip_dst=dst_ip_addr,
		   ip_src=src_ip_addr,
		   ip_id=106,
		   ip_ttl=63)

	       send_packet(self, swports[1], str(pkt))
	       rcv_idx = verify_any_packet_any_port(
		   self, [exp_pkt1, exp_pkt2, exp_pkt3], [
		       swports[2], swports[3], swports[4], swports[5],
		       swports[6], swports[7]
		   ])
	       count[rcv_idx] += 1
	       dst_ip += 1
               src_ip += 1

        print 'ecmp-count:', count
        ecmp_count = [
                count[0] + count[1] + count[2], count[3] + count[4], count[5]
        ]
        for i in range(0, 3):
           self.assertTrue((ecmp_count[i] >= ((max_itrs / 3) * 0.5)),
                                "Ecmp paths are not equally balanced")
        for i in range(0, 3):
           self.assertTrue((count[i] >= ((max_itrs / 9) * 0.5)),
                                "Lag path1 is not equally balanced")
        for i in range(3, 5):
            self.assertTrue((count[i] >= ((max_itrs / 6) * 0.5)),
                                "Lag path2 is not equally balanced")

	print "Sending packet port %d" % swports[0], " -> port %d" % swports[
            1], " (2000::1 -> 3000::1, routing with 3000::0/120 route"
        self.warm_init_begin(device)
	vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v6_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)
        i_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='2000::2',
            prefix_length=120)
        self.client.switch_api_l3_interface_address_add(device, rif1, vrf,
                                                        i_ip1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v6_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='3000::2',
            prefix_length=120)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf,
                                                        i_ip2)

        # Add a static route
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='4000::0',
            prefix_length=124)
        nhop, neighbor = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip3, '00:99:99:99:99:99')
        self.client.switch_api_l3_route_add(device, vrf, i_ip3, nhop)

        # send the test packet(s)
        pkt = simple_tcpv6_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:22:22:22:22:22',
            ipv6_dst='4000::1',
            ipv6_src='2000::1',
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst='00:99:99:99:99:99',
            eth_src='00:77:66:55:44:33',
            ipv6_dst='4000::1',
            ipv6_src='2000::1',
            ipv6_hlim=63)
	self.devport_mgr.devport_mgr_warm_init_end(device)
        send_packet(self, swports[0], str(pkt))
        verify_packets(self, exp_pkt, [swports[1]])
        print "Starting the L2 Lag Flood Test"
        self.warm_init_begin(device)
	print "after the l2 lag flood test warm init"
        vlan = self.client.switch_api_vlan_create(device, 10)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])
        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[4])
        port5 = self.client.switch_api_port_id_to_handle_get(device, swports[5])
        port6 = self.client.switch_api_port_id_to_handle_get(device, swports[6])
        port7 = self.client.switch_api_port_id_to_handle_get(device, swports[7])
        port8 = self.client.switch_api_port_id_to_handle_get(device, swports[8])

        lag1 = self.client.switch_api_lag_create(device)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag1, side=SWITCH_API_DIRECTION_BOTH, port=port1)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag1, side=SWITCH_API_DIRECTION_BOTH, port=port2)
        i_info1 = switcht_interface_info_t(
            handle=lag1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        lag2 = self.client.switch_api_lag_create(device)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag2, side=SWITCH_API_DIRECTION_BOTH, port=port3)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag2, side=SWITCH_API_DIRECTION_BOTH, port=port4)
        i_info2 = switcht_interface_info_t(
            handle=lag2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        lag3 = self.client.switch_api_lag_create(device)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag3, side=SWITCH_API_DIRECTION_BOTH, port=port5)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag3, side=SWITCH_API_DIRECTION_BOTH, port=port6)
        i_info3 = switcht_interface_info_t(
            handle=lag3, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if3 = self.client.switch_api_interface_create(device, i_info3)

        i_info4 = switcht_interface_info_t(
            handle=port7, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if4 = self.client.switch_api_interface_create(device, i_info4)

        i_info5 = switcht_interface_info_t(
            handle=port8, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if5 = self.client.switch_api_interface_create(device, i_info5)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)
        self.client.switch_api_vlan_member_add(device, vlan, if3)
        self.client.switch_api_vlan_member_add(device, vlan, if4)
        self.client.switch_api_vlan_member_add(device, vlan, if5)
	self.devport_mgr.devport_mgr_warm_init_end(device)
	pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.0.0.1',
                ip_id=107,
                ip_ttl=64)
        exp_pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.0.0.1',
                ip_id=107,
                ip_ttl=64)

        print "Sending packet from lag1 on port1 -> lag2, lag3, port7, port8"
        send_packet(self, swports[1], str(pkt))
        verify_any_packet_on_ports_list(self, [exp_pkt], [[
	swports[3], swports[4]
        ], [swports[5], swports[6]], [swports[7]], [swports[8]]])
        print "Sending packet from lag1 on port2 -> lag2, lag3, port7, port8"
	send_packet(self, swports[2], str(pkt))
	verify_any_packet_on_ports_list(self, [exp_pkt], [[
	    swports[3], swports[4]
	], [swports[5], swports[6]], [swports[7]], [swports[8]]])
	print "Sending packet from lag2 on port3 -> lag1, lag3, port7, port8"
	send_packet(self, swports[3], str(pkt))
	verify_any_packet_on_ports_list(self, [exp_pkt], [[
	    swports[1], swports[2]
	], [swports[5], swports[6]], [swports[7]], [swports[8]]])
	print "Sending packet from lag2 on port4 -> lag1, lag3, port7, port8"
	send_packet(self, swports[4], str(pkt))
	verify_any_packet_on_ports_list(self, [exp_pkt], [[
	    swports[1], swports[2]
	], [swports[5], swports[6]], [swports[7]], [swports[8]]])
	print "Sending packet from lag3 on port5 -> lag1, lag2, port7, port8"
	send_packet(self, swports[5], str(pkt))
	verify_any_packet_on_ports_list(self, [exp_pkt], [[
	    swports[1], swports[2]
	], [swports[3], swports[4]], [swports[7]], [swports[8]]])
	print "Sending packet from lag3 on port6 -> lag1, lag2, port7, port8"
	send_packet(self, swports[6], str(pkt))
	verify_any_packet_on_ports_list(self, [exp_pkt], [[
	    swports[1], swports[2]
	], [swports[3], swports[4]], [swports[7]], [swports[8]]])
	print "Sending packet from port7 -> lag1, lag2, lag3, port8"
	send_packet(self, swports[7], str(pkt))
	verify_any_packet_on_ports_list(
	    self, [exp_pkt], [[swports[1], swports[2]],
			      [swports[3], swports[4]],
			      [swports[5], swports[6]], [swports[8]]])
	print "Sending packet from port7 -> lag1, lag2, lag3, port8"
	send_packet(self, swports[8], str(pkt))
	verify_any_packet_on_ports_list(
	    self, [exp_pkt], [[swports[1], swports[2]],
			      [swports[3], swports[4]],
			      [swports[5], swports[6]], [swports[7]]])
	self.devport_mgr.devport_mgr_warm_init_end(device)
	#Starting the L2VxlanUnicastBasic
        self.warm_init_begin(device)
	vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_OUTER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, vrf_handle=vrf, rmac_handle=rmac)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if3 = self.client.switch_api_interface_create(device, i_info3)

        # Create a logical network (LN)
        lognet_info = switcht_logical_network_t()
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)

        tunnel_mapper = switcht_tunnel_mapper_t(
            tunnel_vni=0x1234, ln_handle=ln1)
        mapper_handle = self.client.switch_api_tunnel_mapper_create(
            device,
            direction=SWITCH_API_DIRECTION_BOTH,
            tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
            tunnel_mapper_list=[tunnel_mapper])

        # Create a tunnel interface
        src_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.1', prefix_length=32)
        dst_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.3', prefix_length=32)
        tunnel_info = switcht_tunnel_info_t(
            vrf_handle=vrf,
            tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
            src_ip=src_ip,
            dst_ip=dst_ip,
            decap_mapper_handle=mapper_handle,
            encap_mapper_handle=mapper_handle,
            egress_rif_handle=rif2)
        ift4 = self.client.switch_api_tunnel_interface_create(
            device, SWITCH_API_DIRECTION_BOTH, tunnel_info)

        nhop_key1 = switcht_nhop_key_t(intf_handle=ift4, ip_addr_valid=0)
        nhop1 = self.client.switch_api_nhop_create(device, nhop_key1)

        # add L2 port to LN
        self.client.switch_api_logical_network_member_add(device, ln1, if1)
        self.client.switch_api_logical_network_member_add(device, ln1, if3)
        self.client.switch_api_logical_network_member_add(device, ln1, ift4)

        neighbor_entry1 = switcht_neighbor_info_t(
            nhop_handle=nhop1,
            interface_handle=ift4,
            mac_addr='00:33:33:33:33:33',
            ip_addr=src_ip,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L2,
            neigh_type=SWITCH_API_NEIGHBOR_IPV4_TUNNEL)
        neighbor1 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry1)

        neighbor_entry2 = switcht_neighbor_info_t(
            nhop_handle=0,
            interface_handle=ift4,
            mac_addr='00:33:33:33:33:33',
            ip_addr=src_ip)
        neighbor2 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry2)
	self.devport_mgr.devport_mgr_warm_init_end(device)

        try:
            pkt1 = simple_tcp_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            udp_sport = entropy_hash(pkt1)
            vxlan_pkt1 = simple_vxlan_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:33:33:33:33:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt1)

            pkt2 = simple_tcp_packet(
                eth_dst='00:44:44:44:44:44',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            vxlan_pkt2 = simple_vxlan_packet(
                eth_src='00:33:33:33:33:33',
                eth_dst='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                udp_sport=11638,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt2)

            pkt3 = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            vxlan_pkt3 = simple_vxlan_packet(
                eth_src='00:33:33:33:33:33',
                eth_dst='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                udp_sport=11638,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt3)
            print "Sending packet from Access port1 to Vxlan port2 and native port %d" % swports[
                3], "  - Learn mac on port1"
            send_packet(self, swports[1], str(pkt1))
            verify_each_packet_on_each_port(self, [vxlan_pkt1, pkt1],
                                            [swports[2], swports[3]])
            print "Sending packet from Vxlan port2 to Access port1 and native port %d" % swports[
                3], "  - Learn mac on vxlan tunnel port2"
            send_packet(self, swports[2], str(vxlan_pkt2))
            verify_packets(self, pkt2, [swports[1], swports[3]])
            time.sleep(3)
            print "Sending packet from Access port1 to Vxlan port2 - unicast to vxlan port2"
            send_packet(self, swports[1], str(pkt1))
            verify_packets(self, vxlan_pkt1, [swports[2]])
            print "Sending packet from Vxlan port2 to Access port1 - unicast to native port1"
            send_packet(self, swports[2], str(vxlan_pkt3))
            verify_packets(self, pkt3, [swports[1]])
        finally:
            self.client.switch_api_neighbor_delete(device, neighbor1)
            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_neighbor_delete(device, neighbor2)

            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if1)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if3)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift4)
            self.client.switch_api_logical_network_delete(device, ln1)

            self.client.switch_api_tunnel_interface_delete(device, ift4)
            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)

