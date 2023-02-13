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
import os
import ptf.mask

#import ptf.dataplane as dataplane

#from ptf import config
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
swports = []
#for device, port, ifname in config["interfaces"]:
#    swports.append(port)

if swports == []:
    swports = [x for x in range(65)]


###############################################################################
@group('l2')
@group('feature-tests')
@group('vlan')
@group('ent')
class L2TrunkAllowVlanTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.mac_type = 2
        # assuming MIN sizes for now
        self.allowvlan = 100
        self.maxvlan = 250
        self.intfmac = {1: '00:11:11:11:11:11', 2: '00:22:22:22:22:22'}
        self.ports = [1, 2]
        self.vlanmac = "00:77:66:55:44:33"
        self.ip_dst = "10.0.0.1"
        self.ip_id = 102
        self.vlan, self.intf, self.port = ({} for i in range(3))
        self.vlan_port, self.mac= ({} for i in range(2))

        for vlanid in range(2, self.maxvlan):
            self.vlan[vlanid] = self.add_vlan(device, vlanid)

        for i in self.ports:
            self.port[i] = self.select_port(device, swports[i])
            self.intf[i] = self.cfg_l2intf_on_port(device,
                                                   self.port[i],
                                                   mode='trunk')

        for vlanid in range(2, self.allowvlan):
            for i in self.ports:
                self.mac[i, vlanid] = self.intfmac[i]
                self.add_vlan_member(device, self.vlan[vlanid], self.intf[i])
                mac = self.mac[i, vlanid]
                self.add_mac_table_entry(device,
                                         self.vlan[vlanid],
                                         mac,
                                         self.mac_type,
                                         self.intf[i])
                self.intfmac[i] = macincrement(self.intfmac[i])

    def runTest(self):
        allowvlan = random.sample(range(2, self.allowvlan), 5)
        print "Verify L2 packet from sending port to receiving port is " + \
              "received for all the allowed vlans [%s]" % allowvlan
        for vlanid in allowvlan:
            for send in self.ports:
                for recv in self.ports:
                    if send == recv:
                        continue
                    pkt = simple_tcp_packet(
                        eth_dst=self.mac[recv, vlanid],
                        eth_src=self.mac[send, vlanid],
                        dl_vlan_enable=True,
                        vlan_vid=vlanid,
                        ip_dst=self.ip_dst,
                        ip_id=self.ip_id)
                    exp_pkt = simple_tcp_packet(
                        eth_dst=self.mac[recv, vlanid],
                        eth_src=self.mac[send, vlanid],
                        ip_dst=self.ip_dst,
                        ip_id=self.ip_id,
                        dl_vlan_enable=True,
                        vlan_vid=vlanid)

                    try:
                        send_packet(self, swports[send], str(pkt))
                        verify_packets(self, exp_pkt, [swports[recv]])
                        remaining_ports = copy.copy(self.ports)
                        remaining_ports.remove(send)
                        remaining_ports.remove(recv)
                        for i in remaining_ports:
                            verify_no_packet(self, exp_pkt, swports[i])
                    finally:
                        print "\tPacket from port %d to port %d for " \
                              % (send, recv) + "vlan-id %s" % (vlanid)

        denyvlan = random.sample(range(self.allowvlan, self.maxvlan), 5)
        print "Verify L2 packet from sending port to receiving port is " + \
              "not received for denied vlans [%s]" % denyvlan
        for vlanid in denyvlan:
            for send in self.ports:
                for recv in self.ports:
                    if send == recv:
                        continue
                    pkt = simple_tcp_packet(
                        dl_vlan_enable=True,
                        vlan_vid=vlanid,
                        ip_dst=self.ip_dst,
                        ip_id=self.ip_id)
                    exp_pkt = simple_tcp_packet(
                        ip_dst=self.ip_dst,
                        ip_id=self.ip_id,
                        dl_vlan_enable=True,
                        vlan_vid=vlanid)

                    try:
                        send_packet(self, swports[send], str(pkt))
                        verify_no_packet(self, exp_pkt, swports[recv])
                    finally:
                        print "\tPacket from port %d to port %d for " \
                              % (send, recv) + "vlan-id %s" % (vlanid)

        self.cleanup()

################################################################################
@group('l2')
@group('feature-tests')
@group('vlan')
@group('ent')
class L2TrunkDenyAllVlanTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.maxvlan = 250
        self.mac_type = 2
        self.vlanmac = "00:77:66:55:44:33"
        self.ip_dst = "10.0.0.1"
        self.ip_id = 102
        self.ports = [1, 2]
        self.vlan, self.intf = ({} for i in range(2))
        self.port, vlan_port = ({} for i in range(2))

        print
        for vlanid in range(2, self.maxvlan):
            self.vlan[vlanid] = self.add_vlan(device, vlanid)

        for i in self.ports:
            self.port[i] = self.select_port(device, swports[i])
            self.intf[i] = self.cfg_l2intf_on_port(device,
                                                   self.port[i],
                                                   mode='trunk')

    def runTest(self):
        denyvlan = random.sample(range(2, self.maxvlan), 5)
        print "Verify L2 packets sent from sending port to receiving port " + \
              "are not received for denied vlans[%s]" % denyvlan
        for vlanid in denyvlan:
            for send in self.ports:
                for recv in self.ports:
                    if send == recv:
                        continue
                    pkt = simple_tcp_packet(
                        dl_vlan_enable=True,
                        vlan_vid=vlanid,
                        ip_dst=self.ip_dst,
                        ip_id=self.ip_id)
                    exp_pkt = simple_tcp_packet(
                        ip_dst=self.ip_dst,
                        ip_id=self.ip_id,
                        dl_vlan_enable=True,
                        vlan_vid=vlanid)

                    try:
                        send_packet(self, swports[send], str(pkt))
                        verify_no_packet(self, exp_pkt, swports[recv])
                    finally:
                        print "\tPacket from port %d to port %d for " \
                              % (send, recv) + "vlan-id %s" % (vlanid)

        self.cleanup()

