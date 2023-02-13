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

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *

import os
import ptf.mask

from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

this_dir = os.path.dirname(os.path.abspath(__file__))

sys.path.append(os.path.join(this_dir, '../../base'))
sys.path.append(os.path.join(this_dir, '../../base/common'))

from common.utils import *
sys.path.append(os.path.join(this_dir, '../../base/api-tests'))
import api_base_tests
from api_adapter import ApiAdapter
from api_utils import *

device = 0
cpu_port = 64
swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)

if swports == []:
    swports = [x for x in range(65)]


################################################################################
@group('l2')
@group('stp')
@group('feature-tests')
@group('ent')
class L2ChangeStpVlansTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Initialize test variables"
        self.mac_type = 2
        self.maxvlan = 4
        self.intfmac = {1 : '00:11:11:11:11:11', 2 : '00:22:22:22:22:22'}
        self.ports = [1, 2]
        self.vlanmac = "00:77:66:55:44:33"
        self.ip_dst = "10.0.0.1"
        self.ip_id = 102
        self.stp_mode = 1
        self.vlan, self.intf, i_info, self.stp= ({} for i in range(4))
        self.vlan_port, self.mac, self.port = ({} for i in range(3))

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
                self.add_vlan_member(device, self.vlan[vlanid], self.intf[i])
                self.add_mac_table_entry(device,
                                         self.vlan[vlanid],
                                         self.mac[i, vlanid],
                                         self.mac_type,
                                         self.intf[i])
                self.intfmac[i] = macincrement(self.intfmac[i])

            self.stp[vlanid] = self.add_stp(device, self.stp_mode)
            self.add_stp_group_member(device,
                                      self.stp[vlanid],
                                      self.vlan[vlanid])

            for i in self.ports:
                self.set_stp_port_state(device,
                                        self.stp[vlanid],
                                        self.intf[i],
                                        3)

    def runTest(self):
        print "Verify L2 packet from sending port to receiving port is " + \
              "received after the vlans added to stp group"
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

        print "Remove VLANs from STP group"
        for vlanid in range(2, self.maxvlan):
            for i in self.ports:
                self.no_stp_group_member(device,
                                          self.stp[vlanid],
                                          self.vlan[vlanid])

        print "Add it back to a different STP group"
        for vlanid1, vlanid2 in zip(range(2, self.maxvlan),
                                    range(self.maxvlan-1, 1, -1)):
            self.add_stp_group_member(device,
                                      self.stp[vlanid1],
                                      self.vlan[vlanid2])

        print "Verify L2 packet from sending port to receiving port is " + \
              "received after the vlans removed and re-added to different " + \
              "stp group"
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
@group('stp')
@group('feature-tests')
@group('ent')
class L2ChangeStpStateTest(ApiAdapter):
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
        self.vlanid = 2
        self.stpstate = {1 : 'Disabled',
                         2 : 'Learning',
                         3 : 'Forwarding',
                         4 : 'Blocking'}
        self.portnum = 2
        self.stp_mode = 1
        self.vlan, self.intf, self.stp = ({} for i in range(3))
        self.vlan_port, self.mac, self.port = ({} for i in range(3))

        self.vlan = self.add_vlan(device, self.vlanid)

        for i in self.ports:
            self.port[i] = self.select_port(device, swports[i])
            self.intf[i] = self.cfg_l2intf_on_port(device,
                                                   self.port[i],
                                                   mode='trunk')

        for i in self.ports:
            self.add_vlan_member(device, self.vlan, self.intf[i])

        self.stp = self.add_stp(device, self.stp_mode)
        self.add_stp_group_member(device,
                                  self.stp,
                                  self.vlan)
        for i in self.ports:
            self.set_stp_port_state(device, self.stp, self.intf[i], 3)

    def runTest(self):
        for state in self.stpstate:
            for i in self.ports:
                self.add_mac_table_entry(device,
                                         self.vlan,
                                         self.intfmac[i],
                                         self.mac_type,
                                         self.intf[i])

            print "Change the state of STP to %s " % (self.stpstate[state]) + \
                  "and verify L2 packet from sending port to receiving " + \
                  "port is not received after the vlans added to stp group"

            self.set_stp_port_state(device,
                                    self.stp,
                                    self.intf[self.portnum],
                                    state)
            stp_state = self.get_stp_port_state(device,
                                    self.stp,
                                    self.intf[self.portnum])
            print "Port's stp state from driver:%d" %(stp_state)

            for send in self.ports:
                #This condition exists until JIRA is fixed.
                if (self.stpstate[state] == "Disabled" or \
                    self.stpstate[state] == "Learning" or \
                    self.stpstate[state] == "Forwarding" or \
                    self.stpstate[state] == "Blocking" ) and \
                    send == self.portnum:
                    continue
                for recv in self.ports:
                    if send == recv:
                        continue
                    pkt = simple_tcp_packet(
                        eth_dst=self.intfmac[recv],
                        eth_src=self.intfmac[send],
                        dl_vlan_enable=True,
                        vlan_vid=self.vlanid,
                        ip_dst=self.ip_dst,
                        ip_id=self.ip_id)
                    exp_pkt = simple_tcp_packet(
                        eth_dst=self.intfmac[recv],
                        eth_src=self.intfmac[send],
                        ip_dst=self.ip_dst,
                        ip_id=self.ip_id,
                        dl_vlan_enable=True,
                        vlan_vid=self.vlanid)

                    try:
                        send_packet(self, swports[send], str(pkt))
                        time.sleep(3)
                        if send == self.portnum:
                            verify_no_packet(self, exp_pkt, swports[recv])
                            print "Packet not forwarded"
                        else:
                            verify_packet(self, exp_pkt, swports[recv])
                            print "Packet forwarded"
                    finally:
                        print "\tPacket from port %d to port %d for " \
                              % (send, recv) + "vlan-id %s" % (self.vlanid)

            print "Cleaning up MAC entries"
            for i in self.ports:
                self.no_mac_address_table_entry(device,
                                                 self.vlan,
                                                 self.intfmac[i])

        self.cleanup()

