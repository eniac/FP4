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
INT L45 transit tests for stateless suppression
"""

import switchapi_thrift

import time
import sys
import logging
import ctypes

import unittest
import random

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *

import os
from math import ceil

from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *
import pdb

import pd_base_tests

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../../base'))
from common.utils import *
sys.path.append(os.path.join(this_dir, '../../../base/api-tests'))
import api_base_tests

sys.path.append(os.path.join(this_dir, '../..'))
from dtel_utils import *



device = 0

swports = [0, 1, 2]

SID = 0x11111111
params = SwitchConfig_Params()
params.switch_id = SID
params.mac_self = '00:77:66:55:44:33'
params.nports = 3
params.ipaddr_inf = ['2.2.2.1',  '1.1.1.2', '10.1.0.4']
params.ipaddr_nbr = ['2.2.2.2', '1.1.1.1', '10.1.0.1']
params.mac_nbr = ['00:11:22:33:44:54', '00:11:22:33:44:55', '00:11:22:33:44:56']
params.report_ports = [2]
params.ipaddr_report_src = ['4.4.4.1']
params.ipaddr_report_dst = ['4.4.4.3']
params.mirror_ids = [555]
params.device = device
params.swports = swports

@group('transit_l45')
class intl45_transitTest_hop2_stateless(api_base_tests.ThriftInterfaceDataPlane,
                                        pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        print "Test INT L45 transit device with stateless suppression"
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

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

        exp_pkt_noint = simple_udp_packet(
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
            pkt=exp_pkt_noint)

        # add 1 hop info to the packet
        int_pkt = int_l45_packet_add_hop_info(Packet=int_pkt, val=5)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222222, incr_cnt=1)

        exp_pkt = int_l45_packet_add_hop_info(Packet=exp_pkt, val=0x5)
        exp_pkt_1hop = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x22222222, incr_cnt=1)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt_1hop,
            val=int_port_ids_pack(swports[0], swports[1]))
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=SID, incr_cnt=1)

        exp_inte2e_inner_1 = postcard_report(
            packet=exp_pkt_noint,
            switch_id=SID,
            ingress_port=swports[0],
            egress_port=swports[1],
            queue_id=0,
            queue_depth=0,
            egress_tstamp=0)

        exp_e2e_pkt_noint = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
            dropped=0,
            congested_queue=1,
            path_tracking_flow=0,
            hw_id=hw_id,
            inner_frame=exp_inte2e_inner_1)

        exp_inte2e_inner_1 = postcard_report(
            packet=exp_pkt_1hop,
            switch_id=SID,
            ingress_port=swports[0],
            egress_port=swports[1],
            queue_id=0,
            queue_depth=0,
            egress_tstamp=0)

        exp_e2e_pkt_1hop = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
            dropped=0,
            congested_queue=1,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=exp_inte2e_inner_1)

        exp_inte2e_inner_1 = postcard_report(
            packet=exp_pkt,
            switch_id=SID,
            ingress_port=swports[0],
            egress_port=swports[1],
            queue_id=0,
            queue_depth=0,
            egress_tstamp=0)

        exp_e2e_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
            dropped=0,
            congested_queue=1,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=exp_inte2e_inner_1)

        queue_report_enabled = False
        transit_enabled = False
        try:
            # Enable INT transit processing
            self.client.switch_api_dtel_int_transit_enable(device)
            transit_enabled = True

            print "send traffic without enabling the queue report"
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)

            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt_noint, swports[1])
            verify_no_other_packets(self)

            print "enable queue report threshold: high threshold"
            # set queue report with latency threshold max
            self.client.switch_api_dtel_queue_report_create(
                device, swports[1], 0,
                hex_to_i32(0xfff), hex_to_i32(0xffffffff), 1024, False)
            queue_report_enabled = True

            # high threshould should not set the report bit
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            #receive_print_packet(self, swports[1], exp_pkt, True, True)
            verify_no_other_packets(self)

            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt_noint, swports[1])
            verify_no_other_packets(self)

            print "enable queue report threshold: low threshold"
            # set queue report with latency threshold 0
            self.client.switch_api_dtel_queue_report_update(
                device, swports[1], 0, 0, 0, 1024, False)
            # low threshould should send the report
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)

            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt_noint, swports[1])
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt_noint, swports[params.report_ports[0]])
            verify_no_other_packets(self)

            print "disable transit"
            self.client.switch_api_dtel_int_transit_disable(device)
            transit_enabled = False
            exp_e2e_pkt_1hop[DTEL_REPORT_HDR].path_tracking_flow = 0
            # low threshould should send the report
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt_1hop, swports[1])
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt_1hop, swports[params.report_ports[0]])
            verify_no_other_packets(self)

            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt_noint, swports[1])
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt_noint, swports[params.report_ports[0]])
            verify_no_other_packets(self)

            print "enable transit again"
            self.client.switch_api_dtel_int_transit_enable(device)
            transit_enabled = True
            exp_e2e_pkt_1hop[DTEL_REPORT_HDR].path_tracking_flow = 1
            # low threshould should set the report bit
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)

            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt_noint, swports[1])
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt_noint, swports[params.report_ports[0]])
            verify_no_other_packets(self)

            print "change report dscp"
            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_Q_REPORT_THRESHOLD_BREACH, 10)

            exp_e2e_pkt[IP].tos = 10<<2
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)

            exp_e2e_pkt_noint[IP].tos = 10<<2
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt_noint, swports[1])
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt_noint, swports[params.report_ports[0]])
            verify_no_other_packets(self)

            print "change report port"
            bind_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT^0x1111)
            split_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT)
            self.client.switch_api_dtel_report_udp_dstport_set(
                device, UDP_PORT_DTEL_REPORT^0x1111);
            exp_e2e_pkt[UDP].dport = UDP_PORT_DTEL_REPORT^0x1111
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            exp_e2e_pkt[UDP].dport = UDP_PORT_DTEL_REPORT
            self.client.switch_api_dtel_report_udp_dstport_set(
                device, UDP_PORT_DTEL_REPORT);
            split_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT^0x1111)
            bind_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT)

            print "enable queue report threshold: high threshold"
            # set queue report with latency threshold max
            self.client.switch_api_dtel_queue_report_update(
                device, swports[1], 0,
                hex_to_i32(0xfff), hex_to_i32(0xffffffff), 1024, False)
            # high threshould should not set the report bit
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            #receive_print_packet(self, swports[1], exp_pkt, True, True)
            verify_no_other_packets(self)

            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt_noint, swports[1])
            verify_no_other_packets(self)

            print "disable queue report"
            # disable queue report
            self.client.switch_api_dtel_queue_report_delete(
                device, swports[1], 0)
            queue_report_enabled = False
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)

            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt_noint, swports[1])
            verify_no_other_packets(self)

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_report_udp_dstport_set(
                device, UDP_PORT_DTEL_REPORT);
            split_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT^0x1111)
            bind_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT)
            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_Q_REPORT_THRESHOLD_BREACH, 0)
            if queue_report_enabled:
                self.client.switch_api_dtel_queue_report_delete(
                    device, swports[1], 0)
            if transit_enabled:
                self.client.switch_api_dtel_int_transit_disable(device)
            config.cleanup(self)