################################################################################
@group('l2')
@group('feature-tests')
@group('vlan')
@group('ent')
class L2TrunkDeleteNativeVlanTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.maxvlan = 4
        self.mac_type = 2
        self.vlanmac = "00:77:66:55:44:33"
        self.intfmac = {1: '00:11:11:11:11:11', 2: '00:22:22:22:22:22'}
        self.ip_dst = "10.0.0.1"
        self.ip_id = 102
        self.ports = [1, 2]
        self.maxvlan = 5
        self.native_id = 2
        self.vlan, self.intf, self.port = ({} for i in range(3))
        self.vlan_port, self.mac = ({} for i in range(2))

        print
        for vlanid in range(2, self.maxvlan):
            self.vlan[vlanid] = self.add_vlan(device, vlanid)

        for i in self.ports:
            self.port[i] = self.select_port(device, swports[i])
            self.intf[i] = self.cfg_l2intf_on_port(device,
                                                   self.port[i],
                                                   mode='trunk')

        for vlanid in range(2, self.maxvlan):
            for i in self.ports:
                self.mac[i, vlanid] = self.intfmac[i]
                vlan = self.vlan[vlanid]
                mac = self.mac[i, vlanid]
                if vlanid == self.native_id:
                    status = self.set_native_vlan(device, self.intf[i], self.vlan[self.native_id])
                else:
                    self.add_vlan_member(device,
                                         self.vlan[vlanid],
                                         self.intf[i])
                self.add_mac_table_entry(device,
                                         self.vlan[vlanid],
                                         mac,
                                         self.mac_type,
                                         self.intf[i])
                self.intfmac[i] = macincrement(self.intfmac[i])

    def runTest(self):
        print "Verify tagged and untagged L2 packet sent is received when " + \
              "native vlan of id %s is added" % (self.native_id)
        for vlanid in range(2, self.maxvlan):
            for send in self.ports:
                for recv in self.ports:
                    if send == recv:
                        continue
                    if vlanid == self.native_id:
                        vlanenable = False
                    else:
                        vlanenable = True
                    pkt = simple_tcp_packet(
                        eth_dst=self.mac[recv, vlanid],
                        eth_src=self.mac[send, vlanid],
                        ip_dst=self.ip_dst,
                        ip_id=self.ip_id,
                        dl_vlan_enable=vlanenable,
                        vlan_vid=vlanid)
                    exp_pkt = simple_tcp_packet(
                        eth_dst=self.mac[recv, vlanid],
                        eth_src=self.mac[send, vlanid],
                        ip_dst=self.ip_dst,
                        ip_id=self.ip_id,
                        dl_vlan_enable=vlanenable,
                        vlan_vid=vlanid)

                    try:
                        send_packet(self, swports[send], str(pkt))
                        verify_packets(self, exp_pkt, [swports[recv]])
                        remaining_ports = copy.copy(self.ports)
                        remaining_ports.remove(send)
                        remaining_ports.remove(recv)
                        for i in remaining_ports:
                            verify_no_packet(self, exp_pkt, swports[i])
                    finally:
                        print "\t Packet from port %d to port %d with vlan " \
                              % (send, recv) + "enable %s" % (vlanenable)

        print "Deleting native vlan...."
        for i in self.ports:
            self.no_vlan_member(device, self.vlan[self.native_id], self.intf[i])
        for i in self.ports:
            self.add_vlan_member(device,
                                 self.vlan[self.native_id],
                                 self.intf[i])

        print "Verify only tagged L2 packet that is sent is received and " + \
              "untagged packets aren't received after native vlan of id " + \
              "%s is deleted" % (self.native_id)
        for vlanid in range(2, self.maxvlan):
            for send in self.ports:
                for recv in self.ports:
                    if send == recv:
                        continue
                    if vlanid == self.native_id:
                        vlanenable = False
                    else:
                        vlanenable = True
                    pkt = simple_tcp_packet(
                        eth_dst=self.mac[recv, vlanid],
                        eth_src=self.mac[send, vlanid],
                        dl_vlan_enable=vlanenable,
                        vlan_vid=vlanid,
                        ip_dst=self.ip_dst,
                        ip_id=self.ip_id)
                    exp_pkt = simple_tcp_packet(
                        eth_dst=self.mac[recv, vlanid],
                        eth_src=self.mac[send, vlanid],
                        ip_dst=self.ip_dst,
                        ip_id=self.ip_id,
                        dl_vlan_enable=vlanenable,
                        vlan_vid=vlanid)

                    try:
                        send_packet(self, swports[send], str(pkt))
                        if vlanenable == True:
                            verify_packets(self, exp_pkt, [swports[recv]])
                            remaining_ports = copy.copy(self.ports)
                            remaining_ports.remove(send)
                            remaining_ports.remove(recv)
                            for i in remaining_ports:
                                verify_no_packet(self, exp_pkt, swports[i])
                        else:
                            verify_no_packet(self, exp_pkt, swports[recv])
                    finally:
                        print "\t Packet from port %d to port %d for " \
                              % (send, recv) + "vlanid %s with vlan enable %s" \
                              % (vlanid, vlanenable)

        self.cleanup()

