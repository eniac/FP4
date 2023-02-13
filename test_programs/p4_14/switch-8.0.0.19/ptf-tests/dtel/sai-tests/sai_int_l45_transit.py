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
Thrift SAI interface INT transit tests
"""

import switchsai_thrift
import pdb
import time
import sys
import logging
import os
import unittest
import random

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *

from switchsai_thrift.ttypes import *
from switchsai_thrift.sai_headers import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../base'))
from common.utils import *

sys.path.append(os.path.join(this_dir, '../../base/sai-ocp-tests'))
import sai_base_test
from switch_utils import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '..'))
from dtel_utils import *
from dtel_sai_utils import *

SID = 0x11111111
swports = range(2)
devports = range(2)

params = SwitchSAIConfig_Params()
params.switch_id = SID
params.mac_self = '00:77:66:55:44:33'
params.nports = 2
params.ipaddr_inf = ['2.2.2.1', '1.1.1.2']
params.ipaddr_nbr = ['2.2.2.2', '1.1.1.1']
params.mac_nbr = ['00:11:22:33:44:54', '00:11:22:33:44:55']
params.report_ports = None
params.swports = swports
params.configure_routes = True

@group('int_transit')
class intl45_transitTest_switchid(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 transit device - add switch_id with total_hop_cnt=0"
        prepare_int_l45_bindings()
        # create SAI manager
        sai_mgr = SAIManager(self, params)

        sai_mgr.switch.dtel_int_transit_enable = True

        # create a packet coming from INT src - i.e.
        #   - It has INT meta header, but no INT data
        #   Each transit device will fill the data
        pkt = simple_tcp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_id=0,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=64)

        int_pkt = int_l45_src_packet(
            test=self,
            int_inst_mask=0x8000,  # only swid, 1 byte
            int_inst_cnt=1,
            pkt=pkt)
        exp_pkt = simple_tcp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_id=0,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=63)

        exp_pkt = int_l45_src_packet(
            test=self,
            int_inst_mask=0x8000,  # only swid, 1 byte
            int_inst_cnt=1,
            pkt=exp_pkt)

        try:
            send_packet(self, swports[0], str(int_pkt))

            exp_pkt = int_l45_packet_add_hop_info(
                Packet=exp_pkt, val=params.switch_id, incr_cnt=1)
            m = Mask(exp_pkt)
            m.set_do_not_care_scapy(TCP_INTL45, 'chksum')
            verify_packet(self, m, swports[1])
            verify_no_other_packets(self)

        finally:
            ### Cleanup
            sai_mgr.switch.dtel_int_transit_enable = False
            sai_mgr.cleanup()

@group('int_transit')
class intl45_transitTest_hop2_port_ids(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 transit device - add switch_id and port_ids on hop2"
        prepare_int_l45_bindings()
        # create SAI manager
        sai_mgr = SAIManager(self, params)

        sai_mgr.switch.dtel_int_transit_enable = True

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
            int_inst_mask=0xC000,  # swid, ingress/egress port ids
            int_inst_cnt=2,
            pkt=pkt)
        # add 1 hop info to the packet
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x00110003)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x66666666, incr_cnt=1)
        exp_pkt = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_id=0,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=63,
            udp_sport=101,
            with_udp_chksum=False,
            udp_payload=payload)

        exp_pkt = int_l45_src_packet(
            test=self, int_inst_mask=0xC000, int_inst_cnt=2, pkt=exp_pkt)

        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x00110003)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x66666666, incr_cnt=1)

        in_port = swports[0]
        out_port = swports[1]
        exp_port_ids = in_port * pow(2, 16) + out_port

        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=exp_port_ids)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=params.switch_id, incr_cnt=1)

        try:
            send_packet(self, swports[0], str(int_pkt))

            verify_packets(self, exp_pkt, [swports[1]])
            verify_no_other_packets(self)

        finally:
            ### Cleanup
            sai_mgr.switch.dtel_int_transit_enable = False
            sai_mgr.cleanup()

@group('int_transit')
class intl45_transitTest_Ebit(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 transit device - E bit"
        prepare_int_l45_bindings()
        # create SAI manager
        sai_mgr = SAIManager(self, params)

        sai_mgr.switch.dtel_int_transit_enable = True

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
            test=self, int_inst_mask=0xF700, int_inst_cnt=7, pkt=pkt)
        # add 1 hop info to the packet
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x10, incr_cnt=0)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x11, incr_cnt=0)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x12, incr_cnt=0)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x13, incr_cnt=0)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x14, incr_cnt=0)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x15, incr_cnt=0)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222222, incr_cnt=1)
        # Force Total cnt and max count to be the same
        int_pkt[INT_META_HDR].max_hop_cnt = int_pkt[INT_META_HDR].total_hop_cnt
        exp_pkt = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_id=0,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=63,
            udp_sport=101,
            with_udp_chksum=False,
            udp_payload=payload)
        exp_pkt = int_l45_src_packet(
            test=self, int_inst_mask=0xF700, int_inst_cnt=7, pkt=exp_pkt)

        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x10, incr_cnt=0)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x11, incr_cnt=0)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x12, incr_cnt=0)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x13, incr_cnt=0)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x14, incr_cnt=0)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x15, incr_cnt=0)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x22222222, incr_cnt=1)
        exp_pkt[INT_META_HDR].max_hop_cnt = exp_pkt[INT_META_HDR].total_hop_cnt
        exp_pkt[INT_META_HDR].e = 1

        try:
            send_packet(self, swports[0], str(int_pkt))
            verify_packets(self, exp_pkt, [swports[1]])
            verify_no_other_packets(self)

        finally:
            ### Cleanup
            sai_mgr.switch.dtel_int_transit_enable = False
            sai_mgr.cleanup()

@group('int_transit')
class intl45_transitTest_hop2_latency(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 transit device - encode 2hop info : latency"
        print "NOTE: hop latency is not supported. We skip this test."
        return
        prepare_int_l45_bindings()
        # create SAI manager
        sai_mgr = SAIManager(self, params)

        sai_mgr.switch.dtel_int_transit_enable = True

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
            int_inst_mask=0xA000,  #switch id + hop latency
            int_inst_cnt=2,
            pkt=pkt)
        # add 1 hop info to the packet
        int_pkt = int_l45_packet_add_hop_info(Packet=int_pkt, val=5)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222222, incr_cnt=1)

        exp_pkt = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_id=0,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=63,
            udp_sport=101,
            with_udp_chksum=False,
            udp_payload=payload)

        exp_pkt = int_l45_src_packet(
            test=self,
            int_inst_mask=0xA000,  # switch id + hop latency
            int_inst_cnt=2,
            pkt=exp_pkt)

        exp_pkt = int_l45_packet_add_hop_info(Packet=exp_pkt, val=0x5)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x22222222, incr_cnt=1)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x7FFFFFFF)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=params.switch_id, incr_cnt=1)

        try:
            send_packet(self, swports[0], str(int_pkt))
            (rcv_latency,_) = verify_int_packet(
                test=self,
                pkt=exp_pkt,
                port=swports[1],
                digest=False,
                ignore_hop_indices=[2])  # ignore idx 2 of INT stack

            print "Exported latency :", rcv_latency[0]
            verify_no_other_packets(self)

        finally:
            ### Cleanup
            sai_mgr.switch.dtel_int_transit_enable = False
            sai_mgr.cleanup()

@group('int_transit')
class intl45_transitTest_hop2_qdepth(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 transit device - and encode 2hop info: qdepth"
        prepare_int_l45_bindings()
        # create SAI manager
        sai_mgr = SAIManager(self, params)

        sai_mgr.switch.dtel_int_transit_enable = True

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
            int_inst_mask=0x9000,  #switch id +  q depth
            int_inst_cnt=2,
            pkt=pkt)
        # add 1 hop info to the packet
        int_pkt = int_l45_packet_add_hop_info(Packet=int_pkt, val=5)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222222, incr_cnt=1)

        exp_pkt = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_id=0,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=63,
            udp_sport=101,
            with_udp_chksum=False,
            udp_payload=payload)

        exp_pkt = int_l45_src_packet(
            test=self,
            int_inst_mask=0x9000,  # switch id +  q depth
            int_inst_cnt=2,
            pkt=exp_pkt)

        exp_pkt = int_l45_packet_add_hop_info(Packet=exp_pkt, val=0x5)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x22222222, incr_cnt=1)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x7FFFFFFF)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=params.switch_id, incr_cnt=1)

        try:
            send_packet(self, swports[0], str(int_pkt))
            (rcv_qdepth,_) = verify_int_packet(
                test=self,
                pkt=exp_pkt,
                port=swports[1],
                digest=False,
                ignore_hop_indices=[2])  # ignore idx 2 of INT stack

            print "Exported q depth :", rcv_qdepth[0]
            verify_no_other_packets(self)

        finally:
            ### Cleanup
            sai_mgr.switch.dtel_int_transit_enable = False
            sai_mgr.cleanup()

@group('int_transit')
class intl45_transitTest_Metadata(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 transit device - all metadata"
        prepare_int_l45_bindings()
        # create SAI manager
        sai_mgr = SAIManager(self, params)

        sai_mgr.switch.dtel_int_transit_enable = True

        # create a packet coming from INT src - i.e.
        #   - It has INT meta header, but no INT data
        #   Each transit device will fill the data
        pkt = simple_tcp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_id=0,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=64)

        int_pkt = int_l45_src_packet(
            test=self,
            int_inst_mask=0xDC00,
            int_inst_cnt=5,
            pkt=pkt)
        # add 1 hop info to the packet
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=5, incr_cnt=1)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222221)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222222)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222223)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222224)

        exp_pkt = simple_tcp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_id=0,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=63)

        exp_pkt = int_l45_src_packet(
            test=self,
            int_inst_mask=0xDC00,
            int_inst_cnt=5,
            pkt=exp_pkt)

        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=5, incr_cnt=1)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x22222221)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x22222222)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x22222223)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x22222224)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x22222226)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x22222227)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x22222228)

        in_port = swports[0]
        out_port = swports[1]
        exp_port_ids = in_port * pow(2, 16) + out_port

        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=exp_port_ids)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=params.switch_id, incr_cnt=1)

        try:
            send_packet(self, swports[0], str(int_pkt))
            (rcv_metadata,_) = verify_int_packet(
                test=self,
                pkt=exp_pkt,
                port=swports[1],
                digest=False,
                ignore_hop_indices=[3, 4, 5],
                ignore_chksum=True)

            verify_no_other_packets(self)

        finally:
            ### Cleanup
            sai_mgr.switch.dtel_int_transit_enable = False
            sai_mgr.cleanup()

@group('int_transit')
@group('int_transit_q')
class intl45_transitTest_hop2_stateless(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 transit device with stateless suppression"
        prepare_int_l45_bindings()
        # create SAI manager
        sai_mgr = SAIManager(self, params)

        sai_mgr.switch.dtel_int_transit_enable = True

        # create a packet coming from INT src - i.e.
        #   - It has INT meta header, but no INT data
        #   Each transit device will fill the data
        payload = 'int l45'
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
            int_inst_mask=0xC000,  #switch id + ports
            int_inst_cnt=2,
            pkt=pkt)

        exp_pkt = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_id=0,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=63,
            udp_sport=101,
            with_udp_chksum=False,
            udp_payload=payload)

        exp_pkt = int_l45_src_packet(
            test=self,
            int_inst_mask=0xC000,  #switch id + ports
            int_inst_cnt=2,
            pkt=exp_pkt)

        # add 1 hop info to the packet
        int_pkt = int_l45_packet_add_hop_info(Packet=int_pkt, val=5)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222222, incr_cnt=1)

        exp_pkt = int_l45_packet_add_hop_info(Packet=exp_pkt, val=0x5)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x22222222, incr_cnt=1)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt,
            val=int_port_ids_pack(devports[0], devports[1]))
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=SID, incr_cnt=1)

        queue_report_enabled = False
        try:
            print "send traffic without enabling the queue alert"
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)

            print "send traffic with queue alert from upstream"
            int_pkt[INT_META_HDR].rsvd2 = 0x1
            exp_pkt[INT_META_HDR].rsvd2 = 0x1
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            int_pkt[INT_META_HDR].rsvd2 = 0x0
            exp_pkt[INT_META_HDR].rsvd2 = 0x0

            print "enable queue alert threshold: high threshold"
            # set queue alert with latency threshold max
            #pdb.set_trace()
            queue_report = sai_mgr.create_dtel_queue_report(
                swports[1], 0, hex_to_i32(0xfff), hex_to_i32(0xffffffff),
                1024, True)
            queue_alert_enabled = True

            # high threshould should not set the report bit
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            #receive_print_packet(self, swports[1], exp_pkt, True, True)
            verify_no_other_packets(self)

            print "enable queue alert threshold: low threshold"
            # set queue alert with latency threshold 0
            queue_report.depth_threshold = 0
            queue_report.latency_threshold = 0
            # low threshould should set the report bit
            send_packet(self, swports[0], str(int_pkt))
            exp_pkt[INT_META_HDR].rsvd2 = 0x1
            verify_packet(self, exp_pkt, swports[1])
            #receive_print_packet(self, swports[1], exp_pkt, True, True)
            verify_no_other_packets(self)
            print "intermediate"
            int_pkt[INT_META_HDR].rsvd2 = 0x1
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            int_pkt[INT_META_HDR].rsvd2 = 0x0

            print "disable queue alert"
            # disable queue alsert
            queue_report.delete()
            queue_report_enabled = False
            send_packet(self, swports[0], str(int_pkt))
            exp_pkt[INT_META_HDR].rsvd2 = 0
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)

        finally:
            ### Cleanup
            sai_mgr.switch.dtel_int_transit_enable = False
            sai_mgr.cleanup()
