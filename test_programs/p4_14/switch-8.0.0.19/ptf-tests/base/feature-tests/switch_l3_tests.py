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

import switchapi_thrift

import time
import sys
import logging
import unittest
import random
import copy
import ptf.dataplane as dataplane

from ptf.testutils import *
from ptf.thriftutils import *

import os
import ptf.mask

from switchapi_thrift.ttypes import  *
from switchapi_thrift.switch_api_headers import  *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../base'))
from common.utils import *
sys.path.append(os.path.join(this_dir, '../../base/api-tests'))
sys.path.append(os.path.join(this_dir, '../../base/common'))
import api_base_tests
from api_utils import *
from api_adapter import ApiAdapter

device = 0
cpu_port = 64
swports = [x for x in range(65)]

################################################################################
#Create a group of RMACs and verify that the packet is received for each of the
#RMAC.
#Update one of the RMACs and verify that the packet is received for each of the
#RMAC configured on the device but not received for the deleted RMAC.
################################################################################
@group('l3')
@group('feature-tests')
@group('ent')
class L3IPv4UpdateMacAddressTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.ports = [1, 2]
        self.vrfid = 2
        self.ip_id = 105
        self.ttl = 64
        self.maxrmacs = 3
        self.prefix_len = 32
        self.ip_len = 24
        self.ipaddr = ['192.168.0.2', '10.0.0.2']
        self.staticip = {1 : '192.168.0.1', 2 : '10.0.0.1'}
        self.rmac = "00:77:66:55:44:33"
        self.mac = {1 : '00:11:11:11:11:11', 2 : '00:22:22:22:22:22'}
        self.vlan, self.intf, self.port = ({} for i in range(3))
        self.vlan_port, self.ip, self.nhop = ({} for i in range(3))
        self.neighbor, self.static_ip, self.rif = ({} for i in range(3))

        self.vrf = self.add_vrf(device, self.vrfid)
        self.rmac_group = self.add_rmac_group(device, rmac_type='all')

        self.mac_addr = self.rmac
        for mac in range(0, self.maxrmacs):
            self.add_router_mac(device, self.rmac_group, self.mac_addr)
            self.mac_addr = macincrement(self.mac_addr)

        for i,ipaddr in zip(self.ports, self.ipaddr):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.rmac_group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv4_ipaddr(ipaddr, self.prefix_len)
            self.cfg_ip_address(device, self.intf[i], self.vrf, self.ip[i])

        # Add a static route
        for i in self.ports:
            self.static_ip[i] = self.make_ipv4_ipaddr(self.staticip[i],
                                                      self.ip_len)
            self.nhop[i] = self.add_nhop(device, self.rif[i])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i],
                                                        self.nhop[i],
                                                        self.mac[i],
                                                        self.static_ip[i])
            self.add_static_route(device,
                                  self.vrf,
                                  self.static_ip[i],
                                  self.nhop[i])

    def runTest(self):
        print "Verify that the packets are sent and received for all the " + \
              "rmacs configured on the interface"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                macaddr = self.rmac
                for mac in range(0, self.maxrmacs):
                    pkt = simple_tcp_packet(eth_dst=macaddr,
                                            eth_src=self.mac[send],
                                            ip_dst=self.staticip[recv],
                                            ip_src=self.staticip[send],
                                            ip_id=self.ip_id,
                                            ip_ttl=self.ttl)
                    exp_pkt = simple_tcp_packet(eth_dst=self.mac[recv],
                                                eth_src=self.rmac,
                                                ip_dst=self.staticip[recv],
                                                ip_src=self.staticip[send],
                                                ip_id=self.ip_id,
                                                ip_ttl=self.ttl-1)

                    try:
                        send_packet(self, swports[send], str(pkt))
                        verify_packets(self, exp_pkt, [swports[recv]])
                    finally:
                        print "\tPacket from port %s to port " % (send) + \
                              "%s for mac address %s" % (recv, macaddr)
                        macaddr = macincrement(macaddr)

        print "Update rmac address..."
        self.no_router_mac(device, self.rmac_group, self.rmac)
        new_mac = macaddr
        self.add_router_mac(device, self.rmac_group, new_mac)

        print "Verify that the packets are received only for the " + \
              "updated rmac and not for deleted rmac"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                macaddr = self.rmac
                for mac in range(0, self.maxrmacs+1):
                    pkt = simple_tcp_packet(eth_dst=macaddr,
                                            eth_src=self.mac[send],
                                            ip_dst=self.staticip[recv],
                                            ip_src=self.staticip[send],
                                            ip_id=self.ip_id,
                                            ip_ttl=self.ttl)
                    exp_pkt = simple_tcp_packet(eth_dst=self.mac[recv],
                                                eth_src=new_mac,
                                                ip_dst=self.staticip[recv],
                                                ip_src=self.staticip[send],
                                                ip_id=self.ip_id,
                                                ip_ttl=self.ttl-1)
                    try:
                        send_packet(self, swports[send], str(pkt))
                        if macaddr == self.rmac:
                            verify_no_packet(self, exp_pkt, swports[recv])
                        else:
                            verify_packets(self, exp_pkt, [swports[recv]])
                    finally:
                        print "\tPacket from port %s to port " % (send) + \
                              "%s for mac address %s" % (recv, macaddr)
                        macaddr = macincrement(macaddr)

        self.cleanup()

################################################################################
@group('l3')
@group('feature-tests')
@group('ent')
class L3IPv6UpdateMacAddressTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.ports = [1, 2]
        self.ip_id = 105
        self.vrfid = 2
        self.ttl = 64
        self.maxrmacs = 3
        self.prefix_len = 128
        self.ip_len = 120
        self.ipaddr = ['2000::2', '3000::2']
        self.staticip = {1 : '2000::1', 2 : '3000::1'}
        self.rmac = "00:77:66:55:44:33"
        self.mac = {1 : '00:11:11:11:11:11', 2 : '00:22:22:22:22:22'}
        self.vlan, self.intf, self.rif = ({} for i in range(3))
        self.vlan_port, self.ip, self.nhop = ({} for i in range(3))
        self.neighbor, self.static_ip, self.port = ({} for i in range(3))

        self.vrf = self.add_vrf(device, self.vrfid)
        self.rmac_group = self.add_rmac_group(device, rmac_type='all')

        self.mac_addr = self.rmac
        for mac in range(0, self.maxrmacs):
            self.add_router_mac(device, self.rmac_group, self.mac_addr)
            self.mac_addr = macincrement(self.mac_addr)

        for i,ipaddr in zip(self.ports, self.ipaddr):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.rmac_group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv6_ipaddr(ipaddr, self.prefix_len)
            self.cfg_ip_address(device, self.rif[i], self.vrf, self.ip[i])

        # Add a static route
        for i in self.ports:
            self.static_ip[i] = self.make_ipv6_ipaddr(self.staticip[i],
                                                      self.ip_len)
            self.nhop[i] = self.add_nhop(device, self.rif[i])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i],
                                                        self.nhop[i],
                                                        self.mac[i],
                                                        self.static_ip[i])
            self.add_static_route(device,
                                  self.vrf,
                                  self.static_ip[i],
                                  self.nhop[i])

    def runTest(self):
        print "Verify that the packets are sent and received for all the " + \
              "rmacs configured on the interface"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                macaddr = self.rmac
                for mac in range(0, self.maxrmacs):
                    pkt = simple_tcpv6_packet(eth_dst=macaddr,
                                              eth_src=self.mac[send],
                                              ipv6_dst=self.staticip[recv],
                                              ipv6_src=self.staticip[send],
                                              ipv6_hlim=self.ttl)
                    exp_pkt = simple_tcpv6_packet(eth_dst=self.mac[recv],
                                                  eth_src=self.rmac,
                                                  ipv6_dst=self.staticip[recv],
                                                  ipv6_src=self.staticip[send],
                                                  ipv6_hlim=self.ttl-1)
                    try:
                        send_packet(self, swports[send], str(pkt))
                        verify_packets(self, exp_pkt, [swports[recv]])
                    finally:
                        print "\tPacket from port %s to port " % (send) + \
                              "%s for mac address %s" % (recv, macaddr)
                        macaddr = macincrement(macaddr)

        print "Update rmac address..."
        self.no_router_mac(device, self.rmac_group, self.rmac)
        new_mac = macaddr
        self.add_router_mac(device, self.rmac_group, new_mac)

        print "Verify that the packets are received only for the " + \
              "updated rmac and not for deleted rmac"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                macaddr = self.rmac
                for mac in range(0, self.maxrmacs+1):
                    pkt = simple_tcpv6_packet(eth_dst=macaddr,
                                              eth_src=self.mac[send],
                                              ipv6_dst=self.staticip[recv],
                                              ipv6_src=self.staticip[send],
                                              ipv6_hlim=self.ttl)
                    exp_pkt = simple_tcpv6_packet(eth_dst=self.mac[recv],
                                                  eth_src=new_mac,
                                                  ipv6_dst=self.staticip[recv],
                                                  ipv6_src=self.staticip[send],
                                                  ipv6_hlim=self.ttl-1)
                    try:
                        send_packet(self, swports[send], str(pkt))
                        if macaddr == self.rmac:
                            verify_no_packet(self, exp_pkt, swports[recv])
                        else:
                            verify_packets(self, exp_pkt, [swports[recv]])
                    finally:
                        print "\tPacket from port %s to port " % (send) + \
                              "%s for mac address %s" % (recv, macaddr)
                        macaddr = macincrement(macaddr)

        self.cleanup()