################################################################################
@group('l2')
@group('feature-tests')
@group('vlan')
@group('ent')
class L2TrunkDeleteAllowedVlanTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.mac_type = 2
        self.maxvlan = 4
        self.intfmac = {1: '00:11:11:11:11:11', 2: '00:22:22:22:22:22'}
        self.ports = [1, 2]
        self.vlanmac = "00:77:66:55:44:33"
        self.ip_dst = "10.0.0.1"
        self.ip_id = 102
        self.packettype = {'untagged' : False , 'tagged' : True}
        self.vlan, self.intf, self.mac = ({} for i in range(3))
        self.vlan_port, self.port = ({} for i in range(2))

        for vlanid in range(2, self.maxvlan):
            self.vlan[vlanid] = self.add_vlan(device, vlanid)

        for i in self.ports:
            self.port[i] = self.select_port(device, swports[i])
            self.intf[i] = self.cfg_l2intf_on_port(device,
                                                   self.port[i],
                                                   mode='trunk')

        for vlanid in range(2, self.maxvlan):
            for i in self.ports:
                self.mac[i, vlanid] = self.intfmac[i]
                mac = self.mac[i, vlanid]
                self.add_vlan_member(device, self.vlan[vlanid], self.intf[i])
                self.add_mac_table_entry(device,
                                         self.vlan[vlanid],
                                         mac,
                                         self.mac_type,
                                         self.intf[i])
                self.intfmac[i] = macincrement(self.intfmac[i])

    def runTest(self):
        print "Verify L2 packet from sending port to receiving port is " + \
              "received for all the allowed vlans"
        for vlanid in range(2, self.maxvlan):
            for send in self.ports:
                for recv in self.ports:
                    if send == recv:
                        continue
                    pkt = simple_tcp_packet(
                        eth_dst=self.mac[recv, vlanid],
                        eth_src=self.mac[send, vlanid],
                        dl_vlan_enable=True,
                        vlan_vid=vlanid,
                        ip_dst=self.ip_dst,
                        ip_id=self.ip_id)
                    exp_pkt = simple_tcp_packet(
                        eth_dst=self.mac[recv, vlanid],
                        eth_src=self.mac[send, vlanid],
                        ip_dst=self.ip_dst,
                        ip_id=self.ip_id,
                        dl_vlan_enable=True,
                        vlan_vid=vlanid)

                    try:
                        send_packet(self, swports[send], str(pkt))
                        verify_packets(self, exp_pkt, [swports[recv]])
                        remaining_ports = copy.copy(self.ports)
                        remaining_ports.remove(send)
                        remaining_ports.remove(recv)
                        for i in remaining_ports:
                            verify_no_packet(self, exp_pkt, swports[i])
                    finally:
                        print "\tPacket from port %d to port %d for " \
                              % (send, recv) + "vlan-id %s" % (vlanid)

        print "Deleting allowed vlan...."
        for vlanid in range(2, self.maxvlan):
            for i in self.ports:
                mac = self.mac[i, vlanid]
                self.no_mac_address_table_entry(device, self.vlan[vlanid], mac)
                self.no_vlan_member(device, self.vlan[vlanid], self.intf[i])

        print "Verify both tagged and untagged L2 packets are not received " + \
              "after allowed vlans are deleted"
        for pkttype in self.packettype:
            for vlanid in range(2, self.maxvlan):
                for send in self.ports:
                    for recv in self.ports:
                        if send == recv:
                            continue
                        enablevalue = self.packettype[pkttype]
                        pkt = simple_tcp_packet(eth_dst=self.mac[recv, vlanid],
                                                eth_src=self.mac[send, vlanid],
                                                dl_vlan_enable=enablevalue,
                                                vlan_vid=vlanid,
                                                ip_dst=self.ip_dst,
                                                ip_id=self.ip_id)
                        exp_pkt = \
                            simple_tcp_packet(eth_dst=self.mac[recv, vlanid],
                                              eth_src=self.mac[send, vlanid],
                                              ip_dst=self.ip_dst,
                                              ip_id=self.ip_id,
                                              dl_vlan_enable=enablevalue,
                                              vlan_vid=vlanid)

                        try:
                            send_packet(self, swports[send], str(pkt))
                            verify_no_packet(self, exp_pkt, swports[recv])
                        finally:
                            print "\tPacket from port %d to port %d for " \
                                  % (send, recv) + "vlan-id %s with vlan" \
                                  % (vlanid) + " enable %s" % (enablevalue)

        self.cleanup()

