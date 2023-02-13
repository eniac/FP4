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
Thrift API interface ACL tests
"""

import switchapi_thrift

import time
import sys
import logging

import unittest
import random
import pdb

import ptf.dataplane as dataplane

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *

import os
import ptf.mask

from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

from erspan3 import *

this_dir = os.path.dirname(os.path.abspath(__file__))

sys.path.append(os.path.join(this_dir, '../../base'))
sys.path.append(os.path.join(this_dir, '../../base/api-tests'))
from common.utils import *
from common.api_utils import *
import api_base_tests

device = 0
cpu_port = 64
swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)

if swports == []:
    swports = [x for x in range(65)]


@group('ila')
class L3ILALookupTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "IPv6 ILA lookup test in switchAPI."
        # The lookup is purely done in control plane (switchAPI), not using
        # dataplane tables."

        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v6_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='2000::2',
            prefix_length=120)
        nhop_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='3000::2',
            prefix_length=128)
        self.client.switch_api_l3_interface_address_add(device, rif1, vrf,
                                                        i_ip1)

        nhop_default = switch_api_nhop_create(self, device, rif1, nhop_ip1)
        sir = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6, ipaddr='3000::', prefix_length=128)
        ila_addr = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6, ipaddr='2000::', prefix_length=128)

        try:
            # Add a new mapping
            status = self.client.switch_api_ila_add(device, vrf, sir, ila_addr,
                                                    nhop_default)
            nhop_handle = self.client.switch_api_ila_lookup(device, vrf, sir)
            self.assertTrue(nhop_handle == nhop_default)

            # Update the mapping
            status = self.client.switch_api_ila_update(device, vrf, sir,
                                                       ila_addr, nhop_default)
            self.assertTrue(status == 0)

            # Add an existing mapping
            status = self.client.switch_api_ila_add(device, vrf, sir, ila_addr,
                                                    nhop_default)
            self.assertTrue(status == 6)  # ITEM ALREADY EXISTS

            # Delete the mapping
            status = self.client.switch_api_ila_delete(device, vrf, sir)
            self.assertTrue(status == 0)
            nhop_handle = self.client.switch_api_ila_lookup(device, vrf, sir)
            self.assertTrue(nhop_handle == 0)

            # Delete a deleted mapping
            status = self.client.switch_api_ila_delete(device, vrf, sir)
            self.assertTrue(status == 7)  # ITEM NOT FOUND

        finally:
            # clean up
            self.client.switch_api_nhop_delete(device, nhop_default)

            self.client.switch_api_l3_interface_address_delete(device, rif1,
                                                               vrf, i_ip1)
            self.client.switch_api_interface_delete(device, if1)

            self.client.switch_api_rif_delete(0, rif1)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)

            self.client.switch_api_vrf_delete(device, vrf)


@group('ila')
class L3ILATest(api_base_tests.ThriftInterfaceDataPlane):
    def add_interface(self, port_handle, ip_addr, vrf, rmac):
        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v6_unicast_enabled=True)
        rif = self.client.switch_api_rif_create(0, rif_info)
        info = switcht_interface_info_t(
            handle=port_handle, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif)

        interface = self.client.switch_api_interface_create(device, info)
        ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6, ipaddr=ip_addr, prefix_length=120)
        self.client.switch_api_l3_interface_address_add(device, rif, vrf, ip)
        return ip, interface, rif

    def runTest(self):
        print
        print "IPv6 ILA test"

        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        i_ip1, if1, rif1 = self.add_interface(port1, '2000::2', vrf, rmac)
        i_ip2, if2, rif2 = self.add_interface(port2, '3000::2', vrf, rmac)
        i_ip3, if3, rif3 = self.add_interface(port3, '4000::2', vrf, rmac)

        # ILA host 1 with locally unique identifier (type = 0x1)
        sir1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='4444::2000:0:0:1',
            prefix_length=128)
        # With checksum-neutral mapping
        ila_addr1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='1111::3000:0:0:2334',
            prefix_length=128)
        nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, rif1, ila_addr1, '00:11:22:33:44:55')
        self.client.switch_api_ila_add(device, vrf, sir1, ila_addr1, nhop1)

        # ILA host 2
        sir2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='4444::2000:0:0:2',
            prefix_length=128)
        ila_addr2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='2222::3000:0:0:1224',
            prefix_length=128)
        nhop2, neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, rif2, ila_addr2, '00:11:22:33:44:56')
        self.client.switch_api_ila_add(device, vrf, sir2, ila_addr2, nhop2)

        # Non-ILA host
        ip_host = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='4000:10::10',
            prefix_length=128)
        nhop3, neighbor3 = switch_api_l3_nhop_neighbor_create(self, device, rif3, ip_host, '00:11:22:33:44:57')
        self.client.switch_api_l3_route_add(device, vrf, ip_host, nhop3)

        try:
            # Scenario 1 : Task to Task
            pkt = simple_tcpv6_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:11:22:33:44:55',
                ipv6_dst='4444::2000:0:0:2',
                ipv6_src='4444::2000:0:0:1',
                ipv6_hlim=64)
            exp_pkt = simple_tcpv6_packet(
                eth_dst='00:11:22:33:44:56',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='2222::3000:0:0:1224',
                ipv6_src='4444::2000:0:0:1',
                ipv6_hlim=63)
            send_packet(self, swports[1], str(pkt))
            verify_any_packet_any_port(self, exp_pkt, [swports[2]])

            # Scenario 2 : Task to internet
            pkt = simple_tcpv6_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:11:22:33:44:55',
                ipv6_dst='4000:10::10',
                ipv6_src='4444::2000:0:0:1',
                ipv6_hlim=64)
            exp_pkt = simple_tcpv6_packet(
                eth_dst='00:11:22:33:44:57',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='4000:10::10',
                ipv6_src='4444::2000:0:0:1',
                ipv6_hlim=63)
            send_packet(self, swports[1], str(pkt))
            verify_any_packet_any_port(self, exp_pkt, [swports[3]])

            # Scenario 3 : Internet to Task
            pkt = simple_tcpv6_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:11:22:33:44:57',
                ipv6_dst='4444::2000:0:0:1',
                ipv6_src='4000:10::10',
                ipv6_hlim=64)
            exp_pkt = simple_tcpv6_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='1111::3000:0:0:2334',
                ipv6_src='4000:10::10',
                ipv6_hlim=63)
            send_packet(self, swports[3], str(pkt))
            verify_any_packet_any_port(self, exp_pkt, [swports[1]])

        finally:
            # clean up
            self.client.switch_api_ila_delete(device, vrf, sir1)
            self.client.switch_api_ila_delete(device, vrf, sir2)
            self.client.switch_api_l3_route_delete(device, vrf, ip_host, nhop3)

            self.client.switch_api_neighbor_delete(device, neighbor1)
            self.client.switch_api_neighbor_delete(device, neighbor2)
            self.client.switch_api_neighbor_delete(device, neighbor3)

            self.client.switch_api_nhop_delete(device, nhop1)
            self.client.switch_api_nhop_delete(device, nhop2)
            self.client.switch_api_nhop_delete(device, nhop3)

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