################################################################################
#Delete L3 route and verify that the packet doesn't go through.
#Re-add the L3 route and verify that the packet now goes through.
################################################################################
@group('l3')
@group('feature-tests')
@group('ent')
class L3IPv4DeleteReaddRouteTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.ports = [1, 2]
        self.vrfid = 2
        self.ip_id = 105
        self.ttl = 64
        self.prefix_len = 32
        self.ip_len = 16
        self.ipaddr = ['192.168.0.2', '10.0.0.2']
        self.staticip = {1 : '192.168.0.1', 2 : '10.0.0.1'}
        self.rmac = "00:77:66:55:44:33"
        self.mac = {1 : '00:11:11:11:11:11', 2 : '00:22:22:22:22:22'}
        self.vlan, self.intf, self.rif = ({} for i in range(3))
        self.vlan_port, self.ip, self.nhop = ({} for i in range(3))
        self.neighbor, self.static_ip, self.port = ({} for i in range(3))

        self.vrf = self.add_vrf(device, self.vrfid)
        self.rmac_group = self.add_rmac_group(device, rmac_type='all')
        self.add_router_mac(device, self.rmac_group, self.rmac)

        for i,ipaddr in zip(self.ports, self.ipaddr):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.rmac_group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv4_ipaddr(ipaddr, self.prefix_len)
            self.cfg_ip_address(device, self.intf[i], self.vrf, self.ip[i])

        print "Add static route"
        for i in self.ports:
            self.static_ip[i] = self.make_ipv4_ipaddr(self.staticip[i],
                                                      self.ip_len)
            self.nhop[i] = self.add_nhop(device, self.rif[i])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i],
                                                        self.nhop[i],
                                                        self.mac[i],
                                                        self.static_ip[i])
            self.add_static_route(device,
                                  self.vrf,
                                  self.static_ip[i],
                                  self.nhop[i])

    def runTest(self):
        print "Delete L3 route ..."
        for i in self.ports:
            self.no_neighbor(device, self.neighbor[i])
            self.no_static_route(device,
                                 self.vrf,
                                 self.static_ip[i],
                                 self.nhop[i])
            self.no_nhop(device, self.nhop[i])

        print "Verify the packet is not received after deleting L3 route"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                pkt = simple_tcp_packet(eth_dst=self.rmac,
                                        eth_src=self.mac[send],
                                        ip_dst=self.staticip[recv],
                                        ip_src=self.staticip[send],
                                        ip_id=self.ip_id,
                                        ip_ttl=self.ttl)
                exp_pkt = simple_tcp_packet(eth_dst=self.mac[recv],
                                            eth_src=self.rmac,
                                            ip_dst=self.staticip[recv],
                                            ip_src=self.staticip[send],
                                            ip_id=self.ip_id,
                                            ip_ttl=self.ttl-1)
                try:
                    send_packet(self, swports[send], str(pkt))
                    verify_no_packet(self, exp_pkt, swports[recv])
                finally:
                    print "\tPacket from port %s to port " % (send) + \
                          "%s for mac address %s" % (recv, self.rmac)

        print "Re-add static route..."
        for i in self.ports:
            self.static_ip[i] = self.make_ipv4_ipaddr(self.staticip[i],
                                                      self.ip_len)
            self.nhop[i] = self.add_nhop(device, self.rif[i])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i],
                                                        self.nhop[i],
                                                        self.mac[i],
                                                        self.static_ip[i])
            self.add_static_route(device,
                                  self.vrf,
                                  self.static_ip[i],
                                  self.nhop[i])

        print "Verify the packet is received after re-adding L3 route"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                pkt = simple_tcp_packet(eth_dst=self.rmac,
                                        eth_src=self.mac[send],
                                        ip_dst=self.staticip[recv],
                                        ip_src=self.staticip[send],
                                        ip_id=self.ip_id,
                                        ip_ttl=self.ttl)
                exp_pkt = simple_tcp_packet(eth_dst=self.mac[recv],
                                            eth_src=self.rmac,
                                            ip_dst=self.staticip[recv],
                                            ip_src=self.staticip[send],
                                            ip_id=self.ip_id,
                                            ip_ttl=self.ttl-1)
                try:
                    send_packet(self, swports[send], str(pkt))
                    verify_packets(self, exp_pkt, [swports[recv]])
                finally:
                    print "\tPacket from port %s to port " % (send) + \
                          "%s for mac address %s" % (recv, self.rmac)

        self.cleanup()

################################################################################
@group('l3')
@group('feature-tests')
@group('ent')
class L3IPv6DeleteReaddRouteTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.ports = [1, 2]
        self.vrfid = 2
        self.ip_id = 105
        self.ttl = 64
        self.prefix_len = 128
        self.ip_len = 120
        self.ipaddr = ['2000::2', '3000::2']
        self.staticip = {1 : '2000::1', 2 : '3000::1'}
        self.rmac = "00:77:66:55:44:33"
        self.mac = {1 : '00:11:11:11:11:11', 2 : '00:22:22:22:22:22'}
        self.vlan, self.intf, self.rif = ({} for i in range(3))
        self.vlan_port, self.ip, self.nhop = ({} for i in range(3))
        self.neighbor, self.static_ip, self.port = ({} for i in range(3))

        self.vrf = self.add_vrf(device, self.vrfid)
        self.rmac_group = self.add_rmac_group(device, rmac_type='all')
        self.add_router_mac(device, self.rmac_group, self.rmac)

        for i,ipaddr in zip(self.ports, self.ipaddr):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.rmac_group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv6_ipaddr(ipaddr, self.prefix_len)
            self.cfg_ip_address(device, self.intf[i], self.vrf, self.ip[i])

        print "Add static route"
        for i in self.ports:
            self.static_ip[i] = self.make_ipv6_ipaddr(self.staticip[i],
                                                      self.ip_len)
            self.nhop[i] = self.add_nhop(device, self.rif[i])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i],
                                                        self.nhop[i],
                                                        self.mac[i],
                                                        self.static_ip[i])
            self.add_static_route(device,
                                  self.vrf,
                                  self.static_ip[i],
                                  self.nhop[i])

    def runTest(self):
        print "Delete L3 route ..."
        for i in self.ports:
            self.no_neighbor(device, self.neighbor[i])
            self.no_static_route(device,
                                 self.vrf,
                                 self.static_ip[i],
                                 self.nhop[i])
            self.no_nhop(device, self.nhop[i])

        print "Verify the packet is not received after deleting L3 route"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                pkt = simple_tcpv6_packet(eth_dst=self.rmac,
                                          eth_src=self.mac[send],
                                          ipv6_dst=self.staticip[recv],
                                          ipv6_src=self.staticip[send],
                                          ipv6_hlim=self.ttl)
                exp_pkt = simple_tcpv6_packet(eth_dst=self.mac[recv],
                                              eth_src=self.rmac,
                                              ipv6_dst=self.staticip[recv],
                                              ipv6_src=self.staticip[send],
                                              ipv6_hlim=self.ttl-1)
                try:
                    send_packet(self, swports[send], str(pkt))
                    verify_no_packet(self, exp_pkt, swports[recv])
                finally:
                    print "\tPacket from port %s to port " % (send) + \
                          "%s for mac address %s" % (recv, self.rmac)

        print "Re-add static route..."
        for i in self.ports:
            self.static_ip[i] = self.make_ipv6_ipaddr(self.staticip[i],
                                                      self.ip_len)
            self.nhop[i] = self.add_nhop(device, self.rif[i])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i],
                                                        self.nhop[i],
                                                        self.mac[i],
                                                        self.static_ip[i])
            self.add_static_route(device,
                                  self.vrf,
                                  self.static_ip[i],
                                  self.nhop[i])

        print "Verify the packet is received after re-adding L3 route"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                pkt = simple_tcpv6_packet(eth_dst=self.rmac,
                                          eth_src=self.mac[send],
                                          ipv6_dst=self.staticip[recv],
                                          ipv6_src=self.staticip[send],
                                          ipv6_hlim=self.ttl)
                exp_pkt = simple_tcpv6_packet(eth_dst=self.mac[recv],
                                              eth_src=self.rmac,
                                              ipv6_dst=self.staticip[recv],
                                              ipv6_src=self.staticip[send],
                                              ipv6_hlim=self.ttl-1)
                try:
                    send_packet(self, swports[send], str(pkt))
                    verify_packets(self, exp_pkt, [swports[recv]])
                finally:
                    print "\tPacket from port %s to port " % (send) + \
                          "%s for mac address %s" % (recv, self.rmac)

        self.cleanup()

