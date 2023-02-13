
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
import pdb

import unittest
import random

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
import api_base_tests
sys.path.append(os.path.join(this_dir, '../../base/common'))
from api_adapter import ApiAdapter

device = 0
cpu_port = 64
swports = [x for x in range(65)]

###############################################################################
@group('l3')
@group('ipv6')
@group('maxsizes')
@group('feature-tests')
@group('ecmp')
@group('ent')
class L3IPv6EcmpDeleteReAddTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Description:Delete/re-add same member from ecmp group"
        print "Initialize test variables"
        self.mac = "00:77:66:55:44:33"
        self.nhopmac = {1 : '00:11:22:33:44:55', 2 : '00:11:22:33:44:56'}
        self.mac_type = 2
        self.maxvlan = 4
        self.intfmac = {1 : '00:11:11:11:11:11', 2 : '00:22:22:22:22:22'}
        self.intaddr = {1 : '2000:1:1:0:0:0:0:1', 2 : '3000:1:1:0:0:0:0:1',
                        3: '4000:1:1:0:0:0:0:1', 4 : '5000:1:1:0:0:0:0:1'}
        self.ports = [1, 2, 3]
        self.ip_dst = "10.0.0.1"
        self.ip_id = 102
        self.neighbor, self.intf, self.rif = ( {} for i in range(3))
        self.nhop, self.i_ip, self.port = ({} for i in range(3))

        self.vrf = self.add_vrf(device, 1)
        self.rmac = self.add_rmac_group(device, rmac_type='all')
        self.add_router_mac(device, self.rmac, self.mac)

        for i in self.ports:
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device, self.vrf, self.rmac)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.i_ip[i] = self.make_ipv6_ipaddr(self.intaddr[i], 120)
            self.cfg_ip_address(device, self.rif[i], self.vrf, self.i_ip[i])

        self.i_ip4 = self.make_ipv6_ipaddr('5000:1:1:0:0:0:0:1', 128)

        for i in xrange(1,len(self.ports)):
            self.nhop[i] = self.add_nhop(device, self.rif[i+1])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i+1],
                                                        self.nhop[i],
                                                        self.nhopmac[i],
                                                        self.i_ip4)

        self.ecmp = self.add_ecmp(device)
        self.add_ecmp_member(device,
                             self.ecmp,
                             len(self.nhopmac),
                             [self.nhop[i] for i in xrange(1,len(self.ports))])
        self.add_static_route(device, self.vrf, self.i_ip4, self.ecmp)

    def runTest(self):
        print "Remove one nhop from ECMP group and check traffic is received...."
        self.no_ecmp_member(device, self.ecmp, 1, [self.nhop[1]])

        print "Sending packet port %d" % (swports[1]) + \
              " -> port %d %d" % (swports[2],swports[3])
        pkt = simple_tcpv6_packet(eth_dst=self.mac,
                                  eth_src=self.intfmac[2],
                                  ipv6_dst=self.intaddr[4],
                                  ipv6_src=self.intaddr[1],
                                  tcp_sport=0x1234,
                                  ipv6_hlim=64)
        exp_pkt = {}
        for i in self.nhopmac:
            exp_pkt[i] = simple_tcpv6_packet(eth_dst=self.nhopmac[i],
                                             eth_src=self.mac,
                                             ipv6_dst=
                                             self.intaddr[len(self.intaddr)],
                                             ipv6_src=self.intaddr[1],
                                             tcp_sport=0x1234,
                                             ipv6_hlim=63)

        send_packet(self, swports[1], str(pkt))
        verify_any_packet_any_port(self,
                                   [ exp_pkt[i] for i in self.nhopmac ],
                                   [ swports[i] for i in
                                   xrange(2,len(self.ports)+1) ],
                                   timeout=2)

        print "Adding deleted nhop again to ECMP group and check" + \
              "traffic is received"
        #Add the member back
        status1 = self.add_ecmp_member(device, self.ecmp, 1, [self.nhop[1]])

        #Send traffic Again
        send_packet(self, swports[1], str(pkt))
        verify_any_packet_any_port(self,
                                   [ exp_pkt[i] for i in self.nhopmac ],
                                   [swports[i] for i in
                                   xrange(2,len(self.ports)+1) ],
                                   timeout=2)

        self.cleanup()