################################################################################
@group('l2')
@group('feature-tests')
@group('vlan')
@group('ent')
class L2TrunkRemoveReaddVlanTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.mac_type = 2
        self.maxvlan = 4
        self.intfmac = {1: '00:11:11:11:11:11', 2: '00:22:22:22:22:22'}
        self.ports = [1, 2]
        self.vlanmac = "00:77:66:55:44:33"
        self.ip_dst = "10.0.0.1"
        self.ip_id = 102
        self.packettype = {'untagged' : False , 'tagged' : True}
        self.vlan, self.intf, self.mac = ({} for i in range(3))
        self.vlan_port, self.port = ({} for i in range(2))

        for vlanid in range(2, self.maxvlan):
            self.vlan[vlanid] = self.add_vlan(device, vlanid)

        for i in self.ports:
            self.port[i] = self.select_port(device, swports[i])
            self.intf[i] = self.cfg_l2intf_on_port(device,
                                                   self.port[i],
                                                   mode='trunk')

            #self.vlan_port[i] = switcht_vlan_port_t(
            #    handle=self.intf[i], tagging_mode=0)

        for vlanid in range(2, self.maxvlan):
            for i in self.ports:
                self.mac[i, vlanid] = self.intfmac[i]
                mac = self.mac[i, vlanid]
                self.add_vlan_member(device, self.vlan[vlanid], self.intf[i])
                self.add_mac_table_entry(device,
                                         self.vlan[vlanid],
                                         mac,
                                         self.mac_type,
                                         self.intf[i])
                self.intfmac[i] = macincrement(self.intfmac[i])

    def runTest(self):
        print "Remove all vlans and verify no traffic is received...."
        for vlanid in range(2, self.maxvlan):
            for i in self.ports:
                self.no_vlan_member(device, self.vlan[vlanid], self.intf[i])

        print "Verify both tagged and untagged L2 packets are not received " + \
              "after allowed vlans are deleted"
        for pkttype in self.packettype:
            for vlanid in range(2, self.maxvlan):
                for send in self.ports:
                    for recv in self.ports:
                        if send == recv:
                            continue
                        enablevalue = self.packettype[pkttype]
                        pkt = simple_tcp_packet(eth_dst=self.mac[recv, vlanid],
                                                eth_src=self.mac[send, vlanid],
                                                dl_vlan_enable=enablevalue,
                                                vlan_vid=vlanid,
                                                ip_dst=self.ip_dst,
                                                ip_id=self.ip_id)
                        exp_pkt = \
                            simple_tcp_packet(eth_dst=self.mac[recv, vlanid],
                                              eth_src=self.mac[send, vlanid],
                                              ip_dst=self.ip_dst,
                                              ip_id=self.ip_id,
                                              dl_vlan_enable=enablevalue,
                                              vlan_vid=vlanid)
                        try:
                            send_packet(self, swports[send], str(pkt))
                            verify_no_packet(self, exp_pkt, swports[recv])
                        finally:
                            print "\tPacket from port %d to port %d for " \
                                  % (send, recv) + "vlan-id %s with vlan" \
                                  % (vlanid) + " enable %s" % (enablevalue)

        print "Re-add all vlans and verify traffic is received...."
        for vlanid in range(2, self.maxvlan):
            for i in self.ports:
                self.add_vlan_member(device, self.vlan[vlanid], self.intf[i])

        print "Verify L2 packet from sending port to receiving port is " + \
              "received after all the vlans re-added"
        for vlanid in range(2, self.maxvlan):
            for send in self.ports:
                for recv in self.ports:
                    if send == recv:
                        continue
                    pkt = simple_tcp_packet(
                        eth_dst=self.mac[recv, vlanid],
                        eth_src=self.mac[send, vlanid],
                        dl_vlan_enable=True,
                        vlan_vid=vlanid,
                        ip_dst=self.ip_dst,
                        ip_id=self.ip_id)
                    exp_pkt = simple_tcp_packet(
                        eth_dst=self.mac[recv, vlanid],
                        eth_src=self.mac[send, vlanid],
                        ip_dst=self.ip_dst,
                        ip_id=self.ip_id,
                        dl_vlan_enable=True,
                        vlan_vid=vlanid)

                    try:
                        send_packet(self, swports[send], str(pkt))
                        verify_packets(self, exp_pkt, [swports[recv]])
                        remaining_ports = copy.copy(self.ports)
                        remaining_ports.remove(send)
                        remaining_ports.remove(recv)
                        for i in remaining_ports:
                            verify_no_packet(self, exp_pkt, swports[i])
                    finally:
                        print "\tPacket from port %d to port %d for " \
                              % (send, recv) + "vlan-id %s" % (vlanid)

        self.cleanup()