################################################################################
#Delete RMAC group and verify that the packet doesn't go through for any of the
#RMACs in the deleted group.
#Recreate the RMAc group and verify that the packet now goes through for all of
#the RMACs in the recreated group.
################################################################################
@group('l3')
@group('feature-tests')
@group('ent')
class L3IPv4DeleteRecreateMacgroupTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.ports = [1, 2]
        self.vrfid = 2
        self.ip_id = 105
        self.ttl = 64
        self.maxrmacs = 3
        self.prefix_len = 32
        self.ip_len = 16
        self.ipaddr = ['192.168.0.2', '10.0.0.2']
        self.staticip = {1 : '192.168.0.1', 2 : '10.0.0.1'}
        self.rmac = "00:77:66:55:44:33"
        self.mac = {1 : '00:11:11:11:11:11', 2 : '00:22:22:22:22:22'}
        self.vlan, self.intf, self.rif = ({} for i in range(3))
        self.vlan_port, self.ip, self.nhop = ({} for i in range(3))
        self.neighbor, self.static_ip, self.port = ({} for i in range(3))

        self.vrf = self.add_vrf(device, self.vrfid)
        self.rmac_group = self.add_rmac_group(device, rmac_type='all')

        self.mac_addr = self.rmac
        for mac in range(0, self.maxrmacs):
            self.add_router_mac(device, self.rmac_group, self.mac_addr)
            self.mac_addr = macincrement(self.mac_addr)

        for i,ipaddr in zip(self.ports, self.ipaddr):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.rmac_group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv4_ipaddr(ipaddr, self.prefix_len)
            self.cfg_ip_address(device, self.intf[i], self.vrf, self.ip[i])

        # Add a static route
        for i in self.ports:
            self.static_ip[i] = self.make_ipv4_ipaddr(self.staticip[i],
                                                      self.ip_len)
            self.nhop[i] = self.add_nhop(device, self.rif[i])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i],
                                                        self.nhop[i],
                                                        self.mac[i],
                                                        self.static_ip[i])
            self.add_static_route(device,
                                  self.vrf,
                                  self.static_ip[i],
                                  self.nhop[i])

    def runTest(self):
        print "Delete rmac group..."
        self.mac_addr = self.rmac
        for mac in range(0, self.maxrmacs):
            self.no_router_mac(device, self.rmac_group, self.mac_addr)
            self.mac_addr = macincrement(self.mac_addr)

        print "Verify that the packet is not received after deleting rmac group"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                macaddr = self.rmac
                for mac in range(0, self.maxrmacs):
                    pkt = simple_tcp_packet(eth_dst=macaddr,
                                            eth_src=self.mac[send],
                                            ip_dst=self.staticip[recv],
                                            ip_src=self.staticip[send],
                                            ip_id=self.ip_id,
                                            ip_ttl=self.ttl)
                    exp_pkt = simple_tcp_packet(eth_dst=self.mac[recv],
                                                eth_src=self.rmac,
                                                ip_dst=self.staticip[recv],
                                                ip_src=self.staticip[send],
                                                ip_id=self.ip_id,
                                                ip_ttl=self.ttl-1)
                    try:
                        send_packet(self, swports[send], str(pkt))
                        verify_no_packet(self, exp_pkt, swports[recv])
                    finally:
                        print "\tPacket from port %s to port " % (send) + \
                              "%s for mac address %s" % (recv, macaddr)
                        macaddr = macincrement(macaddr)

        print "Recreate rmac group..."
        self.mac_addr = self.rmac
        for mac in range(0, self.maxrmacs):
            self.add_router_mac(device, self.rmac_group, self.mac_addr)
            self.mac_addr = macincrement(self.mac_addr)

        print "Verify that the packet is received after recreating rmac group"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                macaddr = self.rmac
                for mac in range(0, self.maxrmacs):
                    pkt = simple_tcp_packet(eth_dst=macaddr,
                                            eth_src=self.mac[send],
                                            ip_dst=self.staticip[recv],
                                            ip_src=self.staticip[send],
                                            ip_id=self.ip_id,
                                            ip_ttl=self.ttl)
                    exp_pkt = simple_tcp_packet(eth_dst=self.mac[recv],
                                                eth_src=self.rmac,
                                                ip_dst=self.staticip[recv],
                                                ip_src=self.staticip[send],
                                                ip_id=self.ip_id,
                                                ip_ttl=self.ttl-1)
                    try:
                        send_packet(self, swports[send], str(pkt))
                        verify_packets(self, exp_pkt, [swports[recv]])
                    finally:
                        print "\tPacket from port %s to port " % (send) + \
                              "%s for mac address %s" % (recv, macaddr)
                        macaddr = macincrement(macaddr)

        self.cleanup()

################################################################################
@group('l3')
@group('feature-tests')
@group('ent')
class L3IPv6DeleteRecreateMacgroupTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.ports = [1, 2]
        self.vrfid = 2
        self.ip_id = 105
        self.ttl = 64
        self.maxrmacs = 3
        self.prefix_len = 128
        self.ip_len = 120
        self.ipaddr = ['2000::2', '3000::2']
        self.staticip = {1 : '2000::1', 2 : '3000::1'}
        self.rmac = "00:77:66:55:44:33"
        self.mac = {1 : '00:11:11:11:11:11', 2 : '00:22:22:22:22:22'}
        self.vlan, self.intf, self.rif = ({} for i in range(3))
        self.vlan_port, self.ip, self.nhop = ({} for i in range(3))
        self.neighbor, self.static_ip, self.port = ({} for i in range(3))

        self.vrf = self.add_vrf(device, self.vrfid)
        self.rmac_group = self.add_rmac_group(device, rmac_type='all')

        self.mac_addr = self.rmac
        for mac in range(0, self.maxrmacs):
            self.add_router_mac(device, self.rmac_group, self.mac_addr)
            self.mac_addr = macincrement(self.mac_addr)

        for i,ipaddr in zip(self.ports, self.ipaddr):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.rmac_group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv6_ipaddr(ipaddr, self.prefix_len)
            self.cfg_ip_address(device, self.intf[i], self.vrf, self.ip[i])

        # Add a static route
        for i in self.ports:
            self.static_ip[i] = self.make_ipv6_ipaddr(self.staticip[i],
                                                      self.prefix_len)
            self.nhop[i] = self.add_nhop(device, self.rif[i])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i],
                                                        self.nhop[i],
                                                        self.mac[i],
                                                        self.static_ip[i])
            self.add_static_route(device,
                                  self.vrf,
                                  self.static_ip[i],
                                  self.nhop[i])

    def runTest(self):
        print "Delete rmac group..."
        self.mac_addr = self.rmac
        for mac in range(0, self.maxrmacs):
            self.no_router_mac(device, self.rmac_group, self.mac_addr)
            self.mac_addr = macincrement(self.mac_addr)

        print "Verify that the packet is not received after deleting rmac group"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                macaddr = self.rmac
                for mac in range(0, self.maxrmacs):
                    pkt = simple_tcpv6_packet(eth_dst=macaddr,
                                              eth_src=self.mac[send],
                                              ipv6_dst=self.staticip[recv],
                                              ipv6_src=self.staticip[send],
                                              ipv6_hlim=self.ttl)
                    exp_pkt = simple_tcpv6_packet(eth_dst=self.mac[recv],
                                                  eth_src=self.rmac,
                                                  ipv6_dst=self.staticip[recv],
                                                  ipv6_src=self.staticip[send],
                                                  ipv6_hlim=self.ttl-1)
                    try:
                        send_packet(self, swports[send], str(pkt))
                        verify_no_packet(self, exp_pkt, swports[recv])
                    finally:
                        print "\tPacket from port %s to port " % (send) + \
                              "%s for mac address %s" % (recv, macaddr)
                        macaddr = macincrement(macaddr)

        print "Recreate rmac group..."
        self.mac_addr = self.rmac
        for mac in range(0, self.maxrmacs):
            self.add_router_mac(device, self.rmac_group, self.mac_addr)
            self.mac_addr = macincrement(self.mac_addr)

        print "Verify that the packet is received after recreating rmac group"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                macaddr = self.rmac
                for mac in range(0, self.maxrmacs):
                    pkt = simple_tcpv6_packet(eth_dst=macaddr,
                                              eth_src=self.mac[send],
                                              ipv6_dst=self.staticip[recv],
                                              ipv6_src=self.staticip[send],
                                              ipv6_hlim=self.ttl)
                    exp_pkt = simple_tcpv6_packet(eth_dst=self.mac[recv],
                                                  eth_src=self.rmac,
                                                  ipv6_dst=self.staticip[recv],
                                                  ipv6_src=self.staticip[send],
                                                  ipv6_hlim=self.ttl-1)
                    try:
                        send_packet(self, swports[send], str(pkt))
                        verify_packets(self, exp_pkt, [swports[recv]])
                    finally:
                        print "\tPacket from port %s to port " % (send) + \
                              "%s for mac address %s" % (recv, macaddr)
                        macaddr = macincrement(macaddr)

        self.cleanup()

