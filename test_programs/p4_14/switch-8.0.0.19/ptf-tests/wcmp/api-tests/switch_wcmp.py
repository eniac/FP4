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
@group('wcmp')
class L3IPv4WcmpTest(api_base_tests.ThriftInterfaceDataPlane):
    def exp_pkts(self, src_port, dst_port, dst_ip_addr):
        exp_pkt1 = simple_tcp_packet(
            eth_dst='00:11:22:33:44:55',
            eth_src='00:77:66:55:44:33',
            ip_dst=dst_ip_addr,
            ip_src='192.168.8.1',
            ip_id=106,
            ip_ttl=63,
            tcp_sport=src_port,
            tcp_dport=dst_port)
        exp_pkt2 = simple_tcp_packet(
            eth_dst='00:11:22:33:44:56',
            eth_src='00:77:66:55:44:33',
            ip_dst=dst_ip_addr,
            ip_src='192.168.8.1',
            ip_id=106,
            ip_ttl=63,
            tcp_sport=src_port,
            tcp_dport=dst_port)
        exp_pkt3 = simple_tcp_packet(
            eth_dst='00:11:22:33:44:57',
            eth_src='00:77:66:55:44:33',
            ip_dst=dst_ip_addr,
            ip_src='192.168.8.1',
            ip_id=106,
            ip_ttl=63,
            tcp_sport=src_port,
            tcp_dport=dst_port)
        exp_pkt4 = simple_tcp_packet(
            eth_dst='00:11:22:33:44:58',
            eth_src='00:77:66:55:44:33',
            ip_dst=dst_ip_addr,
            ip_src='192.168.8.1',
            ip_id=106,
            ip_ttl=63,
            tcp_sport=src_port,
            tcp_dport=dst_port)
        return [exp_pkt1, exp_pkt2, exp_pkt3, exp_pkt4]

    def add_interface(self, port, ip_addr, rmac, vrf):
        port_h = self.client.switch_api_port_id_to_handle_get(device, port)
        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        rif = self.client.switch_api_rif_create(0, rif_info)
        info = switcht_interface_info_t(
            handle=port_h, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif)
        interface = self.client.switch_api_interface_create(device, info)
        ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr=ip_addr, prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif, vrf, ip)
        return ip, interface, rif, port_h

    def runTest(self):
        print
        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        i_ip1, if1, rif1, p1 = self.add_interface(swports[0], '192.168.0.2',
                                                  rmac, vrf)
        i_ip2, if2, rif2, p2 = self.add_interface(swports[1], '10.0.0.2', rmac,
                                                  vrf)
        i_ip3, if3, rif3, p3 = self.add_interface(swports[2], '11.0.0.2', rmac,
                                                  vrf)
        i_ip4, if4, rif4, p4 = self.add_interface(swports[3], '12.0.0.2', rmac,
                                                  vrf)
        i_ip5, if5, rif5, p5 = self.add_interface(swports[4], '13.0.0.2', rmac,
                                                  vrf)

        n_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.100',
            prefix_length=32)
        nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, rif2, n_ip1, '00:11:22:33:44:55')

        n_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='11.0.0.100',
            prefix_length=32)
        nhop2, neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, rif3, n_ip2, '00:11:22:33:44:56')

        n_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='12.0.0.101',
            prefix_length=32)
        nhop3, neighbor3 = switch_api_l3_nhop_neighbor_create(self, device, rif4, n_ip3, '00:11:22:33:44:57')

        n_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='13.0.0.101',
            prefix_length=32)
        nhop4, neighbor4 = switch_api_l3_nhop_neighbor_create(self, device, rif5, n_ip4, '00:11:22:33:44:58')

        wcmp = self.client.switch_api_l3_wcmp_create(device)
        weights = [1, 2, 5, 8]
        status = self.client.switch_api_l3_wcmp_member_add(
            device, wcmp, 4, [nhop1, nhop2, nhop3, nhop4], weights)
        r_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.0.0',
            prefix_length=16)
        self.client.switch_api_l3_route_add(device, vrf, r_ip, wcmp)

        try:
            count = [0, 0, 0, 0]
            dst_ip = int(socket.inet_aton('10.10.10.1').encode('hex'), 16)
            max_itrs = 200
            random.seed(314157)
            for i in range(0, max_itrs):
                dst_ip_addr = socket.inet_ntoa(
                    hex(dst_ip)[2:].zfill(8).decode('hex'))
                src_port = random.randint(0, 65535)
                dst_port = random.randint(0, 65535)
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src='192.168.8.1',
                    ip_id=106,
                    ip_ttl=64,
                    tcp_sport=src_port,
                    tcp_dport=dst_port)

                send_packet(self, swports[0], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self,
                    self.exp_pkts(src_port, dst_port, dst_ip_addr),
                    [swports[1], swports[2], swports[3], swports[4]])
                count[rcv_idx] += 1
                dst_ip += 1

            print "wcmp load balancing result ", count
            for i in range(0, 4):
                self.assertTrue((count[i] >= (weights[i] *
                                              (max_itrs / sum(weights)) * 0.9)),
                                "Not all paths are proportionally balanced")

            # Modify
            weights = [7, 2, 0, 4]
            status = self.client.switch_api_l3_wcmp_member_modify(
                device, wcmp, 3, [nhop1, nhop3, nhop4], [7, 0, 4])
            count = [0, 0, 0, 0]
            dst_ip = int(socket.inet_aton('10.10.10.1').encode('hex'), 16)
            max_itrs = 200
            for i in range(0, max_itrs):
                dst_ip_addr = socket.inet_ntoa(
                    hex(dst_ip)[2:].zfill(8).decode('hex'))
                src_port = random.randint(0, 65535)
                dst_port = random.randint(0, 65535)
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src='192.168.8.1',
                    ip_id=106,
                    ip_ttl=64,
                    tcp_sport=src_port,
                    tcp_dport=dst_port)

                send_packet(self, swports[0], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self,
                    self.exp_pkts(src_port, dst_port, dst_ip_addr),
                    [swports[1], swports[2], swports[3], swports[4]])
                count[rcv_idx] += 1
                dst_ip += 1

            print "wcmp load balancing result", count
            for i in range(0, 4):
                self.assertTrue((count[i] >= (weights[i] *
                                              (max_itrs / sum(weights)) * 0.9)),
                                "Not all paths are proportionally balanced")

            # Delete
            status = self.client.switch_api_l3_wcmp_member_delete(device, wcmp,
                                                                  1, [nhop4])
            count = [0, 0, 0]
            dst_ip = int(socket.inet_aton('10.10.10.1').encode('hex'), 16)
            max_itrs = 200
            for i in range(0, max_itrs):
                dst_ip_addr = socket.inet_ntoa(
                    hex(dst_ip)[2:].zfill(8).decode('hex'))
                src_port = random.randint(0, 65535)
                dst_port = random.randint(0, 65535)
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src='192.168.8.1',
                    ip_id=106,
                    ip_ttl=64,
                    tcp_sport=src_port,
                    tcp_dport=dst_port)

                send_packet(self, swports[0], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self,
                    self.exp_pkts(src_port, dst_port, dst_ip_addr),
                    [swports[1], swports[2], swports[3], swports[4]])
                count[rcv_idx] += 1
                dst_ip += 1

            print "wcmp load balancing result ", count
            for i in range(0, 3):
                self.assertTrue((count[i] >= (weights[i] *
                                              (max_itrs / sum(weights)) * 0.9)),
                                "Not all paths are proportionally balanced")

        finally:
            self.client.switch_api_l3_route_delete(device, vrf, r_ip, wcmp)

            self.client.switch_api_l3_wcmp_member_delete(device, wcmp, 3,
                                                         [nhop1, nhop2, nhop3])
            self.client.switch_api_l3_wcmp_delete(device, wcmp)

            self.client.switch_api_neighbor_delete(device, neighbor1)
            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_neighbor_delete(device, neighbor2)
            self.client.switch_api_nhop_delete(device, nhop2)

            self.client.switch_api_neighbor_delete(device, neighbor3)
            self.client.switch_api_nhop_delete(device, nhop3)

            self.client.switch_api_neighbor_delete(device, neighbor4)
            self.client.switch_api_nhop_delete(device, nhop4)

            self.client.switch_api_l3_interface_address_delete(device, rif1,
                                                               vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(device, rif2,
                                                               vrf, i_ip2)
            self.client.switch_api_l3_interface_address_delete(device, rif3,
                                                               vrf, i_ip3)
            self.client.switch_api_l3_interface_address_delete(device, rif4,
                                                               vrf, i_ip4)
            self.client.switch_api_l3_interface_address_delete(device, rif5,
                                                               vrf, i_ip5)
            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)
            self.client.switch_api_interface_delete(device, if4)
            self.client.switch_api_interface_delete(device, if5)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)
            self.client.switch_api_rif_delete(0, rif3)
            self.client.switch_api_rif_delete(0, rif4)
            self.client.switch_api_rif_delete(0, rif5)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)