###############################################################################
@group('l3')
@group('ipv6')
@group('maxsizes')
@group('feature-tests')
@group('ecmp')
@group('ent')
class L3IPv6EcmpDeleteReAddDiffMemberTest(ApiAdapter):
    def setUp(self):
        print
        super(self.__class__, self).setUp()
        print "Description:Delete and add different members to the ecmp group"
        print "Initialize test variables"
        self.mac = "00:77:66:55:44:33"
        self.nhopmac = {1 : '00:11:22:33:44:55', 2 : '00:11:22:33:44:56',
                        3: '00:11:22:33:44:57'}
        self.mac_type = 2
        self.maxvlan = 4
        self.intfmac = {1 : '00:11:11:11:11:11', 2 : '00:22:22:22:22:22'}
        self.intaddr = {1 : '2000:1:1:0:0:0:0:1', 2 : '3000:1:1:0:0:0:0:1',
                        3: '4000:1:1:0:0:0:0:1', 4 : '5000:1:1:0:0:0:0:1',
                        5: '6000:1:1:0:0:0:0:1'}
        self.ports = [1, 2, 3, 4]
        self.ip_dst = "10.0.0.1"
        self.ip_id = 102
        self.intf, self.rif, self.neighbor = ({} for i in range(3))
        self.nhop, self.i_ip, self.port = ({} for i in range(3))

        self.vrf = self.add_vrf(device, 1)
        self.rmac = self.add_rmac_group(device, rmac_type='all')
        self.add_router_mac(device, self.rmac, self.mac)

        for i in self.ports:
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device, self.vrf, self.rmac)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.i_ip[i] = self.make_ipv6_ipaddr(self.intaddr[i], 120)
            self.cfg_ip_address(device, self.rif[i], self.vrf, self.i_ip[i])

        self.i_ip4 = self.make_ipv6_ipaddr('6000:1:1:0:0:0:0:1', 128)

        for i in xrange(1,len(self.ports)):
            self.nhop[i] = self.add_nhop(device, self.rif[i+1])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i+1],
                                                        self.nhop[i],
                                                        self.nhopmac[i],
                                                        self.i_ip4)

        self.ecmp = self.add_ecmp(device)
        self.add_ecmp_member(device,
                             self.ecmp,
                             len(self.nhopmac)-1,
                             [self.nhop[i] for i in range(1,len(self.ports)-1)])
        self.add_static_route(device, self.vrf, self.i_ip4, self.ecmp)

    def runTest(self):
        print "Remove one nhop from ECMP group and check traffic is received...."
        self.no_ecmp_member(device, self.ecmp, 1, [self.nhop[1]])

        print "Sending packet port %d" % (swports[1]) + \
              " -> port %d %d" % (swports[2],swports[3])
        pkt = simple_tcpv6_packet(eth_dst=self.mac,
                                  eth_src=self.intfmac[2],
                                  ipv6_dst=self.intaddr[5],
                                  ipv6_src=self.intaddr[1],
                                  tcp_sport=0x1234,
                                  ipv6_hlim=64)
        exp_pkt = {}
        for i in self.nhopmac:
            exp_pkt[i] = simple_tcpv6_packet(eth_dst=self.nhopmac[i],
                                             eth_src=self.mac,
                                             ipv6_dst=
                                             self.intaddr[len(self.intaddr)],
                                             ipv6_src=self.intaddr[1],
                                             tcp_sport=0x1234,
                                             ipv6_hlim=63)

        send_packet(self, swports[1], str(pkt))
        verify_any_packet_any_port(self, [ exp_pkt[i] for i in self.nhopmac ],
                                   [ swports[i] for i in
                                   xrange(2,len(self.ports)+1) ],
                                   timeout=2)

        #Add the member back
        print "Adding different nhop to ECMP group and check traffic is received"
        self.add_ecmp_member(device, self.ecmp, 1, [self.nhop[3]])

        #Send traffic Again
        print "Sending packet port %d" % (swports[1]) + \
              " -> port %d %d" % (swports[2], swports[3])
        send_packet(self, swports[1], str(pkt))
        verify_any_packet_any_port(self,
                                   [ exp_pkt[i] for i in self.nhopmac ],
                                   [swports[i] for i in
                                   xrange(2,len(self.ports)+1) ],
                                   timeout=2)

        self.cleanup()