################################################################################
#Delete L3 interface and verify that the packet doesn't go through.
#Recreate the L3 interface and verify that the packet now goes through.
################################################################################
@group('l3')
@group('feature-tests')
@group('ent')
class L3IPv4DelRecreateInterfaceTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.ports = [1, 2]
        self.vrfid = 2
        self.ip_id = 105
        self.ttl = 64
        self.maxrmacs = 3
        self.prefix_len = 32
        self.ip_len = 16
        self.ipaddr = ['192.168.0.2', '10.0.0.2']
        self.staticip = {1 : '192.168.0.1', 2 : '10.0.0.1'}
        self.rmac = "00:77:66:55:44:33"
        self.mac = {1 : '00:11:11:11:11:11', 2 : '00:22:22:22:22:22'}
        self.neighbor, self.static_ip = ({} for i in range(2))
        self.vlan_port, self.ip, self.nhop = ({} for i in range(3))
        self.vlan, self.intf, self.rif, self.port = ({} for i in range(4))

        self.vrf = self.add_vrf(device, self.vrfid)
        self.group = self.add_rmac_group(device, rmac_type='all')
        self.add_router_mac(device, self.group, self.rmac)

        for i,ipaddr in zip(self.ports, self.ipaddr):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv4_ipaddr(ipaddr, self.prefix_len)
            self.cfg_ip_address(device, self.intf[i], self.vrf, self.ip[i])

        # Add a static route
        for i in self.ports:
            self.static_ip[i] = self.make_ipv4_ipaddr(self.staticip[i],
                                                      self.ip_len)
            self.nhop[i] = self.add_nhop(device, self.rif[i])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i],
                                                        self.nhop[i],
                                                        self.mac[i],
                                                        self.static_ip[i])
            self.add_static_route(device,
                                  self.vrf,
                                  self.static_ip[i],
                                  self.nhop[i])

    def runTest(self):
        print "Delete L3 interface..."
        for i in self.ports:
            self.no_neighbor(device, self.neighbor[i])
            self.no_static_route(device,
                                 self.vrf,
                                 self.static_ip[i],
                                 self.nhop[i])
            self.no_nhop(device, self.nhop[i])
            self.no_ip_address(device, self.intf[i], self.vrf, self.ip[i])
            self.no_subintf_on_port(device, self.port[i], self.intf[i])

        print "Verify packet is not received after L3 interface is deleted"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                pkt = simple_tcp_packet(eth_dst=self.rmac,
                                        eth_src=self.mac[send],
                                        ip_dst=self.staticip[recv],
                                        ip_src=self.staticip[send],
                                        ip_id=self.ip_id,
                                        ip_ttl=self.ttl)
                exp_pkt = simple_tcp_packet(eth_dst=self.mac[recv],
                                            eth_src=self.rmac,
                                            ip_dst=self.staticip[recv],
                                            ip_src=self.staticip[send],
                                            ip_id=self.ip_id,
                                            ip_ttl=self.ttl-1)
                try:
                    send_packet(self, swports[send], str(pkt))
                    verify_no_packet(self, exp_pkt, swports[recv])
                finally:
                    print "\tPacket from port %s to port " % (send) + \
                          "%s for mac address %s" % (recv, self.rmac)

        print "Recreate L3 interface"
        for i,ipaddr in zip(self.ports, self.ipaddr):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv4_ipaddr(ipaddr, self.prefix_len)
            self.cfg_ip_address(device, self.intf[i], self.vrf, self.ip[i])

        # Add a static route
        for i in self.ports:
            self.static_ip[i] = self.make_ipv4_ipaddr(self.staticip[i],
                                                      self.ip_len)
            self.nhop[i] = self.add_nhop(device, self.rif[i])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i],
                                                        self.nhop[i],
                                                        self.mac[i],
                                                        self.static_ip[i])
            self.add_static_route(device,
                                  self.vrf,
                                  self.static_ip[i],
                                  self.nhop[i])

        print "Verify packet is received after L3 interface is recreated"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                pkt = simple_tcp_packet(eth_dst=self.rmac,
                                        eth_src=self.mac[send],
                                        ip_dst=self.staticip[recv],
                                        ip_src=self.staticip[send],
                                        ip_id=self.ip_id,
                                        ip_ttl=self.ttl)
                exp_pkt = simple_tcp_packet(eth_dst=self.mac[recv],
                                            eth_src=self.rmac,
                                            ip_dst=self.staticip[recv],
                                            ip_src=self.staticip[send],
                                            ip_id=self.ip_id,
                                            ip_ttl=self.ttl-1)
                try:
                    send_packet(self, swports[send], str(pkt))
                    verify_packets(self, exp_pkt, [swports[recv]])
                finally:
                    print "\tPacket from port %s to port " % (send) + \
                          "%s for mac address %s" % (recv, self.rmac)

        self.cleanup()

################################################################################
@group('l3')
@group('feature-tests')
@group('ent')
class L3IPv6DelRecreateInterfaceTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.ports = [1, 2]
        self.vrfid = 2
        self.ip_id = 105
        self.ttl = 64
        self.maxrmacs = 3
        self.prefix_len = 128
        self.ip_len = 120
        self.ipaddr = ['2000::2', '3000::2']
        self.staticip = {1 : '2000::1', 2 : '3000::1'}
        self.rmac = "00:77:66:55:44:33"
        self.mac = {1 : '00:11:11:11:11:11', 2 : '00:22:22:22:22:22'}
        self.neighbor, self.static_ip = ({} for i in range(2))
        self.vlan_port, self.ip, self.nhop = ({} for i in range(3))
        self.vlan, self.intf, self.rif, self.port = ({} for i in range(4))

        self.vrf = self.add_vrf(device, self.vrfid)
        self.group = self.add_rmac_group(device, rmac_type='all')
        self.add_router_mac(device, self.group, self.rmac)

        for i,ipaddr in zip(self.ports, self.ipaddr):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv6_ipaddr(ipaddr, self.prefix_len)
            self.cfg_ip_address(device, self.intf[i], self.vrf, self.ip[i])

        # Add a static route
        for i in self.ports:
            self.static_ip[i] = self.make_ipv6_ipaddr(self.staticip[i],
                                                      self.ip_len)
            self.nhop[i] = self.add_nhop(device, self.rif[i])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i],
                                                        self.nhop[i],
                                                        self.mac[i],
                                                        self.static_ip[i])
            self.add_static_route(device,
                                  self.vrf,
                                  self.static_ip[i],
                                  self.nhop[i])

    def runTest(self):
        print "Delete L3 interface..."
        for i in self.ports:
            self.no_neighbor(device, self.neighbor[i])
            self.no_static_route(device,
                                 self.vrf,
                                 self.static_ip[i],
                                 self.nhop[i])
            self.no_nhop(device, self.nhop[i])
            self.no_ip_address(device, self.intf[i], self.vrf, self.ip[i])
            self.no_subintf_on_port(device, self.port[i], self.intf[i])

        print "Verify packet is not received after L3 interface is deleted"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                pkt = simple_tcpv6_packet(eth_dst=self.rmac,
                                          eth_src=self.mac[send],
                                          ipv6_dst=self.staticip[recv],
                                          ipv6_src=self.staticip[send],
                                          ipv6_hlim=self.ttl)
                exp_pkt = simple_tcpv6_packet(eth_dst=self.mac[recv],
                                              eth_src=self.rmac,
                                              ipv6_dst=self.staticip[recv],
                                              ipv6_src=self.staticip[send],
                                              ipv6_hlim=self.ttl-1)
                try:
                    send_packet(self, swports[send], str(pkt))
                    verify_no_packet(self, exp_pkt, swports[recv])
                finally:
                    print "\tPacket from port %s to port " % (send) + \
                          "%s for mac address %s" % (recv, self.rmac)

        print "Recreate L3 interface"
        for i,ipaddr in zip(self.ports, self.ipaddr):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv6_ipaddr(ipaddr, self.prefix_len)
            self.cfg_ip_address(device, self.intf[i], self.vrf, self.ip[i])

        # Add a static route
        for i in self.ports:
            self.static_ip[i] = self.make_ipv6_ipaddr(self.staticip[i],
                                                      self.ip_len)
            self.nhop[i] = self.add_nhop(device, self.rif[i])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i],
                                                        self.nhop[i],
                                                        self.mac[i],
                                                        self.static_ip[i])
            self.add_static_route(device,
                                  self.vrf,
                                  self.static_ip[i],
                                  self.nhop[i])

        print "Verify packet is received after L3 interface is recreated"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                pkt = simple_tcpv6_packet(eth_dst=self.rmac,
                                          eth_src=self.mac[send],
                                          ipv6_dst=self.staticip[recv],
                                          ipv6_src=self.staticip[send],
                                          ipv6_hlim=self.ttl)
                exp_pkt = simple_tcpv6_packet(eth_dst=self.mac[recv],
                                              eth_src=self.rmac,
                                              ipv6_dst=self.staticip[recv],
                                              ipv6_src=self.staticip[send],
                                              ipv6_hlim=self.ttl-1)
                try:
                    send_packet(self, swports[send], str(pkt))
                    verify_packets(self, exp_pkt, [swports[recv]])
                finally:
                    print "\tPacket from port %s to port " % (send) + \
                          "%s for mac address %s" % (recv, self.rmac)

        self.cleanup()

################################################################################
#Delete L3 interface and verify that the packet doesn't go through.
#Recreate the L2 interface and verify that the packet now goes through.
################################################################################
@group('l3')
@group('l2')
@group('feature-tests')
@group('ent')
class InterfaceIPv4L3DelL2RecreateTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.ports = [1, 2]
        self.vrfid = 2
        self.ip_id = 105
        self.ttl = 64
        self.maxvlan = 4
        self.maxrmacs = 4
        self.prefix_len = 32
        self.ip_len = 16
        self.mac_type = 2
        self.ipaddr = ['192.168.0.2', '10.0.0.2']
        self.staticip = {1 : '192.168.0.1', 2 : '10.0.0.1'}
        self.rmac = "00:77:66:55:44:33"
        self.mac = {1 : '00:11:11:11:11:11', 2 : '00:22:22:22:22:22'}
        self.vlan_port, self.ip, self.nhop = ({} for i in range(3))
        self.neighbor, self.static_ip, self.port = ({} for i in range(3))
        self.vlan, self.intf, self.rif, self.intfmac = ({} for i in range(4))

        self.vrf = self.add_vrf(device, self.vrfid)
        self.group = self.add_rmac_group(device, rmac_type='all')
        self.add_router_mac(device, self.group, self.rmac)

        for i,ipaddr in zip(self.ports, self.ipaddr):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv4_ipaddr(ipaddr, self.prefix_len)
            self.cfg_ip_address(device, self.intf[i], self.vrf, self.ip[i])

        # Add a static route
        for i in self.ports:
            self.static_ip[i] = self.make_ipv4_ipaddr(self.staticip[i],
                                                      self.ip_len)
            self.nhop[i] = self.add_nhop(device, self.rif[i])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i],
                                                        self.nhop[i],
                                                        self.mac[i],
                                                        self.static_ip[i])
            self.add_static_route(device,
                                  self.vrf,
                                  self.static_ip[i],
                                  self.nhop[i])

    def runTest(self):
        print "Verify packet is received after L3 interface is created"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                pkt = simple_tcp_packet(eth_dst=self.rmac,
                                        eth_src=self.mac[send],
                                        ip_dst=self.staticip[recv],
                                        ip_src=self.staticip[send],
                                        ip_id=self.ip_id,
                                        ip_ttl=self.ttl)
                exp_pkt = simple_tcp_packet(eth_dst=self.mac[recv],
                                            eth_src=self.rmac,
                                            ip_dst=self.staticip[recv],
                                            ip_src=self.staticip[send],
                                            ip_id=self.ip_id,
                                            ip_ttl=self.ttl-1)
                try:
                    send_packet(self, swports[send], str(pkt))
                    verify_packets(self, exp_pkt, [swports[recv]])
                finally:
                    print "\tPacket from port %s to port %s" % (send, recv)

        print "Delete L3 interface..."
        for i in self.ports:
            self.no_neighbor(device, self.neighbor[i])
            self.no_static_route(device,
                                 self.vrf,
                                 self.static_ip[i],
                                 self.nhop[i])
            self.no_nhop(device, self.nhop[i])
            self.no_ip_address(device, self.intf[i], self.vrf, self.ip[i])
            self.no_subintf_on_port(device, self.port[i], self.intf[i])

        self.no_router_mac(device, self.group, self.rmac)
        self.no_rmac(device, self.group)
        self.no_vrf(device, self.vrf)

        print "Recreate L2 interface..."
        for vlanid in range(2, self.maxvlan):
            self.vlan[vlanid] = self.add_vlan(device, vlanid)

        for i in self.ports:
            self.port[i] = self.select_port(device, swports[i])
            self.intf[i] = self.cfg_l2intf_on_port(device,
                                                   self.port[i],
                                                   mode='trunk')

        for vlanid in range(2, self.maxvlan):
            for i in self.ports:
                self.intfmac[i, vlanid] = self.mac[i]
                mac = self.intfmac[i, vlanid]
                self.add_vlan_member(device,
                                     self.vlan[vlanid],
                                     self.intf[i])
                self.add_mac_table_entry(device,
                                         self.vlan[vlanid],
                                         mac,
                                         self.mac_type,
                                         self.intf[i])
                self.mac[i] = macincrement(self.mac[i])

        print "Verify packet is received after L3 interface is deleted and " + \
              "L2 interface is recreated"
        for vlanid in range(2, self.maxvlan):
            for send in self.ports:
                for recv in self.ports:
                    if send == recv:
                        continue
                    mac_recv = self.intfmac[recv, vlanid]
                    mac_send = self.intfmac[send, vlanid]
                    pkt = simple_tcp_packet(eth_dst=mac_recv,
                                            eth_src=mac_send,
                                            dl_vlan_enable=True,
                                            vlan_vid=vlanid,
                                            ip_dst=self.staticip[2],
                                            ip_id=self.ip_id)
                    exp_pkt = simple_tcp_packet(eth_dst=mac_recv,
                                                eth_src=mac_send,
                                                ip_dst=self.staticip[2],
                                                ip_id=self.ip_id,
                                                dl_vlan_enable=True,
                                                vlan_vid=vlanid)

                    try:
                        send_packet(self, swports[send], str(pkt))
                        verify_packets(self, exp_pkt, [swports[recv]])
                    finally:
                        print "\tPacket from port %d to port %d for " \
                              % (send, recv) + "vlan-id %s" % (vlanid)

        self.cleanup()