################################################################################
@group('l2')
@group('feature-tests')
@group('vlan')
@group('ent')
class L2TrunkDeleteRecreateVlanTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.mac_type = 2
        self.maxvlan = 4
        self.intfmac = {1: '00:11:11:11:11:11', 2: '00:22:22:22:22:22'}
        self.ports = [1, 2]
        self.vlanmac = "00:77:66:55:44:33"
        self.ip_dst = "10.0.0.1"
        self.ip_id = 102
        self.packettype = {'untagged' : False , 'tagged' : True}
        self.vlan, self.intf, self.mac= ({} for i in range(3))
        self.vlan_port, self.port = ({} for i in range(2))

        for vlanid in range(2, self.maxvlan):
            self.vlan[vlanid] = self.add_vlan(device, vlanid)

        for i in self.ports:
            self.port[i] = self.select_port(device, swports[i])
            self.intf[i] = self.cfg_l2intf_on_port(device,
                                                   self.port[i],
                                                   mode='trunk')

        for vlanid in range(2, self.maxvlan):
            for i in self.ports:
                self.mac[i, vlanid] = self.intfmac[i]
                mac = self.mac[i, vlanid]
                self.add_vlan_member(device, self.vlan[vlanid], self.intf[i])
                self.add_mac_table_entry(device,
                                         self.vlan[vlanid],
                                         mac,
                                         self.mac_type,
                                         self.intf[i])
                self.intfmac[i] = macincrement(self.intfmac[i])

    def runTest(self):
        print "Delete vlans and verify no traffic is received...."
        for vlanid in range(2, self.maxvlan):
            for i in self.ports:
                mac = self.mac[i, vlanid]
                self.no_mac_address_table_entry(device, self.vlan[vlanid], mac)
                self.no_vlan_member(device, self.vlan[vlanid], self.intf[i])
            self.no_vlan(device, self.vlan[vlanid])

        print "Verify both tagged and untagged L2 packets are not received " + \
              "after allowed vlans are deleted"
        for pkttype in self.packettype:
            for vlanid in range(2, self.maxvlan):
                for send in self.ports:
                    for recv in self.ports:
                        if send == recv:
                            continue
                        enablevalue = self.packettype[pkttype]
                        pkt = simple_tcp_packet(eth_dst=self.mac[recv, vlanid],
                                                eth_src=self.mac[send, vlanid],
                                                dl_vlan_enable=enablevalue,
                                                vlan_vid=vlanid,
                                                ip_dst=self.ip_dst,
                                                ip_id=self.ip_id)
                        exp_pkt = \
                            simple_tcp_packet(eth_dst=self.mac[recv, vlanid],
                                              eth_src=self.mac[send, vlanid],
                                              ip_dst=self.ip_dst,
                                              ip_id=self.ip_id,
                                              dl_vlan_enable=enablevalue,
                                              vlan_vid=vlanid)

                        try:
                            send_packet(self, swports[send], str(pkt))
                            verify_no_packet(self, exp_pkt, swports[recv])
                        finally:
                            print "\tPacket from port %d to port %d for " \
                                  % (send, recv) + \
                                  "vlan-id %s with vlan enable %s" \
                                  % (vlanid, enablevalue)

        print "Recreate all vlans and verify traffic is received...."
        for vlanid in range(2, self.maxvlan):
            self.vlan[vlanid] = self.add_vlan(device, vlanid)
            for i in self.ports:
                self.mac[i, vlanid] = self.intfmac[i]
                mac = self.mac[i, vlanid]
                self.add_vlan_member(device, self.vlan[vlanid], self.intf[i])
                self.add_mac_table_entry(device,
                                         self.vlan[vlanid],
                                         mac,
                                         self.mac_type,
                                         self.intf[i])
                self.intfmac[i] = macincrement(self.intfmac[i])

        print "Verify L2 packet from sending port to receiving port is " + \
              "received after all the vlans re-added"
        for vlanid in range(2, self.maxvlan):
            for send in self.ports:
                for recv in self.ports:
                    if send == recv:
                        continue
                    pkt = simple_tcp_packet(
                        eth_dst=self.mac[recv, vlanid],
                        eth_src=self.mac[send, vlanid],
                        dl_vlan_enable=True,
                        vlan_vid=vlanid,
                        ip_dst=self.ip_dst,
                        ip_id=self.ip_id)
                    exp_pkt = simple_tcp_packet(
                        eth_dst=self.mac[recv, vlanid],
                        eth_src=self.mac[send, vlanid],
                        ip_dst=self.ip_dst,
                        ip_id=self.ip_id,
                        dl_vlan_enable=True,
                        vlan_vid=vlanid)

                    try:
                        send_packet(self, swports[send], str(pkt))
                        verify_packets(self, exp_pkt, [swports[recv]])
                        remaining_ports = copy.copy(self.ports)
                        remaining_ports.remove(send)
                        remaining_ports.remove(recv)
                        for i in remaining_ports:
                            verify_no_packet(self, exp_pkt, swports[i])
                    finally:
                        print "\tPacket from port %d to port %d for " \
                              % (send, recv) + "vlan-id %s" % (vlanid)
        self.cleanup()

################################################################################
@group('l2')
@group('feature-tests')
@group('vlan')
@group('ent')
class L2AccessAddMultipleVlanTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.mac_type = 2
        self.accessvlan = 2
        self.maxvlan = 4
        self.intfmac = {1: '00:11:11:11:11:11', 2: '00:22:22:22:22:22'}
        self.ports = [1, 2]
        self.vlanmac = "00:77:66:55:44:33"
        self.ip_dst = "10.0.0.1"
        self.ip_id = 102
        self.fail_flag = ''
        self.packettype = {'untagged': False, 'tagged': True}
        self.vlan, self.intf  = ({} for i in range(2))
        self.vlan_port, self.port = ({} for i in range(2))

    def runTest(self):
        for vlanid in range(self.accessvlan, self.maxvlan):
            self.vlan[vlanid] = self.add_vlan(device, vlanid)

        for i in self.ports:
            self.port[i] = self.select_port(device, swports[i])
            self.intf[i] = self.cfg_l2intf_on_port(device, self.port[i])

        for vlanid in range(self.accessvlan, self.maxvlan):
            for i in self.ports:
                print "\tAdding Vlan ID %s to interface %s" % (vlanid, i)
                try:
                    status = self.add_vlan_member(device,
                                                  self.vlan[vlanid],
                                                  self.intf[i])
                    if vlanid != 2 and status != 0:
                        self.fail_flag = True
                finally:
                    if self.fail_flag == True:
                        self.assertFalse(self.fail_flag,
                                         'Access interface allows multiple ' + \
                                         'VLANS to be added')

        self.cleanup()

################################################################################
@group('l2')
@group('feature-tests')
@group('vlan')
@group('ent')
class L2TrunkAddNonexistingVlanTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.mac_type = 2
        self.intfmac = {1: '00:11:11:11:11:11', 2: '00:22:22:22:22:22'}
        self.ports = [1, 2]
        self.vlanmac = "00:77:66:55:44:33"
        self.ip_dst = "10.0.0.1"
        self.ip_id = 102
        self.fail_flag = ''
        self.vlanid = 2
        self.packettype = {'untagged': False, 'tagged': True}
        self.vlan, self.intf, self.mac = ({} for i in range(3))
        self.vlan_port, self.port = ({} for i in range(2))

    def runTest(self):
        print "Create and delete a vlan"
        self.vlan = self.add_vlan(device, self.vlanid)
        self.no_vlan(device, self.vlan)

        for i in self.ports:
            self.port[i] = self.select_port(device, swports[i])
            self.intf[i] = self.cfg_l2intf_on_port(device,
                                                   self.port[i],
                                                   mode='trunk')

        print "Verify that non existing VLAN cannot be added"
        for i in self.ports:
            try:
                status = self.add_vlan_member(device, self.vlan, self.intf[i])
                if status == 'True':
                    self.fail_flag = True
            finally:
                if self.fail_flag == True:
                    self.assertFalse(self.fail_flag,'Trunk interface allows non-existing ' + \
                                     'VLAN to be added')

        self.cleanup()

