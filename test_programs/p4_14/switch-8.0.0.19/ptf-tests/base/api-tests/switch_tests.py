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
@group('ent')
class L2AccessToAccessVlanTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        self.vlan_id = 10
        self.mac_type = SWITCH_MAC_ENTRY_STATIC
        self.fp_ports = [swports[0], swports[1]]
        self.macs = ['00:22:22:22:22:22', '00:11:11:11:11:11']
        self.vlan_h = [0] * 100
        self.intf_mode = ['access', 'access']
        self.port_h = [0] * len(self.fp_ports)
        self.intf_h = [0] * len(self.fp_ports)

        self.vlan_h[self.vlan_id] = self.add_vlan(device, self.vlan_id)

        for index in range(0, len(self.fp_ports)):
          self.port_h[index] = self.select_port(device, swports[index])
          self.intf_h[index] = self.cfg_l2intf_on_port(device, self.port_h[index], self.intf_mode[index])

        for index in range(0, len(self.intf_h)):
          self.add_vlan_member(device, self.vlan_h[self.vlan_id], self.intf_h[index])

        for index in range(0, len(self.macs)):
          self.add_mac_table_entry(device,
                                   self.vlan_h[self.vlan_id],
                                   self.macs[index],
                                   self.mac_type,
                                   self.intf_h[index])

    def runTest(self):
        pkt = simple_udp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_ttl=64)

        try:
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, pkt, [swports[1]])
        finally:
            print
            print "Packet from port %s to port %s on vlan %s" % (swports[0], swports[1], self.vlan_id)

    def tearDown(self):
      self.cleanup()
      api_base_tests.ThriftInterfaceDataPlane.tearDown(self)

###############################################################################
@group('bfd')
@group('l2')
@group('maxsizes')
@group('2porttests')
@group('ent')
class L2TrunkToTrunkVlanTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        self.vlan_id = 10
        self.mac_type = SWITCH_MAC_ENTRY_STATIC
        self.fp_ports = [swports[0], swports[1]]
        self.macs = ['00:22:22:22:22:22', '00:11:11:11:11:11']
        self.intf_mode = ['trunk', 'trunk']
        self.vlan_h = [0] * 100
        self.port_h = [0] * len(self.fp_ports)
        self.intf_h = [0] * len(self.fp_ports)

        self.vlan_h[self.vlan_id] = self.add_vlan(device, self.vlan_id)

        for index in range(0, len(self.fp_ports)):
          self.port_h[index] = self.select_port(device, swports[index])
          self.intf_h[index] = self.cfg_l2intf_on_port(device, self.port_h[index], mode=self.intf_mode[index])

        for index in range(0, len(self.intf_h)):
          self.add_vlan_member(device, self.vlan_h[self.vlan_id], self.intf_h[index])

        for index in range(0, len(self.macs)):
          self.add_mac_table_entry(device,
                                   self.vlan_h[self.vlan_id],
                                   self.macs[index],
                                   self.mac_type,
                                   self.intf_h[index])

    def runTest(self):
        pkt = simple_udp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            dl_vlan_enable=True,
            vlan_vid=10,
            pktlen=104,
            ip_ttl=64)

        try:
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, pkt, [swports[1]])
        finally:
            print
            print "Packet from port %s to port %s on vlan %s" % (swports[0], swports[1], self.vlan_id)

    def tearDown(self):
      self.cleanup()
      api_base_tests.ThriftInterfaceDataPlane.tearDown(self)

###############################################################################
@group('bfd')
@group('l2')
@group('maxsizes')
@group('2porttests')
@group('ent')
class L2AccessToTrunkVlanTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        self.vlan_id = 10
        self.mac_type = SWITCH_MAC_ENTRY_STATIC
        self.fp_ports = [swports[0], swports[1]]
        self.macs = ['00:22:22:22:22:22', '00:11:11:11:11:11']
        self.intf_mode = ['access', 'trunk']
        self.vlan_h = [0] * 100
        self.port_h = [0] * len(self.fp_ports)
        self.intf_h = [0] * len(self.fp_ports)

        self.vlan_h[self.vlan_id] = self.add_vlan(device, self.vlan_id)

        for index in range(0, len(self.fp_ports)):
          self.port_h[index] = self.select_port(device, swports[index])
          self.intf_h[index] = self.cfg_l2intf_on_port(device, self.port_h[index], mode=self.intf_mode[index])

        for index in range(0, len(self.intf_h)):
          self.add_vlan_member(device, self.vlan_h[self.vlan_id], self.intf_h[index])

        for index in range(0, len(self.macs)):
          self.add_mac_table_entry(device,
                                   self.vlan_h[self.vlan_id],
                                   self.macs[index],
                                   self.mac_type,
                                   self.intf_h[index])

    def runTest(self):
        pkt = simple_udp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_ttl=64)
        exp_pkt = simple_udp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            dl_vlan_enable=True,
            vlan_vid=10,
            pktlen=104,
            ip_ttl=64)

        try:
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])
        finally:
            print
            print "Packet from port %s to port %s on vlan %s" % (swports[0], swports[1], self.vlan_id)

    def tearDown(self):
      self.cleanup()
      api_base_tests.ThriftInterfaceDataPlane.tearDown(self)

###############################################################################
@group('bfd')
@group('l2')
@group('maxsizes')
@group('2porttests')
@group('jumbomtu')
@group('ent')
class L2AccessToTrunkVlanJumboTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Sending L2 packet - port %d" % swports[
            0], " -> port %d" % swports[1], " [trunk vlan=10])"
        vlan = self.client.switch_api_vlan_create(device, 10)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        self.client.switch_api_port_mtu_set(device, port1, 9216, 9216)
        self.client.switch_api_port_mtu_set(device, port2, 9216, 9216)

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_TRUNK)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)

        switch_api_mac_table_entry_create(
            self, device, vlan, '00:11:11:11:11:11', 2, if2)
        switch_api_mac_table_entry_create(
            self, device, vlan, '00:22:22:22:22:22', 2, if1)

        pkt = simple_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_id=102,
            ip_ttl=64,
            pktlen=9100)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_id=102,
            ip_ttl=64,
            dl_vlan_enable=True,
            vlan_vid=10,
            pktlen=9104)
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
@group('ent')
class L2TrunkToAccessVlanTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        self.vlan_id = 10
        self.mac_type = SWITCH_MAC_ENTRY_STATIC
        self.fp_ports = [swports[0], swports[1]]
        self.macs = ['00:22:22:22:22:22', '00:11:11:11:11:11']
        self.intf_mode = ['trunk', 'access']
        self.vlan_h = [0] * 100
        self.port_h = [0] * len(self.fp_ports)
        self.intf_h = [0] * len(self.fp_ports)

        self.vlan_h[self.vlan_id] = self.add_vlan(device, self.vlan_id)

        for index in range(0, len(self.fp_ports)):
          self.port_h[index] = self.select_port(device, swports[index])
          self.intf_h[index] = self.cfg_l2intf_on_port(device, self.port_h[index], mode=self.intf_mode[index])

        for index in range(0, len(self.intf_h)):
          self.add_vlan_member(device, self.vlan_h[self.vlan_id], self.intf_h[index])

        for index in range(0, len(self.macs)):
          self.add_mac_table_entry(device,
                                   self.vlan_h[self.vlan_id],
                                   self.macs[index],
                                   self.mac_type,
                                   self.intf_h[index])

    def runTest(self):
        pkt = simple_udp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            dl_vlan_enable=True,
            vlan_vid=10,
            pktlen=104,
            ip_ttl=64)
        exp_pkt = simple_udp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_ttl=64)

        try:
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])
        finally:
            print
            print "Packet from port %s to port %s on vlan %s" % (swports[0], swports[1], self.vlan_id)

    def tearDown(self):
      self.cleanup()
      api_base_tests.ThriftInterfaceDataPlane.tearDown(self)

###############################################################################
@group('bfd')
@group('l2')
@group('maxsizes')
@group('ent')
class L2StaticMacMoveTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Sending packet port %d" % swports[1], "-> port %d" % swports[
            2], "(00:22:22:22:22:22 -> 00:11:11:11:11:11)"
        vlan = self.client.switch_api_vlan_create(device, 10)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if3 = self.client.switch_api_interface_create(device, i_info3)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)
        self.client.switch_api_vlan_member_add(device, vlan, if3)

        switch_api_mac_table_entry_create(
            self, device, vlan, '00:11:11:11:11:11', 2, if2)
        switch_api_mac_table_entry_create(
            self, device, vlan, '00:22:22:22:22:22', 2, if1)

        pkt = simple_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_id=103,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_id=103,
            ip_ttl=64)
        try:
            send_packet(self, swports[1], str(pkt))
            verify_packets(self, exp_pkt, [swports[2]])

            print "Moving mac (00:11:11:11:11:11) from port %d" % swports[
                2], " to port %d" % swports[3]
            print "Sending packet port %d" % swports[1], "-> port %d" % swports[
                3], " (00:22:22:22:22:22 -> 00:11:11:11:11:11)"

            switch_api_mac_table_entry_update(
                self, device, vlan, '00:11:11:11:11:11', 2, if3)
            send_packet(self, swports[1], str(pkt))
            verify_packets(self, exp_pkt, [swports[3]])
        finally:
            switch_api_mac_table_entry_delete(self, device, vlan,
                                                          '00:11:11:11:11:11')
            switch_api_mac_table_entry_delete(self, device, vlan,
                                                          '00:22:22:22:22:22')

            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)
            self.client.switch_api_vlan_member_remove(device, vlan, if3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_vlan_delete(device, vlan)


###############################################################################
@group('bfd')
@group('l2')
@group('learn')
@group('maxsizes')
@group('ent')
class L2MacLearnTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        vlan = self.client.switch_api_vlan_create(device, 10)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if3 = self.client.switch_api_interface_create(device, i_info3)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)
        self.client.switch_api_vlan_member_add(device, vlan, if3)

        switch_api_mac_table_entry_create(
            self, device, vlan, '00:33:33:33:33:33', 2, if3)
        pkt1 = simple_tcp_packet(
            eth_dst='00:33:33:33:33:33',
            eth_src='00:11:11:11:11:11',
            ip_dst='10.0.0.1',
            ip_id=104,
            ip_ttl=64)
        pkt2 = simple_tcp_packet(
            eth_dst='00:33:33:33:33:33',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_id=104,
            ip_ttl=64)
        pkt3 = simple_tcp_packet(
            eth_src='00:11:11:11:11:11',
            eth_dst='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_id=104,
            ip_ttl=64)

        try:
            print "Sending packet port %d" % swports[1], "-> port %d" % swports[
                3], " (00:11:11:11:11:11 -> 00:33:33:33:33:33)"
            send_packet(self, swports[1], str(pkt1))
            verify_packets(self, pkt1, [swports[3]])
            time.sleep(3)
            print "Sending packet port %d" % swports[
                2], " -> port %d" % swports[
                    3], "  (00:22:22:22:22:22 -> 00:33:33:33:33:33)"
            send_packet(self, swports[2], str(pkt2))
            verify_packets(self, pkt2, [swports[3]])
            time.sleep(3)

            print "Sending packet port %d" % swports[
                1], " -> port %d" % swports[
                    2], " (00:11:11:11:11:11 -> 00:22:22:22:22:22)"
            send_packet(self, swports[3], str(pkt3))
            verify_packets(self, pkt3, [swports[2]])
        finally:
            self.client.switch_api_mac_table_entry_flush(
                device, SWITCH_MAC_FLUSH_TYPE_ALL, 0x0, 0x0)

            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)
            self.client.switch_api_vlan_member_remove(device, vlan, if3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_vlan_delete(device, vlan)


###############################################################################
@group('bfd')
@group('l2')
@group('learn')
@group('maxsizes')
@group('ent')
class L2DynamicMacMoveTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        vlan = self.client.switch_api_vlan_create(device, 10)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if3 = self.client.switch_api_interface_create(device, i_info3)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)
        self.client.switch_api_vlan_member_add(device, vlan, if3)

        pkt1 = simple_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_id=104,
            ip_ttl=64)

        pkt2 = simple_tcp_packet(
            eth_src='00:11:11:11:11:11',
            eth_dst='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_id=104,
            ip_ttl=64)

        try:
            print "Sending packet port %d" % swports[
                1], " -> port %d" % swports[
                    2], " (00:22:22:22:22:22 -> 00:11:11:11:11:11)"
            send_packet(self, swports[1], str(pkt1))
            verify_packets(self, pkt1, [swports[2], swports[3]])
            time.sleep(3)

            print "Sending packet port %d" % swports[
                2], " -> port %d" % swports[
                    1], " (00:11:11:11:11:11 -> 00:22:22:22:22:22)"
            send_packet(self, swports[2], str(pkt2))
            verify_packets(self, pkt2, [swports[1]])
            time.sleep(3)

            print "Moving mac (00:22:22:22:22:22) from port %d" % swports[
                1], " to port %d" % swports[3], " "
            print "Sending packet port %d" % swports[
                3], "  -> port %d" % swports[
                    2], " (00:22:22:22:22:22 -> 00:11:11:11:11:11)"
            send_packet(self, swports[3], str(pkt1))
            verify_packets(self, pkt1, [swports[2]])
            time.sleep(3)

            print "Sending packet port %d" % swports[
                2], " -> port %d" % swports[
                    3], "  (00:11:11:11:11:11 -> 00:22:22:22:22:22)"
            send_packet(self, swports[2], str(pkt2))
            verify_packets(self, pkt2, [swports[3]])
        finally:
            self.client.switch_api_mac_table_entry_flush(
                device, SWITCH_MAC_FLUSH_TYPE_ALL, 0x0, 0x0)

            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)
            self.client.switch_api_vlan_member_remove(device, vlan, if3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_vlan_delete(device, vlan)


###############################################################################
@group('bfd')
@group('l2')
@group('learn')
@group('maxsizes')
@group('ent')
class L2DynamicLearnAgeTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        vlan = self.client.switch_api_vlan_create(device, 10)
        self.client.switch_api_vlan_aging_interval_set(device, vlan, 15000)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if3 = self.client.switch_api_interface_create(device, i_info3)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)
        self.client.switch_api_vlan_member_add(device, vlan, if3)

        pkt1 = simple_tcp_packet(
            eth_dst='00:66:66:66:66:66',
            eth_src='00:77:77:77:77:77',
            ip_dst='10.0.0.1',
            ip_id=115,
            ip_ttl=64)

        pkt2 = simple_tcp_packet(
            eth_src='00:66:66:66:66:66',
            eth_dst='00:77:77:77:77:77',
            ip_dst='10.0.0.1',
            ip_id=115,
            ip_ttl=64)

        try:
            print "Sending packet port %d" % swports[
                1], " -> port %d" % swports[2], ", port %d" % swports[
                    3], "  (00:77:77:77:77:77 -> 00:66:66:66:66:66)"
            send_packet(self, swports[1], str(pkt1))
            verify_packets(self, pkt1, [swports[2], swports[3]])

            # allow it to learn. Next set of packets should be unicast
            time.sleep(5)

            print "Sending packet port %d" % swports[
                2], " -> port %d" % swports[
                    1], " (00:66:66:66:66:66 -> 00:77:77:77:77:77)"
            send_packet(self, swports[2], str(pkt2))
            verify_packets(self, pkt2, [swports[1]])

            # allow it to age. Next set of packets should be flooded
            time.sleep(30)

            print "Sending packet port %d" % swports[
                2], " -> port %d" % swports[
                    1], ", 3 (00:66:66:66:66:66 -> 00:77:77:77:77:77)"
            send_packet(self, swports[2], str(pkt2))
            verify_packets(self, pkt2, [swports[1], swports[3]])

        finally:
            self.client.switch_api_mac_table_entry_flush(
                device, SWITCH_MAC_FLUSH_TYPE_ALL, 0x0, 0x0)

            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)
            self.client.switch_api_vlan_member_remove(device, vlan, if3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_vlan_delete(device, vlan)


###############################################################################
@group('bfd')
@group('l2')
@group('flood')
@group('maxsizes')
@group('ent')
class L2FloodTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        vlan = self.client.switch_api_vlan_create(device, 10)
        self.client.switch_api_vlan_learning_set(device, vlan, False)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if3 = self.client.switch_api_interface_create(device, i_info3)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)
        self.client.switch_api_vlan_member_add(device, vlan, if3)

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
        try:
            print "Sending packets from port %d to %d, %d" % (
                swports[1], swports[2], swports[3])
            send_packet(self, swports[1], str(pkt))
            verify_packets(self, exp_pkt, [swports[2], swports[3]])
            print "Sending packets from port %d to %d, %d" % (
                swports[2], swports[1], swports[3])
            send_packet(self, swports[2], str(pkt))
            verify_packets(self, exp_pkt, [swports[1], swports[3]])
            print "Sending packets from port %d to %d, %d" % (
                swports[3], swports[1], swports[2])
            send_packet(self, swports[3], str(pkt))
            verify_packets(self, exp_pkt, [swports[1], swports[2]])
        finally:
            self.client.switch_api_mac_table_entry_flush(
                device, SWITCH_MAC_FLUSH_TYPE_ALL, 0x0, 0x0)
            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)
            self.client.switch_api_vlan_member_remove(device, vlan, if3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_vlan_delete(device, vlan)


###############################################################################
@group('bfd')
@group('l2')
@group('maxsizes')
@group('ent')
class L2LagTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        vlan = self.client.switch_api_vlan_create(device, 10)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port5 = self.client.switch_api_port_id_to_handle_get(device, swports[5])
        port6 = self.client.switch_api_port_id_to_handle_get(device, swports[6])
        port7 = self.client.switch_api_port_id_to_handle_get(device, swports[7])
        port8 = self.client.switch_api_port_id_to_handle_get(device, swports[8])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        lag = self.client.switch_api_lag_create(device)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port5)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port6)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port7)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port8)
        i_info2 = switcht_interface_info_t(
            handle=lag, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)

        switch_api_mac_table_entry_create(
            self, device, vlan, '00:11:11:11:11:11', 2, if2)
        switch_api_mac_table_entry_create(
            self, device, vlan, '00:22:22:22:22:22', 2, if1)

        try:
            count = [0, 0, 0, 0]
            dst_ip = int(socket.inet_aton('10.10.10.1').encode('hex'), 16)
            src_ip = int(socket.inet_aton('192.168.8.1').encode('hex'), 16)
            max_itrs = 200
            for i in range(0, max_itrs):
                dst_ip_addr = socket.inet_ntoa(
                    format(dst_ip, 'x').zfill(8).decode('hex'))
                src_ip_addr = socket.inet_ntoa(
                    format(src_ip, 'x').zfill(8).decode('hex'))
                pkt = simple_tcp_packet(
                    eth_dst='00:11:11:11:11:11',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src=src_ip_addr,
                    ip_id=109,
                    ip_ttl=64)

                exp_pkt = simple_tcp_packet(
                    eth_dst='00:11:11:11:11:11',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src=src_ip_addr,
                    ip_id=109,
                    ip_ttl=64)

                send_packet(self, swports[1], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self, [exp_pkt, exp_pkt, exp_pkt, exp_pkt],
                    [swports[5], swports[6], swports[7], swports[8]], timeout=2)
                count[rcv_idx] += 1
                dst_ip += 1
                src_ip += 1

            print 'L2LagTest:', count
            for i in range(0, 4):
                self.assertTrue((count[i] >= ((max_itrs / 4) * 0.6)),
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
            print "Sending packet port %d" % swports[
                5], "  (lag member) -> port %d" % swports[1], ""
            send_packet(self, swports[5], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])
            print "Sending packet port %d" % swports[
                6], " (lag member) -> port %d" % swports[1], ""
            send_packet(self, swports[6], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])
            print "Sending packet port %d" % swports[
                7], " (lag member) -> port %d" % swports[1], ""
            send_packet(self, swports[7], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])
            print "Sending packet port %d" % swports[
                8], " (lag member) -> port %d" % swports[1], ""
            send_packet(self, swports[8], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])
        finally:
            switch_api_mac_table_entry_delete(self, device, vlan,
                                                          '00:22:22:22:22:22')
            switch_api_mac_table_entry_delete(self, device, vlan,
                                                          '00:11:11:11:11:11')

            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)

            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port5)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port6)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port7)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port8)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_lag_delete(device, lag)
            self.client.switch_api_vlan_delete(device, vlan)


###############################################################################
@group('bfd')
@group('l2')
@group('stp')
@group('maxsizes')
@group('2porttests')
@group('ent')
class L2StpTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print

        vlan1 = self.client.switch_api_vlan_create(device, 10)
        vlan2 = self.client.switch_api_vlan_create(device, 20)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if3 = self.client.switch_api_interface_create(device, i_info3)

        i_info4 = switcht_interface_info_t(
            handle=port4, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if4 = self.client.switch_api_interface_create(device, i_info4)

        self.client.switch_api_vlan_member_add(device, vlan1, if1)
        self.client.switch_api_vlan_member_add(device, vlan1, if2)
        self.client.switch_api_vlan_member_add(device, vlan2, if3)
        self.client.switch_api_vlan_member_add(device, vlan2, if4)

        stp = self.client.switch_api_stp_group_create(device=0, stp_mode=1)
        self.client.switch_api_stp_group_member_add(device, stp, vlan1)
        self.client.switch_api_stp_group_member_add(device, stp, vlan2)
        self.client.switch_api_stp_port_state_set(
            device=0,
            stp_handle=stp,
            intf_handle=if1,
            stp_state=SWITCH_PORT_STP_STATE_FORWARDING)
        self.client.switch_api_stp_port_state_set(
            device=0,
            stp_handle=stp,
            intf_handle=if2,
            stp_state=SWITCH_PORT_STP_STATE_FORWARDING)
        self.client.switch_api_stp_port_state_set(
            device=0,
            stp_handle=stp,
            intf_handle=if3,
            stp_state=SWITCH_PORT_STP_STATE_FORWARDING)
        self.client.switch_api_stp_port_state_set(
            device=0,
            stp_handle=stp,
            intf_handle=if4,
            stp_state=SWITCH_PORT_STP_STATE_FORWARDING)

        switch_api_mac_table_entry_create(
            self, device, vlan1, '00:11:11:11:11:11', 2, if2)
        switch_api_mac_table_entry_create(
            self, device, vlan1, '00:22:22:22:22:22', 2, if1)
        switch_api_mac_table_entry_create(
            self, device, vlan2, '00:33:33:33:33:33', 2, if4)
        switch_api_mac_table_entry_create(
            self, device, vlan2, '00:44:44:44:44:44', 2, if3)

        pkt = simple_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_id=113,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_id=113,
            ip_ttl=64)

        pkt2 = simple_tcp_packet(
            eth_dst='00:33:33:33:33:33',
            eth_src='00:44:44:44:44:44',
            ip_dst='11.0.0.1',
            ip_id=113,
            ip_ttl=64)
        exp_pkt2 = simple_tcp_packet(
            eth_dst='00:33:33:33:33:33',
            eth_src='00:44:44:44:44:44',
            ip_dst='11.0.0.1',
            ip_id=113,
            ip_ttl=64)
        try:
            print "Sending packet port %d" % swports[
                0], " (forwarding)-> port %d" % swports[
                    1], " (192.168.0.1 -> 10.0.0.1 [id = 101])"
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])

            print "Sending packet port %d" % swports[
                0], " (forwarding) -> port %d" % swports[
                    3], " (192.168.0.1 -> 11.0.0.1 [id = 101])"
            send_packet(self, swports[2], str(pkt2))
            verify_packet(self, exp_pkt2, swports[3])

            self.client.switch_api_stp_port_state_set(
                device=0,
                stp_handle=stp,
                intf_handle=if3,
                stp_state=SWITCH_PORT_STP_STATE_BLOCKING)
            print "Sending packet port %d" % swports[
                0], " (blocking) -> port %d" % swports[
                    3], " (192.168.0.1 -> 11.0.0.1 [id = 101])"
            send_packet(self, swports[2], str(pkt2))
            verify_packets(self, exp_pkt2, [])

        finally:
            switch_api_mac_table_entry_delete(self, device, vlan1,
                                                          '00:11:11:11:11:11')
            switch_api_mac_table_entry_delete(self, device, vlan1,
                                                          '00:22:22:22:22:22')
            switch_api_mac_table_entry_delete(self, device, vlan2,
                                                          '00:33:33:33:33:33')
            switch_api_mac_table_entry_delete(self, device, vlan2,
                                                          '00:44:44:44:44:44')

            self.client.switch_api_stp_port_state_set(
                device=0, stp_handle=stp, intf_handle=if1, stp_state=0)
            self.client.switch_api_stp_port_state_set(
                device=0, stp_handle=stp, intf_handle=if2, stp_state=0)
            self.client.switch_api_stp_port_state_set(
                device=0, stp_handle=stp, intf_handle=if3, stp_state=0)
            self.client.switch_api_stp_port_state_set(
                device=0, stp_handle=stp, intf_handle=if4, stp_state=0)

            self.client.switch_api_stp_group_member_remove(device, stp, vlan1)
            self.client.switch_api_stp_group_member_remove(device, stp, vlan2)
            self.client.switch_api_stp_group_delete(device, stp)

            self.client.switch_api_vlan_member_remove(device, vlan1, if1)
            self.client.switch_api_vlan_member_remove(device, vlan1, if2)
            self.client.switch_api_vlan_member_remove(device, vlan2, if3)
            self.client.switch_api_vlan_member_remove(device, vlan2, if4)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)
            self.client.switch_api_interface_delete(device, if4)

            self.client.switch_api_vlan_delete(device, vlan1)
            self.client.switch_api_vlan_delete(device, vlan2)


###############################################################################
@group('bfd')
@group('l2')
@group('stp')
@group('flood')
@group('maxsizes')
@group('ent')
class L2StpEgressBlockingTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Sending packet port %d" % swports[
            0], " (forwarding)-> port %d" % swports[
                1], " (192.168.0.1 -> 10.0.0.1 [id = 101])"

        vlan = self.client.switch_api_vlan_create(device, 10)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[2])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        i_info2 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_TRUNK)
        if3 = self.client.switch_api_interface_create(device, i_info2)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)
        self.client.switch_api_vlan_member_add(device, vlan, if3)

        stp = self.client.switch_api_stp_group_create(device=0, stp_mode=1)
        self.client.switch_api_stp_group_member_add(device, stp, vlan)
        self.client.switch_api_stp_port_state_set(
            device=0,
            stp_handle=stp,
            intf_handle=if1,
            stp_state=SWITCH_PORT_STP_STATE_FORWARDING)
        self.client.switch_api_stp_port_state_set(
            device=0,
            stp_handle=stp,
            intf_handle=if2,
            stp_state=SWITCH_PORT_STP_STATE_FORWARDING)
        self.client.switch_api_stp_port_state_set(
            device=0,
            stp_handle=stp,
            intf_handle=if3,
            stp_state=SWITCH_PORT_STP_STATE_FORWARDING)

        switch_api_mac_table_entry_create(
            self, device, vlan, '00:11:11:11:11:11', 2, if2)
        switch_api_mac_table_entry_create(
            self, device, vlan, '00:22:22:22:22:22', 2, if1)

        pkt1 = simple_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_id=113,
            ip_ttl=64)
        exp_pkt1 = simple_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_id=113,
            ip_ttl=64)
        pkt2 = simple_tcp_packet(
            eth_dst='00:11:11:11:11:12',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_id=113,
            ip_ttl=64)
        exp_pkt2 = simple_tcp_packet(
            eth_dst='00:11:11:11:11:12',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.0.0.1',
            ip_id=113,
            ip_ttl=64)
        exp_vlan_pkt2 = simple_tcp_packet(
            eth_dst='00:11:11:11:11:12',
            eth_src='00:22:22:22:22:22',
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_dst='10.0.0.1',
            pktlen=104,
            ip_id=113,
            ip_ttl=64)
        try:
            #case 1: send known unicast packet from port1 to port2
            #        and both the ports are in forwarding mode
            send_packet(self, swports[0], str(pkt1))
            verify_packets(self, exp_pkt1, [swports[1]])

            #case 2: send unknown unicast packet from port1 and expect to
            #        flood on all ports except port 1(pruning).
            #        All 3 ports are in forwarding mode.
            send_packet(self, swports[0], str(pkt2))
            verify_each_packet_on_each_port(self,
                                            [exp_pkt2, exp_vlan_pkt2],
                                            [swports[1], swports[2]])

            # set port 2 in blocking mode
            self.client.switch_api_stp_port_state_set(
                device=0,
                stp_handle=stp,
                intf_handle=if2,
                stp_state=SWITCH_PORT_STP_STATE_BLOCKING)

            #case 3: send unknown unicast packet from port1 and expect to
            #        flood only on port 3. port 1 will be pruned and
            #        port 2 is blocked.
            send_packet(self, swports[0], str(pkt2))
            verify_packets(self, exp_vlan_pkt2, [swports[2]])

            # set port 3 in blocking mode
            self.client.switch_api_stp_port_state_set(
                device=0,
                stp_handle=stp,
                intf_handle=if3,
                stp_state=SWITCH_PORT_STP_STATE_BLOCKING)

            #case 4: send unknown unicast packet from port1 and expect no
            #        packets. port 1 will be pruned and
            #        port 2 and port 3 are blocked.
            send_packet(self, swports[0], str(pkt2))
            verify_no_other_packets(self, timeout=1)

            # set port 2 in forwarding mode
            self.client.switch_api_stp_port_state_set(
                device=0,
                stp_handle=stp,
                intf_handle=if2,
                stp_state=SWITCH_PORT_STP_STATE_FORWARDING)

            #case 5: send unknown unicast packet from port1 and expect to
            #        flood only on port 2. port 1 will be pruned and
            #        port 3 is blocked.
            send_packet(self, swports[0], str(pkt2))
            verify_packets(self, exp_pkt2, [swports[1]])

            # set port 1 in blocking mode
            self.client.switch_api_stp_port_state_set(
                device=0,
                stp_handle=stp,
                intf_handle=if1,
                stp_state=SWITCH_PORT_STP_STATE_BLOCKING)

            #case 6: send unknown unicast packet from port1. port1 is in
            #        blocking and packet will be dropped in ingress
            send_packet(self, swports[0], str(pkt2))
            verify_no_other_packets(self, timeout=1)
        finally:
            self.client.switch_api_stp_port_state_set(
                device=0, stp_handle=stp, intf_handle=if1, stp_state=0)
            self.client.switch_api_stp_port_state_set(
                device=0, stp_handle=stp, intf_handle=if2, stp_state=0)
            self.client.switch_api_stp_port_state_set(
                device=0, stp_handle=stp, intf_handle=if3, stp_state=0)

            self.client.switch_api_stp_group_member_remove(device, stp, vlan)
            self.client.switch_api_stp_group_delete(device, stp)

            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)
            self.client.switch_api_vlan_member_remove(device, vlan, if3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_vlan_delete(device, vlan)

###############################################################################
@group('l3')
@group('ipv4')
@group('maxsizes')
@group('clpm')
@group('2porttests')
@group('ent')
class L3IPv4HostTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Sending packet port %d" % swports[0], " -> port %d" % swports[
            1], " (192.168.0.1 -> 10.10.10.1 [id = 105])"

        self.vrf_id = 10
        self.fp_ports = [swports[0], swports[1]]
        self.port_h = [0] * len(self.fp_ports)
        self.rif_h = [0] * len(self.fp_ports)
        self.rif_ip = ['192.168.0.2', '10.0.0.2']
        self.host_ip = '10.10.10.1'
        self.host_mac = '00:11:22:33:44:55'

        self.vrf = self.add_vrf(device, 2)
        self.rmac = self.add_rmac(device, '00:77:66:55:44:33')

        for index in range(0, len(self.fp_ports)):
          self.port_h[index] = self.select_port(device, swports[index])
          self.rif_h[index] = self.create_l3_rif(
                                   device,
                                   self.vrf,
                                   self.rmac,
                                   self.port_h[index],
                                   self.rif_ip[index])

        self.nhop1 = self.add_l3_nhop(device, self.rif_h[1], self.host_ip, self.host_mac)
        self.add_static_route(device, self.vrf, self.host_ip, self.nhop1)

    def runTest(self):

        # send the test packet(s)
        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:11:22:33:44:55',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_ttl=63)
        try:
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

        finally:
            print
            print "Packet from port %s to port %s on vlan %s" % (swports[0], swports[1], self.vrf_id)

    def tearDown(self):
      self.cleanup()
      api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('l3')
@group('ipv4')
@group('maxsizes')
@group('clpm')
@group('2porttests')
@group('jumbomtu')
@group('ent')
class L3IPv4HostJumboTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Sending packet port %d" % swports[0], " -> port %d" % swports[
            1], " (192.168.0.1 -> 10.10.10.1 [id = 105])"

        self.vrf_id = 10
        self.fp_ports = [swports[0], swports[1]]
        self.port_h = [0] * len(self.fp_ports)
        self.rif_h = [0] * len(self.fp_ports)
        self.rif_ip = ['192.168.0.2', '10.0.0.2']
        self.host_ip = '10.10.10.1'
        self.host_mac = '00:11:22:33:44:55'

        self.vrf = self.add_vrf(device, 2)
        self.rmac = self.add_rmac(device, '00:77:66:55:44:33')

        for index in range(0, len(self.fp_ports)):
          self.port_h[index] = self.select_port(device, swports[index])
          self.rif_h[index] = self.create_l3_rif(
                                   device,
                                   self.vrf,
                                   self.rmac,
                                   self.port_h[index],
                                   self.rif_ip[index])

        self.nhop1 = self.add_l3_nhop(device, self.rif_h[1], self.host_ip, self.host_mac)
        self.add_static_route(device, self.vrf, self.host_ip, self.nhop1)

    def runTest(self):
        # send the test packet(s)
        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            pktlen=9100,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:11:22:33:44:55',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            pktlen=9100,
            ip_ttl=63)
        try:
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

        finally:
            print
            print "Packet from port %s to port %s on vlan %s" % (swports[0], swports[1], self.vrf_id)

    def tearDown(self):
      self.cleanup()
      api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('l3')
@group('ipv4')
@group('clpm')
@group('maxsizes')
@group('2porttests')
@group('ent')
class L3IPv4SubIntfHostTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Sending packet port %d" % swports[0], " -> port %d" % swports[
            1], " (192.168.0.1 -> 10.10.10.1 [id = 105])"
        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
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

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        self.client.switch_api_port_bind_mode_set(
            device, port2, SWITCH_PORT_BIND_MODE_PORT_VLAN)
        i_info2 = switcht_interface_info_t(
            handle=port2,
            vlan=10,
            type=SWITCH_INTERFACE_TYPE_PORT_VLAN,
            rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf,
                                                        i_ip2)

        # Add a static route
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.1',
            prefix_length=32)
        nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip3, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(device, vrf, i_ip3, nhop1)

        i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='20.20.20.1',
            prefix_length=32)
        nhop2, neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, rif1, i_ip4, '00:11:22:33:44:56')
        self.client.switch_api_l3_route_add(device, vrf, i_ip4, nhop2)

        # send the test packet(s)
        try:
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                dl_vlan_enable=True,
                vlan_vid=10,
                pktlen=104,
                ip_ttl=63)
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:11:11:11:11:11',
                ip_dst='20.20.20.1',
                ip_src='10.0.0.1',
                dl_vlan_enable=True,
                vlan_vid=10,
                pktlen=104,
                ip_id=105,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:22:33:44:56',
                eth_src='00:77:66:55:44:33',
                ip_dst='20.20.20.1',
                ip_src='10.0.0.1',
                ip_id=105,
                ip_ttl=63)
            send_packet(self, swports[1], str(pkt))
            verify_packets(self, exp_pkt, [swports[0]])

        finally:
            self.client.switch_api_neighbor_delete(device, neighbor1)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip3, nhop1)
            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_neighbor_delete(device, neighbor2)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip4, nhop2)
            self.client.switch_api_nhop_delete(device, nhop2)

            self.client.switch_api_l3_interface_address_delete(device, rif1,
                                                               vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(device, rif2,
                                                               vrf, i_ip2)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_port_bind_mode_set(
                device, port2, SWITCH_PORT_BIND_MODE_PORT)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('l3')
@group('ipv4')
@group('clpm')
@group('maxsizes')
@group('ent')
class L3IPv4HostModifyTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Sending packet port %d" % swports[1], " -> port %d" % swports[
            2], " (192.168.0.1 -> 10.10.10.1 [id = 105] route add)"
        print "Sending packet port %d" % swports[1], " -> port %d" % swports[
            3], "  (192.168.0.1 -> 10.10.10.1 [id = 105] route update)"
        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            v4_unicast_enabled=True,
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

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            v4_unicast_enabled=True,
            rmac_handle=rmac)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf,
                                                        i_ip2)

        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            v4_unicast_enabled=True,
            rmac_handle=rmac)
        rif3 = self.client.switch_api_rif_create(0, rif_info3)
        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='11.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif3, vrf,
                                                        i_ip3)

        i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.1',
            prefix_length=32)
        nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip4, '00:22:22:22:22:22')
        nhop2, neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, rif3, i_ip4, '00:33:33:33:33:33')
        self.client.switch_api_l3_route_add(device, vrf, i_ip4, nhop1)

        try:
            # send the test packet(s)
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)

            exp_pkt = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=63)
            send_packet(self, swports[1], str(pkt))
            verify_packets(self, exp_pkt, [swports[2]])

            self.client.switch_api_l3_route_update(device, vrf, i_ip4, nhop2)

            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:33:33:33:33:33',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=63)
            send_packet(self, swports[1], str(pkt))
            verify_packets(self, exp_pkt, [swports[3]])

        finally:
            self.client.switch_api_l3_route_delete(device, vrf, i_ip4, nhop2)
            self.client.switch_api_neighbor_delete(device, neighbor2)
            self.client.switch_api_nhop_delete(device, nhop2)

            self.client.switch_api_neighbor_delete(device, neighbor1)
            self.client.switch_api_nhop_delete(device, nhop1)

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


###############################################################################
@group('l3')
@group('ipv4')
@group('clpm')
@group('maxsizes')
@group('2porttests')
@group('ent')
class L3IPv4LpmTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Sending packet port %d" % swports[0], " -> port %d" % swports[
            1], " (192.168.0.1 -> 10.0.0.1 [id = 105])"
        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
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

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf,
                                                        i_ip2)

        # Add a static route
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.0',
            prefix_length=24)
        nhop, neighbor = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip3, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(device, vrf, i_ip3, nhop)

        # send the test packet(s)
        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:11:22:33:44:55',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_ttl=63)
        try:
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])
        finally:
            self.client.switch_api_neighbor_delete(device, neighbor)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip3, nhop)
            self.client.switch_api_nhop_delete(device, nhop)

            self.client.switch_api_l3_interface_address_delete(device, rif1,
                                                               vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(device, rif2,
                                                               vrf, i_ip2)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('l3')
@group('ipv4')
@group('2porttests')
@group('clpm')
@group('ent')
class L3IPv4LookupTest(api_base_tests.ThriftInterfaceDataPlane):
    def testLookup(self, vrf, ip_key, exp_nhop):
        result_nhop = self.client.switch_api_l3_route_lookup(device, vrf,
                                                             ip_key)
        self.assertTrue(result_nhop == exp_nhop)

    def runTest(self):
        print
        print "IPv4 FIB lookup test -- both exact match and LPM -- in switchAPI."
        print "The lookup is purely done in control plane (switchAPI), not using dataplane tables."

        vrf = self.client.switch_api_vrf_create(device, 2)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=1)
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

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=1)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf,
                                                        i_ip2)

        nhop_default_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='11.11.11.11', prefix_length=32)
        nhop_default = switch_api_nhop_create(self, device, rif2, nhop_default_ip)
        nhop_classA_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='12.12.12.12', prefix_length=32)
        nhop_classA = switch_api_nhop_create(self, device, rif2, nhop_classA_ip)
        nhop_classB_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='13.13.13.13', prefix_length=32)
        nhop_classB = switch_api_nhop_create(self, device, rif2, nhop_classB_ip)
        nhop_classB23_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='14.14.14.14', prefix_length=32)
        nhop_classB23 = switch_api_nhop_create(self, device, rif2, nhop_classB23_ip)
        nhop_classC_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='15.15.15.15', prefix_length=32)
        nhop_classC = switch_api_nhop_create(self, device, rif2, nhop_classC_ip)
        nhop_host_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='10.10.10.10', prefix_length=32)
        nhop_host = switch_api_nhop_create(self, device, rif2, nhop_host_ip)

        ip_default = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='0.0.0.0', prefix_length=0)
        ip_classA = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='10.0.0.0', prefix_length=8)
        ip_classB = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.0.0',
            prefix_length=16)
        ip_classB23 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.0',
            prefix_length=23)
        ip_classC = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.0',
            prefix_length=24)
        ip_host = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.10',
            prefix_length=32)

        try:
            self.client.switch_api_l3_route_add(device, vrf, ip_default,
                                                nhop_default)
            self.testLookup(vrf, ip_host, nhop_default)

            self.client.switch_api_l3_route_add(device, vrf, ip_classA,
                                                nhop_classA)
            self.testLookup(vrf, ip_host, nhop_classA)

            self.client.switch_api_l3_route_add(device, vrf, ip_classB,
                                                nhop_classB)
            self.testLookup(vrf, ip_host, nhop_classB)

            self.client.switch_api_l3_route_add(device, vrf, ip_classB23,
                                                nhop_classB23)
            self.testLookup(vrf, ip_host, nhop_classB23)

            self.client.switch_api_l3_route_add(device, vrf, ip_classC,
                                                nhop_classC)
            self.testLookup(vrf, ip_host, nhop_classC)

            self.client.switch_api_l3_route_add(device, vrf, ip_host, nhop_host)
            self.testLookup(vrf, ip_host, nhop_host)

            self.client.switch_api_l3_route_delete(device, vrf, ip_host,
                                                   nhop_host)
            self.testLookup(vrf, ip_host, nhop_classC)

            ip_host2 = switcht_ip_addr_t(
                addr_type=SWITCH_API_IP_ADDR_V4,
                ipaddr='20.10.10.10',
                prefix_length=32)
            self.testLookup(vrf, ip_host2, nhop_default)

        finally:
            # clean up
            self.client.switch_api_l3_route_delete(device, vrf, ip_classC,
                                                   nhop_classC)
            self.client.switch_api_l3_route_delete(device, vrf, ip_classB23,
                                                   nhop_classB23)
            self.client.switch_api_l3_route_delete(device, vrf, ip_classB,
                                                   nhop_classB)
            self.client.switch_api_l3_route_delete(device, vrf, ip_classA,
                                                   nhop_classA)
            self.client.switch_api_l3_route_delete(device, vrf, ip_default,
                                                   nhop_default)

            self.client.switch_api_nhop_delete(device, nhop_default)
            self.client.switch_api_nhop_delete(device, nhop_classA)
            self.client.switch_api_nhop_delete(device, nhop_classB)
            self.client.switch_api_nhop_delete(device, nhop_classB23)
            self.client.switch_api_nhop_delete(device, nhop_classC)
            self.client.switch_api_nhop_delete(device, nhop_host)

            self.client.switch_api_l3_interface_address_delete(device, rif1,
                                                               vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(device, rif2,
                                                               vrf, i_ip2)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('l3')
@group('ipv6')
@group('2porttests')
@group('ent')
class L3IPv6LookupTest(api_base_tests.ThriftInterfaceDataPlane):
    def testLookup(self, vrf, ip_key, exp_nhop):
        result_nhop = self.client.switch_api_l3_route_lookup(device, vrf,
                                                             ip_key)
        self.assertTrue(result_nhop == exp_nhop)

    def runTest(self):
        print
        print "IPv6 FIB lookup test -- both exact match and LPM -- in switchAPI."
        print "The lookup is purely done in control plane (switchAPI), not using dataplane tables."

        vrf = self.client.switch_api_vrf_create(device, 2)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v6_unicast_enabled=1)
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
            vrf_handle=vrf,
            rmac_handle=rmac,
            v6_unicast_enabled=1)
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

        nhop_default_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6, ipaddr='2000::1', prefix_length=128)
        nhop_default = switch_api_nhop_create(self, device, rif2, nhop_default_ip)
        nhop_p8_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6, ipaddr='3000::1', prefix_length=128)
        nhop_p8 = switch_api_nhop_create(self, device, rif2, nhop_p8_ip)
        nhop_p16_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6, ipaddr='4000::1', prefix_length=128)
        nhop_p16 = switch_api_nhop_create(self, device, rif2, nhop_p16_ip)
        nhop_host_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6, ipaddr='5000::1', prefix_length=128)
        nhop_host = switch_api_nhop_create(self, device, rif2, nhop_host_ip)

        ip_default = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6, ipaddr='::', prefix_length=0)
        ip_p8 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6, ipaddr='3000::', prefix_length=8)
        ip_p16 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='3000:10::0',
            prefix_length=16)
        ip_host = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='3000:10::10',
            prefix_length=128)

        try:
            self.client.switch_api_l3_route_add(device, vrf, ip_default,
                                                nhop_default)
            self.testLookup(vrf, ip_host, nhop_default)

            self.client.switch_api_l3_route_add(device, vrf, ip_p8, nhop_p8)
            self.testLookup(vrf, ip_host, nhop_p8)

            self.client.switch_api_l3_route_add(device, vrf, ip_p16, nhop_p16)
            self.testLookup(vrf, ip_host, nhop_p16)

            self.client.switch_api_l3_route_add(device, vrf, ip_host, nhop_host)
            self.testLookup(vrf, ip_host, nhop_host)

            self.client.switch_api_l3_route_delete(device, vrf, ip_host,
                                                   nhop_host)
            self.testLookup(vrf, ip_host, nhop_p16)

            ip_host2 = switcht_ip_addr_t(
                addr_type=SWITCH_API_IP_ADDR_V6,
                ipaddr='4000:10:10:10:10:10:10:10:10:10:10:10:10:10:10:10',
                prefix_length=128)
            self.testLookup(vrf, ip_host2, nhop_default)

        finally:
            # clean up
            self.client.switch_api_l3_route_delete(device, vrf, ip_p16,
                                                   nhop_p16)
            self.client.switch_api_l3_route_delete(device, vrf, ip_p8, nhop_p8)
            self.client.switch_api_l3_route_delete(device, vrf, ip_default,
                                                   nhop_default)

            self.client.switch_api_nhop_delete(device, nhop_default)
            self.client.switch_api_nhop_delete(device, nhop_p8)
            self.client.switch_api_nhop_delete(device, nhop_p16)
            self.client.switch_api_nhop_delete(device, nhop_host)

            self.client.switch_api_l3_interface_address_delete(device, rif1,
                                                               vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(device, rif2,
                                                               vrf, i_ip2)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('l3')
@group('ipv6')
@group('maxsizes')
@group('2porttests')
@group('ent')
class L3IPv6HostTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "Sending packet port %d" % swports[0], " -> port %d" % swports[
            1], " (192.168.0.1 -> 10.10.10.1 [id = 105])"

        self.vrf_id = 10
        self.fp_ports = [swports[0], swports[1]]
        self.port_h = [0] * len(self.fp_ports)
        self.rif_h = [0] * len(self.fp_ports)
        self.rif_ip = ['2000::2', '3000::2']
        self.host_ip ='1234:5678:9abc:def0:4422:1133:5577:99aa'
        self.host_mac = '00:11:22:33:44:55'

        self.vrf = self.add_vrf(device, 2)
        self.rmac = self.add_rmac(device, '00:77:66:55:44:33')

        for index in range(0, len(self.fp_ports)):
          self.port_h[index] = self.select_port(device, swports[index])
          self.rif_h[index] = self.create_l3_rif(
                                   device,
                                   self.vrf,
                                   self.rmac,
                                   self.port_h[index],
                                   self.rif_ip[index])

        self.nhop1 = self.add_l3_nhop(device, self.rif_h[1], self.host_ip, self.host_mac, v4=False)
        self.add_static_route(device, self.vrf, self.host_ip, self.nhop1, v4=False)

    def runTest(self):
        # send the test packet(s)
        pkt = simple_tcpv6_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:22:22:22:22:22',
            ipv6_dst='1234:5678:9abc:def0:4422:1133:5577:99aa',
            ipv6_src='2000::1',
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst='00:11:22:33:44:55',
            eth_src='00:77:66:55:44:33',
            ipv6_dst='1234:5678:9abc:def0:4422:1133:5577:99aa',
            ipv6_src='2000::1',
            ipv6_hlim=63)
        try:
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

        finally:
            print
            print "Packet from port %s to port %s on vlan %s" % (swports[0], swports[1], self.vrf_id)

    def tearDown(self):
      self.cleanup()
      api_base_tests.ThriftInterfaceDataPlane.tearDown(self)

###############################################################################
@group('l3')
@group('ipv6')
@group('maxsizes')
@group('2porttests')
@group('ent')
class L3IPv6LpmTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "IPv6 Lpm Test"
        print "Sending packet port %d" % swports[0], " -> port %d" % swports[
            1], " (2000::1 -> 3000::1, routing with 3000::0/120 route"
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
        try:
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])
        finally:
            self.client.switch_api_neighbor_delete(device, neighbor)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip3, nhop)
            self.client.switch_api_nhop_delete(device, nhop)

            self.client.switch_api_l3_interface_address_delete(device, rif1,
                                                               vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(device, rif2,
                                                               vrf, i_ip2)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)



###############################################################################
@group('l3')
@group('ipv4')
@group('clpm')
@group('maxsizes')
@group('ent')
class L3IPv4DynHashEcmpTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        if ((test_param_get('target') == 'bmv2') or
            (test_param_get('target') == 'bmv2' and
             test_param_get('arch') == 'Tofino')):
            return
        print "Sending packet port %d" % swports[1], " -> port %d" % swports[
            2], " (192.168.0.1 -> 10.0.0.1 [id = 101])"
        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

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

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v4_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf,
                                                        i_ip2)

        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v4_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif3 = self.client.switch_api_rif_create(0, rif_info3)
        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='11.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif3, vrf,
                                                        i_ip3)

        i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.1',
            prefix_length=32)
        nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip4, '00:11:22:33:44:55')
        nhop2, neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, rif3, i_ip4, '00:11:22:33:44:56')

        ecmp = self.client.switch_api_ecmp_create(device)
        self.client.switch_api_ecmp_member_add(device, ecmp, 2, [nhop1, nhop2])

        self.client.switch_api_l3_route_add(device, vrf, i_ip4, ecmp)
        default_algo_res = self.client.switch_api_ipv4_hash_algorithm_get(device)
        self.assertTrue(default_algo_res.status==0)
        default_input_fields_res = self.client.switch_api_ipv4_hash_input_fields_get(device)
        self.assertTrue(default_input_fields_res.status==0)
        default_attr_res = self.client.switch_api_ipv4_hash_input_fields_attribute_get(device, default_input_fields_res.fields)
        self.assertTrue(default_attr_res.status==0)

        try:
            self.client.switch_api_ipv4_hash_input_fields_set(device, SWITCH_HASH_IPV4_INPUT_SIP_DIP_PROT_SP_DP)

            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=106,
                ip_ttl=64)
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
            send_packet(self, swports[1], str(pkt))
            rcv_port1 = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                       [swports[2], swports[3]], timeout=10)

            self.client.switch_api_ipv4_hash_input_fields_set(device, SWITCH_HASH_IPV4_INPUT_PROT_DP_DIP_SP_SIP)

            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=106,
                ip_ttl=64)
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
            send_packet(self, swports[1], str(pkt))
            rcv_port2 = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                       [swports[2], swports[3]], timeout=10)
            self.assertTrue(rcv_port1!=rcv_port2)

            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='192.168.100.3',
                ip_id=106,
                ip_ttl=64)

            exp_pkt1 = simple_tcp_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.100.3',
                ip_id=106,
                ip_ttl=63)
            exp_pkt2 = simple_tcp_packet(
                eth_dst='00:11:22:33:44:56',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.100.3',
                ip_id=106,
                ip_ttl=63)
            send_packet(self, swports[1], str(pkt))
            rcv_port3 = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                       [swports[2], swports[3]], timeout=10)
            self.assertTrue(rcv_port3!=rcv_port2)

            self.client.switch_api_ipv4_hash_algorithm_set(device, SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16)
            send_packet(self, swports[1], str(pkt))
            rcv_port4 = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                       [swports[2], swports[3]], timeout=10)
            self.client.switch_api_ipv4_hash_algorithm_set(device, SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_DECT)
            send_packet(self, swports[1], str(pkt))
            rcv_port5 = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                       [swports[2], swports[3]], timeout=10)
            self.client.switch_api_ipv4_hash_algorithm_set(device, SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_GENIBUS)
            send_packet(self, swports[1], str(pkt))
            rcv_port6 = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                       [swports[2], swports[3]], timeout=10)
            self.client.switch_api_ipv4_hash_algorithm_set(device, SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_DNP)
            send_packet(self, swports[1], str(pkt))
            rcv_port7 = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                       [swports[2], swports[3]], timeout=10)
            self.client.switch_api_ipv4_hash_algorithm_set(device, SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_TELEDISK)
            send_packet(self, swports[1], str(pkt))
            rcv_port8 = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                       [swports[2], swports[3]], timeout=10)

            self.assertTrue(not (rcv_port4==rcv_port5 and rcv_port4==rcv_port6 and rcv_port4==rcv_port7 and rcv_port4==rcv_port8))

            print "Running Hash Field Attribute Tests"

            print "Setting Hash field attribute SRC IP"
            status = self.client.switch_api_ipv4_hash_input_fields_attribute_set(device, SWITCH_HASH_IPV4_INPUT_PROT_DP_DIP_SP_SIP, SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_IP)
            self.assertTrue(status == 0)
            flags = self.client.switch_api_ipv4_hash_input_fields_attribute_get(device, SWITCH_HASH_IPV4_INPUT_PROT_DP_DIP_SP_SIP)
            self.assertTrue(flags.status == 0 and flags.attr_flags == SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_IP)

            rcv_ports = set()
            print "Sending packet over multiple DST Ports"
            port = 1
            max_itrs = 10
            for i in range(0, max_itrs):
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst='10.10.10.1',
                    ip_src='192.168.0.1',
                    ip_id=106,
                    ip_ttl=64,
                    tcp_dport=port)
                exp_pkt1 = simple_tcp_packet(
                    eth_dst='00:11:22:33:44:55',
                    eth_src='00:77:66:55:44:33',
                    ip_dst='10.10.10.1',
                    ip_src='192.168.0.1',
                    ip_id=106,
                    ip_ttl=63,
                    tcp_dport=port)
                exp_pkt2 = simple_tcp_packet(
                    eth_dst='00:11:22:33:44:56',
                    eth_src='00:77:66:55:44:33',
                    ip_dst='10.10.10.1',
                    ip_src='192.168.0.1',
                    ip_id=106,
                    ip_ttl=63,
                    tcp_dport=port)
                send_packet(self, swports[1], str(pkt))
                rcv_port = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                           [swports[2], swports[3]], timeout=10)
                rcv_ports.add(rcv_port)
                port+=1
            self.assertTrue(len(rcv_ports)==1)

            rcv_ports = set()
            print "Sending packet from multiple source IP address"
            ip_addr = '192.168.0.3'
            max_itrs = 10
            for i in range(0, max_itrs):
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst='10.10.10.1',
                    ip_src=ip_addr,
                    ip_id=106,
                    ip_ttl=64,
                    tcp_dport=port)
                exp_pkt1 = simple_tcp_packet(
                    eth_dst='00:11:22:33:44:55',
                    eth_src='00:77:66:55:44:33',
                    ip_dst='10.10.10.1',
                    ip_src=ip_addr,
                    ip_id=106,
                    ip_ttl=63,
                    tcp_dport=port)
                exp_pkt2 = simple_tcp_packet(
                    eth_dst='00:11:22:33:44:56',
                    eth_src='00:77:66:55:44:33',
                    ip_dst='10.10.10.1',
                    ip_src=ip_addr,
                    ip_id=106,
                    ip_ttl=63,
                    tcp_dport=port)
                send_packet(self, swports[1], str(pkt))
                rcv_port = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                           [swports[2], swports[3]], timeout=10)
                rcv_ports.add(rcv_port)
                addr=struct.unpack("!I", socket.inet_aton(ip_addr))[0]
                addr+=1
                ip_addr=socket.inet_ntoa(struct.pack("!I", addr))
            self.assertTrue(len(rcv_ports)!=1)

            print "Setting Hash field attribute SRC Port Dst Port"
            attr_flags = SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_DST_PORT | SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_PORT
            status = self.client.switch_api_ipv4_hash_input_fields_attribute_set(device, SWITCH_HASH_IPV4_INPUT_PROT_DP_DIP_SP_SIP, attr_flags)
            self.assertTrue(status == 0)
            flags = self.client.switch_api_ipv4_hash_input_fields_attribute_get(device, SWITCH_HASH_IPV4_INPUT_PROT_DP_DIP_SP_SIP)
            self.assertTrue(flags.status == 0 and flags.attr_flags == attr_flags)

            rcv_ports = set()
            print "Sending packet over multiple DST Ports"
            port = 1
            max_itrs = 10
            for i in range(0, max_itrs):
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst='10.10.10.1',
                    ip_src='192.168.0.1',
                    ip_id=106,
                    ip_ttl=64,
                    tcp_dport=port)
                exp_pkt1 = simple_tcp_packet(
                    eth_dst='00:11:22:33:44:55',
                    eth_src='00:77:66:55:44:33',
                    ip_dst='10.10.10.1',
                    ip_src='192.168.0.1',
                    ip_id=106,
                    ip_ttl=63,
                    tcp_dport=port)
                exp_pkt2 = simple_tcp_packet(
                    eth_dst='00:11:22:33:44:56',
                    eth_src='00:77:66:55:44:33',
                    ip_dst='10.10.10.1',
                    ip_src='192.168.0.1',
                    ip_id=106,
                    ip_ttl=63,
                    tcp_dport=port)
                send_packet(self, swports[1], str(pkt))
                rcv_port = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                           [swports[2], swports[3]], timeout=10)
                rcv_ports.add(rcv_port)
                port+=1
            self.assertTrue(len(rcv_ports)!=1)

            rcv_ports = set()
            print "Sending packet over multiple SRC Ports"
            port = 1
            max_itrs = 10
            for i in range(0, max_itrs):
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst='10.10.10.1',
                    ip_src='192.168.0.1',
                    ip_id=106,
                    ip_ttl=64,
                    tcp_sport=port)
                exp_pkt1 = simple_tcp_packet(
                    eth_dst='00:11:22:33:44:55',
                    eth_src='00:77:66:55:44:33',
                    ip_dst='10.10.10.1',
                    ip_src='192.168.0.1',
                    ip_id=106,
                    ip_ttl=63,
                    tcp_sport=port)
                exp_pkt2 = simple_tcp_packet(
                    eth_dst='00:11:22:33:44:56',
                    eth_src='00:77:66:55:44:33',
                    ip_dst='10.10.10.1',
                    ip_src='192.168.0.1',
                    ip_id=106,
                    ip_ttl=63,
                    tcp_sport=port)
                send_packet(self, swports[1], str(pkt))
                rcv_port = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                           [swports[2], swports[3]], timeout=10)
                rcv_ports.add(rcv_port)
                port+=1
            self.assertTrue(len(rcv_ports)!=1)

            rcv_ports = set()
            print "Sending packet from multiple source IP address"
            ip_addr = '192.168.0.3'
            max_itrs = 10
            for i in range(0, max_itrs):
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst='10.10.10.1',
                    ip_src=ip_addr,
                    ip_id=106,
                    ip_ttl=64,
                    tcp_dport=port)
                exp_pkt1 = simple_tcp_packet(
                    eth_dst='00:11:22:33:44:55',
                    eth_src='00:77:66:55:44:33',
                    ip_dst='10.10.10.1',
                    ip_src=ip_addr,
                    ip_id=106,
                    ip_ttl=63,
                    tcp_dport=port)
                exp_pkt2 = simple_tcp_packet(
                    eth_dst='00:11:22:33:44:56',
                    eth_src='00:77:66:55:44:33',
                    ip_dst='10.10.10.1',
                    ip_src=ip_addr,
                    ip_id=106,
                    ip_ttl=63,
                    tcp_dport=port)
                send_packet(self, swports[1], str(pkt))
                rcv_port = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                           [swports[2], swports[3]], timeout=10)
                rcv_ports.add(rcv_port)
                addr=struct.unpack("!I", socket.inet_aton(ip_addr))[0]
                addr+=1
                ip_addr=socket.inet_ntoa(struct.pack("!I", addr))
            self.assertTrue(len(rcv_ports)==1)

        finally:
            print default_algo_res.algorithm, default_input_fields_res.fields
            self.client.switch_api_ipv4_hash_algorithm_set(device, default_algo_res.algorithm)
            self.client.switch_api_ipv4_hash_input_fields_set(device, default_input_fields_res.fields)
            self.client.switch_api_ipv4_hash_input_fields_attribute_set(device, default_input_fields_res.fields, default_attr_res.attr_flags)
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


###############################################################################
@group('l3')
@group('ipv4')
@group('clpm')
@group('maxsizes')
@group('ent')
class L3IPv4EcmpTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Sending packet port %d" % swports[1], " -> port %d" % swports[
            2], " (192.168.0.1 -> 10.0.0.1 [id = 101])"
        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

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

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v4_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf,
                                                        i_ip2)

        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v4_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif3 = self.client.switch_api_rif_create(0, rif_info3)
        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='11.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif3, vrf,
                                                        i_ip3)

        i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.1',
            prefix_length=32)

        nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip4, '00:11:22:33:44:55')
        nhop2, neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, rif3, i_ip4, '00:11:22:33:44:56')

        ecmp = self.client.switch_api_ecmp_create(device)
        self.client.switch_api_ecmp_member_add(device, ecmp, 2, [nhop1, nhop2])

        self.client.switch_api_l3_route_add(device, vrf, i_ip4, ecmp)

        try:
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=106,
                ip_ttl=64)
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
            send_packet(self, swports[1], str(pkt))
            verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                       [swports[2], swports[3]], timeout=2)

            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='192.168.100.3',
                ip_id=106,
                ip_ttl=64)

            exp_pkt1 = simple_tcp_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.100.3',
                ip_id=106,
                ip_ttl=63)
            exp_pkt2 = simple_tcp_packet(
                eth_dst='00:11:22:33:44:56',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.100.3',
                ip_id=106,
                ip_ttl=63)

            send_packet(self, swports[1], str(pkt))
            verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                       [swports[2], swports[3]], timeout=2)
        finally:
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


###############################################################################
@group('l3')
@group('ipv6')
@group('maxsizes')
@group('ent')
class L3IPv6DynHashEcmpTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        if ((test_param_get('target') == 'bmv2') or
            (test_param_get('target') == 'bmv2' and
             test_param_get('arch') == 'Tofino')):
            return
        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

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
            ipaddr='2000:1:1:0:0:0:0:1',
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
            ipaddr='3000:1:1:0:0:0:0:1',
            prefix_length=120)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf,
                                                        i_ip2)

        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v6_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif3 = self.client.switch_api_rif_create(0, rif_info3)
        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='4000:1:1:0:0:0:0:1',
            prefix_length=120)
        self.client.switch_api_l3_interface_address_add(device, rif3, vrf,
                                                        i_ip3)

        i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='5000:1:1:0:0:0:0:1',
            prefix_length=128)

        nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip4, '00:11:22:33:44:55')
        nhop2, neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, rif3, i_ip4, '00:11:22:33:44:56')

        ecmp = self.client.switch_api_ecmp_create(device)
        self.client.switch_api_ecmp_member_add(device, ecmp, 2, [nhop1, nhop2])

        self.client.switch_api_l3_route_add(device, vrf, i_ip4, ecmp)

        default_algo_res = self.client.switch_api_ipv6_hash_algorithm_get(device)
        self.assertTrue(default_algo_res.status==0)
        default_input_fields_res = self.client.switch_api_ipv6_hash_input_fields_get(device)
        self.assertTrue(default_input_fields_res.status==0)
        default_attr_res = self.client.switch_api_ipv6_hash_input_fields_attribute_get(device, default_input_fields_res.fields)
        self.assertTrue(default_attr_res.status==0)

        try:
            print "Sending packet port %d" % swports[1], " -> port %d" % swports[
                2], " (2000:1:1:0:0:0:0:1 -> 5000:1:1::0:0:0:0:1) [id = 101])"
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
            rcv_port1 = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                       [swports[2], swports[3]], timeout=10)
            print "Sending packet port %d" % swports[1], " -> port %d" % swports[
                3], " (2000:1:1:0:0:0:0:1 -> 5000:1:1::0:0:0:0:1) [id = 101])"
            pkt = simple_tcpv6_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:45',
                ipv6_dst='5000:1:1:0:0:0:0:1',
                ipv6_src='2000:1:1:0:0:0:0:1',
                tcp_sport=0x1248,
                ipv6_hlim=64)
            exp_pkt1 = simple_tcpv6_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='5000:1:1:0:0:0:0:1',
                ipv6_src='2000:1:1:0:0:0:0:1',
                tcp_sport=0x1248,
                ipv6_hlim=63)
            exp_pkt2 = simple_tcpv6_packet(
                eth_dst='00:11:22:33:44:56',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='5000:1:1:0:0:0:0:1',
                ipv6_src='2000:1:1:0:0:0:0:1',
                tcp_sport=0x1248,
                ipv6_hlim=63)
            send_packet(self, swports[1], str(pkt))
            rcv_port2 = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                       [swports[2], swports[3]], timeout=10)

            print "Rerunning test with Hash input field list PROT_DP_SIP_SP_DIP"
            self.client.switch_api_ipv6_hash_input_fields_set(device, SWITCH_HASH_IPV6_INPUT_PROT_DP_SIP_SP_DIP)
            send_packet(self, swports[1], str(pkt))
            rcv_port3 = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                       [swports[2], swports[3]], timeout=10)
	    self.assertTrue(rcv_port2!=rcv_port3)

            print "Rerunning test with different Hash algorithms"
            self.client.switch_api_ipv6_hash_algorithm_set(device, SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16)
            send_packet(self, swports[1], str(pkt))
            rcv_port4 = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                       [swports[2], swports[3]], timeout=10)
            self.client.switch_api_ipv6_hash_algorithm_set(device, SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_DECT)
            send_packet(self, swports[1], str(pkt))
            rcv_port5 = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                       [swports[2], swports[3]], timeout=10)
            self.client.switch_api_ipv6_hash_algorithm_set(device, SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_GENIBUS)
            send_packet(self, swports[1], str(pkt))
            rcv_port6 = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                       [swports[2], swports[3]], timeout=10)
            self.client.switch_api_ipv6_hash_algorithm_set(device, SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_DNP)
            send_packet(self, swports[1], str(pkt))
            rcv_port7 = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                       [swports[2], swports[3]], timeout=10)
            self.client.switch_api_ipv6_hash_algorithm_set(device, SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_TELEDISK)
            send_packet(self, swports[1], str(pkt))
            rcv_port8 = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                       [swports[2], swports[3]], timeout=10)
            self.assertTrue(not (rcv_port4==rcv_port5 and rcv_port4==rcv_port6 and rcv_port4==rcv_port7 and rcv_port4==rcv_port8))

            print "Running Hash Field Attribute Tests"

            print "Setting Hash field attribute SRC IP"
            status = self.client.switch_api_ipv6_hash_input_fields_attribute_set(device, SWITCH_HASH_IPV6_INPUT_PROT_DP_SIP_SP_DIP, SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_IP)
            self.assertTrue(status == 0)
            flags = self.client.switch_api_ipv6_hash_input_fields_attribute_get(device, SWITCH_HASH_IPV6_INPUT_PROT_DP_SIP_SP_DIP)
            self.assertTrue(flags.status == 0 and flags.attr_flags == SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_IP)

            rcv_ports = set()
            print "Sending packet over multiple DST Ports"
            port = 1
            max_itrs = 10
            for i in range(0, max_itrs):
                pkt = simple_tcpv6_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ipv6_dst='5000:1:1:0:0:0:0:1',
                    ipv6_src='2000:1:1:0:0:0:0:1',
                    tcp_dport=port,
                    ipv6_hlim=64)
                exp_pkt1 = simple_tcpv6_packet(
                    eth_dst='00:11:22:33:44:55',
                    eth_src='00:77:66:55:44:33',
                    ipv6_dst='5000:1:1:0:0:0:0:1',
                    ipv6_src='2000:1:1:0:0:0:0:1',
                    tcp_dport=port,
                    ipv6_hlim=63)
                exp_pkt2 = simple_tcpv6_packet(
                   eth_dst='00:11:22:33:44:56',
                   eth_src='00:77:66:55:44:33',
                   ipv6_dst='5000:1:1:0:0:0:0:1',
                   ipv6_src='2000:1:1:0:0:0:0:1',
                   tcp_dport=port,
                   ipv6_hlim=63)
                send_packet(self, swports[1], str(pkt))
                rcv_port = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                           [swports[2], swports[3]], timeout=10)
                rcv_ports.add(rcv_port)
                port+=1
            self.assertTrue(len(rcv_ports)==1)

            rcv_ports = set()
            print "Sending packet from multiple source IP address"
            src_ip = socket.inet_pton(socket.AF_INET6, '2000:1:1:0:0:0:0:1')
            src_ip_arr = list(src_ip)
            max_itrs = 10
            for i in range(0, max_itrs):
                src_ip_addr = socket.inet_ntop(socket.AF_INET6, src_ip)
                pkt = simple_tcpv6_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ipv6_dst='5000:1:1:0:0:0:0:1',
                    ipv6_src=src_ip_addr,
                    tcp_dport=port,
                    ipv6_hlim=64)
                exp_pkt1 = simple_tcpv6_packet(
                    eth_dst='00:11:22:33:44:55',
                    eth_src='00:77:66:55:44:33',
                    ipv6_dst='5000:1:1:0:0:0:0:1',
                    ipv6_src=src_ip_addr,
                    tcp_dport=port,
                    ipv6_hlim=63)
                exp_pkt2 = simple_tcpv6_packet(
                   eth_dst='00:11:22:33:44:56',
                   eth_src='00:77:66:55:44:33',
                   ipv6_dst='5000:1:1:0:0:0:0:1',
                   ipv6_src=src_ip_addr,
                   tcp_dport=port,
                   ipv6_hlim=63)
                send_packet(self, swports[1], str(pkt))
                rcv_port = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                           [swports[2], swports[3]], timeout=10)
                rcv_ports.add(rcv_port)
                src_ip_arr[15] = chr(ord(src_ip_arr[15]) + 1)
                src_ip = ''.join(src_ip_arr)
            self.assertTrue(len(rcv_ports)!=1)

            print "Setting Hash field attribute SRC Port Dst Port"
            attr_flags = SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_PORT | SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_DST_PORT
            status = self.client.switch_api_ipv6_hash_input_fields_attribute_set(device, SWITCH_HASH_IPV6_INPUT_PROT_DP_SIP_SP_DIP, attr_flags)
            self.assertTrue(status == 0)
            flags = self.client.switch_api_ipv6_hash_input_fields_attribute_get(device, SWITCH_HASH_IPV6_INPUT_PROT_DP_SIP_SP_DIP)
            self.assertTrue(flags.status == 0 and flags.attr_flags == attr_flags)

            rcv_ports = set()
            print "Sending packet over multiple DST Ports"
            port = 1
            max_itrs = 10
            for i in range(0, max_itrs):
                pkt = simple_tcpv6_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ipv6_dst='5000:1:1:0:0:0:0:1',
                    ipv6_src='2000:1:1:0:0:0:0:1',
                    tcp_dport=port,
                    ipv6_hlim=64)
                exp_pkt1 = simple_tcpv6_packet(
                    eth_dst='00:11:22:33:44:55',
                    eth_src='00:77:66:55:44:33',
                    ipv6_dst='5000:1:1:0:0:0:0:1',
                    ipv6_src='2000:1:1:0:0:0:0:1',
                    tcp_dport=port,
                    ipv6_hlim=63)
                exp_pkt2 = simple_tcpv6_packet(
                   eth_dst='00:11:22:33:44:56',
                   eth_src='00:77:66:55:44:33',
                   ipv6_dst='5000:1:1:0:0:0:0:1',
                   ipv6_src='2000:1:1:0:0:0:0:1',
                   tcp_dport=port,
                   ipv6_hlim=63)
                send_packet(self, swports[1], str(pkt))
                rcv_port = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                           [swports[2], swports[3]], timeout=10)
                rcv_ports.add(rcv_port)
                port+=1
            self.assertTrue(len(rcv_ports)!=1)

            rcv_ports = set()
            print "Sending packet over multiple SRC Ports"
            port = 1
            max_itrs = 10
            for i in range(0, max_itrs):
                pkt = simple_tcpv6_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ipv6_dst='5000:1:1:0:0:0:0:1',
                    ipv6_src='2000:1:1:0:0:0:0:1',
                    tcp_sport=port,
                    ipv6_hlim=64)
                exp_pkt1 = simple_tcpv6_packet(
                    eth_dst='00:11:22:33:44:55',
                    eth_src='00:77:66:55:44:33',
                    ipv6_dst='5000:1:1:0:0:0:0:1',
                    ipv6_src='2000:1:1:0:0:0:0:1',
                    tcp_sport=port,
                    ipv6_hlim=63)
                exp_pkt2 = simple_tcpv6_packet(
                   eth_dst='00:11:22:33:44:56',
                   eth_src='00:77:66:55:44:33',
                   ipv6_dst='5000:1:1:0:0:0:0:1',
                   ipv6_src='2000:1:1:0:0:0:0:1',
                   tcp_sport=port,
                   ipv6_hlim=63)
                send_packet(self, swports[1], str(pkt))
                rcv_port = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                           [swports[2], swports[3]], timeout=10)
                rcv_ports.add(rcv_port)
                port+=1
            self.assertTrue(len(rcv_ports)!=1)

            rcv_ports = set()
            print "Sending packet from multiple source IP address"
            src_ip = socket.inet_pton(socket.AF_INET6, '2000:1:1:0:0:0:0:1')
            src_ip_arr = list(src_ip)
            max_itrs = 10
            for i in range(0, max_itrs):
                src_ip_addr = socket.inet_ntop(socket.AF_INET6, src_ip)
                pkt = simple_tcpv6_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ipv6_dst='5000:1:1:0:0:0:0:1',
                    ipv6_src=src_ip_addr,
                    tcp_sport=port,
                    ipv6_hlim=64)
                exp_pkt1 = simple_tcpv6_packet(
                    eth_dst='00:11:22:33:44:55',
                    eth_src='00:77:66:55:44:33',
                    ipv6_dst='5000:1:1:0:0:0:0:1',
                    ipv6_src=src_ip_addr,
                    tcp_sport=port,
                    ipv6_hlim=63)
                exp_pkt2 = simple_tcpv6_packet(
                   eth_dst='00:11:22:33:44:56',
                   eth_src='00:77:66:55:44:33',
                   ipv6_dst='5000:1:1:0:0:0:0:1',
                   ipv6_src=src_ip_addr,
                   tcp_sport=port,
                   ipv6_hlim=63)
                send_packet(self, swports[1], str(pkt))
                rcv_port = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                           [swports[2], swports[3]], timeout=10)
                rcv_ports.add(rcv_port)
                src_ip_arr[15] = chr(ord(src_ip_arr[15]) + 1)
                src_ip = ''.join(src_ip_arr)
            self.assertTrue(len(rcv_ports)==1)

        finally:
            self.client.switch_api_ipv6_hash_algorithm_set(device, default_algo_res.algorithm)
            self.client.switch_api_ipv6_hash_input_fields_set(device, default_input_fields_res.fields)
            self.client.switch_api_ipv6_hash_input_fields_attribute_set(device, default_input_fields_res.fields, default_attr_res.attr_flags)

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


###############################################################################
@group('l3')
@group('ipv6')
@group('maxsizes')
@group('ent')
class L3IPv6EcmpTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

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
            ipaddr='2000:1:1:0:0:0:0:1',
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
            ipaddr='3000:1:1:0:0:0:0:1',
            prefix_length=120)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf,
                                                        i_ip2)

        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v6_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif3 = self.client.switch_api_rif_create(0, rif_info3)
        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='4000:1:1:0:0:0:0:1',
            prefix_length=120)
        self.client.switch_api_l3_interface_address_add(device, rif3, vrf,
                                                        i_ip3)

        i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='5000:1:1:0:0:0:0:1',
            prefix_length=128)
        nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip4, '00:11:22:33:44:55')
        nhop2, neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, rif3, i_ip4, '00:11:22:33:44:56')
        ecmp = self.client.switch_api_ecmp_create(device)
        self.client.switch_api_ecmp_member_add(device, ecmp, 2, [nhop1, nhop2])

        self.client.switch_api_l3_route_add(device, vrf, i_ip4, ecmp)

        try:
            print "Sending packet port %d" % swports[1], " -> port %d" % swports[
                2], " (2000:1:1:0:0:0:0:1 -> 5000:1:1::0:0:0:0:1) [id = 101])"
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
            verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                       [swports[2], swports[3]], timeout=2)

            print "Sending packet port %d" % swports[1], " -> port %d" % swports[
                3], " (2000:1:1:0:0:0:0:1 -> 5000:1:1::0:0:0:0:1) [id = 101])"
            pkt = simple_tcpv6_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:45',
                ipv6_dst='5000:1:1:0:0:0:0:1',
                ipv6_src='2000:1:1:0:0:0:0:1',
                tcp_sport=0x1248,
                ipv6_hlim=64)
            exp_pkt1 = simple_tcpv6_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='5000:1:1:0:0:0:0:1',
                ipv6_src='2000:1:1:0:0:0:0:1',
                tcp_sport=0x1248,
                ipv6_hlim=63)
            exp_pkt2 = simple_tcpv6_packet(
                eth_dst='00:11:22:33:44:56',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='5000:1:1:0:0:0:0:1',
                ipv6_src='2000:1:1:0:0:0:0:1',
                tcp_sport=0x1248,
                ipv6_hlim=63)
            send_packet(self, swports[1], str(pkt))
            verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                       [swports[2], swports[3]], timeout=2)
        finally:
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


###############################################################################
@group('l3')
@group('ipv4')
@group('clpm')
@group('maxsizes')
@group('ent')
class L3IPv4LpmEcmpTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port0 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])
        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[4])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v4_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port0, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)
        i_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='192.168.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif1, vrf,
                                                        i_ip1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v4_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf,
                                                        i_ip2)

        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v4_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif3 = self.client.switch_api_rif_create(0, rif_info3)
        i_info3 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='11.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif3, vrf,
                                                        i_ip3)

        rif_info4 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v4_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif4 = self.client.switch_api_rif_create(0, rif_info4)
        i_info4 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif4)
        if4 = self.client.switch_api_interface_create(device, i_info4)
        i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='12.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif4, vrf,
                                                        i_ip4)

        rif_info5 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v4_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif5 = self.client.switch_api_rif_create(0, rif_info5)
        i_info5 = switcht_interface_info_t(
            handle=port4, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif5)
        if5 = self.client.switch_api_interface_create(device, i_info5)
        i_ip5 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='13.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif5, vrf,
                                                        i_ip5)

        n_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.100',
            prefix_length=32)
        n_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='11.0.0.100',
            prefix_length=32)
        n_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='12.0.0.101',
            prefix_length=32)
        n_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='13.0.0.101',
            prefix_length=32)
        nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, rif2, n_ip1, '00:11:22:33:44:55')
        nhop2, neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, rif3, n_ip2, '00:11:22:33:44:56')
        nhop3, neighbor3 = switch_api_l3_nhop_neighbor_create(self, device, rif4, n_ip3, '00:11:22:33:44:57')
        nhop4, neighbor4 = switch_api_l3_nhop_neighbor_create(self, device, rif5, n_ip4, '00:11:22:33:44:58')
        ecmp = self.client.switch_api_ecmp_create(device)
        self.client.switch_api_ecmp_member_add(device, ecmp, 4,
                                               [nhop1, nhop2, nhop3, nhop4])

        r_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.0.0',
            prefix_length=16)
        self.client.switch_api_l3_route_add(device, vrf, r_ip, ecmp)

        try:
            count = [0, 0, 0, 0]
            dst_ip = int(socket.inet_aton('10.10.10.1').encode('hex'), 16)
            max_itrs = 200
            random.seed(314159)
            for i in range(0, max_itrs):
                dst_ip_addr = socket.inet_ntoa(
                    format(dst_ip, 'x').zfill(8).decode('hex'))
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

                send_packet(self, swports[0], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self, [exp_pkt1, exp_pkt2, exp_pkt3, exp_pkt4],
                    [swports[1], swports[2], swports[3], swports[4]], timeout=2)
                count[rcv_idx] += 1
                dst_ip += 1

            print "ECMP load balancing result ", count
            for i in range(0, 4):
                self.assertTrue((count[i] >= ((max_itrs / 4) * 0.7)),
                                "Not all paths are equally balanced")
        finally:
            self.client.switch_api_l3_route_delete(device, vrf, r_ip, ecmp)

            self.client.switch_api_ecmp_member_delete(
                device, ecmp, 4, [nhop1, nhop2, nhop3, nhop4])
            self.client.switch_api_ecmp_delete(device, ecmp)

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


###############################################################################
@group('l3')
@group('ipv6')
@group('maxsizes')
@group('ent')
class L3IPv6LpmEcmpTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])
        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[4])
        port5 = self.client.switch_api_port_id_to_handle_get(device, swports[5])

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
            ipaddr='1000:1:1:0:0:0:0:1',
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
            ipaddr='2000:1:1:0:0:0:0:1',
            prefix_length=120)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf,
                                                        i_ip2)

        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v6_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif3 = self.client.switch_api_rif_create(0, rif_info3)
        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='3000:1:1:0:0:0:0:1',
            prefix_length=120)
        self.client.switch_api_l3_interface_address_add(device, rif3, vrf,
                                                        i_ip3)

        rif_info4 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v6_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif4 = self.client.switch_api_rif_create(0, rif_info4)
        i_info4 = switcht_interface_info_t(
            handle=port4, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif4)
        if4 = self.client.switch_api_interface_create(device, i_info4)
        i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='4000:1:1:0:0:0:0:1',
            prefix_length=120)
        self.client.switch_api_l3_interface_address_add(device, rif4, vrf,
                                                        i_ip4)

        rif_info5 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v6_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif5 = self.client.switch_api_rif_create(0, rif_info5)
        i_info5 = switcht_interface_info_t(
            handle=port5, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif5)
        if5 = self.client.switch_api_interface_create(device, i_info5)
        i_ip5 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='5000:1:1:0:0:0:0:1',
            prefix_length=120)
        self.client.switch_api_l3_interface_address_add(device, rif5, vrf,
                                                        i_ip5)

        nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip2, '00:11:22:33:44:55')
        nhop2, neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, rif3, i_ip3, '00:11:22:33:44:56')
        nhop3, neighbor3 = switch_api_l3_nhop_neighbor_create(self, device, rif4, i_ip4, '00:11:22:33:44:57')
        nhop4, neighbor4 = switch_api_l3_nhop_neighbor_create(self, device, rif5, i_ip5, '00:11:22:33:44:58')
        print hex(port1) + " " + hex(port2) + " " + hex(port3) + " " + hex(
            port4)
        print hex(nhop1) + " " + hex(nhop2) + " " + hex(nhop3) + " " + hex(
            nhop4)
        ecmp = self.client.switch_api_ecmp_create(device)
        self.client.switch_api_ecmp_member_add(device, ecmp, 4,
                                               [nhop1, nhop2, nhop3, nhop4])

        r_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='6000:1:1:0:0:0:0:0',
            prefix_length=64)
        self.client.switch_api_l3_route_add(device, vrf, r_ip, ecmp)

        try:
            count = [0, 0, 0, 0]
            dst_ip = socket.inet_pton(socket.AF_INET6, '6000:1:1:0:0:0:0:1')
            dst_ip_arr = list(dst_ip)
            max_itrs = 200
            sport = 0x1234
            dport = 0x50
            for i in range(0, max_itrs):
                dst_ip_addr = socket.inet_ntop(socket.AF_INET6, dst_ip)
                pkt = simple_tcpv6_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ipv6_dst=dst_ip_addr,
                    ipv6_src='1001:1:1:0:0:0:0:2',
                    tcp_sport=sport,
                    tcp_dport=dport,
                    ipv6_hlim=64)
                exp_pkt1 = simple_tcpv6_packet(
                    eth_dst='00:11:22:33:44:55',
                    eth_src='00:77:66:55:44:33',
                    ipv6_dst=dst_ip_addr,
                    ipv6_src='1001:1:1:0:0:0:0:2',
                    tcp_sport=sport,
                    tcp_dport=dport,
                    ipv6_hlim=63)
                exp_pkt2 = simple_tcpv6_packet(
                    eth_dst='00:11:22:33:44:56',
                    eth_src='00:77:66:55:44:33',
                    ipv6_dst=dst_ip_addr,
                    ipv6_src='1001:1:1:0:0:0:0:2',
                    tcp_sport=sport,
                    tcp_dport=dport,
                    ipv6_hlim=63)
                exp_pkt3 = simple_tcpv6_packet(
                    eth_dst='00:11:22:33:44:57',
                    eth_src='00:77:66:55:44:33',
                    ipv6_dst=dst_ip_addr,
                    ipv6_src='1001:1:1:0:0:0:0:2',
                    tcp_sport=sport,
                    tcp_dport=dport,
                    ipv6_hlim=63)
                exp_pkt4 = simple_tcpv6_packet(
                    eth_dst='00:11:22:33:44:58',
                    eth_src='00:77:66:55:44:33',
                    ipv6_dst=dst_ip_addr,
                    ipv6_src='1001:1:1:0:0:0:0:2',
                    tcp_sport=sport,
                    tcp_dport=dport,
                    ipv6_hlim=63)

                send_packet(self, swports[1], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self, [exp_pkt1, exp_pkt2, exp_pkt3, exp_pkt4],
                    [swports[2], swports[3], swports[4], swports[5]], timeout=2)
                count[rcv_idx] += 1
                dst_ip_arr[15] = chr(ord(dst_ip_arr[15]) + 1)
                dst_ip = ''.join(dst_ip_arr)
                sport += 15
                dport += 20

            print "Count = %s" % str(count)
            for i in range(0, 4):
                self.assertTrue((count[i] >= ((max_itrs / 4) * 0.50)),
                                "Not all paths are equally balanced")
        finally:
            self.client.switch_api_l3_route_delete(device, vrf, r_ip, ecmp)

            self.client.switch_api_ecmp_member_delete(
                device, ecmp, 4, [nhop1, nhop2, nhop3, nhop4])
            self.client.switch_api_ecmp_delete(device, ecmp)

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
###############################################################################

@group('l3')
@group('ipv4')
@group('clpm')
@group('maxsizes')
@group('ent')
class L3IPv4DynHashLagTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        if ((test_param_get('target') == 'bmv2') or
            (test_param_get('target') == 'bmv2' and
             test_param_get('arch') == 'Tofino')):
            return
        print "Sending packet port %d" % swports[1], " -> port %d" % swports[
            2], " (192.168.0.1 -> 10.0.0.1 [id = 101])"
        vrf = self.client.switch_api_vrf_create(device, 2)
        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])
        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[4])
        port5 = self.client.switch_api_port_id_to_handle_get(device, swports[5])

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

        lag = self.client.switch_api_lag_create(device)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port2)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port3)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port4)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port5)
        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v4_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=lag, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf,
                                                        i_ip2)

        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.1',
            prefix_length=32)
        nhop, neighbor = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip3, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(device, vrf, i_ip3, nhop)

        default_algo_res = self.client.switch_api_ipv4_hash_algorithm_get(device)
        self.assertTrue(default_algo_res.status==0)
        default_input_fields_res = self.client.switch_api_ipv4_hash_input_fields_get(device)
        self.assertTrue(default_input_fields_res.status==0)
        default_attr_res = self.client.switch_api_ipv4_hash_input_fields_attribute_get(device, default_input_fields_res.fields)
        self.assertTrue(default_attr_res.status==0)
        try:
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=110,
                ip_ttl=64)

            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=110,
                ip_ttl=63)
            send_packet(self, swports[1], str(pkt))
            rcv_port1 = verify_any_packet_any_port(self, [exp_pkt],
                                       [swports[2], swports[3], swports[4], swports[5]], timeout=10)

            self.client.switch_api_ipv4_hash_input_fields_set(device, SWITCH_HASH_IPV4_INPUT_PROT_DP_DIP_SP_SIP)
            send_packet(self, swports[1], str(pkt))
            rcv_port2 = verify_any_packet_any_port(self, [exp_pkt],
                                       [swports[2], swports[3], swports[4], swports[5]], timeout=10)
            self.assertTrue(rcv_port1!=rcv_port2)

            self.client.switch_api_ipv4_hash_algorithm_set(device, SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16)
            send_packet(self, swports[1], str(pkt))
            rcv_port3 = verify_any_packet_any_port(self, [exp_pkt],
                                       [swports[2], swports[3], swports[4], swports[5]], timeout=10)
            self.client.switch_api_ipv4_hash_algorithm_set(device, SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_DECT)
            send_packet(self, swports[1], str(pkt))
            rcv_port4 = verify_any_packet_any_port(self, [exp_pkt],
                                       [swports[2], swports[3], swports[4], swports[5]], timeout=10)
            self.client.switch_api_ipv4_hash_algorithm_set(device, SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_GENIBUS)
            send_packet(self, swports[1], str(pkt))
            rcv_port5 = verify_any_packet_any_port(self, [exp_pkt],
                                       [swports[2], swports[3], swports[4], swports[5]], timeout=10)
            self.client.switch_api_ipv4_hash_algorithm_set(device, SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_DNP)
            send_packet(self, swports[1], str(pkt))
            rcv_port6 = verify_any_packet_any_port(self, [exp_pkt],
                                       [swports[2], swports[3], swports[4], swports[5]], timeout=10)
            self.client.switch_api_ipv4_hash_algorithm_set(device, SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_TELEDISK)
            send_packet(self, swports[1], str(pkt))
            rcv_port7 = verify_any_packet_any_port(self, [exp_pkt],
                                       [swports[2], swports[3], swports[4], swports[5]], timeout=10)
            self.assertTrue(not (rcv_port3==rcv_port4 and rcv_port3==rcv_port5 and rcv_port3==rcv_port6 and rcv_port3==rcv_port7))

            print "Running Hash Field Attribute Tests"

            print "Setting Hash field attribute SRC IP"
            status = self.client.switch_api_ipv4_hash_input_fields_attribute_set(device, SWITCH_HASH_IPV4_INPUT_PROT_DP_DIP_SP_SIP, SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_IP)
            self.assertTrue(status == 0)
            flags = self.client.switch_api_ipv4_hash_input_fields_attribute_get(device, SWITCH_HASH_IPV4_INPUT_PROT_DP_DIP_SP_SIP)
            self.assertTrue(flags.status == 0 and flags.attr_flags == SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_IP)

            rcv_ports = set()
            print "Sending packet over multiple DST Ports"
            port = 1
            max_itrs = 10
            for i in range(0, max_itrs):
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst='10.10.10.1',
                    ip_src='192.168.0.1',
                    ip_id=110,
                    ip_ttl=64,
                    tcp_dport=port)

                exp_pkt = simple_tcp_packet(
                    eth_dst='00:11:22:33:44:55',
                    eth_src='00:77:66:55:44:33',
                    ip_dst='10.10.10.1',
                    ip_src='192.168.0.1',
                    ip_id=110,
                    ip_ttl=63,
                    tcp_dport=port)
                send_packet(self, swports[1], str(pkt))
                rcv_port = verify_any_packet_any_port(self, [exp_pkt],
                                       [swports[2], swports[3], swports[4], swports[5]], timeout=10)
                rcv_ports.add(rcv_port)
                port+=1
            self.assertTrue(len(rcv_ports)==1)

            rcv_ports = set()
            print "Sending packet from multiple source IP address"
            ip_addr = '192.168.0.3'
            max_itrs = 10
            for i in range(0, max_itrs):
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst='10.10.10.1',
                    ip_src=ip_addr,
                    ip_id=110,
                    ip_ttl=64)
                exp_pkt = simple_tcp_packet(
                    eth_dst='00:11:22:33:44:55',
                    eth_src='00:77:66:55:44:33',
                    ip_dst='10.10.10.1',
                    ip_src=ip_addr,
                    ip_id=110,
                    ip_ttl=63)
                send_packet(self, swports[1], str(pkt))
                rcv_port = verify_any_packet_any_port(self, [exp_pkt],
                                       [swports[2], swports[3], swports[4], swports[5]], timeout=10)
                rcv_ports.add(rcv_port)
                addr=struct.unpack("!I", socket.inet_aton(ip_addr))[0]
                addr+=1
                ip_addr=socket.inet_ntoa(struct.pack("!I", addr))
            self.assertTrue(len(rcv_ports)!=1)

            print "Setting Hash field attribute SRC Port Dst Port"
            attr_flags = SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_DST_PORT | SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_PORT
            status = self.client.switch_api_ipv4_hash_input_fields_attribute_set(device, SWITCH_HASH_IPV4_INPUT_PROT_DP_DIP_SP_SIP, attr_flags)
            self.assertTrue(status == 0)
            flags = self.client.switch_api_ipv4_hash_input_fields_attribute_get(device, SWITCH_HASH_IPV4_INPUT_PROT_DP_DIP_SP_SIP)
            self.assertTrue(flags.status == 0 and flags.attr_flags == attr_flags)

            rcv_ports = set()
            print "Sending packet over multiple DST Ports"
            port = 1
            max_itrs = 10
            for i in range(0, max_itrs):
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst='10.10.10.1',
                    ip_src='192.168.0.1',
                    ip_id=110,
                    ip_ttl=64,
                    tcp_dport=port)

                exp_pkt = simple_tcp_packet(
                    eth_dst='00:11:22:33:44:55',
                    eth_src='00:77:66:55:44:33',
                    ip_dst='10.10.10.1',
                    ip_src='192.168.0.1',
                    ip_id=110,
                    ip_ttl=63,
                    tcp_dport=port)
                send_packet(self, swports[1], str(pkt))
                rcv_port = verify_any_packet_any_port(self, [exp_pkt],
                                       [swports[2], swports[3], swports[4], swports[5]], timeout=10)
                rcv_ports.add(rcv_port)
                port+=1
            self.assertTrue(len(rcv_ports)!=1)

            rcv_ports = set()
            print "Sending packet over multiple SRC Ports"
            port = 1
            max_itrs = 10
            for i in range(0, max_itrs):
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst='10.10.10.1',
                    ip_src='192.168.0.1',
                    ip_id=110,
                    ip_ttl=64,
                    tcp_sport=port)

                exp_pkt = simple_tcp_packet(
                    eth_dst='00:11:22:33:44:55',
                    eth_src='00:77:66:55:44:33',
                    ip_dst='10.10.10.1',
                    ip_src='192.168.0.1',
                    ip_id=110,
                    ip_ttl=63,
                    tcp_sport=port)
                send_packet(self, swports[1], str(pkt))
                rcv_port = verify_any_packet_any_port(self, [exp_pkt],
                                       [swports[2], swports[3], swports[4], swports[5]], timeout=10)
                rcv_ports.add(rcv_port)
                port+=1
            self.assertTrue(len(rcv_ports)!=1)

            rcv_ports = set()
            print "Sending packet from multiple source IP address"
            ip_addr = '192.168.0.3'
            max_itrs = 10
            for i in range(0, max_itrs):
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst='10.10.10.1',
                    ip_src=ip_addr,
                    ip_id=110,
                    ip_ttl=64)
                exp_pkt = simple_tcp_packet(
                    eth_dst='00:11:22:33:44:55',
                    eth_src='00:77:66:55:44:33',
                    ip_dst='10.10.10.1',
                    ip_src=ip_addr,
                    ip_id=110,
                    ip_ttl=63)
                send_packet(self, swports[1], str(pkt))
                rcv_port = verify_any_packet_any_port(self, [exp_pkt],
                                       [swports[2], swports[3], swports[4], swports[5]], timeout=10)
                rcv_ports.add(rcv_port)
                addr=struct.unpack("!I", socket.inet_aton(ip_addr))[0]
                addr+=1
                ip_addr=socket.inet_ntoa(struct.pack("!I", addr))
            self.assertTrue(len(rcv_ports)==1)

        finally:
            self.client.switch_api_ipv4_hash_algorithm_set(device, default_algo_res.algorithm)
            self.client.switch_api_ipv4_hash_input_fields_set(device, default_input_fields_res.fields)
            self.client.switch_api_ipv4_hash_input_fields_attribute_set(device, default_input_fields_res.fields, default_attr_res.attr_flags)

            self.client.switch_api_neighbor_delete(device, neighbor)

            self.client.switch_api_l3_route_delete(device, vrf, i_ip3, nhop)
            self.client.switch_api_nhop_delete(device, nhop)

            self.client.switch_api_l3_interface_address_delete(device, rif1,
                                                               vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(device, rif2,
                                                               vrf, i_ip2)

            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port2)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port3)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port4)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port5)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_lag_delete(device, lag)
            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)

###############################################################################
@group('l3')
@group('ipv4')
@group('clpm')
@group('maxsizes')
@group('ent')
class L3IPv4LagTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Sending packet port %d" % swports[1], " -> port %d" % swports[
            2], " (192.168.0.1 -> 10.0.0.1 [id = 101])"
        vrf = self.client.switch_api_vrf_create(device, 2)
        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

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

        lag = self.client.switch_api_lag_create(device)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port2)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port3)
        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v4_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=lag, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf,
                                                        i_ip2)

        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.1',
            prefix_length=32)
        nhop, neighbor = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip3, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(device, vrf, i_ip3, nhop)

        try:
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=110,
                ip_ttl=64)

            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=110,
                ip_ttl=63)
            send_packet(self, swports[1], str(pkt))
            verify_any_packet_any_port(self, [exp_pkt],
                                       [swports[2], swports[3]], timeout=2)
        finally:
            self.client.switch_api_neighbor_delete(device, neighbor)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip3, nhop)
            self.client.switch_api_nhop_delete(device, nhop)

            self.client.switch_api_l3_interface_address_delete(device, rif1,
                                                               vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(device, rif2,
                                                               vrf, i_ip2)

            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port2)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_lag_delete(device, lag)
            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)

###############################################################################
@group('l3')
@group('ipv6')
@group('maxsizes')
@group('ent')
class L3IPv6DynHashLagTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        if ((test_param_get('target') == 'bmv2') or
            (test_param_get('target') == 'bmv2' and
             test_param_get('arch') == 'Tofino')):
            return
        print "Sending packet port %d" % swports[1], " -> port %d" % swports[
            2], " (4001::1 -> 5001::1[id = 101])"
        vrf = self.client.switch_api_vrf_create(device, 2)
        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])
        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[4])
        port5 = self.client.switch_api_port_id_to_handle_get(device, swports[5])

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
            ipaddr='5001::10',
            prefix_length=120)
        self.client.switch_api_l3_interface_address_add(device, rif1, vrf,
                                                        i_ip1)

        lag = self.client.switch_api_lag_create(device)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port2)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port3)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port4)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port5)
        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v6_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=lag, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='4001::10',
            prefix_length=120)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf,
                                                        i_ip2)

        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='4001::1',
            prefix_length=128)
        nhop, neighbor = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip3, '00:88:88:88:88:88')
        self.client.switch_api_l3_route_add(device, vrf, i_ip3, nhop)

        default_algo_res = self.client.switch_api_ipv6_hash_algorithm_get(device)
        self.assertTrue(default_algo_res.status==0)
        default_input_fields_res = self.client.switch_api_ipv6_hash_input_fields_get(device)
        self.assertTrue(default_input_fields_res.status==0)
        default_attr_res = self.client.switch_api_ipv6_hash_input_fields_attribute_get(device, default_input_fields_res.fields)
        self.assertTrue(default_attr_res.status==0)

        try:
            pkt = simple_tcpv6_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ipv6_dst='4001::1',
                ipv6_src='5001::1',
                ipv6_hlim=64)

            exp_pkt = simple_tcpv6_packet(
                eth_dst='00:88:88:88:88:88',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='4001::1',
                ipv6_src='5001::1',
                ipv6_hlim=63)
            send_packet(self, swports[1], str(pkt))
            rcv_port1 = verify_any_packet_any_port(self, [exp_pkt],
                                       [swports[2], swports[3], swports[4], swports[5]], timeout=10)

            self.client.switch_api_ipv6_hash_input_fields_set(device, SWITCH_HASH_IPV6_INPUT_PROT_DP_SIP_SP_DIP)
            send_packet(self, swports[1], str(pkt))
            rcv_port2 = verify_any_packet_any_port(self, [exp_pkt],
                                       [swports[2], swports[3], swports[4], swports[5]], timeout=10)
            self.assertTrue(rcv_port1!=rcv_port2)

            send_packet(self, swports[1], str(pkt))
            rcv_port3 = verify_any_packet_any_port(self, [exp_pkt],
                                       [swports[2], swports[3], swports[4], swports[5]], timeout=10)

            self.client.switch_api_ipv6_hash_algorithm_set(device, SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_DECT)
            send_packet(self, swports[1], str(pkt))
            rcv_port4 = verify_any_packet_any_port(self, [exp_pkt],
                                       [swports[2], swports[3], swports[4], swports[5]], timeout=10)

            self.client.switch_api_ipv6_hash_algorithm_set(device, SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_GENIBUS)
            send_packet(self, swports[1], str(pkt))
            rcv_port5 = verify_any_packet_any_port(self, [exp_pkt],
                                       [swports[2], swports[3], swports[4], swports[5]], timeout=10)

            self.client.switch_api_ipv6_hash_algorithm_set(device, SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_DNP)
            send_packet(self, swports[1], str(pkt))
            rcv_port6 = verify_any_packet_any_port(self, [exp_pkt],
                                       [swports[2], swports[3], swports[4], swports[5]], timeout=10)

            self.client.switch_api_ipv6_hash_algorithm_set(device, SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_TELEDISK)
            send_packet(self, swports[1], str(pkt))
            rcv_port7 = verify_any_packet_any_port(self, [exp_pkt],
                                       [swports[2], swports[3], swports[4], swports[5]], timeout=10)

            self.assertTrue(not (rcv_port3==rcv_port4 and rcv_port3==rcv_port5 and rcv_port3==rcv_port6 and rcv_port3==rcv_port7))

            print "Running Hash Field Attribute Tests"

            print "Setting Hash field attribute SRC IP"
            status = self.client.switch_api_ipv6_hash_input_fields_attribute_set(device, SWITCH_HASH_IPV6_INPUT_PROT_DP_SIP_SP_DIP, SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_IP)
            self.assertTrue(status == 0)
            flags = self.client.switch_api_ipv6_hash_input_fields_attribute_get(device, SWITCH_HASH_IPV6_INPUT_PROT_DP_SIP_SP_DIP)
            self.assertTrue(flags.status == 0 and flags.attr_flags == SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_IP)

            rcv_ports = set()
            print "Sending packet over multiple DST Ports"
            port = 1
            max_itrs = 10
            for i in range(0, max_itrs):
                pkt = simple_tcpv6_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ipv6_dst='4001::1',
                    ipv6_src='5001::1',
                    ipv6_hlim=64,
                    tcp_dport=port)

                exp_pkt = simple_tcpv6_packet(
                    eth_dst='00:88:88:88:88:88',
                    eth_src='00:77:66:55:44:33',
                    ipv6_dst='4001::1',
                    ipv6_src='5001::1',
                    ipv6_hlim=63,
                    tcp_dport=port)
                send_packet(self, swports[1], str(pkt))
                rcv_port = verify_any_packet_any_port(self, [exp_pkt, exp_pkt, exp_pkt, exp_pkt],
                                           [swports[2], swports[3], swports[4], swports[5]], timeout=10)
                rcv_ports.add(rcv_port)
                port+=1
            self.assertTrue(len(rcv_ports)==1)

            rcv_ports = set()
            print "Sending packet from multiple source IP address"
            src_ip = socket.inet_pton(socket.AF_INET6, '5000::1')
            src_ip_arr = list(src_ip)
            max_itrs = 10
            for i in range(0, max_itrs):
                src_ip_addr = socket.inet_ntop(socket.AF_INET6, src_ip)
                pkt = simple_tcpv6_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ipv6_dst='4001::1',
                    ipv6_src=src_ip_addr,
                    ipv6_hlim=64)

                exp_pkt = simple_tcpv6_packet(
                    eth_dst='00:88:88:88:88:88',
                    eth_src='00:77:66:55:44:33',
                    ipv6_dst='4001::1',
                    ipv6_src=src_ip_addr,
                    ipv6_hlim=63)
                send_packet(self, swports[1], str(pkt))
                rcv_port = verify_any_packet_any_port(self, [exp_pkt, exp_pkt, exp_pkt, exp_pkt],
                                           [swports[2], swports[3], swports[4], swports[5]], timeout=10)
                rcv_ports.add(rcv_port)
                src_ip_arr[15] = chr(ord(src_ip_arr[15]) + 1)
                src_ip = ''.join(src_ip_arr)
            self.assertTrue(len(rcv_ports)!=1)

            print "Setting Hash field attribute SRC Port Dst Port"
            attr_flags = SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_PORT | SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_DST_PORT
            status = self.client.switch_api_ipv6_hash_input_fields_attribute_set(device, SWITCH_HASH_IPV6_INPUT_PROT_DP_SIP_SP_DIP, attr_flags)
            self.assertTrue(status == 0)
            flags = self.client.switch_api_ipv6_hash_input_fields_attribute_get(device, SWITCH_HASH_IPV6_INPUT_PROT_DP_SIP_SP_DIP)
            self.assertTrue(flags.status == 0 and flags.attr_flags == attr_flags)

            rcv_ports = set()
            print "Sending packet over multiple DST Ports"
            port = 1
            max_itrs = 10
            for i in range(0, max_itrs):
                pkt = simple_tcpv6_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ipv6_dst='4001::1',
                    ipv6_src='5001::1',
                    ipv6_hlim=64,
                    tcp_dport=port)

                exp_pkt = simple_tcpv6_packet(
                    eth_dst='00:88:88:88:88:88',
                    eth_src='00:77:66:55:44:33',
                    ipv6_dst='4001::1',
                    ipv6_src='5001::1',
                    ipv6_hlim=63,
                    tcp_dport=port)
                send_packet(self, swports[1], str(pkt))
                rcv_port = verify_any_packet_any_port(self, [exp_pkt, exp_pkt, exp_pkt, exp_pkt],
                                           [swports[2], swports[3], swports[4], swports[5]], timeout=10)
                rcv_ports.add(rcv_port)
                port+=1
            self.assertTrue(len(rcv_ports)!=1)

            rcv_ports = set()
            print "Sending packet over multiple SRC Ports"
            port = 1
            max_itrs = 10
            for i in range(0, max_itrs):
                pkt = simple_tcpv6_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ipv6_dst='4001::1',
                    ipv6_src='5001::1',
                    ipv6_hlim=64,
                    tcp_sport=port)

                exp_pkt = simple_tcpv6_packet(
                    eth_dst='00:88:88:88:88:88',
                    eth_src='00:77:66:55:44:33',
                    ipv6_dst='4001::1',
                    ipv6_src='5001::1',
                    ipv6_hlim=63,
                    tcp_sport=port)
                send_packet(self, swports[1], str(pkt))
                rcv_port = verify_any_packet_any_port(self, [exp_pkt, exp_pkt, exp_pkt, exp_pkt],
                                           [swports[2], swports[3], swports[4], swports[5]], timeout=10)
                rcv_ports.add(rcv_port)
                port+=1
            self.assertTrue(len(rcv_ports)!=1)

            rcv_ports = set()
            print "Sending packet from multiple source IP address"
            src_ip = socket.inet_pton(socket.AF_INET6, '5000::1')
            src_ip_arr = list(src_ip)
            max_itrs = 10
            for i in range(0, max_itrs):
                src_ip_addr = socket.inet_ntop(socket.AF_INET6, src_ip)
                pkt = simple_tcpv6_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ipv6_dst='4001::1',
                    ipv6_src=src_ip_addr,
                    ipv6_hlim=64)

                exp_pkt = simple_tcpv6_packet(
                    eth_dst='00:88:88:88:88:88',
                    eth_src='00:77:66:55:44:33',
                    ipv6_dst='4001::1',
                    ipv6_src=src_ip_addr,
                    ipv6_hlim=63)
                send_packet(self, swports[1], str(pkt))
                rcv_port = verify_any_packet_any_port(self, [exp_pkt, exp_pkt, exp_pkt, exp_pkt],
                                           [swports[2], swports[3], swports[4], swports[5]], timeout=10)
                rcv_ports.add(rcv_port)
                src_ip_arr[15] = chr(ord(src_ip_arr[15]) + 1)
                src_ip = ''.join(src_ip_arr)
            self.assertTrue(len(rcv_ports)==1)


        finally:
            self.client.switch_api_ipv6_hash_algorithm_set(device, default_algo_res.algorithm)
            self.client.switch_api_ipv6_hash_input_fields_set(device, default_input_fields_res.fields)
            self.client.switch_api_ipv6_hash_input_fields_attribute_set(device, default_input_fields_res.fields, default_attr_res.attr_flags)

            self.client.switch_api_neighbor_delete(device, neighbor)

            self.client.switch_api_l3_route_delete(device, vrf, i_ip3, nhop)
            self.client.switch_api_nhop_delete(device, nhop)

            self.client.switch_api_l3_interface_address_delete(device, rif1,
                                                               vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(device, rif2,
                                                               vrf, i_ip2)

            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port2)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port3)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port4)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port5)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_lag_delete(device, lag)
            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)

###############################################################################
@group('l3')
@group('ipv6')
@group('maxsizes')
@group('ent')
class L3IPv6LagTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Sending packet port %d" % swports[1], " -> port %d" % swports[
            2], " (4001::1 -> 5001::1[id = 101])"
        vrf = self.client.switch_api_vrf_create(device, 2)
        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])
        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[4])

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
            ipaddr='5001::10',
            prefix_length=120)
        self.client.switch_api_l3_interface_address_add(device, rif1, vrf,
                                                        i_ip1)

        lag = self.client.switch_api_lag_create(device)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port2)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port3)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port4)
        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v6_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=lag, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='4001::10',
            prefix_length=120)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf,
                                                        i_ip2)

        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='4001::1',
            prefix_length=128)
        nhop, neighbor = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip3, '00:88:88:88:88:88')
        self.client.switch_api_l3_route_add(device, vrf, i_ip3, nhop)

        try:
            pkt = simple_tcpv6_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ipv6_dst='4001::1',
                ipv6_src='5001::1',
                ipv6_hlim=64)

            exp_pkt = simple_tcpv6_packet(
                eth_dst='00:88:88:88:88:88',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='4001::1',
                ipv6_src='5001::1',
                ipv6_hlim=63)
            send_packet(self, swports[1], str(pkt))
            verify_any_packet_any_port(self, [exp_pkt],
                                       [swports[2], swports[3], swports[4]], timeout=2)
        finally:
            self.client.switch_api_neighbor_delete(device, neighbor)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip3, nhop)
            self.client.switch_api_nhop_delete(device, nhop)

            self.client.switch_api_l3_interface_address_delete(device, rif1,
                                                               vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(device, rif2,
                                                               vrf, i_ip2)

            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port2)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port3)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port4)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_lag_delete(device, lag)
            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('l3')
@group('ipv4')
@group('clpm')
@group('maxsizes')
@group('ent')
class L3EcmpLagTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Sending packet port %d" % swports[1], " -> ecmp -> lag"
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

        try:
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
                    ], timeout=2)
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
        finally:
            self.client.switch_api_l3_route_delete(device, vrf, i_ip5, ecmp)

            self.client.switch_api_ecmp_member_delete(device, ecmp, 3,
                                                      [nhop1, nhop2, nhop3])
            self.client.switch_api_ecmp_delete(device, ecmp)

            self.client.switch_api_neighbor_delete(device, neighbor1)
            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_neighbor_delete(device, neighbor2)
            self.client.switch_api_nhop_delete(device, nhop2)

            self.client.switch_api_neighbor_delete(device, neighbor3)
            self.client.switch_api_nhop_delete(device, nhop3)

            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag1,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port2)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag1,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port3)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag1,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port4)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag2,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port5)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag2,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port6)

            self.client.switch_api_l3_interface_address_delete(device, rif1,
                                                               vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(device, rif2,
                                                               vrf, i_ip2)
            self.client.switch_api_l3_interface_address_delete(device, rif3,
                                                               vrf, i_ip3)
            self.client.switch_api_l3_interface_address_delete(device, rif4,
                                                               vrf, i_ip4)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)
            self.client.switch_api_interface_delete(device, if4)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)
            self.client.switch_api_rif_delete(0, rif3)
            self.client.switch_api_rif_delete(0, rif4)

            self.client.switch_api_lag_delete(device, lag1)
            self.client.switch_api_lag_delete(device, lag2)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('l3')
@group('ipv4')
@group('urpf')
@group('2porttests')
class L3RpfTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_urpf_mode=1,
            v4_unicast_enabled=1)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)
        intf_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='192.168.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif1, vrf,
                                                        intf_ip1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_urpf_mode=2,
            v4_unicast_enabled=1)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        intf_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.2.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf,
                                                        intf_ip2)

        # add neighbor 192.168.0.1
        i_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='192.168.0.1',
            prefix_length=32)

        # add neighbor 10.0.0.2
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=32)
        nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, rif1, i_ip1, '00:11:22:33:44:55')
        nhop2, neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip2, '00:11:22:33:44:56')

        # Add a static route 10.10/16 --> if1
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.0.0',
            prefix_length=16)
        self.client.switch_api_l3_route_add(device, vrf, i_ip3, nhop1)

        # Add a static route 10.11/16 --> if2
        i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.11.0.0',
            prefix_length=16)
        self.client.switch_api_l3_route_add(device, vrf, i_ip4, nhop2)

        # Add a static route 10.13/16 --> if1
        i_ip5 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.13.0.0',
            prefix_length=16)
        self.client.switch_api_l3_route_add(device, vrf, i_ip5, nhop1)

        # send the test packet(s)
        try:
            print "Sending packet port %d" % swports[
                0], " -> port %d" % swports[1], ". Loose urpf (permit)"
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.11.10.1',
                ip_src='10.10.10.1',
                ip_id=114,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:22:33:44:56',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.11.10.1',
                ip_src='10.10.10.1',
                ip_id=114,
                ip_ttl=63)
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

            print "Sending packet port %d" % swports[
                0], " -> port %d" % swports[1], ". Loose urpf (drop)"
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.11.10.1',
                ip_src='10.12.10.1',
                ip_id=114,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:22:33:44:56',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.11.10.1',
                ip_src='10.12.10.1',
                ip_id=114,
                ip_ttl=63)
            send_packet(self, swports[0], str(pkt))
            verify_no_other_packets(self, timeout=1)

            print "Sending packet port %d" % swports[
                1], " -> port %d" % swports[0], ". Strict urpf (permit)"
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:11:11:11:11:11',
                ip_dst='10.10.10.1',
                ip_src='10.11.10.1',
                ip_id=114,
                ip_ttl=64)

            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='10.11.10.1',
                ip_id=114,
                ip_ttl=63)
            send_packet(self, swports[1], str(pkt))
            verify_packets(self, exp_pkt, [swports[0]])

            print "Sending packet port %d" % swports[
                1], " -> port %d" % swports[0], ". Strict urpf (miss drop)"
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:11:11:11:11:11',
                ip_dst='10.10.10.1',
                ip_src='10.12.10.1',
                ip_id=114,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='10.12.10.1',
                ip_id=114,
                ip_ttl=63)
            send_packet(self, swports[1], str(pkt))
            verify_no_other_packets(self, timeout=1)

            print "Sending packet port %d" % swports[
                1], " -> port %d" % swports[0], ". Strict urpf (hit drop)"
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:11:11:11:11:11',
                ip_dst='10.10.10.1',
                ip_src='10.13.10.1',
                ip_id=114,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='10.13.10.1',
                ip_id=114,
                ip_ttl=63)
            send_packet(self, swports[1], str(pkt))
            verify_no_other_packets(self, timeout=1)
        finally:
            self.client.switch_api_l3_route_delete(device, vrf, i_ip3, nhop1)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip4, nhop2)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip5, nhop1)

            self.client.switch_api_neighbor_delete(device, neighbor1)
            self.client.switch_api_neighbor_delete(device, neighbor2)

            self.client.switch_api_nhop_delete(device, nhop1)
            self.client.switch_api_nhop_delete(device, nhop2)

            self.client.switch_api_l3_interface_address_delete(device, rif1,
                                                               vrf, intf_ip1)
            self.client.switch_api_l3_interface_address_delete(device, rif2,
                                                               vrf, intf_ip2)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('bfd')
@group('l2')
@group('maxsizes')
@group('ent')
class L2StaticMacBulkDeleteTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "L2 mac bulk delete"
        vlan = self.client.switch_api_vlan_create(device, 10)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if3 = self.client.switch_api_interface_create(device, i_info3)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)
        self.client.switch_api_vlan_member_add(device, vlan, if3)

        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:01', 2, if1)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:02', 2, if1)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:03', 2, if1)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:04', 2, if1)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:05', 2, if1)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:06', 2, if1)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:07', 2, if1)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:08', 2, if1)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:09', 2, if1)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:0a', 2, if1)

        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:01', 2, if2)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:02', 2, if2)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:03', 2, if2)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:04', 2, if2)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:05', 2, if2)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:06', 2, if2)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:07', 2, if2)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:08', 2, if2)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:09', 2, if2)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:0a', 2, if2)

        print "L2 mac delete by interface if1"
        self.client.switch_api_mac_table_entry_flush(
            device, SWITCH_MAC_FLUSH_TYPE_INTERFACE, 0x0, if1)
        print "L2 mac delete by interface if2"
        self.client.switch_api_mac_table_entry_flush(
            device, SWITCH_MAC_FLUSH_TYPE_INTERFACE, 0x0, if2)

        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:01', 2, if1)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:02', 2, if1)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:03', 2, if1)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:04', 2, if1)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:05', 2, if1)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:06', 2, if1)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:07', 2, if1)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:08', 2, if1)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:09', 2, if1)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:0a', 2, if1)

        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:01', 2, if2)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:02', 2, if2)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:03', 2, if2)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:04', 2, if2)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:05', 2, if2)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:06', 2, if2)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:07', 2, if2)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:08', 2, if2)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:09', 2, if2)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:0a', 2, if2)

        print "L2 mac delete by vlan"
        self.client.switch_api_mac_table_entry_flush(
            device, SWITCH_MAC_FLUSH_TYPE_NETWORK, vlan, 0x0)

        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:01', 2, if1)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:02', 2, if1)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:03', 2, if1)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:04', 2, if1)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:11:00:00:00:05', 2, if1)

        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:01', 2, if2)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:02', 2, if2)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:03', 2, if2)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:04', 2, if2)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:22:00:00:00:05', 2, if2)

        switch_api_mac_table_entry_create(self,
            device, vlan, '00:33:00:00:00:01', 2, if3)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:33:00:00:00:02', 2, if3)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:33:00:00:00:03', 2, if3)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:33:00:00:00:04', 2, if3)
        switch_api_mac_table_entry_create(self,
            device, vlan, '00:33:00:00:00:05', 2, if3)

        print "L2 mac delete all"
        self.client.switch_api_mac_table_entry_flush(
            device, SWITCH_MAC_FLUSH_TYPE_ALL, 0x0, 0x0)

        self.client.switch_api_vlan_member_remove(device, vlan, if1)
        self.client.switch_api_vlan_member_remove(device, vlan, if2)
        self.client.switch_api_vlan_member_remove(device, vlan, if3)

        self.client.switch_api_interface_delete(device, if1)
        self.client.switch_api_interface_delete(device, if2)
        self.client.switch_api_interface_delete(device, if3)

        self.client.switch_api_vlan_delete(device, vlan)


###############################################################################
@group('bfd')
@group('l2')
@group('l3')
@group('tunnel')
@group('maxsizes')
@group('2porttests')
@group('ent')
class L2VxlanUnicastBasicTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        self.vrf_id = 10
        self.underlay_vrf_id = 20
        self.fp_ports = [swports[0], swports[1]]
        self.port_h = [0] * len(self.fp_ports)
        self.rif_h = [0] * len(self.fp_ports)
        self.rif_ip = '192.168.0.2'
        self.host_mac = '00:11:22:33:44:55'
        self.port_h = [0] * len(self.fp_ports)
        self.intf_h = [0] * len(self.fp_ports)
        self.vni = 10000
        self.mac_type = SWITCH_MAC_ENTRY_STATIC
        self.tunnel_type = SWITCH_TUNNEL_TYPE_VXLAN
        self.tunnel_map_type = [SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE,
                                SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI]
        self.mapper_h = [0] * 2
        self.mapper_entry_h = [0] * 2
        self.tunnel_src_ip = '1.1.1.1'
        self.encap_src_ip = '1.1.1.3'
        self.tunnel_dst_ip = '1.1.1.3'
        self.nhop_mac = '00:33:33:33:33:33'
        self.entry_type = SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P
        self.host_mac = ['00:11:11:11:11:11', '00:22:22:22:22:22']

        self.vrf = self.add_vrf(device, self.vrf_id)
        self.underlay_vrf = self.add_vrf(device, self.underlay_vrf_id)
        self.rmac = self.add_rmac(device, '00:77:66:55:44:33', rmac_type='all')

        for index in range(0, len(self.fp_ports)):
          self.port_h[index] = self.select_port(device, swports[index])

        self.if_h = self.cfg_l2intf_on_port(device, self.port_h[0])

        self.rif_h = self.create_l3_rif(
                                   device,
                                   self.underlay_vrf,
                                   self.rmac,
                                   self.port_h[1],
                                   self.rif_ip)

        self.ln = self.create_logical_network(device)
        self.add_logical_network_member(device, self.ln, self.if_h)

        for index in range(0, len(self.tunnel_map_type)):
          self.mapper_h[index] = self.create_tunnel_mapper(
                              device=device,
                              map_type=self.tunnel_map_type[index],
                              tunnel_type=self.tunnel_type)
          self.mapper_entry_h[index] = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=self.tunnel_map_type[index],
                              mapper=self.mapper_h[index],
                              handle=self.ln,
                              vni=self.vni)

        self.underlay_lb_h = self.create_loopback_rif(device, self.underlay_vrf, self.rmac)
        self.overlay_lb_h = self.create_loopback_rif(device, self.vrf, self.rmac)

        self.tunnel_h = self.create_tunnel_table(
                              device=device,
                              tunnel_type=self.tunnel_type,
                              src_ip=self.encap_src_ip,
                              imapper_h=self.mapper_h[0],
                              emapper_h=self.mapper_h[1],
                              urif=self.underlay_lb_h,
                              orif=self.overlay_lb_h)

        self.tunnel_term_h = self.create_tunnel_term(
                              device=device,
                              tunnel_type=self.tunnel_type,
                              vrf=self.underlay_vrf,
                              tunnel=self.tunnel_h,
                              entry_type=self.entry_type,
                              src_ip=self.tunnel_src_ip,
                              dst_ip=self.tunnel_dst_ip)

        self.tunnel_if_h = self.create_tunnel_interface(
                              device=device,
                              tunnel=self.tunnel_h)

        self.nhop1 = self.add_l3_nhop(device, self.rif_h, self.tunnel_src_ip, self.nhop_mac)
        self.add_static_route(device, self.underlay_vrf, self.tunnel_src_ip, self.nhop1)

        self.add_mac_table_entry(device, self.ln, self.host_mac[0], self.mac_type, self.if_h)
        self.add_mac_table_entry(device, self.ln, self.host_mac[1],
                                 self.mac_type, self.tunnel_if_h,
                                 tunnel=True, tunnel_ip=self.tunnel_src_ip)

    def runTest(self):
        try:
            print "Sending packet from Access port1 to Vxlan port2"
            pkt = simple_tcp_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            udp_sport = entropy_hash(pkt)
            vxlan_pkt = simple_vxlan_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:33:33:33:33:33',
                ip_id=0,
                ip_src='1.1.1.3',
                ip_dst='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=self.vni,
                inner_frame=pkt)
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, vxlan_pkt, [swports[1]])

            print "Sending packet from Vxlan port2 to Access port1"
            pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            vxlan_pkt = simple_vxlan_packet(
                eth_src='00:33:33:33:33:33',
                eth_dst='00:77:66:55:44:33',
                ip_id=0,
                ip_src='1.1.1.1',
                ip_dst='1.1.1.3',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=11638,
                with_udp_chksum=False,
                vxlan_vni=self.vni,
                inner_frame=pkt)
            send_packet(self, swports[1], str(vxlan_pkt))
            verify_packets(self, pkt, [swports[0]])
        finally:
            print
            print "Packet from port %s to port %s on vrf %s" % (swports[0], swports[1], self.vrf_id)

    def tearDown(self):
      self.cleanup()
      api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('tunnel')
@group('maxsizes')
@group('2porttests')
@group('ent')
class L2VxlanArpUnicastBasicTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "L2 Vxlan Basic Mode Unicast Test"

        if ((test_param_get('target') == 'bmv2') or
            (test_param_get('target') == 'bmv2' and
             test_param_get('arch') == 'Tofino')):
            return

        vrf = self.add_vrf(device, 2)
        rmac = self.add_rmac(device, '00:77:66:55:44:33', rmac_type='all')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        self.if1 = self.cfg_l2intf_on_port(device, port1)

        rif2 = self.create_l3_rif(
                           device,
                           vrf,
                           rmac,
                           port2,
                           '192.168.10.2')

        ln1 = self.create_logical_network(device)
        self.add_logical_network_member(device, ln1, self.if1)

        imapper_h = self.create_tunnel_mapper(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN)
        imapper_entry_h = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE,
                              mapper=imapper_h,
                              handle=ln1,
                              vni=0x1234)
        emapper_h = self.create_tunnel_mapper(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN)
        emapper_entry_h = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI,
                              mapper=emapper_h,
                              handle=ln1,
                              vni=0x1234)

        underlay_lb_h = self.create_loopback_rif(device, vrf, rmac)
        overlay_lb_h = self.create_loopback_rif(device, vrf, rmac)

        tunnel_h = self.create_tunnel_table(
                              device=device,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
                              src_ip='1.1.1.3',
                              imapper_h=imapper_h,
                              emapper_h=emapper_h,
                              urif=underlay_lb_h,
                              orif=overlay_lb_h)

        tunnel_term_h = self.create_tunnel_term(
                              device=device,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
                              vrf=vrf,
                              tunnel=tunnel_h,
                              entry_type=SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P,
                              src_ip='1.1.1.1',
                              dst_ip='1.1.1.3')

        tunnel_if_h = self.create_tunnel_interface(
                              device=device,
                              tunnel=tunnel_h)

        nhop = self.add_l3_nhop(device, rif2, '1.1.1.1', '00:33:33:33:33:33')
        self.add_static_route(device, vrf, '1.1.1.1', nhop)

        self.add_mac_table_entry(device, ln1, '00:11:11:11:11:11', 2, self.if1)
        self.add_mac_table_entry(device, ln1, '00:22:22:22:22:22', 2, tunnel_if_h, tunnel=True, tunnel_ip='1.1.1.1')

    def runTest(self):
        if ((test_param_get('target') == 'bmv2') or
            (test_param_get('target') == 'bmv2' and
             test_param_get('arch') == 'Tofino')):
            return

        try:
            print "Sending packet from Access port1 to Vxlan port2"
            pkt = simple_arp_packet(
                arp_op=2,
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                pktlen=100)

            ingress_ifindex = self.client.switch_api_interface_ifindex_get(
                device, self.if1)
            udp_sport = entropy_hash(
                pkt, layer='ether', ifindex=ingress_ifindex)

            if test_param_get('target') == "asic-model" or test_param_get('target') == "hw":
                ba = 4
            else:
                ba = 0

            vxlan_pkt = simple_vxlan_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:33:33:33:33:33',
                ip_id=0,
                ip_dst='1.1.1.1',
                ip_src='1.1.1.3',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)
            # asic-model added 4 bytes of FCS to arp pkt in ingress. These 4 bytes
            # were added to IP and UDP hdr of vxlan pkt created in asic-model.
            # Account for that in IP and UDP header of expected packet.
            vxlan_pkt['IP'].len = len(vxlan_pkt['IP']) + ba
            vxlan_pkt['UDP'].len = len(vxlan_pkt['UDP']) + ba

            send_packet(self, swports[0], str(pkt))
            verify_packets(self, vxlan_pkt, [swports[1]])

            print "Sending packet from Vxlan port2 to Access port1"
            pkt = simple_arp_packet(arp_op=1, pktlen=100)
            vxlan_pkt = simple_vxlan_packet(
                eth_src='00:33:33:33:33:33',
                eth_dst='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=11638,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)
            send_packet(self, swports[1], str(vxlan_pkt))
            verify_packets(self, pkt, [swports[0]])

        finally:
            print
            print "Packet from port %s to port %s" % (swports[0], swports[1])

    def tearDown(self):
      if ((test_param_get('target') == 'bmv2') or
          (test_param_get('target') == 'bmv2' and
           test_param_get('arch') == 'Tofino')):
        return

      self.cleanup()
      api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('l2')
@group('l3')
@group('ipv6')
@group('tunnel')
@group('maxsizes')
@group('2porttests')
class L2IPv4InIPv6VxlanUnicastBasicTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "L2 Vxlan Over IPv6 Basic Mode Unicast Test"

        self.src_ip = '1234:5678:9abc:def0:1234:5678:9abc:def0'
        self.dst_ip = '1111:2222:3333:4444:5555:6666:7777:8888'

        vrf = self.add_vrf(device, 2)
        rmac = self.add_rmac(device, '00:77:66:55:44:33', rmac_type='all')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        self.if1 = self.cfg_l2intf_on_port(device, port1)

        rif2 = self.create_l3_rif(
                           device,
                           vrf,
                           rmac,
                           port2,
                           '2000::1',
                           v4=False)

        ln1 = self.create_logical_network(device)
        self.add_logical_network_member(device, ln1, self.if1)

        imapper_h = self.create_tunnel_mapper(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
                              tunnel_ip_type=SWITCH_TUNNEL_IP_ADDR_TYPE_IPV6)
        imapper_entry_h = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE,
                              mapper=imapper_h,
                              handle=ln1,
                              vni=0x1234)
        emapper_h = self.create_tunnel_mapper(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
                              tunnel_ip_type=SWITCH_TUNNEL_IP_ADDR_TYPE_IPV6)
        emapper_entry_h = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI,
                              mapper=emapper_h,
                              handle=ln1,
                              vni=0x1234)

        underlay_lb_h = self.create_loopback_rif(device, vrf, rmac)
        overlay_lb_h = self.create_loopback_rif(device, vrf, rmac)

        tunnel_h = self.create_tunnel_table(
                              device=device,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
                              src_ip=self.src_ip,
                              imapper_h=imapper_h,
                              emapper_h=emapper_h,
                              tunnel_ip_type=SWITCH_TUNNEL_IP_ADDR_TYPE_IPV6,
                              urif=underlay_lb_h,
                              orif=overlay_lb_h,
                              v4=False)

        tunnel_term_h = self.create_tunnel_term(
                              device=device,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
                              vrf=vrf,
                              tunnel=tunnel_h,
                              entry_type=SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P,
                              src_ip=self.dst_ip,
                              dst_ip=self.src_ip,
                              v4=False)

        tunnel_if_h = self.create_tunnel_interface(
                              device=device,
                              tunnel=tunnel_h)

        nhop = self.add_l3_nhop(device, rif2, self.dst_ip, '00:33:33:33:33:33', v4=False)
        self.add_static_route(device, vrf, self.dst_ip, nhop, v4=False)

        self.add_mac_table_entry(device, ln1, '00:11:11:11:11:11', 2, self.if1)
        self.add_mac_table_entry(device, ln1, '00:22:22:22:22:22', 2, tunnel_if_h, tunnel=True, tunnel_ip=self.dst_ip, v4=False)

    def runTest(self):
        try:
            print "Sending packet from Access port1 to Vxlan port2"
            pkt = simple_tcp_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            udp_sport = entropy_hash(pkt)
            vxlan_pkt = simple_vxlanv6_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:33:33:33:33:33',
                ipv6_dst=self.dst_ip,
                ipv6_src=self.src_ip,
                ipv6_hlim=64,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, vxlan_pkt, [swports[1]])


            print "Sending packet from Access port1 to Vxlan port2"
            pkt = simple_tcpv6_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                ipv6_dst='1234:5678:9abc:def0:4422:1133:5577:99aa',
                ipv6_src='2000::1',
                ipv6_hlim=64)
            udp_sport = entropy_hash(pkt, 'ipv6')
            vxlan_pkt = simple_vxlanv6_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:33:33:33:33:33',
                ipv6_dst=self.dst_ip,
                ipv6_src=self.src_ip,
                ipv6_hlim=64,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, vxlan_pkt, [swports[1]])

            print "Sending packet from Vxlan port2 to Access Port1"
            pkt = simple_tcpv6_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:22:22:22:22:22',
                ipv6_dst='3000::1',
                ipv6_src='2000::1',
                ipv6_hlim=64)
            vxlan_pkt = simple_vxlanv6_packet(
                eth_src='00:33:33:33:33:33',
                eth_dst='00:77:66:55:44:33',
                ipv6_dst=self.src_ip,
                ipv6_src=self.dst_ip,
                ipv6_hlim=64,
                udp_sport=0,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)
            send_packet(self, swports[1], str(vxlan_pkt))
            verify_packets(self, pkt, [swports[0]])
        finally:
            print
            print "Packet from port %s to port %s" % (swports[0], swports[1])

    def tearDown(self):
        self.cleanup()
        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('bfd')
@group('l2')
@group('l3')
@group('tunnel')
@group('maxsizes')
@group('2porttests')
@group('mcast')
@group('non-vxlan-tunnel')
class L2NvgreUnicastBasicTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        self.vrf_id = 10
        self.underlay_vrf_id = 20
        self.fp_ports = [swports[0], swports[1]]
        self.port_h = [0] * len(self.fp_ports)
        self.rif_h = [0] * len(self.fp_ports)
        self.rif_ip = '192.168.0.2'
        self.host_mac = '00:11:22:33:44:55'
        self.port_h = [0] * len(self.fp_ports)
        self.intf_h = [0] * len(self.fp_ports)
        self.vni = 10000
        self.mac_type = SWITCH_MAC_ENTRY_STATIC
        self.tunnel_type = SWITCH_TUNNEL_TYPE_NVGRE
        self.tunnel_map_type = [SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE,
                                SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI]
        self.mapper_h = [0] * 2
        self.mapper_entry_h = [0] * 2
        self.tunnel_src_ip = '1.1.1.1'
        self.encap_src_ip = '1.1.1.3'
        self.tunnel_dst_ip = '1.1.1.3'
        self.nhop_mac = '00:33:33:33:33:33'
        self.entry_type = SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P
        self.host_mac = ['00:11:11:11:11:11', '00:22:22:22:22:22']

        self.vrf = self.add_vrf(device, self.vrf_id)
        self.underlay_vrf = self.add_vrf(device, self.underlay_vrf_id)
        self.rmac = self.add_rmac(device, '00:77:66:55:44:33', rmac_type='all')

        for index in range(0, len(self.fp_ports)):
          self.port_h[index] = self.select_port(device, swports[index])

        self.if_h = self.cfg_l2intf_on_port(device, self.port_h[0])

        self.rif_h = self.create_l3_rif(
                                   device,
                                   self.underlay_vrf,
                                   self.rmac,
                                   self.port_h[1],
                                   self.rif_ip)

        self.ln = self.create_logical_network(device)
        self.add_logical_network_member(device, self.ln, self.if_h)

        for index in range(0, len(self.tunnel_map_type)):
          self.mapper_h[index] = self.create_tunnel_mapper(
                              device=device,
                              map_type=self.tunnel_map_type[index],
                              tunnel_type=self.tunnel_type)
          self.mapper_entry_h[index] = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=self.tunnel_map_type[index],
                              mapper=self.mapper_h[index],
                              handle=self.ln,
                              vni=self.vni)

        self.underlay_lb_h = self.create_loopback_rif(device, self.underlay_vrf, self.rmac)
        self.overlay_lb_h = self.create_loopback_rif(device, self.vrf, self.rmac)

        self.tunnel_h = self.create_tunnel_table(
                              device=device,
                              tunnel_type=self.tunnel_type,
                              src_ip=self.encap_src_ip,
                              imapper_h=self.mapper_h[0],
                              emapper_h=self.mapper_h[1],
                              urif=self.underlay_lb_h,
                              orif=self.overlay_lb_h)

        self.tunnel_term_h = self.create_tunnel_term(
                              device=device,
                              tunnel_type=self.tunnel_type,
                              vrf=self.underlay_vrf,
                              tunnel=self.tunnel_h,
                              entry_type=self.entry_type,
                              src_ip=self.tunnel_src_ip,
                              dst_ip=self.tunnel_dst_ip)

        self.tunnel_if_h = self.create_tunnel_interface(
                              device=device,
                              tunnel=self.tunnel_h)

        self.nhop1 = self.add_l3_nhop(device, self.rif_h, self.tunnel_src_ip, self.nhop_mac)
        self.add_static_route(device, self.underlay_vrf, self.tunnel_src_ip, self.nhop1)

        self.add_mac_table_entry(device, self.ln, self.host_mac[0], self.mac_type, self.if_h)
        self.add_mac_table_entry(device, self.ln, self.host_mac[1],
                                 self.mac_type, self.tunnel_if_h,
                                 tunnel=True, tunnel_ip=self.tunnel_src_ip)

    def runTest(self):
        try:
            print "L2 Nvgre Basic Mode Unicast Test"
            print "Sending packet from Access port1 to Nvgre port2"
            pkt = simple_tcp_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            nvgre_flowid = entropy_hash(pkt) & 0xFF
            nvgre_pkt = simple_nvgre_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:33:33:33:33:33',
                ip_id=0,
                ip_src='1.1.1.3',
                ip_dst='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                nvgre_tni=self.vni,
                nvgre_flowid=nvgre_flowid,
                inner_frame=pkt)
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, nvgre_pkt, [swports[1]])

            print "Sending packet from Nvgre port2 to Access port1"
            pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            nvgre_pkt = simple_nvgre_packet(
                eth_src='00:33:33:33:33:33',
                eth_dst='00:77:66:55:44:33',
                ip_id=0,
                ip_src='1.1.1.1',
                ip_dst='1.1.1.3',
                ip_ttl=64,
                ip_flags=0x2,
                nvgre_tni=self.vni,
                inner_frame=pkt)
            send_packet(self, swports[1], str(nvgre_pkt))
            verify_packets(self, pkt, [swports[0]])
        finally:
            print
            print "Packet from port %s to port %s on vrf %s" % (swports[0], swports[1], self.vrf_id)

    def tearDown(self):
      self.cleanup()
      api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('bfd')
@group('l2')
@group('l3')
@group('tunnel')
@group('maxsizes')
@group('ent')
class L2VxlanUnicastLagBasicTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        self.vrf_id = 10
        self.underlay_vrf_id = 20
        self.fp_ports = [swports[1], swports[2], swports[3], swports[4]]
        self.port_h = [0] * len(self.fp_ports)
        self.rif_h = [0] * len(self.fp_ports)
        self.rif_ip = '192.168.0.2'
        self.host_mac = '00:11:22:33:44:55'
        self.port_h = [0] * len(self.fp_ports)
        self.intf_h = [0] * len(self.fp_ports)
        self.vni = 10000
        self.mac_type = SWITCH_MAC_ENTRY_STATIC
        self.tunnel_type = SWITCH_TUNNEL_TYPE_VXLAN
        self.tunnel_map_type = [SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE,
                                SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI]
        self.mapper_h = [0] * 2
        self.mapper_entry_h = [0] * 2
        self.tunnel_src_ip = '1.1.1.1'
        self.encap_src_ip = '1.1.1.3'
        self.tunnel_dst_ip = '1.1.1.3'
        self.nhop_mac = '00:33:33:33:33:33'
        self.entry_type = SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P
        self.host_mac = ['00:11:11:11:11:11', '00:22:22:22:22:22']

        self.vrf = self.add_vrf(device, self.vrf_id)
        self.underlay_vrf = self.add_vrf(device, self.underlay_vrf_id)
        self.rmac = self.add_rmac(device, '00:77:66:55:44:33', rmac_type='all')

        for index in range(0, len(self.fp_ports)):
          self.port_h[index] = self.select_port(device, self.fp_ports[index])

        self.lag1_h = self.add_lag(device)
        self.add_lag_member(device, self.lag1_h, self.port_h[0])
        self.add_lag_member(device, self.lag1_h, self.port_h[1])

        self.lag2_h = self.add_lag(device)
        self.add_lag_member(device, self.lag2_h, self.port_h[2])
        self.add_lag_member(device, self.lag2_h, self.port_h[3])

        self.if_h = self.cfg_l2intf_on_port(device, self.lag1_h)

        self.rif_h = self.create_l3_rif(
                                   device,
                                   self.underlay_vrf,
                                   self.rmac,
                                   self.lag2_h,
                                   self.rif_ip)

        self.ln = self.create_logical_network(device)
        self.add_logical_network_member(device, self.ln, self.if_h)

        for index in range(0, len(self.tunnel_map_type)):
          self.mapper_h[index] = self.create_tunnel_mapper(
                              device=device,
                              map_type=self.tunnel_map_type[index],
                              tunnel_type=self.tunnel_type)
          self.mapper_entry_h[index] = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=self.tunnel_map_type[index],
                              mapper=self.mapper_h[index],
                              handle=self.ln,
                              vni=self.vni)

        self.underlay_lb_h = self.create_loopback_rif(device, self.underlay_vrf, self.rmac)
        self.overlay_lb_h = self.create_loopback_rif(device, self.vrf, self.rmac)

        self.tunnel_h = self.create_tunnel_table(
                              device=device,
                              tunnel_type=self.tunnel_type,
                              src_ip=self.encap_src_ip,
                              imapper_h=self.mapper_h[0],
                              emapper_h=self.mapper_h[1],
                              urif=self.underlay_lb_h,
                              orif=self.overlay_lb_h)

        self.tunnel_term_h = self.create_tunnel_term(
                              device=device,
                              tunnel_type=self.tunnel_type,
                              vrf=self.underlay_vrf,
                              tunnel=self.tunnel_h,
                              entry_type=self.entry_type,
                              src_ip=self.tunnel_src_ip,
                              dst_ip=self.tunnel_dst_ip)

        self.tunnel_if_h = self.create_tunnel_interface(
                              device=device,
                              tunnel=self.tunnel_h)

        self.nhop1 = self.add_l3_nhop(device, self.rif_h, self.tunnel_src_ip, self.nhop_mac)
        self.add_static_route(device, self.underlay_vrf, self.tunnel_src_ip, self.nhop1)

        self.add_mac_table_entry(device, self.ln, self.host_mac[0], self.mac_type, self.if_h)
        self.add_mac_table_entry(device, self.ln, self.host_mac[1],
                                 self.mac_type, self.tunnel_if_h,
                                 tunnel=True, tunnel_ip=self.tunnel_src_ip)

    def runTest(self):
        try:
            pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            vxlan_pkt = simple_vxlan_packet(
                eth_src='00:33:33:33:33:33',
                eth_dst='00:77:66:55:44:33',
                ip_id=0,
                ip_src='1.1.1.1',
                ip_dst='1.1.1.3',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=11638,
                with_udp_chksum=False,
                vxlan_vni=self.vni,
                inner_frame=pkt)

            print "Sending packet from Lag Vxlan port3 to Access Lag"
            send_packet(self, swports[3], str(vxlan_pkt))
            verify_any_packet_any_port(self, [pkt], [swports[1], swports[2]], timeout=2)

            print "Sending packet from Lag Vxlan port4 to Access Lag"
            send_packet(self, swports[4], str(vxlan_pkt))
            verify_any_packet_any_port(self, [pkt], [swports[1], swports[2]], timeout=2)
            pkt = simple_tcp_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            udp_sport = entropy_hash(pkt)
            vxlan_pkt = simple_vxlan_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:33:33:33:33:33',
                ip_id=0,
                ip_src='1.1.1.3',
                ip_dst='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=self.vni,
                inner_frame=pkt)

            print "Sending packet from Lag Access port1 to Vxlan Lag"
            send_packet(self, swports[1], str(pkt))
            verify_any_packet_any_port(self, [vxlan_pkt],
                                       [swports[3], swports[4]])

            print "Sending packet from Lag Access port2 to Vxlan Lag"
            send_packet(self, swports[2], str(pkt))
            verify_any_packet_any_port(self, [vxlan_pkt],
                                       [swports[3], swports[4]])
        finally:
            print
            print "Packet from port %s to port %s on vrf %s" % (swports[0], swports[1], self.vrf_id)

    def tearDown(self):
      self.cleanup()
      api_base_tests.ThriftInterfaceDataPlane.tearDown(self)

###############################################################################
@group('bfd')
@group('l2')
@group('l3')
@group('tunnel')
@group('maxsizes')
@group('2porttests')
@group('mcast')
@group('non-vxlan-tunnel')
class L2GeneveUnicastBasicTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        self.vrf_id = 10
        self.underlay_vrf_id = 20
        self.fp_ports = [swports[0], swports[1]]
        self.port_h = [0] * len(self.fp_ports)
        self.rif_h = [0] * len(self.fp_ports)
        self.rif_ip = '192.168.0.2'
        self.host_mac = '00:11:22:33:44:55'
        self.port_h = [0] * len(self.fp_ports)
        self.intf_h = [0] * len(self.fp_ports)
        self.vni = 10000
        self.mac_type = SWITCH_MAC_ENTRY_STATIC
        self.tunnel_type = SWITCH_TUNNEL_TYPE_GENEVE
        self.tunnel_map_type = [SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE,
                                SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI]
        self.mapper_h = [0] * 2
        self.mapper_entry_h = [0] * 2
        self.tunnel_src_ip = '1.1.1.1'
        self.encap_src_ip = '1.1.1.3'
        self.tunnel_dst_ip = '1.1.1.3'
        self.nhop_mac = '00:33:33:33:33:33'
        self.entry_type = SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P
        self.host_mac = ['00:11:11:11:11:11', '00:22:22:22:22:22']

        self.vrf = self.add_vrf(device, self.vrf_id)
        self.underlay_vrf = self.add_vrf(device, self.underlay_vrf_id)
        self.rmac = self.add_rmac(device, '00:77:66:55:44:33', rmac_type='all')

        for index in range(0, len(self.fp_ports)):
          self.port_h[index] = self.select_port(device, swports[index])

        self.if_h = self.cfg_l2intf_on_port(device, self.port_h[0])

        self.rif_h = self.create_l3_rif(
                                   device,
                                   self.underlay_vrf,
                                   self.rmac,
                                   self.port_h[1],
                                   self.rif_ip)

        self.ln = self.create_logical_network(device)
        self.add_logical_network_member(device, self.ln, self.if_h)

        for index in range(0, len(self.tunnel_map_type)):
          self.mapper_h[index] = self.create_tunnel_mapper(
                              device=device,
                              map_type=self.tunnel_map_type[index],
                              tunnel_type=self.tunnel_type)
          self.mapper_entry_h[index] = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=self.tunnel_map_type[index],
                              mapper=self.mapper_h[index],
                              handle=self.ln,
                              vni=self.vni)

        self.underlay_lb_h = self.create_loopback_rif(device, self.underlay_vrf, self.rmac)
        self.overlay_lb_h = self.create_loopback_rif(device, self.vrf, self.rmac)

        self.tunnel_h = self.create_tunnel_table(
                              device=device,
                              tunnel_type=self.tunnel_type,
                              src_ip=self.encap_src_ip,
                              imapper_h=self.mapper_h[0],
                              emapper_h=self.mapper_h[1],
                              urif=self.underlay_lb_h,
                              orif=self.overlay_lb_h)

        self.tunnel_term_h = self.create_tunnel_term(
                              device=device,
                              tunnel_type=self.tunnel_type,
                              vrf=self.underlay_vrf,
                              tunnel=self.tunnel_h,
                              entry_type=self.entry_type,
                              src_ip=self.tunnel_src_ip,
                              dst_ip=self.tunnel_dst_ip)

        self.tunnel_if_h = self.create_tunnel_interface(
                              device=device,
                              tunnel=self.tunnel_h)

        self.nhop1 = self.add_l3_nhop(device, self.rif_h, self.tunnel_src_ip, self.nhop_mac)
        self.add_static_route(device, self.underlay_vrf, self.tunnel_src_ip, self.nhop1)

        self.add_mac_table_entry(device, self.ln, self.host_mac[0], self.mac_type, self.if_h)
        self.add_mac_table_entry(device, self.ln, self.host_mac[1],
                                 self.mac_type, self.tunnel_if_h,
                                 tunnel=True, tunnel_ip=self.tunnel_src_ip)

    def runTest(self):
        try:
            print "L2 Geneve Basic Mode Unicast Test"
            print "Sending packet from Access port1 to Geneve port2"
            pkt = simple_tcp_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            udp_sport = entropy_hash(pkt)
            geneve_pkt = simple_geneve_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:33:33:33:33:33',
                ip_id=0,
                ip_src='1.1.1.3',
                ip_dst='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                geneve_vni=self.vni,
                inner_frame=pkt)
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, geneve_pkt, [swports[1]])

            print "Sending packet from Geneve port2 to Access port1"
            pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            geneve_pkt = simple_geneve_packet(
                eth_src='00:33:33:33:33:33',
                eth_dst='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=11638,
                with_udp_chksum=False,
                geneve_vni=self.vni,
                inner_frame=pkt)

            send_packet(self, swports[1], str(geneve_pkt))
            verify_packets(self, pkt, [swports[0]])
        finally:
            print
            print "Packet from port %s to port %s on vrf %s" % (swports[0], swports[1], self.vrf_id)

    def tearDown(self):
      self.cleanup()
      api_base_tests.ThriftInterfaceDataPlane.tearDown(self)

###############################################################################
@group('bfd')
@group('l2')
@group('l3')
@group('tunnel')
@group('maxsizes')
@group('non-vxlan-tunnel')
class L2GeneveUnicastLagBasicTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        self.vrf_id = 10
        self.underlay_vrf_id = 20
        self.fp_ports = [swports[1], swports[2], swports[3], swports[4]]
        self.port_h = [0] * len(self.fp_ports)
        self.rif_h = [0] * len(self.fp_ports)
        self.rif_ip = '192.168.0.2'
        self.host_mac = '00:11:22:33:44:55'
        self.port_h = [0] * len(self.fp_ports)
        self.intf_h = [0] * len(self.fp_ports)
        self.vni = 10000
        self.mac_type = SWITCH_MAC_ENTRY_STATIC
        self.tunnel_type = SWITCH_TUNNEL_TYPE_GENEVE
        self.tunnel_map_type = [SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE,
                                SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI]
        self.mapper_h = [0] * 2
        self.mapper_entry_h = [0] * 2
        self.tunnel_src_ip = '1.1.1.1'
        self.encap_src_ip = '1.1.1.3'
        self.tunnel_dst_ip = '1.1.1.3'
        self.nhop_mac = '00:33:33:33:33:33'
        self.entry_type = SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P
        self.host_mac = ['00:11:11:11:11:11', '00:22:22:22:22:22']

        self.vrf = self.add_vrf(device, self.vrf_id)
        self.underlay_vrf = self.add_vrf(device, self.underlay_vrf_id)
        self.rmac = self.add_rmac(device, '00:77:66:55:44:33', rmac_type='all')

        for index in range(0, len(self.fp_ports)):
          self.port_h[index] = self.select_port(device, self.fp_ports[index])

        self.lag1_h = self.add_lag(device)
        self.add_lag_member(device, self.lag1_h, self.port_h[0])
        self.add_lag_member(device, self.lag1_h, self.port_h[1])

        self.lag2_h = self.add_lag(device)
        self.add_lag_member(device, self.lag2_h, self.port_h[2])
        self.add_lag_member(device, self.lag2_h, self.port_h[3])

        self.if_h = self.cfg_l2intf_on_port(device, self.lag1_h)

        self.rif_h = self.create_l3_rif(
                                   device,
                                   self.underlay_vrf,
                                   self.rmac,
                                   self.lag2_h,
                                   self.rif_ip)

        self.ln = self.create_logical_network(device)
        self.add_logical_network_member(device, self.ln, self.if_h)

        for index in range(0, len(self.tunnel_map_type)):
          self.mapper_h[index] = self.create_tunnel_mapper(
                              device=device,
                              map_type=self.tunnel_map_type[index],
                              tunnel_type=self.tunnel_type)
          self.mapper_entry_h[index] = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=self.tunnel_map_type[index],
                              mapper=self.mapper_h[index],
                              handle=self.ln,
                              vni=self.vni)

        self.underlay_lb_h = self.create_loopback_rif(device, self.underlay_vrf, self.rmac)
        self.overlay_lb_h = self.create_loopback_rif(device, self.vrf, self.rmac)

        self.tunnel_h = self.create_tunnel_table(
                              device=device,
                              tunnel_type=self.tunnel_type,
                              src_ip=self.encap_src_ip,
                              imapper_h=self.mapper_h[0],
                              emapper_h=self.mapper_h[1],
                              urif=self.underlay_lb_h,
                              orif=self.overlay_lb_h)

        self.tunnel_term_h = self.create_tunnel_term(
                              device=device,
                              tunnel_type=self.tunnel_type,
                              vrf=self.underlay_vrf,
                              tunnel=self.tunnel_h,
                              entry_type=self.entry_type,
                              src_ip=self.tunnel_src_ip,
                              dst_ip=self.tunnel_dst_ip)

        self.tunnel_if_h = self.create_tunnel_interface(
                              device=device,
                              tunnel=self.tunnel_h)

        self.nhop1 = self.add_l3_nhop(device, self.rif_h, self.tunnel_src_ip, self.nhop_mac)
        self.add_static_route(device, self.underlay_vrf, self.tunnel_src_ip, self.nhop1)

        self.add_mac_table_entry(device, self.ln, self.host_mac[0], self.mac_type, self.if_h)
        self.add_mac_table_entry(device, self.ln, self.host_mac[1],
                                 self.mac_type, self.tunnel_if_h,
                                 tunnel=True, tunnel_ip=self.tunnel_src_ip)

    def runTest(self):
        try:
            print
            print "L2 Geneve Lag Basic Mode Unicast Test"
            pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            geneve_pkt = simple_geneve_packet(
                eth_src='00:33:33:33:33:33',
                eth_dst='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=11638,
                with_udp_chksum=False,
                geneve_vni=self.vni,
                inner_frame=pkt)

            print "Sending packet from Lag Geneve port3 to Access Lag"
            send_packet(self, swports[3], str(geneve_pkt))
            verify_any_packet_any_port(self, [pkt], [swports[1], swports[2]])

            print "Sending packet from Lag Geneve port4 to Access Lag"
            send_packet(self, swports[4], str(geneve_pkt))
            verify_any_packet_any_port(self, [pkt], [swports[1], swports[2]])

            pkt = simple_tcp_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            udp_sport = entropy_hash(pkt)
            geneve_pkt = simple_geneve_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:33:33:33:33:33',
                ip_id=0,
                ip_dst='1.1.1.1',
                ip_src='1.1.1.3',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                geneve_vni=self.vni,
                inner_frame=pkt)

            print "Sending packet from Lag Access port1 to Geneve Lag"
            send_packet(self, swports[1], str(pkt))
            verify_any_packet_any_port(self, [geneve_pkt],
                                       [swports[3], swports[4]])

            print "Sending packet from Lag Access port2 to Geneve Lag"
            send_packet(self, swports[2], str(pkt))
            verify_any_packet_any_port(self, [geneve_pkt],
                                       [swports[3], swports[4]])
        finally:
            print
            print "Packet from port %s to port %s on vrf %s" % (swports[0], swports[1], self.vrf_id)

    def tearDown(self):
      self.cleanup()
      api_base_tests.ThriftInterfaceDataPlane.tearDown(self)

###############################################################################
@group('bfd')
@group('l2')
@group('l3')
@group('tunnel')
@group('maxsizes')
@group('mcast')
@group('non-vxlan-tunnel')
class L2NvgreUnicastLagBasicTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        self.vrf_id = 10
        self.underlay_vrf_id = 20
        self.fp_ports = [swports[1], swports[2], swports[3], swports[4]]
        self.port_h = [0] * len(self.fp_ports)
        self.rif_h = [0] * len(self.fp_ports)
        self.rif_ip = '192.168.0.2'
        self.host_mac = '00:11:22:33:44:55'
        self.port_h = [0] * len(self.fp_ports)
        self.intf_h = [0] * len(self.fp_ports)
        self.vni = 10000
        self.mac_type = SWITCH_MAC_ENTRY_STATIC
        self.tunnel_type = SWITCH_TUNNEL_TYPE_NVGRE
        self.tunnel_map_type = [SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE,
                                SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI]
        self.mapper_h = [0] * 2
        self.mapper_entry_h = [0] * 2
        self.tunnel_src_ip = '1.1.1.1'
        self.encap_src_ip = '1.1.1.3'
        self.tunnel_dst_ip = '1.1.1.3'
        self.nhop_mac = '00:33:33:33:33:33'
        self.entry_type = SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P
        self.host_mac = ['00:11:11:11:11:11', '00:22:22:22:22:22']

        self.vrf = self.add_vrf(device, self.vrf_id)
        self.underlay_vrf = self.add_vrf(device, self.underlay_vrf_id)
        self.rmac = self.add_rmac(device, '00:77:66:55:44:33', rmac_type='all')

        for index in range(0, len(self.fp_ports)):
          self.port_h[index] = self.select_port(device, self.fp_ports[index])

        self.lag1_h = self.add_lag(device)
        self.add_lag_member(device, self.lag1_h, self.port_h[0])
        self.add_lag_member(device, self.lag1_h, self.port_h[1])

        self.lag2_h = self.add_lag(device)
        self.add_lag_member(device, self.lag2_h, self.port_h[2])
        self.add_lag_member(device, self.lag2_h, self.port_h[3])

        self.if_h = self.cfg_l2intf_on_port(device, self.lag1_h)

        self.rif_h = self.create_l3_rif(
                                   device,
                                   self.underlay_vrf,
                                   self.rmac,
                                   self.lag2_h,
                                   self.rif_ip)

        self.ln = self.create_logical_network(device)
        self.add_logical_network_member(device, self.ln, self.if_h)

        for index in range(0, len(self.tunnel_map_type)):
          self.mapper_h[index] = self.create_tunnel_mapper(
                              device=device,
                              map_type=self.tunnel_map_type[index],
                              tunnel_type=self.tunnel_type)
          self.mapper_entry_h[index] = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=self.tunnel_map_type[index],
                              mapper=self.mapper_h[index],
                              handle=self.ln,
                              vni=self.vni)

        self.underlay_lb_h = self.create_loopback_rif(device, self.underlay_vrf, self.rmac)
        self.overlay_lb_h = self.create_loopback_rif(device, self.vrf, self.rmac)

        self.tunnel_h = self.create_tunnel_table(
                              device=device,
                              tunnel_type=self.tunnel_type,
                              src_ip=self.encap_src_ip,
                              imapper_h=self.mapper_h[0],
                              emapper_h=self.mapper_h[1],
                              urif=self.underlay_lb_h,
                              orif=self.overlay_lb_h)

        self.tunnel_term_h = self.create_tunnel_term(
                              device=device,
                              tunnel_type=self.tunnel_type,
                              vrf=self.underlay_vrf,
                              tunnel=self.tunnel_h,
                              entry_type=self.entry_type,
                              src_ip=self.tunnel_src_ip,
                              dst_ip=self.tunnel_dst_ip)

        self.tunnel_if_h = self.create_tunnel_interface(
                              device=device,
                              tunnel=self.tunnel_h)

        self.nhop1 = self.add_l3_nhop(device, self.rif_h, self.tunnel_src_ip, self.nhop_mac)
        self.add_static_route(device, self.underlay_vrf, self.tunnel_src_ip, self.nhop1)

        self.add_mac_table_entry(device, self.ln, self.host_mac[0], self.mac_type, self.if_h)
        self.add_mac_table_entry(device, self.ln, self.host_mac[1],
                                 self.mac_type, self.tunnel_if_h,
                                 tunnel=True, tunnel_ip=self.tunnel_src_ip)

    def runTest(self):
        print
        print "L2 Nvgre Lag Basic Mode Unicast Test"
        try:
            pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            nvgre_pkt = simple_nvgre_packet(
                eth_src='00:33:33:33:33:33',
                eth_dst='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                nvgre_tni=self.vni,
                inner_frame=pkt)

            print "Sending packet from Lag Nvgre port3 to Access Lag"
            send_packet(self, swports[3], str(nvgre_pkt))
            verify_any_packet_any_port(self, [pkt], [swports[1], swports[2]])

            print "Sending packet from Lag Nvgre port4 to Access Lag"
            send_packet(self, swports[4], str(nvgre_pkt))
            verify_any_packet_any_port(self, [pkt], [swports[1], swports[2]])

            pkt = simple_tcp_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            nvgre_flowid = entropy_hash(pkt) & 0xFF
            nvgre_pkt = simple_nvgre_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:33:33:33:33:33',
                ip_id=0,
                ip_dst='1.1.1.1',
                ip_src='1.1.1.3',
                ip_ttl=64,
                ip_flags=0x2,
                nvgre_tni=self.vni,
                nvgre_flowid=nvgre_flowid,
                inner_frame=pkt)

            print "Sending packet from Lag Access port1 to Nvgre Lag"
            send_packet(self, swports[1], str(pkt))
            verify_any_packet_any_port(self, [nvgre_pkt],
                                       [swports[3], swports[4]])

            print "Sending packet from Lag Access port2 to Nvgre Lag"
            send_packet(self, swports[2], str(pkt))
            verify_any_packet_any_port(self, [nvgre_pkt],
                                       [swports[3], swports[4]])
        finally:
            print
            print "Packet from port %s to port %s on vrf %s" % (swports[0], swports[1], self.vrf_id)

    def tearDown(self):
      self.cleanup()
      api_base_tests.ThriftInterfaceDataPlane.tearDown(self)

###############################################################################
@group('bfd')
@group('l2')
@group('tunnel')
@group('maxsizes')
@group('2porttests')
@group('ent')
class L2LNSubIntfEncapTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print

        vrf = self.client.switch_api_vrf_create(device, 2)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        self.client.switch_api_port_bind_mode_set(
            device, port1, SWITCH_PORT_BIND_MODE_PORT_VLAN)
        self.client.switch_api_port_bind_mode_set(
            device, port2, SWITCH_PORT_BIND_MODE_PORT_VLAN)

        i_info1 = switcht_interface_info_t(
            handle=port1, vlan=10, type=SWITCH_INTERFACE_TYPE_PORT_VLAN)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, vlan=20, type=SWITCH_INTERFACE_TYPE_PORT_VLAN)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        # Create a logical network (LN)
        lognet_info = switcht_logical_network_t()
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)

        self.client.switch_api_logical_network_member_add(device, ln1, if1)
        self.client.switch_api_logical_network_member_add(device, ln1, if2)

        switch_api_mac_table_entry_create(
            self, device, ln1, '00:11:11:11:11:11', 2, if2)
        switch_api_mac_table_entry_create(
            self, device, ln1, '00:22:22:22:22:22', 2, if1)

        try:
            print "Sending L2 packet - port %d" % swports[
                0], "(vlan 10) -> port %d" % swports[1], "(vlan 20)"
            pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:22:22:22:22:22',
                dl_vlan_enable=True,
                vlan_vid=10,
                ip_dst='10.0.0.1',
                ip_id=102,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.0.0.1',
                ip_id=102,
                ip_ttl=64,
                dl_vlan_enable=True,
                vlan_vid=20)
            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

            print "Sending L2 packet - port %d" % swports[
                1], "(vlan 20) -> port %d" % swports[0], "(vlan 10)"
            pkt = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                dl_vlan_enable=True,
                vlan_vid=20,
                ip_dst='10.0.0.1',
                ip_id=102,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                ip_dst='10.0.0.1',
                ip_id=102,
                ip_ttl=64,
                dl_vlan_enable=True,
                vlan_vid=10)
            send_packet(self, swports[1], str(pkt))
            verify_packets(self, exp_pkt, [swports[0]])

        finally:
            switch_api_mac_table_entry_delete(self, device, ln1,
                                                          '00:11:11:11:11:11')
            switch_api_mac_table_entry_delete(self, device, ln1,
                                                          '00:22:22:22:22:22')

            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if1)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if2)
            self.client.switch_api_logical_network_delete(device, ln1)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_port_bind_mode_set(
                device, port1, SWITCH_PORT_BIND_MODE_PORT)
            self.client.switch_api_port_bind_mode_set(
                device, port2, SWITCH_PORT_BIND_MODE_PORT)

            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('bfd')
@group('l2')
@group('l3')
@group('tunnel')
@group('2porttests')
@group('mcast')
@group('non-vxlan-tunnel')
class L2VxlanToGeneveUnicastBasicTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "L2 Geneve-Vxlan Basic Mode Unicast Test"
        self.tunnel1_src_ip = '1.1.1.3'
        self.tunnel1_dst_ip = '1.1.1.1'
        self.tunnel2_src_ip = '2.2.2.3'
        self.tunnel2_dst_ip = '2.2.2.1'

        vrf = self.add_vrf(device, 2)
        rmac = self.add_rmac(device, '00:77:66:55:44:33', rmac_type='all')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rif1 = self.create_l3_rif(device, vrf, rmac, port1, '12.12.12.1')
        rif2 = self.create_l3_rif(device, vrf, rmac, port2, '192.168.10.2')

        ln1 = self.create_logical_network(device)

        imapper_h1 = self.create_tunnel_mapper(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE,
                              tunnel_type=SWITCH_TUNNEL_TYPE_GENEVE)
        imapper_entry_h1 = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE,
                              mapper=imapper_h1,
                              handle=ln1,
                              vni=0x1234)
        emapper_h1 = self.create_tunnel_mapper(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI,
                              tunnel_type=SWITCH_TUNNEL_TYPE_GENEVE)
        emapper_entry_h1 = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI,
                              mapper=emapper_h1,
                              handle=ln1,
                              vni=0x1234)

        imapper_h2 = self.create_tunnel_mapper(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN)
        imapper_entry_h2 = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE,
                              mapper=imapper_h2,
                              handle=ln1,
                              vni=0x1234)
        emapper_h2 = self.create_tunnel_mapper(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI,
                              tunnel_type=SWITCH_TUNNEL_TYPE_GENEVE)
        emapper_entry_h2 = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI,
                              mapper=emapper_h2,
                              handle=ln1,
                              vni=0x1234)

        underlay_lb_h = self.create_loopback_rif(device, vrf, rmac)
        overlay_lb_h = self.create_loopback_rif(device, vrf, rmac)

        tunnel_h1 = self.create_tunnel_table(
                              device=device,
                              tunnel_type=SWITCH_TUNNEL_TYPE_GENEVE,
                              src_ip=self.tunnel1_src_ip,
                              imapper_h=imapper_h1,
                              emapper_h=emapper_h1,
                              urif=underlay_lb_h,
                              orif=overlay_lb_h)

        tunnel_term_h1 = self.create_tunnel_term(
                              device=device,
                              tunnel_type=SWITCH_TUNNEL_TYPE_GENEVE,
                              vrf=vrf,
                              tunnel=tunnel_h1,
                              entry_type=SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P,
                              src_ip=self.tunnel1_dst_ip,
                              dst_ip=self.tunnel1_src_ip)

        tunnel_if_h1 = self.create_tunnel_interface(
                              device=device,
                              tunnel=tunnel_h1)

        tunnel_h2 = self.create_tunnel_table(
                              device=device,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
                              src_ip=self.tunnel2_src_ip,
                              imapper_h=imapper_h2,
                              emapper_h=emapper_h2,
                              urif=underlay_lb_h,
                              orif=overlay_lb_h)

        tunnel_term_h2 = self.create_tunnel_term(
                              device=device,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
                              vrf=vrf,
                              tunnel=tunnel_h2,
                              entry_type=SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P,
                              src_ip=self.tunnel2_dst_ip,
                              dst_ip=self.tunnel2_src_ip)

        tunnel_if_h2 = self.create_tunnel_interface(
                              device=device,
                              tunnel=tunnel_h2)

        nhop1 = self.add_l3_nhop(device, rif1, self.tunnel1_dst_ip, '00:33:33:33:33:33')
        self.add_static_route(device, vrf, self.tunnel1_dst_ip, nhop1)

        nhop2 = self.add_l3_nhop(device, rif2, self.tunnel2_dst_ip, '00:44:44:44:44:44')
        self.add_static_route(device, vrf, self.tunnel2_dst_ip, nhop2)

        self.add_mac_table_entry(device, ln1, '00:11:11:11:11:11', 2, tunnel_if_h1, tunnel=True, tunnel_ip=self.tunnel1_dst_ip)
        self.add_mac_table_entry(device, ln1, '00:22:22:22:22:22', 2, tunnel_if_h2, tunnel=True, tunnel_ip=self.tunnel2_dst_ip)

    def runTest(self):
        print "L2 Tunnel Splicing - Geneve <-> Vxlan (Basic Mode)"
        print "Sending packet from Geneve port1 to Vxlan port2"
        try:
            pkt = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            geneve_pkt = simple_geneve_packet(
                eth_src='00:33:33:33:33:33',
                eth_dst='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=11638,
                with_udp_chksum=False,
                geneve_vni=0x1234,
                inner_frame=pkt)

            udp_sport = entropy_hash(pkt)
            vxlan_pkt = simple_vxlan_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:44:44:44:44:44',
                ip_id=0,
                ip_dst='2.2.2.1',
                ip_src='2.2.2.3',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)

            send_packet(self, swports[0], str(geneve_pkt))
            verify_packets(self, vxlan_pkt, [swports[1]])

            print "Sending packet from Vxlan port2 to Geneve port1"
            pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            vxlan_pkt = simple_vxlan_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:44:44:44:44:44',
                ip_id=0,
                ip_dst='2.2.2.3',
                ip_src='2.2.2.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=574,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)

            udp_sport = entropy_hash(pkt)
            geneve_pkt = simple_geneve_packet(
                eth_dst='00:33:33:33:33:33',
                eth_src='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='1.1.1.1',
                ip_src='1.1.1.3',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                geneve_vni=0x1234,
                inner_frame=pkt)
            send_packet(self, swports[1], str(vxlan_pkt))
            verify_packets(self, geneve_pkt, [swports[0]])
        finally:
            print
            print "Packet from port %s to port %s" % (swports[0], swports[1])

    def tearDown(self):
      self.cleanup()
      api_base_tests.ThriftInterfaceDataPlane.tearDown(self)

###############################################################################
@group('bfd')
@group('l2')
@group('l3')
@group('tunnel')
@group('maxsizes')
@group('mcast')
@group('non-vxlan-tunnel')
class L2VxlanToGeneveUnicastLagBasicTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        print "L2 Vxlan-Geneve Lag Basic Mode Unicast Test"
        self.tunnel1_src_ip = '1.1.1.3'
        self.tunnel1_dst_ip = '1.1.1.1'
        self.tunnel2_src_ip = '2.2.2.3'
        self.tunnel2_dst_ip = '2.2.2.1'

        vrf = self.add_vrf(device, 2)
        rmac = self.add_rmac(device, '00:77:66:55:44:33', rmac_type='all')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])
        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[4])

        lag1_h = self.add_lag(device)
        self.add_lag_member(device, lag1_h, port1)
        self.add_lag_member(device, lag1_h, port2)

        lag2_h = self.add_lag(device)
        self.add_lag_member(device, lag2_h, port3)
        self.add_lag_member(device, lag2_h, port4)

        rif1 = self.create_l3_rif(device, vrf, rmac, lag1_h, '12.12.12.1')
        rif2 = self.create_l3_rif(device, vrf, rmac, lag2_h, '192.168.10.2')

        ln1 = self.create_logical_network(device)

        imapper_h1 = self.create_tunnel_mapper(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE,
                              tunnel_type=SWITCH_TUNNEL_TYPE_GENEVE)
        imapper_entry_h1 = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE,
                              mapper=imapper_h1,
                              handle=ln1,
                              vni=0x1234)
        emapper_h1 = self.create_tunnel_mapper(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI,
                              tunnel_type=SWITCH_TUNNEL_TYPE_GENEVE)
        emapper_entry_h1 = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI,
                              mapper=emapper_h1,
                              handle=ln1,
                              vni=0x1234)

        imapper_h2 = self.create_tunnel_mapper(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN)
        imapper_entry_h2 = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE,
                              mapper=imapper_h2,
                              handle=ln1,
                              vni=0x1234)
        emapper_h2 = self.create_tunnel_mapper(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI,
                              tunnel_type=SWITCH_TUNNEL_TYPE_GENEVE)
        emapper_entry_h2 = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI,
                              mapper=emapper_h2,
                              handle=ln1,
                              vni=0x1234)

        underlay_lb_h = self.create_loopback_rif(device, vrf, rmac)
        overlay_lb_h = self.create_loopback_rif(device, vrf, rmac)

        tunnel_h1 = self.create_tunnel_table(
                              device=device,
                              tunnel_type=SWITCH_TUNNEL_TYPE_GENEVE,
                              src_ip=self.tunnel1_src_ip,
                              imapper_h=imapper_h1,
                              emapper_h=emapper_h1,
                              urif=underlay_lb_h,
                              orif=overlay_lb_h)

        tunnel_term_h1 = self.create_tunnel_term(
                              device=device,
                              tunnel_type=SWITCH_TUNNEL_TYPE_GENEVE,
                              vrf=vrf,
                              tunnel=tunnel_h1,
                              entry_type=SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P,
                              src_ip=self.tunnel1_dst_ip,
                              dst_ip=self.tunnel1_src_ip)

        tunnel_if_h1 = self.create_tunnel_interface(
                              device=device,
                              tunnel=tunnel_h1)

        tunnel_h2 = self.create_tunnel_table(
                              device=device,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
                              src_ip=self.tunnel2_src_ip,
                              imapper_h=imapper_h2,
                              emapper_h=emapper_h2,
                              urif=underlay_lb_h,
                              orif=overlay_lb_h)

        tunnel_term_h2 = self.create_tunnel_term(
                              device=device,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
                              vrf=vrf,
                              tunnel=tunnel_h2,
                              entry_type=SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P,
                              src_ip=self.tunnel2_dst_ip,
                              dst_ip=self.tunnel2_src_ip)

        tunnel_if_h2 = self.create_tunnel_interface(
                              device=device,
                              tunnel=tunnel_h2)

        nhop1 = self.add_l3_nhop(device, rif1, self.tunnel1_dst_ip, '00:33:33:33:33:33')
        self.add_static_route(device, vrf, self.tunnel1_dst_ip, nhop1)

        nhop2 = self.add_l3_nhop(device, rif2, self.tunnel2_dst_ip, '00:44:44:44:44:44')
        self.add_static_route(device, vrf, self.tunnel2_dst_ip, nhop2)

        self.add_mac_table_entry(device, ln1, '00:11:11:11:11:11', 2, tunnel_if_h1, tunnel=True, tunnel_ip=self.tunnel1_dst_ip)
        self.add_mac_table_entry(device, ln1, '00:22:22:22:22:22', 2, tunnel_if_h2, tunnel=True, tunnel_ip=self.tunnel2_dst_ip)

    def runTest(self):
        print "L2 Tunnel Splicing - Geneve <-> Vxlan (Basic Mode)"
        try:
            pkt = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            geneve_pkt = simple_geneve_packet(
                eth_src='00:33:33:33:33:33',
                eth_dst='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=11638,
                with_udp_chksum=False,
                geneve_vni=0x1234,
                inner_frame=pkt)
            udp_sport = entropy_hash(pkt)
            vxlan_pkt = simple_vxlan_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:44:44:44:44:44',
                ip_id=0,
                ip_dst='2.2.2.1',
                ip_src='2.2.2.3',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)

            print "Sending packet from Geneve member port1 to Vxlan lag"
            send_packet(self, swports[1], str(geneve_pkt))
            verify_any_packet_any_port(self, [vxlan_pkt],
                                       [swports[3], swports[4]], timeout=2)

            print "Sending packet from Geneve member port2 to Vxlan lag"
            send_packet(self, swports[2], str(geneve_pkt))
            verify_any_packet_any_port(self, [vxlan_pkt],
                                       [swports[3], swports[4]], timeout=2)

            pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            vxlan_pkt = simple_vxlan_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:44:44:44:44:44',
                ip_id=0,
                ip_dst='2.2.2.3',
                ip_src='2.2.2.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=574,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)
            udp_sport = entropy_hash(pkt)
            geneve_pkt = simple_geneve_packet(
                eth_dst='00:33:33:33:33:33',
                eth_src='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='1.1.1.1',
                ip_src='1.1.1.3',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                geneve_vni=0x1234,
                inner_frame=pkt)

            print "Sending packet from Vxlan member port1 to Geneve lag"
            send_packet(self, swports[3], str(vxlan_pkt))
            verify_any_packet_any_port(self, [geneve_pkt],
                                       [swports[1], swports[2]], timeout=2)

            print "Sending packet from Vxlan member port2 to Geneve lag"
            send_packet(self, swports[4], str(vxlan_pkt))
            verify_any_packet_any_port(self, [geneve_pkt],
                                       [swports[1], swports[2]], timeout=2)

        finally:
            print
            print "Packet from port %s to port %s" % (swports[0], swports[1])

    def tearDown(self):
      self.cleanup()
      api_base_tests.ThriftInterfaceDataPlane.tearDown(self)

###############################################################################
@group('bfd')
@group('l2')
@group('l3')
@group('tunnel')
@group('maxsizes')
@group('2porttests')
@group('mcast')
@group('non-vxlan-tunnel')
class L2VxlanToGeneveUnicastEnhancedTest(
        api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "L2 Geneve-Vxlan Enhanced Mode Unicast Test"
        return

        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_OUTER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, vrf_handle=vrf, rmac_handle=rmac)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, vrf_handle=vrf, rmac_handle=rmac)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        # Create a logical network (LN)
        lognet_info = switcht_logical_network_t()
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)

        tunnel_mapper1 = switcht_tunnel_mapper_t(
            tunnel_vni=0x4321, ln_handle=ln1)
        mapper_handle1 = self.client.switch_api_tunnel_mapper_create(
            device,
            direction=SWITCH_API_DIRECTION_BOTH,
            tunnel_type=SWITCH_TUNNEL_TYPE_GENEVE,
            tunnel_mapper_list=[tunnel_mapper1])

        # Create a tunnel interface
        src_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.1', prefix_length=32)
        dst_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.3', prefix_length=32)
        tunnel_info = switcht_tunnel_info_t(
            vrf_handle=vrf,
            tunnel_type=SWITCH_TUNNEL_TYPE_GENEVE,
            src_ip=src_ip3,
            dst_ip=dst_ip3,
            decap_mapper_handle=mapper_handle1,
            encap_mapper_handle=mapper_handle1,
            egress_rif_handle=rif1)
        ift3 = self.client.switch_api_tunnel_interface_create(
            device, SWITCH_API_DIRECTION_BOTH, tunnel_info)

        tunnel_mapper2 = switcht_tunnel_mapper_t(
            tunnel_vni=0x1234, ln_handle=ln1)
        mapper_handle2 = self.client.switch_api_tunnel_mapper_create(
            device,
            direction=SWITCH_API_DIRECTION_BOTH,
            tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
            tunnel_mapper_list=[tunnel_mapper2])

        # Create a tunnel interface
        src_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='2.2.2.1', prefix_length=32)
        dst_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='2.2.2.3', prefix_length=32)
        tunnel_info = switcht_tunnel_info_t(
            vrf_handle=vrf,
            tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
            src_ip=src_ip4,
            dst_ip=dst_ip4,
            decap_mapper_handle=mapper_handle2,
            encap_mapper_handle=mapper_handle2,
            egress_rif_handle=rif2)
        ift4 = self.client.switch_api_tunnel_interface_create(
            device, SWITCH_API_DIRECTION_BOTH, tunnel_info)

        # add L2 port to LN
        self.client.switch_api_logical_network_member_add(device, ln1, ift3)
        self.client.switch_api_logical_network_member_add(device, ln1, ift4)

        nhop_key3 = switcht_nhop_key_t(intf_handle=ift3, ln_handle=ln1, ip_addr_valid=0)
        nhop3 = self.client.switch_api_nhop_create(device, nhop_key3)
        neighbor_entry3 = switcht_neighbor_info_t(
            nhop_handle=nhop3,
            interface_handle=ift3,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L2,
            neigh_type=SWITCH_API_NEIGHBOR_IPV4_TUNNEL)
        neighbor3 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry3)

        nhop_key1 = switcht_nhop_key_t(intf_handle=rif1, ip_addr_valid=0)
        nhop1 = self.client.switch_api_nhop_create(device, nhop_key1)
        neighbor_entry1 = switcht_neighbor_info_t(
            nhop_handle=nhop1,
            interface_handle=rif1,
            mac_addr='00:33:33:33:33:33',
            ip_addr=src_ip3)
        neighbor1 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry1)

        nhop_key4 = switcht_nhop_key_t(intf_handle=ift4, ln_handle=ln1, ip_addr_valid=0)
        nhop4 = self.client.switch_api_nhop_create(device, nhop_key4)
        neighbor_entry4 = switcht_neighbor_info_t(
            nhop_handle=nhop4,
            interface_handle=ift4,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L2,
            neigh_type=SWITCH_API_NEIGHBOR_IPV4_TUNNEL)
        neighbor4 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry4)

        nhop_key2 = switcht_nhop_key_t(intf_handle=rif2, ip_addr_valid=0)
        nhop2 = self.client.switch_api_nhop_create(device, nhop_key2)
        neighbor_entry2 = switcht_neighbor_info_t(
            nhop_handle=nhop2,
            interface_handle=rif2,
            mac_addr='00:44:44:44:44:44',
            ip_addr=src_ip4)
        neighbor2 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry2)

        self.client.switch_api_l3_route_add(device, vrf, dst_ip3, nhop1)
        self.client.switch_api_l3_route_add(device, vrf, dst_ip4, nhop2)

        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:11:11:11:11:11', 2, nhop3)
        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:22:22:22:22:22', 2, nhop4)

        print "L2 Tunnel Splicing - Geneve <-> Vxlan (Enhanced Mode)"
        print "Sending packet from Geneve port1 to Vxlan port2"

        try:
            pkt = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            geneve_pkt = simple_geneve_packet(
                eth_src='00:33:33:33:33:33',
                eth_dst='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=11638,
                with_udp_chksum=False,
                geneve_vni=0x4321,
                inner_frame=pkt)

            udp_sport = entropy_hash(pkt)
            vxlan_pkt = simple_vxlan_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:44:44:44:44:44',
                ip_id=0,
                ip_dst='2.2.2.3',
                ip_src='2.2.2.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)
            send_packet(self, swports[0], str(geneve_pkt))
            verify_packets(self, vxlan_pkt, [swports[1]])

            print "Sending packet from Vxlan port2 to Geneve port1"
            pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            vxlan_pkt = simple_vxlan_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:44:44:44:44:44',
                ip_id=0,
                ip_dst='2.2.2.3',
                ip_src='2.2.2.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=574,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)

            udp_sport = entropy_hash(pkt)
            geneve_pkt = simple_geneve_packet(
                eth_dst='00:33:33:33:33:33',
                eth_src='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                geneve_vni=0x4321,
                inner_frame=pkt)
            send_packet(self, swports[1], str(vxlan_pkt))
            verify_packets(self, geneve_pkt, [swports[0]])
        finally:
            self.client.switch_api_l3_route_delete(device, vrf, dst_ip3, nhop1)
            self.client.switch_api_l3_route_delete(device, vrf, dst_ip4, nhop2)

            self.client.switch_api_neighbor_delete(device, neighbor3)
            self.client.switch_api_nhop_delete(device, nhop3)

            self.client.switch_api_neighbor_delete(device, neighbor4)
            self.client.switch_api_nhop_delete(device, nhop4)

            self.client.switch_api_neighbor_delete(device, neighbor1)
            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_neighbor_delete(device, neighbor2)
            self.client.switch_api_nhop_delete(device, nhop2)

            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:11:11:11:11:11')
            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:22:22:22:22:22')

            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift3)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift4)
            self.client.switch_api_logical_network_delete(device, ln1)

            self.client.switch_api_tunnel_interface_delete(device, ift3)
            self.client.switch_api_tunnel_interface_delete(device, ift4)
            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('bfd')
@group('l2')
@group('l3')
@group('tunnel')
@group('maxsizes')
@group('mcast')
@group('non-vxlan-tunnel')
class L2VxlanToGeneveUnicastLagEnhancedTest(
        api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "L2 Geneve-Vxlan Lag Enhanced Mode Unicast Test"
        return

        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_OUTER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])
        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[4])

        lag1 = self.client.switch_api_lag_create(device)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag1, side=SWITCH_API_DIRECTION_BOTH, port=port1)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag1, side=SWITCH_API_DIRECTION_BOTH, port=port2)
        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, vrf_handle=vrf, rmac_handle=rmac)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=lag1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        lag2 = self.client.switch_api_lag_create(device)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag2, side=SWITCH_API_DIRECTION_BOTH, port=port3)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag2, side=SWITCH_API_DIRECTION_BOTH, port=port4)
        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, vrf_handle=vrf, rmac_handle=rmac)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=lag2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        # Create a logical network (LN)
        lognet_info = switcht_logical_network_t()
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)

        tunnel_mapper1 = switcht_tunnel_mapper_t(
            tunnel_vni=0x4321, ln_handle=ln1)
        mapper_handle1 = self.client.switch_api_tunnel_mapper_create(
            device,
            direction=SWITCH_API_DIRECTION_BOTH,
            tunnel_type=SWITCH_TUNNEL_TYPE_GENEVE,
            tunnel_mapper_list=[tunnel_mapper1])

        # Create a tunnel interface
        src_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.1', prefix_length=32)
        dst_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.3', prefix_length=32)
        tunnel_info = switcht_tunnel_info_t(
            vrf_handle=vrf,
            tunnel_type=SWITCH_TUNNEL_TYPE_GENEVE,
            src_ip=src_ip3,
            dst_ip=dst_ip3,
            decap_mapper_handle=mapper_handle1,
            encap_mapper_handle=mapper_handle1,
            egress_rif_handle=rif1)
        ift3 = self.client.switch_api_tunnel_interface_create(
            device, SWITCH_API_DIRECTION_BOTH, tunnel_info)

        tunnel_mapper2 = switcht_tunnel_mapper_t(
            tunnel_vni=0x1234, ln_handle=ln1)
        mapper_handle2 = self.client.switch_api_tunnel_mapper_create(
            device,
            direction=SWITCH_API_DIRECTION_BOTH,
            tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
            tunnel_mapper_list=[tunnel_mapper2])

        # Create a tunnel interface
        src_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='2.2.2.1', prefix_length=32)
        dst_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='2.2.2.3', prefix_length=32)
        tunnel_info = switcht_tunnel_info_t(
            vrf_handle=vrf,
            tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
            src_ip=src_ip4,
            dst_ip=dst_ip4,
            decap_mapper_handle=mapper_handle2,
            encap_mapper_handle=mapper_handle2,
            egress_rif_handle=rif2)
        ift4 = self.client.switch_api_tunnel_interface_create(
            device, SWITCH_API_DIRECTION_BOTH, tunnel_info)

        # add L2 port to LN
        self.client.switch_api_logical_network_member_add(device, ln1, ift3)
        self.client.switch_api_logical_network_member_add(device, ln1, ift4)

        nhop_key3 = switcht_nhop_key_t(intf_handle=ift3, ln_handle=ln1, ip_addr_valid=0)
        nhop3 = self.client.switch_api_nhop_create(device, nhop_key3)
        neighbor_entry3 = switcht_neighbor_info_t(
            nhop_handle=nhop3,
            interface_handle=ift3,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L2,
            neigh_type=SWITCH_API_NEIGHBOR_IPV4_TUNNEL)
        neighbor3 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry3)

        nhop_key1 = switcht_nhop_key_t(intf_handle=rif1, ip_addr_valid=0)
        nhop1 = self.client.switch_api_nhop_create(device, nhop_key1)
        neighbor_entry1 = switcht_neighbor_info_t(
            nhop_handle=nhop1,
            interface_handle=rif1,
            mac_addr='00:33:33:33:33:33',
            ip_addr=src_ip3)
        neighbor1 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry1)

        nhop_key4 = switcht_nhop_key_t(intf_handle=ift4, ln_handle=ln1, ip_addr_valid=0)
        nhop4 = self.client.switch_api_nhop_create(device, nhop_key4)
        neighbor_entry4 = switcht_neighbor_info_t(
            nhop_handle=nhop4,
            interface_handle=ift4,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L2,
            neigh_type=SWITCH_API_NEIGHBOR_IPV4_TUNNEL)
        neighbor4 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry4)

        nhop_key2 = switcht_nhop_key_t(intf_handle=rif2, ip_addr_valid=0)
        nhop2 = self.client.switch_api_nhop_create(device, nhop_key2)
        neighbor_entry2 = switcht_neighbor_info_t(
            nhop_handle=nhop2,
            interface_handle=rif2,
            mac_addr='00:44:44:44:44:44',
            ip_addr=src_ip4)
        neighbor2 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry2)

        self.client.switch_api_l3_route_add(device, vrf, dst_ip3, nhop1)
        self.client.switch_api_l3_route_add(device, vrf, dst_ip4, nhop2)

        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:11:11:11:11:11', 2, nhop3)
        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:22:22:22:22:22', 2, nhop4)

        print "L2 Tunnel Splicing - Geneve <-> Vxlan (Enahanced Mode)"

        try:
            pkt = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            geneve_pkt = simple_geneve_packet(
                eth_src='00:33:33:33:33:33',
                eth_dst='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=11638,
                with_udp_chksum=False,
                geneve_vni=0x4321,
                inner_frame=pkt)

            udp_sport = entropy_hash(pkt)
            vxlan_pkt = simple_vxlan_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:44:44:44:44:44',
                ip_id=0,
                ip_dst='2.2.2.3',
                ip_src='2.2.2.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)

            print "Sending packet from Geneve member port1 to Vxlan lag"
            send_packet(self, swports[1], str(geneve_pkt))
            verify_any_packet_any_port(self, [vxlan_pkt],
                                       [swports[3], swports[4]], timeout=2)

            print "Sending packet from Geneve member port2 to Vxlan lag"
            send_packet(self, swports[2], str(geneve_pkt))
            verify_any_packet_any_port(self, [vxlan_pkt],
                                       [swports[3], swports[4]], timeout=2)

            print "Sending packet from Vxlan port2 to Geneve port1"
            pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            vxlan_pkt = simple_vxlan_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:44:44:44:44:44',
                ip_id=0,
                ip_dst='2.2.2.3',
                ip_src='2.2.2.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=574,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)

            udp_sport = entropy_hash(pkt)
            geneve_pkt = simple_geneve_packet(
                eth_dst='00:33:33:33:33:33',
                eth_src='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                geneve_vni=0x4321,
                inner_frame=pkt)

            print "Sending packet from Vxlan member port1 to Geneve lag"
            send_packet(self, swports[3], str(vxlan_pkt))
            verify_any_packet_any_port(self, [geneve_pkt],
                                       [swports[1], swports[2]], timeout=2)

            print "Sending packet from Vxlan member port2 to Geneve lag"
            send_packet(self, swports[4], str(vxlan_pkt))
            verify_any_packet_any_port(self, [geneve_pkt],
                                       [swports[1], swports[2]], timeout=2)
        finally:
            self.client.switch_api_l3_route_delete(device, vrf, dst_ip3, nhop1)
            self.client.switch_api_l3_route_delete(device, vrf, dst_ip4, nhop2)

            self.client.switch_api_neighbor_delete(device, neighbor3)
            self.client.switch_api_nhop_delete(device, nhop3)

            self.client.switch_api_neighbor_delete(device, neighbor4)
            self.client.switch_api_nhop_delete(device, nhop4)

            self.client.switch_api_neighbor_delete(device, neighbor1)
            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_neighbor_delete(device, neighbor2)
            self.client.switch_api_nhop_delete(device, nhop2)

            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:11:11:11:11:11')
            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:22:22:22:22:22')

            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift3)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift4)
            self.client.switch_api_logical_network_delete(device, ln1)

            self.client.switch_api_tunnel_interface_delete(device, ift3)
            self.client.switch_api_tunnel_interface_delete(device, ift4)
            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag1,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port1)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag1,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port2)

            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag2,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port3)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag2,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port4)

            self.client.switch_api_lag_delete(device, lag1)
            self.client.switch_api_lag_delete(device, lag2)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('bfd')
@group('l2')
@group('l3')
@group('tunnel')
@group('flood')
@group('maxsizes')
@group('mcast')
@group('ent')
class L2VxlanFloodBasicTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "L2 Vxlan Basic Mode Flood Test"
        return

        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif3 = self.client.switch_api_rif_create(0, rif_info3)
        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)

        # Create a logical network (LN)
        lognet_info = switcht_logical_network_t()
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)
        self.client.switch_api_logical_network_learning_set(device, ln1, False)

        tunnel_mapper2 = switcht_tunnel_mapper_t(
            tunnel_vni=0x1234, ln_handle=ln1)
        mapper_handle2 = self.client.switch_api_tunnel_mapper_create(
            device,
            direction=SWITCH_API_DIRECTION_BOTH,
            tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
            tunnel_mapper_list=[tunnel_mapper2])

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
            decap_mapper_handle=mapper_handle2,
            encap_mapper_handle=mapper_handle2,
            egress_rif_handle=rif3)
        ift4 = self.client.switch_api_tunnel_interface_create(
            device, SWITCH_API_DIRECTION_BOTH, tunnel_info)

        # add L2 port to LN
        self.client.switch_api_logical_network_member_add(device, ln1, if1)
        self.client.switch_api_logical_network_member_add(device, ln1, if2)

        nhop_key1 = switcht_nhop_key_t(intf_handle=ift4, ln_handle=ln1, ip_addr_valid=0)
        nhop1 = self.client.switch_api_nhop_create(device, nhop_key1)
        neighbor_entry1 = switcht_neighbor_info_t(
            nhop_handle=nhop1,
            interface_handle=ift4,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L2,
            neigh_type=SWITCH_API_NEIGHBOR_IPV4_TUNNEL)
        neighbor1 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry1)

        nhop_key2 = switcht_nhop_key_t(intf_handle=rif3, ip_addr_valid=0)
        nhop2 = self.client.switch_api_nhop_create(device, nhop_key2)
        neighbor_entry2 = switcht_neighbor_info_t(
            nhop_handle=nhop2,
            interface_handle=rif3,
            mac_addr='00:33:33:33:33:33',
            ip_addr=src_ip)
        neighbor2 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry2)

        self.client.switch_api_l3_route_add(device, vrf, dst_ip, nhop2)
        self.client.switch_api_logical_network_member_add(device, ln1, ift4)

        try:
            pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            vxlan_pkt = simple_vxlan_packet(
                eth_src='00:33:33:33:33:33',
                eth_dst='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=11638,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)

            print "Sending packet from Vxlan port3 to Access port1 and Access port2"
            send_packet(self, swports[3], str(vxlan_pkt))
            verify_packets(self, pkt, [swports[1], swports[2]])

            pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            udp_sport = entropy_hash(pkt)
            vxlan_pkt = simple_vxlan_packet(
                eth_dst='00:33:33:33:33:33',
                eth_src='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)

            print "Sending packet from Access port1 to Access port2 and Vxlan port3"
            send_packet(self, swports[1], str(pkt))
            verify_each_packet_on_each_port(self, [pkt, vxlan_pkt],
                                            [swports[2], swports[3]])

            print "Sending packet from Access port2 to Access port1 and Vxlan port3"
            send_packet(self, swports[2], str(pkt))
            verify_each_packet_on_each_port(self, [pkt, vxlan_pkt],
                                            [swports[1], swports[3]])
        finally:
            self.client.switch_api_l3_route_delete(device, vrf, dst_ip, nhop2)
            self.client.switch_api_neighbor_delete(device, neighbor1)
            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_neighbor_delete(device, neighbor2)
            self.client.switch_api_nhop_delete(device, nhop2)

            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if1)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if2)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift4)
            self.client.switch_api_logical_network_delete(device, ln1)

            self.client.switch_api_tunnel_interface_delete(device, ift4)
            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_rif_delete(0, rif3)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)

###############################################################################
@group('bfd')
@group('l2')
@group('learn')
@group('maxsizes')
@group('ent')
class L2DynamicMacLearnTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        vlan = self.client.switch_api_vlan_create(device, 10)
        self.client.switch_api_vlan_aging_interval_set(device, vlan, 60000)

        default_learn_timeout = 500
        learn_timeout_in_ms = 5000

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])
        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[4])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if3 = self.client.switch_api_interface_create(device, i_info3)

        i_info4 = switcht_interface_info_t(
            handle=port4, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if4 = self.client.switch_api_interface_create(device, i_info4)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)
        self.client.switch_api_vlan_member_add(device, vlan, if3)
        self.client.switch_api_vlan_member_add(device, vlan, if4)

        for port in range(1, 5):
            for mac_offset in range(1, 9):
                dst_mac = '00:33:33:33:' + str(port) + ':' + str(mac_offset)
                src_mac = '00:22:22:22:' + str(port) + ':' + str(mac_offset)
                pkt = simple_tcp_packet(
                    eth_dst=dst_mac,
                    eth_src=src_mac,
                    ip_dst='10.10.10.1',
                    ip_src='20.20.20.1',
                    ip_id=108,
                    ip_ttl=0)
                send_packet(self, swports[port], str(pkt))

        time.sleep(int(learn_timeout_in_ms / 1000) + 2)

        try:
            for dst_port in range(1, 5):
                for src_port in range(1, 5):
                    for mac_offset in range(1, 9):
                        if src_port == dst_port:
                            continue
                        dst_mac = '00:22:22:22:' + \
                            str(dst_port) + ':' + str(mac_offset)
                        src_mac = '00:22:22:22:' + \
                            str(src_port) + ':' + str(mac_offset)
                        pkt = simple_tcp_packet(
                            eth_dst=dst_mac,
                            eth_src=src_mac,
                            ip_dst='10.10.10.1',
                            ip_src='20.20.20.1',
                            ip_id=108,
                            ip_ttl=64)
                        send_packet(self, swports[src_port], str(pkt))
                        verify_packet(self, pkt, swports[dst_port])

            verify_no_other_packets(self)

        finally:
            self.client.switch_api_mac_table_entry_flush(
                device, SWITCH_MAC_FLUSH_TYPE_ALL, 0x0, 0x0)

            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)
            self.client.switch_api_vlan_member_remove(device, vlan, if3)
            self.client.switch_api_vlan_member_remove(device, vlan, if4)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)
            self.client.switch_api_interface_delete(device, if4)

            self.client.switch_api_vlan_delete(device, vlan)


###############################################################################
@group('l2')
@group('tunnel')
@group('mpls')
@group('maxsizes')
@group('2porttests')
class L2MplsPopTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print

        vrf = self.client.switch_api_vrf_create(device, 2)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_OUTER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, vrf_handle=vrf, rmac_handle=rmac)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        lognet_info = switcht_logical_network_t()
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)
        self.client.switch_api_logical_network_member_add(device, ln1, if2)

        mpls_info = switcht_api_mpls_info_t(
            tunnel_type=SWITCH_MPLS_TUNNEL_TYPE_MPLS,
            network_handle=ln1,
            mpls_mode=SWITCH_MPLS_MODE_TERMINATE,
            mpls_type=SWITCH_MPLS_TYPE_EOMPLS,
            intf_handle=if1,
            pop_label=0xabcde,
            pop_count=2)
        mpls = self.client.switch_api_mpls_tunnel_create(device, mpls_info)

        switch_api_mac_table_entry_create(
            self, device, ln1, '00:22:22:22:22:22', 2, if2)

        try:
            tag1 = {'label': 0xabcde, 'tc': 0x5, 'ttl': 0xAA, 's': 0x0}
            tag2 = {'label': 0x54321, 'tc': 0x2, 'ttl': 0xBB, 's': 0x1}
            mpls_tags = [tag1, tag2]
            pkt = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63)
            mpls_pkt = simple_mpls_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:11:11:11:11:11',
                mpls_tags=mpls_tags,
                inner_frame=pkt)
            send_packet(self, swports[0], str(mpls_pkt))
            verify_packets(self, pkt, [swports[1]])
        finally:
            switch_api_mac_table_entry_delete(self, device, ln1,
                                                          '00:22:22:22:22:22')

            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if2)
            self.client.switch_api_logical_network_delete(device, ln1)

            self.client.switch_api_mpls_tunnel_delete(device, mpls)
            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('l2')
@group('mpls')
@group('tunnel')
@group('maxsizes')
@group('2porttests')
@group('jumbomtu')
class L2MplsPushJumboTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print

        vrf = self.client.switch_api_vrf_create(device, 2)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        self.client.switch_api_port_mtu_set(device, port1, 9216, 9216)
        self.client.switch_api_port_mtu_set(device, port2, 9216, 9216)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_OUTER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, rmac_handle=rmac, vrf_handle=vrf)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        lognet_info = switcht_logical_network_t()
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)
        self.client.switch_api_logical_network_member_add(device, ln1, if2)

        mpls_tag1 = switcht_mpls_t(label=0xabcde, exp=0x5, ttl=0x30)
        mpls_tag2 = switcht_mpls_t(label=0x54321, exp=0x2, ttl=0x40)
        label_list = [mpls_tag1, mpls_tag2]
        label_stack_info = switcht_mpls_label_stack_t(label_list=label_list, bos=True)
        label_stack = self.client.switch_api_mpls_label_stack_create(device, label_stack_info)

        mpls_info = switcht_api_mpls_info_t(
            tunnel_type=SWITCH_MPLS_TUNNEL_TYPE_MPLS,
            network_handle=ln1,
            mpls_mode=SWITCH_MPLS_MODE_INITIATE,
            mpls_type=SWITCH_MPLS_TYPE_EOMPLS,
            intf_handle=if1,
            mac_addr='00:44:44:44:44:44')
        mpls = self.client.switch_api_mpls_tunnel_create(device, mpls_info)

        nhop_info = switcht_api_nhop_info_t(
                      nhop_type=SWITCH_NHOP_TYPE_MPLS,
                      nhop_tunnel_type=SWITCH_NHOP_TUNNEL_TYPE_NONE,
                      mpls_handle=mpls,
                      label_stack_handle=label_stack)
        nhop1 = self.client.switch_api_nhop_create(device, nhop_info)

        # neighbor type 5 is push l2vpn
        neighbor_entry1 = switcht_api_neighbor_info_t(
            nhop_handle=nhop1,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L2,
            neighbor_tunnel_type=SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_PUSH_L2VPN,
            neighbor_type=SWITCH_NEIGHBOR_TYPE_NHOP)
        neighbor1 = self.client.switch_api_neighbor_create(device, neighbor_entry1)

        switch_api_mac_table_entry_create(
            self, device, ln1, '00:11:11:11:11:11', 2, if2)
        switch_api_mac_table_entry_create(
            self, device, ln1, '00:22:22:22:22:22', 2, nhop1)

        try:
            tag1 = {'label': 0xabcde, 'tc': 0x5, 'ttl': 0x30, 's': 0x0}
            tag2 = {'label': 0x54321, 'tc': 0x2, 'ttl': 0x40, 's': 0x1}
            mpls_tags = [tag1, tag2]
            pkt = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63,
                pktlen=9100)
            mpls_pkt = simple_mpls_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:44:44:44:44:44',
                mpls_tags=mpls_tags,
                inner_frame=pkt,
                pktlen=9122)
            send_packet(self, swports[1], str(pkt))
            verify_packets(self, mpls_pkt, [swports[0]])
        finally:
            switch_api_mac_table_entry_delete(self, device, ln1,
                                                          '00:11:11:11:11:11')
            switch_api_mac_table_entry_delete(self, device, ln1,
                                                          '00:22:22:22:22:22')

            self.client.switch_api_neighbor_delete(device, neighbor1)
            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if2)
            self.client.switch_api_logical_network_delete(device, ln1)

            self.client.switch_api_mpls_tunnel_delete(device, mpls)
            self.client.switch_api_mpls_label_stack_delete(device, label_stack)
            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('l2')
@group('mpls')
@group('tunnel')
@group('maxsizes')
@group('2porttests')
class L2MplsPushTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print

        vrf = self.client.switch_api_vrf_create(device, 2)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_OUTER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, rmac_handle=rmac, vrf_handle=vrf)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        lognet_info = switcht_logical_network_t()
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)
        self.client.switch_api_logical_network_member_add(device, ln1, if2)

        mpls_tag1 = switcht_mpls_t(label=0xabcde, exp=0x5, ttl=0x30)
        mpls_tag2 = switcht_mpls_t(label=0x54321, exp=0x2, ttl=0x40)
        label_list = [mpls_tag1, mpls_tag2]
        label_stack_info = switcht_mpls_label_stack_t(label_list=label_list, bos=True)
        label_stack = self.client.switch_api_mpls_label_stack_create(device, label_stack_info)

        mpls_info = switcht_api_mpls_info_t(
            tunnel_type=SWITCH_MPLS_TUNNEL_TYPE_MPLS,
            network_handle=ln1,
            mpls_mode=SWITCH_MPLS_MODE_INITIATE,
            mpls_type=SWITCH_MPLS_TYPE_EOMPLS,
            intf_handle=if1,
            mac_addr='00:44:44:44:44:44')
        mpls = self.client.switch_api_mpls_tunnel_create(device, mpls_info)

        nhop_info = switcht_api_nhop_info_t(
                      nhop_type=SWITCH_NHOP_TYPE_MPLS,
                      nhop_tunnel_type=SWITCH_NHOP_TUNNEL_TYPE_NONE,
                      mpls_handle=mpls,
                      label_stack_handle=label_stack)
        nhop1 = self.client.switch_api_nhop_create(device, nhop_info)

        # neighbor type 5 is push l2vpn
        neighbor_entry1 = switcht_api_neighbor_info_t(
            nhop_handle=nhop1,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L2,
            neighbor_tunnel_type=SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_PUSH_L2VPN,
            neighbor_type=SWITCH_NEIGHBOR_TYPE_NHOP)
        neighbor1 = self.client.switch_api_neighbor_create(device, neighbor_entry1)

        switch_api_mac_table_entry_create(
            self, device, ln1, '00:11:11:11:11:11', 2, if2)
        switch_api_mac_table_entry_create(
            self, device, ln1, '00:22:22:22:22:22', 2, nhop1)

        try:
            tag1 = {'label': 0xabcde, 'tc': 0x5, 'ttl': 0x30, 's': 0x0}
            tag2 = {'label': 0x54321, 'tc': 0x2, 'ttl': 0x40, 's': 0x1}
            mpls_tags = [tag1, tag2]
            pkt = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63)
            mpls_pkt = simple_mpls_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:44:44:44:44:44',
                mpls_tags=mpls_tags,
                inner_frame=pkt)
            send_packet(self, swports[1], str(pkt))
            verify_packets(self, mpls_pkt, [swports[0]])
        finally:
            switch_api_mac_table_entry_delete(self, device, ln1,
                                                          '00:11:11:11:11:11')
            switch_api_mac_table_entry_delete(self, device, ln1,
                                                          '00:22:22:22:22:22')

            self.client.switch_api_neighbor_delete(device, neighbor1)
            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_logical_network_member_remove(device, ln1, if2)
            self.client.switch_api_logical_network_delete(device, ln1)

            self.client.switch_api_mpls_tunnel_delete(device, mpls)
            self.client.switch_api_mpls_label_stack_delete(device, label_stack)
            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('l2')
@group('mpls')
@group('tunnel')
@group('maxsizes')
@group('2porttests')
class L2MplsSwapTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        return

        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_OUTER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, rmac_handle=rmac, vrf_handle=vrf)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, rmac_handle=rmac, vrf_handle=vrf)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        nhop_key1 = switcht_nhop_key_t(intf_handle=rif2, ip_addr_valid=0)
        nhop1 = self.client.switch_api_nhop_create(device, nhop_key1)

        pop_tag = switcht_mpls_t(label=0xabcde, exp=0x5, ttl=0x30, bos=0)
        tun_info = switcht_tunnel_info_t(
            tunnel_type=SWITCH_TUNNEL_TYPE_MPLS,
            vrf_handle=vrf,
            egress_rif_handle=rif1,
            mpls_mode=SWITCH_MPLS_MODE_TRANSIT,
            pop_tag=[pop_tag])
        ift3 = self.client.switch_api_tunnel_interface_create(device, 0,
                                                              tun_info)

        # neighbor type 2 is swap l2vpn
        neighbor_entry1 = switcht_neighbor_info_t(
            neigh_type=SWITCH_API_NEIGHBOR_MPLS_SWAP_L2VPN,
            nhop_handle=nhop1,
            interface_handle=ift3,
            mac_addr='00:44:44:44:44:44',
            mpls_label=0x98765,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L2)
        neighbor1 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry1)

        neighbor_entry2 = switcht_neighbor_info_t(
            nhop_handle=0, interface_handle=ift3, mac_addr='00:44:44:44:44:44')
        neighbor2 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry2)
        try:
            old_tag = {'label': 0xabcde, 'tc': 0x5, 'ttl': 0x30, 's': 0x0}
            new_tag = {'label': 0x98765, 'tc': 0x5, 'ttl': 0x2f, 's': 0x0}
            inner_tag = {'label': 0x54321, 'tc': 0x2, 'ttl': 0x40, 's': 0x1}
            mpls_tags1 = [old_tag, inner_tag]
            mpls_tags2 = [new_tag, inner_tag]
            pkt = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63)
            mpls_pkt1 = simple_mpls_packet(
                eth_src='00:33:33:33:33:33',
                eth_dst='00:77:66:55:44:33',
                mpls_tags=mpls_tags1,
                inner_frame=pkt)
            mpls_pkt2 = simple_mpls_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:44:44:44:44:44',
                mpls_tags=mpls_tags2,
                inner_frame=pkt)
            send_packet(self, swports[0], str(mpls_pkt1))
            verify_packets(self, mpls_pkt2, [swports[1]])
        finally:

            self.client.switch_api_neighbor_delete(device, neighbor2)
            self.client.switch_api_neighbor_delete(device, neighbor1)
            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_tunnel_interface_delete(device, ift3)
            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('l2')
@group('l3')
@group('mpls')
@group('tunnel')
@group('maxsizes')
@group('2porttests')
class L3MplsPopTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        return

        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf,
                                                        i_ip2)

        lognet_info = switcht_logical_network_t(
            vrf_handle=vrf, rmac_handle=rmac, ipv4_unicast_enabled=True)
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)

        mpls_tag1 = switcht_mpls_t(label=0xabcde, exp=0x5, ttl=0x30, bos=0)
        mpls_tag2 = switcht_mpls_t(label=0x54321, exp=0x2, ttl=0x40, bos=1)
        pop_tag = [mpls_tag1, mpls_tag2]
        tun_info = switcht_tunnel_info_t(
            tunnel_type=SWITCH_TUNNEL_TYPE_MPLS,
            vrf_handle=vrf,
            egress_rif_handle=rif1,
            mpls_mode=SWITCH_MPLS_MODE_TERMINATE,
            mpls_type=SWITCH_MPLS_TYPE_IPV4_MPLS,
            pop_tag=pop_tag)
        ift3 = self.client.switch_api_tunnel_interface_create(device, 0,
                                                              tun_info)

        self.client.switch_api_logical_network_member_add(device, ln1, if1)
        self.client.switch_api_logical_network_member_add(device, ln1, ift3)

        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='20.20.20.1',
            prefix_length=32)
        nhop_key = switcht_nhop_key_t(intf_handle=rif2, ip_addr_valid=0)
        nhop = self.client.switch_api_nhop_create(device, nhop_key)
        neighbor_entry = switcht_neighbor_info_t(
            nhop_handle=nhop,
            interface_handle=rif2,
            mac_addr='00:33:33:33:33:33',
            ip_addr=i_ip3,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L3)
        neighbor = self.client.switch_api_neighbor_entry_add(device,
                                                             neighbor_entry)
        self.client.switch_api_l3_route_add(device, vrf, i_ip3, nhop)

        try:
            tag1 = {'label': 0xabcde, 'tc': 0x5, 'ttl': 0xAA, 's': 0x0}
            tag2 = {'label': 0x54321, 'tc': 0x2, 'ttl': 0xBB, 's': 0x1}
            mpls_tags = [tag1, tag2]
            pkt1 = simple_ip_only_packet(
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64,
                pktlen=86)
            mpls_pkt = simple_mpls_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:11:11:11:11:11',
                mpls_tags=mpls_tags,
                inner_frame=pkt1)
            pkt2 = simple_tcp_packet(
                eth_dst='00:33:33:33:33:33',
                eth_src='00:77:66:55:44:33',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63)
            send_packet(self, swports[0], str(mpls_pkt))
            verify_packets(self, pkt2, [swports[1]])
        finally:
            self.client.switch_api_neighbor_delete(device, neighbor)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip3, nhop)
            self.client.switch_api_nhop_delete(device, nhop)

            self.client.switch_api_l3_interface_address_delete(device, rif2,
                                                               vrf, i_ip2)

            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if1)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift3)

            self.client.switch_api_logical_network_delete(device, ln1)

            self.client.switch_api_tunnel_interface_delete(device, ift3)
            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('l2')
@group('l3')
@group('mpls')
@group('tunnel')
@group('maxsizes')
@group('2porttests')
class L3MplsPushTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print

        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        mpls_tag1 = switcht_mpls_t(label=0xabcde, exp=0x5, ttl=0x30)
        mpls_tag2 = switcht_mpls_t(label=0x54321, exp=0x2, ttl=0x40)
        label_list = [mpls_tag1, mpls_tag2]
        label_stack_info = switcht_mpls_label_stack_t(label_list=label_list, bos=True)
        label_stack = self.client.switch_api_mpls_label_stack_create(device, label_stack_info)

        mpls_info = switcht_api_mpls_info_t(
            tunnel_type=SWITCH_MPLS_TUNNEL_TYPE_MPLS,
            vrf_handle=vrf,
            mpls_mode=SWITCH_MPLS_MODE_INITIATE,
            mpls_type=SWITCH_MPLS_TYPE_IPV4_MPLS,
            intf_handle=if1,
            mac_addr='00:44:44:44:44:44')
        mpls = self.client.switch_api_mpls_tunnel_create(device, mpls_info)

        nhop_info = switcht_api_nhop_info_t(
                      nhop_type=SWITCH_NHOP_TYPE_MPLS,
                      nhop_tunnel_type=SWITCH_NHOP_TUNNEL_TYPE_NONE,
                      mpls_handle=mpls,
                      label_stack_handle=label_stack)
        nhop1 = self.client.switch_api_nhop_create(device, nhop_info)

        neighbor_entry1 = switcht_api_neighbor_info_t(
            nhop_handle=nhop1,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L3,
            neighbor_tunnel_type=SWITCH_NEIGHBOR_TUNNEL_TYPE_MPLS_PUSH_L3VPN,
            neighbor_type=SWITCH_NEIGHBOR_TYPE_NHOP)
        neighbor1 = self.client.switch_api_neighbor_create(device, neighbor_entry1)

        i_ip3 = switcht_ip_addr_t(
                addr_type=SWITCH_API_IP_ADDR_V4,
                ipaddr='20.20.20.1',
                prefix_length=32)
        self.client.switch_api_l3_route_add(device, vrf, i_ip3, nhop1)

        try:
            tag1 = {'label': 0xabcde, 'tc': 0x5, 'ttl': 0x30, 's': 0x0}
            tag2 = {'label': 0x54321, 'tc': 0x2, 'ttl': 0x40, 's': 0x1}
            mpls_tags = [tag1, tag2]
            pkt1 = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            pkt2 = simple_ip_only_packet(
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63,
                pktlen=86)
            mpls_pkt = simple_mpls_packet(
                eth_src='00:ba:7e:f0:00:00',
                eth_dst='00:44:44:44:44:44',
                mpls_tags=mpls_tags,
                inner_frame=pkt2)
            send_packet(self, swports[1], str(pkt1))
            verify_packets(self, mpls_pkt, [swports[0]])
        finally:

            self.client.switch_api_neighbor_delete(device, neighbor1)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip3, nhop1)
            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_mpls_tunnel_delete(device, mpls)
            self.client.switch_api_mpls_label_stack_delete(device, label_stack)
            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('l2')
@group('l3')
@group('mpls')
@group('tunnel')
@group('maxsizes')
@group('mcast')
@group('non-vxlan-tunnel')
class L2TunnelSplicingExtreme1Test(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        return
        print "L2 Geneve-Vxlan-Mpls Enhanced Mode Unicast Test"

        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_OUTER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, vrf_handle=vrf, rmac_handle=rmac)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, vrf_handle=vrf, rmac_handle=rmac)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, vrf_handle=vrf, rmac_handle=rmac)
        rif3 = self.client.switch_api_rif_create(0, rif_info3)
        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)

        # Create a logical network (LN)
        lognet_info = switcht_logical_network_t()
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)

        tunnel_mapper1 = switcht_tunnel_mapper_t(
            tunnel_vni=0x4321, ln_handle=ln1)
        mapper_handle1 = self.client.switch_api_tunnel_mapper_create(
            device,
            direction=SWITCH_API_DIRECTION_BOTH,
            tunnel_type=SWITCH_TUNNEL_TYPE_GENEVE,
            tunnel_mapper_list=[tunnel_mapper1])

        # Create a tunnel interface
        src_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.1', prefix_length=32)
        dst_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.3', prefix_length=32)
        tunnel_info = switcht_tunnel_info_t(
            vrf_handle=vrf,
            tunnel_type=SWITCH_TUNNEL_TYPE_GENEVE,
            src_ip=src_ip4,
            dst_ip=dst_ip4,
            decap_mapper_handle=mapper_handle1,
            encap_mapper_handle=mapper_handle1,
            egress_rif_handle=rif1)
        ift4 = self.client.switch_api_tunnel_interface_create(
            device, SWITCH_API_DIRECTION_BOTH, tunnel_info)

        tunnel_mapper2 = switcht_tunnel_mapper_t(
            tunnel_vni=0x1234, ln_handle=ln1)
        mapper_handle2 = self.client.switch_api_tunnel_mapper_create(
            device,
            direction=SWITCH_API_DIRECTION_BOTH,
            tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
            tunnel_mapper_list=[tunnel_mapper2])

        # Create a tunnel interface
        src_ip5 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='2.2.2.1', prefix_length=32)
        dst_ip5 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='2.2.2.3', prefix_length=32)
        tunnel_info = switcht_tunnel_info_t(
            vrf_handle=vrf,
            tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
            src_ip=src_ip5,
            dst_ip=dst_ip5,
            decap_mapper_handle=mapper_handle2,
            encap_mapper_handle=mapper_handle2,
            egress_rif_handle=rif2)
        ift5 = self.client.switch_api_tunnel_interface_create(
            device, SWITCH_API_DIRECTION_BOTH, tunnel_info)

        mpls_tag1 = switcht_mpls_t(label=0xaaaaa, exp=0x1, ttl=0x30, bos=0)
        mpls_tag2 = switcht_mpls_t(label=0xbbbbb, exp=0x2, ttl=0x40, bos=1)
        push_tag = [mpls_tag1, mpls_tag2]
        tun_info = switcht_tunnel_info_t(
            tunnel_type=SWITCH_TUNNEL_TYPE_MPLS,
            vrf_handle=vrf,
            egress_rif_handle=rif3,
            mpls_mode=SWITCH_MPLS_MODE_INITIATE,
            mpls_type=SWITCH_MPLS_TYPE_EOMPLS,
            push_tag=push_tag)
        ift6 = self.client.switch_api_tunnel_interface_create(device, 0,
                                                              tun_info)

        mpls_tag1 = switcht_mpls_t(label=0xccccc, exp=0x1, ttl=0x30, bos=0)
        mpls_tag2 = switcht_mpls_t(label=0xddddd, exp=0x2, ttl=0x40, bos=1)
        pop_tag = [mpls_tag1, mpls_tag2]
        tun_info = switcht_tunnel_info_t(
            tunnel_type=SWITCH_TUNNEL_TYPE_MPLS,
            vrf_handle=vrf,
            egress_rif_handle=rif3,
            mpls_mode=SWITCH_MPLS_MODE_TERMINATE,
            mpls_type=SWITCH_MPLS_TYPE_EOMPLS,
            pop_tag=pop_tag)
        ift7 = self.client.switch_api_tunnel_interface_create(device, 0,
                                                              tun_info)

        # add L2 port to LN
        self.client.switch_api_logical_network_member_add(device, ln1, ift4)
        self.client.switch_api_logical_network_member_add(device, ln1, ift5)
        self.client.switch_api_logical_network_member_add(device, ln1, ift6)
        self.client.switch_api_logical_network_member_add(device, ln1, ift7)

        nhop_key4 = switcht_nhop_key_t(intf_handle=ift4, ln_handle=ln1, ip_addr_valid=0)
        nhop4 = self.client.switch_api_nhop_create(device, nhop_key4)
        neighbor_entry4 = switcht_neighbor_info_t(
            nhop_handle=nhop4,
            interface_handle=ift4,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L2,
            neigh_type=SWITCH_API_NEIGHBOR_IPV4_TUNNEL)
        neighbor4 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry4)
        nhop_key1 = switcht_nhop_key_t(intf_handle=rif1, ip_addr_valid=0)
        nhop1 = self.client.switch_api_nhop_create(device, nhop_key1)
        neighbor_entry1 = switcht_neighbor_info_t(
            nhop_handle=nhop1,
            interface_handle=rif1,
            mac_addr='00:33:33:33:33:33',
            ip_addr=src_ip4)
        neighbor1 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry1)

        nhop_key5 = switcht_nhop_key_t(intf_handle=ift5, ln_handle=ln1, ip_addr_valid=0)
        nhop5 = self.client.switch_api_nhop_create(device, nhop_key5)
        neighbor_entry5 = switcht_neighbor_info_t(
            nhop_handle=nhop5,
            interface_handle=ift5,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L2,
            neigh_type=SWITCH_API_NEIGHBOR_IPV4_TUNNEL)
        neighbor5 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry5)
        nhop_key2 = switcht_nhop_key_t(intf_handle=rif2, ip_addr_valid=0)
        nhop2 = self.client.switch_api_nhop_create(device, nhop_key2)
        neighbor_entry2 = switcht_neighbor_info_t(
            nhop_handle=nhop2,
            interface_handle=rif2,
            mac_addr='00:44:44:44:44:44',
            ip_addr=src_ip5)
        neighbor2 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry2)

        # neighbor type 5 is push l2vpn
        nhop_key6 = switcht_nhop_key_t(intf_handle=ift6, ln_handle=ln1, ip_addr_valid=0)
        nhop6 = self.client.switch_api_nhop_create(device, nhop_key6)
        neighbor_entry6 = switcht_neighbor_info_t(
            nhop_handle=nhop6,
            neigh_type=SWITCH_API_NEIGHBOR_MPLS_PUSH_L2VPN,
            interface_handle=ift6,
            mpls_label=0,
            mac_addr='00:55:55:55:55:55',
            header_count=2,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L2)
        neighbor6 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry6)

        self.client.switch_api_l3_route_add(device, vrf, dst_ip4, nhop1)
        self.client.switch_api_l3_route_add(device, vrf, dst_ip5, nhop2)

        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:aa:aa:aa:aa:aa', 2, nhop4)  # geneve mac
        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:bb:bb:bb:bb:bb', 2, nhop5)  # vxlan mac
        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:cc:cc:cc:cc:cc', 2, nhop6)  # mpls mac

        print "L2 Tunnel Splicing - Geneve <-> Vxlan (Enhanced Mode)"
        print "Sending packet from Geneve port1 to Vxlan port2"

        try:
            pkt = simple_tcp_packet(
                eth_dst='00:bb:bb:bb:bb:bb',
                eth_src='00:11:11:11:11:11',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            geneve_pkt = simple_geneve_packet(
                eth_src='00:33:33:33:33:33',
                eth_dst='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=11638,
                with_udp_chksum=False,
                geneve_vni=0x4321,
                inner_frame=pkt)

            udp_sport = entropy_hash(pkt)
            vxlan_pkt = simple_vxlan_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:44:44:44:44:44',
                ip_id=0,
                ip_dst='2.2.2.3',
                ip_src='2.2.2.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)
            send_packet(self, swports[1], str(geneve_pkt))
            verify_packets(self, vxlan_pkt, [swports[2]])

            print "Sending packet from Vxlan port2 to Geneve port1"
            pkt = simple_tcp_packet(
                eth_dst='00:aa:aa:aa:aa:aa',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            vxlan_pkt = simple_vxlan_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:44:44:44:44:44',
                ip_id=0,
                ip_dst='2.2.2.3',
                ip_src='2.2.2.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=574,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)

            udp_sport = entropy_hash(pkt)
            geneve_pkt = simple_geneve_packet(
                eth_dst='00:33:33:33:33:33',
                eth_src='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                geneve_vni=0x4321,
                inner_frame=pkt)
            send_packet(self, swports[2], str(vxlan_pkt))
            verify_packets(self, geneve_pkt, [swports[1]])

            print "Sending packet from Vxlan port2 to Mpls port %d" % swports[
                3], " "
            pkt = simple_tcp_packet(
                eth_dst='00:cc:cc:cc:cc:cc',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            vxlan_pkt = simple_vxlan_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:44:44:44:44:44',
                ip_id=0,
                ip_dst='2.2.2.3',
                ip_src='2.2.2.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=574,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)

            tag1 = {'label': 0xaaaaa, 'tc': 0x1, 'ttl': 0x30, 's': 0x0}
            tag2 = {'label': 0xbbbbb, 'tc': 0x2, 'ttl': 0x40, 's': 0x1}
            mpls_tags = [tag1, tag2]
            mpls_pkt = simple_mpls_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:55:55:55:55:55',
                mpls_tags=mpls_tags,
                inner_frame=pkt)
            send_packet(self, swports[2], str(vxlan_pkt))
            verify_packets(self, mpls_pkt, [swports[3]])

            print "Sending packet from Geneve port1 to Mpls port %d" % swports[
                3], " "
            pkt = simple_tcp_packet(
                eth_dst='00:cc:cc:cc:cc:cc',
                eth_src='00:11:11:11:11:11',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            geneve_pkt = simple_geneve_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:33:33:33:33:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=11638,
                with_udp_chksum=False,
                geneve_vni=0x4321,
                inner_frame=pkt)

            tag1 = {'label': 0xaaaaa, 'tc': 0x1, 'ttl': 0x30, 's': 0x0}
            tag2 = {'label': 0xbbbbb, 'tc': 0x2, 'ttl': 0x40, 's': 0x1}
            mpls_tags = [tag1, tag2]
            mpls_pkt = simple_mpls_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:55:55:55:55:55',
                mpls_tags=mpls_tags,
                inner_frame=pkt)
            send_packet(self, swports[1], str(geneve_pkt))
            verify_packets(self, mpls_pkt, [swports[3]])

            print "Sending packet from Mpls port %d" % swports[
                3], "to Geneve port %d" % swports[1], ""
            pkt = simple_tcp_packet(
                eth_dst='00:aa:aa:aa:aa:aa',
                eth_src='00:cc:cc:cc:cc:cc',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            tag1 = {'label': 0xccccc, 'tc': 0x1, 'ttl': 0x30, 's': 0x0}
            tag2 = {'label': 0xddddd, 'tc': 0x2, 'ttl': 0x40, 's': 0x1}
            mpls_tags = [tag1, tag2]
            mpls_pkt = simple_mpls_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:55:55:55:55:55',
                mpls_tags=mpls_tags,
                inner_frame=pkt)
            udp_sport = entropy_hash(pkt)
            geneve_pkt = simple_geneve_packet(
                eth_dst='00:33:33:33:33:33',
                eth_src='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                geneve_vni=0x4321,
                inner_frame=pkt)
            send_packet(self, swports[3], str(mpls_pkt))
            verify_packets(self, geneve_pkt, [swports[1]])
        finally:

            self.client.switch_api_l3_route_delete(device, vrf, dst_ip4, nhop1)
            self.client.switch_api_l3_route_delete(device, vrf, dst_ip5, nhop2)

            self.client.switch_api_neighbor_delete(device, neighbor4)
            self.client.switch_api_nhop_delete(device, nhop4)

            self.client.switch_api_neighbor_delete(device, neighbor5)
            self.client.switch_api_nhop_delete(device, nhop5)

            self.client.switch_api_neighbor_delete(device, neighbor6)
            self.client.switch_api_nhop_delete(device, nhop6)

            self.client.switch_api_neighbor_delete(device, neighbor1)
            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_neighbor_delete(device, neighbor2)
            self.client.switch_api_nhop_delete(device, nhop2)

            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:aa:aa:aa:aa:aa')
            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:bb:bb:bb:bb:bb')
            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:cc:cc:cc:cc:cc')
            self.client.switch_api_mac_table_entry_flush(
                device, SWITCH_MAC_FLUSH_TYPE_ALL, 0x0, 0x0)

            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift4)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift5)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift6)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift7)
            self.client.switch_api_logical_network_delete(device, ln1)

            self.client.switch_api_tunnel_interface_delete(device, ift4)
            self.client.switch_api_tunnel_interface_delete(device, ift5)
            self.client.switch_api_tunnel_interface_delete(device, ift6)
            self.client.switch_api_tunnel_interface_delete(device, ift7)
            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)
            self.client.switch_api_rif_delete(0, rif3)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)


###############################################################################
@group('bfd')
@group('l2')
@group('flood')
@group('maxsizes')
@group('mcast')
@group('ent')
class L2LagFloodTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Lag and native port flooding - lag1[1, 2], lag2[3, 4], lag3[5, 6], port7, port8"
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

        try:
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
        finally:
            self.client.switch_api_mac_table_entry_flush(
                device, SWITCH_MAC_FLUSH_TYPE_ALL, 0x0, 0x0)
            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)
            self.client.switch_api_vlan_member_remove(device, vlan, if3)
            self.client.switch_api_vlan_member_remove(device, vlan, if4)
            self.client.switch_api_vlan_member_remove(device, vlan, if5)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)
            self.client.switch_api_interface_delete(device, if4)
            self.client.switch_api_interface_delete(device, if5)

            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag1,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port1)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag1,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port2)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag2,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port3)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag2,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port4)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag3,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port5)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag3,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port6)

            self.client.switch_api_lag_delete(device, lag1)
            self.client.switch_api_lag_delete(device, lag2)
            self.client.switch_api_lag_delete(device, lag3)

            self.client.switch_api_vlan_delete(device, vlan)


###############################################################################
@group('l2')
@group('l3')
@group('mpls')
@group('tunnel')
@group('maxsizes')
@group('mcast')
@group('non-vxlan-tunnel')
class L2TunnelSplicingExtreme2Test(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        return
        print "L2 Tunnel Splicing - Extreme2 (Enhanced Mode)"
        print "L2 Geneve-Vxlan--Nvgre-Mpls-L2Lag-L2Native Enhanced Mode Unicast Test"

        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_OUTER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])
        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[4])
        port5 = self.client.switch_api_port_id_to_handle_get(device, swports[5])
        port6 = self.client.switch_api_port_id_to_handle_get(device, swports[6])
        port7 = self.client.switch_api_port_id_to_handle_get(device, swports[7])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, vrf_handle=vrf, rmac_handle=rmac)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, vrf_handle=vrf, rmac_handle=rmac)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, vrf_handle=vrf, rmac_handle=rmac)
        rif3 = self.client.switch_api_rif_create(0, rif_info3)
        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)

        rif_info4 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, vrf_handle=vrf, rmac_handle=rmac)
        rif4 = self.client.switch_api_rif_create(0, rif_info4)
        i_info4 = switcht_interface_info_t(
            handle=port4, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif4)
        if4 = self.client.switch_api_interface_create(device, i_info4)

        lag1 = self.client.switch_api_lag_create(device)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag1, side=SWITCH_API_DIRECTION_BOTH, port=port5)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag1, side=SWITCH_API_DIRECTION_BOTH, port=port6)
        i_info5 = switcht_interface_info_t(
            handle=lag1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if5 = self.client.switch_api_interface_create(device, i_info5)

        i_info7 = switcht_interface_info_t(
            handle=port7, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if7 = self.client.switch_api_interface_create(device, i_info7)

        # Create a logical network (LN)
        lognet_info = switcht_logical_network_t()
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)

        tunnel_mapper1 = switcht_tunnel_mapper_t(
            tunnel_vni=0x4321, ln_handle=ln1)
        mapper_handle1 = self.client.switch_api_tunnel_mapper_create(
            device,
            direction=SWITCH_API_DIRECTION_BOTH,
            tunnel_type=SWITCH_TUNNEL_TYPE_GENEVE,
            tunnel_mapper_list=[tunnel_mapper1])

        # Create a tunnel interface
        src_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.1', prefix_length=32)
        dst_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.3', prefix_length=32)
        tunnel_info = switcht_tunnel_info_t(
            vrf_handle=vrf,
            tunnel_type=SWITCH_TUNNEL_TYPE_GENEVE,
            src_ip=src_ip4,
            dst_ip=dst_ip4,
            decap_mapper_handle=mapper_handle1,
            encap_mapper_handle=mapper_handle1,
            egress_rif_handle=rif1)
        if11 = self.client.switch_api_tunnel_interface_create(
            device, SWITCH_API_DIRECTION_BOTH, tunnel_info)

        tunnel_mapper2 = switcht_tunnel_mapper_t(
            tunnel_vni=0x1234, ln_handle=ln1)
        mapper_handle2 = self.client.switch_api_tunnel_mapper_create(
            device,
            direction=SWITCH_API_DIRECTION_BOTH,
            tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
            tunnel_mapper_list=[tunnel_mapper2])

        # Create a tunnel interface
        src_ip5 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='2.2.2.1', prefix_length=32)
        dst_ip5 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='2.2.2.3', prefix_length=32)
        tunnel_info = switcht_tunnel_info_t(
            vrf_handle=vrf,
            tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
            src_ip=src_ip5,
            dst_ip=dst_ip5,
            decap_mapper_handle=mapper_handle2,
            encap_mapper_handle=mapper_handle2,
            egress_rif_handle=rif2)
        if21 = self.client.switch_api_tunnel_interface_create(
            device, SWITCH_API_DIRECTION_BOTH, tunnel_info)

        mpls_tag1 = switcht_mpls_t(label=0xaaaaa, exp=0x1, ttl=0x30, bos=0)
        mpls_tag2 = switcht_mpls_t(label=0xbbbbb, exp=0x2, ttl=0x40, bos=1)
        push_tag = [mpls_tag1, mpls_tag2]
        tun_info = switcht_tunnel_info_t(
            tunnel_type=SWITCH_TUNNEL_TYPE_MPLS,
            vrf_handle=vrf,
            egress_rif_handle=rif3,
            mpls_mode=SWITCH_MPLS_MODE_INITIATE,
            mpls_type=SWITCH_MPLS_TYPE_EOMPLS,
            push_tag=push_tag)
        if31 = self.client.switch_api_tunnel_interface_create(device, 0,
                                                              tun_info)

        mpls_tag1 = switcht_mpls_t(label=0xccccc, exp=0x1, ttl=0x30, bos=0)
        mpls_tag2 = switcht_mpls_t(label=0xddddd, exp=0x2, ttl=0x40, bos=1)
        pop_tag = [mpls_tag1, mpls_tag2]
        tun_info = switcht_tunnel_info_t(
            tunnel_type=SWITCH_TUNNEL_TYPE_MPLS,
            vrf_handle=vrf,
            egress_rif_handle=rif3,
            mpls_mode=SWITCH_MPLS_MODE_TERMINATE,
            mpls_type=SWITCH_MPLS_TYPE_EOMPLS,
            pop_tag=pop_tag)
        if32 = self.client.switch_api_tunnel_interface_create(device, 0,
                                                              tun_info)

        # Create a tunnel interface
        tunnel_mapper3 = switcht_tunnel_mapper_t(
            tunnel_vni=0x4545, ln_handle=ln1)
        mapper_handle3 = self.client.switch_api_tunnel_mapper_create(
            device,
            direction=SWITCH_API_DIRECTION_BOTH,
            tunnel_type=SWITCH_TUNNEL_TYPE_NVGRE,
            tunnel_mapper_list=[tunnel_mapper3])

        src_ip6 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='3.3.3.1', prefix_length=32)
        dst_ip6 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='3.3.3.3', prefix_length=32)
        tunnel_info = switcht_tunnel_info_t(
            vrf_handle=vrf,
            tunnel_type=SWITCH_TUNNEL_TYPE_NVGRE,
            src_ip=src_ip6,
            dst_ip=dst_ip6,
            decap_mapper_handle=mapper_handle3,
            encap_mapper_handle=mapper_handle3,
            egress_rif_handle=rif4)
        if41 = self.client.switch_api_tunnel_interface_create(
            device, SWITCH_API_DIRECTION_BOTH, tunnel_info)

        # add L2 port to LN
        self.client.switch_api_logical_network_member_add(device, ln1, if5)
        self.client.switch_api_logical_network_member_add(device, ln1, if7)
        self.client.switch_api_logical_network_member_add(device, ln1, if11)
        self.client.switch_api_logical_network_member_add(device, ln1, if21)
        self.client.switch_api_logical_network_member_add(device, ln1, if31)
        self.client.switch_api_logical_network_member_add(device, ln1, if32)
        self.client.switch_api_logical_network_member_add(device, ln1, if41)

        nhop_key11 = switcht_nhop_key_t(intf_handle=if11, ln_handle=ln1, ip_addr_valid=0)
        nhop11 = self.client.switch_api_nhop_create(device, nhop_key11)
        neighbor_entry11 = switcht_neighbor_info_t(
            nhop_handle=nhop11,
            interface_handle=if11,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L2,
            neigh_type=SWITCH_API_NEIGHBOR_IPV4_TUNNEL)
        neighbor11 = self.client.switch_api_neighbor_entry_add(device,
                                                               neighbor_entry11)

        nhop_key12 = switcht_nhop_key_t(intf_handle=rif1, ip_addr_valid=0)
        nhop12 = self.client.switch_api_nhop_create(device, nhop_key12)
        neighbor_entry12 = switcht_neighbor_info_t(
            nhop_handle=nhop12,
            interface_handle=rif1,
            mac_addr='00:33:33:33:33:33',
            ip_addr=src_ip4)
        neighbor12 = self.client.switch_api_neighbor_entry_add(device,
                                                               neighbor_entry12)

        nhop_key21 = switcht_nhop_key_t(intf_handle=if21, ln_handle=ln1, ip_addr_valid=0)
        nhop21 = self.client.switch_api_nhop_create(device, nhop_key21)
        neighbor_entry21 = switcht_neighbor_info_t(
            nhop_handle=nhop21,
            interface_handle=if21,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L2,
            neigh_type=SWITCH_API_NEIGHBOR_IPV4_TUNNEL)
        neighbor21 = self.client.switch_api_neighbor_entry_add(device,
                                                               neighbor_entry21)

        nhop_key22 = switcht_nhop_key_t(intf_handle=rif2, ip_addr_valid=0)
        nhop22 = self.client.switch_api_nhop_create(device, nhop_key22)
        neighbor_entry22 = switcht_neighbor_info_t(
            nhop_handle=nhop22,
            interface_handle=rif2,
            mac_addr='00:44:44:44:44:44',
            ip_addr=src_ip5)
        neighbor22 = self.client.switch_api_neighbor_entry_add(device,
                                                               neighbor_entry22)

        # neighbor type 5 is push l2vpn
        nhop_key31 = switcht_nhop_key_t(intf_handle=if31, ln_handle=ln1, ip_addr_valid=0)
        nhop31 = self.client.switch_api_nhop_create(device, nhop_key31)
        neighbor_entry31 = switcht_neighbor_info_t(
            nhop_handle=nhop31,
            neigh_type=SWITCH_API_NEIGHBOR_MPLS_PUSH_L2VPN,
            interface_handle=if31,
            mac_addr='00:55:55:55:55:55',
            mpls_label=0,
            header_count=2,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L2)
        neighbor31 = self.client.switch_api_neighbor_entry_add(device,
                                                               neighbor_entry31)

        nhop_key41 = switcht_nhop_key_t(intf_handle=if41, ln_handle=ln1, ip_addr_valid=0)
        nhop41 = self.client.switch_api_nhop_create(device, nhop_key41)
        neighbor_entry41 = switcht_neighbor_info_t(
            nhop_handle=nhop41,
            interface_handle=if41,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L2,
            neigh_type=SWITCH_API_NEIGHBOR_IPV4_TUNNEL)
        neighbor41 = self.client.switch_api_neighbor_entry_add(device,
                                                               neighbor_entry41)

        nhop_key42 = switcht_nhop_key_t(intf_handle=rif4, ip_addr_valid=0)
        nhop42 = self.client.switch_api_nhop_create(device, nhop_key42)
        neighbor_entry42 = switcht_neighbor_info_t(
            nhop_handle=nhop42,
            interface_handle=rif4,
            mac_addr='00:66:66:66:66:66',
            ip_addr=src_ip6)
        neighbor42 = self.client.switch_api_neighbor_entry_add(device,
                                                               neighbor_entry42)

        self.client.switch_api_l3_route_add(device, vrf, dst_ip4, nhop12)
        self.client.switch_api_l3_route_add(device, vrf, dst_ip5, nhop22)
        self.client.switch_api_l3_route_add(device, vrf, dst_ip6, nhop42)

        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:01:00:00:00:11', 2, nhop11)  # geneve mac
        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:02:00:00:00:21', 2, nhop21)  # vxlan mac
        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:03:00:00:00:31', 2, nhop31)  # mpls mac
        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:04:00:00:00:41', 2, nhop41)  # mpls mac
        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:05:00:00:00:05', 2, if5)  # l2 lag port mac
        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:07:00:00:00:07', 2, if7)  # l2 native port mac

        print "Sending packet from Geneve port1 to Vxlan port2"
        try:
            pkt = simple_tcp_packet(
                eth_dst='00:02:00:00:00:21',
                eth_src='00:01:00:00:00:11',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            geneve_pkt = simple_geneve_packet(
                eth_src='00:33:33:33:33:33',
                eth_dst='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=11638,
                with_udp_chksum=False,
                geneve_vni=0x4321,
                inner_frame=pkt)

            udp_sport = entropy_hash(pkt)
            vxlan_pkt = simple_vxlan_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:44:44:44:44:44',
                ip_id=0,
                ip_dst='2.2.2.3',
                ip_src='2.2.2.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)
            send_packet(self, swports[1], str(geneve_pkt))
            verify_packets(self, vxlan_pkt, [swports[2]])

            print "Sending packet from Vxlan port2 to Geneve port1"
            pkt = simple_tcp_packet(
                eth_dst='00:01:00:00:00:11',
                eth_src='00:02:00:00:00:21',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            vxlan_pkt = simple_vxlan_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:44:44:44:44:44',
                ip_id=0,
                ip_dst='2.2.2.3',
                ip_src='2.2.2.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=574,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)

            udp_sport = entropy_hash(pkt)
            geneve_pkt = simple_geneve_packet(
                eth_dst='00:33:33:33:33:33',
                eth_src='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                geneve_vni=0x4321,
                inner_frame=pkt)
            send_packet(self, swports[2], str(vxlan_pkt))
            verify_packets(self, geneve_pkt, [swports[1]])

            print "Sending packet from Vxlan port2 to Mpls port %d" % swports[
                3], " "
            pkt = simple_tcp_packet(
                eth_dst='00:03:00:00:00:31',
                eth_src='00:02:00:00:00:21',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            vxlan_pkt = simple_vxlan_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:44:44:44:44:44',
                ip_id=0,
                ip_dst='2.2.2.3',
                ip_src='2.2.2.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=574,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)

            tag1 = {'label': 0xaaaaa, 'tc': 0x1, 'ttl': 0x30, 's': 0x0}
            tag2 = {'label': 0xbbbbb, 'tc': 0x2, 'ttl': 0x40, 's': 0x1}
            mpls_tags = [tag1, tag2]
            mpls_pkt = simple_mpls_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:55:55:55:55:55',
                mpls_tags=mpls_tags,
                inner_frame=pkt)
            send_packet(self, swports[2], str(vxlan_pkt))
            verify_packets(self, mpls_pkt, [swports[3]])

            print "Sending packet from Geneve port1 to Mpls port %d" % swports[
                3], " "
            pkt = simple_tcp_packet(
                eth_dst='00:03:00:00:00:31',
                eth_src='00:01:00:00:00:11',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            geneve_pkt = simple_geneve_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:33:33:33:33:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=11638,
                with_udp_chksum=False,
                geneve_vni=0x4321,
                inner_frame=pkt)

            tag1 = {'label': 0xaaaaa, 'tc': 0x1, 'ttl': 0x30, 's': 0x0}
            tag2 = {'label': 0xbbbbb, 'tc': 0x2, 'ttl': 0x40, 's': 0x1}
            mpls_tags = [tag1, tag2]
            mpls_pkt = simple_mpls_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:55:55:55:55:55',
                mpls_tags=mpls_tags,
                inner_frame=pkt)
            send_packet(self, swports[1], str(geneve_pkt))
            verify_packets(self, mpls_pkt, [swports[3]])

            print "Sending packet from Mpls port %d" % swports[
                3], "  to Geneve port %d" % swports[1], ""
            pkt = simple_tcp_packet(
                eth_dst='00:01:00:00:00:11',
                eth_src='00:03:00:00:00:31',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            tag1 = {'label': 0xccccc, 'tc': 0x1, 'ttl': 0x30, 's': 0x0}
            tag2 = {'label': 0xddddd, 'tc': 0x2, 'ttl': 0x40, 's': 0x1}
            mpls_tags = [tag1, tag2]
            mpls_pkt = simple_mpls_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:55:55:55:55:55',
                mpls_tags=mpls_tags,
                inner_frame=pkt)
            udp_sport = entropy_hash(pkt)
            geneve_pkt = simple_geneve_packet(
                eth_dst='00:33:33:33:33:33',
                eth_src='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                geneve_vni=0x4321,
                inner_frame=pkt)
            send_packet(self, swports[3], str(mpls_pkt))
            verify_packets(self, geneve_pkt, [swports[1]])

            print "Sending packet from Geneve port1 to Nvgre port4"
            pkt = simple_tcp_packet(
                eth_dst='00:04:00:00:00:41',
                eth_src='00:01:00:00:00:11',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            geneve_pkt = simple_geneve_packet(
                eth_src='00:33:33:33:33:33',
                eth_dst='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=11638,
                with_udp_chksum=False,
                geneve_vni=0x4321,
                inner_frame=pkt)

            nvgre_flowid = entropy_hash(pkt) & 0xFF
            nvgre_pkt = simple_nvgre_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:66:66:66:66:66',
                ip_id=0,
                ip_dst='3.3.3.3',
                ip_src='3.3.3.1',
                ip_ttl=64,
                ip_flags=0x2,
                nvgre_tni=0x4545,
                nvgre_flowid=nvgre_flowid,
                inner_frame=pkt)
            send_packet(self, swports[1], str(geneve_pkt))
            verify_packets(self, nvgre_pkt, [swports[4]])

            print "Sending packet from Vxlan port2 to Nvgre port4"
            pkt = simple_tcp_packet(
                eth_dst='00:04:00:00:00:41',
                eth_src='00:02:00:00:00:21',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            vxlan_pkt = simple_vxlan_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:44:44:44:44:44',
                ip_id=0,
                ip_dst='2.2.2.3',
                ip_src='2.2.2.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=574,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)

            nvgre_flowid = entropy_hash(pkt) & 0xFF
            nvgre_pkt = simple_nvgre_packet(
                eth_dst='00:66:66:66:66:66',
                eth_src='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='3.3.3.3',
                ip_src='3.3.3.1',
                ip_ttl=64,
                ip_flags=0x2,
                nvgre_tni=0x4545,
                nvgre_flowid=nvgre_flowid,
                inner_frame=pkt)
            send_packet(self, swports[2], str(vxlan_pkt))
            verify_packets(self, nvgre_pkt, [swports[4]])

            print "Sending packet from Nvgre port4 to Geneve port1"
            pkt = simple_tcp_packet(
                eth_dst='00:01:00:00:00:11',
                eth_src='00:04:00:00:00:41',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            nvgre_pkt = simple_nvgre_packet(
                eth_src='00:66:66:66:66:66',
                eth_dst='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='3.3.3.3',
                ip_src='3.3.3.1',
                ip_flags=0x2,
                ip_ttl=64,
                nvgre_tni=0x4545,
                inner_frame=pkt)

            udp_sport = entropy_hash(pkt)
            geneve_pkt = simple_geneve_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:33:33:33:33:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                geneve_vni=0x4321,
                inner_frame=pkt)
            send_packet(self, swports[4], str(nvgre_pkt))
            verify_packets(self, geneve_pkt, [swports[1]])

            print "Sending packet from Nvgre port4 to Vxlan port2"
            pkt = simple_tcp_packet(
                eth_dst='00:02:00:00:00:21',
                eth_src='00:04:00:00:00:41',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            nvgre_pkt = simple_nvgre_packet(
                eth_src='00:66:66:66:66:66',
                eth_dst='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='3.3.3.3',
                ip_src='3.3.3.1',
                ip_flags=0x2,
                ip_ttl=64,
                nvgre_tni=0x4545,
                inner_frame=pkt)

            udp_sport = entropy_hash(pkt)
            vxlan_pkt = simple_vxlan_packet(
                eth_dst='00:44:44:44:44:44',
                eth_src='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='2.2.2.3',
                ip_src='2.2.2.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)

            send_packet(self, swports[4], str(nvgre_pkt))
            verify_packets(self, vxlan_pkt, [swports[2]])

            print "Sending packet from Nvgre port4 to Mpls port %d" % swports[
                3], " "
            pkt = simple_tcp_packet(
                eth_dst='00:03:00:00:00:31',
                eth_src='00:04:00:00:00:41',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            nvgre_pkt = simple_nvgre_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:66:66:66:66:66',
                ip_id=0,
                ip_dst='3.3.3.3',
                ip_src='3.3.3.1',
                ip_ttl=64,
                ip_flags=0x2,
                nvgre_tni=0x4545,
                inner_frame=pkt)

            tag1 = {'label': 0xaaaaa, 'tc': 0x1, 'ttl': 0x30, 's': 0x0}
            tag2 = {'label': 0xbbbbb, 'tc': 0x2, 'ttl': 0x40, 's': 0x1}
            mpls_tags = [tag1, tag2]
            mpls_pkt = simple_mpls_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:55:55:55:55:55',
                mpls_tags=mpls_tags,
                inner_frame=pkt)
            send_packet(self, swports[4], str(nvgre_pkt))
            verify_packets(self, mpls_pkt, [swports[3]])

            print "Sending packet from Mpls port %d" % swports[
                3], "to Nvgre port %d" % swports[4], ""
            pkt = simple_tcp_packet(
                eth_dst='00:04:00:00:00:41',
                eth_src='00:03:00:00:00:31',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            tag1 = {'label': 0xccccc, 'tc': 0x1, 'ttl': 0x30, 's': 0x0}
            tag2 = {'label': 0xddddd, 'tc': 0x2, 'ttl': 0x40, 's': 0x1}
            mpls_tags = [tag1, tag2]
            mpls_pkt = simple_mpls_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:55:55:55:55:55',
                mpls_tags=mpls_tags,
                inner_frame=pkt)
            nvgre_flowid = entropy_hash(pkt) & 0xFF
            nvgre_pkt = simple_nvgre_packet(
                eth_dst='00:66:66:66:66:66',
                eth_src='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='3.3.3.3',
                ip_src='3.3.3.1',
                ip_ttl=64,
                ip_flags=0x2,
                nvgre_tni=0x4545,
                nvgre_flowid=nvgre_flowid,
                inner_frame=pkt)
            send_packet(self, swports[3], str(mpls_pkt))
            verify_packets(self, nvgre_pkt, [swports[4]])

            print "Sending packet from native port7 to Geneve port %d" % swports[
                1], ""
            pkt = simple_tcp_packet(
                eth_dst='00:01:00:00:00:11',
                eth_src='00:07:00:00:00:71',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            udp_sport = entropy_hash(pkt)
            geneve_pkt = simple_geneve_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:33:33:33:33:33',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                geneve_vni=0x4321,
                inner_frame=pkt)
            send_packet(self, swports[7], str(pkt))
            verify_packets(self, geneve_pkt, [swports[1]])

            print "Sending packet from native port7 to Vxlan port %d" % swports[
                2], ""
            pkt = simple_tcp_packet(
                eth_dst='00:02:00:00:00:21',
                eth_src='00:07:00:00:00:71',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            udp_sport = entropy_hash(pkt)
            vxlan_pkt = simple_vxlan_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:44:44:44:44:44',
                ip_id=0,
                ip_dst='2.2.2.3',
                ip_src='2.2.2.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)
            send_packet(self, swports[7], str(pkt))
            verify_packets(self, vxlan_pkt, [swports[2]])

            print "Sending packet from native port7 to Nvgre port %d" % swports[
                4], ""
            pkt = simple_tcp_packet(
                eth_dst='00:04:00:00:00:41',
                eth_src='00:07:00:00:00:71',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            nvgre_flowid = entropy_hash(pkt) & 0xFF
            nvgre_pkt = simple_nvgre_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:66:66:66:66:66',
                ip_id=0,
                ip_dst='3.3.3.3',
                ip_src='3.3.3.1',
                ip_ttl=64,
                ip_flags=0x2,
                nvgre_tni=0x4545,
                nvgre_flowid=nvgre_flowid,
                inner_frame=pkt)

            send_packet(self, swports[7], str(pkt))
            verify_packets(self, nvgre_pkt, [swports[4]])
        finally:
            self.client.switch_api_l3_route_delete(device, vrf, dst_ip4, nhop12)
            self.client.switch_api_l3_route_delete(device, vrf, dst_ip5, nhop22)
            self.client.switch_api_l3_route_delete(device, vrf, dst_ip6, nhop42)

            self.client.switch_api_neighbor_delete(device, neighbor11)
            self.client.switch_api_nhop_delete(device, nhop11)

            self.client.switch_api_neighbor_delete(device, neighbor12)
            self.client.switch_api_nhop_delete(device, nhop12)

            self.client.switch_api_neighbor_delete(device, neighbor21)
            self.client.switch_api_nhop_delete(device, nhop21)

            self.client.switch_api_neighbor_delete(device, neighbor22)
            self.client.switch_api_nhop_delete(device, nhop22)

            self.client.switch_api_neighbor_delete(device, neighbor31)
            self.client.switch_api_nhop_delete(device, nhop31)

            self.client.switch_api_neighbor_delete(device, neighbor41)
            self.client.switch_api_nhop_delete(device, nhop41)

            self.client.switch_api_neighbor_delete(device, neighbor42)
            self.client.switch_api_nhop_delete(device, nhop42)

            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:01:00:00:00:11')
            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:02:00:00:00:21')
            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:03:00:00:00:31')
            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:04:00:00:00:41')
            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:05:00:00:00:05')
            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:07:00:00:00:07')

            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if11)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if21)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if31)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if32)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if41)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if5)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if7)
            self.client.switch_api_logical_network_delete(device, ln1)

            self.client.switch_api_tunnel_interface_delete(device, if11)
            self.client.switch_api_tunnel_interface_delete(device, if21)
            self.client.switch_api_tunnel_interface_delete(device, if31)
            self.client.switch_api_tunnel_interface_delete(device, if32)
            self.client.switch_api_tunnel_interface_delete(device, if41)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)
            self.client.switch_api_interface_delete(device, if4)
            self.client.switch_api_interface_delete(device, if5)
            self.client.switch_api_interface_delete(device, if7)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)
            self.client.switch_api_rif_delete(0, rif3)
            self.client.switch_api_rif_delete(0, rif4)

            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag1,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port5)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag1,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port6)
            self.client.switch_api_lag_delete(device, lag1)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)


###############################################################################
@group('bfd')
@group('l2')
@group('l3')
@group('learn')
@group('tunnel')
@group('maxsizes')
@group('mcast')
@group('ent')
class L2VxlanLearnBasicTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "L2 Vxlan Basic Mode Learn Test"
        return

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
                ip_flags=0x2,
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
                ip_flags=0x2,
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
                ip_flags=0x2,
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


###############################################################################
@group('bfd')
@group('l2')
@group('stats')
@group('maxsizes')
@group('2porttests')
@group('ent')
class L2VlanStatsTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Sending L2 packet port %d" % swports[0], " -> port %d" % swports[
            1], " [access vlan=10])"
        vlan = self.client.switch_api_vlan_create(device, 10)
        self.client.switch_api_vlan_stats_enable(device, vlan)

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
            self, device, vlan, '00:11:11:11:11:11', 2, if1)
        switch_api_mac_table_entry_create(
            self, device, vlan, '00:22:22:22:22:22', 2, if2)

        counter0 = self.client.switch_api_vlan_stats_get(device, vlan,
                                                         [0, 1, 2, 4, 5, 6])

        ba = 0
        if test_param_get('target') == "hw" or test_param_get('target') == "asic-model":
            ba = 4

        try:
            num_bytes = 0
            num_packets = 200
            random.seed(314159)
            for i in range(0, num_packets):
                pktlen = random.randint(100, 250)
                pkt = simple_tcp_packet(
                    eth_dst='00:22:22:22:22:22',
                    eth_src='00:11:11:11:11:11',
                    ip_dst='10.10.10.1',
                    ip_src='192.168.8.1',
                    ip_id=106,
                    ip_ttl=64,
                    pktlen=pktlen)

                exp_pkt = simple_tcp_packet(
                    eth_dst='00:22:22:22:22:22',
                    eth_src='00:11:11:11:11:11',
                    ip_dst='10.10.10.1',
                    ip_src='192.168.8.1',
                    ip_id=106,
                    ip_ttl=64,
                    pktlen=pktlen)
                send_packet(self, swports[0], str(pkt))
                verify_each_packet_on_each_port(self, [exp_pkt], [swports[1]])
                num_bytes += pktlen

            time.sleep(2)
            counter = self.client.switch_api_vlan_stats_get(device, vlan,
                                                            [0, 1, 2, 4, 5, 6])

            print counter0, counter

            for i in range(0, 6):
                counter[i].num_packets = counter[i].num_packets - counter0[
                    i].num_packets
                counter[i].num_bytes = counter[i].num_bytes - counter0[
                    i].num_bytes

            print "Stats results: ", counter
            self.assertEqual(counter[0].num_packets, num_packets)
            self.assertEqual(counter[0].num_bytes, num_bytes + ba * num_packets)
            self.assertEqual(counter[1].num_packets, 0)
            self.assertEqual(counter[1].num_bytes, 0)
            self.assertEqual(counter[2].num_packets, 0)
            self.assertEqual(counter[2].num_bytes, 0)
            self.assertEqual(counter[3].num_packets, num_packets)
            # self.assertEqual(counter[3].num_bytes, num_bytes)
            self.assertEqual(counter[4].num_packets, 0)
            self.assertEqual(counter[4].num_bytes, 0)
            self.assertEqual(counter[5].num_packets, 0)
            self.assertEqual(counter[5].num_bytes, 0)
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
@group('stats')
@group('maxsizes')
@group('2porttests')
@group('ent')
class L2LNStatsTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Sending L2 packet port %d" % swports[0], " -> port %d" % swports[
            1], " [logical network])"
        lognet_info = switcht_logical_network_t()
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        self.client.switch_api_logical_network_member_add(device, ln1, if1)
        self.client.switch_api_logical_network_member_add(device, ln1, if2)

        switch_api_mac_table_entry_create(
            self, device, ln1, '00:11:11:11:11:11', 2, if1)
        switch_api_mac_table_entry_create(
            self, device, ln1, '00:22:22:22:22:22', 2, if2)

        counter0 = self.client.switch_api_logical_network_stats_get(device, ln1, [0, 1, 2, 4, 5, 6])

        ba = 0
        if test_param_get('target') == "hw" or test_param_get('target') == "asic-model":
            ba = 4

        try:
            num_bytes = 0
            num_packets = 200
            random.seed(314159)
            for i in range(0, num_packets):
                pktlen = random.randint(100, 250)
                pkt = simple_tcp_packet(
                    eth_dst='00:22:22:22:22:22',
                    eth_src='00:11:11:11:11:11',
                    ip_dst='10.10.10.1',
                    ip_src='192.168.8.1',
                    ip_id=106,
                    ip_ttl=64,
                    pktlen=pktlen)

                exp_pkt = simple_tcp_packet(
                    eth_dst='00:22:22:22:22:22',
                    eth_src='00:11:11:11:11:11',
                    ip_dst='10.10.10.1',
                    ip_src='192.168.8.1',
                    ip_id=106,
                    ip_ttl=64,
                    pktlen=pktlen)
                send_packet(self, swports[0], str(pkt))
                verify_each_packet_on_each_port(self, [exp_pkt], [swports[1]])
                num_bytes += pktlen

            time.sleep(2)
            counter = self.client.switch_api_logical_network_stats_get(device, ln1, [0, 1, 2, 4, 5, 6])

            print counter0, counter

            for i in range(0, 6):
                counter[i].num_packets = counter[i].num_packets - counter0[i].num_packets
                counter[i].num_bytes = counter[i].num_bytes - counter0[i].num_bytes

            print "Stats results: ", counter
            self.assertEqual(counter[0].num_packets, num_packets)
            self.assertEqual(counter[0].num_bytes, num_bytes + ba * num_packets)
            self.assertEqual(counter[1].num_packets, 0)
            self.assertEqual(counter[1].num_bytes, 0)
            self.assertEqual(counter[2].num_packets, 0)
            self.assertEqual(counter[2].num_bytes, 0)
            self.assertEqual(counter[3].num_packets, num_packets)
            # self.assertEqual(counter[3].num_bytes, num_bytes)
            self.assertEqual(counter[4].num_packets, 0)
            self.assertEqual(counter[4].num_bytes, 0)
            self.assertEqual(counter[5].num_packets, 0)
            self.assertEqual(counter[5].num_bytes, 0)
        finally:
            switch_api_mac_table_entry_delete(self, device, ln1, '00:11:11:11:11:11')
            switch_api_mac_table_entry_delete(self, device, ln1, '00:22:22:22:22:22')

            self.client.switch_api_logical_network_member_remove(device, ln1, if1)
            self.client.switch_api_logical_network_member_remove(device, ln1, if2)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_logical_network_delete(device, ln1)


###############################################################################
@group('bfd')
@group('l2')
@group('l3')
@group('tunnel')
@group('flood')
@group('maxsizes')
@group('non-vxlan-tunnel')
class L2TunnelFloodEnhancedTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        return
        print "L2 Tunnel Flooding (Enhanced Mode)"
        print "L2 Geneve-Vxlan--Nvgre-Portvlan Enhanced Mode Flood Test"

        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_OUTER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])
        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[4])
        port5 = self.client.switch_api_port_id_to_handle_get(device, swports[5])
        port6 = self.client.switch_api_port_id_to_handle_get(device, swports[6])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, vrf_handle=vrf, rmac_handle=rmac)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, vrf_handle=vrf, rmac_handle=rmac)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, vrf_handle=vrf, rmac_handle=rmac)
        rif3 = self.client.switch_api_rif_create(0, rif_info3)
        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)

        # Create a logical network (LN)
        lognet_info = switcht_logical_network_t()
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)

        tunnel_mapper1 = switcht_tunnel_mapper_t(
            tunnel_vni=0x4321, ln_handle=ln1)
        mapper_handle1 = self.client.switch_api_tunnel_mapper_create(
            device,
            direction=SWITCH_API_DIRECTION_BOTH,
            tunnel_type=SWITCH_TUNNEL_TYPE_GENEVE,
            tunnel_mapper_list=[tunnel_mapper1])

        # Create a tunnel interface
        src_ip11 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.1', prefix_length=32)
        dst_ip11 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.3', prefix_length=32)
        tunnel_info = switcht_tunnel_info_t(
            vrf_handle=vrf,
            tunnel_type=SWITCH_TUNNEL_TYPE_GENEVE,
            src_ip=src_ip11,
            dst_ip=dst_ip11,
            decap_mapper_handle=mapper_handle1,
            encap_mapper_handle=mapper_handle1,
            egress_rif_handle=rif1)
        if11 = self.client.switch_api_tunnel_interface_create(
            device, SWITCH_API_DIRECTION_BOTH, tunnel_info)

        tunnel_mapper2 = switcht_tunnel_mapper_t(
            tunnel_vni=0x1234, ln_handle=ln1)
        mapper_handle2 = self.client.switch_api_tunnel_mapper_create(
            device,
            direction=SWITCH_API_DIRECTION_BOTH,
            tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
            tunnel_mapper_list=[tunnel_mapper2])

        # Create a tunnel interface
        src_ip21 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='2.2.2.1', prefix_length=32)
        dst_ip21 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='2.2.2.3', prefix_length=32)
        tunnel_info = switcht_tunnel_info_t(
            vrf_handle=vrf,
            tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
            src_ip=src_ip21,
            dst_ip=dst_ip21,
            decap_mapper_handle=mapper_handle2,
            encap_mapper_handle=mapper_handle2,
            egress_rif_handle=rif2)
        if21 = self.client.switch_api_tunnel_interface_create(
            device, SWITCH_API_DIRECTION_BOTH, tunnel_info)

        # Create a tunnel interface
        tunnel_mapper3 = switcht_tunnel_mapper_t(
            tunnel_vni=0x4545, ln_handle=ln1)
        mapper_handle3 = self.client.switch_api_tunnel_mapper_create(
            device,
            direction=SWITCH_API_DIRECTION_BOTH,
            tunnel_type=SWITCH_TUNNEL_TYPE_NVGRE,
            tunnel_mapper_list=[tunnel_mapper3])

        src_ip31 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='3.3.3.1', prefix_length=32)
        dst_ip31 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='3.3.3.3', prefix_length=32)
        tunnel_info = switcht_tunnel_info_t(
            vrf_handle=vrf,
            tunnel_type=SWITCH_TUNNEL_TYPE_NVGRE,
            src_ip=src_ip31,
            dst_ip=dst_ip31,
            decap_mapper_handle=mapper_handle3,
            encap_mapper_handle=mapper_handle3,
            egress_rif_handle=rif3)
        if31 = self.client.switch_api_tunnel_interface_create(
            device, SWITCH_API_DIRECTION_BOTH, tunnel_info)

        self.client.switch_api_port_bind_mode_set(
            device, port4, SWITCH_PORT_BIND_MODE_PORT_VLAN)
        i_info4 = switcht_interface_info_t(
            handle=port4, vlan=10, type=SWITCH_INTERFACE_TYPE_PORT_VLAN)
        if4 = self.client.switch_api_interface_create(device, i_info4)

        self.client.switch_api_port_bind_mode_set(
            device, port5, SWITCH_PORT_BIND_MODE_PORT_VLAN)
        i_info5 = switcht_interface_info_t(
            handle=port5, vlan=20, type=SWITCH_INTERFACE_TYPE_PORT_VLAN)
        if5 = self.client.switch_api_interface_create(device, i_info5)

        i_info6 = switcht_interface_info_t(
            handle=port6, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if6 = self.client.switch_api_interface_create(device, i_info6)

        self.client.switch_api_logical_network_member_add(device, ln1, if4)
        self.client.switch_api_logical_network_member_add(device, ln1, if5)
        self.client.switch_api_logical_network_member_add(device, ln1, if6)

        nhop_key11 = switcht_nhop_key_t(intf_handle=if11, ln_handle=ln1, ip_addr_valid=0)
        nhop11 = self.client.switch_api_nhop_create(device, nhop_key11)
        neighbor_entry11 = switcht_neighbor_info_t(
            nhop_handle=nhop11,
            interface_handle=if11,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L2,
            neigh_type=SWITCH_API_NEIGHBOR_IPV4_TUNNEL)
        neighbor11 = self.client.switch_api_neighbor_entry_add(device,
                                                               neighbor_entry11)

        nhop_key12 = switcht_nhop_key_t(intf_handle=rif1, ip_addr_valid=0)
        nhop12 = self.client.switch_api_nhop_create(device, nhop_key12)
        neighbor_entry12 = switcht_neighbor_info_t(
            nhop_handle=nhop12,
            interface_handle=rif1,
            mac_addr='00:11:11:11:11:11',
            ip_addr=src_ip11)
        neighbor12 = self.client.switch_api_neighbor_entry_add(device,
                                                               neighbor_entry12)

        nhop_key21 = switcht_nhop_key_t(intf_handle=if21, ln_handle=ln1, ip_addr_valid=0)
        nhop21 = self.client.switch_api_nhop_create(device, nhop_key21)
        neighbor_entry21 = switcht_neighbor_info_t(
            nhop_handle=nhop21,
            interface_handle=if21,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L2,
            neigh_type=SWITCH_API_NEIGHBOR_IPV4_TUNNEL)
        neighbor21 = self.client.switch_api_neighbor_entry_add(device,
                                                               neighbor_entry21)

        nhop_key22 = switcht_nhop_key_t(intf_handle=rif2, ip_addr_valid=0)
        nhop22 = self.client.switch_api_nhop_create(device, nhop_key22)
        neighbor_entry22 = switcht_neighbor_info_t(
            nhop_handle=nhop22,
            interface_handle=rif2,
            mac_addr='00:22:22:22:22:22',
            ip_addr=src_ip21)
        neighbor22 = self.client.switch_api_neighbor_entry_add(device,
                                                               neighbor_entry22)

        nhop_key31 = switcht_nhop_key_t(intf_handle=if31, ln_handle=ln1, ip_addr_valid=0)
        nhop31 = self.client.switch_api_nhop_create(device, nhop_key31)
        neighbor_entry31 = switcht_neighbor_info_t(
            nhop_handle=nhop31,
            interface_handle=if31,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L2,
            neigh_type=SWITCH_API_NEIGHBOR_IPV4_TUNNEL)
        neighbor31 = self.client.switch_api_neighbor_entry_add(device,
                                                               neighbor_entry31)

        nhop_key32 = switcht_nhop_key_t(intf_handle=rif3, ln_handle=ln1, ip_addr_valid=0)
        nhop32 = self.client.switch_api_nhop_create(device, nhop_key32)
        neighbor_entry32 = switcht_neighbor_info_t(
            nhop_handle=nhop32,
            interface_handle=rif3,
            mac_addr='00:33:33:33:33:33:33',
            ip_addr=src_ip31)
        neighbor32 = self.client.switch_api_neighbor_entry_add(device,
                                                               neighbor_entry32)
        self.client.switch_api_l3_route_add(device, vrf, dst_ip11, nhop12)
        self.client.switch_api_l3_route_add(device, vrf, dst_ip21, nhop22)
        self.client.switch_api_l3_route_add(device, vrf, dst_ip31, nhop32)

        self.client.switch_api_logical_network_member_add(device, ln1, if11)
        self.client.switch_api_logical_network_member_add(device, ln1, if21)
        self.client.switch_api_logical_network_member_add(device, ln1, if31)

        try:
            pkt = simple_tcp_packet(
                eth_dst='00:02:00:00:00:21',
                eth_src='00:01:00:00:00:11',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            udp_sport = entropy_hash(pkt)
            geneve_pkt = simple_geneve_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:11:11:11:11:11',
                ip_id=0,
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                geneve_vni=0x4321,
                inner_frame=pkt)

            vxlan_pkt = simple_vxlan_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:22:22:22:22:22',
                ip_id=0,
                ip_dst='2.2.2.3',
                ip_src='2.2.2.1',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt)

            nvgre_flowid = udp_sport & 0xFF
            nvgre_pkt = simple_nvgre_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:33:33:33:33:33',
                ip_id=0,
                ip_dst='3.3.3.3',
                ip_src='3.3.3.1',
                ip_ttl=64,
                ip_flags=0x2,
                nvgre_tni=0x4545,
                nvgre_flowid=nvgre_flowid,
                inner_frame=pkt)

            encap_pkt1 = simple_tcp_packet(
                eth_dst='00:02:00:00:00:21',
                eth_src='00:01:00:00:00:11',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64,
                dl_vlan_enable=True,
                vlan_vid=10,
                pktlen=104)

            encap_pkt2 = simple_tcp_packet(
                eth_dst='00:02:00:00:00:21',
                eth_src='00:01:00:00:00:11',
                ip_dst='10.10.10.1',
                ip_id=108,
                ip_ttl=64,
                dl_vlan_enable=True,
                vlan_vid=20,
                pktlen=104)

            print "Sending packet on native access port %d" % swports[6]
            send_packet(self, swports[6], str(pkt))
            print "Packets expected on [geneve port1]. [vxlan port2], [nvgre port3], [encap vlan 10 port %d" % swports[
                4], "], [encap vlan 20 port %d" % swports[5], " ]"
            verify_each_packet_on_each_port(
                self,
                [geneve_pkt, vxlan_pkt, nvgre_pkt, encap_pkt1, encap_pkt2],
                [swports[1], swports[2], swports[3], swports[4], swports[5]])
        finally:

            self.client.switch_api_l3_route_delete(device, vrf, dst_ip11, nhop12)
            self.client.switch_api_l3_route_delete(device, vrf, dst_ip21, nhop22)
            self.client.switch_api_l3_route_delete(device, vrf, dst_ip31, nhop32)

            self.client.switch_api_neighbor_delete(device, neighbor11)
            self.client.switch_api_nhop_delete(device, nhop11)
            self.client.switch_api_neighbor_delete(device, neighbor12)
            self.client.switch_api_nhop_delete(device, nhop12)

            self.client.switch_api_neighbor_delete(device, neighbor21)
            self.client.switch_api_nhop_delete(device, nhop21)
            self.client.switch_api_neighbor_delete(device, neighbor22)
            self.client.switch_api_nhop_delete(device, nhop22)

            self.client.switch_api_neighbor_delete(device, neighbor31)
            self.client.switch_api_nhop_delete(device, nhop31)
            self.client.switch_api_neighbor_delete(device, neighbor32)
            self.client.switch_api_nhop_delete(device, nhop32)

            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if11)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if21)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if31)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if4)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if5)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if6)
            self.client.switch_api_logical_network_delete(device, ln1)

            self.client.switch_api_tunnel_interface_delete(device, if11)
            self.client.switch_api_tunnel_interface_delete(device, if21)
            self.client.switch_api_tunnel_interface_delete(device, if31)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)
            self.client.switch_api_interface_delete(device, if4)
            self.client.switch_api_interface_delete(device, if5)
            self.client.switch_api_interface_delete(device, if6)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)
            self.client.switch_api_rif_delete(0, rif3)

            self.client.switch_api_port_bind_mode_set(
                device, port4, SWITCH_PORT_BIND_MODE_PORT)
            self.client.switch_api_port_bind_mode_set(
                device, port5, SWITCH_PORT_BIND_MODE_PORT)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')


###############################################################################
@group('bfd')
@group('l2')
@group('l3')
@group('ipv4')
@group('clpm')
@group('maxsizes')
@group('ent')
class L3VIIPv4HostTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        vrf = self.client.switch_api_vrf_create(device, 2)

        vlan = self.client.switch_api_vlan_create(device, 10)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)

        switch_api_mac_table_entry_create(
            self, device, vlan, '00:11:11:11:11:11', 2, if1)
        switch_api_mac_table_entry_create(
            self, device, vlan, '00:22:22:22:22:22', 2, if2)

        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1)
        rif3 = self.client.switch_api_rif_create(0, rif_info3)
        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='192.168.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif3, vrf,
                                                        i_ip3)

        rif_info4 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1)
        rif4 = self.client.switch_api_rif_create(0, rif_info4)
        i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif4, vrf,
                                                        i_ip4)

        # Add a static route
        i_ip41 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.1',
            prefix_length=32)
        nhop41, neighbor41 = switch_api_l3_nhop_neighbor_create(self, device, rif4, i_ip41, '00:11:11:11:11:11')
        self.client.switch_api_l3_route_add(device, vrf, i_ip41, nhop41)

        i_ip42 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='11.11.11.1',
            prefix_length=32)
        nhop42, neighbor42 = switch_api_l3_nhop_neighbor_create(self, device, rif4, i_ip42, '00:22:22:22:22:22')
        self.client.switch_api_l3_route_add(device, vrf, i_ip42, nhop42)

        i_ip31 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='12.12.12.1',
            prefix_length=32)
        nhop31, neighbor31 = switch_api_l3_nhop_neighbor_create(self, device, rif3, i_ip31, '00:33:33:33:33:33')
        self.client.switch_api_l3_route_add(device, vrf, i_ip31, nhop31)

        # send the test packet(s)
        try:
            print "Sending packet l3 port %d" % swports[
                3], " to vlan interface port %d" % swports[1]
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:66:66:66:66:66',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=63)
            send_packet(self, swports[3], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

            print "Sending packet l3 port %d" % swports[
                3], " to vlan interface port %d" % swports[2]
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:66:66:66:66:66',
                ip_dst='11.11.11.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:77:66:55:44:33',
                ip_dst='11.11.11.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=63)
            send_packet(self, swports[3], str(pkt))
            verify_packets(self, exp_pkt, [swports[2]])

            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:77:77:77:77:77',
                ip_dst='12.12.12.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:33:33:33:33:33',
                eth_src='00:77:66:55:44:33',
                ip_dst='12.12.12.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=63)

            print "Sending packet vlan interface  port %d" % swports[
                1], " to l3  port %d" % swports[3]
            send_packet(self, swports[1], str(pkt))
            verify_packets(self, exp_pkt, [swports[3]])

            print "Sending packet vlan interface  port %d" % swports[
                2], " to l3  port %d" % swports[3]
            send_packet(self, swports[2], str(pkt))
            verify_packets(self, exp_pkt, [swports[3]])

        finally:
            self.client.switch_api_mac_table_entry_flush(
                device, SWITCH_MAC_FLUSH_TYPE_ALL, 0x0, 0x0)
            self.client.switch_api_neighbor_delete(device, neighbor41)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip41, nhop41)
            self.client.switch_api_nhop_delete(device, nhop41)

            self.client.switch_api_neighbor_delete(device, neighbor42)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip42, nhop42)
            self.client.switch_api_nhop_delete(device, nhop42)

            self.client.switch_api_neighbor_delete(device, neighbor31)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip31, nhop31)
            self.client.switch_api_nhop_delete(device, nhop31)

            self.client.switch_api_l3_interface_address_delete(device, rif3,
                                                               vrf, i_ip3)
            self.client.switch_api_l3_interface_address_delete(device, rif4,
                                                               vrf, i_ip4)

            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_rif_delete(0, rif3)
            self.client.switch_api_rif_delete(0, rif4)

            self.client.switch_api_vlan_delete(device, vlan)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('l2')
@group('l3')
@group('ipv6')
@group('maxsizes')
@group('ent')
class L3VIIPv6HostTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        vrf = self.client.switch_api_vrf_create(device, 2)

        vlan = self.client.switch_api_vlan_create(device, 10)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)

        switch_api_mac_table_entry_create(
            self, device, vlan, '00:11:11:11:11:11', 2, if1)
        switch_api_mac_table_entry_create(
            self, device, vlan, '00:22:22:22:22:22', 2, if2)

        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1)
        rif3 = self.client.switch_api_rif_create(0, rif_info3)
        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='2000::2',
            prefix_length=120)
        self.client.switch_api_l3_interface_address_add(device, rif3, vrf,
                                                        i_ip3)

        rif_info4 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1)
        rif4 = self.client.switch_api_rif_create(0, rif_info4)
        i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='3000::2',
            prefix_length=120)
        self.client.switch_api_l3_interface_address_add(device, rif4, vrf,
                                                        i_ip4)

        # Add a static route
        i_ip41 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='1234:5678:9abc:def0:4422:1133:5577:99aa',
            prefix_length=128)
        nhop41, neighbor41 = switch_api_l3_nhop_neighbor_create(self, device, rif4, i_ip41, '00:11:11:11:11:11')
        self.client.switch_api_l3_route_add(device, vrf, i_ip41, nhop41)

        i_ip42 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='1234:5678:9abc:def0:4422:1133:5577:99bb',
            prefix_length=128)
        nhop42, neighbor42 = switch_api_l3_nhop_neighbor_create(self, device, rif4, i_ip42, '00:22:22:22:22:22')
        self.client.switch_api_l3_route_add(device, vrf, i_ip42, nhop42)

        i_ip31 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='1234:5678:9abc:def0:4422:1133:5577:99cc',
            prefix_length=128)
        nhop31, neighbor31 = switch_api_l3_nhop_neighbor_create(self, device, rif3, i_ip31, '00:33:33:33:33:33')
        self.client.switch_api_l3_route_add(device, vrf, i_ip31, nhop31)

        # send the test packet(s)
        try:
            print "Sending packet l3 port %d" % swports[
                3], " to vlan interface port %d" % swports[1]
            pkt = simple_tcpv6_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:66:66:66:66:66',
                ipv6_dst='1234:5678:9abc:def0:4422:1133:5577:99aa',
                ipv6_src='2000::1',
                ipv6_hlim=64)
            exp_pkt = simple_tcpv6_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='1234:5678:9abc:def0:4422:1133:5577:99aa',
                ipv6_src='2000::1',
                ipv6_hlim=63)
            send_packet(self, swports[3], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

            print "Sending packet l3 port %d" % swports[
                3], " to vlan interface port %d" % swports[2]
            pkt = simple_tcpv6_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:66:66:66:66:66',
                ipv6_dst='1234:5678:9abc:def0:4422:1133:5577:99bb',
                ipv6_src='2000::1',
                ipv6_hlim=64)
            exp_pkt = simple_tcpv6_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='1234:5678:9abc:def0:4422:1133:5577:99bb',
                ipv6_src='2000::1',
                ipv6_hlim=63)
            send_packet(self, swports[3], str(pkt))
            verify_packets(self, exp_pkt, [swports[2]])

            pkt = simple_tcpv6_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:77:77:77:77:77',
                ipv6_dst='1234:5678:9abc:def0:4422:1133:5577:99cc',
                ipv6_src='3000::1',
                ipv6_hlim=64)
            exp_pkt = simple_tcpv6_packet(
                eth_dst='00:33:33:33:33:33',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='1234:5678:9abc:def0:4422:1133:5577:99cc',
                ipv6_src='3000::1',
                ipv6_hlim=63)

            print "Sending packet vlan interface  port %d" % swports[
                1], " to l3  port %d" % swports[3]
            send_packet(self, swports[1], str(pkt))
            verify_packets(self, exp_pkt, [swports[3]])

            print "Sending packet vlan interface  port %d" % swports[
                2], " to l3  port %d" % swports[3]
            send_packet(self, swports[2], str(pkt))
            verify_packets(self, exp_pkt, [swports[3]])

        finally:
            self.client.switch_api_mac_table_entry_flush(
                device, SWITCH_MAC_FLUSH_TYPE_ALL, 0x0, 0x0)
            self.client.switch_api_neighbor_delete(device, neighbor41)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip41, nhop41)
            self.client.switch_api_nhop_delete(device, nhop41)

            self.client.switch_api_neighbor_delete(device, neighbor42)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip42, nhop42)
            self.client.switch_api_nhop_delete(device, nhop42)

            self.client.switch_api_neighbor_delete(device, neighbor31)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip31, nhop31)
            self.client.switch_api_nhop_delete(device, nhop31)

            self.client.switch_api_l3_interface_address_delete(device, rif3,
                                                               vrf, i_ip3)
            self.client.switch_api_l3_interface_address_delete(device, rif4,
                                                               vrf, i_ip4)

            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_rif_delete(0, rif3)
            self.client.switch_api_rif_delete(0, rif4)

            self.client.switch_api_vlan_delete(device, vlan)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('bfd')
@group('l2')
@group('l3')
@group('ipv4')
@group('flood')
@group('clpm')
@group('maxsizes')
@group('ent')
class L3VIIPv4HostFloodTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        vrf = self.client.switch_api_vrf_create(device, 2)

        vlan = self.client.switch_api_vlan_create(device, 10)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)

        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1)
        rif3 = self.client.switch_api_rif_create(0, rif_info3)
        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='192.168.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif3, vrf,
                                                        i_ip3)

        rif_info4 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1)
        i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=16)
        rif4 = self.client.switch_api_rif_create(0, rif_info4)
        self.client.switch_api_l3_interface_address_add(device, rif4, vrf,
                                                        i_ip4)

        # Add a static route
        i_ip41 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.1',
            prefix_length=32)
        nhop41, neighbor41 = switch_api_l3_nhop_neighbor_create(self, device, rif4, i_ip41, '00:11:11:11:11:11')
        self.client.switch_api_l3_route_add(device, vrf, i_ip41, nhop41)

        i_ip42 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='11.11.11.1',
            prefix_length=32)
        nhop42, neighbor42 = switch_api_l3_nhop_neighbor_create(self, device, rif4, i_ip42, '00:22:22:22:22:22')
        self.client.switch_api_l3_route_add(device, vrf, i_ip42, nhop42)

        i_ip31 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='12.12.12.1',
            prefix_length=32)
        nhop31, neighbor31 = switch_api_l3_nhop_neighbor_create(self, device, rif3, i_ip31, '00:33:33:33:33:33')
        self.client.switch_api_l3_route_add(device, vrf, i_ip31, nhop31)

        # send the test packet(s)
        try:
            print "Sending packet l3 port %d " % swports[
                3], "to vlan interface - flood the packet on port %d" % swports[
                    1], " and port %d" % swports[2]
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:66:66:66:66:66',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=63)
            send_packet(self, swports[3], str(pkt))
            verify_packets(self, exp_pkt, [swports[1], swports[2]])

            print "Sending packet l3 port %d " % swports[
                3], "to vlan interface - flood the packet on port %d" % swports[
                    1], " and port %d" % swports[2]
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:77:77:77:77:77',
                ip_dst='11.11.11.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:77:66:55:44:33',
                ip_dst='11.11.11.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=63)
            send_packet(self, swports[3], str(pkt))
            verify_packets(self, exp_pkt, [swports[1], swports[2]])

            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:88:88:88:88:88',
                ip_dst='12.12.12.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:33:33:33:33:33',
                eth_src='00:77:66:55:44:33',
                ip_dst='12.12.12.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=63)

            print "Sending packet vlan interface  port %d" % swports[
                1], " to l3  port %d" % swports[3]
            send_packet(self, swports[1], str(pkt))
            verify_packets(self, exp_pkt, [swports[3]])

            print "Sending packet vlan interface  port %d" % swports[
                2], " to l3  port %d" % swports[3]
            send_packet(self, swports[2], str(pkt))
            verify_packets(self, exp_pkt, [swports[3]])

        finally:
            self.client.switch_api_mac_table_entry_flush(
                device, SWITCH_MAC_FLUSH_TYPE_ALL, 0x0, 0x0)
            self.client.switch_api_neighbor_delete(device, neighbor41)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip41, nhop41)
            self.client.switch_api_nhop_delete(device, nhop41)

            self.client.switch_api_neighbor_delete(device, neighbor42)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip42, nhop42)
            self.client.switch_api_nhop_delete(device, nhop42)

            self.client.switch_api_neighbor_delete(device, neighbor31)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip31, nhop31)
            self.client.switch_api_nhop_delete(device, nhop31)

            self.client.switch_api_l3_interface_address_delete(device, rif3,
                                                               vrf, i_ip3)
            self.client.switch_api_l3_interface_address_delete(device, rif4,
                                                               vrf, i_ip4)

            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_rif_delete(0, rif3)
            self.client.switch_api_rif_delete(0, rif4)

            self.client.switch_api_vlan_delete(device, vlan)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('bfd')
@group('l2')
@group('l3')
@group('ipv4')
@group('flood')
@group('maxsizes')
@group('ent')
class L3VIFloodTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Flood on L3 vlan interface"
        vrf = self.client.switch_api_vrf_create(device, 2)

        vlan = self.client.switch_api_vlan_create(device, 10)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if3 = self.client.switch_api_interface_create(device, i_info3)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)
        self.client.switch_api_vlan_member_add(device, vlan, if3)

        rif_info4 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1)
        rif4 = self.client.switch_api_rif_create(0, rif_info4)

        i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif4, vrf,
                                                        i_ip4)

        # send the test packet(s)
        try:
            pkt1 = simple_tcp_packet(
                eth_dst='00:44:44:44:44:44',
                eth_src='00:11:11:11:11:11',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)
            print "sending packet on port %d " % swports[
                1], "to port %d " % swports[2], "and port %d" % swports[3]
            send_packet(self, swports[1], str(pkt1))
            verify_packets(self, pkt1, [swports[2], swports[3]])

            pkt2 = simple_tcp_packet(
                eth_dst='00:44:44:44:44:44',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)
            print "sending packet on port %d " % swports[
                2], "to port %d " % swports[1], "and port %d" % swports[3]
            send_packet(self, swports[2], str(pkt2))
            verify_packets(self, pkt2, [swports[1], swports[3]])

            pkt3 = simple_tcp_packet(
                eth_dst='00:44:44:44:44:44',
                eth_src='00:33:33:33:33:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)
            print "sending packet on port %d " % swports[
                3], "to port %d " % swports[1], "and port %d" % swports[2]
            send_packet(self, swports[3], str(pkt3))
            verify_packets(self, pkt3, [swports[1], swports[2]])

        finally:

            self.client.switch_api_mac_table_entry_flush(
                device, SWITCH_MAC_FLUSH_TYPE_ALL, 0x0, 0x0)

            self.client.switch_api_l3_interface_address_delete(device, rif4,
                                                               vrf, i_ip4)

            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)
            self.client.switch_api_vlan_member_remove(device, vlan, if3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_rif_delete(0, rif4)

            self.client.switch_api_vlan_delete(device, vlan)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('bfd')
@group('l2')
@group('l3')
@group('ipv4')
@group('clpm')
@group('maxsizes')
@group('ent')
class L3VIIPv4LagTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        vrf = self.client.switch_api_vrf_create(device, 2)

        vlan = self.client.switch_api_vlan_create(device, 10)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])
        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[4])
        port5 = self.client.switch_api_port_id_to_handle_get(device, swports[5])

        lag12 = self.client.switch_api_lag_create(device)
        self.client.switch_api_lag_member_add(
            device,
            lag_handle=lag12,
            side=SWITCH_API_DIRECTION_BOTH,
            port=port1)
        self.client.switch_api_lag_member_add(
            device,
            lag_handle=lag12,
            side=SWITCH_API_DIRECTION_BOTH,
            port=port2)
        i_info12 = switcht_interface_info_t(
            handle=lag12, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if12 = self.client.switch_api_interface_create(device, i_info12)

        lag34 = self.client.switch_api_lag_create(device)
        self.client.switch_api_lag_member_add(
            device,
            lag_handle=lag34,
            side=SWITCH_API_DIRECTION_BOTH,
            port=port3)
        self.client.switch_api_lag_member_add(
            device,
            lag_handle=lag34,
            side=SWITCH_API_DIRECTION_BOTH,
            port=port4)
        i_info34 = switcht_interface_info_t(
            handle=lag34, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if34 = self.client.switch_api_interface_create(device, i_info34)

        self.client.switch_api_vlan_member_add(device, vlan, if12)
        self.client.switch_api_vlan_member_add(device, vlan, if34)

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='100.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif1, vrf,
                                                        i_ip1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port5, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='200.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf,
                                                        i_ip2)

        i_ip11 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='11.11.11.1',
            prefix_length=32)
        nhop11, neighbor11 = switch_api_l3_nhop_neighbor_create(self, device, rif1, i_ip11, '00:11:11:11:11:11')
        self.client.switch_api_l3_route_add(device, vrf, i_ip11, nhop11)

        i_ip12 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='12.12.12.1',
            prefix_length=32)
        nhop12, neighbor12 = switch_api_l3_nhop_neighbor_create(self, device, rif1, i_ip12, '00:12:12:12:12:12')
        self.client.switch_api_l3_route_add(device, vrf, i_ip12, nhop12)

        switch_api_mac_table_entry_create(
            self, device, vlan, '00:11:11:11:11:11', 2, if12)
        switch_api_mac_table_entry_create(
            self, device, vlan, '00:12:12:12:12:12', 2, if34)

        try:
            print "Sending packet from port %d " % swports[
                5], "to VI lag (%d " % swports[1], "or %d)" % swports[2]
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='11.11.11.1',
                ip_src='192.168.0.1',
                ip_id=110,
                ip_ttl=64)

            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:77:66:55:44:33',
                ip_dst='11.11.11.1',
                ip_src='192.168.0.1',
                ip_id=110,
                ip_ttl=63)
            send_packet(self, swports[5], str(pkt))
            verify_any_packet_any_port(self, [exp_pkt],
                                       [swports[1], swports[2]], timeout=2)

            print "Sending packet from port %d " % swports[
                5], "to VI lag (%d " % swports[3], "or %d)" % swports[4]
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='12.12.12.1',
                ip_src='192.168.0.1',
                ip_id=110,
                ip_ttl=64)

            exp_pkt = simple_tcp_packet(
                eth_dst='00:12:12:12:12:12',
                eth_src='00:77:66:55:44:33',
                ip_dst='12.12.12.1',
                ip_src='192.168.0.1',
                ip_id=110,
                ip_ttl=63)
            send_packet(self, swports[5], str(pkt))
            verify_any_packet_any_port(self, [exp_pkt],
                                       [swports[3], swports[4]], timeout=2)
        finally:
            switch_api_mac_table_entry_delete(self, device, vlan,
                                                          '00:11:11:11:11:11')
            switch_api_mac_table_entry_delete(self, device, vlan,
                                                          '00:12:12:12:12:12')

            self.client.switch_api_neighbor_delete(device, neighbor11)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip11, nhop11)
            self.client.switch_api_nhop_delete(device, nhop11)

            self.client.switch_api_neighbor_delete(device, neighbor12)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip12, nhop12)
            self.client.switch_api_nhop_delete(device, nhop12)

            self.client.switch_api_vlan_member_remove(device, vlan, if12)
            self.client.switch_api_vlan_member_remove(device, vlan, if34)

            self.client.switch_api_l3_interface_address_delete(device, rif1,
                                                               vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(device, rif2,
                                                               vrf, i_ip2)

            self.client.switch_api_interface_delete(device, if12)
            self.client.switch_api_interface_delete(device, if34)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag12,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port1)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag12,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port2)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag34,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port3)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag34,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port4)

            self.client.switch_api_lag_delete(device, lag12)
            self.client.switch_api_lag_delete(device, lag34)

            self.client.switch_api_vlan_delete(device, vlan)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('bfd')
@group('l2')
@group('l3')
@group('ipv4')
@group('clpm')
@group('maxsizes')
@group('ent')
class L3VIIPv4HostVlanTaggingTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        vrf = self.client.switch_api_vrf_create(device, 2)

        vlan = self.client.switch_api_vlan_create(device, 10)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_TRUNK)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)

        switch_api_mac_table_entry_create(
            self, device, vlan, '00:11:11:11:11:11', 2, if1)
        switch_api_mac_table_entry_create(
            self, device, vlan, '00:22:22:22:22:22', 2, if2)

        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=1)
        rif3 = self.client.switch_api_rif_create(0, rif_info3)
        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='192.168.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif3, vrf,
                                                        i_ip3)

        rif_info4 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1)
        rif4 = self.client.switch_api_rif_create(0, rif_info4)
        i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif4, vrf,
                                                        i_ip4)

        # Add a static route
        i_ip41 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.1',
            prefix_length=32)
        nhop41, neighbor41 = switch_api_l3_nhop_neighbor_create(self, device, rif4, i_ip41, '00:11:11:11:11:11')
        self.client.switch_api_l3_route_add(device, vrf, i_ip41, nhop41)

        i_ip42 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='11.11.11.1',
            prefix_length=32)
        nhop42, neighbor42 = switch_api_l3_nhop_neighbor_create(self, device, rif4, i_ip42, '00:22:22:22:22:22')
        self.client.switch_api_l3_route_add(device, vrf, i_ip42, nhop42)

        i_ip31 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='12.12.12.1',
            prefix_length=32)
        nhop31, neighbor31 = switch_api_l3_nhop_neighbor_create(self, device, rif3, i_ip31, '00:33:33:33:33:33')
        self.client.switch_api_l3_route_add(device, vrf, i_ip31, nhop31)

        # send the test packet(s)
        try:
            print "Sending packet l3 port %d " % swports[
                3], "to vlan interface port (untagged) %d" % swports[1]
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:66:66:66:66:66',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=63)
            send_packet(self, swports[3], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

            print "Sending packet l3 port %d " % swports[
                3], "to vlan interface port (tagged) %d" % swports[2]
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:66:66:66:66:66',
                ip_dst='11.11.11.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:77:66:55:44:33',
                ip_dst='11.11.11.1',
                ip_src='192.168.0.1',
                ip_id=105,
                dl_vlan_enable=True,
                vlan_vid=10,
                pktlen=104,
                ip_ttl=63)
            send_packet(self, swports[3], str(pkt))
            verify_packets(self, exp_pkt, [swports[2]])

            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:77:77:77:77:77',
                ip_dst='12.12.12.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:33:33:33:33:33',
                eth_src='00:77:66:55:44:33',
                ip_dst='12.12.12.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=63)

            print "sending packet vlan interface port(untagged) %d" % swports[
                1], "to l3 port %d" % swports[3]
            send_packet(self, swports[1], str(pkt))
            verify_packets(self, exp_pkt, [swports[3]])

            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:77:77:77:77:77',
                dl_vlan_enable=True,
                vlan_vid=10,
                ip_dst='12.12.12.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:33:33:33:33:33',
                eth_src='00:77:66:55:44:33',
                ip_dst='12.12.12.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=63,
                pktlen=96)
            print "sending packet vlan interface port(tagged) %d" % swports[
                2], "to l3 port %d" % swports[3]
            send_packet(self, swports[2], str(pkt))
            verify_packets(self, exp_pkt, [swports[3]])

        finally:
            self.client.switch_api_mac_table_entry_flush(
                device, SWITCH_MAC_FLUSH_TYPE_ALL, 0x0, 0x0)
            self.client.switch_api_neighbor_delete(device, neighbor41)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip41, nhop41)
            self.client.switch_api_nhop_delete(device, nhop41)

            self.client.switch_api_neighbor_delete(device, neighbor42)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip42, nhop42)
            self.client.switch_api_nhop_delete(device, nhop42)

            self.client.switch_api_neighbor_delete(device, neighbor31)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip31, nhop31)
            self.client.switch_api_nhop_delete(device, nhop31)

            self.client.switch_api_l3_interface_address_delete(device, rif3,
                                                               vrf, i_ip3)
            self.client.switch_api_l3_interface_address_delete(device, rif4,
                                                               vrf, i_ip4)

            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_rif_delete(0, rif3)
            self.client.switch_api_rif_delete(0, rif4)

            self.client.switch_api_vlan_delete(device, vlan)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('bfd')
@group('l2')
@group('l3')
@group('ipv4')
@group('clpm')
@group('maxsizes')
@group('ent')
class L3VINhopGleanTest(pd_base_tests.ThriftInterfaceDataPlane,
                        api_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print

        devport = []
        for i in range(0,4):
            devport.append(swport_to_devport(self, swports[i]))
        print devport

        cpu_port = get_cpu_port(self)

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.client.switch_api_init(device)
        vrf = self.client.switch_api_vrf_create(device, 2)
        vlan_id = 10
        vlan = self.client.switch_api_vlan_create(device, vlan_id)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)

        switch_api_mac_table_entry_create(
            self, device, vlan, '00:22:22:22:22:22', 2, if2)

        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1)
        rif3 = self.client.switch_api_rif_create(0, rif_info3)
        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='192.168.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif3, vrf,
                                                        i_ip3)

        rif_info4 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=vlan_id,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1)
        rif4 = self.client.switch_api_rif_create(0, rif_info4)
        i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif4, vrf,
                                                        i_ip4)

        # Add a static route
        i_ip41 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.1',
            prefix_length=32)
        nhop41 = switch_api_nhop_create(self, device, rif4, i_ip41)
        self.client.switch_api_l3_route_add(device, vrf, i_ip41, nhop41)

        i_ip42 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='11.11.11.1',
            prefix_length=32)
        nhop42, neighbor42 = switch_api_l3_nhop_neighbor_create(self, device, rif4, i_ip42, '00:22:22:22:22:22')
        self.client.switch_api_l3_route_add(device, vrf, i_ip42, nhop42)

        # send the test packet(s)
        try:
            pkt1 = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:66:66:66:66:66',
                ip_dst='10.0.10.2',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)
            ingress_ifindex = self.client.switch_api_interface_ifindex_get(
                device, if3)

            exp_pkt1 = simple_cpu_packet(
                ingress_port=devport[3],
                ingress_ifindex=ingress_ifindex,
                reason_code=0x213,
                ingress_bd=2,
                inner_pkt=pkt1)
            exp_pkt1 = cpu_packet_mask_ingress_bd(exp_pkt1)
            print "Sending packet l3 port %d" % swports[
                3], "to cpu %d" % cpu_port
            send_packet(self, swports[3], str(pkt1))
            verify_packets(self, exp_pkt1, [cpu_port])

            pkt2 = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:66:66:66:66:66',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)
            exp_pkt2 = simple_cpu_packet(
                ingress_port=devport[3],
                ingress_ifindex=ingress_ifindex,
                reason_code=0x213,
                ingress_bd=2,
                inner_pkt=pkt2)
            exp_pkt2 = cpu_packet_mask_ingress_bd(exp_pkt2)

            print "Sending packet l3 port %d" % swports[
                3], "to cpu %d" % cpu_port
            send_packet(self, swports[3], str(pkt2))
            verify_packets(self, exp_pkt2, [cpu_port])

            switch_api_mac_table_entry_create(
                self, device, vlan, '00:11:11:11:11:11', 2, if1)

            print "Sending packet l3 port %d" % swports[
                3], "to cpu %d" % cpu_port
            send_packet(self, swports[3], str(pkt2))
            verify_packets(self, exp_pkt2, [cpu_port])

            neighbor41 = switch_api_neighbor_create(self, device, nhop41, '00:11:11:11:11:11')

            exp_pkt3 = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=63)

            print "Sending packet l3 port %d" % swports[
                3], "to vlan interface port %d" % swports[1]
            send_packet(self, swports[3], str(pkt2))
            verify_packets(self, exp_pkt3, [swports[1]])

            print "Sending packet l3 port %d" % swports[
                3], "to vlan interface port %d" % swports[2]
            pkt4 = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:66:66:66:66:66',
                ip_dst='11.11.11.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)
            exp_pkt4 = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:77:66:55:44:33',
                ip_dst='11.11.11.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=63)
            try:
                send_packet(self, swports[3], str(pkt4))
                verify_packets(self, exp_pkt4, [swports[2]])
            finally:
                self.client.switch_api_neighbor_delete(device, neighbor41)
                self.client.switch_api_l3_route_delete(device, vrf, i_ip41,
                                                       nhop41)
                self.client.switch_api_nhop_delete(device, nhop41)

        finally:
            self.client.switch_api_neighbor_delete(device, neighbor42)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip42, nhop42)
            self.client.switch_api_nhop_delete(device, nhop42)

            self.client.switch_api_l3_interface_address_delete(device, rif3,
                                                               vrf, i_ip3)
            self.client.switch_api_l3_interface_address_delete(device, rif4,
                                                               vrf, i_ip4)

            switch_api_mac_table_entry_delete(self, device, vlan,
                                                          '00:11:11:11:11:11')
            switch_api_mac_table_entry_delete(self, device, vlan,
                                                          '00:22:22:22:22:22')

            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_rif_delete(0, rif3)
            self.client.switch_api_rif_delete(0, rif4)

            self.client.switch_api_vlan_delete(device, vlan)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('l2')
@group('l3')
@group('error')
@group('maxsizes')
@group('ent')
class MalformedPacketsTest(ApiAdapter):
    def setUp(self):
        print
        print 'Configuring devices for malformed packet test cases'

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)


        self.vrf = self.client.switch_api_vrf_create(device, 2)
        self.rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, self.rmac,
                                              '00:77:66:55:44:33')

        self.port1 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[0])
        self.port2 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[1])

        # vlan 10, with two ports 0 and 1
        self.vlan = self.client.switch_api_vlan_create(device, 10)

        i_info1 = switcht_interface_info_t(
            handle=self.port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        self.if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=self.port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        self.if2 = self.client.switch_api_interface_create(device, i_info2)

        self.client.switch_api_vlan_member_add(device, self.vlan, self.if1)
        self.client.switch_api_vlan_member_add(device, self.vlan, self.if2)
        switch_api_mac_table_entry_create(
            self, device, self.vlan, '00:01:00:00:00:12', 2, self.if1)
        switch_api_mac_table_entry_create(
            self, device, self.vlan, '00:01:00:00:00:22', 2, self.if2)

    def runTest(self):
        init_drop_stats = self.client.switch_api_drop_stats_get(device)
        num_drops = 0

        print "Valid packet from port 0 to 1"
        pkt = simple_tcp_packet(
            eth_dst='00:01:00:00:00:22',
            eth_src='00:01:00:00:00:12',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ttl=64)
        send_packet(self, swports[0], str(pkt))
        verify_packets(self, pkt, [swports[1]])

        print "Same if check fail, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:01:00:00:00:12',
            eth_src='00:01:00:00:00:12',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ttl=64)
        send_packet(self, swports[0], str(pkt))
        num_drops += 1

        print "MAC DA zeros, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:00:00:00:00:00',
            eth_src='00:01:00:00:00:12',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ttl=64)
        send_packet(self, swports[0], str(pkt))
        num_drops += 1

        print "MAC SA zeros, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:01:00:00:00:11',
            eth_src='00:00:00:00:00:00',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ttl=64)
        send_packet(self, swports[0], str(pkt))
        num_drops += 1

        print "MAC SA broadcast, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:01:00:00:00:11',
            eth_src='ff:ff:ff:ff:ff:ff',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ttl=64)
        send_packet(self, swports[0], str(pkt))
        num_drops += 1

        print "MAC SA IP multicast, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:01:00:00:00:11',
            eth_src='01:00:5e:00:00:01',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ttl=64)
        send_packet(self, swports[0], str(pkt))
        num_drops += 1

        print "IPv4 IHL 0, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:01:00:00:00:22',
            eth_src='00:01:00:00:00:11',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ihl=0)
        send_packet(self, swports[0], str(pkt))
        num_drops += 1

        print "IPv4 IHL 1, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:01:00:00:00:22',
            eth_src='00:01:00:00:00:11',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ihl=1)
        send_packet(self, swports[0], str(pkt))
        num_drops += 1

        print "IPv4 IHL 2, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:01:00:00:00:22',
            eth_src='00:01:00:00:00:11',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ihl=2)
        send_packet(self, swports[0], str(pkt))
        num_drops += 1

        print "IPv4 IHL 3, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:01:00:00:00:22',
            eth_src='00:01:00:00:00:11',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ihl=3)
        send_packet(self, swports[0], str(pkt))
        num_drops += 1

        print "IPv4 IHL 4, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:01:00:00:00:22',
            eth_src='00:01:00:00:00:11',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ihl=4)
        send_packet(self, swports[0], str(pkt))
        num_drops += 1

        print "IPv4 TTL 0, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:01:00:00:00:22',
            eth_src='00:01:00:00:00:11',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ttl=0)
        send_packet(self, swports[0], str(pkt))
        num_drops += 1

        print "IPv4 invalid version, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:01:00:00:00:22',
            eth_src='00:01:00:00:00:11',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ttl=64)
        pkt[IP].version = 6
        send_packet(self, swports[0], str(pkt))
        num_drops += 1

        print "IPv4 dst is loopback, drop"
        pkt = simple_tcp_packet(
            eth_src='00:11:11:11:11:11',
            eth_dst='00:77:66:55:44:33',
            ip_dst='127.10.10.1',
            ip_src='10.10.10.1',
            ip_id=108,
            ip_ttl=64)
        # send_packet(self, swports[3], str(pkt))

        print "Port vlan mapping miss, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:01:00:00:00:22',
            eth_src='00:01:00:00:00:11',
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ttl=64)
        send_packet(self, swports[0], str(pkt))
        verify_no_other_packets(self, timeout=1)
        num_drops += 1

        print "Sleeping for 5 sec before fetching stats"
        time.sleep(5)
        try:
            final_drop_stats = self.client.switch_api_drop_stats_get(device)
            drop_stats = [a - b for a, b in zip(final_drop_stats, init_drop_stats)]

            final_count = 0
            print "Drop Stats: "
            for i in range(0, 256):
                if (drop_stats[i] != 0):
                    print "[%d:%d]" % (i, drop_stats[i])
                    print
                    final_count += drop_stats[i]
            print "Expected drop count: %d" % num_drops
            print "Final drop count   : %d" % final_count
            self.assertEqual(num_drops, final_count)
        finally:
            pass

    def tearDown(self):
        self.client.switch_api_mac_table_entry_flush(
            device, SWITCH_MAC_FLUSH_TYPE_ALL, 0x0, 0x0)

        self.client.switch_api_vlan_member_remove(device, self.vlan, self.if1)
        self.client.switch_api_vlan_member_remove(device, self.vlan, self.if2)
        self.client.switch_api_interface_delete(device, self.if1)
        self.client.switch_api_interface_delete(device, self.if2)

        self.client.switch_api_vlan_delete(device, self.vlan)
        self.client.switch_api_router_mac_delete(device, self.rmac,
                                                 '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(device, self.rmac)
        self.client.switch_api_vrf_delete(device, self.vrf)

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)

###############################################################################
@group('l2')
@group('l3')
@group('ipv6')
@group('error')
@group('maxsizes')
@group('tunnel')
@group('ent')
class MalformedPacketsTest_tunnel(ApiAdapter):
    def setUp(self):
        print
        print 'Configuring devices for malformed packet test cases'
        print 'that require tunnel and IPv6 feature'

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)


        self.vrf = self.client.switch_api_vrf_create(device, 2)
        self.rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, self.rmac,
                                              '00:77:66:55:44:33')

        self.port3 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[2])
        self.port4 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[3])

        # logical network with one native 2 and one tunnel interface 3
        i_info3 = switcht_interface_info_t(
            handle=self.port3, type=SWITCH_INTERFACE_TYPE_ACCESS)
        self.if3 = self.client.switch_api_interface_create(device, i_info3)

        rif_info4 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif4 = self.client.switch_api_rif_create(0, rif_info4)
        i_info4 = switcht_interface_info_t(
            handle=self.port4,
            type=SWITCH_INTERFACE_TYPE_PORT,
            rif_handle=self.rif4)
        self.if4 = self.client.switch_api_interface_create(device, i_info4)

        # Create a logical network (LN)
        ln_info = switcht_logical_network_t()
        self.ln1 = self.client.switch_api_logical_network_create(device,
                                                                 ln_info)

        self.imapper_h = self.create_tunnel_mapper(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN)
        self.imapper_entry_h = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE,
                              mapper=self.imapper_h,
                              handle=self.ln1,
                              vni=0x1234)

        self.underlay_lb_h = self.create_loopback_rif(device, self.vrf, self.rmac)
        self.overlay_lb_h = self.create_loopback_rif(device, self.vrf, self.rmac)

        self.tunnel_h = self.create_tunnel_table(
                              device=device,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
                              src_ip='1.1.1.3',
                              imapper_h=self.imapper_h,
                              emapper_h=0,
                              urif=self.underlay_lb_h,
                              orif=self.overlay_lb_h)

        self.tunnel_term_h = self.create_tunnel_term(
                              device=device,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
                              vrf=self.vrf,
                              tunnel=self.tunnel_h,
                              entry_type=SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P,
                              src_ip='1.1.1.1',
                              dst_ip='1.1.1.3')

        self.tunnel_if_h = self.create_tunnel_interface(
                              device=device,
                              tunnel=self.tunnel_h)

        # add the two interfaces
        self.client.switch_api_logical_network_member_add(device, self.ln1,
                                                          self.if3)

        switch_api_mac_table_entry_create(
            self, device, self.ln1, '00:11:11:11:11:11', 2, self.if3)

    def runTest(self):
        init_drop_stats = self.client.switch_api_drop_stats_get(device)
        num_drops = 0

        print "Valid Vxlan packet from Vxlan port2 to Access port1"
        pkt = simple_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ttl=64)
        vxlan_pkt = simple_vxlan_packet(
            eth_src='00:33:33:33:33:33',
            eth_dst='00:77:66:55:44:33',
            ip_id=0,
            ip_dst='1.1.1.3',
            ip_src='1.1.1.1',
            ip_ttl=64,
            ip_flags=0x2,
            udp_sport=11638,
            with_udp_chksum=False,
            vxlan_vni=0x1234,
            inner_frame=pkt)
        send_packet(self, swports[3], str(vxlan_pkt))
        time.sleep(2)
        verify_packets(self, pkt, [swports[2]])

        print "Inner MAC DA zeros, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:00:00:00:00:00',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ttl=64)
        vxlan_pkt = simple_vxlan_packet(
            eth_src='00:33:33:33:33:33',
            eth_dst='00:77:66:55:44:33',
            ip_id=0,
            ip_dst='1.1.1.3',
            ip_src='1.1.1.1',
            ip_ttl=64,
            ip_flags=0x2,
            udp_sport=11638,
            with_udp_chksum=False,
            vxlan_vni=0x1234,
            inner_frame=pkt)
        send_packet(self, swports[3], str(vxlan_pkt))
        num_drops += 1

        print "Inner MAC SA zeros, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:01:00:00:00:11',
            eth_src='00:00:00:00:00:00',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ttl=64)
        vxlan_pkt = simple_vxlan_packet(
            eth_src='00:33:33:33:33:33',
            eth_dst='00:77:66:55:44:33',
            ip_id=0,
            ip_dst='1.1.1.3',
            ip_src='1.1.1.1',
            ip_ttl=64,
            ip_flags=0x2,
            udp_sport=11638,
            with_udp_chksum=False,
            vxlan_vni=0x1234,
            inner_frame=pkt)
        send_packet(self, swports[3], str(vxlan_pkt))
        num_drops += 1

        print "Inner MAC SA broadcast, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:01:00:00:00:11',
            eth_src='ff:ff:ff:ff:ff:ff',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ttl=64)
        vxlan_pkt = simple_vxlan_packet(
            eth_src='00:33:33:33:33:33',
            eth_dst='00:77:66:55:44:33',
            ip_id=0,
            ip_dst='1.1.1.3',
            ip_src='1.1.1.1',
            ip_ttl=64,
            ip_flags=0x2,
            udp_sport=11638,
            with_udp_chksum=False,
            vxlan_vni=0x1234,
            inner_frame=pkt)
        send_packet(self, swports[3], str(vxlan_pkt))
        num_drops += 1

        print "Inner MAC SA IP multicast, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:01:00:00:00:11',
            eth_src='01:00:5e:00:00:05',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ttl=64)
        vxlan_pkt = simple_vxlan_packet(
            eth_src='00:33:33:33:33:33',
            eth_dst='00:77:66:55:44:33',
            ip_id=0,
            ip_dst='1.1.1.3',
            ip_src='1.1.1.1',
            ip_ttl=64,
            udp_sport=11638,
            with_udp_chksum=False,
            vxlan_vni=0x1234,
            inner_frame=pkt)
        send_packet(self, swports[3], str(vxlan_pkt))
        num_drops += 1

        print "Inner MAC SA IPv6 multicast, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:01:00:00:00:11',
            eth_src='33:33:00:00:00:05',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ttl=64)
        vxlan_pkt = simple_vxlan_packet(
            eth_src='00:33:33:33:33:33',
            eth_dst='00:77:66:55:44:33',
            ip_id=0,
            ip_dst='1.1.1.3',
            ip_src='1.1.1.1',
            ip_ttl=64,
            ip_flags=0x2,
            udp_sport=11638,
            with_udp_chksum=False,
            vxlan_vni=0x1234,
            inner_frame=pkt)
        send_packet(self, swports[3], str(vxlan_pkt))
        num_drops += 1

        print "Inner IHL 0, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:01:00:00:00:22',
            eth_src='00:01:00:00:00:11',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ihl=0)
        vxlan_pkt = simple_vxlan_packet(
            eth_src='00:33:33:33:33:33',
            eth_dst='00:77:66:55:44:33',
            ip_id=0,
            ip_dst='1.1.1.3',
            ip_src='1.1.1.1',
            ip_ttl=64,
            ip_flags=0x2,
            udp_sport=11638,
            with_udp_chksum=False,
            vxlan_vni=0x1234,
            inner_frame=pkt)
        send_packet(self, swports[3], str(vxlan_pkt))
        num_drops += 1

        print "Inner IHL 1, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:01:00:00:00:22',
            eth_src='00:01:00:00:00:11',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ihl=1)
        vxlan_pkt = simple_vxlan_packet(
            eth_src='00:33:33:33:33:33',
            eth_dst='00:77:66:55:44:33',
            ip_id=0,
            ip_dst='1.1.1.3',
            ip_src='1.1.1.1',
            ip_ttl=64,
            ip_flags=0x2,
            udp_sport=11638,
            with_udp_chksum=False,
            vxlan_vni=0x1234,
            inner_frame=pkt)
        send_packet(self, swports[3], str(vxlan_pkt))
        num_drops += 1

        print "Inner IHL 2, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:01:00:00:00:22',
            eth_src='00:01:00:00:00:11',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ihl=2)
        vxlan_pkt = simple_vxlan_packet(
            eth_src='00:33:33:33:33:33',
            eth_dst='00:77:66:55:44:33',
            ip_id=0,
            ip_dst='1.1.1.3',
            ip_src='1.1.1.1',
            ip_ttl=64,
            ip_flags=0x2,
            udp_sport=11638,
            with_udp_chksum=False,
            vxlan_vni=0x1234,
            inner_frame=pkt)
        send_packet(self, swports[3], str(vxlan_pkt))
        num_drops += 1

        print "Inner IHL 3, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:01:00:00:00:22',
            eth_src='00:01:00:00:00:11',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ihl=3)
        vxlan_pkt = simple_vxlan_packet(
            eth_src='00:33:33:33:33:33',
            eth_dst='00:77:66:55:44:33',
            ip_id=0,
            ip_dst='1.1.1.3',
            ip_src='1.1.1.1',
            ip_ttl=64,
            ip_flags=0x2,
            udp_sport=11638,
            with_udp_chksum=False,
            vxlan_vni=0x1234,
            inner_frame=pkt)
        send_packet(self, swports[3], str(vxlan_pkt))
        num_drops += 1

        print "Inner IHL 4, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:01:00:00:00:22',
            eth_src='00:01:00:00:00:11',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ihl=4)
        vxlan_pkt = simple_vxlan_packet(
            eth_src='00:33:33:33:33:33',
            eth_dst='00:77:66:55:44:33',
            ip_id=0,
            ip_dst='1.1.1.3',
            ip_src='1.1.1.1',
            ip_ttl=64,
            ip_flags=0x2,
            udp_sport=11638,
            with_udp_chksum=False,
            vxlan_vni=0x1234,
            inner_frame=pkt)
        send_packet(self, swports[3], str(vxlan_pkt))
        num_drops += 1

        print "Inner IPv4 TTL 0, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ttl=0)
        vxlan_pkt = simple_vxlan_packet(
            eth_src='00:33:33:33:33:33',
            eth_dst='00:77:66:55:44:33',
            ip_id=0,
            ip_dst='1.1.1.3',
            ip_src='1.1.1.1',
            ip_ttl=64,
            ip_flags=0x2,
            udp_sport=11638,
            with_udp_chksum=False,
            vxlan_vni=0x1234,
            inner_frame=pkt)
        send_packet(self, swports[3], str(vxlan_pkt))
        num_drops += 1

        print "Inner IPv6 TTL 0, drop"
        pkt = simple_tcpv6_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ipv6_hlim=0)
        vxlan_pkt = simple_vxlan_packet(
            eth_src='00:33:33:33:33:33',
            eth_dst='00:77:66:55:44:33',
            ip_id=0,
            ip_dst='1.1.1.3',
            ip_src='1.1.1.1',
            ip_ttl=64,
            ip_flags=0x2,
            udp_sport=11638,
            with_udp_chksum=False,
            vxlan_vni=0x1234,
            inner_frame=pkt)
        send_packet(self, swports[3], str(vxlan_pkt))
        num_drops += 1

        print "Inner IPv4 invalid version, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ttl=64)
        pkt[IP].version = 6
        vxlan_pkt = simple_vxlan_packet(
            eth_src='00:33:33:33:33:33',
            eth_dst='00:77:66:55:44:33',
            ip_id=0,
            ip_dst='1.1.1.3',
            ip_src='1.1.1.1',
            ip_ttl=64,
            ip_flags=0x2,
            udp_sport=11638,
            with_udp_chksum=False,
            vxlan_vni=0x1234,
            inner_frame=pkt)
        send_packet(self, swports[3], str(vxlan_pkt))
        num_drops += 1

        print "Inner IPv6 invalid version, drop (skipped for now)"
        pkt = simple_tcpv6_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ipv6_hlim=64)
        pkt[IPv6].version = 4
        vxlan_pkt = simple_vxlan_packet(
            eth_src='00:33:33:33:33:33',
            eth_dst='00:77:66:55:44:33',
            ip_id=0,
            ip_dst='1.1.1.3',
            ip_src='1.1.1.1',
            ip_ttl=64,
            ip_flags=0x2,
            udp_sport=11638,
            with_udp_chksum=False,
            vxlan_vni=0x1234,
            inner_frame=pkt)
        send_packet(self, swports[3], str(vxlan_pkt))
        num_drops += 1

        print "IPv4 src is loopback, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ttl=64)
        vxlan_pkt = simple_vxlan_packet(
            eth_src='00:33:33:33:33:33',
            eth_dst='00:77:66:55:44:33',
            ip_id=0,
            ip_dst='1.1.1.3',
            ip_src='127.1.1.1',
            ip_ttl=64,
            ip_flags=0x2,
            udp_sport=11638,
            with_udp_chksum=False,
            vxlan_vni=0x1234,
            inner_frame=pkt)
        send_packet(self, swports[3], str(vxlan_pkt))
        num_drops += 1

        print "IPv4 src is multicast, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ttl=64)
        vxlan_pkt = simple_vxlan_packet(
            eth_src='00:33:33:33:33:33',
            eth_dst='00:77:66:55:44:33',
            ip_id=0,
            ip_dst='1.1.1.3',
            ip_src='225.1.1.1',
            ip_ttl=64,
            ip_flags=0x2,
            udp_sport=11638,
            with_udp_chksum=False,
            vxlan_vni=0x1234,
            inner_frame=pkt)
        send_packet(self, swports[3], str(vxlan_pkt))
        num_drops += 1

        print "Inner IPv4 src is loopback, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.10.10.1',
            ip_src='127.10.10.1',
            ip_id=108,
            ip_ttl=64)
        vxlan_pkt = simple_vxlan_packet(
            eth_src='00:33:33:33:33:33',
            eth_dst='00:77:66:55:44:33',
            ip_id=0,
            ip_dst='1.1.1.3',
            ip_src='1.1.1.1',
            ip_ttl=64,
            ip_flags=0x2,
            udp_sport=11638,
            with_udp_chksum=False,
            vxlan_vni=0x1234,
            inner_frame=pkt)
        send_packet(self, swports[3], str(vxlan_pkt))
        num_drops += 1

        print "Inner IPv4 src is multicast, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.10.10.1',
            ip_src='226.10.10.1',
            ip_id=108,
            ip_ttl=64)
        vxlan_pkt = simple_vxlan_packet(
            eth_src='00:33:33:33:33:33',
            eth_dst='00:77:66:55:44:33',
            ip_id=0,
            ip_dst='1.1.1.3',
            ip_src='1.1.1.1',
            ip_ttl=64,
            ip_flags=0x2,
            udp_sport=11638,
            with_udp_chksum=False,
            vxlan_vni=0x1234,
            inner_frame=pkt)
        send_packet(self, swports[3], str(vxlan_pkt))
        num_drops += 1

        print "Inner IPv6 src multicast, drop"
        pkt = simple_tcpv6_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ipv6_src='ff02::1')
        vxlan_pkt = simple_vxlan_packet(
            eth_src='00:33:33:33:33:33',
            eth_dst='00:77:66:55:44:33',
            ip_id=0,
            ip_dst='1.1.1.3',
            ip_src='1.1.1.1',
            ip_ttl=64,
            ip_flags=0x2,
            udp_sport=11638,
            with_udp_chksum=False,
            vxlan_vni=0x1234,
            inner_frame=pkt)
        send_packet(self, swports[3], str(vxlan_pkt))
        num_drops += 1

        print "IPv4 dst is loopback, drop"
        pkt = simple_tcp_packet(
            eth_src='00:11:11:11:11:11',
            eth_dst='00:77:66:55:44:33',
            ip_dst='127.10.10.1',
            ip_src='10.10.10.1',
            ip_id=108,
            ip_ttl=64)
        # send_packet(self, swports[3], str(pkt))

        print "IPv6 dst is loopback, drop"
        pkt = simple_tcpv6_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:22:22:22:22:22',
            ipv6_dst='::1',
            ipv6_src='2000::1',
            ipv6_hlim=64)
        # send_packet(self, swports[3], str(pkt))

        print "Sleeping for 5 sec before fetching stats"
        time.sleep(15)
        try:
            final_drop_stats = self.client.switch_api_drop_stats_get(device)
            drop_stats = [a - b for a, b in zip(final_drop_stats, init_drop_stats)]

            final_count = 0
            print "Drop Stats: "
            for i in range(0, 256):
                if (drop_stats[i] != 0):
                    print "[%d:%d]" % (i, drop_stats[i])
                    print
                    final_count += drop_stats[i]
            print "Expected drop count: %d" % num_drops
            print "Final drop count   : %d" % final_count
            self.assertEqual(num_drops, final_count)
        finally:
            pass

    def tearDown(self):
        self.client.switch_api_mac_table_entry_flush(
            device, SWITCH_MAC_FLUSH_TYPE_ALL, 0x0, 0x0)
        switch_api_mac_table_entry_delete(self, device, self.ln1,
                                                      '00:11:11:11:11:11')
        self.cleanup()

        self.client.switch_api_logical_network_member_remove(device, self.ln1,
                                                             self.if3)
        self.client.switch_api_logical_network_delete(device, self.ln1)

        self.client.switch_api_interface_delete(device, self.if3)
        self.client.switch_api_interface_delete(device, self.if4)

        self.client.switch_api_rif_delete(0, self.rif4)

        self.client.switch_api_router_mac_delete(device, self.rmac,
                                                 '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(device, self.rmac)
        self.client.switch_api_vrf_delete(device, self.vrf)

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)

###############################################################################
@group('l2')
@group('l3')
@group('ipv6')
@group('error')
@group('maxsizes')
@group('ent')
class MalformedPacketsTest_ipv6(api_base_tests.ThriftInterfaceDataPlane):
    def setUp(self):
        print
        print 'Configuring devices for malformed packet test cases'

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)


        self.vrf = self.client.switch_api_vrf_create(device, 2)
        self.rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, self.rmac,
                                              '00:77:66:55:44:33')

        self.port1 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[0])
        self.port2 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[1])

        # vlan 10, with two ports 0 and 1
        self.vlan = self.client.switch_api_vlan_create(device, 10)

        i_info1 = switcht_interface_info_t(
            handle=self.port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        self.if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=self.port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        self.if2 = self.client.switch_api_interface_create(device, i_info2)

        self.client.switch_api_vlan_member_add(device, self.vlan, self.if1)
        self.client.switch_api_vlan_member_add(device, self.vlan, self.if2)
        switch_api_mac_table_entry_create(
            self, device, self.vlan, '00:01:00:00:00:12', 2, self.if1)
        switch_api_mac_table_entry_create(
            self, device, self.vlan, '00:01:00:00:00:22', 2, self.if2)

    def runTest(self):
        init_drop_stats = self.client.switch_api_drop_stats_get(device)
        num_drops = 0

        print "MAC SA IPv6 multicast, drop"
        pkt = simple_tcp_packet(
            eth_dst='00:01:00:00:00:11',
            eth_src='33:33:5e:00:00:01',
            ip_dst='10.10.10.1',
            ip_id=108,
            ip_ttl=64)
        send_packet(self, swports[0], str(pkt))
        verify_no_other_packets(self, timeout=1)
        num_drops += 1

        print "IPv6 TTL 0, drop"
        pkt = simple_tcpv6_packet(
            eth_dst='00:01:00:00:00:22',
            eth_src='00:01:00:00:00:11',
            ipv6_hlim=0)
        send_packet(self, swports[0], str(pkt))
        num_drops += 1

        print "IPv6 invalid version, drop"
        pkt = simple_tcpv6_packet(
            eth_dst='00:01:00:00:00:22', eth_src='00:01:00:00:00:11')
        pkt[IPv6].version = 4
        send_packet(self, swports[0], str(pkt))
        num_drops += 1

        print "IPv6 src multicast, drop"
        pkt = simple_tcpv6_packet(
            eth_dst='00:01:00:00:00:22',
            eth_src='00:01:00:00:00:11',
            ipv6_src='ff02::1')
        send_packet(self, swports[0], str(pkt))
        num_drops += 1

        print "IPv6 dst is loopback, drop"
        pkt = simple_tcpv6_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:22:22:22:22:22',
            ipv6_dst='::1',
            ipv6_src='2000::1',
            ipv6_hlim=64)
        # send_packet(self, swports[3], str(pkt))

        print "Sleeping for 5 sec before fetching stats"
        time.sleep(5)
        try:
            final_drop_stats = self.client.switch_api_drop_stats_get(device)
            drop_stats = [a - b for a, b in zip(final_drop_stats, init_drop_stats)]

            final_count = 0
            print "Drop Stats: "
            for i in range(0, 256):
                if (drop_stats[i] != 0):
                    print "[%d:%d]" % (i, drop_stats[i])
                    print
                    final_count += drop_stats[i]
            print "Expected drop count: %d" % num_drops
            print "Final drop count   : %d" % final_count
            self.assertEqual(num_drops, final_count)
        finally:
            pass

    def tearDown(self):
        self.client.switch_api_mac_table_entry_flush(
            device, SWITCH_MAC_FLUSH_TYPE_ALL, 0x0, 0x0)

        self.client.switch_api_vlan_member_remove(device, self.vlan, self.if1)
        self.client.switch_api_vlan_member_remove(device, self.vlan, self.if2)
        self.client.switch_api_interface_delete(device, self.if1)
        self.client.switch_api_interface_delete(device, self.if2)

        self.client.switch_api_vlan_delete(device, self.vlan)
        self.client.switch_api_router_mac_delete(device, self.rmac,
                                                 '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(device, self.rmac)
        self.client.switch_api_vrf_delete(device, self.vrf)

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('bfd')
@group('l2')
@group('l3')
@group('ipv4')
@group('maxsizes')
@group('ent')
class L3VIIPv4HostMacMoveTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print

        vrf = self.client.switch_api_vrf_create(device, 2)

        vlan = self.client.switch_api_vlan_create(device, 10)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)

        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif3 = self.client.switch_api_rif_create(0, rif_info3)
        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='192.168.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif3, vrf,
                                                        i_ip3)

        rif_info4 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1)
        rif4 = self.client.switch_api_rif_create(0, rif_info4)
        i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif4, vrf,
                                                        i_ip4)

        # Add a static route
        i_ip41 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.1',
            prefix_length=32)
        nhop41, neighbor41 = switch_api_l3_nhop_neighbor_create(self, device, rif4, i_ip41, '00:11:11:11:11:11')
        self.client.switch_api_l3_route_add(device, vrf, i_ip41, nhop41)

        i_ip31 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='12.12.12.1',
            prefix_length=32)
        nhop31, neighbor31 = switch_api_l3_nhop_neighbor_create(self, device, rif3, i_ip31, '00:33:33:33:33:33')
        self.client.switch_api_l3_route_add(device, vrf, i_ip31, nhop31)

        # send the test packet(s)
        try:
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:11:11:11:11:11',
                ip_dst='12.12.12.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:33:33:33:33:33',
                eth_src='00:77:66:55:44:33',
                ip_dst='12.12.12.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=63)
            print "Sending packet vlan interface  port %d" % swports[
                1], " to l3  port %d" % swports[3]
            send_packet(self, swports[1], str(pkt))
            verify_packets(self, exp_pkt, [swports[3]])

            # don't sleep too long, other wise mac aging is kicking in
            time.sleep(1)

            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:33:33:33:33:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=63)
            print "Sending packet l3 port %d" % swports[
                3], " to vlan interface port %d" % swports[1]
            send_packet(self, swports[3], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:11:11:11:11:11',
                ip_dst='12.12.12.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:33:33:33:33:33',
                eth_src='00:77:66:55:44:33',
                ip_dst='12.12.12.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=63)
            print "Sending packet vlan interface  port %d" % swports[
                2], " to l3  port %d" % swports[3]
            send_packet(self, swports[2], str(pkt))
            verify_packet(self, exp_pkt, swports[3])
            # don't sleep too long, other wise mac aging is kicking in
            time.sleep(1)

            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:33:33:33:33:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=63)

            print "Sending packet l3 port %d" % swports[
                3], " to vlan interface port %d" % swports[2]
            send_packet(self, swports[3], str(pkt))
            verify_packets(self, exp_pkt, [swports[2]])

        finally:
            self.client.switch_api_mac_table_entry_flush(
                device, SWITCH_MAC_FLUSH_TYPE_NETWORK, vlan, 0x0)

            self.client.switch_api_neighbor_delete(device, neighbor41)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip41, nhop41)
            self.client.switch_api_nhop_delete(device, nhop41)

            self.client.switch_api_neighbor_delete(device, neighbor31)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip31, nhop31)
            self.client.switch_api_nhop_delete(device, nhop31)

            self.client.switch_api_l3_interface_address_delete(device, rif3,
                                                               vrf, i_ip3)
            self.client.switch_api_l3_interface_address_delete(device, rif4,
                                                               vrf, i_ip4)

            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_rif_delete(0, rif3)
            self.client.switch_api_rif_delete(0, rif4)

            self.client.switch_api_vlan_delete(device, vlan)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################


@group('l3')
@group('error')
@group('maxsizes')
@group('ent')
class ExceptionPacketsTest(pd_base_tests.ThriftInterfaceDataPlane,
                           api_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def setUp(self):
        print
        print 'Configuring devices for exception packet test cases'

        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.devport = []
        self.devport.append(swport_to_devport(self, swports[0]))
        self.devport.append(swport_to_devport(self, swports[1]))

        self.cpu_port = get_cpu_port(self)

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.client.switch_api_init(device)

        self.vrf = self.client.switch_api_vrf_create(device, 2)
        self.rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, self.rmac,
                                              '00:77:66:55:44:33')

        self.port1 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[0])
        self.port2 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[1])

        # create two l3 interfaces
        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=False)
        self.rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=self.port1,
            type=SWITCH_INTERFACE_TYPE_PORT,
            rif_handle=self.rif1)
        self.if1 = self.client.switch_api_interface_create(device, i_info1)
        self.i_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='192.168.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, self.rif1,
                                                        self.vrf, self.i_ip1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=False)
        self.rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=self.port2,
            type=SWITCH_INTERFACE_TYPE_PORT,
            rif_handle=self.rif2)
        self.if2 = self.client.switch_api_interface_create(device, i_info2)
        self.i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, self.rif2,
                                                        self.vrf, self.i_ip2)

        # add ipv4 static routes
        self.i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.1',
            prefix_length=32)
        self.nhop1, self.neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, self.rif2, self.i_ip3, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(device, self.vrf, self.i_ip3,
                                            self.nhop1)

        self.i_ip31 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.20.10.1',
            prefix_length=32)
        self.nhop11, self.neighbor11 = switch_api_l3_nhop_neighbor_create(self, device, self.rif1, self.i_ip31, '00:11:22:33:44:66')
        self.client.switch_api_l3_route_add(device, self.vrf, self.i_ip31,
                                            self.nhop11)

    def runTest(self):
        print "Valid IPv4 packet from port 0 to 1"
        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:11:22:33:44:55',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_ttl=63)
        send_packet(self, swports[0], str(pkt))
        verify_packets(self, exp_pkt, [swports[1]])

        print "ipv4, routed, ttl = 1, redirect to cpu"
        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_ttl=1)

        ingress_ifindex = self.client.switch_api_interface_ifindex_get(device,
                                                                       self.if1)

        cpu_pkt = simple_cpu_packet(
            ingress_port=self.devport[0],
            ingress_ifindex=ingress_ifindex,
            reason_code=0x212,
            ingress_bd=0x01,
            inner_pkt=pkt)
        cpu_pkt = cpu_packet_mask_ingress_bd(cpu_pkt)
        send_packet(self, swports[0], str(pkt))
        verify_packets(self, cpu_pkt, [self.cpu_port])

        print "ipv4, routed, ingress bd == egress_bd, copy to cpu"
        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.20.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_ttl=63)
        cpu_pkt = simple_cpu_packet(
            ingress_port=self.devport[0],
            ingress_ifindex=ingress_ifindex,
            reason_code=0x215,
            ingress_bd=0x01,
            inner_pkt=pkt)
        cpu_pkt = cpu_packet_mask_ingress_bd(cpu_pkt)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:11:22:33:44:66',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.20.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_ttl=62)
        send_packet(self, swports[0], str(pkt))
        verify_each_packet_on_each_port(self, [cpu_pkt, exp_pkt],
                                        [self.cpu_port, swports[0]])
        verify_no_other_packets(self, timeout=1)

    def tearDown(self):
        self.client.switch_api_neighbor_delete(device, self.neighbor1)
        self.client.switch_api_neighbor_delete(device, self.neighbor11)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.i_ip3,
                                               self.nhop1)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.i_ip31,
                                               self.nhop11)
        self.client.switch_api_nhop_delete(device, self.nhop1)
        self.client.switch_api_nhop_delete(device, self.nhop11)
        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip1)
        self.client.switch_api_l3_interface_address_delete(device, self.rif2,
                                                           self.vrf, self.i_ip2)
        self.client.switch_api_interface_delete(device, self.if1)
        self.client.switch_api_interface_delete(device, self.if2)

        self.client.switch_api_rif_delete(0, self.rif1)
        self.client.switch_api_rif_delete(0, self.rif2)

        self.client.switch_api_router_mac_delete(device, self.rmac,
                                                 '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(device, self.rmac)
        self.client.switch_api_vrf_delete(device, self.vrf)

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)

###############################################################################

@group('l3')
@group('ipv6')
@group('error')
@group('maxsizes')
@group('ent')
class ExceptionPacketsTest_IPV6(pd_base_tests.ThriftInterfaceDataPlane,
                           api_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def setUp(self):
        print
        print 'Configuring devices for exception packet test cases only for IPv6'

        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.devport = []
        self.devport.append(swport_to_devport(self, swports[0]))
        self.devport.append(swport_to_devport(self, swports[1]))

        self.cpu_port = get_cpu_port(self)

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.client.switch_api_init(device)

        self.vrf = self.client.switch_api_vrf_create(device, 2)
        self.rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, self.rmac,
                                              '00:77:66:55:44:33')

        self.port1 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[0])
        self.port2 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[1])

        # create two l3 interfaces
        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=self.port1,
            type=SWITCH_INTERFACE_TYPE_PORT,
            rif_handle=self.rif1)
        self.if1 = self.client.switch_api_interface_create(device, i_info1)
        self.i_ip11 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6, ipaddr='2000::2', prefix_length=64)
        self.client.switch_api_l3_interface_address_add(device, self.rif1,
                                                        self.vrf, self.i_ip11)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=self.port2,
            type=SWITCH_INTERFACE_TYPE_PORT,
            rif_handle=self.rif2)
        self.if2 = self.client.switch_api_interface_create(device, i_info2)
        self.i_ip21 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='3000::2',
            prefix_length=120)
        self.client.switch_api_l3_interface_address_add(device, self.rif2,
                                                        self.vrf, self.i_ip21)
        # add an ipv6 static route
        self.i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='3000::3',
            prefix_length=128)
        self.nhop2, self.neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, self.rif2, self.i_ip4, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(device, self.vrf, self.i_ip4,
                                            self.nhop2)

        self.i_ip41 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='2000::3',
            prefix_length=128)
        self.nhop21, self.neighbor21 = switch_api_l3_nhop_neighbor_create(self, device, self.rif1, self.i_ip41, '00:11:22:33:44:77')
        self.client.switch_api_l3_route_add(device, self.vrf, self.i_ip41,
                                            self.nhop21)

    def runTest(self):

        print "Valid IPv6 packet from port 0 to 1"
        pkt = simple_tcpv6_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:22:22:22:22:22',
            ipv6_dst='3000::3',
            ipv6_src='2000::3',
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst='00:11:22:33:44:55',
            eth_src='00:77:66:55:44:33',
            ipv6_dst='3000::3',
            ipv6_src='2000::3',
            ipv6_hlim=63)
        send_packet(self, swports[0], str(pkt))
        verify_packets(self, exp_pkt, [swports[1]])

        ingress_ifindex = self.client.switch_api_interface_ifindex_get(device,
                                                                       self.if1)

        print "ipv6, routed, ttl = 1, redirect to cpu"
        pkt = simple_tcpv6_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:22:22:22:22:22',
            ipv6_dst='3000::3',
            ipv6_src='2000::3',
            ipv6_hlim=1)
        cpu_pkt = simple_cpu_packet(
            ingress_port=self.devport[0],
            ingress_ifindex=ingress_ifindex,
            reason_code=0x212,
            ingress_bd=0x01,
            inner_pkt=pkt)
        cpu_pkt = cpu_packet_mask_ingress_bd(cpu_pkt)
        send_packet(self, swports[0], str(pkt))
        verify_packets(self, cpu_pkt, [self.cpu_port])

        print "ipv6, routed, ingress bd == egress_bd, copy to cpu"
        pkt = simple_tcpv6_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:22:22:22:22:22',
            ipv6_dst='2000::3',
            ipv6_src='3000::3',
            ipv6_hlim=64)
        cpu_pkt = simple_cpu_packet(
            ingress_port=self.devport[0],
            ingress_ifindex=ingress_ifindex,
            reason_code=0x215,
            ingress_bd=0x01,
            inner_pkt=pkt)
        cpu_pkt = cpu_packet_mask_ingress_bd(cpu_pkt)
        exp_pkt = simple_tcpv6_packet(
            eth_dst='00:11:22:33:44:77',
            eth_src='00:77:66:55:44:33',
            ipv6_dst='2000::3',
            ipv6_src='3000::3',
            ipv6_hlim=63)
        send_packet(self, swports[0], str(pkt))
        verify_each_packet_on_each_port(self, [cpu_pkt, exp_pkt],
                                        [self.cpu_port, swports[0]])
        verify_no_other_packets(self, timeout=1)

        print "ipv6, routed, src is link-local, redirect to cpu"
        pkt = simple_tcpv6_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:22:22:22:22:22',
            ipv6_dst='3000::3',
            ipv6_src='fe80::1',
            ipv6_hlim=64)
        cpu_pkt = simple_cpu_packet(
            ingress_port=self.devport[0],
            ingress_ifindex=ingress_ifindex,
            reason_code=0x216,
            ingress_bd=0x01,
            inner_pkt=pkt)
        cpu_pkt = cpu_packet_mask_ingress_bd(cpu_pkt)
        send_packet(self, swports[0], str(pkt))
        verify_packets(self, cpu_pkt, [self.cpu_port])

    def tearDown(self):
        self.client.switch_api_neighbor_delete(device, self.neighbor2)
        self.client.switch_api_neighbor_delete(device, self.neighbor21)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.i_ip4,
                                               self.nhop2)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.i_ip41,
                                               self.nhop21)
        self.client.switch_api_nhop_delete(device, self.nhop2)
        self.client.switch_api_nhop_delete(device, self.nhop21)
        self.client.switch_api_l3_interface_address_delete(
            device, self.rif1, self.vrf, self.i_ip11)
        self.client.switch_api_l3_interface_address_delete(
            device, self.rif2, self.vrf, self.i_ip21)
        self.client.switch_api_interface_delete(device, self.if1)
        self.client.switch_api_interface_delete(device, self.if2)

        self.client.switch_api_rif_delete(0, self.rif1)
        self.client.switch_api_rif_delete(0, self.rif2)

        self.client.switch_api_router_mac_delete(device, self.rmac,
                                                 '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(device, self.rmac)
        self.client.switch_api_vrf_delete(device, self.vrf)

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)

###############################################################################
@group('l3')
@group('ipv4')
@group('clpm')
@group('maxsizes')
@group('2porttests')
@group('ent')
class L3IPv4MtuTest(pd_base_tests.ThriftInterfaceDataPlane,
                    api_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print
        print "Skipping L3IPv4MtuTest"
        return

        devport = []
        devport.append(swport_to_devport(self, swports[0]))
        devport.append(swport_to_devport(self, swports[1]))

        self.client.switch_api_init(device)
        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(device)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        iu1 = interface_union(port_lag_handle=swports[1])
        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            u=iu1,
            mac='00:77:66:55:44:33',
            label=0,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            device=0, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)
        i_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='192.168.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif1, vrf,
                                                        i_ip1)

        iu2 = interface_union(port_lag_handle=swports[2])
        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            u=iu2,
            mac='00:77:66:55:44:33',
            label=0,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            device=0, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf,
                                                        i_ip2)

        # Add a static route
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.1',
            prefix_length=32)
        nhop, neighbor = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip3, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(device, vrf, i_ip3, nhop)

        # send the test packet(s)
        try:
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64,
                pktlen=128)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=63,
                pktlen=128)
            send_packet(self, swports[1], str(pkt))
            verify_packets(self, exp_pkt, [swports[2]])

            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64,
                pktlen=523 + 14)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=63,
                pktlen=523 + 14)
            ingress_ifindex = self.client.switch_api_interface_ifindex_get(
                device, if1)

            cpu_pkt = simple_cpu_packet(
                ingress_port=devport[1],
                ingress_ifindex=ingress_ifindex,
                reason_code=0x1c,
                ingress_bd=0x02,
                inner_pkt=exp_pkt)
            cpu_pkt = cpu_packet_mask_ingress_bd(cpu_pkt)
            send_packet(self, swports[1], str(pkt))
            verify_packets(self, cpu_pkt, [cpu_port])
        finally:
            self.client.switch_api_neighbor_delete(device, neighbor)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip3, nhop)
            self.client.switch_api_nhop_delete(device, nhop)

            self.client.switch_api_l3_interface_address_delete(device, rif1,
                                                               vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(device, rif2,
                                                               vrf, i_ip2)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('l3')
@group('tunnel')
@group('ipv6')
@group('maxsizes')
@group('2porttests')
class IPinIPTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        self.vrf_id = 10
        self.underlay_vrf_id = 20
        self.fp_ports = [swports[0], swports[1]]
        self.port_h = [0] * len(self.fp_ports)
        self.rif_h = [0] * len(self.fp_ports)
        self.rif_ip = ['200.10.10.1', '192.168.0.2']
        self.host_mac = '00:11:22:33:44:55'
        self.port_h = [0] * len(self.fp_ports)
        self.intf_h = [0] * len(self.fp_ports)
        self.tunnel_type = [SWITCH_TUNNEL_TYPE_GRE, SWITCH_TUNNEL_TYPE_IPIP]
        self.tunnel_src_ip = ['10.100.1.1', '3ffe::1']
        self.tunnel_dst_ip = ['10.200.1.3', '3ffe::2']
        self.nhop_mac = ['00:11:11:11:11:11', '00:55:55:55:55:55']
        self.entry_type = SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P
        self.inner_v4_route_ip = ['10.10.10.1', '10.10.10.2', '10.10.10.3', '10.10.10.4']
        self.inner_v6_route_ip = ['2ffe::1', '2ffe::2', '2ffe::3', '2ffe::4']
        self.rif1_route_ip = ['10.20.10.1', '2000::1']
        self.tun_nhop_type = SWITCH_NHOP_TUNNEL_TYPE_VRF
        self.ipv4_type = [True, False]
        self.tunnel_ip_type = [SWITCH_TUNNEL_IP_ADDR_TYPE_IPV4, SWITCH_TUNNEL_IP_ADDR_TYPE_IPV6]

        self.vrf_h = self.add_vrf(device, self.vrf_id)
        self.underlay_vrf = self.add_vrf(device, self.underlay_vrf_id)
        self.vrf = [self.vrf_h, self.underlay_vrf]
        self.rmac = self.add_rmac(device, '00:77:66:55:44:33', rmac_type='all')
        self.client.switch_api_vrf_rmac_handle_set(device, self.vrf_h, self.rmac)

        for index in range(0, len(self.fp_ports)):
          self.port_h[index] = self.select_port(device, swports[index])
          self.rif_h[index] = self.create_l3_rif(
                                   device,
                                   self.vrf[index],
                                   self.rmac,
                                   self.port_h[index],
                                   self.rif_ip[index])

        self.underlay_lb_h = self.create_loopback_rif(device, self.underlay_vrf, self.rmac)
        self.overlay_lb_h = self.create_loopback_rif(device, self.vrf_h, self.rmac)

        self.tunnel_h = [0] * (len(self.tunnel_type) * len(self.tunnel_src_ip))
        self.tunnel_nhop_h = [0] * (len(self.tunnel_type) * len(self.tunnel_src_ip))
        index = 0
        for index1 in range(0, len(self.tunnel_type)):
          for index2 in range(0, len(self.tunnel_src_ip)):
            self.tunnel_h[index] = self.create_tunnel(
                              device=device,
                              underlay_vrf=self.underlay_vrf,
                              tunnel_type=self.tunnel_type[index1],
                              src_ip=self.tunnel_src_ip[index2],
                              dst_ip=self.tunnel_dst_ip[index2],
                              entry_type=self.entry_type,
                              urif=self.underlay_lb_h,
                              orif=self.overlay_lb_h,
                              tunnel_ip_type=self.tunnel_ip_type[index2],
                              v4=self.ipv4_type[index2])
            self.tunnel_nhop_h[index] = self.add_nhop_tunnel(
                              device,
                              self.tun_nhop_type,
                              self.vrf_h,
                              self.tunnel_h[index],
                              self.tunnel_src_ip[index2],
                              rw_type=SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L3,
                              v4=self.ipv4_type[index2])
            index += 1

        for index in range(0, len(self.inner_v4_route_ip)):
          self.add_static_route(device, self.vrf_h, self.inner_v4_route_ip[index],
                  self.tunnel_nhop_h[index], prefix_length=32)

        for index in range(0, len(self.inner_v6_route_ip)):
          self.add_static_route(device, self.vrf_h, self.inner_v6_route_ip[index],
                  self.tunnel_nhop_h[index], prefix_length=128, v4=False)

        for index in range(0, len(self.tunnel_src_ip)):
          nhop = self.add_l3_nhop(
                              device,
                              self.rif_h[1],
                              self.tunnel_src_ip[index],
                              self.nhop_mac[1],
                              v4=self.ipv4_type[index])
          self.add_static_route(device, self.underlay_vrf,
                  self.tunnel_src_ip[index], nhop, v4=self.ipv4_type[index], host=True)

        for index in range(0, len(self.rif1_route_ip)):
          nhop = self.add_l3_nhop(
                              device,
                              self.rif_h[0],
                              self.rif1_route_ip[index],
                              self.nhop_mac[0],
                              v4=self.ipv4_type[index])
          self.add_static_route(device, self.vrf_h,
                  self.rif1_route_ip[index], nhop, v4=self.ipv4_type[index], host=True)

    def runTest(self):
        try:
            print "Verifying GRE 4in4 (encap)"
            pkt1 = simple_tcp_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='10.20.10.1',
                ip_id=108,
                ip_ttl=64)
            pkt2 = simple_tcp_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:44:44:44:44:44',
                ip_dst='10.10.10.1',
                ip_src='10.20.10.1',
                ip_id=108,
                ip_ttl=63)
            ipip_pkt = simple_gre_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:55:55:55:55:55',
                ip_id=0,
                ip_src='10.200.1.3',
                ip_dst='10.100.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                inner_frame=pkt2['IP'])
            send_packet(self, swports[0], str(pkt1))
            verify_packets(self, ipip_pkt, [swports[1]])

            print "Verifying GRE 6in4 (encap)"
            pkt1 = simple_tcpv6_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:77:66:55:44:33',
                ipv6_dst='2ffe::1',
                ipv6_src='2000::1',
                ipv6_hlim=64)
            pkt2 = simple_tcpv6_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:44:44:44:44:44',
                ipv6_dst='2ffe::1',
                ipv6_src='2000::1',
                ipv6_hlim=63)
            ipip_pkt = simple_gre_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:55:55:55:55:55',
                ip_id=0,
                ip_src='10.200.1.3',
                ip_dst='10.100.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                inner_frame=pkt2['IPv6'])
            send_packet(self, swports[0], str(pkt1))
            verify_packets(self, ipip_pkt, [swports[1]])

            print "Verifying GRE 4in6 (encap)"
            pkt1 = simple_tcp_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:77:66:55:44:33',
                ip_dst='10.10.10.2',
                ip_src='10.20.10.1',
                ip_id=108,
                ip_ttl=64)
            pkt2 = simple_tcp_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:44:44:44:44:44',
                ip_dst='10.10.10.2',
                ip_src='10.20.10.1',
                ip_id=108,
                ip_ttl=63)
            ipip_pkt = simple_grev6_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:55:55:55:55:55',
                ipv6_src='3ffe::2',
                ipv6_dst='3ffe::1',
                ipv6_hlim=64,
                inner_frame=pkt2['IP'])
            send_packet(self, swports[0], str(pkt1))
            verify_packets(self, ipip_pkt, [swports[1]])

            print "Verifying GRE 6in6 (encap)"
            pkt1 = simple_tcpv6_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:77:66:55:44:33',
                ipv6_dst='2ffe::2',
                ipv6_src='2000::1',
                ipv6_hlim=64)
            pkt2 = simple_tcpv6_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:44:44:44:44:44',
                ipv6_dst='2ffe::2',
                ipv6_src='2000::1',
                ipv6_hlim=63)
            ipip_pkt = simple_grev6_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:55:55:55:55:55',
                ipv6_src='3ffe::2',
                ipv6_dst='3ffe::1',
                ipv6_hlim=64,
                inner_frame=pkt2['IPv6'])
            send_packet(self, swports[0], str(pkt1))
            verify_packets(self, ipip_pkt, [swports[1]])

            print "Verifying 4in4 (encap)"
            pkt1 = simple_tcp_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:77:66:55:44:33',
                ip_dst='10.10.10.3',
                ip_src='10.20.10.1',
                ip_id=108,
                ip_ttl=64)
            pkt2 = simple_tcp_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:44:44:44:44:44',
                ip_dst='10.10.10.3',
                ip_src='10.20.10.1',
                ip_id=108,
                ip_ttl=63)
            ipip_pkt = simple_ipv4ip_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:55:55:55:55:55',
                ip_id=0,
                ip_src='10.200.1.3',
                ip_dst='10.100.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                inner_frame=pkt2['IP'])
            send_packet(self, swports[0], str(pkt1))
            verify_packets(self, ipip_pkt, [swports[1]])

            print "Verifying 6in4 (encap)"
            pkt1 = simple_tcpv6_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:77:66:55:44:33',
                ipv6_dst='2ffe::3',
                ipv6_src='2000::1',
                ipv6_hlim=64)
            pkt2 = simple_tcpv6_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:44:44:44:44:44',
                ipv6_dst='2ffe::3',
                ipv6_src='2000::1',
                ipv6_hlim=63)
            ipip_pkt = simple_ipv4ip_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:55:55:55:55:55',
                ip_id=0,
                ip_src='10.200.1.3',
                ip_dst='10.100.1.1',
                ip_flags=0x2,
                ip_ttl=64,
                inner_frame=pkt2['IPv6'])
            send_packet(self, swports[0], str(pkt1))
            verify_packets(self, ipip_pkt, [swports[1]])

            print "Verifying 4in6 (encap)"
            pkt1 = simple_tcp_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:77:66:55:44:33',
                ip_dst='10.10.10.4',
                ip_src='10.20.10.1',
                ip_id=108,
                ip_ttl=64)
            pkt2 = simple_tcp_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:44:44:44:44:44',
                ip_dst='10.10.10.4',
                ip_src='10.20.10.1',
                ip_id=108,
                ip_ttl=63)
            ipip_pkt = simple_ipv6ip_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:55:55:55:55:55',
                ipv6_src='3ffe::2',
                ipv6_dst='3ffe::1',
                ipv6_hlim=64,
                inner_frame=pkt2['IP'])
            send_packet(self, swports[0], str(pkt1))
            verify_packets(self, ipip_pkt, [swports[1]])

            print "Verifying 6in6 (encap)"
            pkt1 = simple_tcpv6_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:77:66:55:44:33',
                ipv6_dst='2ffe::4',
                ipv6_src='2000::1',
                ipv6_hlim=64)
            pkt2 = simple_tcpv6_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:44:44:44:44:44',
                ipv6_dst='2ffe::4',
                ipv6_src='2000::1',
                ipv6_hlim=63)
            ipip_pkt = simple_ipv6ip_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:55:55:55:55:55',
                ipv6_src='3ffe::2',
                ipv6_dst='3ffe::1',
                ipv6_hlim=64,
                inner_frame=pkt2['IPv6'])
            send_packet(self, swports[0], str(pkt1))
            verify_packets(self, ipip_pkt, [swports[1]])

            print "Verifying GRE 4in4 (decap)"
            pkt1 = simple_tcp_packet(
                eth_src='00:44:44:44:44:44',
                eth_dst='00:77:66:55:44:33',
                ip_dst='10.20.10.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            ipip_pkt = simple_gre_packet(
                eth_src='00:55:55:55:55:55',
                eth_dst='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='10.200.1.3',
                ip_src='10.100.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                inner_frame=pkt1['IP'])
            pkt2 = simple_tcp_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:11:11:11:11:11',
                ip_dst='10.20.10.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63)
            send_packet(self, swports[1], str(ipip_pkt))
            verify_packets(self, pkt2, [swports[0]])

            print "Verifying GRE 6in4 (decap)"
            pkt1 = simple_tcpv6_packet(
                eth_src='00:44:44:44:44:44',
                eth_dst='00:77:66:55:44:33',
                ipv6_dst='2000::1',
                ipv6_src='2ffe::1',
                ipv6_hlim=64)
            ipip_pkt = simple_gre_packet(
                eth_src='00:55:55:55:55:55',
                eth_dst='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='10.200.1.3',
                ip_src='10.100.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                inner_frame=pkt1['IPv6'])
            pkt2 = simple_tcpv6_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:11:11:11:11:11',
                ipv6_dst='2000::1',
                ipv6_src='2ffe::1',
                ipv6_hlim=63)
            send_packet(self, swports[1], str(ipip_pkt))
            verify_packets(self, pkt2, [swports[0]])

            print "Verifying GRE 4in6 (decap)"
            pkt1 = simple_tcp_packet(
                eth_src='00:44:44:44:44:44',
                eth_dst='00:77:66:55:44:33',
                ip_dst='10.20.10.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            ipip_pkt = simple_grev6_packet(
                eth_src='00:55:55:55:55:55',
                eth_dst='00:77:66:55:44:33',
                ipv6_dst='3ffe::2',
                ipv6_src='3ffe::1',
                ipv6_hlim=64,
                inner_frame=pkt1['IP'])
            pkt2 = simple_tcp_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:11:11:11:11:11',
                ip_dst='10.20.10.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63)
            send_packet(self, swports[1], str(ipip_pkt))
            verify_packets(self, pkt2, [swports[0]])

            print "Verifying GRE 6in6 (decap)"
            pkt1 = simple_tcpv6_packet(
                eth_src='00:44:44:44:44:44',
                eth_dst='00:77:66:55:44:33',
                ipv6_dst='2000::1',
                ipv6_src='2ffe::1',
                ipv6_hlim=64)
            ipip_pkt = simple_grev6_packet(
                eth_src='00:55:55:55:55:55',
                eth_dst='00:77:66:55:44:33',
                ipv6_dst='3ffe::2',
                ipv6_src='3ffe::1',
                ipv6_hlim=64,
                inner_frame=pkt1['IPv6'])
            pkt2 = simple_tcpv6_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:11:11:11:11:11',
                ipv6_dst='2000::1',
                ipv6_src='2ffe::1',
                ipv6_hlim=63)
            send_packet(self, swports[1], str(ipip_pkt))
            verify_packets(self, pkt2, [swports[0]])

            print "Verifying 4in4 (decap)"
            pkt1 = simple_tcp_packet(
                eth_src='00:44:44:44:44:44',
                eth_dst='00:77:66:55:44:33',
                ip_dst='10.20.10.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            ipip_pkt = simple_ipv4ip_packet(
                eth_src='00:55:55:55:55:55',
                eth_dst='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='10.200.1.3',
                ip_src='10.100.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                inner_frame=pkt1['IP'])
            pkt2 = simple_tcp_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:11:11:11:11:11',
                ip_dst='10.20.10.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63)
            send_packet(self, swports[1], str(ipip_pkt))
            verify_packets(self, pkt2, [swports[0]])

            print "Verifying 6in4 (decap)"
            pkt1 = simple_tcpv6_packet(
                eth_src='00:44:44:44:44:44',
                eth_dst='00:77:66:55:44:33',
                ipv6_dst='2000::1',
                ipv6_src='2ffe::1',
                ipv6_hlim=64)
            ipip_pkt = simple_ipv4ip_packet(
                eth_src='00:55:55:55:55:55',
                eth_dst='00:77:66:55:44:33',
                ip_id=0,
                ip_dst='10.200.1.3',
                ip_src='10.100.1.1',
                ip_ttl=64,
                ip_flags=0x2,
                inner_frame=pkt1['IPv6'])
            pkt2 = simple_tcpv6_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:11:11:11:11:11',
                ipv6_dst='2000::1',
                ipv6_src='2ffe::1',
                ipv6_hlim=63)
            send_packet(self, swports[1], str(ipip_pkt))
            verify_packets(self, pkt2, [swports[0]])

            print "Verifying 4in6 (decap)"
            pkt1 = simple_tcp_packet(
                eth_src='00:44:44:44:44:44',
                eth_dst='00:77:66:55:44:33',
                ip_dst='10.20.10.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            ipip_pkt = simple_ipv6ip_packet(
                eth_src='00:55:55:55:55:55',
                eth_dst='00:77:66:55:44:33',
                ipv6_dst='3ffe::2',
                ipv6_src='3ffe::1',
                ipv6_hlim=64,
                inner_frame=pkt1['IP'])
            pkt2 = simple_tcp_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:11:11:11:11:11',
                ip_dst='10.20.10.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63)
            send_packet(self, swports[1], str(ipip_pkt))
            verify_packets(self, pkt2, [swports[0]])

            print "Verifying 6in6 (decap)"
            pkt1 = simple_tcpv6_packet(
                eth_src='00:44:44:44:44:44',
                eth_dst='00:77:66:55:44:33',
                ipv6_dst='2000::1',
                ipv6_src='2ffe::1',
                ipv6_hlim=64)
            ipip_pkt = simple_ipv6ip_packet(
                eth_src='00:55:55:55:55:55',
                eth_dst='00:77:66:55:44:33',
                ipv6_dst='3ffe::2',
                ipv6_src='3ffe::1',
                ipv6_hlim=64,
                inner_frame=pkt1['IPv6'])
            pkt2 = simple_tcpv6_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:11:11:11:11:11',
                ipv6_dst='2000::1',
                ipv6_src='2ffe::1',
                ipv6_hlim=63)
            send_packet(self, swports[1], str(ipip_pkt))
            verify_packets(self, pkt2, [swports[0]])

        finally:
            print "IPinIP test - 4 tunnels with IPIP and GRE"

    def tearDown(self):
      self.cleanup()
      api_base_tests.ThriftInterfaceDataPlane.tearDown(self)

###############################################################################
@group('bfd')
@group('l2')
@group('l3')
@group('cpu')
@group('maxsizes')
@group('ent')
class CpuTxTest(pd_base_tests.ThriftInterfaceDataPlane,
                api_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def setUp(self):
        print
        print 'Configuring devices for CPU Tx test cases'
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.devport = []
        for i in range(0,5):
            self.devport.append(swport_to_devport(self, swports[i]))
        self.cpu_port = get_cpu_port(self)

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.client.switch_api_init(device)

        self.vrf = self.client.switch_api_vrf_create(device, 2)
        self.rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, self.rmac,
                                              '00:77:66:55:44:33')

        self.port0 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[0])
        self.port1 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[1])
        self.port2 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[2])
        self.port3 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[3])
        self.port4 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[4])

        # create l3 interface
        rinfo = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1)
        self.rif = self.client.switch_api_rif_create(0, rinfo)
        info = switcht_interface_info_t(
            handle=self.port0,
            type=SWITCH_INTERFACE_TYPE_PORT,
            rif_handle=self.rif)
        self.if1 = self.client.switch_api_interface_create(device, info)
        self.ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.1.1',
            prefix_length=24)
        self.client.switch_api_l3_interface_address_add(device, self.rif,
                                                        self.vrf, self.ip1)

        # create l3 vlan interface
        self.vlan = self.client.switch_api_vlan_create(device, 10)
        self.bd = self.client.switch_api_vlan_bd_get(device, self.vlan)

        self.lag1 = self.client.switch_api_lag_create(device)
        self.client.switch_api_lag_member_add(
            device,
            lag_handle=self.lag1,
            side=SWITCH_API_DIRECTION_BOTH,
            port=self.port1)
        self.client.switch_api_lag_member_add(
            device,
            lag_handle=self.lag1,
            side=SWITCH_API_DIRECTION_BOTH,
            port=self.port2)
        info = switcht_interface_info_t(
            handle=self.lag1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        self.if2 = self.client.switch_api_interface_create(device, info)

        self.lag2 = self.client.switch_api_lag_create(device)
        self.client.switch_api_lag_member_add(
            device,
            lag_handle=self.lag2,
            side=SWITCH_API_DIRECTION_BOTH,
            port=self.port3)
        self.client.switch_api_lag_member_add(
            device,
            lag_handle=self.lag2,
            side=SWITCH_API_DIRECTION_BOTH,
            port=self.port4)
        info = switcht_interface_info_t(
            handle=self.lag2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        self.if3 = self.client.switch_api_interface_create(device, info)

        self.client.switch_api_vlan_member_add(device, self.vlan, self.if2)
        self.client.switch_api_vlan_member_add(device, self.vlan, self.if3)

        rinfo = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1)
        self.rif4 = self.client.switch_api_rif_create(0, rinfo)
        self.ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.2.1',
            prefix_length=24)
        self.client.switch_api_l3_interface_address_add(device, self.rif4,
                                                        self.vrf, self.ip2)

        switch_api_mac_table_entry_create(
            self, device, self.vlan, '00:11:11:11:11:11', 2, self.if2)
        switch_api_mac_table_entry_create(
            self, device, self.vlan, '00:22:22:22:22:22', 2, self.if3)

        # add ipv4 static routes
        self.ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.1',
            prefix_length=32)
        self.nhop1, self.neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, self.rif4, self.ip3, '00:11:11:11:11:11')
        self.client.switch_api_l3_route_add(device, self.vrf, self.ip3,
                                            self.nhop1)

    def runTest(self):
        pkt = simple_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:66:66:66:66:66',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_ttl=64)
        cpu_pkt = simple_cpu_packet(
            dst_device=0,
            dst_port_or_group=self.devport[2],
            ingress_ifindex=0,
            ingress_port=0,
            ingress_bd=0,
            tx_bypass=True,
            reason_code=0xFFFF,
            inner_pkt=pkt)
        print "Sending packet from cpu port %d" % cpu_port
        send_packet(self, self.cpu_port, str(cpu_pkt))
        verify_packets(self, pkt, [swports[2]])

        pkt = simple_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:66:66:66:66:66',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_ttl=64,
            pktlen=100)
        cpu_pkt = simple_cpu_packet(
            dst_device=0,
            dst_port_or_group=0,
            ingress_ifindex=0,
            ingress_port=0,
            ingress_bd=self.bd,
            tx_bypass=False,
            reason_code=SWITCH_BYPASS_SYSTEM_ACL,
            inner_pkt=pkt)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:66:66:66:66:66',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_ttl=64,
            pktlen=100)
        print "Sending packet from cpu port %d" % self.cpu_port
        send_packet(self, self.cpu_port, str(cpu_pkt))
        verify_packets_any(self, exp_pkt, [swports[1], swports[2]])

        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:66:66:66:66:66',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_ttl=64,
            pktlen=100)
        cpu_pkt = simple_cpu_packet(
            dst_device=0,
            dst_port_or_group=0,
            ingress_ifindex=0,
            ingress_port=0,
            ingress_bd=self.bd,
            tx_bypass=False,
            reason_code=SWITCH_BYPASS_SYSTEM_ACL,
            inner_pkt=pkt)
        exp_pkt = simple_tcp_packet(
            eth_src='00:77:66:55:44:33',
            eth_dst='00:11:11:11:11:11',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_ttl=63,
            pktlen=100)
        print "Sending packet from cpu port %d" % self.cpu_port
        send_packet(self, self.cpu_port, str(cpu_pkt))
        verify_packets_any(self, exp_pkt, [swports[1], swports[2]])

    def tearDown(self):
        switch_api_mac_table_entry_delete(self, device, self.vlan,
                                                      '00:11:11:11:11:11')
        switch_api_mac_table_entry_delete(self, device, self.vlan,
                                                      '00:22:22:22:22:22')
        self.client.switch_api_neighbor_delete(device, self.neighbor1)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.ip3,
                                               self.nhop1)
        self.client.switch_api_nhop_delete(device, self.nhop1)

        self.client.switch_api_l3_interface_address_delete(device, self.rif,
                                                           self.vrf, self.ip1)
        self.client.switch_api_l3_interface_address_delete(device, self.rif4,
                                                           self.vrf, self.ip2)

        self.client.switch_api_vlan_member_remove(device, self.vlan, self.if2)
        self.client.switch_api_vlan_member_remove(device, self.vlan, self.if3)

        self.client.switch_api_lag_member_delete(
            device,
            lag_handle=self.lag1,
            side=SWITCH_API_DIRECTION_BOTH,
            port=self.port1)
        self.client.switch_api_lag_member_delete(
            device,
            lag_handle=self.lag1,
            side=SWITCH_API_DIRECTION_BOTH,
            port=self.port2)
        self.client.switch_api_lag_member_delete(
            device,
            lag_handle=self.lag2,
            side=SWITCH_API_DIRECTION_BOTH,
            port=self.port3)
        self.client.switch_api_lag_member_delete(
            device,
            lag_handle=self.lag2,
            side=SWITCH_API_DIRECTION_BOTH,
            port=self.port4)

        self.client.switch_api_interface_delete(device, self.if1)
        self.client.switch_api_interface_delete(device, self.if2)
        self.client.switch_api_interface_delete(device, self.if3)

        self.client.switch_api_rif_delete(0, self.rif)
        self.client.switch_api_rif_delete(0, self.rif4)

        self.client.switch_api_lag_delete(device, self.lag1)
        self.client.switch_api_lag_delete(device, self.lag2)

        self.client.switch_api_vlan_delete(device, self.vlan)

        self.client.switch_api_router_mac_delete(device, self.rmac,
                                                 '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(device, self.rmac)

        self.client.switch_api_vrf_delete(device, self.vrf)

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('l2')
@group('l3')
@group('acl')
@group('ent')
class HostIfTest(pd_base_tests.ThriftInterfaceDataPlane,
                 api_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print
        # this test is not valid when runing against a remote host.
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.devport = []
        self.devport.append(swport_to_devport(self, swports[0]))
        self.devport.append(swport_to_devport(self, swports[1]))
        self.cpu_port = get_cpu_port(self)

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.client.switch_api_init(device)

        if self.thrift_server != 'localhost':
            return

        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
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

        cpu_port_handle = self.client.switch_api_port_id_to_handle_get(
                device, self.cpu_port)
        queue_handles = self.client.switch_api_queues_get(device, cpu_port_handle)
        hostif_group1 = switcht_hostif_group_t(queue_handles[0], policer_handle=0)
        hostif_group_id1 = self.client.switch_api_hostif_group_create(
            device, hostif_group1)

        hostif_name = "test_host_if"
        hostif = switcht_hostif_t(
            intf_name=hostif_name,
            handle=port1,
            operstatus=True,
            admin_state=True)
        hostif_flags = 0
        hostif_flags |= SWITCH_HOSTIF_ATTR_INTERFACE_NAME
        hostif_flags |= SWITCH_HOSTIF_ATTR_HANDLE
        hostif_flags |= SWITCH_HOSTIF_ATTR_OPER_STATUS
        hostif_flags |= SWITCH_HOSTIF_ATTR_ADMIN_STATE
        hostif_id = self.client.switch_api_hostif_create(device, hostif_flags, hostif)
        self.assertTrue(hostif_id != 0)
        rc_list = [
            SWITCH_HOSTIF_REASON_CODE_ARP_REQUEST,
            SWITCH_HOSTIF_REASON_CODE_ARP_RESPONSE,
            SWITCH_HOSTIF_REASON_CODE_OSPF,
            SWITCH_HOSTIF_REASON_CODE_IGMP,
            SWITCH_HOSTIF_REASON_CODE_PIM,
            SWITCH_HOSTIF_REASON_CODE_STP,
        ]
        hostif_table_entry_id = {}

        s = open_packet_socket(hostif_name)
        try:
            print 'Installing hostif reason codes Arp Request/Resp, OSPF, PIM, IGMP, STP and BGPv6'
            flags = 0
            flags |= SWITCH_HOSTIF_RCODE_ATTR_REASON_CODE
            flags |= SWITCH_HOSTIF_RCODE_ATTR_PACKET_ACTION
            flags |= SWITCH_HOSTIF_RCODE_ATTR_HOSTIF_GROUP
            arp_req_rcode_info = switcht_hostif_rcode_info_t(
                reason_code=SWITCH_HOSTIF_REASON_CODE_ARP_REQUEST,
                action=SWITCH_ACL_ACTION_COPY_TO_CPU,
                hostif_group_id=hostif_group_id1)
            rcode_handle1 = self.client.switch_api_hostif_reason_code_create(
                device, flags, arp_req_rcode_info)

            arp_resp_rcode_info = switcht_hostif_rcode_info_t(
                reason_code=SWITCH_HOSTIF_REASON_CODE_ARP_RESPONSE,
                action=SWITCH_ACL_ACTION_REDIRECT_TO_CPU,
                hostif_group_id=hostif_group_id1)
            rcode_handle2 = self.client.switch_api_hostif_reason_code_create(
                device, flags, arp_resp_rcode_info)

            ospf_rcode_info = switcht_hostif_rcode_info_t(
                reason_code=SWITCH_HOSTIF_REASON_CODE_OSPF,
                action=SWITCH_ACL_ACTION_COPY_TO_CPU,
                hostif_group_id=hostif_group_id1)
            rcode_handle3 = self.client.switch_api_hostif_reason_code_create(
                device, flags, ospf_rcode_info)

            igmp_v2_rcode_info = switcht_hostif_rcode_info_t(
                reason_code=SWITCH_HOSTIF_REASON_CODE_IGMP,
                action=SWITCH_ACL_ACTION_COPY_TO_CPU,
                hostif_group_id=hostif_group_id1)
            rcode_handle4 = self.client.switch_api_hostif_reason_code_create(
                device, flags, igmp_v2_rcode_info)

            pim_rcode_info = switcht_hostif_rcode_info_t(
                reason_code=SWITCH_HOSTIF_REASON_CODE_PIM,
                action=SWITCH_ACL_ACTION_COPY_TO_CPU,
                hostif_group_id=hostif_group_id1)
            rcode_handle5 = self.client.switch_api_hostif_reason_code_create(
                device, flags, pim_rcode_info)

            stp_rcode_info = switcht_hostif_rcode_info_t(
                reason_code=SWITCH_HOSTIF_REASON_CODE_STP,
                action=SWITCH_ACL_ACTION_REDIRECT_TO_CPU,
                hostif_group_id=hostif_group_id1)
            rcode_handle6 = self.client.switch_api_hostif_reason_code_create(
                device, flags, stp_rcode_info)

            # Broadcast ARP Request
            pkt = simple_arp_packet(arp_op=1, pktlen=100)
            ingress_ifindex = self.client.switch_api_interface_ifindex_get(
                device, if1)

            exp_pkt = simple_cpu_packet(
                ingress_port=self.devport[1],
                ingress_ifindex=ingress_ifindex,
                reason_code=SWITCH_HOSTIF_REASON_CODE_ARP_REQUEST,
                ingress_bd=2,
                inner_pkt=pkt)
            exp_arpq_bc_pkt = cpu_packet_mask_ingress_bd(exp_pkt)

            print 'Sending ARP request broadcast'
            send_packet(self, swports[1], str(pkt))
            verify_packet(self, exp_arpq_bc_pkt, self.cpu_port)
            self.assertTrue(socket_verify_packet(pkt, s))

            # Unicast ARP Request to Router MAC
            pkt = simple_arp_packet(
                arp_op=1, eth_dst='00:77:66:55:44:33', pktlen=100)
            exp_pkt = simple_cpu_packet(
                ingress_port=self.devport[1],
                ingress_ifindex=ingress_ifindex,
                reason_code=SWITCH_HOSTIF_REASON_CODE_ARP_REQUEST,
                ingress_bd=1,
                inner_pkt=pkt)
            exp_arpq_uc_pkt = cpu_packet_mask_ingress_bd(exp_pkt)

            print 'Sending unicast ARP request to router MAC'
            send_packet(self, swports[1], str(pkt))
            verify_packet(self, exp_arpq_uc_pkt, self.cpu_port)
            self.assertTrue(socket_verify_packet(pkt, s))

            # Unicast ARP Request to other MAC
            pkt = simple_arp_packet(
                arp_op=1, eth_dst='00:AA:BB:CC:DD:EE', pktlen=100)
            unexp_pkt = simple_cpu_packet(
                ingress_port=self.devport[1],
                ingress_ifindex=ingress_ifindex,
                reason_code=SWITCH_HOSTIF_REASON_CODE_ARP_REQUEST,
                ingress_bd=1,
                inner_pkt=pkt)
            unexp_arpq_uc_pkt = cpu_packet_mask_ingress_bd(exp_pkt)

            print 'Sending unicast ARP request to other MAC'
            send_packet(self, swports[1], str(pkt))
            verify_no_packet(self, unexp_arpq_uc_pkt, self.cpu_port)

            # Unicast ARP Response to Router MAC
            pkt = simple_arp_packet(
                arp_op=2, eth_dst='00:77:66:55:44:33', pktlen=100)
            exp_pkt = simple_cpu_packet(
                ingress_port=self.devport[1],
                ingress_ifindex=ingress_ifindex,
                reason_code=SWITCH_HOSTIF_REASON_CODE_ARP_RESPONSE,
                ingress_bd=1,
                inner_pkt=pkt)
            exp_arpr_pkt = cpu_packet_mask_ingress_bd(exp_pkt)

            print 'Sending unicast ARP response to router MAC'
            send_packet(self, swports[1], str(pkt))
            verify_packet(self, exp_arpr_pkt, self.cpu_port)
            self.assertTrue(socket_verify_packet(pkt, s))

            # Unicast ARP Response to other MAC
            pkt = simple_arp_packet(
                arp_op=2, eth_dst='00:11:22:33:44:55', pktlen=100)
            unexp_pkt = simple_cpu_packet(
                ingress_port=self.devport[1],
                ingress_ifindex=ingress_ifindex,
                reason_code=SWITCH_HOSTIF_REASON_CODE_ARP_RESPONSE,
                ingress_bd=1,
                inner_pkt=pkt)
            unexp_arpr_uc_pkt = cpu_packet_mask_ingress_bd(exp_pkt)

            print 'Sending unicast ARP response to other MAC'
            send_packet(self, swports[1], str(pkt))
            verify_no_packet(self, unexp_arpr_uc_pkt, self.cpu_port)

            # Broadcast ARP Response
            pkt = simple_arp_packet(arp_op=2, pktlen=100)
            ingress_ifindex = self.client.switch_api_interface_ifindex_get(
                device, if1)

            exp_pkt = simple_cpu_packet(
                ingress_port=self.devport[1],
                ingress_ifindex=ingress_ifindex,
                reason_code=SWITCH_HOSTIF_REASON_CODE_ARP_RESPONSE,
                ingress_bd=2,
                inner_pkt=pkt)
            exp_arpr_bc_pkt = cpu_packet_mask_ingress_bd(exp_pkt)

            print 'Sending ARP response broadcast'
            send_packet(self, swports[1], str(pkt))
            verify_packet(self, exp_arpr_bc_pkt, self.cpu_port)
            self.assertTrue(socket_verify_packet(pkt, s))

            # OSPF Hello
            pkt = simple_ip_packet(ip_proto=89, ip_dst='224.0.0.5')
            exp_pkt = simple_cpu_packet(
                ingress_port=self.devport[1],
                ingress_ifindex=ingress_ifindex,
                reason_code=SWITCH_HOSTIF_REASON_CODE_OSPF,
                ingress_bd=1,
                inner_pkt=pkt)
            exp_ospf1_pkt = cpu_packet_mask_ingress_bd(exp_pkt)

            print 'Sending OSPF packet destined to 224.0.0.5'
            send_packet(self, swports[1], str(pkt))
            verify_packet(self, exp_ospf1_pkt, self.cpu_port)
            self.assertTrue(socket_verify_packet(pkt, s))

            # OSPF Designated Routers
            pkt = simple_ip_packet(ip_proto=89, ip_dst='224.0.0.6')
            exp_pkt = simple_cpu_packet(
                ingress_port=self.devport[1],
                ingress_ifindex=ingress_ifindex,
                reason_code=SWITCH_HOSTIF_REASON_CODE_OSPF,
                ingress_bd=1,
                inner_pkt=pkt)
            exp_ospf2_pkt = cpu_packet_mask_ingress_bd(exp_pkt)

            print 'Sending OSPF packet destined to 224.0.0.6'
            send_packet(self, swports[1], str(pkt))
            verify_packet(self, exp_ospf2_pkt, self.cpu_port)
            self.assertTrue(socket_verify_packet(pkt, s))

            # IGMP
            pkt = simple_ip_packet(ip_proto=2)
            exp_pkt = simple_cpu_packet(
                ingress_port=self.devport[1],
                ingress_ifindex=ingress_ifindex,
                reason_code=SWITCH_HOSTIF_REASON_CODE_IGMP,
                ingress_bd=1,
                inner_pkt=pkt)
            exp_igmp_pkt = cpu_packet_mask_ingress_bd(exp_pkt)

            print 'Sending IGMP packet'
            send_packet(self, swports[1], str(pkt))
            verify_packet(self, exp_igmp_pkt, self.cpu_port)
            self.assertTrue(socket_verify_packet(pkt, s))

            # PIM
            pkt = simple_ip_packet(ip_proto=103)
            exp_pkt = simple_cpu_packet(
                ingress_port=self.devport[1],
                ingress_ifindex=ingress_ifindex,
                reason_code=SWITCH_HOSTIF_REASON_CODE_PIM,
                ingress_bd=1,
                inner_pkt=pkt)
            exp_pim_pkt = cpu_packet_mask_ingress_bd(exp_pkt)

            print 'Sending PIM packet'
            send_packet(self, swports[1], str(pkt))
            verify_packet(self, exp_pim_pkt, self.cpu_port)
            self.assertTrue(socket_verify_packet(pkt, s))

            # STP
            pkt = simple_eth_packet(eth_dst='01:80:C2:00:00:00', pktlen=100)
            exp_pkt = simple_cpu_packet(
                ingress_port=self.devport[1],
                ingress_ifindex=ingress_ifindex,
                reason_code=SWITCH_HOSTIF_REASON_CODE_STP,
                ingress_bd=1,
                inner_pkt=pkt)
            exp_stp_pkt = cpu_packet_mask_ingress_bd(exp_pkt)
            print 'Sending STP packet'
            send_packet(self, swports[1], str(pkt))
            verify_packet(self, exp_stp_pkt, self.cpu_port)
            self.assertTrue(socket_verify_packet(pkt, s))

            print 'Deleting hostif reason codes Arp Request/Resp, OSPF, PIM, IGMP, PTP and STP'
            self.client.switch_api_hostif_reason_code_delete(
                device, rcode_handle1)
            self.client.switch_api_hostif_reason_code_delete(
                device, rcode_handle2)
            self.client.switch_api_hostif_reason_code_delete(
                device, rcode_handle3)
            self.client.switch_api_hostif_reason_code_delete(
                device, rcode_handle4)
            self.client.switch_api_hostif_reason_code_delete(
                device, rcode_handle5)
            self.client.switch_api_hostif_reason_code_delete(
                device, rcode_handle6)

            print 'Sending ARP request broadcast'
            pkt = simple_arp_packet(arp_op=1, pktlen=100)
            send_packet(self, swports[1], str(pkt))
            verify_no_packet(self, exp_arpq_bc_pkt, self.cpu_port, timeout=1)

            print 'Sending OSPF packet destined to 224.0.0.5'
            pkt = simple_ip_packet(ip_proto=89, ip_dst='224.0.0.5')
            send_packet(self, swports[1], str(pkt))
            verify_no_packet(self, exp_ospf1_pkt, self.cpu_port, timeout=1)

            print 'Sending OSPF packet destined to 224.0.0.6'
            pkt = simple_ip_packet(ip_proto=89, ip_dst='224.0.0.6')
            send_packet(self, swports[1], str(pkt))
            verify_no_packet(self, exp_ospf2_pkt, self.cpu_port, timeout=1)

            print 'Sending IGMP v2 report'
            pkt = simple_ip_packet(ip_proto=2)
            send_packet(self, swports[1], str(pkt))
            verify_no_packet(self, exp_igmp_pkt, self.cpu_port, timeout=1)

            print 'Sending PIM packet'
            pkt = simple_ip_packet(ip_proto=103)
            send_packet(self, swports[1], str(pkt))
            verify_no_packet(self, exp_pim_pkt, self.cpu_port, timeout=1)

            print 'Sending STP packet'
            pkt = simple_eth_packet(eth_dst='01:80:C2:00:00:00', pktlen=100)
            send_packet(self, swports[1], str(pkt))
            verify_no_packet(self, exp_stp_pkt, self.cpu_port, timeout=1)

        finally:
            s.close()
            self.client.switch_api_hostif_delete(device, hostif_id)
            self.client.switch_api_hostif_group_delete(device, hostif_group_id1)

            self.client.switch_api_l3_interface_address_delete(device, rif1,
                                                               vrf, i_ip1)

            self.client.switch_api_interface_delete(device, if1)

            self.client.switch_api_rif_delete(0, rif1)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_vrf_delete(device, vrf)

###############################################################################
@group('l2')
@group('l3')
@group('acl')
@group('ent')
@group('ipv6')
class HostIfV6Test(pd_base_tests.ThriftInterfaceDataPlane,
                   api_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print
        # this test is not valid when runing against a remote host.
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.devport = []
        self.devport.append(swport_to_devport(self, swports[0]))
        self.devport.append(swport_to_devport(self, swports[1]))
        self.cpu_port = get_cpu_port(self)

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.client.switch_api_init(device)

        if self.thrift_server != 'localhost':
            return

        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='1234::2',
            prefix_length=120)
        self.client.switch_api_l3_interface_address_add(device, rif1, vrf,
                                                        i_ip2)

        cpu_port_handle = self.client.switch_api_port_id_to_handle_get(
                device, self.cpu_port)
        queue_handles = self.client.switch_api_queues_get(device, cpu_port_handle)
        hostif_group1 = switcht_hostif_group_t(queue_handles[0], policer_handle=0)
        hostif_group_id1 = self.client.switch_api_hostif_group_create(
            device, hostif_group1)

        hostif_name = "test_host_if"
        hostif = switcht_hostif_t(
            intf_name=hostif_name,
            handle=port1,
            operstatus=True,
            admin_state=True)
        hostif_flags = 0
        hostif_flags |= SWITCH_HOSTIF_ATTR_INTERFACE_NAME
        hostif_flags |= SWITCH_HOSTIF_ATTR_HANDLE
        hostif_flags |= SWITCH_HOSTIF_ATTR_OPER_STATUS
        hostif_flags |= SWITCH_HOSTIF_ATTR_ADMIN_STATE
        hostif_id = self.client.switch_api_hostif_create(device, hostif_flags, hostif)
        self.assertTrue(hostif_id != 0)
        rc_list = [
            SWITCH_HOSTIF_REASON_CODE_BGPV6
        ]
        hostif_table_entry_id = {}

        s = open_packet_socket(hostif_name)
        try:
            print 'Installing hostif reason codes for BGPv6'
            flags = 0
            flags |= SWITCH_HOSTIF_RCODE_ATTR_REASON_CODE
            flags |= SWITCH_HOSTIF_RCODE_ATTR_PACKET_ACTION
            flags |= SWITCH_HOSTIF_RCODE_ATTR_HOSTIF_GROUP

            bgpv6_rcode_info = switcht_hostif_rcode_info_t(
                reason_code=SWITCH_HOSTIF_REASON_CODE_BGPV6,
                action=SWITCH_ACL_ACTION_REDIRECT_TO_CPU,
                hostif_group_id=hostif_group_id1)
            rcode_handle7 = self.client.switch_api_hostif_reason_code_create(
                device, flags, bgpv6_rcode_info)

            ingress_ifindex = self.client.switch_api_interface_ifindex_get(
                device, if1)
            bgpv6_pkt = simple_tcpv6_packet(
                eth_dst='00:77:66:55:44:33',
                ipv6_dst='1234::2',
                ipv6_src='2000::1',
                tcp_dport=179,
                ipv6_hlim=64)
            exp_pkt = simple_cpu_packet(
                ingress_port=self.devport[1],
                ingress_ifindex=ingress_ifindex,
                reason_code=SWITCH_HOSTIF_REASON_CODE_BGPV6,
                ingress_bd=2,
                inner_pkt=bgpv6_pkt)
            exp_bgpv6_pkt = cpu_packet_mask_ingress_bd(exp_pkt)
            print 'Sending BGPv6 packet to router IP'
            send_packet(self, swports[1], str(bgpv6_pkt))
            verify_packet(self, exp_bgpv6_pkt, self.cpu_port)
            self.assertTrue(socket_verify_packet(bgpv6_pkt, s))

            print 'Deleting hostif reason code for BGPv6'
            self.client.switch_api_hostif_reason_code_delete(
                device, rcode_handle7)

            print 'Sending BGPv6 packet'
            send_packet(self, swports[1], str(bgpv6_pkt))
            verify_no_packet(self, exp_bgpv6_pkt, self.cpu_port, timeout=1)

            bgpv6_pkt = simple_tcpv6_packet(
                eth_dst='00:77:66:55:44:33',
                ipv6_dst='1234::3',
                ipv6_src='2000::1',
                tcp_dport=179,
                ipv6_hlim=64)
            exp_pkt = simple_cpu_packet(
                ingress_port=self.devport[1],
                ingress_ifindex=ingress_ifindex,
                reason_code=SWITCH_HOSTIF_REASON_CODE_GLEAN,
                ingress_bd=2,
                inner_pkt=bgpv6_pkt)
            exp_bgpv6_pkt = cpu_packet_mask_ingress_bd(exp_pkt)
            print 'Sending BGPv6 packet for glean'
            send_packet(self, swports[1], str(bgpv6_pkt))
            verify_packet(self, exp_bgpv6_pkt, self.cpu_port)
            self.assertTrue(socket_verify_packet(bgpv6_pkt, s))

        finally:
            s.close()
            self.client.switch_api_hostif_delete(device, hostif_id)
            self.client.switch_api_hostif_group_delete(device, hostif_group_id1)

            self.client.switch_api_l3_interface_address_delete(device, rif1,
                                                               vrf, i_ip2)

            self.client.switch_api_interface_delete(device, if1)

            self.client.switch_api_rif_delete(0, rif1)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_vrf_delete(device, vrf)

###############################################################################
###############################################################################
@group('bfd')
@group('l2')
@group('learn')
@group('maxsizes')
@group('ent')
class L2AgingTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        if ((test_param_get('target') == 'bmv2' and
             test_param_get('arch') == 'Tofino')):
            return
        vlan10 = self.client.switch_api_vlan_create(device, 10)
        vlan20 = self.client.switch_api_vlan_create(device, 20)
        self.client.switch_api_vlan_aging_interval_set(device, vlan10, 15000)
        self.client.switch_api_vlan_aging_interval_set(device, vlan20, 20000)
        self.client.switch_api_device_mac_aging_interval_set(device, 30000)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_TRUNK)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_TRUNK)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_TRUNK)
        if3 = self.client.switch_api_interface_create(device, i_info3)

        self.client.switch_api_vlan_member_add(device, vlan10, if1)
        self.client.switch_api_vlan_member_add(device, vlan10, if2)
        self.client.switch_api_vlan_member_add(device, vlan10, if3)
        self.client.switch_api_vlan_member_add(device, vlan20, if1)
        self.client.switch_api_vlan_member_add(device, vlan20, if2)
        self.client.switch_api_vlan_member_add(device, vlan20, if3)

        pkt1_vlan10 = simple_tcp_packet(
            eth_dst='00:66:66:66:66:66',
            eth_src='00:77:77:77:77:77',
            ip_dst='10.0.0.1',
            ip_id=115,
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_ttl=64)

        pkt2_vlan10 = simple_tcp_packet(
            eth_dst='00:77:77:77:77:77',
            eth_src='00:66:66:66:66:66',
            ip_dst='10.0.0.1',
            ip_id=115,
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_ttl=64)

        pkt1_vlan20 = simple_tcp_packet(
            eth_dst='00:66:66:66:66:68',
            eth_src='00:77:77:77:77:78',
            ip_dst='10.0.0.1',
            ip_id=115,
            dl_vlan_enable=True,
            vlan_vid=20,
            ip_ttl=64)

        pkt2_vlan20 = simple_tcp_packet(
            eth_dst='00:77:77:77:77:78',
            eth_src='00:66:66:66:66:68',
            ip_dst='10.0.0.1',
            ip_id=115,
            dl_vlan_enable=True,
            vlan_vid=20,
            ip_ttl=64)

        try:
            #vlan 10 is set an aging time of 15 seconds and vlan 20 inherits the global aging time for 30 seconds
            print "Sending packet vlan 10 port %d" % swports[1], " -> ports %d %d" % (swports[2], swports[3])
            send_packet(self, swports[1], str(pkt1_vlan10))
            verify_packets(self, pkt1_vlan10, [swports[2], swports[3]])
            print "Sending packet vlan 20 port %d" % swports[1], " -> ports %d %d" % (swports[2], swports[3])
            send_packet(self, swports[1], str(pkt1_vlan20))
            verify_packets(self, pkt1_vlan20, [swports[2], swports[3]])

            #allow it to learn
            time.sleep(2)

            print "Sending packet vlan 10 port %d" % swports[2], " -> port %d" % swports[1]
            send_packet(self, swports[2], str(pkt2_vlan10))
            verify_packets(self, pkt2_vlan10, [swports[1]])
            print "Sending packet vlan 20 port %d" % swports[2], " -> port %d" % swports[1]
            send_packet(self, swports[2], str(pkt2_vlan20))
            verify_packets(self, pkt2_vlan20, [swports[1]])

            time.sleep(30)

            # both vlans will flood now.
            print "Sending packet vlan 10 port %d" % swports[2], " -> ports %d %d" % (swports[1], swports[3])
            send_packet(self, swports[2], str(pkt2_vlan10))
            verify_packets(self, pkt2_vlan10, [swports[1], swports[3]])
            print "Sending packet vlan 20 port %d" % swports[2], " -> port %d %d" % (swports[1], swports[3])
            send_packet(self, swports[2], str(pkt2_vlan20))
            verify_packets(self, pkt2_vlan20, [swports[1], swports[3]])

            vlan10_age = self.client.switch_api_vlan_aging_interval_get(device, vlan10)
            vlan20_age = self.client.switch_api_vlan_aging_interval_get(device, vlan20)
            device_age = self.client.switch_api_device_mac_aging_interval_get(device)
            print "vlan 10 age", vlan10_age
            print "vlan 20 age", vlan20_age
            print "device age", device_age
            self.assertTrue(vlan10_age == 15000)
            self.assertTrue(vlan20_age == 20000)
            self.assertTrue(device_age == 30000)

        finally:
            self.client.switch_api_mac_table_entry_flush(
                device, SWITCH_MAC_FLUSH_TYPE_ALL, 0x0, 0x0)

            self.client.switch_api_vlan_member_remove(device, vlan10, if1)
            self.client.switch_api_vlan_member_remove(device, vlan10, if2)
            self.client.switch_api_vlan_member_remove(device, vlan10, if3)
            self.client.switch_api_vlan_member_remove(device, vlan20, if1)
            self.client.switch_api_vlan_member_remove(device, vlan20, if2)
            self.client.switch_api_vlan_member_remove(device, vlan20, if3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_vlan_delete(device, vlan10)
            self.client.switch_api_vlan_delete(device, vlan20)

###############################################################################
class DeviceInfoTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print

        print "Table info"
        tables = self.client.switch_api_table_all_get(device)
        print tables[1]
        size = self.client.switch_api_table_size_get(device, 1)
        print size
        table = self.client.switch_api_table_get(device, 1)
        print table

###############################################################################
class L3VINhopGleanBGPTest(pd_base_tests.ThriftInterfaceDataPlane,
                        api_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print

        devport = []
        for i in range(0,4):
            devport.append(swport_to_devport(self, swports[i]))
        print devport
        self.cpu_port = get_cpu_port(self)
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.client.switch_api_init(device)

        cpu_port_handle = self.client.switch_api_port_id_to_handle_get(device, self.cpu_port)
        queue_handles = self.client.switch_api_queues_get(device, cpu_port_handle)
        hostif_group1 = switcht_hostif_group_t(queue_handles[0], policer_handle=0)
        hostif_group_id1 = self.client.switch_api_hostif_group_create(
            device, hostif_group1)

        flags = 0
        flags |= SWITCH_HOSTIF_RCODE_ATTR_REASON_CODE
        flags |= SWITCH_HOSTIF_RCODE_ATTR_PACKET_ACTION
        flags |= SWITCH_HOSTIF_RCODE_ATTR_HOSTIF_GROUP
        bgp_rcode_info = switcht_hostif_rcode_info_t(
            reason_code=SWITCH_HOSTIF_REASON_CODE_BGP,
            action=SWITCH_ACL_ACTION_REDIRECT_TO_CPU,
            hostif_group_id=hostif_group_id1,
            priority=50)
        rcode_handle1 = self.client.switch_api_hostif_reason_code_create(
            device, flags, bgp_rcode_info)

        vrf = self.client.switch_api_vrf_create(device, 2)
        vlan_id = 10
        vlan = self.client.switch_api_vlan_create(device, vlan_id)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)

        switch_api_mac_table_entry_create(
            self, device, vlan, '00:22:22:22:22:22', 2, if2)

        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1)
        rif3 = self.client.switch_api_rif_create(0, rif_info3)
        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='192.168.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif3, vrf,
                                                        i_ip3)

        rif_info4 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=vlan_id,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1)
        rif4 = self.client.switch_api_rif_create(0, rif_info4)
        i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif4, vrf,
                                                        i_ip4)

        # Add a static route
        i_ip41 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.1',
            prefix_length=32)
        nhop41 = switch_api_nhop_create(self, device, rif4, i_ip41)
        self.client.switch_api_l3_route_add(device, vrf, i_ip41, nhop41)

        # send the test packet(s)
        try:
            pkt1 = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:66:66:66:66:66',
                ip_dst='10.0.0.2',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64,
                tcp_dport=179)
            ingress_ifindex = self.client.switch_api_interface_ifindex_get(
                device, if3)

            exp_pkt1 = simple_cpu_packet(
                ingress_port=devport[3],
                ingress_ifindex=ingress_ifindex,
                reason_code=0x401,
                ingress_bd=2,
                inner_pkt=pkt1)
            exp_pkt1 = cpu_packet_mask_ingress_bd(exp_pkt1)
            print "Sending packet l3 port %d" % swports[
                3], "to cpu %d" % self.cpu_port
            send_packet(self, swports[3], str(pkt1))
            verify_packets(self, exp_pkt1, [self.cpu_port])

        finally:
            self.client.switch_api_l3_route_delete(device, vrf, i_ip41, nhop41)
            self.client.switch_api_nhop_delete(device, nhop41)

            self.client.switch_api_l3_interface_address_delete(device, rif3,
                                                               vrf, i_ip3)
            self.client.switch_api_l3_interface_address_delete(device, rif4,
                                                               vrf, i_ip4)

            switch_api_mac_table_entry_delete(self, device, vlan,
                                                          '00:22:22:22:22:22')

            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_rif_delete(0, rif3)
            self.client.switch_api_rif_delete(0, rif4)

            self.client.switch_api_vlan_delete(device, vlan)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)
            self.client.switch_api_hostif_group_delete(device, hostif_group_id1)
            self.client.switch_api_hostif_reason_code_delete(device, rcode_handle1)

class IPNeighborTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Testing neighbor/nhop creation and deletion"
        vrf = self.client.switch_api_vrf_create(device, 2)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[4])

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(0, rmac, '00:77:66:55:44:33')

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info)
        rif2 = self.client.switch_api_rif_create(0, rif_info)
        rif4 = self.client.switch_api_rif_create(0, rif_info)

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(0, i_info1)
        i_ip1 = switcht_ip_addr_t(ipaddr='192.168.0.2', prefix_length=16)
        self.client.switch_api_l3_interface_address_add(0, rif1, vrf, i_ip1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(0, i_info2)
        i_ip2 = switcht_ip_addr_t(ipaddr='10.0.0.2', prefix_length=16)
        self.client.switch_api_l3_interface_address_add(0, rif2, vrf, i_ip2)

        i_info4 = switcht_interface_info_t(
            handle=port4, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif4)
        if4 = self.client.switch_api_interface_create(0, i_info4)
        i_ip4 = switcht_ip_addr_t(ipaddr='11.0.0.2', prefix_length=16)
        self.client.switch_api_l3_interface_address_add(0, rif4, vrf, i_ip4)

        # Add a static route
        i_ip3 = switcht_ip_addr_t(ipaddr='10.10.10.1', prefix_length=32)
        nhop, neighbor = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip3, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(0, vrf, i_ip3, nhop)

        i_ip41 = switcht_ip_addr_t(ipaddr='11.10.10.4', prefix_length=32)
        nhop4, neighbor4 = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip41, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(0, vrf, i_ip41, nhop4)

        i_ip42 = switcht_ip_addr_t(ipaddr='11.10.12.4', prefix_length=32)
        nhop2, neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip42, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(0, vrf, i_ip42, nhop2)

        #cleanup
        self.client.switch_api_l3_route_delete(0, vrf, i_ip41, nhop4)
        self.client.switch_api_l3_route_delete(0, vrf, i_ip42, nhop2)
        self.client.switch_api_l3_route_delete(0, vrf, i_ip3, nhop)
        self.client.switch_api_neighbor_delete(0, neighbor2)
        self.client.switch_api_neighbor_delete(0, neighbor4)
        self.client.switch_api_neighbor_delete(0, neighbor)
        self.client.switch_api_nhop_delete(0, nhop2)
        self.client.switch_api_nhop_delete(0, nhop4)
        self.client.switch_api_nhop_delete(0, nhop)

        self.client.switch_api_l3_interface_address_delete(0, rif1, vrf, i_ip1)
        self.client.switch_api_l3_interface_address_delete(0, rif2, vrf, i_ip2)
        self.client.switch_api_l3_interface_address_delete(0, rif4, vrf, i_ip4)

        self.client.switch_api_interface_delete(0, if1)
        self.client.switch_api_interface_delete(0, if2)
        self.client.switch_api_interface_delete(0, if4)

        self.client.switch_api_rif_delete(0, rif1)
        self.client.switch_api_rif_delete(0, rif2)
        self.client.switch_api_rif_delete(0, rif4)

        self.client.switch_api_router_mac_delete(0, rmac, '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(0, rmac)
        self.client.switch_api_vrf_delete(0, vrf)

@group('l3')
@group('ipv4')
@group('clpm')
@group('maxsizes')
@group('ent')
class L3IPv4EcmpSeedTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        if ((test_param_get('target') == 'bmv2') or
            (test_param_get('target') == 'bmv2' and
             test_param_get('arch') == 'Tofino')):
            return
        print "Sending packet port %d" % swports[1], " -> port %d" % swports[
            2], " (192.168.0.1 -> 10.0.0.1 [id = 101])"
        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])
        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[4])
        port5 = self.client.switch_api_port_id_to_handle_get(device, swports[5])

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

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v4_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf,
                                                        i_ip2)
        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v4_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif3 = self.client.switch_api_rif_create(0, rif_info3)
        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='11.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif3, vrf,
                                                        i_ip3)

        rif_info5 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v4_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif5 = self.client.switch_api_rif_create(0, rif_info5)
        i_info5 = switcht_interface_info_t(
            handle=port4, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif5)
        if5 = self.client.switch_api_interface_create(device, i_info5)
        i_ip5 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='200.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif5, vrf,
                                                        i_ip5)

        rif_info6 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v4_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif6 = self.client.switch_api_rif_create(0, rif_info6)
        i_info6 = switcht_interface_info_t(
            handle=port5, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif6)
        if6 = self.client.switch_api_interface_create(device, i_info6)
        i_ip6 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='200.1.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif6, vrf,
                                                        i_ip6)

        i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.1',
            prefix_length=32)
        nhop1, neighbor1= switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip4, '00:11:22:33:44:55')
        nhop2, neighbor2= switch_api_l3_nhop_neighbor_create(self, device, rif3, i_ip4, '00:11:22:33:44:56')
        nhop3, neighbor3= switch_api_l3_nhop_neighbor_create(self, device, rif5, i_ip4, '00:11:22:33:44:57')
        nhop4, neighbor4= switch_api_l3_nhop_neighbor_create(self, device, rif6, i_ip4, '00:11:22:33:44:58')

        ecmp = self.client.switch_api_ecmp_create(device)
        self.client.switch_api_ecmp_member_add(device, ecmp, 4, [nhop1, nhop2, nhop3, nhop4])

        self.client.switch_api_l3_route_add(device, vrf, i_ip4, ecmp)
        default_ecmp_seed = self.client.switch_api_ecmp_hash_seed_get(device)

        try:
            port_set = set()
            print "Changing ecmp hash seed"
            seed = 137
            max_itrs = 15
            for i in range(0, max_itrs):
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst='10.10.10.1',
                    ip_src='192.168.0.1',
                    ip_id=106,
                    ip_ttl=64)
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
                exp_pkt3 = simple_tcp_packet(
                    eth_dst='00:11:22:33:44:57',
                    eth_src='00:77:66:55:44:33',
                    ip_dst='10.10.10.1',
                    ip_src='192.168.0.1',
                    ip_id=106,
                    ip_ttl=63)
                exp_pkt4 = simple_tcp_packet(
                    eth_dst='00:11:22:33:44:58',
                    eth_src='00:77:66:55:44:33',
                    ip_dst='10.10.10.1',
                    ip_src='192.168.0.1',
                    ip_id=106,
                    ip_ttl=63)
                send_packet(self, swports[1], str(pkt))
                rcv_port = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2, exp_pkt3, exp_pkt4],
                                           [swports[2], swports[3], swports[4], swports[5]], timeout=2)
                port_set.add(rcv_port)
                seed=seed+(i+1)*17
                self.client.switch_api_ecmp_hash_seed_set(device,seed)
            self.assertTrue(len(port_set)!=1)

        finally:
            self.client.switch_api_ecmp_hash_seed_set(device,default_ecmp_seed)

            self.client.switch_api_l3_route_delete(device, vrf, i_ip4, ecmp)

            self.client.switch_api_ecmp_member_delete(device, ecmp, 4,
                                                      [nhop1, nhop2, nhop3, nhop4])
            self.client.switch_api_ecmp_delete(device, ecmp)

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
            self.client.switch_api_l3_interface_address_delete(device, rif5,
                                                               vrf, i_ip5)
            self.client.switch_api_l3_interface_address_delete(device, rif6,
                                                               vrf, i_ip6)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)
            self.client.switch_api_interface_delete(device, if5)
            self.client.switch_api_interface_delete(device, if6)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)
            self.client.switch_api_rif_delete(0, rif3)
            self.client.switch_api_rif_delete(0, rif5)
            self.client.switch_api_rif_delete(0, rif6)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)