################################################################################
@group('l3')
@group('l2')
@group('feature-tests')
@group('ent')
class InterfaceIPv6L3DelL2RecreateTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.ports = [1, 2]
        self.vrfid = 2
        self.ip_id = 105
        self.ttl = 64
        self.maxvlan = 4
        self.prefix_len = 128
        self.ip_len = 120
        self.mac_type = 2
        self.ipaddr = ['2000::2', '3000::2']
        self.staticip = {1 : '2000::1', 2 : '3000::1'}
        self.rmac = "00:77:66:55:44:33"
        self.mac = {1 : '00:11:11:11:11:11', 2 : '00:22:22:22:22:22'}
        self.vlan_port, self.ip, self.nhop = ({} for i in range(3))
        self.neighbor, self.static_ip, self.intfmac = ({} for i in range(3))
        self.vlan, self.intf, self.rif, self.port = ({} for i in range(4))

        self.vrf = self.add_vrf(device, self.vrfid)
        self.group = self.add_rmac_group(device, rmac_type='all')
        self.add_router_mac(device, self.group, self.rmac)

        for i,ipaddr in zip(self.ports, self.ipaddr):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv6_ipaddr(ipaddr, self.prefix_len)
            self.cfg_ip_address(device, self.intf[i], self.vrf, self.ip[i])

        # Add a static route
        for i in self.ports:
            self.static_ip[i] = self.make_ipv6_ipaddr(self.staticip[i],
                                                      self.ip_len)
            self.nhop[i] = self.add_nhop(device, self.rif[i])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i],
                                                        self.nhop[i],
                                                        self.mac[i],
                                                        self.static_ip[i])
            self.add_static_route(device,
                                  self.vrf,
                                  self.static_ip[i],
                                  self.nhop[i])

    def runTest(self):
        print "Verify packet is received after L3 interface is created"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                pkt = simple_tcpv6_packet(eth_dst=self.rmac,
                                          eth_src=self.mac[send],
                                          ipv6_dst=self.staticip[recv],
                                          ipv6_src=self.staticip[send],
                                          ipv6_hlim=self.ttl)
                exp_pkt = simple_tcpv6_packet(eth_dst=self.mac[recv],
                                              eth_src=self.rmac,
                                              ipv6_dst=self.staticip[recv],
                                              ipv6_src=self.staticip[send],
                                              ipv6_hlim=self.ttl-1)
                try:
                    send_packet(self, swports[send], str(pkt))
                    verify_packets(self, exp_pkt, [swports[recv]])
                finally:
                    print "\tPacket from port %s to port " % (send) + \
                          "%s for mac address %s" % (recv, self.rmac)

        print "Delete L3 interface..."
        for i in self.ports:
            self.no_neighbor(device, self.neighbor[i])
            self.no_static_route(device,
                                 self.vrf,
                                 self.static_ip[i],
                                 self.nhop[i])
            self.no_nhop(device, self.nhop[i])
            self.no_ip_address(device, self.intf[i], self.vrf, self.ip[i])
            self.no_subintf_on_port(device, self.port[i], self.intf[i])

        self.no_router_mac(device, self.group, self.rmac)
        self.no_rmac(device, self.group)
        self.no_vrf(device, self.vrf)

        print "Recreate L2 interface..."
        for vlanid in range(2, self.maxvlan):
            self.vlan[vlanid] = self.add_vlan(device, vlanid)

        for i in self.ports:
            self.port[i] = self.select_port(device, swports[i])
            self.intf[i] = self.cfg_l2intf_on_port(device,
                                                   self.port[i],
                                                   mode='trunk')

        for vlanid in range(2, self.maxvlan):
            for i in self.ports:
                self.intfmac[i, vlanid] = self.mac[i]
                mac = self.intfmac[i, vlanid]
                self.add_vlan_member(device,
                                     self.vlan[vlanid],
                                     self.intf[i])
                self.add_mac_table_entry(device,
                                         self.vlan[vlanid],
                                         mac,
                                         self.mac_type,
                                         self.intf[i])
                self.mac[i] = macincrement(self.mac[i])

        print "Verify packet is received after L3 interface is deleted and" + \
              " L2 interface is recreated"
        for vlanid in range(2, self.maxvlan):
            for send in self.ports:
                for recv in self.ports:
                    if send == recv:
                        continue
                    mac_recv = self.intfmac[recv, vlanid]
                    mac_send = self.intfmac[send, vlanid]
                    pkt = simple_tcpv6_packet(eth_dst=mac_recv,
                                              eth_src=mac_send,
                                              dl_vlan_enable=True,
                                              vlan_vid=vlanid,
                                              ipv6_dst=self.staticip[2],
                                              ipv6_hlim=self.ttl)
                    exp_pkt = simple_tcpv6_packet(eth_dst=mac_recv,
                                                  eth_src=mac_send,
                                                  ipv6_dst=self.staticip[2],
                                                  ipv6_hlim=self.ttl,
                                                  dl_vlan_enable=True,
                                                  vlan_vid=vlanid)
                    try:
                        send_packet(self, swports[send], str(pkt))
                        verify_packets(self, exp_pkt, [swports[recv]])
                    finally:
                        print "\tPacket from port %d to port %d for " \
                              % (send, recv) + "vlan-id %s" % (vlanid)

        self.cleanup()

