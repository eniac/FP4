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

quantization_shift = MAX_QUANTIZATION

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

# flow 2.2.2.200 -> 1.1.1.100 for watchlist
twl_kvp = []
kvp_val = switcht_twl_value_t(
    value_num=ipv4Addr_to_i32(params.ipaddr_nbr[0]))
kvp_mask = switcht_twl_value_t(value_num=0xffffff00)
twl_kvp.append(switcht_twl_key_value_pair_t(
    SWITCH_TWL_FIELD_IPV4_SRC, kvp_val, kvp_mask))
kvp_val = switcht_twl_value_t(
    value_num=ipv4Addr_to_i32(params.ipaddr_nbr[1]))
kvp_mask = switcht_twl_value_t(value_num=0xffffff00)
twl_kvp.append(switcht_twl_key_value_pair_t(
    SWITCH_TWL_FIELD_IPV4_DST, kvp_val, kvp_mask))

@group('transit_l45')
class intl45_transitTest_switchid(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 transit device - add switch_id with total_hop_cnt=0"
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        # Enable INT transit processing
        self.client.switch_api_dtel_int_transit_enable(device)

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
            if exp_pkt.haslayer(TCP_INTL45):
                m.set_do_not_care_scapy(TCP_INTL45, 'chksum')
            else:
                m.set_do_not_care_scapy(TCP, 'chksum')
            verify_packet(self, m, swports[1])
            verify_no_other_packets(self)

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_transit_disable(device)
            config.cleanup(self)

@group('transit_l45')
class intl45_transitTest_latency_shift(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 transit, test TM/P4 latency shift/mask"
        print "dataplane doesn't report latency, this test needs inspection of model log"
        return

        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        # Enable INT transit processing
        self.client.switch_api_dtel_int_transit_enable(device)

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

        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=params.switch_id, incr_cnt=1)
        m = Mask(exp_pkt)
        if exp_pkt.haslayer(TCP_INTL45):
            m.set_do_not_care_scapy(TCP_INTL45, 'chksum')
        else:
            m.set_do_not_care_scapy(TCP, 'chksum')

        try:
            print "assume the model puts a known value into deq_timedelta (TM latency)"
            # change model/src/shared/queueing.cpp
            # put pkt->qing2e_metadata()->set_delay(0xF732F731);
            # and compile model.

            # to check the computed field values:
            # tee model log into a file.
            # tail -f [file] | grep deq_timedelta
            # tail -f [file] | grep quantized_latency

            tm_latency = 0xF732F731
            tm_shift = 0
            p4_shift = 0
            # test the entire range of quantization shift
            for shift in range(0, MAX_QUANTIZATION + 1):
                # change quantization shift
                self.client.switch_api_dtel_latency_quantization_shift(
                    device=device, quant_shift=shift)

                send_packet(self, swports[0], str(int_pkt))

                verify_packet(self, m, swports[1])
                verify_no_other_packets(self)


                if shift >= 27:
                    tm_shift = 0
                    quantized_latency = 0
                else:
                    shift = int(shift / 2) * 2
                    if shift <= 12:
                        tm_shift = 0
                        p4_shift = shift
                        quantized_latency = (tm_latency >> p4_shift) & (0x3FFFF >> p4_shift)
                    else:
                        tm_shift = shift - 12
                        p4_shift = 12
                        quantized_latency = (tm_latency >> shift) & 0x3F

                masked_deq_timedelta = tm_latency & (0x3FFFF << tm_shift)

                print "shift is", shift
                print "masked deq_timedelta must be", hex(masked_deq_timedelta)
                print "quantized_latency must be", hex(quantized_latency)
                raw_input("compare to model log")

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_transit_disable(device)
            self.client.switch_api_dtel_latency_quantization_shift(
                device=device, quant_shift=0)
            config.cleanup(self)

@group('transit_l45')
class intl45_DSCP_TransitTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 transit device - change L45 DSCP value"
        if get_int_l45_encap() != "dscp":
            print "Not running with INT L45 encap using diffserv"
            print "Skipping this test"
            return

        config = SwitchConfig(self, params)

        # Enable INT transit processing
        self.client.switch_api_dtel_int_transit_enable(device)

        current_dscp = 0x02
        prepare_int_l45_bindings(current_dscp, current_dscp)
        self.client.switch_api_dtel_int_dscp_value_set(
            device,
            current_dscp,
            current_dscp)

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

        exp_pkt[IP].tos = current_dscp << 2
        int_pkt[IP].tos = current_dscp << 2
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=params.switch_id, incr_cnt=1)

        try:
            send_packet(self, swports[0], str(int_pkt))
            m = Mask(exp_pkt)
            m.set_do_not_care_scapy(TCP_INTL45, 'chksum')
            verify_packet(self, m, swports[1])
            verify_no_other_packets(self)
            print "pass dscp 0x%x mask 0x%x" %(current_dscp,
                                               current_dscp)

            current_dscp = 0x01
            prepare_int_l45_bindings(current_dscp, current_dscp)
            self.client.switch_api_dtel_int_dscp_value_set(
                device,
                current_dscp,
                current_dscp)
            exp_pkt[IP].tos = current_dscp << 2
            int_pkt[IP].tos = current_dscp << 2

            send_packet(self, swports[0], str(int_pkt))
            m = Mask(exp_pkt)
            m.set_do_not_care_scapy(TCP_INTL45, 'chksum')
            verify_packet(self, m, swports[1])
            verify_no_other_packets(self)
            print "pass dscp 0x%x mask 0x%x" %(current_dscp,
                                               current_dscp)

            current_dscp = 0xF
            current_dscp_mask = 0x3f
            prepare_int_l45_bindings(current_dscp, current_dscp_mask)
            self.client.switch_api_dtel_int_dscp_value_set(
                device,
                current_dscp,
                current_dscp_mask);
            exp_pkt[IP].tos = current_dscp << 2
            int_pkt[IP].tos = current_dscp << 2

            send_packet(self, swports[0], str(int_pkt))
            m = Mask(exp_pkt)
            m.set_do_not_care_scapy(TCP_INTL45, 'chksum')
            verify_packet(self, m, swports[1])
            verify_no_other_packets(self)
            print "pass dscp 0x%x mask 0x%x" %(current_dscp,
                                               current_dscp_mask)

            self.client.switch_api_dtel_int_transit_disable(device)
            self.client.switch_api_dtel_int_transit_enable(device)
            send_packet(self, swports[0], str(int_pkt))
            m = Mask(exp_pkt)
            m.set_do_not_care_scapy(TCP_INTL45, 'chksum')
            verify_packet(self, m, swports[1])
            verify_no_other_packets(self)
            print "pass dscp 0x%x mask 0x%x after disable/enable" %(current_dscp,
                                               current_dscp_mask)

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_transit_disable(device)
            config.cleanup(self)

