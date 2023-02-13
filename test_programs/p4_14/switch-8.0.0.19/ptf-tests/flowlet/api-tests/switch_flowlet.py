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
Flowlet swithcing tests
"""

import switchapi_thrift

import time
import sys
import logging

import unittest
import random

import ptf.dataplane as dataplane

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *

import os
import ptf.mask

from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../base'))

from common.utils import *
from common.api_utils import *
sys.path.append(os.path.join(this_dir, '../../base/api-tests'))
import api_base_tests

device = 0
cpu_port = 64
swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)

if swports == []:
    swports = [x for x in range(65)]


###############################################################################
@group('flowlet')
class L3IPv4FlowletTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Sending packet from port %d -> 10.0.0.1" % swports[1]
        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info)
        rif2 = self.client.switch_api_rif_create(0, rif_info)
        rif3 = self.client.switch_api_rif_create(0, rif_info)

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)
        i_ip1 = switcht_ip_addr_t(
            addr_type=0, ipaddr='192.168.0.2', prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif1, vrf,
                                                        i_ip1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=0, ipaddr='10.0.0.2', prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf,
                                                        i_ip2)

        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)
        i_ip3 = switcht_ip_addr_t(
            addr_type=0, ipaddr='11.0.0.2', prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif3, vrf,
                                                        i_ip3)

        i_ip4 = switcht_ip_addr_t(
            addr_type=0, ipaddr='10.10.10.1', prefix_length=32)

        nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip4, '00:11:22:33:44:55')
        nhop2, neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, rif3, i_ip4, '00:11:22:33:44:56')

        ecmp = self.client.switch_api_ecmp_create(device)
        self.client.switch_api_ecmp_member_add(device, ecmp, 2, [nhop1, nhop2])

        self.client.switch_api_l3_route_add(device, vrf, i_ip4, ecmp)
        self.client.switch_api_flowlet_switching_set(device, 0)
        try:
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=106,
                ip_ttl=64)
            send_packet(self, swports[1], str(pkt))

            received = False
            for device_number, port in ptf_ports():
                if device != device_number or port not in swports[2:4]:
                    continue
                (_, rcv_port, rcv_pkt, _) = dp_poll(
                    self, device_number=device, port_number=port)
                if rcv_pkt != None:
                    received = True
                    break
            self.assertTrue(received == True,
                            "Did not receive expected pkt on any of the ports")
            exp_pkt1 = simple_tcp_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=106,
                ip_ttl=63)
            exp_pkt2 = simple_tcp_packet(
                eth_dst='00:11:22:33:44:56',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=106,
                ip_ttl=63)

            exp_pkts = {swports[2]: exp_pkt1, swports[3]: exp_pkt2}
            time.sleep(1)
            send_packet(self, swports[1], str(pkt))
            verify_any_packet_any_port(self, [exp_pkts[rcv_port]], [rcv_port],
                                       device)

            self.client.switch_api_flowlet_switching_set(device, 1)

            max_itrs = 100
            count = [0, 0]
            for i in range(max_itrs):
                send_packet(self, swports[1], str(pkt))
                rcv_idx = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                                     [swports[2], swports[3]],
                                                     device)
                count[rcv_idx] += 1

            print "Flowlet load balancing results ", count
            for i in range(2):
                self.assertTrue(count[i] >= max_itrs / 2 * 0.8,
                                "Two paths are not equally balanced")

        finally:
            self.client.switch_api_flowlet_switching_set(device, 0)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip4, ecmp)

            self.client.switch_api_ecmp_member_delete(device, ecmp, 2,
                                                      [nhop1, nhop2])
            self.client.switch_api_ecmp_delete(device, ecmp)

            self.client.switch_api_neighbor_delete(device, neighbor1)
            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_neighbor_delete(device, neighbor2)
            self.client.switch_api_nhop_delete(device, nhop2)

            self.client.switch_api_l3_interface_address_delete(device, rif1,
                                                               vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(device, rif2,
                                                               vrf, i_ip2)
            self.client.switch_api_l3_interface_address_delete(device, rif3,
                                                               vrf, i_ip3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)
            self.client.switch_api_rif_delete(0, rif3)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


@group('flowlet')
class L3IPv6FlowletTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v6_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info)
        rif2 = self.client.switch_api_rif_create(0, rif_info)
        rif3 = self.client.switch_api_rif_create(0, rif_info)

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)
        i_ip1 = switcht_ip_addr_t(
            addr_type=1, ipaddr='2000:1:1:0:0:0:0:1', prefix_length=120)
        self.client.switch_api_l3_interface_address_add(device, rif1, vrf,
                                                        i_ip1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=1, ipaddr='3000:1:1:0:0:0:0:1', prefix_length=120)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf,
                                                        i_ip2)

        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)
        i_ip3 = switcht_ip_addr_t(
            addr_type=1, ipaddr='4000:1:1:0:0:0:0:1', prefix_length=120)
        self.client.switch_api_l3_interface_address_add(device, rif3, vrf,
                                                        i_ip3)

        i_ip4 = switcht_ip_addr_t(
            addr_type=1, ipaddr='5000:1:1:0:0:0:0:1', prefix_length=128)

        nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip4, '00:11:22:33:44:55')
        nhop2, neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, rif3, i_ip4, '00:11:22:33:44:56')

        ecmp = self.client.switch_api_ecmp_create(device)
        self.client.switch_api_ecmp_member_add(device, ecmp, 2, [nhop1, nhop2])

        self.client.switch_api_l3_route_add(device, vrf, i_ip4, ecmp)

        try:
            pkt = simple_tcpv6_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ipv6_dst='5000:1:1:0:0:0:0:1',
                ipv6_src='2000:1:1:0:0:0:0:1',
                tcp_sport=0x1234,
                ipv6_hlim=64)
            exp_pkt1 = simple_tcpv6_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='5000:1:1:0:0:0:0:1',
                ipv6_src='2000:1:1:0:0:0:0:1',
                tcp_sport=0x1234,
                ipv6_hlim=63)
            exp_pkt2 = simple_tcpv6_packet(
                eth_dst='00:11:22:33:44:56',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='5000:1:1:0:0:0:0:1',
                ipv6_src='2000:1:1:0:0:0:0:1',
                tcp_sport=0x1234,
                ipv6_hlim=63)
            send_packet(self, swports[1], str(pkt))
            received = False
            for device_number, port in ptf_ports():
                if device != device_number or port not in swports[2:4]:
                    continue
                (_, rcv_port, rcv_pkt, _) = dp_poll(
                    self, device_number=device, port_number=port)
                if rcv_pkt != None:
                    received = True
                    break
            self.assertTrue(received == True,
                            "Did not receive expected pkt on any of the ports")

            exp_pkts = {swports[2]: exp_pkt1, swports[3]: exp_pkt2}
            time.sleep(1)
            send_packet(self, swports[1], str(pkt))
            verify_any_packet_any_port(self, [exp_pkts[rcv_port]], [rcv_port],
                                       device)

            self.client.switch_api_flowlet_switching_set(device, 1)

            max_itrs = 100
            count = [0, 0]
            for i in range(max_itrs):
                send_packet(self, swports[1], str(pkt))
                rcv_idx = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                                     [swports[2], swports[3]],
                                                     device)
                count[rcv_idx] += 1

            print "Flowlet load balancing results ", count
            for i in range(2):
                self.assertTrue(count[i] >= max_itrs / 2 * 0.8,
                                "Two paths are not equally balanced")

        finally:
            self.client.switch_api_flowlet_switching_set(device, 0)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip4, ecmp)

            self.client.switch_api_ecmp_member_delete(device, ecmp, 2,
                                                      [nhop1, nhop2])
            self.client.switch_api_ecmp_delete(device, ecmp)

            self.client.switch_api_neighbor_delete(device, neighbor1)
            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_neighbor_delete(device, neighbor2)
            self.client.switch_api_nhop_delete(device, nhop2)

            self.client.switch_api_l3_interface_address_delete(
                device, rif1, vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(
                device, rif2, vrf, i_ip2)
            self.client.switch_api_l3_interface_address_delete(
                device, rif3, vrf, i_ip3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)
            self.client.switch_api_rif_delete(0, rif3)

            self.client.switch_api_router_mac_delete(
                device, rmac, '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


@group('flowlet')
class L2LagFlowletTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        vlan = self.client.switch_api_vlan_create(device, 10)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port5 = self.client.switch_api_port_id_to_handle_get(device, swports[5])
        port6 = self.client.switch_api_port_id_to_handle_get(device, swports[6])
        port7 = self.client.switch_api_port_id_to_handle_get(device, swports[7])
        port8 = self.client.switch_api_port_id_to_handle_get(device, swports[8])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        lag = self.client.switch_api_lag_create(device)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=0, port=port5)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=0, port=port6)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=0, port=port7)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=0, port=port8)
        i_info2 = switcht_interface_info_t(
            handle=lag, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)

        switch_api_mac_table_entry_create(
            self, device, vlan, '00:11:11:11:11:11', 2, if2)
        switch_api_mac_table_entry_create(
            self, device, vlan, '00:22:22:22:22:22', 2, if1)

        self.client.switch_api_flowlet_switching_set(device, 1)

        try:
            count = [0, 0, 0, 0]
            pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='192.168.8.1',
                ip_id=109,
                ip_ttl=64)

            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='192.168.8.1',
                ip_id=109,
                ip_ttl=64)

            max_itrs = 200
            count = [0, 0, 0, 0]
            for i in range(max_itrs):
                send_packet(self, swports[1], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self, [exp_pkt, exp_pkt, exp_pkt, exp_pkt],
                    [swports[5], swports[6], swports[7], swports[8]])

                count[rcv_idx] += 1

            print 'Flowlet lag results:', count
            for i in range(0, 4):
                self.assertTrue((count[i] >= ((max_itrs / 4) * 0.7)),
                                "Not all paths are equally balanced")

            pkt = simple_tcp_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:22:22:22:22:22',
                ip_dst='10.0.0.1',
                ip_id=109,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:22:22:22:22:22',
                ip_dst='10.0.0.1',
                ip_id=109,
                ip_ttl=64)
            print('Sending packet port %d (lag member) -> port %d' %
                  (swports[5], swports[1]))
            send_packet(self, swports[5], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])
            print('Sending packet port %d (lag member) -> port %d' %
                  (swports[6], swports[1]))
            send_packet(self, swports[6], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])
            print('Sending packet port %d (lag member) -> port %d' %
                  (swports[7], swports[1]))
            send_packet(self, swports[7], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])
            print('Sending packet port %d (lag member) -> port %d' %
                  (swports[8], swports[1]))
            send_packet(self, swports[8], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])
        finally:
            self.client.switch_api_flowlet_switching_set(device, 0)
            switch_api_mac_table_entry_delete(self, device, vlan,
                                                          '00:22:22:22:22:22')
            switch_api_mac_table_entry_delete(self, device, vlan,
                                                          '00:11:11:11:11:11')

            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)

            self.client.switch_api_lag_member_delete(
                device, lag_handle=lag, side=0, port=port5)
            self.client.switch_api_lag_member_delete(
                device, lag_handle=lag, side=0, port=port6)
            self.client.switch_api_lag_member_delete(
                device, lag_handle=lag, side=0, port=port7)
            self.client.switch_api_lag_member_delete(
                device, lag_handle=lag, side=0, port=port8)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_lag_delete(device, lag)

            self.client.switch_api_vlan_delete(device, vlan)