################################################################################
@group('l2')
@group('feature-tests')
@group('vlan')
@group('ent')
class L2AddSameVlanTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.mac_type = 2
        self.intfmac = {1: '00:11:11:11:11:11', 2: '00:22:22:22:22:22'}
        self.vlanmac = "00:77:66:55:44:33"
        self.ip_dst = "10.0.0.1"
        self.ip_id = 102
        self.intftype = {1 : ['access', [1, 2]], \
                         2 : ['trunk', [3, 4]]}
        self.fail_flag = ''
        self.vlanid = 2
        self.packettype = {'untagged' : False , 'tagged' : True}
        self.vlan, self.intf, interface = ({} for i in range(3))
        self.vlan_port, self.mac, self.port = ({} for i in range(3))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.vlan = self.add_vlan(device, self.vlanid)

    def runTest(self):
        for intf in self.intftype:
            for i in self.intftype[intf][1]:
                intftype = self.intftype[intf][0]
                self.port[i] = self.select_port(device, swports[i])
                self.intf[i, intf] = self.cfg_l2intf_on_port(device,
                                                             self.port[i],
                                                             mode=intftype)
                try:
                    for num in range(1, 3):
                        status = self.add_vlan_member(device,
                                                      self.vlan,
                                                      self.intf[i, intf])
                        if num > 1 and status == True:
                            self.fail_flag = True
                finally:
                    if self.fail_flag == True:
                        self.assertFalse(self.fail_flag,'Same VLAN could ' + \
                                         'be added to port %d, ' % (i) + \
                                         '%s interface' % (intftype))

        self.cleanup()