@group('transit_l45_chksum')
class intl45_transitTest_CHECKSUM(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 transit device - add switch_id and checksum"
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        # Enable INT transit processing
        self.client.switch_api_dtel_int_transit_enable(device)

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
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_transit_disable(device)
            config.cleanup(self)

@group('transit_l45')
class intl45_transitTest_hop2_txutil_yet_supported(
        api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 transit device - add switch_id and tx util on hop2"
        print "NOTE: tx util is not supported. We skip this test."
        return
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        # Enable INT transit processing
        self.client.switch_api_dtel_int_transit_enable(device)

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
            int_inst_mask=0x8100,  # swid, tx_util 1 byte
            int_inst_cnt=2,
            pkt=pkt)
        # add 1 hop info to the packet
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x66666666)
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
            test=self, int_inst_mask=0x8100, int_inst_cnt=2, pkt=exp_pkt)

        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x66666666)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x22222222, incr_cnt=1)
        ## At this time p4 code does not support this info
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x7FFFFFFF)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=params.switch_id, incr_cnt=1)

        try:
            send_packet(self, swports[0], str(int_pkt))
            verify_packets(self, exp_pkt, [swports[1]])
            verify_no_other_packets(self)

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_transit_disable(device)
            config.cleanup(self)


@group('transit_l45')
class intl45_transitTest_hop2_port_ids(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 transit device - add switch_id and port_ids on hop2"
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        # Enable INT transit processing
        self.client.switch_api_dtel_int_transit_enable(device)

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
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_transit_disable(device)
            config.cleanup(self)


@group('transit_l45')
class intl45_transitTest_Ebit(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 transit device - E bit"
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        # Enable INT transit processing
        self.client.switch_api_dtel_int_transit_enable(device)

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
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_transit_disable(device)
            config.cleanup(self)


@group('transit_l45')
class intl45_transitTest_hop2_latency(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 transit device - encode 2hop info : latency"
        print "NOTE: hop latency is not supported. We skip this test."
        return
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        # Enable INT transit processing
        self.client.switch_api_dtel_int_transit_enable(device)

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
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_transit_disable(device)
            config.cleanup(self)


@group('transit_l45')
class intl45_transitTest_hop2_qdepth(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 transit device - and encode 2hop info: qdepth"
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        # Enable INT transit processing
        self.client.switch_api_dtel_int_transit_enable(device)

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
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_transit_disable(device)
            config.cleanup(self)


@group('transit_l45')
class intl45_transitTest_Metadata(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 transit device - all metadata"
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        # Enable INT transit processing
        self.client.switch_api_dtel_int_transit_enable(device)

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
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_transit_disable(device)
            config.cleanup(self)


@group('transit_l45')
class INTL45_TransitTest_Enable(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print ("Test INT L45 transit device - test packets before and after"
        " enable and for non int packets")
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

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
            int_inst_mask=0x8000,  #switch id
            int_inst_cnt=1,
            pkt=pkt)
        # add 1 hop info to the packet
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222222, incr_cnt=1)

        exp_pkt_orig = simple_udp_packet(
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
            int_inst_mask=0x8000,  # switch id
            int_inst_cnt=1,
            pkt=exp_pkt_orig)

        exp_pkt_1 = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x22222222, incr_cnt=1)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt_1, val=params.switch_id, incr_cnt=1)

        transit_enabled = False
        try:
            print "send packets without enabling transit"
            send_packet(self, swports[0], str(int_pkt))
            # no new int is expected
            verify_packet(self, exp_pkt_1, swports[1])
            verify_no_other_packets(self)

            # packets without INT go through too
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt_orig, swports[1])
            verify_no_other_packets(self)

            print "enable transit and send packets"
            # Enable INT transit processing
            self.client.switch_api_dtel_int_transit_enable(device)
            transit_enabled = True

            send_packet(self, swports[0], str(int_pkt))
            # no new int is expected
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)

            # packets without INT go through too
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt_orig, swports[1])
            verify_no_other_packets(self)

            print "disable transit and send packets"
            self.client.switch_api_dtel_int_transit_disable(device)
            transit_enabled = False

            send_packet(self, swports[0], str(int_pkt))
            # no new int is expected
            verify_packet(self, exp_pkt_1, swports[1])
            verify_no_other_packets(self)

            # packets without INT go through too
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt_orig, swports[1])
            verify_no_other_packets(self)

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            if transit_enabled:
                self.client.switch_api_dtel_int_transit_disable(device)
            config.cleanup(self)