################################################################################
#Create RMAC group and verify that the packet goes through for all of the
#RMACs in the configured group.
#Delete and recreate a different RMAC group and verify that the packet now goes
#through for all of the RMACs in the recreated group.
################################################################################
@group('l3')
@group('feature-tests')
@group('ent')
class L3DeleteRecreateDiffMacgroupTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.ports = [1, 2]
        self.vrfid = 2
        self.ip_id = 105
        self.ttl = 64
        self.maxrmacs = 3
        self.prefix_len = 32
        self.ip_len = 16
        self.ipaddr = ['192.168.0.2', '10.0.0.2']
        self.staticip = {1 : '192.168.0.1', 2 : '10.0.0.1'}
        self.rmac1 = "00:77:66:55:44:33"
        self.rmac2 = "00:77:66:55:44:37"
        self.mac = {1 : '00:11:11:11:11:11', 2 : '00:22:22:22:22:22'}
        self.neighbor, self.static_ip, self.rif = ({} for i in range(3))
        self.vlan_port, self.ip, self.nhop = ({} for i in range(3))
        self.vlan, self.intf, self.port = ({} for i in range(3))

        self.vrf = self.add_vrf(device, self.vrfid)
        self.rmac_group = self.add_rmac_group(device, rmac_type='all')

        self.mac_addr = self.rmac1
        for mac in range(0, self.maxrmacs):
            self.add_router_mac(device, self.rmac_group, self.mac_addr)
            self.mac_addr = macincrement(self.mac_addr)

        for i,ipaddr in zip(self.ports, self.ipaddr):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.rmac_group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv4_ipaddr(ipaddr, self.prefix_len)
            self.cfg_ip_address(device, self.intf[i], self.vrf, self.ip[i])

        # Add a static route
        for i in self.ports:
            self.static_ip[i] = self.make_ipv4_ipaddr(self.staticip[i],
                                                      self.ip_len)
            self.nhop[i] = self.add_nhop(device, self.rif[i])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i],
                                                        self.nhop[i],
                                                        self.mac[i],
                                                        self.static_ip[i])
            self.add_static_route(device,
                                  self.vrf,
                                  self.static_ip[i],
                                  self.nhop[i])

    def runTest(self):
        print "Verify that the packet is received for first rmac group"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                macaddr = self.rmac1
                for mac in range(0, self.maxrmacs):
                    pkt = simple_tcp_packet(eth_dst=macaddr,
                                            eth_src=self.mac[send],
                                            ip_dst=self.staticip[recv],
                                            ip_src=self.staticip[send],
                                            ip_id=self.ip_id,
                                            ip_ttl=self.ttl)
                    exp_pkt = simple_tcp_packet(eth_dst=self.mac[recv],
                                                eth_src=self.rmac1,
                                                ip_dst=self.staticip[recv],
                                                ip_src=self.staticip[send],
                                                ip_id=self.ip_id,
                                                ip_ttl=self.ttl-1)
                    try:
                        send_packet(self, swports[send], str(pkt))
                        verify_packets(self, exp_pkt, [swports[recv]])
                    finally:
                        print "\tPacket from port %s to port " % (send) + \
                              "%s for mac address %s" % (recv, macaddr)
                        macaddr = macincrement(macaddr)

        print "Delete RMAC group and recreate a new RMAC group..."
        self.mac_addr = self.rmac1
        for mac in range(0, self.maxrmacs):
            self.no_router_mac(device, self.rmac_group, self.mac_addr)
            self.mac_addr = macincrement(self.mac_addr)

        self.mac_addr = self.rmac2
        for mac in range(0, self.maxrmacs):
            self.add_router_mac(device, self.rmac_group, self.mac_addr)
            self.mac_addr = macincrement(self.mac_addr)

        print "Verify that the packet is received after recreating rmac group"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                macaddr = self.rmac2
                for mac in range(0, self.maxrmacs):
                    pkt = simple_tcp_packet(eth_dst=macaddr,
                                            eth_src=self.mac[send],
                                            ip_dst=self.staticip[recv],
                                            ip_src=self.staticip[send],
                                            ip_id=self.ip_id,
                                            ip_ttl=self.ttl)
                    exp_pkt = simple_tcp_packet(eth_dst=self.mac[recv],
                                                eth_src=self.rmac2,
                                                ip_dst=self.staticip[recv],
                                                ip_src=self.staticip[send],
                                                ip_id=self.ip_id,
                                                ip_ttl=self.ttl-1)
                    try:
                        send_packet(self, swports[send], str(pkt))
                        verify_packets(self, exp_pkt, [swports[recv]])
                    finally:
                        print "\tPacket from port %s to port " % (send) + \
                              "%s for mac address %s" % (recv, macaddr)
                        macaddr = macincrement(macaddr)

        self.cleanup()

################################################################################
#Delete L3 interface and verify that the packet doesn't go through.
#Recreate the L3 interface and verify that the packet now goes through.
################################################################################
@group('l3')
@group('feature-tests')
@group('ent')
class L3DelIPv4RecreateIPv6IntfTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.ports = [1, 2]
        self.vrfid = 2
        self.ip_id = 105
        self.ttl = 64
        self.maxrmacs = 3
        self.prefix_len = [32, 128]
        self.ip_len = [16, 120]
        self.ipaddr4 = ['192.168.0.2', '10.0.0.2']
        self.staticip4 = {1 : '192.168.0.1', 2 : '10.0.0.1'}
        self.rmac = "00:77:66:55:44:33"
        self.mac = {1 : '00:11:11:11:11:11', 2 : '00:22:22:22:22:22'}
        self.ipaddr6 = ['2000::2', '3000::2']
        self.staticip6 = {1 : '2000::1', 2 : '3000::1'}
        self.vlan, self.intf, self.rif = ({} for i in range(3))
        self.vlan_port, self.ip, self.nhop = ({} for i in range(3))
        self.neighbor, self.static_ip, self.port = ({} for i in range(3))

        self.vrf = self.add_vrf(device, self.vrfid)
        self.group = self.add_rmac_group(device, rmac_type='all')
        self.add_router_mac(device, self.group, self.rmac)

        for i,ipaddr in zip(self.ports, self.ipaddr4):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv4_ipaddr(ipaddr, self.prefix_len[0])
            self.cfg_ip_address(device, self.intf[i], self.vrf, self.ip[i])

        # Add a static route
        for i in self.ports:
            self.static_ip[i] = self.make_ipv4_ipaddr(self.staticip4[i],
                                                      self.ip_len[0])
            self.nhop[i] = self.add_nhop(device, self.rif[i])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i],
                                                        self.nhop[i],
                                                        self.mac[i],
                                                        self.static_ip[i])
            self.add_static_route(device,
                                  self.vrf,
                                  self.static_ip[i],
                                  self.nhop[i])

    def runTest(self):
        print "Verify packet is received after IPv4 L3 interface is created"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                pkt = simple_tcp_packet(eth_dst=self.rmac,
                                        eth_src=self.mac[send],
                                        ip_dst=self.staticip4[recv],
                                        ip_src=self.staticip4[send],
                                        ip_id=self.ip_id,
                                        ip_ttl=self.ttl)
                exp_pkt = simple_tcp_packet(eth_dst=self.mac[recv],
                                            eth_src=self.rmac,
                                            ip_dst=self.staticip4[recv],
                                            ip_src=self.staticip4[send],
                                            ip_id=self.ip_id,
                                            ip_ttl=self.ttl-1)
                try:
                    send_packet(self, swports[send], str(pkt))
                    verify_packets(self, exp_pkt, [swports[recv]])
                finally:
                    print "\tPacket from port %s to port " % (send) + \
                          "%s for mac address %s" % (recv, self.rmac)

        print "Delete IPv4 L3 interface..."
        for i in self.ports:
            self.no_neighbor(device, self.neighbor[i])
            self.no_static_route(device,
                                 self.vrf,
                                 self.static_ip[i],
                                 self.nhop[i])
            self.no_nhop(device, self.nhop[i])
            self.no_ip_address(device, self.intf[i], self.vrf, self.ip[i])
            self.no_subintf_on_port(device, self.port[i], self.intf[i])

        print "Recreate IPv6 L3 interface"
        for i,ipaddr in zip(self.ports, self.ipaddr6):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv6_ipaddr(ipaddr, self.prefix_len[1])
            self.cfg_ip_address(device, self.intf[i], self.vrf, self.ip[i])

        for i in self.ports:
            self.static_ip[i] = self.make_ipv6_ipaddr(self.staticip6[i],
                                                      self.ip_len[1])
            self.nhop[i] = self.add_nhop(device, self.rif[i])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i],
                                                        self.nhop[i],
                                                        self.mac[i],
                                                        self.static_ip[i])
            self.add_static_route(device,
                                  self.vrf,
                                  self.static_ip[i],
                                  self.nhop[i])

        print "Verify packet is received after IPv6 L3 interface is recreated"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                pkt = simple_tcpv6_packet(eth_dst=self.rmac,
                                          eth_src=self.mac[send],
                                          ipv6_dst=self.staticip6[recv],
                                          ipv6_src=self.staticip6[send],
                                          ipv6_hlim=self.ttl)
                exp_pkt = simple_tcpv6_packet(eth_dst=self.mac[recv],
                                              eth_src=self.rmac,
                                              ipv6_dst=self.staticip6[recv],
                                              ipv6_src=self.staticip6[send],
                                              ipv6_hlim=self.ttl-1)
                try:
                    send_packet(self, swports[send], str(pkt))
                    verify_packets(self, exp_pkt, [swports[recv]])
                finally:
                    print "\tPacket from port %s to port " % (send) + \
                          "%s for mac address %s" % (recv, self.rmac)

        self.cleanup()