################################################################################
@group('l2')
@group('feature-tests')
@group('vlan')
@group('ent')
class L2VlanMemberAccessInterface(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Description:Change vlan membership of access interfaces"
        print "Initialize test variables"
        self.mac_type = 2
        self.maxvlan = 12
        self.intfmac = {1 : '00:11:11:11:11:11', 2 : '00:22:22:22:22:22'}
        self.ports = [1, 2]
        self.vlanmac = "00:77:66:55:44:33"
        self.ip_dst = "10.0.0.1"
        self.ip_id = 102
        self.vlan, self.intf, self.mac= ({} for i in range(3))
        self.vlan_port, self.port = ({} for i in range(2))

        #Creating VLAN's
        for vlanid in xrange(10, self.maxvlan):
            self.vlan[vlanid] = self.add_vlan(device, vlanid)

        #Creating and adding interface to a vlan
        for i in self.ports:
            self.port[i] = self.select_port(device, swports[i])
            self.intf[i] = self.cfg_l2intf_on_port(device,
                                                   self.port[i],
                                                   mode='access')

        for vlanid in range(10, self.maxvlan-1):
            for i in self.ports:
                self.mac[i, vlanid] = self.intfmac[i]
                self.add_vlan_member(device, self.vlan[vlanid], self.intf[i])
                self.add_mac_table_entry(device,
                                         self.vlan[vlanid],
                                         self.mac[i, vlanid],
                                         self.mac_type,
                                         self.intf[i])
    def runTest(self):
        print "Verify L2 packet from sending port to receiving port is " + \
              "received for vlan configured "
        for vlanid in range(10, self.maxvlan-1):
            for send in self.ports:
                for recv in self.ports:
                    if send == recv:
                        continue
                    pkt = simple_udp_packet(eth_dst=self.mac[recv, vlanid],
                                            eth_src=self.mac[send, vlanid],
                                            ip_dst=self.ip_dst,
                                            ip_ttl=64)
                    exp_pkt = simple_udp_packet(eth_dst=self.mac[recv, vlanid],
                                                eth_src=self.mac[send, vlanid],
                                                ip_dst=self.ip_dst,
                                                ip_ttl=64)
                    try:
                        send_packet(self, swports[send], str(pkt))
                        verify_packets(self, exp_pkt, [swports[recv]])
                    finally:
                        print "\tPacket from port %d to port %d for " \
                              % (send, recv) + "vlan-id %s" % (vlanid)

        #Removing interface from first vlan
        for vlanid in range(10, self.maxvlan-1):
            for i in self.ports:
                self.mac[i, vlanid] = self.intfmac[i]
                self.no_mac_address_table_entry(device,
                                                 self.vlan[vlanid],
                                                 self.mac[i, vlanid])
                self.no_vlan_member(device, self.vlan[vlanid], self.intf[i])

        time.sleep(5)
        #Adding interface to different vlan
        print "Verify L2 packet from sending port to receiving port is " + \
              "received for changed vlan configured "
        for vlanid in range(11, self.maxvlan):
            for i in self.ports:
                self.mac[i, vlanid] = self.intfmac[i]
                self.add_vlan_member(device, self.vlan[vlanid], self.intf[i])
                self.add_mac_table_entry(device,
                                         self.vlan[vlanid],
                                         self.mac[i, vlanid],
                                         self.mac_type,
                                         self.intf[i])

        for vlanid in range(11, self.maxvlan):
            for send in self.ports:
                for recv in self.ports:
                    if send == recv:
                        continue
                    pkt = simple_udp_packet(eth_dst=self.mac[recv, vlanid],
                                            eth_src=self.mac[send, vlanid],
                                            ip_dst=self.ip_dst,
                                            ip_ttl=64)
                    exp_pkt = simple_udp_packet(eth_dst=self.mac[recv, vlanid],
                                                eth_src=self.mac[send, vlanid],
                                                ip_dst=self.ip_dst,
                                                ip_ttl=64)

                    try:
                        send_packet(self, swports[send], str(pkt))
                        verify_packets(self, exp_pkt, [swports[recv]])
                    finally:
                        print "\tPacket from port %d to port %d for " \
                              % (send, recv) + "vlan-id %s" % (vlanid)
        self.cleanup()

################################################################################
@group('l2')
@group('feature-tests')
@group('vlan')
@group('ent')
class L2VlanMemberTrunkInterface(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Description:Change vlan membership of trunk interfaces"
        print "Initialize test variables"
        self.mac_type = 2
        self.maxvlan = 12
        self.intfmac = {1 : '00:11:11:11:11:11', 2 : '00:22:22:22:22:22'}
        self.ports = [1, 2]
        self.vlanmac = "00:77:66:55:44:33"
        self.ip_dst = "10.0.0.1"
        self.ip_id = 102
        self.vlan, self.intf, self.mac = ({} for i in range(3))
        self.vlan_port, self.port = ({} for i in range(2))

        #Creating VLAN's
        for vlanid in xrange(10, self.maxvlan):
            self.vlan[vlanid] = self.add_vlan(device, vlanid)
        #Creating and adding interface to a vlan
        for i in self.ports:
            self.port[i] = self.select_port(device, swports[i])
            self.intf[i] = self.cfg_l2intf_on_port(device,
                                                   self.port[i],
                                                   mode='trunk')
        for vlanid in range(10, self.maxvlan-1):
            for i in self.ports:
                self.mac[i, vlanid] = self.intfmac[i]
                self.add_vlan_member(device, self.vlan[vlanid], self.intf[i])
                self.add_mac_table_entry(device,
                                         self.vlan[vlanid],
                                         self.mac[i, vlanid],
                                         self.mac_type,
                                         self.intf[i])

    def runTest(self):
        print "Verify L2 packet from sending port to receiving port is " + \
              "received for the configured vlan "
        for vlanid in range(10, self.maxvlan-1):
            for send in self.ports:
                for recv in self.ports:
                    if send == recv:
                        continue
                    pkt = simple_udp_packet(eth_dst=self.mac[recv, vlanid],
                                            eth_src=self.mac[send, vlanid],
                                            ip_dst=self.ip_dst,
                                            dl_vlan_enable=True,
                                            vlan_vid=vlanid,
                                            ip_ttl=64)
                    exp_pkt = simple_udp_packet(eth_dst=self.mac[recv, vlanid],
                                                eth_src=self.mac[send, vlanid],
                                                ip_dst=self.ip_dst,
                                                dl_vlan_enable=True,
                                                vlan_vid=vlanid,
                                                ip_ttl=64)
                    try:
                        send_packet(self, swports[send], str(pkt))
                        verify_packets(self, exp_pkt, [swports[recv]])
                    finally:
                        print "\tPacket from port %d to port %d for " \
                              % (send, recv) + "vlan-id %s" % (vlanid)

      #Removing interface from first vlan
        for vlanid in range(10, self.maxvlan-1):
            for i in self.ports:
                self.mac[i, vlanid] = self.intfmac[i]
                self.no_mac_address_table_entry(device,
                                                 self.vlan[vlanid],
                                                 self.mac[i, vlanid])
                self.no_vlan_member(device, self.vlan[vlanid], self.intf[i])

       #Adding interface to different vlan
        for vlanid in range(11, self.maxvlan):
            for i in self.ports:
                self.mac[i, vlanid] = self.intfmac[i]
                self.add_vlan_member(device, self.vlan[vlanid], self.intf[i])
                self.add_mac_table_entry(device,
                                         self.vlan[vlanid],
                                         self.mac[i, vlanid],
                                         self.mac_type,
                                         self.intf[i])

        print "Verify L2 packet from sending port to receiving port is " + \
              "received for the changed configured vlan "
        for vlanid in range(11, self.maxvlan):
            for send in self.ports:
                for recv in self.ports:
                    if send == recv:
                        continue
                    pkt = simple_udp_packet(eth_dst=self.mac[recv, vlanid],
                                            eth_src=self.mac[send, vlanid],
                                            ip_dst=self.ip_dst,
                                            dl_vlan_enable=True,
                                            vlan_vid=vlanid,
                                            ip_ttl=64)
                    exp_pkt = simple_udp_packet(eth_dst=self.mac[recv, vlanid],
                                                eth_src=self.mac[send, vlanid],
                                                ip_dst=self.ip_dst,
                                                dl_vlan_enable=True,
                                                vlan_vid=vlanid,
                                                ip_ttl=64)
                    try:
                        send_packet(self, swports[send], str(pkt))
                        verify_packets(self, exp_pkt, [swports[recv]])
                    finally:
                        print "\tPacket from port %d to port %d for " \
                              % (send, recv) + "vlan-id %s" % (vlanid)
        self.cleanup()

################################################################################
@group('l2')
@group('feature-tests')
@group('vlan')
@group('ent')
class L2VlanMemberLagTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Description:Change vlan membership of lag interface"
        print "Initialize test variables"
        self.mac_type = 2
        self.maxvlan = 12
        self.intfmac = {1 : '00:11:11:11:11:11', 2 : '00:22:22:22:22:22'}
        self.ports = [1, 2, 3, 4, 5]
        self.vlanmac = "00:77:66:55:44:33"
        self.ip_dst = "10.0.0.1"
        self.ip_src='192.168.8.1'
        self.ip_id = 102
        self.vlan, self.intf, self.mac = ({} for i in range(3))
        self.vlan_port, self.port = ({} for i in range(2))

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.client.switch_api_init(device)

        #Creating VLAN's
        for vlanid in xrange(10, self.maxvlan):
            self.vlan[vlanid] = self.add_vlan(device, vlanid)

        #Creating and adding interface to a vlan
        for i in self.ports:
            self.port[i] = self.select_port(device, swports[i])
        for i in self.ports[:1]:
            self.intf[i] = self.cfg_l2intf_on_port(device,
                                                   self.port[i],
                                                   mode='access')
            self.add_vlan_member(device,
                                 self.vlan[vlanid],
                                 self.intf[i])

        #Creating and adding interface to a vlan lag
        self.lag = self.add_lag(device)
        for i in self.ports[1:]:
            self.add_lag_member(device, self.lag, self.port[i])

        for i in self.ports[1:2]:
            self.intf[i] = self.cfg_l2intf_on_port(device,
                                                   self.port[i],
                                                   mode='access')
            self.add_vlan_member(device,
                                 self.vlan[vlanid],
                                 self.intf[i])

        for vlanid in range(10, self.maxvlan-1):
            for i in self.ports[:2]:
                self.mac[i, vlanid] = self.intfmac[i]
                self.add_vlan_member(device,
                                 self.vlan[vlanid],
                                 self.intf[i])
                self.add_mac_table_entry(device,
                                         self.vlan[vlanid],
                                         self.mac[i, vlanid],
                                         self.mac_type,
                                         self.intf[i])

    def runTest(self):
        print "Verify L2 packet from sending port to receiving port is " + \
              "received for the configured vlan "
        for vlanid in range(10, self.maxvlan-1):
            for send in self.ports[:1]:
                for recv in self.ports[:2]:
                    if send == recv:
                        continue
                    pkt = simple_tcp_packet(eth_dst=self.mac[recv, vlanid],
                                            eth_src=self.mac[send, vlanid],
                                            ip_dst=self.ip_dst,
                                            ip_src=self.ip_src,
                                            ip_id=self.ip_id,
                                            ip_ttl=64)

                    exp_pkt = simple_tcp_packet(eth_dst=self.mac[recv, vlanid],
                                                eth_src=self.mac[send, vlanid],
                                                ip_dst=self.ip_dst,
                                                ip_src=self.ip_src,
                                                ip_id=self.ip_id,
                                                ip_ttl=64)

                    try:
                         send_packet(self, swports[send], str(pkt))
                         verify_any_packet_any_port(self,[exp_pkt,
                                                    exp_pkt, exp_pkt, exp_pkt],
                                                    [swports[recv],
                                                    swports[recv+1],
                                                    swports[recv+2],
                                                    swports[recv+3]])
                    finally:
                        print "\tPacket from port %d to port %d for " \
                              % (send, recv) + "vlan-id %s" % (vlanid)

      #Removing interface from first vlan
        for vlanid in range(10, self.maxvlan-1):
            for i in self.ports[:2]:
                self.mac[i, vlanid] = self.intfmac[i]
                self.no_mac_address_table_entry(device,
                                                 self.vlan[vlanid],
                                                 self.mac[i, vlanid])
                self.no_vlan_member(device, self.vlan[vlanid], self.intf[i])

       #Adding interface to different vlan
        for vlanid in range(11, self.maxvlan):
            for i in self.ports[:2]:
                self.mac[i, vlanid] = self.intfmac[i]
                self.add_vlan_member(device, self.vlan[vlanid], self.intf[i])
                self.add_mac_table_entry(device,
                                         self.vlan[vlanid],
                                         self.mac[i, vlanid],
                                         self.mac_type,
                                         self.intf[i])

        print "Verify L2 packet from sending port to receiving port is " + \
              "received for the changed configured vlan "
        for vlanid in range(11, self.maxvlan):
            for send in self.ports[:1]:
                for recv in self.ports[:2]:
                    if send == recv:
                        continue
                    pkt = simple_tcp_packet(eth_dst=self.mac[recv, vlanid],
                                            eth_src=self.mac[send, vlanid],
                                            ip_dst=self.ip_dst,
                                            ip_src=self.ip_src,
                                            ip_id=self.ip_id,
                                            ip_ttl=64)

                    exp_pkt = simple_tcp_packet(eth_dst=self.mac[recv, vlanid],
                                                eth_src=self.mac[send, vlanid],
                                                ip_dst=self.ip_dst,
                                                ip_src=self.ip_src,
                                                ip_id=self.ip_id,
                                                ip_ttl=64)
                    try:
                         send_packet(self, swports[send], str(pkt))
                         verify_any_packet_any_port(self,
                                                    [exp_pkt, exp_pkt,
                                                    exp_pkt, exp_pkt],
                                                    [swports[recv],
                                                    swports[recv+1],
                                                    swports[recv+2],
                                                    swports[recv+3]])
                    finally:
                        print "\tPacket from port %d to port %d for " \
                              % (send, recv) + "vlan-id %s" % (vlanid)

        self.cleanup()