@group('transit_l45')
class INTL45_Transit_EgressMoDTest(api_base_tests.ThriftInterfaceDataPlane,
                           pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT Transit device with mirror on drop at egress"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        prepare_int_l45_bindings()
        bind_mirror_on_drop_pkt()
        config = SwitchConfig(self, params)

        mtu_set = False
        system_acl_rule_created = False

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)


        # Add MoD watchlist
        ap = switcht_twl_mod_params_t(report_queue_tail_drops=False)
        self.client.switch_api_dtel_drop_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        payload = 'int l45'
        # make input frame to inject to sink
        pkt = simple_udp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_id=108,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=64,
            with_udp_chksum=False,
            pktlen=256,
            udp_sport=101,
            udp_payload=payload)

        int_pkt_orig = int_l45_src_packet(
            test=self,
            int_inst_mask=0x8000,  # swid
            int_inst_cnt=1,
            max_hop_cnt=8,
            pkt=pkt)

        # add 2 hop info to the packet
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt_orig, val=0x66666666, incr_cnt=1)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222222, incr_cnt=1)

        exp_pkt = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_id=108,
            ip_ttl=63,
            pktlen=256,
            with_udp_chksum=False,
            udp_sport=101,
            udp_payload=payload)

        exp_int_orig = int_l45_src_packet(
            test=self,
            int_inst_mask=0x8000,  # swid
            int_inst_cnt=1,
            max_hop_cnt=8,
            pkt=exp_pkt)

        # add 3 hop info to the packet
        exp_int_pkt = int_l45_packet_add_hop_info(
            Packet=exp_int_orig, val=0x66666666, incr_cnt=1)
        exp_int_pkt = int_l45_packet_add_hop_info(
            Packet=exp_int_pkt, val=0x22222222, incr_cnt=1)

        exp_mod_inner_1 = mod_report(
            packet=exp_int_pkt,
            switch_id=SID,
            ingress_port=swports[0],
            egress_port=swports[1],
            queue_id=0,
            drop_reason=70)  # drop mtu check fail

        exp_mod_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_MOD,
            dropped=1,
            congested_queue=0,
            path_tracking_flow=0,
            hw_id=hw_id,
            inner_frame=exp_mod_inner_1)

        queue_report_enabled = False
        try:
            # create system_acl rule to drop packets exceeding MTU
            acl = self.client.switch_api_acl_list_create(
                device, SWITCH_API_DIRECTION_EGRESS,
                SWITCH_ACL_TYPE_EGRESS_SYSTEM, SWITCH_HANDLE_TYPE_NONE)
            system_acl_rule_created = True
            acl_kvp = []
            acl_kvp_val = switcht_acl_value_t(value_num=0)
            acl_kvp_mask = switcht_acl_value_t(value_num=1)
            acl_kvp.append(switcht_acl_key_value_pair_t(
                SWITCH_ACL_EGRESS_SYSTEM_FIELD_DEFLECT, acl_kvp_val, acl_kvp_mask))
            acl_kvp_val = switcht_acl_value_t(value_num=0)
            acl_kvp_mask = switcht_acl_value_t(value_num=0xffff)
            acl_kvp.append(switcht_acl_key_value_pair_t(
                SWITCH_ACL_EGRESS_SYSTEM_FIELD_L3_MTU_CHECK, acl_kvp_val, acl_kvp_mask))
            action = SWITCH_ACL_EGRESS_SYSTEM_ACTION_DROP
            action_params = switcht_acl_action_params_t(
                drop=switcht_acl_action_drop(reason_code=70))  # mtu check fail
            opt_action_params = switcht_acl_opt_action_params_t()
            ace = self.client.switch_api_acl_egress_system_rule_create(
                device, acl, 3000, 2, acl_kvp, action, action_params,
                opt_action_params)

            # set MTU to 200
            self.client.switch_api_dtel_drop_report_enable(device)
            mtu_200 = self.client.switch_api_l3_mtu_create(
                device, SWITCH_MTU_TYPE_IPV4, 200)
            self.client.switch_api_rif_mtu_set(
                device, config.rifs[1], mtu_200)
            mtu_set = True

            # send a test pkt
            send_packet(self, swports[0], str(int_pkt))
            # packet is dropped
            #verify_packet(self, exp_pkt, swports[1])
            # verify mod packet as mod wins
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for int egress + MOD + transit disabled"

            exp_int_pkt = int_l45_packet_add_hop_info(
                Packet=exp_int_pkt, val=SID, incr_cnt=1)
            exp_mod_inner_1 = mod_report(
                packet=exp_int_pkt,
                switch_id=SID,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                drop_reason=70)  # drop mtu check fail

            exp_mod_pkt = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_MOD,
                dropped=1,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=exp_mod_inner_1)


            # enable int-tr
            self.client.switch_api_dtel_int_transit_enable(device)

            # send a test pkt
            send_packet(self, swports[0], str(int_pkt))
            # packet is dropped
            #verify_packet(self, exp_pkt, swports[1])
            # verify mod packet as mod wins
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for int egress + MOD"

            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_DROP_REPORT, 5)
            self.assertTrue(self.client.switch_api_dtel_event_get_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_DROP_REPORT)==5)
            exp_mod_pkt[IP].tos = 5<<2
            send_packet(self, swports[0], str(int_pkt))
            # packet is dropped
            #verify_packet(self, exp_pkt, swports[1])
            # verify mod packet as mod wins
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            print "Passed for int egress + MOD + new DSCP"

            self.client.switch_api_dtel_report_udp_dstport_set(
                device, UDP_PORT_DTEL_REPORT^0x1111);
            bind_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT^0x1111)
            split_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT)
            exp_mod_pkt[UDP].dport = UDP_PORT_DTEL_REPORT^0x1111
            send_packet(self, swports[0], str(int_pkt))
            # packet is dropped
            #verify_packet(self, exp_pkt, swports[1])
            # verify mod packet as mod wins
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)

            self.client.switch_api_dtel_report_udp_dstport_set(
                device, UDP_PORT_DTEL_REPORT);
            split_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT^0x1111)
            bind_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT)
            exp_mod_pkt[UDP].dport = UDP_PORT_DTEL_REPORT
            print "Passed for int egress + MOD + Report UDP port"


            self.client.switch_api_dtel_queue_report_create(
                device, swports[1], 0, 0, 0, 1, False)
            queue_report_enabled = True

            exp_mod_pkt[DTEL_REPORT_HDR].congested_queue = 1
            send_packet(self, swports[0], str(int_pkt))
            # verify mod packet as mod wins
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)

            # quota finished now
            exp_mod_pkt[DTEL_REPORT_HDR].congested_queue = 0
            send_packet(self, swports[0], str(int_pkt))
            # verify mod packet as mod wins
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)

            self.client.switch_api_dtel_queue_report_delete(
                    device, swports[1], 0)
            queue_report_enabled = False
            print "Passed for int egress + MOD + Queue Report"

        finally:
            ### Cleanup
            split_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT^0x1111)
            bind_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT)
            cleanup_int_l45_bindings()
            split_mirror_on_drop_pkt()
            if queue_report_enabled:
                self.client.switch_api_dtel_queue_report_delete(
                    device, swports[1], 0)
            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_DROP_REPORT, 0)
            self.client.switch_api_dtel_report_udp_dstport_set(
                device, UDP_PORT_DTEL_REPORT);
            if mtu_set:
                self.client.switch_api_l3_mtu_delete(device, mtu_200)
            self.client.switch_api_dtel_int_transit_disable(device=device)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            self.client.switch_api_dtel_drop_report_disable(device)
            self.client.switch_api_dtel_drop_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            if system_acl_rule_created:
                self.client.switch_api_acl_rule_delete(device, acl, ace)
                self.client.switch_api_acl_list_delete(device, acl)
            config.cleanup(self)

