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
INT L45 transit tests, optionally with Digest
"""

import switchapi_thrift

import time
import sys
import logging

import unittest
import random

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *

import pd_base_tests

import os

from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *
import pdb

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../../base'))
from common.utils import *
sys.path.append(os.path.join(this_dir, '../../../base/api-tests'))
import api_base_tests

sys.path.append(os.path.join(this_dir, '../..'))
from dtel_utils import *

device = 0

quantization_shift = 0

swports = [0, 1, 2]

SID = 0x11111111
params = SwitchConfig_Params()
params.switch_id = SID
params.mac_self = '00:77:66:55:44:33'
params.nports = 3
params.ipaddr_inf = ['2.2.2.1',  '1.1.1.2', '10.1.0.4']
params.ipaddr_nbr = ['2.2.2.2', '1.1.1.1', '10.1.0.1']
params.mac_nbr = ['00:11:22:33:44:54', '00:11:22:33:44:55', '00:11:22:33:44:56']
params.report_ports = None
params.device = device
params.swports = swports

@group('transit_l45')
@group('ep_l45')
class intl45_route_dtel_reports(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 transit device - "
        print "Correctly routing DTel reports"
        prepare_int_l45_bindings()

        # Enable INT transit or endpoint processing
        # Depending on the image, only one of these should work
        self.client.switch_api_dtel_int_transit_enable(device)
        self.client.switch_api_dtel_int_endpoint_enable(device)

        # send the test packet(s)
        payload = 'int l45'
        # create a packet coming from INT src - i.e.
        #   - It has INT meta header, but no INT data
        #   Each transit device will fill the data
        pkt = simple_udp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_id=0,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=64,
            udp_sport=101,
            with_udp_chksum=False,
            udp_payload=payload)

        int_pkt = int_l45_src_packet(
            test=self,
            int_inst_mask=0xDC00,
            int_inst_cnt=5,
            pkt=pkt)
        # add 2 hop info to the packet
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x87654321, incr_cnt=1)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222221)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222222)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222223)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222224)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x12345678, incr_cnt=1)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222225)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222226)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222227)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222228)

        # make a DTel report packet
        exp_i2e_mirrored_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_src=params.ipaddr_nbr[0],
            ip_dst=params.ipaddr_nbr[1],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
            dropped=0,
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=0,
            inner_frame=int_pkt)

        try:
            print "Forward DTel report with no mirror session: neighbor"
            config = SwitchConfig(self, params)

            exp_i2e_mirrored_pkt.getlayer(IP, 1).dst=params.ipaddr_nbr[1]
            exp_pkt = exp_i2e_mirrored_pkt.copy()
            exp_pkt[Ether].src=params.mac_self
            exp_pkt[Ether].dst=params.mac_nbr[1]
            exp_pkt.getlayer(IP, 1).ttl=63

            try:
                send_packet(self, swports[0], str(exp_i2e_mirrored_pkt))
                verify_packet(self, exp_pkt, swports[1])
                verify_no_other_packets(self)
            finally:
                config.cleanup(self)

            print "Forward DTel report with no mirror session: "
            print "static route"

            params.routes = [('192.168.0.1', 1)]
            config = SwitchConfig(self, params)

            exp_i2e_mirrored_pkt.getlayer(IP, 1).dst='192.168.0.1'
            exp_pkt = exp_i2e_mirrored_pkt.copy()
            exp_pkt[Ether].src=params.mac_self
            exp_pkt[Ether].dst=params.mac_nbr[1]
            exp_pkt.getlayer(IP, 1).ttl=63

            try:
                send_packet(self, swports[0], str(exp_i2e_mirrored_pkt))
                verify_packet(self, exp_pkt, swports[1])
                verify_no_other_packets(self)
            finally:
                config.cleanup(self)
                params.routes = []

            print "Forward DTel report with mirror session: "
            print "diff next hop as mirror: neighbor"
            params.report_ports = [0]
            params.ipaddr_report_src = ['10.201.124.31']
            params.ipaddr_report_dst = ['10.201.124.40']
            params.mirror_ids = [555]
            config = SwitchConfig(self, params)

            exp_i2e_mirrored_pkt.getlayer(IP, 1).dst=params.ipaddr_nbr[1]
            exp_pkt = exp_i2e_mirrored_pkt.copy()
            exp_pkt[Ether].src=params.mac_self
            exp_pkt[Ether].dst=params.mac_nbr[1]
            exp_pkt.getlayer(IP, 1).ttl=63

            try:
                send_packet(self, swports[0], str(exp_i2e_mirrored_pkt))
                verify_packet(self, exp_pkt, swports[1])
                verify_no_other_packets(self)
            finally:
                config.cleanup(self)
                params.report_ports = None

            print "Forward DTel report with mirror session: "
            print "diff next hop as mirror: static"
            params.report_ports = [0]
            params.ipaddr_report_src = ['10.201.124.31']
            params.ipaddr_report_dst = ['10.201.124.40']
            params.mirror_ids = [555]
            params.routes = [('192.168.0.1', 1)]
            config = SwitchConfig(self, params)

            exp_i2e_mirrored_pkt.getlayer(IP, 1).dst='192.168.0.1'
            exp_pkt = exp_i2e_mirrored_pkt.copy()
            exp_pkt[Ether].src=params.mac_self
            exp_pkt[Ether].dst=params.mac_nbr[1]
            exp_pkt.getlayer(IP, 1).ttl=63

            try:
                send_packet(self, swports[0], str(exp_i2e_mirrored_pkt))
                verify_packet(self, exp_pkt, swports[1])
                verify_no_other_packets(self)
            finally:
                config.cleanup(self)
                params.report_ports = None
                params.routes = []

            print "Forward DTel report with mirror session: "
            print "same next hop as mirror: neighbor"
            params.report_ports = [1]
            params.ipaddr_report_src = ['10.201.124.31']
            params.ipaddr_report_dst = ['10.201.124.40']
            params.mirror_ids = [555]
            config = SwitchConfig(self, params)

            exp_i2e_mirrored_pkt.getlayer(IP, 1).dst=params.ipaddr_nbr[1]
            exp_pkt = exp_i2e_mirrored_pkt.copy()
            exp_pkt[Ether].src=params.mac_self
            exp_pkt[Ether].dst=params.mac_nbr[1]
            exp_pkt.getlayer(IP, 1).ttl=63

            try:
                send_packet(self, swports[0], str(exp_i2e_mirrored_pkt))
                verify_packet(self, exp_pkt, swports[1])
                verify_no_other_packets(self)
            finally:
                config.cleanup(self)
                params.report_ports = None

            print "Forward DTel report with mirror session: "
            print "same next hop as mirror: static"
            params.report_ports = [1]
            params.ipaddr_report_src = ['10.201.124.31']
            params.ipaddr_report_dst = ['10.201.124.40']
            params.mirror_ids = [555]
            params.routes = [('192.168.0.1', 1)]
            config = SwitchConfig(self, params)

            exp_i2e_mirrored_pkt.getlayer(IP, 1).dst='192.168.0.1'
            exp_pkt = exp_i2e_mirrored_pkt.copy()
            exp_pkt[Ether].src=params.mac_self
            exp_pkt[Ether].dst=params.mac_nbr[1]
            exp_pkt.getlayer(IP, 1).ttl=63

            send_packet(self, swports[0], str(exp_i2e_mirrored_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)

            config.cleanup(self)
            params.report_ports = None
            params.routes = []

            print "Forward DTel report with mirror session: "
            print "same dst as mirror: neighbor"
            params.report_ports = [1]
            params.ipaddr_report_src = ['10.201.124.31']
            params.ipaddr_report_dst = ['10.201.124.40']
            params.mirror_ids = [555]
            config = SwitchConfig(self, params)

            exp_i2e_mirrored_pkt.getlayer(IP, 1).dst='10.201.124.40'
            exp_pkt = exp_i2e_mirrored_pkt.copy()
            exp_pkt[Ether].src=params.mac_self
            exp_pkt[Ether].dst=params.mac_nbr[1]
            exp_pkt.getlayer(IP, 1).ttl=63

            try:
                send_packet(self, swports[0], str(exp_i2e_mirrored_pkt))
                verify_packet(self, exp_pkt, swports[1])
                verify_no_other_packets(self)
            finally:
                config.cleanup(self)
                params.report_ports = None

            print "Forward DTel report with mirror session: "
            print "same next hop as mirror: static"
            params.report_ports = [1]
            params.ipaddr_report_src = ['10.201.124.31']
            params.ipaddr_report_dst = ['10.201.124.40']
            params.mirror_ids = [555]
            params.routes = [('10.201.124.40', 1)]
            config = SwitchConfig(self, params)

            exp_i2e_mirrored_pkt.getlayer(IP, 1).dst='10.201.124.40'
            exp_pkt = exp_i2e_mirrored_pkt.copy()
            exp_pkt[Ether].src=params.mac_self
            exp_pkt[Ether].dst=params.mac_nbr[1]
            exp_pkt.getlayer(IP, 1).ttl=63

            try:
                send_packet(self, swports[0], str(exp_i2e_mirrored_pkt))
                verify_packet(self, exp_pkt, swports[1])
                verify_no_other_packets(self)
            finally:
                config.cleanup(self)
                params.report_ports = None
                params.routes = []

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_transit_disable(device)
            self.client.switch_api_dtel_int_endpoint_disable(device)
