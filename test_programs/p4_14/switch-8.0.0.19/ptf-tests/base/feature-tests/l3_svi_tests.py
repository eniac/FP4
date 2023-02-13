###############################################################################
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
##############################################################################
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


##############################################################################
@group('l3')
@group('ipv4')
@group('svi')
@group('maxsizes')
@group('ent')
class L3SVIDeleteReAddAddressTest(ApiAdapter):

    def runTest(self):
        """
            Test SVI deletion and re-creation

                           .1 +---------------------+ .1
            10.0.10.5 >-------|1         SVI       2|--------> 10.0.20.5
                      vlan-10 | vl10: 10.0.10.1/24  | vlan-20
                              | vl20: 10.0.20.1/24  |
                              |                     |
                              +---------------------+
        """
        print ""
        print "Sending L3 packet from port %d -> port %d" % (swports[0],
                                                             swports[1])

        vrf = self.add_vrf(device, 2)

        # Create vlan 10 & 20
        vlan10 = self.add_vlan(device, 10)
        vlan20 = self.add_vlan(device, 20)

        # Add router MAC
        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        port1 = self.select_port(device, swports[0])
        port2 = self.select_port(device, swports[1])

        # Make switchports a vlan member port
        eth1 = self.cfg_l2intf_on_port(device, port1, mode='access')
        self.add_vlan_member(device, vlan10, eth1)

        eth2 = self.cfg_l2intf_on_port(device, port2, mode='access')
        self.add_vlan_member(device, vlan20, eth2)

        # Create mac address table entries
        self.add_mac_address_table_entry(device, vlan10, '00:10:10:10:10:15',
            2, eth1)
        self.add_mac_address_table_entry(device, vlan20, '00:20:20:20:20:25',
            2, eth2)

        # Add SVI IP addresses to vlan 10 & 20
        intf_vl10 = self.add_logical_l3vlan(device, vrf, rmac, 10)
        ipaddr1 = self.make_ipv4_ipaddr('10.0.10.1', 24)
        self.cfg_ip_address(device, intf_vl10, vrf, ipaddr1)

        intf_vl20 = self.add_logical_l3vlan(device, vrf, rmac, 20)
        ipaddr2 = self.make_ipv4_ipaddr('10.0.20.1', 24)
        self.cfg_ip_address(device, intf_vl20, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv4_ipaddr('10.0.20.5', 32)
        nhop1 = self.add_nhop(device, intf_vl20)
        neighbor1 = self.add_neighbor_l3intf(device, intf_vl20, nhop1,
            '00:20:20:20:20:25', ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            ip_id=105,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:20:20:20:20:25',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            ip_id=105,
            ip_ttl=63)

        try:
            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (10.0.10.5 -> 10.0.20.5 [id = 105])")
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

            print "Delete IP address"
            self.no_neighbor(device, neighbor1)
            self.no_static_route(device, vrf, ipaddr3, nhop1)
            self.no_nhop(device, nhop1)

            print "Re-add IP address"
            ipaddr3 = self.make_ipv4_ipaddr('10.0.20.5', 32)
            nhop1 = self.add_nhop(device, intf_vl20)
            neighbor1 = self.add_neighbor_l3intf(device, intf_vl20, nhop1,
                '00:20:20:20:20:25', ipaddr3)
            self.add_static_route(device, vrf, ipaddr3, nhop1)

            print ("Re-sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (10.0.10.5 -> 10.0.20.5 [id = 105])")
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

        finally:
            self.cleanup()


##############################################################################
# DUPLICATE ENTRY DIDN'T RETURN ERROR
@group('l3')
@group('ipv4')
@group('svi')
@group('maxsizes')
@group('ent')
class L3SVIAddDuplicateAddressTest(ApiAdapter):

    def runTest(self):
        """
            Test SVI with duplicate address

                              +--------------------------+
            10.0.10.5 >-------|1        SVI             2|--------> 10.0.20.5
                      vlan-10 | vl10: 10.0.10.1/24       | vlan-20
                              | vl20: 10.0.20.1/24       |
                              | vl20: 10.0.20.1/24 (dup) |
                              +--------------------------+
        """
        print ""
        print "Sending L3 packet from port %d -> port %d" % (swports[0],
                                                             swports[1])

        vrf = self.add_vrf(device, 2)

        # Create vlan 10 & 20
        vlan10 = self.add_vlan(device, 10)
        vlan20 = self.add_vlan(device, 20)

        # Add router MAC
        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        port1 = self.select_port(device, swports[0])
        port2 = self.select_port(device, swports[1])

        # Make switchports a vlan member port
        eth1 = self.cfg_l2intf_on_port(device, port1, mode='access')
        self.add_vlan_member(device, vlan10, eth1)

        eth2 = self.cfg_l2intf_on_port(device, port2, mode='access')
        self.add_vlan_member(device, vlan20, eth2)

        # Create mac address table entries
        self.add_mac_address_table_entry(device, vlan10, '00:10:10:10:10:15',
            2, eth1)
        self.add_mac_address_table_entry(device, vlan20, '00:20:20:20:20:25',
            2, eth2)

        # Add SVI IP addresses to vlan 10 & 20
        intf_vl10 = self.add_logical_l3vlan(device, vrf, rmac, 10)
        ipaddr1 = self.make_ipv4_ipaddr('10.0.10.1', 24)
        self.cfg_ip_address(device, intf_vl10, vrf, ipaddr1)

        intf_vl20 = self.add_logical_l3vlan(device, vrf, rmac, 20)
        ipaddr2 = self.make_ipv4_ipaddr('10.0.20.1', 24)
        self.cfg_ip_address(device, intf_vl20, vrf, ipaddr2)

        # Duplicate SVI address. This will fail to add
        intf_vl20_dup = self.add_logical_l3vlan(device, vrf, rmac, 20,
                                                store=False)
        ipaddr2_dup = self.make_ipv4_ipaddr('10.0.20.1', 24)
        self.cfg_ip_address(device, intf_vl20_dup, vrf, ipaddr2_dup,
                           store=False)

        # Create ip addr and use it as host
        ipaddr4 = self.make_ipv4_ipaddr('10.0.20.5', 32)
        nhop1 = self.add_nhop(device, intf_vl20)
        neighbor1 = self.add_neighbor_l3intf(device, intf_vl20, nhop1,
            '00:20:20:20:20:25', ipaddr4)
        # currently status doesn't return correct error code
        status = self.add_static_route(device, vrf, ipaddr4, nhop1)


        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            ip_id=105,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:20:20:20:20:25',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            ip_id=105,
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
@group('svi')
@group('maxsizes')
@group('ent')
class L3SVIAddMultipleAddressesTest(ApiAdapter):

    def runTest(self):
        """
            Test SVI with multiple IP addresses

                              +---------------------+
            10.0.10.5 >-------|1        SVI        2|--------> 10.0.20.5
                      vlan-10 | vl10: 10.0.10.1/24  | vlan-20
                              | vl20: 10.0.20.1/24  |
                              | vl20: 10.0.21.1/24  |
                              | vl20: 10.0.22.1/24  |
                              +---------------------+
        """
        print ""
        print "Sending L3 packet from port %d -> port %d" % (swports[0],
                                                             swports[1])

        vrf = self.add_vrf(device, 2)

        # Create vlan 10 & 20
        vlan10 = self.add_vlan(device, 10)
        vlan20 = self.add_vlan(device, 20)

        # Add router MAC
        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        port1 = self.select_port(device, swports[0])
        port2 = self.select_port(device, swports[1])

        # Make switchports a vlan member port
        eth1 = self.cfg_l2intf_on_port(device, port1, mode='access')
        self.add_vlan_member(device, vlan10, eth1)

        eth2 = self.cfg_l2intf_on_port(device, port2, mode='access')
        self.add_vlan_member(device, vlan20, eth2)

        # Create mac address table entries
        self.add_mac_address_table_entry(device, vlan10, '00:10:10:10:10:15',
            2, eth1)
        self.add_mac_address_table_entry(device, vlan20, '00:20:20:20:20:25',
            2, eth2)

        # Add SVI IP addresses to vlan 10 & 20
        intf_vl10 = self.add_logical_l3vlan(device, vrf, rmac, 10)
        ipaddr1 = self.make_ipv4_ipaddr('10.0.10.1', 24)
        self.cfg_ip_address(device, intf_vl10, vrf, ipaddr1)

        intf_vl20 = self.add_logical_l3vlan(device, vrf, rmac, 20)
        ipaddr2 = self.make_ipv4_ipaddr('10.0.20.1', 24)
        self.cfg_ip_address(device, intf_vl20, vrf, ipaddr2)

        # Add multiple SVI IP addresses (SVI won't allow)
        intf_vl20_1 = self.add_logical_l3vlan(device, vrf, rmac, 20,
                                              store=False)
        ipaddr2_1 = self.make_ipv4_ipaddr('10.0.21.1', 24)
        status = self.cfg_ip_address(device, intf_vl20_1, vrf, ipaddr2_1,
                                    store=False)

        intf_vl20_2 = self.add_logical_l3vlan(device, vrf, rmac, 20,
                                              store=False)
        ipaddr2_2 = self.make_ipv4_ipaddr('10.0.22.1', 24)
        status = self.cfg_ip_address(device, intf_vl20_2, vrf, ipaddr2_2,
                                    store=False)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv4_ipaddr('10.0.20.5', 32)
        nhop1 = self.add_nhop(device, intf_vl20)
        neighbor1 = self.add_neighbor_l3intf(device, intf_vl20, nhop1,
            '00:20:20:20:20:25', ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            ip_id=105,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:20:20:20:20:25',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            ip_id=105,
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
@group('svi')
@group('maxsizes')
@group('ent')
class L3SVIDeleteReAddTest(ApiAdapter):

    def runTest(self):
        """
            Test SVI delete and re-add

                           .1 +---------------------+ .1
            10.0.10.5 >-------|1        SVI        2|--------> 10.0.20.5
                      vlan-10 | vl10: 10.0.10.1/24  | vlan-20
                              |                     |
                              +---------------------+
        """
        print ""
        print "Sending L3 packet from port %d -> port %d" % (swports[0],
                                                             swports[1])

        vrf = self.add_vrf(device, 2)

        # Create vlan 10 & 20
        vlan10 = self.add_vlan(device, 10)
        vlan20 = self.add_vlan(device, 20)

        # Add router MAC
        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        port1 = self.select_port(device, swports[0])
        port2 = self.select_port(device, swports[1])

        # Make switchports a vlan member port
        eth1 = self.cfg_l2intf_on_port(device, port1, mode='access')
        self.add_vlan_member(device, vlan10, eth1)

        eth2 = self.cfg_l2intf_on_port(device, port2, mode='access')
        self.add_vlan_member(device, vlan20, eth2)

        # Create mac address table entries
        self.add_mac_address_table_entry(device, vlan10, '00:10:10:10:10:15',
            2, eth1)
        self.add_mac_address_table_entry(device, vlan20, '00:20:20:20:20:25',
            2, eth2)

        # Add SVI IP addresses to vlan 10 & 20
        intf_vl10 = self.add_logical_l3vlan(device, vrf, rmac, 10)
        ipaddr1 = self.make_ipv4_ipaddr('10.0.10.1', 24)
        self.cfg_ip_address(device, intf_vl10, vrf, ipaddr1)

        intf_vl20 = self.add_logical_l3vlan(device, vrf, rmac, 20)
        ipaddr2 = self.make_ipv4_ipaddr('10.0.20.1', 24)
        self.cfg_ip_address(device, intf_vl20, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv4_ipaddr('10.0.20.5', 32)
        nhop1 = self.add_nhop(device, intf_vl20)
        neighbor1 = self.add_neighbor_l3intf(device, intf_vl20, nhop1,
            '00:20:20:20:20:25', ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            ip_id=105,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:20:20:20:20:25',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.0.20.5',
            ip_src='10.0.10.5',
            ip_id=105,
            ip_ttl=63)

        try:
            print ("Sending packet port %d" % swports[0], " -> port %d" 
                   % swports[1], " (10.0.10.5 -> 10.0.20.5 [id = 105])")
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

            print "Delete SVI interface"
            # remove neighbor before remove SVI 
            self.no_neighbor(device, neighbor1)
            self.no_static_route(device, vrf, ipaddr3, nhop1)
            self.no_nhop(device, nhop1)
            # remove SVI
            self.no_ip_address(device, intf_vl20, vrf, ipaddr2)
            self.no_logical_l3vlan(device, intf_vl20)

            print "Re-add SVI interface"
            intf_vl20 = self.add_logical_l3vlan(device, vrf, rmac, 20)
            ipaddr2 = self.make_ipv4_ipaddr('10.0.20.1', 24)
            self.cfg_ip_address(device, intf_vl20, vrf, ipaddr2)

            # re-add neighbor
            ipaddr3 = self.make_ipv4_ipaddr('10.0.20.5', 32)
            nhop1 = self.add_nhop(device, intf_vl20)
            neighbor1 = self.add_neighbor_l3intf(device, intf_vl20, nhop1,
                '00:20:20:20:20:25', ipaddr3)
            self.add_static_route(device, vrf, ipaddr3, nhop1)


            print ("Re-sending packet port %d" % swports[0], " -> port %d"
                   % swports[1], " (10.0.10.5 -> 10.0.20.5 [id = 105])")
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

        finally:
            self.cleanup()


##############################################################################
@group('l3')
@group('ipv4')
@group('svi')
@group('maxsizes')
@group('ent')
class L3SVIToSinglePortTest(ApiAdapter):

    def runTest(self):
        """
            Test svi with packet in/out from a single port

                              +--------------------------+
            10.0.10.5 >-------|    vl10: 10.0.10.1/24    |
                     trunk p1 |                          |
            10.0.20.5 <-------|    vl20: 10.0.20.1/24    |
                              +--------------------------+
        """
        print ""
        print ("Sending packet port %d" % swports[0], " -> port %d" 
                % swports[0], " (10.0.10.5 -> 10.0.20.5 [id = 105])")

        vrf = self.add_vrf(device, 2)

        # Create vlan 10 & 20
        vlan10 = self.add_vlan(device, 10)
        vlan20 = self.add_vlan(device, 20)

        # Add router MAC
        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        port1 = self.select_port(device, swports[0])

        # Make this single switchport a 'trunk' port
        eth1 = self.cfg_l2intf_on_port(device, port1, mode='trunk')
        self.add_vlan_member(device, vlan10, eth1)
        self.add_vlan_member(device, vlan20, eth1)

        # Add SVI IP addresses to vlan 10 & 20
        intf_vl10 = self.add_logical_l3vlan(device, vrf, rmac, 10)
        ipaddr1 = self.make_ipv4_ipaddr('10.0.10.1', 24)
        self.cfg_ip_address(device, intf_vl10, vrf, ipaddr1)

        intf_vl20 = self.add_logical_l3vlan(device, vrf, rmac, 20)
        ipaddr2 = self.make_ipv4_ipaddr('10.0.20.1', 24)
        self.cfg_ip_address(device, intf_vl20, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv4_ipaddr('10.0.20.5', 32)
        nhop1 = self.add_nhop(device, intf_vl20)
        neighbor1 = self.add_neighbor_l3intf(device, intf_vl20, nhop1,
            '00:20:20:20:20:25', ipaddr3)
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
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[0]])

        finally:
            self.cleanup()


##############################################################################
@group('l3')
@group('ipv4')
@group('svi')
@group('maxsizes')
@group('ent')
class L3SVIToTrunkPortTest(ApiAdapter):

    def runTest(self):
        """
            Test svi with packet out to trunk port

                              +--------------------------+
            10.0.10.5 >-------|    vl10: 10.0.10.1/24    |------> 10.0.20.5
                     trunk p1 |                          | trunk p2
                              |    vl10: 10.0.20.1/24    |
                              +--------------------------+
        """
        print ""
        print ("Sending packet port %d" % swports[0], " -> port %d" 
                % swports[1], " (10.0.10.5 -> 10.0.20.5 [id = 105])")

        vrf = self.add_vrf(device, 2)

        # Create vlan 10 & 20
        vlan10 = self.add_vlan(device, 10)
        vlan20 = self.add_vlan(device, 20)

        # Add router MAC
        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        port1 = self.select_port(device, swports[0])
        port2 = self.select_port(device, swports[1])

        # Make switchport a 'trunk' port
        eth1 = self.cfg_l2intf_on_port(device, port1, mode='trunk')
        self.add_vlan_member(device, vlan10, eth1)

        eth2 = self.cfg_l2intf_on_port(device, port2, mode='trunk')
        self.add_vlan_member(device, vlan20, eth2)

        # Add SVI IP addresses to vlan 10 & 20
        intf_vl10 = self.add_logical_l3vlan(device, vrf, rmac, 10)
        ipaddr1 = self.make_ipv4_ipaddr('10.0.10.1', 24)
        self.cfg_ip_address(device, intf_vl10, vrf, ipaddr1)

        intf_vl20 = self.add_logical_l3vlan(device, vrf, rmac, 20)
        ipaddr2 = self.make_ipv4_ipaddr('10.0.20.1', 24)
        self.cfg_ip_address(device, intf_vl20, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv4_ipaddr('10.0.20.5', 32)
        nhop1 = self.add_nhop(device, intf_vl20)
        neighbor1 = self.add_neighbor_l3intf(device, intf_vl20, nhop1,
            '00:20:20:20:20:25', ipaddr3)
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
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

        finally:
            self.cleanup()


##############################################################################
@group('l3')
@group('ipv4')
@group('svi')
@group('maxsizes')
@group('ent')
class L3SVIToSubIntTest(ApiAdapter):

    def runTest(self):
        """
            Test svi with packet out to sub-interface port

                              +--------------------------+.1
            10.0.10.5 >-------|p1  vl10: 10.0.10.1/24  p2|------> 10.0.20.5
                       trunk  |                          |
                              +--------------------------+
        """
        print ""
        print ("Sending packet port %d" % swports[0], " -> port %d" 
                % swports[1], " (10.0.10.5 -> 10.0.20.5 [id = 105])")

        vrf = self.add_vrf(device, 2)

        # Create vlan 10
        vlan10 = self.add_vlan(device, 10)

        # Add router MAC
        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        port1 = self.select_port(device, swports[0])
        port2 = self.select_port(device, swports[1])

        # Make 1 port a 'trunk' port
        eth1 = self.cfg_l2intf_on_port(device, port1, mode='trunk')
        self.add_vlan_member(device, vlan10, eth1)

        # Add SVI IP address to vlan 10
        intf_vl10 = self.add_logical_l3vlan(device, vrf, rmac, 10)
        ipaddr1 = self.make_ipv4_ipaddr('10.0.10.1', 24)
        self.cfg_ip_address(device, intf_vl10, vrf, ipaddr1)

        # Make other port a sub-interface port
        eth2_20 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg2 = self.cfg_subintf_on_port(device, port2, eth2_20, vlan_id=20)
        ipaddr2 = self.make_ipv4_ipaddr('10.0.20.1', 24)
        self.cfg_ip_address(device, eth2_20, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv4_ipaddr('10.0.20.5', 32)
        nhop1 = self.add_nhop(device, eth2_20)
        self.add_neighbor_l3intf(device, eth2_20, nhop1, '00:20:20:20:20:25',
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
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

        finally:
            self.cleanup()


##############################################################################
@group('l3')
@group('ipv4')
@group('svi')
@group('maxsizes')
@group('ent')
class L3SVIToEcmpSubIntTest(ApiAdapter):

    def runTest(self):
        """
            Test svi with packet out to Ecmp port

                              +--------------------------+.1
            10.0.10.5 >-------|p1  vl10: 10.0.10.1/24  p2|------> 10.0.20.5
                       trunk  |                        p3|------> 10.0.21.5
                              +--------------------------+
        """
        print ""
        print ("Sending packet port %d" % swports[0], " -> port %d" 
                % swports[1], " (10.0.10.5 -> 10.0.30.5 [id = 105])")

        vrf = self.add_vrf(device, 2)

        # Create vlan 10 & 20
        vlan10 = self.add_vlan(device, 10)
        vlan20 = self.add_vlan(device, 20)

        # Add router MAC
        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        # ingress
        port1 = self.select_port(device, swports[0])

        # egress
        port2 = self.select_port(device, swports[1])
        port3 = self.select_port(device, swports[2])

        # make ingress port a 'trunk' port
        eth1 = self.cfg_l2intf_on_port(device, port1, mode='trunk')
        self.add_vlan_member(device, vlan10, eth1)

        # Add SVI IP addresses to vlan 10 & 20
        intf_vl10 = self.add_logical_l3vlan(device, vrf, rmac, 10)
        ipaddr1 = self.make_ipv4_ipaddr('10.0.10.1', 24)
        self.cfg_ip_address(device, intf_vl10, vrf, ipaddr1)


        # make egress ports 'sub-interface' ports
        eth2_20 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg2 = self.cfg_subintf_on_port(device, port2, eth2_20, vlan_id=20)
        ipaddr2 = self.make_ipv4_ipaddr('10.0.20.1', 24)
        self.cfg_ip_address(device, eth2_20, vrf, ipaddr2)

        eth3_20 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg3 = self.cfg_subintf_on_port(device, port3, eth3_20, vlan_id=20)
        ipaddr3 = self.make_ipv4_ipaddr('10.0.21.1', 24)
        self.cfg_ip_address(device, eth3_20, vrf, ipaddr3)

        # neighbor ip
        ipaddr4 = self.make_ipv4_ipaddr('10.0.30.5', 32)

        nhop1 = self.add_nhop(device, eth2_20)
        self.add_neighbor_l3intf(device, eth2_20, nhop1, '00:11:22:33:44:55',
            ipaddr4)

        nhop2 = self.add_nhop(device, eth3_20)
        self.add_neighbor_l3intf(device, eth3_20, nhop2, '00:11:22:33:44:56',
            ipaddr4)

        # ecmp
        ecmp = self.add_ecmp(device)
        self.add_ecmp_member(device, ecmp, 2, [nhop1, nhop2])
        self.add_static_route(device, vrf, ipaddr4, ecmp)

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

        try:
            send_packet(self, swports[0], str(pkt))
            verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                       [swports[1], swports[2]])

        finally:
            self.cleanup()


##############################################################################
@group('l3')
@group('ipv6')
@group('svi')
@group('maxsizes')
@group('ent')
class L3v6SVIToSinglePortTest(ApiAdapter):

    def runTest(self):
        """
            Test svi with packet in/out from a single port

                                +--------------------------+
            3000:10::10 >-------|   vl10: 3000:10::1/120   |
                       trunk p1 |                          |
            3000:20::20 <-------|   vl20: 3000:20::1/120   |
                                +--------------------------+
        """
        print ""
        print ("Sending packet port %d" % swports[0], " -> port %d" 
                % swports[0], " (3000:10::10 -> 3000:20::20)")

        vrf = self.add_vrf(device, 2)

        # Create vlan 10 & 20
        vlan10 = self.add_vlan(device, 10)
        vlan20 = self.add_vlan(device, 20)

        # Add router MAC
        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        port1 = self.select_port(device, swports[0])

        # Make this single switchport a 'trunk' port
        eth1 = self.cfg_l2intf_on_port(device, port1, mode='trunk')
        self.add_vlan_member(device, vlan10, eth1)
        self.add_vlan_member(device, vlan20, eth1)

        # Add SVI IP addresses to vlan 10 & 20
        intf_vl10 = self.add_logical_l3vlan(device, vrf, rmac, 10)
        ipaddr1 = self.make_ipv6_ipaddr('3000:10:0:0:0:0:0:1', 120)
        self.cfg_ip_address(device, intf_vl10, vrf, ipaddr1)

        intf_vl20 = self.add_logical_l3vlan(device, vrf, rmac, 20)
        ipaddr2 = self.make_ipv6_ipaddr('3000:20:0:0:0:0:0:1', 120)
        self.cfg_ip_address(device, intf_vl20, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv6_ipaddr('3000:20::20', 128)
        nhop1 = self.add_nhop(device, intf_vl20)
        neighbor1 = self.add_neighbor_l3intf(device, intf_vl20, nhop1,
            '00:20:20:20:20:25', ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        pkt = simple_tcpv6_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ipv6_dst='3000:20:0:0:0:0:0:20',
            ipv6_src='3000:10:0:0:0:0:0:10',
            dl_vlan_enable=True,
            vlan_vid=10,
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst='00:20:20:20:20:25',
            eth_src='00:77:66:55:44:33',
            ipv6_dst='3000:20:0:0:0:0:0:20',
            ipv6_src='3000:10:0:0:0:0:0:10',
            dl_vlan_enable=True,
            vlan_vid=20,
            ipv6_hlim=63)

        try:
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[0]])

        finally:
            self.cleanup()


##############################################################################
@group('l3')
@group('ipv6')
@group('svi')
@group('maxsizes')
@group('ent')
class L3v6SVIToTrunkPortTest(ApiAdapter):

    def runTest(self):
        """
            Test svi with packet out to trunk port

                                +--------------------------+
            3000:10::10 >-------|   vl10: 3000:10::1/120   |------> 3000:20::20
                       trunk p1 |                          | trunk p2
                                |   vl20: 3000:20::1/120   |
                                +--------------------------+
        """
        print ""
        print ("Sending packet port %d" % swports[0], " -> port %d" 
                % swports[1], " (3000:10::10 -> 3000:20::20)")

        vrf = self.add_vrf(device, 2)

        # Create vlan 10 & 20
        vlan10 = self.add_vlan(device, 10)
        vlan20 = self.add_vlan(device, 20)

        # Add router MAC
        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        port1 = self.select_port(device, swports[0])
        port2 = self.select_port(device, swports[1])

        # Make switchport a 'trunk' port
        eth1 = self.cfg_l2intf_on_port(device, port1, mode='trunk')
        self.add_vlan_member(device, vlan10, eth1)

        eth2 = self.cfg_l2intf_on_port(device, port2, mode='trunk')
        self.add_vlan_member(device, vlan20, eth2)

        # Add SVI IP addresses to vlan 10 & 20
        intf_vl10 = self.add_logical_l3vlan(device, vrf, rmac, 10)
        ipaddr1 = self.make_ipv6_ipaddr('3000:10:0:0:0:0:0:1', 120)
        self.cfg_ip_address(device, intf_vl10, vrf, ipaddr1)

        intf_vl20 = self.add_logical_l3vlan(device, vrf, rmac, 20)
        ipaddr2 = self.make_ipv6_ipaddr('3000:20:0:0:0:0:0:1', 120)
        self.cfg_ip_address(device, intf_vl20, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv6_ipaddr('3000:20::20', 128)
        nhop1 = self.add_nhop(device, intf_vl20)
        neighbor1 = self.add_neighbor_l3intf(device, intf_vl20, nhop1,
            '00:20:20:20:20:25', ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        pkt = simple_tcpv6_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ipv6_dst='3000:20:0:0:0:0:0:20',
            ipv6_src='3000:10:0:0:0:0:0:10',
            dl_vlan_enable=True,
            vlan_vid=10,
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst='00:20:20:20:20:25',
            eth_src='00:77:66:55:44:33',
            ipv6_dst='3000:20:0:0:0:0:0:20',
            ipv6_src='3000:10:0:0:0:0:0:10',
            dl_vlan_enable=True,
            vlan_vid=20,
            ipv6_hlim=63)

        try:
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

        finally:
            self.cleanup()


##############################################################################
@group('l3')
@group('ipv6')
@group('svi')
@group('maxsizes')
@group('ent')
class L3v6SVIToSubIntTest(ApiAdapter):

    def runTest(self):
        """
            Test svi with packet out to sub-interface port

                               +---------------------------+.1
            3000:10::10 >------|p1  vl10: 3000:10::1/120 p2|------> 3000:20::20
                        trunk  |                           |
                               +---------------------------+
        """
        print ""
        print ("Sending packet port %d" % swports[0], " -> port %d" 
                % swports[1], " (3000:10::10 -> 3000:20::20)")

        vrf = self.add_vrf(device, 2)

        # Create vlan 10
        vlan10 = self.add_vlan(device, 10)

        # Add router MAC
        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        port1 = self.select_port(device, swports[0])
        port2 = self.select_port(device, swports[1])

        # Make 1 port a 'trunk' port
        eth1 = self.cfg_l2intf_on_port(device, port1, mode='trunk')
        self.add_vlan_member(device, vlan10, eth1)

        # Add SVI IP address to vlan 10
        intf_vl10 = self.add_logical_l3vlan(device, vrf, rmac, 10)
        ipaddr1 = self.make_ipv6_ipaddr('3000:10::1', 120)
        self.cfg_ip_address(device, intf_vl10, vrf, ipaddr1)

        # Make other port a sub-interface port
        eth2_20 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg2 = self.cfg_subintf_on_port(device, port2, eth2_20, vlan_id=20)
        ipaddr2 = self.make_ipv6_ipaddr('3000:20::1', 120)
        self.cfg_ip_address(device, eth2_20, vrf, ipaddr2)

        # Create ip addr and use it as host
        ipaddr3 = self.make_ipv6_ipaddr('3000:20::20', 128)
        nhop1 = self.add_nhop(device, eth2_20)
        self.add_neighbor_l3intf(device, eth2_20, nhop1, '00:20:20:20:20:25',
            ipaddr3)
        self.add_static_route(device, vrf, ipaddr3, nhop1)

        pkt = simple_tcpv6_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:10:10:10:10:15',
            ipv6_dst='3000:20:0:0:0:0:0:20',
            ipv6_src='3000:10:0:0:0:0:0:10',
            dl_vlan_enable=True,
            vlan_vid=10,
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst='00:20:20:20:20:25',
            eth_src='00:77:66:55:44:33',
            ipv6_dst='3000:20:0:0:0:0:0:20',
            ipv6_src='3000:10:0:0:0:0:0:10',
            dl_vlan_enable=True,
            vlan_vid=20,
            ipv6_hlim=63)

        try:
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

        finally:
            self.cleanup()


##############################################################################
@group('l3')
@group('ipv6')
@group('svi')
@group('maxsizes')
@group('ent')
class L3v6SVIToEcmpSubIntTest(ApiAdapter):

    def runTest(self):
        """
            Test svi with packet out to Ecmp port

                               +----------------------------+.1
            3000:10::10 >------|p1  vl10: 3000:10::1/120  p2|------> 3000:20::10
                        trunk  |                          p3|------> 3000:21::10
                               +----------------------------+
        """
        print ""
        print ("Sending packet port %d" % swports[0], " -> port %d" 
                % swports[1], " (3000:10::10 -> 3000:30::10)")

        vrf = self.add_vrf(device, 2)

        # Create vlan 10 & 20
        vlan10 = self.add_vlan(device, 10)
        vlan20 = self.add_vlan(device, 20)

        # Add router MAC
        rmac = self.add_rmac(device)
        self.add_router_mac(device, rmac,'00:77:66:55:44:33')

        # ingress
        port1 = self.select_port(device, swports[0])

        # egress
        port2 = self.select_port(device, swports[1])
        port3 = self.select_port(device, swports[2])

        # make ingress port a 'trunk' port
        eth1 = self.cfg_l2intf_on_port(device, port1, mode='trunk')
        self.add_vlan_member(device, vlan10, eth1)

        # Add SVI IP addresses to vlan 10
        intf_vl10 = self.add_logical_l3vlan(device, vrf, rmac, 10)
        ipaddr1 = self.make_ipv6_ipaddr('3000:10::1', 120)
        self.cfg_ip_address(device, intf_vl10, vrf, ipaddr1)

        # make egress ports 'sub-interface' ports
        eth2_20 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg2 = self.cfg_subintf_on_port(device, port2, eth2_20, vlan_id=20)
        ipaddr2 = self.make_ipv6_ipaddr('3000:20::1', 120)
        self.cfg_ip_address(device, eth2_20, vrf, ipaddr2)

        eth3_20 = self.add_logical_l3intf(device, vrf, rmac)
        ethcfg3 = self.cfg_subintf_on_port(device, port3, eth3_20, vlan_id=20)
        ipaddr3 = self.make_ipv6_ipaddr('3000:21::1', 120)
        self.cfg_ip_address(device, eth3_20, vrf, ipaddr3)

        # neighbor ip
        ipaddr4 = self.make_ipv6_ipaddr('3000:30::10', 128)

        nhop1 = self.add_nhop(device, eth2_20)
        self.add_neighbor_l3intf(device, eth2_20, nhop1, '00:11:22:33:44:55',
            ipaddr4)

        nhop2 = self.add_nhop(device, eth3_20)
        self.add_neighbor_l3intf(device, eth3_20, nhop2, '00:11:22:33:44:56',
            ipaddr4)

        # ecmp
        ecmp = self.add_ecmp(device)
        self.add_ecmp_member(device, ecmp, 2, [nhop1, nhop2])
        self.add_static_route(device, vrf, ipaddr4, ecmp)

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

        try:
            send_packet(self, swports[0], str(pkt))
            verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                       [swports[1], swports[2]])

        finally:
            self.cleanup()


##############################################################################