@group('transit_l45')
class INTL45_Transit_IngressMoDTest(api_base_tests.ThriftInterfaceDataPlane,
                            pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45 Transit device with mirror on drop at Ingress"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        prepare_int_l45_bindings()
        bind_mirror_on_drop_pkt()
        config = SwitchConfig(self, params)
        twl_kvp = []

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        # Add MoD watchlist
        ap = switcht_twl_mod_params_t(report_queue_tail_drops=False)
        self.client.switch_api_dtel_drop_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        payload = 'int l45'
        # make input frame to inject to sink
        pkt = simple_udp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[1],
            ip_id=108,
            ip_dst=params.ipaddr_nbr[2],
            ip_src=params.ipaddr_nbr[1],
            ip_ttl=64,
            with_udp_chksum=False,
            udp_sport=101,
            udp_payload=payload)

        int_pkt_orig = int_l45_src_packet(
            test=self,
            int_inst_mask=0x8000,  # swid
            int_inst_cnt=1,
            max_hop_cnt=8,
            pkt=pkt)

        # add 2 hop info to the packet
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt_orig, val=0x66666666, incr_cnt=1)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222222, incr_cnt=1)

        exp_pkt = simple_udp_packet(
            eth_dst=params.mac_nbr[2],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[2],
            ip_src=params.ipaddr_nbr[1],
            ip_id=108,
            ip_ttl=63,
            with_udp_chksum=False,
            udp_sport=101,
            udp_payload=payload)

        exp_int_orig = int_l45_src_packet(
            test=self,
            int_inst_mask=0x8000,  # swid
            int_inst_cnt=1,
            max_hop_cnt=8,
            pkt=exp_pkt)

        # add 3 hop info to the packet
        exp_int_pkt = int_l45_packet_add_hop_info(
            Packet=exp_int_orig, val=0x66666666, incr_cnt=1)
        exp_int_pkt = int_l45_packet_add_hop_info(
            Packet=exp_int_pkt, val=0x22222222, incr_cnt=1)

        # mod report packet
        exp_mod_inner = mod_report(
            packet=int_pkt,
            switch_id=SID,
            ingress_port=swports[1],
            egress_port=INVALID_PORT_ID,
            queue_id=0,
            drop_reason=80)  # drop acl deny

        exp_mod_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_MOD,
            dropped=1,
            congested_queue=0,
            path_tracking_flow=0,
            hw_id=hw_id,
            inner_frame=exp_mod_inner)

        try:
            # config MoD
            self.client.switch_api_dtel_drop_report_enable(device)

            # setup acl
            acl = self.client.switch_api_acl_list_create(
                0, SWITCH_API_DIRECTION_INGRESS,
                SWITCH_ACL_TYPE_IP,
                SWITCH_HANDLE_TYPE_PORT)

            # create kvp to match destination IP
            kvp = []
            kvp_val1 = switcht_acl_value_t(
                value_num=int(socket.inet_aton(
                    params.ipaddr_nbr[2]).encode('hex'), 16))
            kvp_mask1 = switcht_acl_value_t(value_num=int("ffffffff", 16))
            kvp.append(
                switcht_acl_key_value_pair_t(SWITCH_ACL_IP_FIELD_IPV4_DEST,
                                             kvp_val1, kvp_mask1))
            action = SWITCH_ACL_ACTION_DROP
            action_params = switcht_acl_action_params_t(
                redirect=switcht_acl_action_redirect(handle=0))
            opt_action_params = switcht_acl_opt_action_params_t()
            ace = self.client.switch_api_acl_ip_rule_create(
                0, acl, 10, 1, kvp, action, action_params, opt_action_params)
            port = self.client.switch_api_port_id_to_handle_get(0, swports[1])
            self.client.switch_api_acl_reference(0, acl, port)

            # send a test pkt
            send_packet(self, swports[1], str(int_pkt))
            # verify mod packet as mod wins
            # dropped at ingress
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed INT + ingress MoD + transit disabled"

            # enable int-tr
            self.client.switch_api_dtel_int_transit_enable(device)
            exp_mod_pkt[DTEL_REPORT_HDR].path_tracking_flow = 1
            # send a test pkt
            send_packet(self, swports[1], str(int_pkt))
            # verify mod packet as mod wins
            # dropped at ingress
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed INT + ingress MoD + transit enabled"

            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_DROP_REPORT, 5)
            exp_mod_pkt[IP].tos = 5<<2
            # send a test pkt
            send_packet(self, swports[1], str(int_pkt))
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed INT + ingress MoD + DSCP"

            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS, 6)
            exp_mod_pkt[IP].tos = 6<<2
            # send a test pkt
            send_packet(self, swports[1], str(int_pkt))
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed INT + ingress MoD + DSCP"

            self.client.switch_api_dtel_report_udp_dstport_set(
                device, UDP_PORT_DTEL_REPORT^0x1111);
            exp_mod_pkt[UDP].dport = UDP_PORT_DTEL_REPORT^0x1111
            bind_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT^0x1111)
            split_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT)
            send_packet(self, swports[1], str(int_pkt))
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            self.client.switch_api_dtel_report_udp_dstport_set(
                device, UDP_PORT_DTEL_REPORT);
            split_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT^0x1111)
            bind_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT)
            print "Passed INT + ingress MoD + Report UDP port"

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            split_mirror_on_drop_pkt()
            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_DROP_REPORT, 0)
            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS, 0)
            self.client.switch_api_dtel_report_udp_dstport_set(
                device, UDP_PORT_DTEL_REPORT);
            split_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT^0x1111)
            bind_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT)

            # ip_acl
            self.client.switch_api_acl_dereference(0, acl, port)
            self.client.switch_api_acl_rule_delete(0, acl, ace)
            self.client.switch_api_acl_list_delete(0, acl)

            self.client.switch_api_dtel_int_transit_disable(device=device)
            self.client.switch_api_dtel_drop_report_disable(device)
            self.client.switch_api_dtel_drop_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            config.cleanup(self)

