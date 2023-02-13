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
Thrift API interface basic tests
"""

import os
import ptf.mask
import switchapi_thrift
import sys
import unittest

from ptf.testutils import *
from ptf.thriftutils import *
from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../base'))
sys.path.append(os.path.join(this_dir, '../../base/api-tests'))
sys.path.append(os.path.join(this_dir, '../../base/common'))

from common.utils import *
import api_base_tests
from api_utils import *
from api_adapter import ApiAdapter

device = 0
cpu_port = 64
swports = [x for x in range(65)]


###############################################################################
@group('l3')
@group('ipv4')
@group('subinterface')
@group('maxsizes')
@group('ent')
class L3SubIntUpdateDeleteTest(ApiAdapter):

    def runTest(self):
        """
            Test subinterface update and delete

                            .1 +--------------------------+ .1
            10.0.10.5 >--------|p1.10  subinterface  p2.20|--------> 10.0.20.5
                               |                          |
                               +--------------------------+
        """
        print ""
        vrf = self.add_vrf(device, 2)

        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')
        port1 = self.select_port(device, swports[0])
        port2 = self.select_port(device, swports[1])

        # Create sub interfaces
        eth1_10 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg1 = self.cfg_subintf_on_port(device, port1, eth1_10, vlan_id=10)
        ipaddr1 = self.make_ipv4_ipaddr('10.0.10.1', 24)
        self.cfg_ip_address(device, eth1_10, vrf, ipaddr1)

        eth2_20 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg2 = self.cfg_subintf_on_port(device, port2, eth2_20, vlan_id=20)
        ipaddr2 = self.make_ipv4_ipaddr('10.0.20.1', 24)
        self.cfg_ip_address(device, eth2_20, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv4_ipaddr('10.0.10.5', 32)
        nhop1 = self.add_nhop(device, eth1_10)
        self.add_neighbor_l3intf(device, eth1_10, nhop1, '00:10:10:10:10:15',
            ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        ipaddr4 = self.make_ipv4_ipaddr('10.0.20.5', 32)
        nhop2 = self.add_nhop(device, eth2_20)
        self.add_neighbor_l3intf(device, eth2_20, nhop2, '00:20:20:20:20:25',
            ipaddr4)
        self.add_static_route(device, vrf, ipaddr4, nhop2)

        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            ip_id=105,
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:20:20:20:20:25',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            dl_vlan_enable=True,
            vlan_vid=20,
            ip_id=105,
            ip_ttl=63)

        try:
            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (10.0.10.5 -> 10.0.20.5 [id = 105])")
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

            # Delete ip ipaddr
            self.no_ip_address(device, eth2_20, vrf, ipaddr2)
            self.no_subintf_on_port(device, port2, ethcfg2)

            # Update with vlan-25
            self.cfg_subintf_on_port(device, port2, eth2_20, 25)
            self.cfg_ip_address(device, eth2_20, vrf, ipaddr2)

            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:10:10:10:10:15',
                ip_dst='10.0.20.5',
                ip_src='10.0.10.5',
                ip_id=105,
                dl_vlan_enable=True,
                vlan_vid=10,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:20:20:20:20:25',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.0.20.5',
                ip_src='10.0.10.5',
                dl_vlan_enable=True,
                vlan_vid=25,
                ip_id=105,
                ip_ttl=63)

            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (10.0.10.5 -> 10.0.20.5 [id = 105])")
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

        finally:
            self.cleanup()


###############################################################################
@group('l3')
@group('ipv4')
@group('subinterface')
@group('maxsizes')
@group('ent')
class L3SubIntDeleteAddRouteTest(ApiAdapter):

    def runTest(self):
        """
            Test subinterface delete and re-add route

                           .1 +--------------------------+ .1
            10.0.10.5 >-------|p1.10  subinterface  p2.20|--------> 10.0.20.5
                              |                          |
                              +--------------------------+
        """
        print ""
        vrf = self.add_vrf(device, 2)

        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')
        port1 = self.select_port(device, swports[0])
        port2 = self.select_port(device, swports[1])

        # Create sub interfaces
        eth1_10 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg1 = self.cfg_subintf_on_port(device, port1, eth1_10, vlan_id=10)
        ipaddr1 = self.make_ipv4_ipaddr('10.0.10.1', 24)
        self.cfg_ip_address(device, eth1_10, vrf, ipaddr1)

        eth2_20 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg2 = self.cfg_subintf_on_port(device, port2, eth2_20, vlan_id=20)
        ipaddr2 = self.make_ipv4_ipaddr('10.0.20.1', 24)
        self.cfg_ip_address(device, eth2_20, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv4_ipaddr('10.0.10.5', 32)
        nhop1 = self.add_nhop(device, eth1_10)
        neighbor1 = self.add_neighbor_l3intf(device, eth1_10, nhop1,
            '00:10:10:10:10:15', ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        ipaddr4 = self.make_ipv4_ipaddr('10.0.20.5', 32)
        nhop2 = self.add_nhop(device, eth2_20)
        neighbor2 = self.add_neighbor_l3intf(device, eth2_20, nhop2,
            '00:20:20:20:20:25', ipaddr4)
        self.add_static_route(device, vrf, ipaddr4, nhop2)

        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            ip_id=105,
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:20:20:20:20:25',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            dl_vlan_enable=True,
            vlan_vid=20,
            ip_id=105,
            ip_ttl=63)

        try:
            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (10.0.10.5 -> 10.0.20.5 [id = 105])")
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

            # Delete old route
            self.no_neighbor(device, neighbor2)
            self.no_static_route(device, vrf, ipaddr4, nhop2)
            self.no_nhop(device, nhop2)

            # Add a new route
            ipaddr4 = self.make_ipv4_ipaddr('10.0.20.6', 32)
            nhop2 = self.add_nhop(device, eth2_20)
            neighbor2 = self.add_neighbor_l3intf(device, eth2_20, nhop2,
                '00:20:20:20:20:26', ipaddr4)
            self.add_static_route(device, vrf, ipaddr4, nhop2)

            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:10:10:10:10:15',
                ip_dst='10.0.20.6',
                ip_src='10.0.10.5',
                ip_id=105,
                dl_vlan_enable=True,
                vlan_vid=10,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:20:20:20:20:26',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.0.20.6',
                ip_src='10.0.10.5',
                dl_vlan_enable=True,
                vlan_vid=20,
                ip_id=105,
                ip_ttl=63)

            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (10.0.10.5 -> 10.0.20.6 [id = 105])")
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

        finally:
            self.cleanup()


###############################################################################
@group('l3')
@group('ipv4')
@group('subinterface')
@group('maxsizes')
@group('ent')
class L3SubIntAddDuplicateRouteTest(ApiAdapter):

    def runTest(self):
        """
            Test subinterface add duplicate route

                           .1 +--------------------------+ .1
            10.0.10.5 >-------|p1.10  subinterface  p2.20|--------> 10.0.20.5
                              |                          |
                              +--------------------------+
        """
        print ""
        vrf = self.add_vrf(device, 2)

        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')
        port1 = self.select_port(device, swports[0])
        port2 = self.select_port(device, swports[1])

        # Create sub interfaces
        eth1_10 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg1 = self.cfg_subintf_on_port(device, port1, eth1_10, vlan_id=10)
        ipaddr1 = self.make_ipv4_ipaddr('10.0.10.1', 24)
        self.cfg_ip_address(device, eth1_10, vrf, ipaddr1)

        eth2_20 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg2 = self.cfg_subintf_on_port(device, port2, eth2_20, vlan_id=20)
        ipaddr2 = self.make_ipv4_ipaddr('10.0.20.1', 24)
        self.cfg_ip_address(device, eth2_20, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv4_ipaddr('10.0.10.5', 32)
        nhop1 = self.add_nhop(device, eth1_10)
        neighbor1 = self.add_neighbor_l3intf(device, eth1_10, nhop1,
            '00:10:10:10:10:15', ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        ipaddr4 = self.make_ipv4_ipaddr('10.0.20.5', 32)
        nhop2 = self.add_nhop(device, eth2_20)
        neighbor2 = self.add_neighbor_l3intf(device, eth2_20, nhop2,
            '00:20:20:20:20:25', ipaddr4)
        self.add_static_route(device, vrf, ipaddr4, nhop2)

        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            ip_id=105,
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:20:20:20:20:25',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            dl_vlan_enable=True,
            vlan_vid=20,
            ip_id=105,
            ip_ttl=63)

        try:
            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (10.0.10.5 -> 10.0.20.5 [id = 105])")
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

            # Add duplicate route
            ipaddr5 = self.make_ipv4_ipaddr('10.0.20.5', 32)
            nhop3 = self.add_nhop(device, eth2_20)

            # This should return error status. Bug filed
            status = self.add_static_route(device, vrf, ipaddr5, nhop3, store=False)

            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:10:10:10:10:15',
                ip_dst='10.0.20.5',
                ip_src='10.0.10.5',
                ip_id=105,
                dl_vlan_enable=True,
                vlan_vid=10,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:20:20:20:20:25',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.0.20.5',
                ip_src='10.0.10.5',
                dl_vlan_enable=True,
                vlan_vid=20,
                ip_id=105,
                ip_ttl=63)

            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (10.0.10.5 -> 10.0.20.5 [id = 105])")
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

        finally:
            self.cleanup()


###############################################################################
@group('l3')
@group('ipv4')
@group('subinterface')
@group('maxsizes')
@group('ent')
class L3SubIntAddDeleteAddressTest(ApiAdapter):

    def runTest(self):
        """
            Test subinterface add and then delete ipaddr

                           .1 +--------------------------+ .1
            10.0.10.5 >-------|p1.10  subinterface  p2.20|--------> 10.0.20.5
                              |                          |
                              +--------------------------+
        """
        print ""
        vrf = self.add_vrf(device, 2)

        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')
        port1 = self.select_port(device, swports[0])
        port2 = self.select_port(device, swports[1])

        # Create sub interfaces
        eth1_10 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg1 = self.cfg_subintf_on_port(device, port1, eth1_10, vlan_id=10)
        ipaddr1 = self.make_ipv4_ipaddr('10.0.10.1', 24)
        self.cfg_ip_address(device, eth1_10, vrf, ipaddr1)

        eth2_20 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg2 = self.cfg_subintf_on_port(device, port2, eth2_20, vlan_id=20)
        ipaddr2 = self.make_ipv4_ipaddr('10.0.20.1', 24)
        self.cfg_ip_address(device, eth2_20, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv4_ipaddr('10.0.10.5', 32)
        nhop1 = self.add_nhop(device, eth1_10)
        neighbor1 = self.add_neighbor_l3intf(device, eth1_10, nhop1,
            '00:10:10:10:10:15', ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        ipaddr4 = self.make_ipv4_ipaddr('10.0.20.5', 32)
        nhop2 = self.add_nhop(device, eth2_20)
        neighbor2 = self.add_neighbor_l3intf(device, eth2_20, nhop2,
            '00:20:20:20:20:25', ipaddr4)
        self.add_static_route(device, vrf, ipaddr4, nhop2)

        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            ip_id=105,
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:20:20:20:20:25',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            dl_vlan_enable=True,
            vlan_vid=20,
            ip_id=105,
            ip_ttl=63)

        try:
            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (10.0.10.5 -> 10.0.20.5 [id = 105])")
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

            # Delete ip addr
            self.no_ip_address(device, eth2_20, vrf, ipaddr2)

            # Add ip addr
            ipaddr2 = self.make_ipv4_ipaddr('10.0.20.2', 24)
            self.cfg_ip_address(device, eth2_20, vrf, ipaddr2)

            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:10:10:10:10:15',
                ip_dst='10.0.20.5',
                ip_src='10.0.10.5',
                ip_id=105,
                dl_vlan_enable=True,
                vlan_vid=10,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:20:20:20:20:25',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.0.20.5',
                ip_src='10.0.10.5',
                dl_vlan_enable=True,
                vlan_vid=20,
                ip_id=105,
                ip_ttl=63)

            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (10.0.10.5 -> 10.0.20.5 [id = 105])")
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

        finally:
            self.cleanup()


##############################################################################
@group('l3')
@group('ipv4')
@group('subinterface')
@group('maxsizes')
@group('ent')
class L3SubIntSinglePortTest(ApiAdapter):

    def runTest(self):
        """
            Test subinterface with packet in/out from a single port

                           .1 +--------------------------+
            10.0.10.5 >-------|p1.10  subinterface       |
            10.0.20.5 <-------|p1.20                     |
                              +--------------------------+
        """
        print ""
        vrf = self.add_vrf(device, 2)

        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')
        port1 = self.select_port(device, swports[0])

        # Create sub interfaces
        eth1_10 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg1 = self.cfg_subintf_on_port(device, port1, eth1_10, vlan_id=10)
        ipaddr1 = self.make_ipv4_ipaddr('10.0.10.1', 24)
        self.cfg_ip_address(device, eth1_10, vrf, ipaddr1)

        eth1_20 = self.add_logical_l3intf(device, vrf, rmac)
        port1_20 = self.cfg_subintf_on_port(device, port1, eth1_20, vlan_id=20)
        ipaddr2 = self.make_ipv4_ipaddr('10.0.20.1', 24)
        self.cfg_ip_address(device, eth1_20, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv4_ipaddr('10.0.10.5', 32)
        nhop1 = self.add_nhop(device, eth1_10)
        self.add_neighbor_l3intf(device, eth1_10, nhop1, '00:10:10:10:10:15',
            ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        ipaddr4 = self.make_ipv4_ipaddr('10.0.20.5', 32)
        nhop2 = self.add_nhop(device, eth1_20)
        self.add_neighbor_l3intf(device, eth1_20, nhop2, '00:20:20:20:20:25',
            ipaddr4)
        self.add_static_route(device, vrf, ipaddr4, nhop2)

        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            ip_id=105,
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:20:20:20:20:25',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            dl_vlan_enable=True,
            vlan_vid=20,
            ip_id=105,
            ip_ttl=63)

        try:
            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[0], " (10.0.10.5 -> 10.0.20.5 [id = 105])")
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[0]])

        finally:
            self.cleanup()


##############################################################################
@group('l3')
@group('ipv4')
@group('subinterface')
@group('maxsizes')
@group('ent')
class L3SubIntToLagAccessPortTest(ApiAdapter):

    def runTest(self):
        """
            Test subinterface with packet out to lag access port

                           .1 +--------------------------+ .1
            10.0.10.5 >-------|p1.10  subinterface    p2 |========> 10.0.20.5
                              |                       p3 | LAG access
                              +--------------------------+
        """
        print ""
        vrf = self.add_vrf(device, 2)

        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        # add vlan
        vlan20 = self.add_vlan(device, 20)

        # ingress port
        port1 = self.select_port(device, swports[0])

        # egress portgroup
        port2 = self.select_port(device, swports[1])
        port3 = self.select_port(device, swports[2])

        # sub-interface
        eth1_10 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg1 = self.cfg_subintf_on_port(device, port1, eth1_10, vlan_id=10)
        ipaddr1 = self.make_ipv4_ipaddr('10.0.10.1', 24)
        self.cfg_ip_address(device, eth1_10, vrf, ipaddr1)

        # LAG
        lag = self.add_lag(device)
        self.add_lag_member(device, lag, port2)
        self.add_lag_member(device, lag, port3)

        intf_lag = self.add_logical_l2lag(device, lag, mode='access')
        self.add_vlan_member(device, vlan20, intf_lag)

        # SVI
        intf_vl20 = self.add_logical_l3vlan(device, vrf, rmac, 20)
        ipaddr2 = self.make_ipv4_ipaddr('10.0.20.1', 24)
        self.cfg_ip_address(device, intf_vl20, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv4_ipaddr('10.0.20.5', 32)
        nhop1 = self.add_nhop(device, intf_vl20)
        self.add_neighbor_l3intf(device, intf_vl20, nhop1, '00:20:20:20:20:25',
            ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            ip_id=105,
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:20:20:20:20:25',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            ip_id=105,
            pktlen=96,
            ip_ttl=63)

        try:
            print ("Sending packet port %d" % swports[0], " -> lag port" 
                   " (10.0.10.5 -> 10.0.20.5 [id = 105])")
            send_packet(self, swports[0], str(pkt))
            verify_packet_any_port(self, exp_pkt, [swports[1], swports[2]])

        finally:
            self.cleanup()


##############################################################################
@group('l3')
@group('ipv4')
@group('subinterface')
@group('maxsizes')
@group('ent')
class L3SubIntToLagTrunkPortTest(ApiAdapter):

    def runTest(self):
        """
            Test subinterface with packet out to lag trunk port

                           .1 +--------------------------+ .1
            10.0.10.5 >-------|p1.10  subinterface    p2 |========> 10.0.20.5
                              |                       p3 | LAG trunk
                              +--------------------------+
        """
        print ""
        vrf = self.add_vrf(device, 2)

        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        # add vlan
        vlan20 = self.add_vlan(device, 20)

        # ingress port
        port1 = self.select_port(device, swports[0])

        # egress portgroup
        port2 = self.select_port(device, swports[1])
        port3 = self.select_port(device, swports[2])

        # sub-interface
        eth1_10 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg1 = self.cfg_subintf_on_port(device, port1, eth1_10, vlan_id=10)
        ipaddr1 = self.make_ipv4_ipaddr('10.0.10.1', 24)
        self.cfg_ip_address(device, eth1_10, vrf, ipaddr1)

        # LAG
        lag = self.add_lag(device)
        self.add_lag_member(device, lag, port2)
        self.add_lag_member(device, lag, port3)

        intf_lag = self.add_logical_l2lag(device, lag, mode='trunk')
        self.add_vlan_member(device, vlan20, intf_lag)

        # SVI
        intf_vl20 = self.add_logical_l3vlan(device, vrf, rmac, 20)
        ipaddr2 = self.make_ipv4_ipaddr('10.0.20.1', 24)
        self.cfg_ip_address(device, intf_vl20, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv4_ipaddr('10.0.20.5', 32)
        nhop1 = self.add_nhop(device, intf_vl20)
        self.add_neighbor_l3intf(device, intf_vl20, nhop1, '00:20:20:20:20:25',
            ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            ip_id=105,
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:20:20:20:20:25',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            ip_id=105,
            dl_vlan_enable=True,
            vlan_vid=20,
            ip_ttl=63)

        try:
            print ("Sending packet port %d" % swports[0], " -> lag port" 
                   " (10.0.10.5 -> 10.0.20.5 [id = 105])")
            send_packet(self, swports[0], str(pkt))
            verify_packet_any_port(self, exp_pkt, [swports[1], swports[2]])

        finally:
            self.cleanup()


##############################################################################
@group('l3')
@group('ipv4')
@group('subinterface')
@group('maxsizes')
@group('ent')
class L3SubIntToPortAccessTest(ApiAdapter):

    def runTest(self):
        """
            Test subinterface with packet out to access port

                           .1 +--------------------------+ .1
            10.0.10.5 >-------|p1.10  subinterface    p2 |-------> 10.0.20.5
                              |                  vlan-20 | access
                              +--------------------------+ 
        """
        print ""
        vrf = self.add_vrf(device, 2)

        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        # add vlan
        vlan20 = self.add_vlan(device, 20)

        # ingress port
        port1 = self.select_port(device, swports[0])

        # egress port
        port2 = self.select_port(device, swports[1])

        # sub-interface
        eth1_10 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg1 = self.cfg_subintf_on_port(device, port1, eth1_10, vlan_id=10)
        ipaddr1 = self.make_ipv4_ipaddr('10.0.10.1', 24)
        self.cfg_ip_address(device, eth1_10, vrf, ipaddr1)

        # interface
        eth2 = self.cfg_l2intf_on_port(device, port2, mode='access')
        self.add_vlan_member(device, vlan20, eth2)

        # SVI
        intf_vl20 = self.add_logical_l3vlan(device, vrf, rmac, 20)
        ipaddr2 = self.make_ipv4_ipaddr('10.0.20.1', 24)
        self.cfg_ip_address(device, intf_vl20, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv4_ipaddr('10.0.20.5', 32)
        nhop1 = self.add_nhop(device, intf_vl20)
        self.add_neighbor_l3intf(device, intf_vl20, nhop1, '00:20:20:20:20:25',
            ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            ip_id=105,
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:20:20:20:20:25',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            ip_id=105,
            pktlen=96,
            ip_ttl=63)

        try:
            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (10.0.10.5 -> 10.0.20.5 [id = 105])")
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

        finally:
            self.cleanup()


##############################################################################
@group('l3')
@group('ipv4')
@group('subinterface')
@group('maxsizes')
@group('ent')
class L3SubIntToPortTrunkTest(ApiAdapter):

    def runTest(self):
        """
            Test subinterface with packet out to trunk port

                           .1 +--------------------------+ .1
            10.0.10.5 >-------|p1.10  subinterface    p2 |-------> 10.0.20.5
                              |                  vlan-20 | trunk
                              +--------------------------+ 
        """
        print ""
        vrf = self.add_vrf(device, 2)

        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        # add vlan
        vlan20 = self.add_vlan(device, 20)

        # ingress port
        port1 = self.select_port(device, swports[0])

        # egress port
        port2 = self.select_port(device, swports[1])

        # sub-interface
        eth1_10 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg1 = self.cfg_subintf_on_port(device, port1, eth1_10, vlan_id=10)
        ipaddr1 = self.make_ipv4_ipaddr('10.0.10.1', 24)
        self.cfg_ip_address(device, eth1_10, vrf, ipaddr1)

        # interface
        eth2 = self.cfg_l2intf_on_port(device, port2, mode='trunk')
        self.add_vlan_member(device, vlan20, eth2)

        # SVI
        intf_vl20 = self.add_logical_l3vlan(device, vrf, rmac, 20)
        ipaddr2 = self.make_ipv4_ipaddr('10.0.20.1', 24)
        self.cfg_ip_address(device, intf_vl20, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv4_ipaddr('10.0.20.5', 32)
        nhop1 = self.add_nhop(device, intf_vl20)
        self.add_neighbor_l3intf(device, intf_vl20, nhop1, '00:20:20:20:20:25',
            ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            ip_id=105,
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:20:20:20:20:25',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            ip_id=105,
            dl_vlan_enable=True,
            vlan_vid=20,
            ip_ttl=63)

        try:
            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (10.0.10.5 -> 10.0.20.5 [id = 105])")
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

        finally:
           self.cleanup()


##############################################################################
# LAG BINDING IS MISSING (bug)
@group('l3')
@group('ipv4')
@group('subinterface')
@group('maxsizes')
@group('ent')
class L3SubIntToL3LagSubIntTest(ApiAdapter):

    def runTest(self):
        """
            Test subinterface to l3 lag sub-interface

                           .1 +--------------------------+ .1
            10.0.10.5 >-------|p1.10  subinterface  p2.20|========> 10.0.20.5
                              |                     p3.20|
                              +--------------------------+
        """
        print ""
        vrf = self.add_vrf(device, 2)

        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        # ingress port
        port1 = self.select_port(device, swports[0])

        # egress portgroup
        port2 = self.select_port(device, swports[1])
        port3 = self.select_port(device, swports[2])

        # sub-interface
        eth1_10 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg1 = self.cfg_subintf_on_port(device, port1, eth1_10, vlan_id=10)
        ipaddr1 = self.make_ipv4_ipaddr('10.0.10.1', 24)
        self.cfg_ip_address(device, eth1_10, vrf, ipaddr1)

        # LAG
        lag = self.add_lag(device)
        self.add_lag_member(device, lag, port2)
        self.add_lag_member(device, lag, port3)

        # sub-interface
        intf_lag = self.add_logical_l3intf(device, vrf, rmac)
        port_lag = self.cfg_subintf_on_lag(device, lag, intf_lag, vlan_id=20)
        ipaddr2 = self.make_ipv4_ipaddr('10.0.20.1', 24)
        self.cfg_ip_address(device, intf_lag, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv4_ipaddr('10.0.20.5', 32)
        nhop1 = self.add_nhop(device, intf_lag)
        self.add_neighbor_l3intf(device, intf_lag, nhop1, '00:20:20:20:20:25',
            ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            ip_id=105,
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:20:20:20:20:25',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            dl_vlan_enable=True,
            vlan_vid=20,
            ip_id=105,
            ip_ttl=63)

        try:
            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (10.0.10.5 -> 10.0.20.5 [id = 105])")
            send_packet(self, swports[0], str(pkt))
            verify_packet_any_port(self, exp_pkt, [swports[1], swports[2]])

        finally:
            self.cleanup()


###############################################################################
@group('l3')
@group('ipv4')
@group('subinterface')
@group('maxsizes')
@group('ent')
class L3SubIntToL3IntTest(ApiAdapter):

    def runTest(self):
        """
            Test subinterface to l3 normal port

                           .1 +--------------------------+ .1
            10.0.10.5 >-------|p1.10  subinterface    p2 |-------> 10.0.20.5
                              |                          |
                              +--------------------------+
        """
        print ""
        vrf = self.add_vrf(device, 2)

        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        # ingress port
        port1 = self.select_port(device, swports[0])

        # egress
        port2 = self.select_port(device, swports[1])

        # sub-interface
        eth1_10 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg1 = self.cfg_subintf_on_port(device, port1, eth1_10, vlan_id=10)
        ipaddr1 = self.make_ipv4_ipaddr('10.0.10.1', 24)
        self.cfg_ip_address(device, eth1_10, vrf, ipaddr1)

        # interface
        eth2 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg2 = self.cfg_l3intf_on_port(device, port2, eth2)
        ipaddr2 = self.make_ipv4_ipaddr('10.0.20.1', 24)
        self.cfg_ip_address(device, eth2, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv4_ipaddr('10.0.10.5', 32)
        nhop1 = self.add_nhop(device, eth1_10)
        self.add_neighbor_l3intf(device, eth1_10, nhop1, '00:10:10:10:10:15',
            ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        ipaddr4 = self.make_ipv4_ipaddr('10.0.20.5', 32)
        nhop2 = self.add_nhop(device, eth2)
        self.add_neighbor_l3intf(device, eth2, nhop2, '00:20:20:20:20:25',
            ipaddr4)
        self.add_static_route(device, vrf, ipaddr4, nhop2)

        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            ip_id=105,
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:20:20:20:20:25',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            pktlen=96,
            ip_id=105,
            ip_ttl=63)

        try:
            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (10.0.10.5 -> 10.0.20.5 [id = 105])")
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

        finally:
            self.cleanup()


###############################################################################
@group('l3')
@group('ipv4')
@group('subinterface')
@group('maxsizes')
@group('ent')
class L3SubIntToEcmpTest(ApiAdapter):

    def runTest(self):
        """
            Test subinterface to ecmp normal ports

                           .1 +--------------------------+ .1
            10.0.10.5 >-------|p1.10  subinterface    p2 |-------> 10.0.20.5
                              |                       p3 |-------> 10.0.21.5
                              |                       p4 |-------> 10.0.22.5
                              +--------------------------+
        """
        print ""
        vrf = self.add_vrf(device, 2)

        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        # ingress port
        port1 = self.select_port(device, swports[0])

        # egress
        port2 = self.select_port(device, swports[1])
        port3 = self.select_port(device, swports[2])
        port4 = self.select_port(device, swports[3])

        # sub-interface
        eth1_10 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg1 = self.cfg_subintf_on_port(device, port1, eth1_10, vlan_id=10)
        ipaddr1 = self.make_ipv4_ipaddr('10.0.10.1', 24)
        self.cfg_ip_address(device, eth1_10, vrf, ipaddr1)

        # interfaces
        eth2 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg2 = self.cfg_l3intf_on_port(device, port2, eth2)
        ipaddr2 = self.make_ipv4_ipaddr('10.0.20.1', 24)
        self.cfg_ip_address(device, eth2, vrf, ipaddr2)

        eth3 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg3 = self.cfg_l3intf_on_port(device, port3, eth3)
        ipaddr3 = self.make_ipv4_ipaddr('10.0.21.1', 24)
        self.cfg_ip_address(device, eth3, vrf, ipaddr3)

        eth4 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg4 = self.cfg_l3intf_on_port(device, port4, eth4)
        ipaddr4 = self.make_ipv4_ipaddr('10.0.22.1', 24)
        self.cfg_ip_address(device, eth4, vrf, ipaddr4)

        # neighbor ip
        ipaddr5 = self.make_ipv4_ipaddr('10.0.30.0', 24)

        nhop1 = self.add_nhop(device, eth2)
        self.add_neighbor_l3intf(device, eth2, nhop1, '00:11:22:33:44:55',
            ipaddr5)

        nhop2 = self.add_nhop(device, eth3)
        self.add_neighbor_l3intf(device, eth3, nhop2, '00:11:22:33:44:56',
            ipaddr5)

        nhop3 = self.add_nhop(device, eth4)
        self.add_neighbor_l3intf(device, eth4, nhop3, '00:11:22:33:44:57',
            ipaddr5)

        # ecmp
        ecmp = self.add_ecmp(device)
        self.add_ecmp_member(device, ecmp, 2, [nhop1, nhop2, nhop3])
        self.add_static_route(device, vrf, ipaddr5, ecmp)


        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ip_dst='10.0.30.5',
            ip_src='10.0.10.5',
            ip_id=105,
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_ttl=64)
        exp_pkt1 = simple_tcp_packet(
            eth_dst='00:11:22:33:44:55',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.0.30.5',
            ip_src='10.0.10.5',
            pktlen=96,
            ip_id=105,
            ip_ttl=63)
        exp_pkt2 = simple_tcp_packet(
            eth_dst='00:11:22:33:44:56',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.0.30.5',
            ip_src='10.0.10.5',
            pktlen=96,
            ip_id=105,
            ip_ttl=63)
        exp_pkt3 = simple_tcp_packet(
            eth_dst='00:11:22:33:44:57',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.0.30.5',
            ip_src='10.0.10.5',
            pktlen=96,
            ip_id=105,
            ip_ttl=63)

        try:
            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (10.0.10.5 -> 10.0.30.5 [id = 105])")
            send_packet(self, swports[0], str(pkt))
            verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2, exp_pkt3],
                                      [swports[1], swports[2], swports[3]])

        finally:
            self.cleanup()


###############################################################################
@group('l3')
@group('ipv4')
@group('subinterface')
@group('maxsizes')
@group('ent')
class L3SubIntToEcmpSubIntTest(ApiAdapter):

    def runTest(self):
        """
            Test subinterface to ecmp sub-interfaces

                           .1 +--------------------------+ .1
            10.0.10.5 >-------|p1.10  subinterface p2.20 |-------> 10.0.20.5
                              |                    p3.20 |-------> 10.0.21.5
                              |                    p4.20 |-------> 10.0.22.5
                              +--------------------------+
        """
        print ""
        vrf = self.add_vrf(device, 2)

        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        # ingress port
        port1 = self.select_port(device, swports[0])

        # egress
        port2 = self.select_port(device, swports[1])
        port3 = self.select_port(device, swports[2])
        port4 = self.select_port(device, swports[3])

        # ingress sub-interface
        eth1_10 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg1 = self.cfg_subintf_on_port(device, port1, eth1_10, vlan_id=10)
        ipaddr1 = self.make_ipv4_ipaddr('10.0.10.1', 24)
        self.cfg_ip_address(device, eth1_10, vrf, ipaddr1)

        # egress sub-interfaces
        eth2_20 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg2 = self.cfg_subintf_on_port(device, port2, eth2_20, vlan_id=20)
        ipaddr2 = self.make_ipv4_ipaddr('10.0.20.1', 24)
        self.cfg_ip_address(device, eth2_20, vrf, ipaddr2)

        eth3_20 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg3 = self.cfg_subintf_on_port(device, port3, eth3_20, vlan_id=20)
        ipaddr3 = self.make_ipv4_ipaddr('10.0.21.1', 24)
        self.cfg_ip_address(device, eth3_20, vrf, ipaddr3)

        eth4_20 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg4 = self.cfg_subintf_on_port(device, port4, eth4_20, vlan_id=20)
        ipaddr4 = self.make_ipv4_ipaddr('10.0.22.1', 24)
        self.cfg_ip_address(device, eth4_20, vrf, ipaddr4)

        # neighbor ip
        ipaddr5 = self.make_ipv4_ipaddr('10.0.30.5', 32)

        nhop1 = self.add_nhop(device, eth2_20)
        self.add_neighbor_l3intf(device, eth2_20, nhop1, '00:11:22:33:44:55',
            ipaddr5)

        nhop2 = self.add_nhop(device, eth3_20)
        self.add_neighbor_l3intf(device, eth3_20, nhop2, '00:11:22:33:44:56',
            ipaddr5)

        nhop3 = self.add_nhop(device, eth4_20)
        self.add_neighbor_l3intf(device, eth4_20, nhop3, '00:11:22:33:44:57',
            ipaddr5)

        # ecmp
        ecmp = self.add_ecmp(device)
        self.add_ecmp_member(device, ecmp, 2, [nhop1, nhop2, nhop3])
        self.add_static_route(device, vrf, ipaddr5, ecmp)

        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ip_dst='10.0.30.5',
            ip_src='10.0.10.5',
            ip_id=105,
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_ttl=64)
        exp_pkt1 = simple_tcp_packet(
            eth_dst='00:11:22:33:44:55',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.0.30.5',
            ip_src='10.0.10.5',
            dl_vlan_enable=True,
            vlan_vid=20,
            ip_id=105,
            ip_ttl=63)
        exp_pkt2 = simple_tcp_packet(
            eth_dst='00:11:22:33:44:56',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.0.30.5',
            ip_src='10.0.10.5',
            dl_vlan_enable=True,
            vlan_vid=20,
            ip_id=105,
            ip_ttl=63)
        exp_pkt3 = simple_tcp_packet(
            eth_dst='00:11:22:33:44:57',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.0.30.5',
            ip_src='10.0.10.5',
            dl_vlan_enable=True,
            vlan_vid=20,
            ip_id=105,
            ip_ttl=63)

        try:
            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (10.0.10.5 -> 10.0.30.5 [id = 105])")
            send_packet(self, swports[0], str(pkt))
            verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2, exp_pkt3],
                                      [swports[1], swports[2], swports[3]])

        finally:
            self.cleanup()


###############################################################################
@group('l3')
@group('ipv4')
@group('subinterface')
@group('maxsizes')
class L3SubIntToRpfSubIntTest(ApiAdapter):

    def runTest(self):
        """
            Test subinterface update and delete

                           .1 +-----------------------------+ .1
            10.0.10.5 >-------|p1.10  subinterface    p2.20 |-------> 10.0.20.5
                              |                             |
            Next hop          | Route            GW         | Next hop
            10.0.10.2         | 172.0.10.0/24 10.0.10.2 > p1| 10.0.20.2
            00:10:10:10:10:12 | 172.0.20.0/24 10.0.20.2 > p2| 00:20:20:20:20:22
                              | 172.0.30.0/24 10.0.10.2 > p1|
                              +-----------------------------+
        """
        print ""
        vrf = self.add_vrf(device, 2)

        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        # ingress port
        port1 = self.select_port(device, swports[0])

        # egress port
        port2 = self.select_port(device, swports[1])

        # sub-interface
        eth1_10 = self.add_logical_l3intf_urpf(device, vrf, rmac,
            v4_urpf_mode=1, v4_unicast_enabled=True)
        ethcfg1 = self.cfg_subintf_on_port(device, port1, eth1_10, vlan_id=10)
        ipaddr1 = self.make_ipv4_ipaddr('10.0.10.1', 24)
        self.cfg_ip_address(device, eth1_10, vrf, ipaddr1)

        eth2_20 = self.add_logical_l3intf_urpf(device, vrf, rmac,
            v4_urpf_mode=2, v4_unicast_enabled=True)
        ethcfg2 = self.cfg_subintf_on_port(device, port2, eth2_20, vlan_id=20)
        ipaddr2 = self.make_ipv4_ipaddr('10.0.20.1', 24)
        self.cfg_ip_address(device, eth2_20, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv4_ipaddr('10.0.10.2', 32)
        nhop1 = self.add_nhop(device, eth1_10)
        self.add_neighbor_l3intf(device, eth1_10, nhop1, '00:10:10:10:10:12',
            ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        ipaddr4 = self.make_ipv4_ipaddr('10.0.20.2', 32)
        nhop2 = self.add_nhop(device, eth2_20)
        self.add_neighbor_l3intf(device, eth2_20, nhop2, '00:20:20:20:20:22',
            ipaddr4)
        self.add_static_route(device, vrf, ipaddr4, nhop2)

        # Add a static route 172.0.10/24 --> if1
        ipaddr5 = self.make_ipv4_ipaddr('172.0.10.0', 24)
        self.add_static_route(device, vrf, ipaddr5, nhop1)

        # Add a static route 172.0.20/24 --> if2
        ipaddr6 = self.make_ipv4_ipaddr('172.0.20.0', 24)
        self.add_static_route(device, vrf, ipaddr6, nhop2)

        # Add a static route 172.0.30/24 --> if1
        ipaddr7 = self.make_ipv4_ipaddr('172.0.30.0', 24)
        self.add_static_route(device, vrf, ipaddr7, nhop1)

        try:
            # ---------------------------
            # Loose urpf (permit)
            # ---------------------------
            # In loose mode each incoming packet's source ipaddr is tested
            # against the FIB. The packet is dropped only if the source ipaddr
            # is not reachable via any interface on that router.
            #
            # Send pkt to a listed network from known src network
            #            .1 +-----------------------------+ .1
            # 10.0.10.5 >---|p1.10  subinterface    p2.20 |-------> 172.0.20.5
            #               |                             |
            # Next hop      | Route            GW         | Next hop
            # 10.0.10.2     | 172.0.10.0/24 10.0.10.2 > p1| 10.0.20.2
            # 00:10:*:10:12 | 172.0.20.0/24 10.0.20.2 > p2| 00:20:20:20:20:22
            #               | 172.0.30.0/24 10.0.10.2 > p1|
            #               +-----------------------------+
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:10:10:10:10:15',
                ip_dst='172.0.20.5',
                ip_src='10.0.10.5',
                ip_id=105,
                dl_vlan_enable=True,
                vlan_vid=10,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:20:20:20:20:22',
                eth_src='00:77:66:55:44:33',
                ip_dst='172.0.20.5',
                ip_src='10.0.10.5',
                dl_vlan_enable=True,
                vlan_vid=20,
                ip_id=105,
                ip_ttl=63)
            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (10.0.10.5 -> 172.0.20.5 [id = 105])"
                   ". Loose urpf (permit)")
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

            # ---------------------------
            # Loose urpf (drop)
            # ---------------------------
            # Send pkt to a listed network from unknown src network
            #              .1 +-----------------------------+ .1
            # 192.168.1.5 >---|p1.10  subinterface    p2.20 |------> 172.0.20.5
            #                 |                             |
            # Next hop        | Route            GW         | Next hop
            # 10.0.10.2       | 172.0.10.0/24 10.0.10.2 > p1| 10.0.20.2
            # 00:10:*:10:12   | 172.0.20.0/24 10.0.20.2 > p2| 00:20:20:20:20:22
            #                 | 172.0.30.0/24 10.0.10.2 > p1|
            #                 +-----------------------------+
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:10:10:10:10:15',
                ip_dst='172.0.20.5',
                ip_src='192.168.1.5',
                ip_id=105,
                dl_vlan_enable=True,
                vlan_vid=10,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:20:20:20:20:22',
                eth_src='00:77:66:55:44:33',
                ip_dst='172.0.20.5',
                ip_src='192.168.1.5',
                dl_vlan_enable=True,
                vlan_vid=20,
                ip_id=105,
                ip_ttl=63)
            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (192.168.1.5 -> 172.0.20.5 [id = 105])"
                   ". Loose urpf (drop)")
            send_packet(self, swports[0], str(pkt))
            verify_no_other_packets(self, timeout=1)

            # ---------------------------
            # Strict urpf (permit)
            # ---------------------------
            # In strict mode each incoming packet is tested against the FIB and
            # if the incoming interface is not the best reverse path the packet
            # check will fail. By default failed packets are discarded.
            #
            # Send pkt to a listed network from known src network
            #              .1 +-----------------------------+ .1
            # 172.0.10.5 <----|p1.10  subinterface    p2.20 |------< 172.0.20.5
            #                 |                             |
            # Next hop        | Route            GW         | Next hop
            # 10.0.10.2       | 172.0.10.0/24 10.0.10.2 > p1| 10.0.20.2
            # 00:10:*:10:12   | 172.0.20.0/24 10.0.20.2 > p2| 00:20:20:20:20:22
            #                 | 172.0.30.0/24 10.0.10.2 > p1|
            #                 +-----------------------------+
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:20:20:20:20:25',
                ip_dst='172.0.10.5',
                ip_src='172.0.20.5',
                ip_id=105,
                dl_vlan_enable=True,
                vlan_vid=20,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:10:10:10:10:12',
                eth_src='00:77:66:55:44:33',
                ip_dst='172.0.10.5',
                ip_src='172.0.20.5',
                dl_vlan_enable=True,
                vlan_vid=10,
                ip_id=105,
                ip_ttl=63)
            print ("Sending packet port %d" % swports[1], " -> port %d" 
                   % swports[0], " (172.0.20.5 -> 172.0.10.5 [id = 105])"
                   ". Strict urpf (permit)")
            send_packet(self, swports[1], str(pkt))
            verify_packets(self, exp_pkt, [swports[0]])

            # ---------------------------
            # Strict urpf (miss drop)
            # ---------------------------
            # Send pkt to a listed network from unknown src network (miss)
            #              .1 +-----------------------------+ .1
            # 172.0.10.5 <----|p1.10  subinterface    p2.20 |------< 192.168.1.5
            #                 |                             |
            # Next hop        | Route            GW         | Next hop
            # 10.0.10.2       | 172.0.10.0/24 10.0.10.2 > p1| 10.0.20.2
            # 00:10:*:10:12   | 172.0.20.0/24 10.0.20.2 > p2| 00:20:20:20:20:22
            #                 | 172.0.30.0/24 10.0.10.2 > p1|
            #                 +-----------------------------+
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:20:20:20:20:25',
                ip_dst='172.0.10.5',
                ip_src='192.168.1.5',
                ip_id=105,
                dl_vlan_enable=True,
                vlan_vid=20,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:10:10:10:10:12',
                eth_src='00:77:66:55:44:33',
                ip_dst='172.0.10.5',
                ip_src='192.168.1.5',
                dl_vlan_enable=True,
                vlan_vid=10,
                ip_id=105,
                ip_ttl=63)
            print ("Sending packet port %d" % swports[1], " -> port %d" 
                   % swports[0], " (192.168.1.5 -> 172.0.10.5 [id = 105])"
                   ". Strict urpf (miss drop)")
            send_packet(self, swports[1], str(pkt))
            verify_no_other_packets(self, timeout=1)

            # ---------------------------
            # Strict urpf (hit drop)
            # ---------------------------
            # Send pkt to a listed network from known src network (not best
            #                                                      reverse)
            #              .1 +-----------------------------+ .1
            # 172.0.10.5 <----|p1.10  subinterface    p2.20 |------< 172.0.30.5
            #                 |                             |
            # Next hop        | Route            GW         | Next hop
            # 10.0.10.2       | 172.0.10.0/24 10.0.10.2 > p1| 10.0.20.2
            # 00:10:*:10:12   | 172.0.20.0/24 10.0.20.2 > p2| 00:20:20:20:20:22
            #                 | 172.0.30.0/24 10.0.10.2 > p1|
            #                 +-----------------------------+
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:20:20:20:20:25',
                ip_dst='172.0.10.5',
                ip_src='172.0.30.5',
                ip_id=105,
                dl_vlan_enable=True,
                vlan_vid=20,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:10:10:10:10:12',
                eth_src='00:77:66:55:44:33',
                ip_dst='172.0.10.5',
                ip_src='172.0.30.5',
                dl_vlan_enable=True,
                vlan_vid=10,
                ip_id=105,
                ip_ttl=63)
            print ("Sending packet port %d" % swports[1], " -> port %d" 
                   % swports[0], " (172.0.30.5 -> 172.0.10.5 [id = 105])"
                   ". Strict urpf (hit drop)")
            send_packet(self, swports[1], str(pkt))
            verify_no_other_packets(self, timeout=1)

        finally:
            self.cleanup()


###############################################################################
@group('l3')
@group('ipv6')
@group('subinterface')
@group('maxsizes')
@group('ent')
class L3v6SubIntSinglePortTest(ApiAdapter):

    def runTest(self):
        """
            Test subinterface with packet in/out from a single port

                             .1 +--------------------------+
            3000:10::10 >-------|p1.10  subinterface       |
            3000:20::10 <-------|p1.20                     |
                                +--------------------------+
        """
        print ""
        vrf = self.add_vrf(device, 2)

        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')
        port1 = self.select_port(device, swports[0])

        # Create sub interfaces
        eth1_10 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg1 = self.cfg_subintf_on_port(device, port1, eth1_10, vlan_id=10)
        ipaddr1 = self.make_ipv6_ipaddr('3000:10:0:0:0:0:0:1', 120)
        self.cfg_ip_address(device, eth1_10, vrf, ipaddr1)

        eth1_20 = self.add_logical_l3intf(device, vrf, rmac)
        port1_20 = self.cfg_subintf_on_port(device, port1, eth1_20, vlan_id=20)
        ipaddr2 = self.make_ipv6_ipaddr('3000:20:0:0:0:0:0:1', 120)
        self.cfg_ip_address(device, eth1_20, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv6_ipaddr('3000:10:0:0:0:0:0:10', 128)
        nhop1 = self.add_nhop(device, eth1_10)
        self.add_neighbor_l3intf(device, eth1_10, nhop1, '00:10:10:10:10:15',
            ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        ipaddr4 = self.make_ipv6_ipaddr('3000:20:0:0:0:0:0:10', 128)
        nhop2 = self.add_nhop(device, eth1_20)
        self.add_neighbor_l3intf(device, eth1_20, nhop2, '00:20:20:20:20:25',
            ipaddr4)
        self.add_static_route(device, vrf, ipaddr4, nhop2)

        pkt = simple_tcpv6_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ipv6_dst='3000:20:0:0:0:0:0:10',
            ipv6_src='3000:10:0:0:0:0:0:10',
            dl_vlan_enable=True,
            vlan_vid=10,
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst='00:20:20:20:20:25',
            eth_src='00:77:66:55:44:33',
            ipv6_dst='3000:20:0:0:0:0:0:10',
            ipv6_src='3000:10:0:0:0:0:0:10',
            dl_vlan_enable=True,
            vlan_vid=20,
            ipv6_hlim=63)

        try:
            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[0], " (3000:10::10 -> 3000:20::10)")
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[0]])

        finally:
            self.cleanup()


##############################################################################
@group('l3')
@group('ipv6')
@group('subinterface')
@group('maxsizes')
@group('ent')
class L3v6SubIntToLagAccessPortTest(ApiAdapter):

    def runTest(self):
        """
            Test subinterface with packet out to lag access port

                            .1 +--------------------------+ .1
            3000:10::10 >------|p1.10  subinterface    p2 |=======> 3000:20::10
                               |                       p3 | LAG access
                               +--------------------------+
        """
        print ""
        vrf = self.add_vrf(device, 2)

        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        # add vlan
        vlan20 = self.add_vlan(device, 20)

        # ingress port
        port1 = self.select_port(device, swports[0])

        # egress portgroup
        port2 = self.select_port(device, swports[1])
        port3 = self.select_port(device, swports[2])

        # sub-interface
        eth1_10 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg1 = self.cfg_subintf_on_port(device, port1, eth1_10, vlan_id=10)
        ipaddr1 = self.make_ipv6_ipaddr('3000:10::1', 120)
        self.cfg_ip_address(device, eth1_10, vrf, ipaddr1)

        # LAG
        lag = self.add_lag(device)
        self.add_lag_member(device, lag, port2)
        self.add_lag_member(device, lag, port3)

        intf_lag = self.add_logical_l2lag(device, lag, mode='access')
        self.add_vlan_member(device, vlan20, intf_lag)

        # SVI
        intf_vl20 = self.add_logical_l3vlan(device, vrf, rmac, 20)
        ipaddr2 = self.make_ipv6_ipaddr('3000:20::1', 120)
        self.cfg_ip_address(device, intf_vl20, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv6_ipaddr('3000:20::10', 128)
        nhop1 = self.add_nhop(device, intf_vl20)
        self.add_neighbor_l3intf(device, intf_vl20, nhop1, '00:20:20:20:20:25',
            ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        pkt = simple_tcpv6_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ipv6_dst='3000:20:0:0:0:0:0:10',
            ipv6_src='3000:10:0:0:0:0:0:10',
            dl_vlan_enable=True,
            vlan_vid=10,
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst='00:20:20:20:20:25',
            eth_src='00:77:66:55:44:33',
            ipv6_dst='3000:20:0:0:0:0:0:10',
            ipv6_src='3000:10:0:0:0:0:0:10',
            pktlen=96,
            ipv6_hlim=63)

        try:
            print ("Sending packet port %d" % swports[0], " -> lag port" 
                   " (10.0.10.5 -> 10.0.20.5 [id = 105])")
            send_packet(self, swports[0], str(pkt))
            verify_packet_any_port(self, exp_pkt, [swports[1], swports[2]])

        finally:
            self.cleanup()


##############################################################################
@group('l3')
@group('ipv6')
@group('subinterface')
@group('maxsizes')
@group('ent')
class L3v6SubIntToLagTrunkPortTest(ApiAdapter):

    def runTest(self):
        """
            Test subinterface with packet out to lag trunk port

                            .1 +--------------------------+ .1
            3000:10::10 >------|p1.10  subinterface    p2 |=======> 3000:20::10
                               |                       p3 | LAG trunk
                               +--------------------------+
        """
        print ""
        vrf = self.add_vrf(device, 2)

        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        # add vlan
        vlan20 = self.add_vlan(device, 20)

        # ingress port
        port1 = self.select_port(device, swports[0])

        # egress portgroup
        port2 = self.select_port(device, swports[1])
        port3 = self.select_port(device, swports[2])

        # sub-interface
        eth1_10 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg1 = self.cfg_subintf_on_port(device, port1, eth1_10, vlan_id=10)
        ipaddr1 = self.make_ipv6_ipaddr('3000:10::1', 120)
        self.cfg_ip_address(device, eth1_10, vrf, ipaddr1)

        # LAG
        lag = self.add_lag(device)
        self.add_lag_member(device, lag, port2)
        self.add_lag_member(device, lag, port3)

        intf_lag = self.add_logical_l2lag(device, lag, mode='trunk')
        self.add_vlan_member(device, vlan20, intf_lag)

        # SVI
        intf_vl20 = self.add_logical_l3vlan(device, vrf, rmac, 20)
        ipaddr2 = self.make_ipv6_ipaddr('3000:20::1', 120)
        self.cfg_ip_address(device, intf_vl20, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv6_ipaddr('3000:20::10', 128)
        nhop1 = self.add_nhop(device, intf_vl20)
        self.add_neighbor_l3intf(device, intf_vl20, nhop1, '00:20:20:20:20:25',
            ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        pkt = simple_tcpv6_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ipv6_dst='3000:20:0:0:0:0:0:10',
            ipv6_src='3000:10:0:0:0:0:0:10',
            dl_vlan_enable=True,
            vlan_vid=10,
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst='00:20:20:20:20:25',
            eth_src='00:77:66:55:44:33',
            ipv6_dst='3000:20:0:0:0:0:0:10',
            ipv6_src='3000:10:0:0:0:0:0:10',
            dl_vlan_enable=True,
            vlan_vid=20,
            ipv6_hlim=63)

        try:
            print ("Sending packet port %d" % swports[0], " -> lag port" 
                   " (3000:10::10 -> 3000:20::10)")
            send_packet(self, swports[0], str(pkt))
            verify_packet_any_port(self, exp_pkt, [swports[1], swports[2]])

        finally:
            self.cleanup()


##############################################################################
@group('l3')
@group('ipv6')
@group('subinterface')
@group('maxsizes')
@group('ent')
class L3v6SubIntToPortAccessTest(ApiAdapter):

    def runTest(self):
        """
            Test subinterface with packet out to access port

                            .1 +--------------------------+ .1
            3000:10::10 >------|p1.10  subinterface    p2 |-------> 3000:20::10
                               |                  vlan-20 | access
                               +--------------------------+ 
        """
        print ""
        vrf = self.add_vrf(device, 2)

        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        # add vlan
        vlan20 = self.add_vlan(device, 20)

        # ingress port
        port1 = self.select_port(device, swports[0])

        # egress port
        port2 = self.select_port(device, swports[1])

        # sub-interface
        eth1_10 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg1 = self.cfg_subintf_on_port(device, port1, eth1_10, vlan_id=10)
        ipaddr1 = self.make_ipv6_ipaddr('3000:10::1', 120)
        self.cfg_ip_address(device, eth1_10, vrf, ipaddr1)

        # interface
        eth2 = self.cfg_l2intf_on_port(device, port2, mode='access')
        self.add_vlan_member(device, vlan20, eth2)

        # SVI
        intf_vl20 = self.add_logical_l3vlan(device, vrf, rmac, 20)
        ipaddr2 = self.make_ipv6_ipaddr('3000:20::1', 120)
        self.cfg_ip_address(device, intf_vl20, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv6_ipaddr('3000:20::10', 128)
        nhop1 = self.add_nhop(device, intf_vl20)
        self.add_neighbor_l3intf(device, intf_vl20, nhop1, '00:20:20:20:20:25',
            ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        pkt = simple_tcpv6_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ipv6_dst='3000:20:0:0:0:0:0:10',
            ipv6_src='3000:10:0:0:0:0:0:10',
            dl_vlan_enable=True,
            vlan_vid=10,
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst='00:20:20:20:20:25',
            eth_src='00:77:66:55:44:33',
            ipv6_dst='3000:20:0:0:0:0:0:10',
            ipv6_src='3000:10:0:0:0:0:0:10',
            pktlen=96,
            ipv6_hlim=63)

        try:
            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (3000:10::10 -> 3000:20::10)")
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

        finally:
            self.cleanup()


##############################################################################
@group('l3')
@group('ipv6')
@group('subinterface')
@group('maxsizes')
@group('ent')
class L3v6SubIntToPortTrunkTest(ApiAdapter):

    def runTest(self):
        """
            Test subinterface with packet out to trunk port

                            .1 +--------------------------+ .1
            3000:10::10 >------|p1.10  subinterface    p2 |------> 3000:20::10
                               |                  vlan-20 | trunk
                               +--------------------------+ 
        """
        print ""
        vrf = self.add_vrf(device, 2)

        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        # add vlan
        vlan20 = self.add_vlan(device, 20)

        # ingress port
        port1 = self.select_port(device, swports[0])

        # egress port
        port2 = self.select_port(device, swports[1])

        # sub-interface
        eth1_10 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg1 = self.cfg_subintf_on_port(device, port1, eth1_10, vlan_id=10)
        ipaddr1 = self.make_ipv6_ipaddr('3000:10::1', 120)
        self.cfg_ip_address(device, eth1_10, vrf, ipaddr1)

        # interface
        eth2 = self.cfg_l2intf_on_port(device, port2, mode='trunk')
        self.add_vlan_member(device, vlan20, eth2)

        # SVI
        intf_vl20 = self.add_logical_l3vlan(device, vrf, rmac, 20)
        ipaddr2 = self.make_ipv6_ipaddr('3000:20::1', 120)
        self.cfg_ip_address(device, intf_vl20, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv6_ipaddr('3000:20::10', 128)
        nhop1 = self.add_nhop(device, intf_vl20)
        self.add_neighbor_l3intf(device, intf_vl20, nhop1, '00:20:20:20:20:25',
            ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        pkt = simple_tcpv6_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ipv6_dst='3000:20:0:0:0:0:0:10',
            ipv6_src='3000:10:0:0:0:0:0:10',
            dl_vlan_enable=True,
            vlan_vid=10,
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst='00:20:20:20:20:25',
            eth_src='00:77:66:55:44:33',
            ipv6_dst='3000:20:0:0:0:0:0:10',
            ipv6_src='3000:10:0:0:0:0:0:10',
            dl_vlan_enable=True,
            vlan_vid=20,
            ipv6_hlim=63)

        try:
            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (3000:10::10 -> 3000:20::10)")
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

        finally:
           self.cleanup()


###############################################################################
@group('l3')
@group('ipv6')
@group('subinterface')
@group('maxsizes')
@group('ent')
class L3v6SubIntToL3IntTest(ApiAdapter):

    def runTest(self):
        """
            Test subinterface to l3 normal port

                            .1 +--------------------------+ .1
            3000:10::10 >------|p1.10  subinterface    p2 |------> 3000:20::10
                               |                          |
                               +--------------------------+
        """
        print ""
        vrf = self.add_vrf(device, 2)

        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        # ingress port
        port1 = self.select_port(device, swports[0])

        # egress
        port2 = self.select_port(device, swports[1])

        # sub-interface
        eth1_10 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg1 = self.cfg_subintf_on_port(device, port1, eth1_10, vlan_id=10)
        ipaddr1 = self.make_ipv6_ipaddr('3000:10::1', 120)
        self.cfg_ip_address(device, eth1_10, vrf, ipaddr1)

        # interface
        eth2 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg2 = self.cfg_l3intf_on_port(device, port2, eth2)
        ipaddr2 = self.make_ipv6_ipaddr('3000:20::1', 120)
        self.cfg_ip_address(device, eth2, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv6_ipaddr('3000:10::10', 128)
        nhop1 = self.add_nhop(device, eth1_10)
        self.add_neighbor_l3intf(device, eth1_10, nhop1, '00:10:10:10:10:15',
            ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        ipaddr4 = self.make_ipv6_ipaddr('3000:20::10', 128)
        nhop2 = self.add_nhop(device, eth2)
        self.add_neighbor_l3intf(device, eth2, nhop2, '00:20:20:20:20:25',
            ipaddr4)
        self.add_static_route(device, vrf, ipaddr4, nhop2)

        pkt = simple_tcpv6_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ipv6_dst='3000:20:0:0:0:0:0:10',
            ipv6_src='3000:10:0:0:0:0:0:10',
            dl_vlan_enable=True,
            vlan_vid=10,
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst='00:20:20:20:20:25',
            eth_src='00:77:66:55:44:33',
            ipv6_dst='3000:20:0:0:0:0:0:10',
            ipv6_src='3000:10:0:0:0:0:0:10',
            pktlen=96,
            ipv6_hlim=63)

        try:
            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (3000:10::10 -> 3000:20::10)")
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

        finally:
            self.cleanup()


###############################################################################
@group('l3')
@group('ipv6')
@group('subinterface')
@group('maxsizes')
@group('ent')
class L3v6SubIntToEcmpTest(ApiAdapter):

    def runTest(self):
        """
            Test subinterface to ecmp normal ports

                            .1 +--------------------------+ .1
            3000:10::10 >------|p1.10  subinterface    p2 |-------> 3000:20::10
                               |                       p3 |-------> 3000:21::10
                               |                       p4 |-------> 3000:22::10
                               +--------------------------+
        """
        print ""
        vrf = self.add_vrf(device, 2)

        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        # ingress port
        port1 = self.select_port(device, swports[0])

        # egress
        port2 = self.select_port(device, swports[1])
        port3 = self.select_port(device, swports[2])
        port4 = self.select_port(device, swports[3])

        # sub-interface
        eth1_10 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg1 = self.cfg_subintf_on_port(device, port1, eth1_10, vlan_id=10)
        ipaddr1 = self.make_ipv6_ipaddr('3000:10::1', 120)
        self.cfg_ip_address(device, eth1_10, vrf, ipaddr1)

        # interfaces
        eth2 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg2 = self.cfg_l3intf_on_port(device, port2, eth2)
        ipaddr2 = self.make_ipv6_ipaddr('3000:20::1', 120)
        self.cfg_ip_address(device, eth2, vrf, ipaddr2)

        eth3 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg3 = self.cfg_l3intf_on_port(device, port3, eth3)
        ipaddr3 = self.make_ipv6_ipaddr('3000:21::1', 120)
        self.cfg_ip_address(device, eth3, vrf, ipaddr3)

        eth4 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg4 = self.cfg_l3intf_on_port(device, port4, eth4)
        ipaddr4 = self.make_ipv6_ipaddr('3000:22::1', 120)
        self.cfg_ip_address(device, eth4, vrf, ipaddr4)

        # neighbor ip
        ipaddr5 = self.make_ipv6_ipaddr('3000:30::0', 120)

        nhop1 = self.add_nhop(device, eth2)
        self.add_neighbor_l3intf(device, eth2, nhop1, '00:11:22:33:44:55',
            ipaddr5)

        nhop2 = self.add_nhop(device, eth3)
        self.add_neighbor_l3intf(device, eth3, nhop2, '00:11:22:33:44:56',
            ipaddr5)

        nhop3 = self.add_nhop(device, eth4)
        self.add_neighbor_l3intf(device, eth4, nhop3, '00:11:22:33:44:57',
            ipaddr5)

        # ecmp
        ecmp = self.add_ecmp(device)
        self.add_ecmp_member(device, ecmp, 2, [nhop1, nhop2, nhop3])
        self.add_static_route(device, vrf, ipaddr5, ecmp)

        pkt = simple_tcpv6_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ipv6_dst='3000:30:0:0:0:0:0:10',
            ipv6_src='3000:10:0:0:0:0:0:10',
            dl_vlan_enable=True,
            vlan_vid=10,
            ipv6_hlim=64)
        exp_pkt1 = simple_tcpv6_packet(
            eth_dst='00:11:22:33:44:55',
            eth_src='00:77:66:55:44:33',
            ipv6_dst='3000:30:0:0:0:0:0:10',
            ipv6_src='3000:10:0:0:0:0:0:10',
            pktlen=96,
            ipv6_hlim=63)
        exp_pkt2 = simple_tcpv6_packet(
            eth_dst='00:11:22:33:44:56',
            eth_src='00:77:66:55:44:33',
            ipv6_dst='3000:30:0:0:0:0:0:10',
            ipv6_src='3000:10:0:0:0:0:0:10',
            pktlen=96,
            ipv6_hlim=63)
        exp_pkt3 = simple_tcpv6_packet(
            eth_dst='00:11:22:33:44:57',
            eth_src='00:77:66:55:44:33',
            ipv6_dst='3000:30:0:0:0:0:0:10',
            ipv6_src='3000:10:0:0:0:0:0:10',
            pktlen=96,
            ipv6_hlim=63)

        try:
            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (3000:10::10 -> 3000:30::10)")
            send_packet(self, swports[0], str(pkt))
            verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2, exp_pkt3],
                                      [swports[1], swports[2], swports[3]])

        finally:
            self.cleanup()


###############################################################################
@group('l3')
@group('ipv6')
@group('subinterface')
@group('maxsizes')
@group('ent')
class L3v6SubIntToEcmpSubIntTest(ApiAdapter):

    def runTest(self):
        """
            Test subinterface to ecmp sub-interfaces

                            .1 +--------------------------+ .1
            3000:10::10 >------|p1.10  subinterface p2.20 |-------> 3000:20::10
                               |                    p3.20 |-------> 3000:21::10
                               |                    p4.20 |-------> 3000:22::10
                               +--------------------------+
        """
        print ""
        vrf = self.add_vrf(device, 2)

        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        # ingress port
        port1 = self.select_port(device, swports[0])

        # egress
        port2 = self.select_port(device, swports[1])
        port3 = self.select_port(device, swports[2])
        port4 = self.select_port(device, swports[3])

        # ingress sub-interface
        eth1_10 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg1 = self.cfg_subintf_on_port(device, port1, eth1_10, vlan_id=10)
        ipaddr1 = self.make_ipv6_ipaddr('3000:10::1', 120)
        self.cfg_ip_address(device, eth1_10, vrf, ipaddr1)

        # egress sub-interfaces
        eth2_20 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg2 = self.cfg_subintf_on_port(device, port2, eth2_20, vlan_id=20)
        ipaddr2 = self.make_ipv6_ipaddr('3000:20::1', 120)
        self.cfg_ip_address(device, eth2_20, vrf, ipaddr2)

        eth3_20 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg3 = self.cfg_subintf_on_port(device, port3, eth3_20, vlan_id=20)
        ipaddr3 = self.make_ipv6_ipaddr('3000:21::1', 120)
        self.cfg_ip_address(device, eth3_20, vrf, ipaddr3)

        eth4_20 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg4 = self.cfg_subintf_on_port(device, port4, eth4_20, vlan_id=20)
        ipaddr4 = self.make_ipv6_ipaddr('3000:22::1', 120)
        self.cfg_ip_address(device, eth4_20, vrf, ipaddr4)

        # neighbor ip
        ipaddr5 = self.make_ipv6_ipaddr('3000:30::10', 128)

        nhop1 = self.add_nhop(device, eth2_20)
        self.add_neighbor_l3intf(device, eth2_20, nhop1, '00:11:22:33:44:55',
            ipaddr5)

        nhop2 = self.add_nhop(device, eth3_20)
        self.add_neighbor_l3intf(device, eth3_20, nhop2, '00:11:22:33:44:56',
            ipaddr5)

        nhop3 = self.add_nhop(device, eth4_20)
        self.add_neighbor_l3intf(device, eth4_20, nhop3, '00:11:22:33:44:57',
            ipaddr5)

        # ecmp
        ecmp = self.add_ecmp(device)
        self.add_ecmp_member(device, ecmp, 2, [nhop1, nhop2, nhop3])
        self.add_static_route(device, vrf, ipaddr5, ecmp)

        pkt = simple_tcpv6_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ipv6_dst='3000:30:0:0:0:0:0:10',
            ipv6_src='3000:10:0:0:0:0:0:10',
            dl_vlan_enable=True,
            vlan_vid=10,
            ipv6_hlim=64)
        exp_pkt1 = simple_tcpv6_packet(
            eth_dst='00:11:22:33:44:55',
            eth_src='00:77:66:55:44:33',
            ipv6_dst='3000:30:0:0:0:0:0:10',
            ipv6_src='3000:10:0:0:0:0:0:10',
            dl_vlan_enable=True,
            vlan_vid=20,
            ipv6_hlim=63)
        exp_pkt2 = simple_tcpv6_packet(
            eth_dst='00:11:22:33:44:56',
            eth_src='00:77:66:55:44:33',
            ipv6_dst='3000:30:0:0:0:0:0:10',
            ipv6_src='3000:10:0:0:0:0:0:10',
            dl_vlan_enable=True,
            vlan_vid=20,
            ipv6_hlim=63)
        exp_pkt3 = simple_tcpv6_packet(
            eth_dst='00:11:22:33:44:57',
            eth_src='00:77:66:55:44:33',
            ipv6_dst='3000:30:0:0:0:0:0:10',
            ipv6_src='3000:10:0:0:0:0:0:10',
            dl_vlan_enable=True,
            vlan_vid=20,
            ipv6_hlim=63)

        try:
            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (3000:10::10 -> 3000:30::10)")
            send_packet(self, swports[0], str(pkt))
            verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2, exp_pkt3],
                                      [swports[1], swports[2], swports[3]])

        finally:
            self.cleanup()


###############################################################################
# NEED to add v6_urpf_mode to api_adapter
@group('l3')
@group('ipv6')
@group('subinterface')
@group('maxsizes')
class L3v6SubIntToRpfSubIntTest(ApiAdapter):

    def runTest(self):
        """
            Test subinterface update and delete

                           .1 +-------------------------------+ .1
            3000:10::10 >-----|p1.10  subinterface      p2.20 |-----> 3000:20::10
                              |                               |
            Next hop          | Route              GW         | Next hop
            3000:10::2        | 4000:10::0/120 3000:10::2 > p1| 3000:20::2
            00:10:10:10:10:12 | 4000:20::0/120 3000:20::2 > p2| 00:20:20:20:20:22
                              | 4000:30::0/120 3000:10::2 > p1|
                              +-------------------------------+
        """
        print ""
        vrf = self.add_vrf(device, 2)

        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        # ingress port
        port1 = self.select_port(device, swports[0])

        # egress port
        port2 = self.select_port(device, swports[1])

        # sub-interface
        eth1_10 = self.add_logical_l3intf_urpf(device, vrf, rmac,
            v6_urpf_mode=1, v6_unicast_enabled=True)
        ethcfg1 = self.cfg_subintf_on_port(device, port1, eth1_10, vlan_id=10)
        ipaddr1 = self.make_ipv6_ipaddr('3000:10::1', 120)
        self.cfg_ip_address(device, eth1_10, vrf, ipaddr1)

        eth2_20 = self.add_logical_l3intf_urpf(device, vrf, rmac,
            v6_urpf_mode=2, v6_unicast_enabled=True)
        ethcfg2 = self.cfg_subintf_on_port(device, port2, eth2_20, vlan_id=20)
        ipaddr2 = self.make_ipv6_ipaddr('3000:20::1', 120)
        self.cfg_ip_address(device, eth2_20, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv6_ipaddr('3000:10::2', 128)
        nhop1 = self.add_nhop(device, eth1_10)
        self.add_neighbor_l3intf(device, eth1_10, nhop1, '00:10:10:10:10:12',
            ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        ipaddr4 = self.make_ipv6_ipaddr('3000:20::2', 128)
        nhop2 = self.add_nhop(device, eth2_20)
        self.add_neighbor_l3intf(device, eth2_20, nhop2, '00:20:20:20:20:22',
            ipaddr4)
        self.add_static_route(device, vrf, ipaddr4, nhop2)

        # Add a static route 4000:10::0/120 --> if1
        ipaddr5 = self.make_ipv6_ipaddr('4000:10::0', 120)
        self.add_static_route(device, vrf, ipaddr5, nhop1)

        # Add a static route 4000:20::0/120 --> if2
        ipaddr6 = self.make_ipv6_ipaddr('4000:20::0', 120)
        self.add_static_route(device, vrf, ipaddr6, nhop2)

        # Add a static route 4000:30::0/120 --> if1
        ipaddr7 = self.make_ipv6_ipaddr('4000:30::0', 120)
        self.add_static_route(device, vrf, ipaddr7, nhop1)

        try:
            # ---------------------------
            # Loose urpf (permit)
            # ---------------------------
            # In loose mode each incoming packet's source ipaddr is tested
            # against the FIB. The packet is dropped only if the source ipaddr
            # is not reachable via any interface on that router.
            #
            # Send pkt to a listed network from known src network
            #             .1 +-----------------------------+ .1
            # 3000:10::10 >--|p1.10  subinterface    p2.20 |--> 3000:20::10
            #                |                             |
            # Next hop       | Route            GW         | Next hop
            # 3000:10::2     | 4000:10::0/120   p1         | 3000:20::2
            # 00:10:*:10:12  | 4000:20::0/120   p2         | 00:20:20:20:20:22
            #                | 4000:30::0/120   p1         |
            #                +-----------------------------+
            pkt = simple_tcpv6_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:10:10:10:10:15',
                ipv6_dst='4000:20:0:0:0:0:0:10',
                ipv6_src='3000:10:0:0:0:0:0:10',
                dl_vlan_enable=True,
                vlan_vid=10,
                ipv6_hlim=64)
            exp_pkt = simple_tcpv6_packet(
                eth_dst='00:20:20:20:20:22',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='4000:20:0:0:0:0:0:10',
                ipv6_src='3000:10:0:0:0:0:0:10',
                dl_vlan_enable=True,
                vlan_vid=20,
                ipv6_hlim=63)

            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (3000:10::10 -> 4000:10::10)"
                   ". Loose urpf (permit)")
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

            # ---------------------------
            # Loose urpf (drop)
            # ---------------------------
            # Send pkt to a listed network from unknown src network
            #              .1 +-----------------------------+ .1
            # 2000:10::10 >---|p1.10  subinterface    p2.20 |--> 4000:20::10
            #                 |                             |
            # Next hop        | Route            GW         | Next hop
            # 3000:10::2      | 4000:10::0/120   p1         | 3000:20::2
            # 00:10:*:10:12   | 4000:20::0/120   p2         | 00:20:20:20:20:22
            #                 | 4000:30::0/120   p1         |
            #                 +-----------------------------+
            pkt = simple_tcpv6_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:10:10:10:10:15',
                ipv6_dst='4000:20:0:0:0:0:0:10',
                ipv6_src='2000:10:0:0:0:0:0:10',
                dl_vlan_enable=True,
                vlan_vid=10,
                ipv6_hlim=64)
            exp_pkt = simple_tcpv6_packet(
                eth_dst='00:20:20:20:20:22',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='4000:20:0:0:0:0:0:10',
                ipv6_src='2000:10:0:0:0:0:0:10',
                dl_vlan_enable=True,
                vlan_vid=20,
                ipv6_hlim=63)
            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (2000:10::10 -> 4000:10::10)"
                   ". Loose urpf (drop)")
            send_packet(self, swports[0], str(pkt))
            verify_no_other_packets(self, timeout=1)

            # ---------------------------
            # Strict urpf (permit)
            # ---------------------------
            # In strict mode each incoming packet is tested against the FIB and
            # if the incoming interface is not the best reverse path the packet
            # check will fail. By default failed packets are discarded.
            #
            # Send pkt to a listed network from known src network
            #              .1 +-----------------------------+ .1
            # 4000:10::10 <---|p1.10  subinterface    p2.20 |--< 4000:20::10
            #                 |                             |
            # Next hop        | Route            GW         | Next hop
            # 3000:10::2      | 4000:10::0/120   p1         | 3000:20::2
            # 00:10:*:10:12   | 4000:20::0/120   p2         | 00:20:20:20:20:22
            #                 | 4000:30::0/120   p1         |
            #                 +-----------------------------+
            pkt = simple_tcpv6_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:20:20:20:20:25',
                ipv6_dst='4000:10:0:0:0:0:0:10',
                ipv6_src='4000:20:0:0:0:0:0:10',
                dl_vlan_enable=True,
                vlan_vid=20,
                ipv6_hlim=64)
            exp_pkt = simple_tcpv6_packet(
                eth_dst='00:10:10:10:10:12',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='4000:10:0:0:0:0:0:10',
                ipv6_src='4000:20:0:0:0:0:0:10',
                dl_vlan_enable=True,
                vlan_vid=10,
                ipv6_hlim=63)
            print ("Sending packet port %d" % swports[1], " -> port %d" 
                   % swports[0], " (4000:20::10 -> 4000:10::10)"
                   ". Strict urpf (permit)")
            send_packet(self, swports[1], str(pkt))
            verify_packets(self, exp_pkt, [swports[0]])

            # ---------------------------
            # Strict urpf (miss drop)
            # ---------------------------
            # Send pkt to a listed network from unknown src network (miss)
            #              .1 +-----------------------------+ .1
            # 4000:10::10 <---|p1.10  subinterface    p2.20 |----< 4000:20::10
            #                 |                             |
            # Next hop        | Route            GW         | Next hop
            # 3000:10::2      | 4000:10::0/120   p1         | 3000:20::2
            # 00:10:*:10:12   | 4000:20::0/120   p2         | 00:20:20:20:20:22
            #                 | 4000:30::0/120   p1         |
            #                 +-----------------------------+
            pkt = simple_tcpv6_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:20:20:20:20:25',
                ipv6_dst='4000:10:0:0:0:0:0:10',
                ipv6_src='2000:10:0:0:0:0:0:10',
                dl_vlan_enable=True,
                vlan_vid=20,
                ipv6_hlim=64)
            exp_pkt = simple_tcpv6_packet(
                eth_dst='00:10:10:10:10:12',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='4000:10:0:0:0:0:0:10',
                ipv6_src='2000:10:0:0:0:0:0:10',
                dl_vlan_enable=True,
                vlan_vid=10,
                ipv6_hlim=63)
            print ("Sending packet port %d" % swports[1], " -> port %d" 
                   % swports[0], " (2000:10::10 -> 4000:10::10)"
                   ". Strict urpf (miss drop)")
            send_packet(self, swports[1], str(pkt))
            verify_no_other_packets(self, timeout=1)

            # ---------------------------
            # Strict urpf (hit drop)
            # ---------------------------
            # Send pkt to a listed network from known src network (not best
            #                                                      reverse)
            #              .1 +-----------------------------+ .1
            # 4000:10::10 <---|p1.10  subinterface    p2.20 |-----< 4000:30::10
            #                 |                             |
            # Next hop        | Route            GW         | Next hop
            # 3000:10::2      | 172.0.10.0/24 10.0.10.2 > p1| 3000:20::2
            # 00:10:*:10:12   | 172.0.20.0/24 10.0.20.2 > p2| 00:20:20:20:20:22
            #                 | 172.0.30.0/24 10.0.10.2 > p1|
            #                 +-----------------------------+
            pkt = simple_tcpv6_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:20:20:20:20:25',
                ipv6_dst='4000:10:0:0:0:0:0:10',
                ipv6_src='4000:30:0:0:0:0:0:10',
                dl_vlan_enable=True,
                vlan_vid=20,
                ipv6_hlim=64)
            exp_pkt = simple_tcpv6_packet(
                eth_dst='00:10:10:10:10:12',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='4000:10:0:0:0:0:0:10',
                ipv6_src='4000:30:0:0:0:0:0:10',
                dl_vlan_enable=True,
                vlan_vid=10,
                ipv6_hlim=63)
            print ("Sending packet port %d" % swports[1], " -> port %d" 
                   % swports[0], " (4000:30::10 -> 4000:10::10)"
                   ". Strict urpf (hit drop)")
            send_packet(self, swports[1], str(pkt))
            verify_no_other_packets(self, timeout=1)

        finally:
            self.cleanup()


###############################################################################