################################################################################
#Create the L3 interface and verify that the packet goes through.
#Delete the ip address and add it back. Verify packet goes through.
#Delete again, and add a different ip address. Verify packet goes through.
################################################################################
@group('l3')
@group('feature-tests')
@group('ent')
class L3IPv4DeleteAddAddressTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.ports = [1, 2]
        self.vrfid = 2
        self.ip_id = 105
        self.ttl = 64
        self.maxrmacs = 3
        self.prefix_len = 32
        self.ip_len = 16
        self.ipaddr = ['192.168.0.2', '10.0.0.2']
        self.staticip = {1 : '192.168.0.1', 2 : '10.0.0.1'}
        self.rmac = "00:77:66:55:44:33"
        self.mac = {1 : '00:11:11:11:11:11', 2 : '00:22:22:22:22:22'}
        self.neighbor, self.static_ip = ({} for i in range(2))
        self.vlan_port, self.ip, self.nhop = ({} for i in range(3))
        self.vlan, self.intf, self.rif, self.port = ({} for i in range(4))

        self.vrf = self.add_vrf(device, self.vrfid)
        self.rmac_group = self.add_rmac_group(device, rmac_type='all')
        self.add_router_mac(device, self.rmac_group, self.rmac)

        for i,ipaddr in zip(self.ports, self.ipaddr):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.rmac_group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv4_ipaddr(ipaddr, self.prefix_len)
            self.cfg_ip_address(device, self.intf[i], self.vrf, self.ip[i])

        # Add a static route
        for i in self.ports:
            self.static_ip[i] = self.make_ipv4_ipaddr(self.staticip[i],
                                                      self.ip_len)
            self.nhop[i] = self.add_nhop(device, self.rif[i])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i],
                                                        self.nhop[i],
                                                        self.mac[i],
                                                        self.static_ip[i])
            self.add_static_route(device,
                                  self.vrf,
                                  self.static_ip[i],
                                  self.nhop[i])
    def runTest(self):
        print "Verify packet is received after L3 interface is created"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                ip_address4 = self.staticip
                pkt = simple_tcp_packet(eth_dst=self.rmac,
                                        eth_src=self.mac[send],
                                        ip_dst=ip_address4[recv],
                                        ip_src=ip_address4[send],
                                        ip_id=self.ip_id,
                                        ip_ttl=self.ttl)
                exp_pkt = simple_tcp_packet(eth_dst=self.mac[recv],
                                            eth_src=self.rmac,
                                            ip_dst=ip_address4[recv],
                                            ip_src=ip_address4[send],
                                            ip_id=self.ip_id,
                                            ip_ttl=self.ttl-1)
                try:
                    send_packet(self, swports[send], str(pkt))
                    verify_packets(self, exp_pkt, [swports[recv]])
                finally:
                    print "\tPacket from port %s to port " % (send) + \
                          "%s for mac address %s" % (recv, self.rmac)

        print "Delete l3 interface address"
        for i in self.ports:
            self.no_ip_address(device, self.intf[i], self.vrf, self.ip[i]) #delete ip
        print "Add same l3 interface address"
        new_ipaddr = ip_address4
        for i,new_ipaddr in zip(self.ports, self.ipaddr):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.rmac_group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv4_ipaddr(new_ipaddr, self.prefix_len)
            self.cfg_ip_address(device, self.intf[i], self.vrf, self.ip[i])

        print "Verify packet is received after L3 interface address is added back"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                pkt = simple_tcp_packet(eth_dst=self.rmac,
                                        eth_src=self.mac[send],
                                        ip_dst=ip_address4[recv],
                                        ip_src=ip_address4[send],
                                        ip_id=self.ip_id,
                                        ip_ttl=self.ttl)
                exp_pkt = simple_tcp_packet(eth_dst=self.mac[recv],
                                            eth_src=self.rmac,
                                            ip_dst=ip_address4[recv],
                                            ip_src=ip_address4[send],
                                            ip_id=self.ip_id,
                                            ip_ttl=self.ttl-1)
                try:
                    send_packet(self, swports[send], str(pkt))
                    if ip_address4 == self.ipaddr:
                        verify_no_packet(self, exp_pkt, swports[recv])
                    else:
                        verify_packets(self, exp_pkt, [swports[recv]])
                finally:
                    print "\tPacket from port %s to port " % (send) + \
                          "%s for mac address %s" % (recv, self.rmac)
        print "Delete l3 interface address"
        for i in self.ports:
            self.no_ip_address(device, self.intf[i], self.vrf, self.ip[i]) #delete ip
        #add a different  address
        print "Add different l3 interface address from same subnet"
        self.new_ip = {1 : '192.168.0.3', 2 : '10.0.0.2'}
        self.newip1 = ['192.168.0.3', '10.0.0.2']
        newip = self.new_ip
        newipaddr4 = newip
        for i,newipaddr4 in zip(self.ports, self.newip1):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.rmac_group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv4_ipaddr(newipaddr4, self.prefix_len)
            self.cfg_ip_address(device, self.intf[i], self.vrf, self.ip[i])

        print "Verify packet is received after new L3 interface address added"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                pkt = simple_tcp_packet(eth_dst=self.rmac,
                                        eth_src=self.mac[send],
                                        ip_dst=newip[recv],
                                        ip_src=newip[send],
                                        ip_id=self.ip_id,
                                        ip_ttl=self.ttl)
                exp_pkt = simple_tcp_packet(eth_dst=self.mac[recv],
                                            eth_src=self.rmac,
                                            ip_dst=newip[recv],
                                            ip_src=newip[send],
                                            ip_id=self.ip_id,
                                            ip_ttl=self.ttl-1)
                try:
                    send_packet(self, swports[send], str(pkt))
                    if newip == self.newip1:
                        verify_no_packet(self, exp_pkt, swports[recv])
                    else:
                        verify_packets(self, exp_pkt, [swports[recv]])
                finally:
                    print "\tPacket from port %s to port " % (send) + \
                          "%s for mac address %s" % (recv, self.rmac)


        self.cleanup()

################################################################################
@group('l3')
@group('feature-tests')
@group('ent')
class L3IPv6DeleteAddAddressTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.ports = [1, 2]
        self.vrfid = 2
        self.ip_id = 105
        self.ttl = 64
        self.maxrmacs = 3
        self.prefix_len = 128
        self.ip_len = 120
        self.ipaddr = ['2000::2', '3000::2']
        self.staticip = {1 : '2000::1', 2 : '3000::1'}
        self.rmac = "00:77:66:55:44:33"
        self.mac = {1 : '00:11:11:11:11:11', 2 : '00:22:22:22:22:22'}
        self.neighbor, self.static_ip = ({} for i in range(2))
        self.vlan_port, self.ip, self.nhop = ({} for i in range(3))
        self.vlan, self.intf, self.rif, self.port = ({} for i in range(4))

        self.vrf = self.add_vrf(device, self.vrfid)
        self.group = self.add_rmac_group(device, rmac_type='all')
        self.add_router_mac(device, self.group, self.rmac)

        for i,ipaddr in zip(self.ports, self.ipaddr):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv6_ipaddr(ipaddr, self.prefix_len)
            self.cfg_ip_address(device, self.intf[i], self.vrf, self.ip[i])

        # Add a static route
        for i in self.ports:
            self.static_ip[i] = self.make_ipv6_ipaddr(self.staticip[i],
                                                      self.ip_len)
            self.nhop[i] = self.add_nhop(device, self.rif[i])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i],
                                                        self.nhop[i],
                                                        self.mac[i],
                                                        self.static_ip[i])
            self.add_static_route(device,
                                  self.vrf,
                                  self.static_ip[i],
                                  self.nhop[i])

    def runTest(self):
        print "Verify packet is received after L3 interface created"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                ip_address6 = self.staticip
                pkt = simple_tcpv6_packet(eth_dst=self.rmac,
                                          eth_src=self.mac[send],
                                          ipv6_dst=ip_address6[recv],
                                          ipv6_src=ip_address6[send],
                                          ipv6_hlim=self.ttl)
                exp_pkt = simple_tcpv6_packet(eth_dst=self.mac[recv],
                                              eth_src=self.rmac,
                                              ipv6_dst=ip_address6[recv],
                                              ipv6_src=ip_address6[send],
                                              ipv6_hlim=self.ttl-1)
                try:
                    send_packet(self, swports[send], str(pkt))
                    verify_packets(self, exp_pkt, [swports[recv]])
                finally:
                    print "\tPacket from port %s to port " % (send) + \
                          "%s for mac address %s" % (recv, self.rmac)

        print "Delete l3 interface address"
        for i in self.ports:
            self.no_ip_address(device, self.intf[i], self.vrf, self.ip[i])
        print "Add same l3 interface address"
        new_ipaddr = ip_address6
        for i,new_ipaddr in zip(self.ports, self.ipaddr):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv6_ipaddr(new_ipaddr, self.prefix_len)
            self.cfg_ip_address(device, self.intf[i], self.vrf, self.ip[i])

        print "Verify packet is received after L3 interface address is added back"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                ip_address6 = self.staticip
                pkt = simple_tcpv6_packet(eth_dst=self.rmac,
                                          eth_src=self.mac[send],
                                          ipv6_dst=ip_address6[recv],
                                          ipv6_src=ip_address6[send],
                                          ipv6_hlim=self.ttl)
                exp_pkt = simple_tcpv6_packet(eth_dst=self.mac[recv],
                                              eth_src=self.rmac,
                                              ipv6_dst=ip_address6[recv],
                                              ipv6_src=ip_address6[send],
                                              ipv6_hlim=self.ttl-1)
                try:
                    send_packet(self, swports[send], str(pkt))
                    if ip_address6 == self.ipaddr:
                        verify_no_packet(self, exp_pkt, swports[recv])
                    else:
                        verify_packets(self, exp_pkt, [swports[recv]])
                finally:
                    print "\tPacket from port %s to port " % (send) + \
                          "%s for mac address %s" % (recv, self.rmac)

        print "Delete l3 interface address"
        for i in self.ports:
            self.no_ip_address(device, self.intf[i], self.vrf, self.ip[i])
        #add a different address
        print "Add different l3 interface address from same subnet"
        self.newipaddr = ['2000::4', '3000::4']
        self.newstaticip = {1 : '2000::3', 2 : '3000::3'}
        newip = self.newstaticip
        newipaddr6 = newip
        for i,newipaddr6 in zip(self.ports, self.newipaddr):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv6_ipaddr(newipaddr6, self.prefix_len)
            self.cfg_ip_address(device, self.intf[i], self.vrf, self.ip[i])

        print "Verify packet is received after new L3 interface address added"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                pkt = simple_tcpv6_packet(eth_dst=self.rmac,
                                          eth_src=self.mac[send],
                                          ipv6_dst=newip[recv],
                                          ipv6_src=newip[send],
                                          ipv6_hlim=self.ttl)
                exp_pkt = simple_tcpv6_packet(eth_dst=self.mac[recv],
                                              eth_src=self.rmac,
                                              ipv6_dst=newip[recv],
                                              ipv6_src=newip[send],
                                              ipv6_hlim=self.ttl-1)
                try:
                    send_packet(self, swports[send], str(pkt))
                    if newip == self.newipaddr:
                        verify_no_packet(self, exp_pkt, swports[recv])
                    else:
                        verify_packets(self, exp_pkt, [swports[recv]])
                finally:
                    print "\tPacket from port %s to port " % (send) + \
                          "%s for mac address %s" % (recv, self.rmac)
        self.cleanup()