@group('transit_l45_dod')
class INTL45_Transit_DoDTest(api_base_tests.ThriftInterfaceDataPlane,
                            pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45 Transit device with deflect on drop"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        prepare_int_l45_bindings()
        bind_mirror_on_drop_pkt()
        config = SwitchConfig(self, params)
        twl_kvp = []

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        # Add MoD watchlist
        ap = switcht_twl_mod_params_t(report_queue_tail_drops=True)
        self.client.switch_api_dtel_drop_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        payload = 'int l45'
        # make input frame to inject to sink
        pkt = simple_udp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[1],
            ip_id=108,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=64,
            with_udp_chksum=False,
            udp_sport=101,
            udp_payload=payload)

        int_pkt_orig = int_l45_src_packet(
            test=self,
            int_inst_mask=0x8000,  # swid
            int_inst_cnt=1,
            max_hop_cnt=8,
            pkt=pkt)

        # add 2 hop info to the packet
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt_orig, val=0x66666666, incr_cnt=1)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222222, incr_cnt=1)

        exp_pkt = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_id=108,
            ip_ttl=63,
            with_udp_chksum=False,
            udp_sport=101,
            udp_payload=payload)

        exp_int_orig = int_l45_src_packet(
            test=self,
            int_inst_mask=0x8000,  # swid
            int_inst_cnt=1,
            max_hop_cnt=8,
            pkt=exp_pkt)

        # add 3 hop info to the packet
        exp_int_pkt = int_l45_packet_add_hop_info(
            Packet=exp_int_orig, val=0x66666666, incr_cnt=1)
        exp_int_pkt = int_l45_packet_add_hop_info(
            Packet=exp_int_pkt, val=0x22222222, incr_cnt=1)

        # mod report packet
        exp_mod_inner = mod_report(
            packet=int_pkt,
            switch_id=SID,
            ingress_port=swports[0],
            egress_port=swports[1],
            queue_id=0,
            drop_reason=71)  # drop traffic manager

        exp_dod_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_MOD,
            dropped=1,
            congested_queue=0,
            path_tracking_flow=0,
            hw_id=hw_id,
            inner_frame=exp_mod_inner)


        queue_report_enabled = False
        try:
            # config MoD
            self.client.switch_api_dtel_drop_report_enable(device)

            dtel_checkDoD(self, swports[0], swports[1],
                           swports[params.report_ports[0]],
                           int_pkt, exp_int_pkt,
                           True, exp_dod_pkt)
            print "Passed INT + DoD + transit disabled"

            # enable int-tr
            self.client.switch_api_dtel_int_transit_enable(device)
            exp_dod_pkt[DTEL_REPORT_HDR].path_tracking_flow = 1
            exp_int_pkt = int_l45_packet_add_hop_info(
                Packet=exp_int_pkt, val=SID, incr_cnt=1)

            dtel_checkDoD(self, swports[0], swports[1],
                           swports[params.report_ports[0]],
                           int_pkt, exp_int_pkt,
                           True, exp_dod_pkt)
            print "Passed INT + DoD + transit enabled"

            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_DROP_REPORT, 5)
            exp_dod_pkt[IP].tos = 5<<2
            dtel_checkDoD(self, swports[0], swports[1],
                           swports[params.report_ports[0]],
                           int_pkt, exp_int_pkt,
                           True, exp_dod_pkt)
            print "Passed INT + DoD + DSCP"

            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS, 6)
            exp_dod_pkt[IP].tos = 6<<2
            dtel_checkDoD(self, swports[0], swports[1],
                           swports[params.report_ports[0]],
                           int_pkt, exp_int_pkt,
                           True, exp_dod_pkt)
            print "Passed INT + DoD + Flow DSCP"

            exp_inte2e_inner_1 = postcard_report(
                packet=exp_int_pkt,
                switch_id=SID,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_q_pkt = ipv4_dtel_pkt(
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

            self.client.switch_api_dtel_queue_report_create(
                device, swports[1], 0, 0, 0, 1024, True)
            queue_report_enabled = True
            exp_dod_pkt[DTEL_REPORT_HDR].congested_queue = 1
            dtel_checkDoD(self, swports[0], swports[1],
                           swports[params.report_ports[0]],
                           int_pkt, exp_int_pkt,
                           True, exp_dod_pkt, exp_e2e_pkt=exp_q_pkt)
            print "Passed INT + DoD + QDoD"

            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP, 7)
            exp_dod_pkt[IP].tos = 7<<2
            dtel_checkDoD(self, swports[0], swports[1],
                           swports[params.report_ports[0]],
                           int_pkt, exp_int_pkt,
                           True, exp_dod_pkt, exp_e2e_pkt=exp_q_pkt)
            print "Passed INT + DoD + QDoD + DSCP"


        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            split_mirror_on_drop_pkt()
            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_DROP_REPORT, 0)
            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS, 0)
            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP, 0)
            if queue_report_enabled:
                self.client.switch_api_dtel_queue_report_delete(
                    device, swports[1], 0)

            self.client.switch_api_dtel_int_transit_disable(device=device)
            self.client.switch_api_dtel_drop_report_disable(device)
            self.client.switch_api_dtel_drop_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            config.cleanup(self)