################################################################################
@group('l2')
@group('stp')
@group('feature-tests')
@group('ent')
class L2EnableDisableStpTest(ApiAdapter):
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
        self.stp_mode = 1
        self.vlan, self.intf, self.stp = ({} for i in range(3))
        self.vlan_port, self.mac, self.port = ({} for i in range(3))

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
                self.add_vlan_member(device, self.vlan[vlanid], self.intf[i])
                self.add_mac_table_entry(device,
                                         self.vlan[vlanid],
                                         self.mac[i, vlanid],
                                         self.mac_type,
                                         self.intf[i])
                self.intfmac[i] = macincrement(self.intfmac[i])

    def runTest(self):
        print "Enable STP..."
        for vlanid in range(2, self.maxvlan):
            self.stp[vlanid] = self.add_stp(device, self.stp_mode)
            self.add_stp_group_member(device,
                                      self.stp[vlanid],
                                      self.vlan[vlanid])

            for i in self.ports:
                self.set_stp_port_state(device,
                                        self.stp[vlanid],
                                        self.intf[i],
                                        3)

        print "Verify L2 packet from sending port to receiving port is " + \
              "received after STP is enabled"
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

        print "Disable STP..."
        for vlanid in range(2, self.maxvlan):
            for i in self.ports:
                self.set_stp_port_state(device,
                                        self.stp[vlanid],
                                        self.intf[i],
                                        0)
            self.no_stp_group_member(device,
                                      self.stp[vlanid],
                                      self.vlan[vlanid])
            self.no_stp(device, self.stp[vlanid])

        print "Verify L2 packet from sending port to receiving port is " + \
              "received after STP is disabled"
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