################################################################################
#Create the L3 interface and verify that the packet goes through.
#Delete the ip address and add it back. Verify packet goes through.
#Delete again, and add a different, new ip address. Verify packet goes through.
################################################################################
@group('l3')
@group('feature-tests')
@group('ent')
class L3IPv4DeleteAddNewAddressTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.ports = [1, 2]
        self.vrfid = 2
        self.ip_id = 105
        self.ttl = 64
        self.maxrmacs = 3
        self.prefix_len = 32
        self.ip_len = 32
        self.ipaddr = ['192.68.0.2', '10.0.0.2']
        self.staticip = {1: '192.168.0.1', 2: '10.0.0.1'}
        #ipaddr1, staticip1 are the new ips to be added
        self.ipaddr1 = ['172.0.16.2', '10.0.10.2']
        self.staticip1 = {1: '172.0.16.1', 2: '10.0.10.1'}
        self.rmac = "00:77:66:55:44:33"
        self.mac = {1 : '00:11:11:11:11:11', 2 : '00:22:22:22:22:22'}
        self.neighbor, self.static_ip = ({} for i in range(2))
        self.vlan_port, self.ip, self.nhop = ({} for i in range(3))
        self.vlan, self.intf, self.rif, self.port = ({} for i in range(4))

        self.vrf = self.add_vrf(device, self.vrfid)
        self.rmac_group = self.add_rmac_group(device, rmac_type='all')
        self.add_router_mac(device, self.rmac_group, self.rmac)

        for i,ipaddr in zip(self.ports, self.ipaddr):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.rmac_group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv4_ipaddr(ipaddr, self.prefix_len)
            self.cfg_ip_address(device, self.intf[i], self.vrf, self.ip[i])

        # Add a static route
        for i in self.ports:
            self.static_ip[i] = self.make_ipv4_ipaddr(self.staticip[i],
                                                      self.ip_len)
            self.nhop[i] = self.add_nhop(device, self.rif[i])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i],
                                                        self.nhop[i],
                                                        self.mac[i],
                                                        self.static_ip[i])
            self.add_static_route(device,
                                  self.vrf,
                                  self.static_ip[i],
                                  self.nhop[i])
    def runTest(self):
        print "Verify packet is received after L3 interface is created"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                ip_address4 = self.staticip
                pkt = simple_tcp_packet(eth_dst=self.rmac,
                                        eth_src=self.mac[send],
                                        ip_dst=ip_address4[recv],
                                        ip_src=ip_address4[send],
                                        ip_id=self.ip_id,
                                        ip_ttl=self.ttl)
                exp_pkt = simple_tcp_packet(eth_dst=self.mac[recv],
                                            eth_src=self.rmac,
                                            ip_dst=ip_address4[recv],
                                            ip_src=ip_address4[send],
                                            ip_id=self.ip_id,
                                            ip_ttl=self.ttl-1)
                try:
                    send_packet(self, swports[send], str(pkt))
                    verify_packets(self, exp_pkt, [swports[recv]])
                finally:
                    print "\tPacket from port %s to port " % (send) + \
                          "%s for mac address %s" % (recv, self.rmac)
        print "Delete L3 interface address..."
        for i in self.ports:
            self.no_neighbor(device, self.neighbor[i])
            self.no_static_route(device,
                                 self.vrf,
                                 self.static_ip[i],
                                 self.nhop[i])
            self.no_nhop(device, self.nhop[i])
            self.no_ip_address(device, self.intf[i], self.vrf, self.ip[i]) #delete ip
            self.no_subintf_on_port(device, self.port[i], self.intf[i])

        print "Update/add new L3 interface address"
        for i,ipaddr1 in zip(self.ports, self.ipaddr1):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.rmac_group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv4_ipaddr(ipaddr1, self.prefix_len)
            self.cfg_ip_address(device, self.intf[i], self.vrf, self.ip[i])

        # Add a static route
        for i in self.ports:
            self.static_ip[i] = self.make_ipv4_ipaddr(self.staticip1[i],
                                                      self.ip_len)
            self.nhop[i] = self.add_nhop(device, self.rif[i])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i],
                                                        self.nhop[i],
                                                        self.mac[i],
                                                        self.static_ip[i])
            self.add_static_route(device,
                                  self.vrf,
                                  self.static_ip[i],
                                  self.nhop[i])

        print "Verify packet is received after L3 interface address is updated"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                ip_address4 = self.staticip1
                pkt = simple_tcp_packet(eth_dst=self.rmac,
                                        eth_src=self.mac[send],
                                        ip_dst=ip_address4[recv],
                                        ip_src=ip_address4[send],
                                        ip_id=self.ip_id,
                                        ip_ttl=self.ttl)
                exp_pkt = simple_tcp_packet(eth_dst=self.mac[recv],
                                            eth_src=self.rmac,
                                            ip_dst=ip_address4[recv],
                                            ip_src=ip_address4[send],
                                            ip_id=self.ip_id,
                                            ip_ttl=self.ttl-1)
                try:
                    send_packet(self, swports[send], str(pkt))
                    verify_packets(self, exp_pkt, [swports[recv]])
                finally:
                    print "\tPacket from port %s to port " % (send) + \
                          "%s for mac address %s" % (recv, self.rmac)

        self.cleanup()

################################################################################
@group('l3')
@group('feature-tests')
@group('ent')
class L3IPv6DeleteAddNewAddressTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.ports = [1, 2]
        self.vrfid = 2
        self.ip_id = 105
        self.ttl = 64
        self.maxrmacs = 3
        self.prefix_len = 128
        self.ip_len = 120
        self.ipaddr = ['2000::2', '3000::2']
        self.staticip = {1 : '2000::1', 2 : '3000::1'}
        self.ipaddr1 = ['2001::2', '3001::2']
        self.staticip1 = {1 : '2001::1', 2 : '3001::1'}
        self.rmac = "00:77:66:55:44:33"
        self.mac = {1 : '00:11:11:11:11:11', 2 : '00:22:22:22:22:22'}
        self.neighbor, self.static_ip = ({} for i in range(2))
        self.vlan_port, self.ip, self.nhop = ({} for i in range(3))
        self.vlan, self.intf, self.rif, self.port = ({} for i in range(4))

        self.vrf = self.add_vrf(device, self.vrfid)
        self.group = self.add_rmac_group(device, rmac_type='all')
        self.add_router_mac(device, self.group, self.rmac)

        for i,ipaddr in zip(self.ports, self.ipaddr):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv6_ipaddr(ipaddr, self.prefix_len)
            self.cfg_ip_address(device, self.intf[i], self.vrf, self.ip[i])

        # Add a static route
        for i in self.ports:
            self.static_ip[i] = self.make_ipv6_ipaddr(self.staticip[i],
                                                      self.ip_len)
            self.nhop[i] = self.add_nhop(device, self.rif[i])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i],
                                                        self.nhop[i],
                                                        self.mac[i],
                                                        self.static_ip[i])
            self.add_static_route(device,
                                  self.vrf,
                                  self.static_ip[i],
                                  self.nhop[i])

    def runTest(self):
        print "Verify packet is received after L3 interface created"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                ip_address6 = self.staticip
                pkt = simple_tcpv6_packet(eth_dst=self.rmac,
                                          eth_src=self.mac[send],
                                          ipv6_dst=ip_address6[recv],
                                          ipv6_src=ip_address6[send],
                                          ipv6_hlim=self.ttl)
                exp_pkt = simple_tcpv6_packet(eth_dst=self.mac[recv],
                                              eth_src=self.rmac,
                                              ipv6_dst=ip_address6[recv],
                                              ipv6_src=ip_address6[send],
                                              ipv6_hlim=self.ttl-1)
                try:
                    send_packet(self, swports[send], str(pkt))
                    verify_packets(self, exp_pkt, [swports[recv]])
                finally:
                    print "\tPacket from port %s to port " % (send) + \
                          "%s for mac address %s" % (recv, self.rmac)

        print "Delete L3 interface address..."
        for i in self.ports:
            self.no_neighbor(device, self.neighbor[i])
            self.no_static_route(device,
                                 self.vrf,
                                 self.static_ip[i],
                                 self.nhop[i])
            self.no_nhop(device, self.nhop[i])
            self.no_ip_address(device, self.intf[i], self.vrf, self.ip[i])
            self.no_subintf_on_port(device, self.port[i], self.intf[i])

        print "Update/add new L3 interface address"
        for i,ipaddr1 in zip(self.ports, self.ipaddr1):
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device,
                                                  self.vrf,
                                                  self.group)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.ip[i] = self.make_ipv6_ipaddr(ipaddr1, self.prefix_len)
            self.cfg_ip_address(device, self.intf[i], self.vrf, self.ip[i])

        # Add a static route
        for i in self.ports:
            self.static_ip[i] = self.make_ipv6_ipaddr(self.staticip1[i],
                                                      self.ip_len)
            self.nhop[i] = self.add_nhop(device, self.rif[i])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i],
                                                        self.nhop[i],
                                                        self.mac[i],
                                                        self.static_ip[i])
            self.add_static_route(device,
                                  self.vrf,
                                  self.static_ip[i],
                                  self.nhop[i])

        print "Verify packet is received after L3 interface address is updated"
        for send in self.ports:
            for recv in self.ports:
                if send == recv:
                    continue
                ip_address6 = self.staticip1
                pkt = simple_tcpv6_packet(eth_dst=self.rmac,
                                          eth_src=self.mac[send],
                                          ipv6_dst=ip_address6[recv],
                                          ipv6_src=ip_address6[send],
                                          ipv6_hlim=self.ttl)
                exp_pkt = simple_tcpv6_packet(eth_dst=self.mac[recv],
                                              eth_src=self.rmac,
                                              ipv6_dst=ip_address6[recv],
                                              ipv6_src=ip_address6[send],
                                              ipv6_hlim=self.ttl-1)
                try:
                    send_packet(self, swports[send], str(pkt))
                    verify_packets(self, exp_pkt, [swports[recv]])
                finally:
                    print "\tPacket from port %s to port " % (send) + \
                          "%s for mac address %s" % (recv, self.rmac)
        self.cleanup()