@group('transit_l45')
@group('transit_l45_no_suppression')
class INTL45_Marker_TransitTest(api_base_tests.ThriftInterfaceDataPlane,
                          pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45 Transit device generating DTel report"
        print "handling Marker"
        if get_int_l45_encap() != "marker":
            print "Not running with INT L45 encap using marker"
            print "Skipping this test"
            return
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)
        # Enable INT transit processing
        self.client.switch_api_dtel_int_transit_enable(device)

        payload = 'int l45'
        # make input frame to inject to sink
        pkt = simple_udp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_id=108,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=64,
            with_udp_chksum=False,
            udp_sport=101,
            udp_payload=payload)

        int_pkt_orig = int_l45_src_packet(
            test=self,
            int_inst_mask=0x8000,  # swid
            int_inst_cnt=1,
            max_hop_cnt=8,
            pkt=pkt)

        # add 2 hop info to the packet
        int_pkt = int_pkt_orig
        for i in range(2):
          int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222222, incr_cnt=1)

        exp_pkt = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            with_udp_chksum=False,
            ip_id=108,
            ip_ttl=63,
            udp_sport=101,
            udp_payload=payload)

        exp_pkt = int_l45_src_packet(
            test=self,
            int_inst_mask=0x8000,  # only swid, 1 byte
            max_hop_cnt=8,
            int_inst_cnt=1,
            pkt=exp_pkt)
        for i in range(2):
          exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x22222222, incr_cnt=1)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=params.switch_id, incr_cnt=1)

        try:
            # doesn't remove/add headers if port is not matching or not set
            int_pkt[UDP].dport = 81
            exp_pkt_ = int_pkt.copy()
            exp_pkt_[IP].ttl-=1
            exp_pkt_[Ether].src=exp_pkt[Ether].src
            exp_pkt_[Ether].dst=exp_pkt[Ether].dst
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt_, swports[1])
            verify_no_other_packets(self)
            print "pass UDP marker port is not configured"

            # now add the port
            self.client.switch_api_dtel_int_marker_port_add(
               device, 17, 81, hex_to_i16(0xffff))
            exp_pkt[UDP].dport=81
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass UDP marker port is configured"

            set_int_l45_marker(0xdeadbeefdeadbeef, 17)
            self.client.switch_api_dtel_int_marker_set(
                device, 17, hex_to_i64(0xdeadbeefdeadbeef))
            int_pkt[INTL45_MARKER].marker=0xdeadbeefdeadbeef
            exp_pkt[INTL45_MARKER].marker=0xdeadbeefdeadbeef
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)

            exp_pkt[UDP].dport=80
            int_pkt[UDP].dport=80
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass marker changes for all added ports"

            set_int_l45_marker(0xabcdabcdabcdabcd, 6)
            self.client.switch_api_dtel_int_marker_set(
                device, 6, hex_to_i64(0xabcdabcdabcdabcd))
            exp_pkt[UDP].dport=80
            int_pkt[UDP].dport=80
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass marker change for TCP doesn't affect UDP"

            self.client.switch_api_dtel_int_marker_delete(
                device, 6)
            exp_pkt[UDP].dport=80
            int_pkt[UDP].dport=80
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass disabling Marker for TCP doesn't affect UDP"

            self.client.switch_api_dtel_int_marker_port_delete(
                device, 17, 81, hex_to_i16(0xffff))
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)

            int_pkt[UDP].dport = 81
            exp_pkt_[UDP].dport = 81
            exp_pkt_[INTL45_MARKER].marker=0xdeadbeefdeadbeef
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt_, swports[1])
            verify_no_other_packets(self)
            print "pass removing port works and doesn't affect another port"

            self.client.switch_api_dtel_int_marker_port_add(
               device, 17, 81, hex_to_i16(0xffff))
            int_pkt[UDP].dport = 81
            exp_pkt[UDP].dport = 81
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass add port back after delete works"

            self.client.switch_api_dtel_int_marker_port_clear(device, 6)
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass clear TCP doesn't affect UDP"

            self.client.switch_api_dtel_int_marker_port_clear(device, 17)
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt_, swports[1])
            verify_no_other_packets(self)

            int_pkt[UDP].dport = 80
            exp_pkt_[UDP].dport = 80
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt_, swports[1])
            verify_no_other_packets(self)
            print "pass clear ports removes all"

            self.client.switch_api_dtel_int_marker_port_add(
               device, 17, 80, hex_to_i16(0xfffe))
            int_pkt[UDP].dport = 80
            exp_pkt[UDP].dport = 80
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)

            int_pkt[UDP].dport = 81
            exp_pkt[UDP].dport = 81
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass port add with mask works"

            self.client.switch_api_dtel_int_marker_delete(
                device, 17)
            int_pkt[UDP].dport = 80
            exp_pkt_[UDP].dport = 80
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt_, swports[1])
            verify_no_other_packets(self)
            print "pass disable UDP marker works"

            set_int_l45_marker(INT_L45_MARKER, 6)
            self.client.switch_api_dtel_int_marker_set(
                device, 6, hex_to_i64(INT_L45_MARKER))
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt_, swports[1])
            verify_no_other_packets(self)
            print "pass enable for TCP has no effect on UDP"

            self.client.switch_api_dtel_int_marker_port_add(
               device, 17, 900, hex_to_i16(0xffff))
            self.client.switch_api_dtel_int_marker_port_add(
               device, 17, 901, hex_to_i16(0xffff))
            set_int_l45_marker(INT_L45_MARKER, 17)
            self.client.switch_api_dtel_int_marker_set(
                device, 17, hex_to_i64(INT_L45_MARKER))
            int_pkt[INTL45_MARKER].marker=INT_L45_MARKER
            exp_pkt_[INTL45_MARKER].marker=INT_L45_MARKER
            int_pkt[UDP].dport = 80
            exp_pkt_[UDP].dport = 80
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt_, swports[1])
            verify_no_other_packets(self)
            print "pass disable UDP marker also clears UDP marker ports"

            int_pkt[UDP].dport = 900
            exp_pkt[UDP].dport = 900
            exp_pkt[INTL45_MARKER].marker=INT_L45_MARKER
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            int_pkt[UDP].dport = 901
            exp_pkt[UDP].dport = 901
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass enable UDP marker after adding ports works for multiple port entries"

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_transit_disable(device)
            config.cleanup(self)