###############################################################################
@group('l3')
@group('ipv6')
@group('maxsizes')
@group('feature-tests')
@group('ecmp')
@group('ent')
class L3IPv6EcmpDeleteReAddL3RouteTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Description:Delete l3 route and re-add"
        print "Initialize test variables"
        self.mac = "00:77:66:55:44:33"
        self.nhopmac = {1 : '00:11:22:33:44:55', 2 : '00:11:22:33:44:56'}
        self.mac_type = 2
        self.maxvlan = 4
        self.intfmac = {1 : '00:11:11:11:11:11', 2 : '00:22:22:22:22:22'}
        self.intaddr = {1 : '2000:1:1:0:0:0:0:1', 2 : '3000:1:1:0:0:0:0:1',
                        3: '4000:1:1:0:0:0:0:1', 4 : '5000:1:1:0:0:0:0:1'}
        self.ports = [1, 2, 3]
        self.ip_dst = "10.0.0.1"
        self.ip_id = 102

        self.nhop, self.i_ip, self.port = ({} for i in range(3))
        self.intf, self.rif, self.neighbor = ({} for i in range(3))

        self.vrf = self.add_vrf(device, 1)

        self.rmac = self.add_rmac_group(device, rmac_type='all')
        self.add_router_mac(device, self.rmac, self.mac)

        for i in self.ports:
            self.port[i] = self.select_port(device, swports[i])
            self.rif[i] = self.add_logical_l3intf(device, self.vrf, self.rmac)
            self.intf[i] = self.cfg_l3intf_on_port(device,
                                                   self.port[i],
                                                   self.rif[i])
            self.i_ip[i] = self.make_ipv6_ipaddr(self.intaddr[i], 120)
            self.cfg_ip_address(device, self.rif[i], self.vrf, self.i_ip[i])

        self.i_ip4 = self.make_ipv6_ipaddr('5000:1:1:0:0:0:0:1', 128)

        for i in xrange(1,len(self.ports)):
            self.nhop[i] = self.add_nhop(device, self.rif[i+1])
            self.neighbor[i] = self.add_neighbor_l3intf(device,
                                                        self.rif[i+1],
                                                        self.nhop[i],
                                                        self.nhopmac[i],
                                                        self.i_ip4)
        self.ecmp = self.add_ecmp(device)
        self.add_ecmp_member(device,
                             self.ecmp,
                             len(self.nhopmac),
                             [self.nhop[i] for i in xrange(1,len(self.ports))])
        self.add_static_route(device, self.vrf, self.i_ip4, self.ecmp)

    def runTest(self):
        print "Remove ECMP L3 Route and check traffic is not received on cpu port 64"
        self.no_static_route(device, self.vrf, self.i_ip4, self.ecmp)

        print "Sending packet port %d" % (swports[1]) + \
              " -> ports %d %d" % (swports[2], swports[3])
        pkt = simple_tcpv6_packet(eth_dst=self.mac,
                                  eth_src=self.intfmac[2],
                                  ipv6_dst=self.intaddr[4],
                                  ipv6_src=self.intaddr[1],
                                  tcp_sport=0x1234,
                                  ipv6_hlim=64)
        exp_pkt = {}
        for i in self.nhopmac:
            exp_pkt[i] = simple_tcpv6_packet(eth_dst=self.nhopmac[i],
                                             eth_src=self.mac,
                                             ipv6_dst=
                                             self.intaddr[len(self.intaddr)],
                                             ipv6_src=self.intaddr[1],
                                             tcp_sport=0x1234,
                                             ipv6_hlim=63)

        exp_pkt1 = simple_cpu_packet(ingress_port = 1,
                                        ingress_ifindex = 2,
                                        reason_code = 0x0,
                                        ingress_bd = 1 ,
                                        inner_pkt = pkt)

        send_packet(self, swports[1], str(pkt))
        verify_no_other_packets(self)

        print "Adding deleted L3 Route again to ECMP and" + \
               "check traffic is received..."
        #Add the L3 route back
        self.add_static_route(device, self.vrf, self.i_ip4, self.ecmp)

        #Send traffic Again
        print "Sending packet port %d" % (swports[1]) + \
              " -> ports %d %d"  % (swports[2], swports[3])
        send_packet(self, swports[1], str(pkt))
        verify_any_packet_any_port(self, [ exp_pkt[i] for i in self.nhopmac ],
                                   [swports[i] for i in
                                   xrange(2,len(self.ports)+1) ],
                                   timeout=2)

        self.cleanup()
