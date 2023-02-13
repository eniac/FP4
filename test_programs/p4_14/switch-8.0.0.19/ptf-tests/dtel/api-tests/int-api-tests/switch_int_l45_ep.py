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
INT L45 source and sink endpoint tests, without Digest.
"""

import switchapi_thrift

import time
import sys
import logging

import unittest
import random
from copy import deepcopy

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *
import ptf.mask

import os

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
SID = 0x0ACEFACE
quantization_shift = 0

swports = [0, 1, 2]

params = SwitchConfig_Params()
params.switch_id = SID
params.mac_self = '00:77:66:55:44:33'
params.nports = 3
params.ipaddr_inf = ['2.2.0.1',  '1.1.0.1', '10.1.0.4']
params.ipaddr_nbr = ['2.2.0.200', '1.1.0.100', '10.1.0.1']
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

###############################################################################
@group('simple')
@group('vlan')
@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_Sink_Over_VLAN_Test(api_base_tests.ThriftInterfaceDataPlane,
                                 pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print
        print "Test INT L45 Sink generating DTel report over VLAN port"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        prepare_int_l45_bindings()

        vlan_id = 10
        # set DTEL report port as vlan port
        params.vlans = {vlan_id: {params.report_ports[0]:True}}
        config = SwitchConfig(self, params)

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)

        # Add INT watchlist entry
        # session_id = 1, report_all_packets = true (no digest)
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=True, flow_sample_percent=100)
        self.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        # add INT sink port
        self.client.switch_api_dtel_int_sink_ports_add(
            device=device, port=swports[1])

        int_enabled = False
        try:
            pkt_in = simple_tcp_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[params.report_ports[0]],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                ip_ttl=64,
                tcp_flags=None,
                pktlen=128)

            exp_pkt_out = simple_tcp_packet(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[params.report_ports[0]],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                tcp_flags=None,
                ip_ttl=63,
                dl_vlan_enable=True,
                vlan_vid=vlan_id,
                pktlen=132)

            send_packet(self, swports[0], str(pkt_in))
            verify_packet(self, exp_pkt_out, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass the test pkt to the vlan report port"

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
                pktlen=128)

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

            # upstream report packet
            exp_i2e_pkt = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=int_pkt)

            exp_pkt = simple_udp_packet(
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                with_udp_chksum=False,
                ip_id=108,
                ip_ttl=63,
                udp_sport=101,
                pktlen=128)

            exp_inte2e_inner = postcard_report(
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
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=exp_inte2e_inner)

            exp_pkt_ = int_pkt.copy()
            exp_pkt_[IP].ttl-=1
            exp_pkt_[Ether].src=exp_pkt[Ether].src
            exp_pkt_[Ether].dst=exp_pkt[Ether].dst

            # enable int-ep
            self.client.switch_api_dtel_int_endpoint_enable(device)
            int_enabled = True
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            #receive_print_packet(self, swports[1], exp_pkt, True)
            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass packet w/ INT enabled"

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            if int_enabled:
                self.client.switch_api_dtel_int_endpoint_disable(device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            self.client.switch_api_dtel_int_sink_ports_delete(
                device=device, port=swports[1])
            config.cleanup(self)
            params.vlans=None

@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_UDP_SourceTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 UDP Source device - not VTEP-src, just INT-src"
        prepare_int_l45_bindings()
        p = params.report_ports;
        params.report_ports=None
        config = SwitchConfig(self, params)

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)

        # Add INT watchlist entry
        # session_id = 1, report_all_packets = true (no digest)
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=True, flow_sample_percent=100)
        self.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        payload = 'int_l45'
        pkt = simple_udp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_id=108,
            ip_ttl=64,
            with_udp_chksum=False,
            udp_payload=payload)

        exp_pkt_ = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            with_udp_chksum=False,
            ip_id=108,
            ip_ttl=63,
            udp_payload=payload)

        exp_pkt = int_l45_src_packet(
            test=self,
            int_inst_mask=0x8000,
            int_inst_cnt=1,
            max_hop_cnt=8,
            pkt=exp_pkt_)

        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=SID, incr_cnt=1)
        int_enabled = False

        try:
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt_, swports[1])
            verify_no_other_packets(self)
            print "pass 1st packet w/ INT disabled"

            self.client.switch_api_dtel_int_endpoint_enable(device)
            int_enabled = True
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass 2nd packet w/ INT enabled"

            self.client.switch_api_dtel_int_endpoint_disable(device)
            int_enabled = False
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt_, swports[1])
            verify_no_other_packets(self)
            print "pass 3rd packet w/ INT disabled"

            self.client.switch_api_dtel_int_endpoint_enable(device)
            int_enabled = True
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass 4th packet w/ INT enabled"

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            if int_enabled:
                self.client.switch_api_dtel_int_endpoint_disable(device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            config.cleanup(self)
            params.report_ports = p

@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_UDP_Source_WL_DSCP_Test(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 UDP Source device - watchlist matching on DSCP"
        p = params.report_ports;
        params.report_ports=None
        config = SwitchConfig(self, params)

        INT_DSCP = 0x10
        INT_DSCP_MASK=0x10
        cleanup_int_l45_bindings()
        prepare_int_l45_bindings(int_dscp=INT_DSCP, int_dscp_mask=INT_DSCP_MASK)
        self.client.switch_api_dtel_int_dscp_value_set(
            device,
            INT_DSCP,
            INT_DSCP_MASK);

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)

        # Add INT watchlist entry
        # session_id = 1, report_all_packets = true (no digest)
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=True, flow_sample_percent=100)

        # create a copy of original watchlist and set DSCP
        WL_DSCP_VAL = 0x01
        WL_DSCP_MASK = 0x01
        twl_kvp_dscp = deepcopy(twl_kvp)
        kvp_val = switcht_twl_value_t(value_num=WL_DSCP_VAL)
        kvp_mask = switcht_twl_value_t(value_num=WL_DSCP_MASK)
        twl_kvp_dscp.append(switcht_twl_key_value_pair_t(
            SWITCH_TWL_FIELD_DSCP, kvp_val, kvp_mask))

        self.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp_dscp, priority=1, watch=True, action_params=ap)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        payload = 'int_l45'
        pkt = simple_udp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_id=108,
            ip_ttl=64,
            ip_tos=WL_DSCP_VAL<<2,
            with_udp_chksum=False,
            udp_payload=payload)

        exp_pkt_ = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            with_udp_chksum=False,
            ip_id=108,
            ip_ttl=63,
            ip_tos=WL_DSCP_VAL<<2,
            udp_payload=payload)

        exp_pkt = int_l45_src_packet(
            test=self,
            int_inst_mask=0x8000,
            int_inst_cnt=1,
            max_hop_cnt=8,
            dscp=INT_DSCP,
            dscp_mask=INT_DSCP_MASK,
            pkt=exp_pkt_)

        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=SID, incr_cnt=1)
        int_enabled = False

        try:
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt_, swports[1])
            verify_no_other_packets(self)
            print "pass 1st packet w/ INT disabled"

            self.client.switch_api_dtel_int_endpoint_enable(device)
            int_enabled = True
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass 2nd packet w/ INT enabled"

            self.client.switch_api_dtel_int_endpoint_disable(device)
            int_enabled = False
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt_, swports[1])
            verify_no_other_packets(self)
            print "pass 3rd packet w/ INT disabled"

            self.client.switch_api_dtel_int_endpoint_enable(device)
            int_enabled = True
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass 4th packet w/ INT enabled"

            nomatch_tos = 0xFC - ((WL_DSCP_VAL | INT_DSCP) << 2)
            pkt[IP].tos = nomatch_tos
            exp_pkt_[IP].tos = nomatch_tos
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt_, swports[1])
            verify_no_other_packets(self)
            print "pass 5th packet w/ DSCP no match"

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            if int_enabled:
                self.client.switch_api_dtel_int_endpoint_disable(device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp_dscp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            config.cleanup(self)
            params.report_ports = p

@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_UDP_SinkTest(api_base_tests.ThriftInterfaceDataPlane,
                          pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45 UDP Sink device generating DTel report"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)

        # Add INT watchlist entry
        # session_id = 1, report_all_packets = true (no digest)
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=True, flow_sample_percent=100)
        self.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        # add INT sink port
        self.client.switch_api_dtel_int_sink_ports_add(
            device=device, port=swports[1])

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

        # upstream report packet
        exp_i2e_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
            dropped=0,
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=int_pkt)

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
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=exp_inte2e_inner_1)

        int_enabled = False

        exp_pkt_ = int_pkt.copy()
        exp_pkt_[IP].ttl-=1
        exp_pkt_[Ether].src=exp_pkt[Ether].src
        exp_pkt_[Ether].dst=exp_pkt[Ether].dst

        try:
            # send a test pkt
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt_, swports[1])
            verify_no_other_packets(self)
            print "pass 1st packet w/ INT disabled"

            # enable int-ep
            self.client.switch_api_dtel_int_endpoint_enable(device)
            int_enabled = True
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass 2nd packet w/ INT enabled"

            self.client.switch_api_dtel_int_endpoint_disable(device)
            int_enabled = False
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt_, swports[1])
            verify_no_other_packets(self)
            print "pass 3rd packet w/ INT disabled"

            # enable int-ep
            self.client.switch_api_dtel_int_endpoint_enable(device)
            int_enabled = True
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass 4th packet w/ INT enabled"

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            if int_enabled:
                self.client.switch_api_dtel_int_endpoint_disable(device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            self.client.switch_api_dtel_int_sink_ports_delete(
                device=device, port=swports[1])
            config.cleanup(self)


@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_TCP_SourceTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 TCP Source device - not VTEP-src, just INT-src"
        prepare_int_l45_bindings()
        p = params.report_ports;
        params.report_ports=None
        config = SwitchConfig(self, params)

        pkt = simple_tcp_packet(
            eth_dst=params.mac_self,
            tcp_flags= None,
            eth_src=params.mac_nbr[0],
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_id=108,
            ip_ttl=64)

        exp_pkt = simple_tcp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            tcp_flags= None,
            ip_id=108,
            ip_ttl=63)

        exp_pkt = int_l45_src_packet(
            test=self,
            int_inst_mask=0x8000,
            int_inst_cnt=1,
            max_hop_cnt=8,
            pkt=exp_pkt)

        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=SID, incr_cnt=1)

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)

        # Add INT watchlist entry
        # session_id = 1, report_all_packets = true (no digest)
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=True, flow_sample_percent=100)
        self.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        # enable int-ep
        self.client.switch_api_dtel_int_endpoint_enable(device=device)

        m = Mask(exp_pkt)
        if exp_pkt.haslayer(TCP_INTL45):
            m.set_do_not_care_scapy(TCP_INTL45, 'chksum')
        else:
            m.set_do_not_care_scapy(TCP, 'chksum')

        try:
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, m, swports[1])
            verify_no_other_packets(self)

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_endpoint_disable(device=device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            config.cleanup(self)
            params.report_ports = p


@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_TCP_SinkTest(api_base_tests.ThriftInterfaceDataPlane,
                          pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45 TCP Sink device generating DTel report"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)

        # Add INT watchlist entry
        # session_id = 1, report_all_packets = true (no digest)
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=True, flow_sample_percent=100)
        self.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        # enable int-ep
        self.client.switch_api_dtel_int_endpoint_enable(device=device)

        # add INT sink port
        self.client.switch_api_dtel_int_sink_ports_add(
            device=device, port=swports[1])
        # make input frame to inject to sink
        pkt = simple_tcp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_id=108,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            tcp_flags= None,
            ip_ttl=64,
            tcp_sport=101)
        # force checksum calculation
        #del pkt[TCP].chksum
        #pkt[TCP] = pkt[TCP].__class__(str(pkt[TCP]))
        #chksum = pkt[TCP].chksum


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

        # bring back the checksum
        #int_pkt[TCP_INTL45].chksum = chksum

        # upstream report packet
        exp_i2e_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
            dropped=0,
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=int_pkt)

        exp_pkt = simple_tcp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            tcp_flags= None,
            ip_id=108,
            ip_ttl=63,
            tcp_sport=101)

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
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=exp_inte2e_inner_1)


        try:
            m = Mask(exp_pkt)
            m.set_do_not_care_scapy(TCP, 'chksum')

            # send a test pkt
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, m, swports[1])

            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)

            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)

            verify_no_other_packets(self)

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_endpoint_disable(device=device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            self.client.switch_api_dtel_int_sink_ports_delete(
                device=device, port=swports[1])
            config.cleanup(self)

@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_ICMP_SourceTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 ICMP Source device - not VTEP-src, just INT-src"
        prepare_int_l45_bindings()
        p = params.report_ports;
        params.report_ports=None
        config = SwitchConfig(self, params)

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)

        # Add INT watchlist entry
        # session_id = 1, report_all_packets = true (no digest)
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=True, flow_sample_percent=100)
        self.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        # enable int-ep
        self.client.switch_api_dtel_int_endpoint_enable(device=device)

        pkt = simple_icmp_packet(
            pktlen=64,
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_id=108,
            ip_ttl=64)

        exp_pkt = simple_icmp_packet(
            pktlen=64,
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_id=108,
            ip_ttl=63)

        exp_pkt = int_l45_src_packet(
            test=self,
            int_inst_mask=0x8000,
            int_inst_cnt=1,
            max_hop_cnt=8,
            pkt=exp_pkt)

        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=SID, incr_cnt=1)

        m = Mask(exp_pkt)
        if exp_pkt.haslayer(ICMP_INTL45):
            m.set_do_not_care_scapy(ICMP_INTL45, 'chksum')
        else:
            m.set_do_not_care_scapy(ICMP, 'chksum')

        try:
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, m, swports[1])
            verify_no_other_packets(self)

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_endpoint_disable(device=device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            config.cleanup(self)
            params.report_ports = p


@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_ICMP_SinkTest(api_base_tests.ThriftInterfaceDataPlane,
                           pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45 ICMP Sink device generating DTel report"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)

        # Add INT watchlist entry
        # session_id = 1, report_all_packets = true (no digest)
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=True, flow_sample_percent=100)
        self.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        # enable int-ep
        self.client.switch_api_dtel_int_endpoint_enable(device=device)

        # add INT sink port
        self.client.switch_api_dtel_int_sink_ports_add(
            device=device, port=swports[1])

        # make input frame to inject to sink
        pkt = simple_icmp_packet(
            pktlen=64,
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_id=108,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=64)


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

        # bring back the checksum
        if int_pkt.haslayer(ICMP_INTL45):
            del int_pkt[ICMP_INTL45].chksum
        else:
            del int_pkt[ICMP].chksum

        # upstream report packet
        exp_i2e_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
            dropped=0,
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=int_pkt)

        exp_pkt = simple_icmp_packet(
            pktlen=64,
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_id=108,
            ip_ttl=63)

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
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=exp_inte2e_inner_1)

        try:
            m = Mask(exp_pkt)
            m.set_do_not_care_scapy(ICMP, 'chksum')
            # send a test pkt
            send_packet(self, swports[0], str(int_pkt))

            verify_packet(self, m, swports[1])

            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)

            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)

            verify_no_other_packets(self)

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_endpoint_disable(device=device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            self.client.switch_api_dtel_int_sink_ports_delete(
                device=device, port=swports[1])
            config.cleanup(self)

@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_Encap_SourceTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 UDP Source - just INT-src on encapsulated packet"
        prepare_int_l45_bindings()
        p = params.report_ports;
        params.report_ports=None
        config = SwitchConfig(self, params)

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)

        # Add INT watchlist entry
        # session_id = 1, report_all_packets = true (no digest)
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=True, flow_sample_percent=100)
        self.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        # enable int-ep
        self.client.switch_api_dtel_int_endpoint_enable(device=device)

        pkt = simple_tcp_packet(
            eth_src='00:11:11:11:11:11',
            eth_dst='00:33:33:33:33:33',
            ip_dst='1.1.1.100',
            ip_src='2.2.2.200',
            tcp_flags= None,
            ip_id=108,
            ip_ttl=64)

        vxlan_pkt = simple_vxlan_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_id=0,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=64,
            udp_sport=101,
            vxlan_vni=0xaaaa,
            with_udp_chksum=False,
            inner_frame=pkt)

        exp_pkt = simple_vxlan_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_id=0,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=63,
            udp_sport=101,
            vxlan_vni=0xaaaa,
            with_udp_chksum=False,
            inner_frame=pkt)

        exp_pkt = int_l45_src_packet(
            test=self,
            int_inst_mask=0x8000,
            int_inst_cnt=1,
            max_hop_cnt=8,
            pkt=exp_pkt)

        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=SID, incr_cnt=1)

        try:

            send_packet(self, swports[0], str(vxlan_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_endpoint_disable(device=device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            config.cleanup(self)
            params.report_ports = p


@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_Encap_SinkTest(api_base_tests.ThriftInterfaceDataPlane,
                            pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45 UDP Sink generating DTel report "
        print "- INT termination only, for encapsulated packet"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)

        # Add INT watchlist entry
        # session_id = 1, report_all_packets = true (no digest)
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=True, flow_sample_percent=100)
        self.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        # enable int-ep
        self.client.switch_api_dtel_int_endpoint_enable(device=device)

        # add INT sink port
        self.client.switch_api_dtel_int_sink_ports_add(
            device=device, port=swports[1])

        pkt = simple_tcp_packet(
            eth_src='00:11:11:11:11:11',
            eth_dst='00:33:33:33:33:33',
            ip_dst='1.1.1.111',
            ip_src='2.2.2.2',
            tcp_flags= None,
            ip_id=108,
            ip_ttl=64)

        vxlan_pkt = simple_vxlan_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_id=0,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=64,
            udp_sport=101,
            vxlan_vni=0xaaaa,
            with_udp_chksum=False,
            inner_frame=pkt)

        int_pkt_orig = int_l45_src_packet(
            test=self,
            int_inst_mask=0x8000,
            int_inst_cnt=1,
            max_hop_cnt=8,
            pkt=vxlan_pkt)

        # add 2 hop info to the packet
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt_orig, val=0x66666666, incr_cnt=1)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222222, incr_cnt=1)

        # upstream report packet
        exp_i2e_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
            dropped=0,
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=int_pkt)

        exp_pkt = simple_vxlan_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_id=0,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=63,
            udp_sport=101,
            vxlan_vni=0xaaaa,
            with_udp_chksum=False,
            inner_frame=pkt)

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
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=exp_inte2e_inner_1)

        try:
            self.client.switch_api_dtel_int_marker_port_add(
               device, 17, vxlan_pkt.getlayer(UDP,1).dport, hex_to_i16(0xffff))

            # send a test pkt
            send_packet(self, swports[0], str(int_pkt))

            verify_packet(self, exp_pkt, swports[1])

            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])

            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])

            verify_no_other_packets(self)

        finally:
            ### Cleanup
            self.client.switch_api_dtel_int_marker_port_delete(
                device, 17, vxlan_pkt.getlayer(UDP,1).dport, hex_to_i16(0xffff))
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_endpoint_disable(device=device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            self.client.switch_api_dtel_int_sink_ports_delete(
                device=device, port=swports[1])
            config.cleanup(self)


@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_EgressMoDTest(api_base_tests.ThriftInterfaceDataPlane,
                           pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT Sink device with mirror on drop at egress"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        prepare_int_l45_bindings()
        bind_mirror_on_drop_pkt()
        config = SwitchConfig(self, params)

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)

        # Add INT watchlist entry
        # session_id = 1, report_all_packets = true (no digest)
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=True, flow_sample_percent=100)
        self.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # set quantization shift to max to avoid change detection mess quota
        # tests
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=MAX_QUANTIZATION)

        # add INT sink port
        self.client.switch_api_dtel_int_sink_ports_add(
            device=device, port=swports[1])

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

        # upstream report packet
        exp_i2e_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
            dropped=0,
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=int_pkt)

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
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=exp_inte2e_inner_1)

        exp_mod_inner_1 = mod_report(
            packet=exp_pkt,
            switch_id=SID,
            ingress_port=swports[0],
            egress_port=swports[1],
            queue_id=0,
            drop_reason=92)  # drop egress ACL deny

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

        exp_pkt_ = int_pkt.copy()
        exp_pkt_[IP].ttl-=1
        exp_pkt_[Ether].src=exp_pkt[Ether].src
        exp_pkt_[Ether].dst=exp_pkt[Ether].dst

        exp_mod_inner_1 = mod_report(
            packet=exp_pkt_,
            switch_id=SID,
            ingress_port=swports[0],
            egress_port=swports[1],
            queue_id=0,
            drop_reason=92)  # drop egress ACL deny

        exp_mod_pkt_ = ipv4_dtel_pkt(
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
        ep_enabled = False
        acl_enabled = False

        try:
            # config MoD
            self.client.switch_api_dtel_drop_report_enable(device)

            # send a test pkt, should pass
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt_, swports[1])
            verify_no_other_packets(self)

            # create egress system ACL on port 1 to deny packets
            acl = self.client.switch_api_acl_list_create(
                device, SWITCH_API_DIRECTION_EGRESS,
                SWITCH_ACL_TYPE_EGRESS_SYSTEM, SWITCH_HANDLE_TYPE_PORT)
            port = self.client.switch_api_port_id_to_handle_get(
                device, swports[1])
            acl_kvp = []
            acl_kvp_val = switcht_acl_value_t(value_num=port)
            acl_kvp_mask = switcht_acl_value_t(value_num=0xff)
            acl_kvp.append(switcht_acl_key_value_pair_t(
                SWITCH_ACL_EGRESS_SYSTEM_FIELD_DEST_PORT, acl_kvp_val, acl_kvp_mask))
            action = SWITCH_ACL_EGRESS_SYSTEM_ACTION_DROP
            action_params = switcht_acl_action_params_t(
                drop=switcht_acl_action_drop(reason_code=92)) # egress acl deny
            opt_action_params = switcht_acl_opt_action_params_t()
            ace = self.client.switch_api_acl_egress_system_rule_create(
                device, acl, 10, 1, acl_kvp, action, action_params,
                opt_action_params)
            acl_enabled = True

            # send a test pkt
            send_packet(self, swports[0], str(int_pkt))
            # packet is dropped
            #verify_packet(self, exp_pkt, swports[1])

            # verify mod packet as mod wins
            verify_dtel_packet(
                self, exp_mod_pkt_, swports[params.report_ports[0]])

            verify_no_other_packets(self)
            print "Passed for int egress + MOD + endpoint disabled"

            exp_mod_pkt[DTEL_REPORT_HDR].path_tracking_flow = 1
            # enable int-ep
            self.client.switch_api_dtel_int_endpoint_enable(device=device)
            ep_enabled = True

            # send a test pkt
            send_packet(self, swports[0], str(int_pkt))
            # packet is dropped
            #verify_packet(self, exp_pkt, swports[1])

            # verify i2e mirrored packet
            # dropped at egress
            verify_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
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

            # verify i2e mirrored packet
            # dropped at egress
            verify_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
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
            exp_i2e_pkt[UDP].dport = UDP_PORT_DTEL_REPORT^0x1111
            exp_mod_pkt[UDP].dport = UDP_PORT_DTEL_REPORT^0x1111
            send_packet(self, swports[0], str(int_pkt))
            # packet is dropped
            #verify_packet(self, exp_pkt, swports[1])

            # verify i2e mirrored packet
            # dropped at egress
            verify_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
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
            exp_i2e_pkt[UDP].dport = UDP_PORT_DTEL_REPORT
            exp_mod_pkt[UDP].dport = UDP_PORT_DTEL_REPORT
            print "Passed for int egress + MOD + Report UDP port"


            self.client.switch_api_dtel_queue_report_create(
                device, swports[1], 0, 0, 0, 1, False)
            queue_report_enabled = True

            exp_mod_pkt[DTEL_REPORT_HDR].congested_queue = 1
            send_packet(self, swports[0], str(int_pkt))
            # dropped at egress
            verify_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
            # verify mod packet as mod wins
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)

            # quota finished now
            exp_mod_pkt[DTEL_REPORT_HDR].congested_queue = 0
            send_packet(self, swports[0], str(int_pkt))
            # dropped at egress
            verify_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
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
            if ep_enabled:
                self.client.switch_api_dtel_int_endpoint_disable(device=device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            self.client.switch_api_dtel_int_sink_ports_delete(
                device=device, port=swports[1])
            self.client.switch_api_dtel_drop_report_disable(device)
            self.client.switch_api_dtel_drop_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            if acl_enabled:
                self.client.switch_api_acl_rule_delete(device, acl, ace)
                self.client.switch_api_acl_list_delete(device, acl)
            config.cleanup(self)

@group('ep_l45_dod')
class INTL45_DoDTest(api_base_tests.ThriftInterfaceDataPlane,
                           pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])
    def run_test_(self, pkt_in, exp_pkt, exp_i2e_pkt, exp_e2e_pkt,
                  exp_q_pkt, exp_dod_pkt):
        dtel_checkDoD(self, swports[0], swports[1],
                           swports[params.report_ports[0]],
                           pkt_in, exp_pkt, drop=False,
                           exp_i2e_pkt=exp_i2e_pkt, exp_e2e_pkt=exp_e2e_pkt,
                           INT=True)
        print "     Passed for int egress + without DoD"

        ap = switcht_twl_mod_params_t(report_queue_tail_drops=True)
        self.client.switch_api_dtel_drop_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)
        self.mod_watchlist=True
        dtel_checkDoD(self, swports[0], swports[1],
                           swports[params.report_ports[0]],
                           pkt_in, exp_pkt,
                           True, exp_dod_pkt,
                           exp_i2e_pkt=exp_i2e_pkt, exp_e2e_pkt=exp_e2e_pkt,
                           INT=True)
        print "     Passed for int egress + DOD"

        self.client.switch_api_dtel_event_set_dscp(
            device, SWITCH_DTEL_EVENT_TYPE_DROP_REPORT, 8)
        self.assertTrue(self.client.switch_api_dtel_event_get_dscp(
            device, SWITCH_DTEL_EVENT_TYPE_DROP_REPORT)==8)
        exp_dod_pkt[IP].tos = 8<<2
        dtel_checkDoD(self, swports[0], swports[1],
                           swports[params.report_ports[0]],
                           pkt_in, exp_pkt,
                           True, exp_dod_pkt,
                           exp_i2e_pkt=exp_i2e_pkt, exp_e2e_pkt=exp_e2e_pkt,
                           INT=True)
        print "     Passed for int egress + DOD + new DSCP"

        self.client.switch_api_dtel_queue_report_create(
            device, swports[1], 0, 0, 0, 1024, False)
        self.queue_report_enabled = True
        dtel_checkDoD(self, swports[0], swports[1],
                           swports[params.report_ports[0]],
                           pkt_in, exp_pkt,
                           True, exp_dod_pkt,
                           exp_i2e_pkt=exp_i2e_pkt, exp_e2e_pkt=exp_q_pkt,
                           INT=True)
        print "     Passed for int egress + DOD + Q"

        self.client.switch_api_dtel_queue_report_update(
            device, swports[1], 0, 0, 0, 1024, True)
        exp_dod_pkt[DTEL_REPORT_HDR].congested_queue = 1
        dtel_checkDoD(self, swports[0], swports[1],
                           swports[params.report_ports[0]],
                           pkt_in, exp_pkt,
                           True, exp_dod_pkt,
                           exp_i2e_pkt=exp_i2e_pkt, exp_e2e_pkt=exp_q_pkt,
                           INT=True)
        print "     Passed for int egress + DOD + QDoD"

        self.client.switch_api_dtel_event_set_dscp(
            device, SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP, 6)
        self.assertTrue(self.client.switch_api_dtel_event_get_dscp(
            device, SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP)==6)
        dtel_checkDoD(self, swports[0], swports[1],
                           swports[params.report_ports[0]],
                           pkt_in, exp_pkt, True, exp_dod_pkt,
                           exp_i2e_pkt=exp_i2e_pkt, exp_e2e_pkt=exp_q_pkt,
                           INT=True)
        print "     Passed for int egress + DOD + QDoD + new DSCP"

        ap = switcht_twl_mod_params_t(report_queue_tail_drops=False)
        self.client.switch_api_dtel_drop_watchlist_entry_update(
            device, twl_kvp, priority=1, watch=True, action_params=ap)
        # no dod in drop watchlist thus get dscp of q_tail_drop
        exp_dod_pkt[IP].tos = 6<<2
        dtel_checkDoD(self, swports[0], swports[1],
                           swports[params.report_ports[0]],
                           pkt_in, exp_pkt,
                           True, exp_dod_pkt,
                           exp_i2e_pkt=exp_i2e_pkt, exp_e2e_pkt=exp_q_pkt,
                           INT=True)
        print "     Passed for int egress + QDoD + new DSCP"


        self.client.switch_api_dtel_drop_watchlist_entry_delete(
          device=device, twl_kvp=twl_kvp)
        self.mod_watchlist=False
        exp_dod_pkt[IP].tos = 6<<2
        dtel_checkDoD(self, swports[0], swports[1],
                           swports[params.report_ports[0]],
                           pkt_in, exp_pkt,
                           True, exp_dod_pkt,
                           exp_i2e_pkt=exp_i2e_pkt, exp_e2e_pkt=exp_q_pkt,
                           INT=True)
        print "     Passed for int egress + QDoD + new DSCP"

        self.client.switch_api_dtel_queue_report_delete(
            device, swports[1], 0)
        self.queue_report_enabled = False
        self.client.switch_api_dtel_event_set_dscp(
            device, SWITCH_DTEL_EVENT_TYPE_DROP_REPORT, 0)
        self.client.switch_api_dtel_event_set_dscp(
            device, SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP, 0)
        exp_dod_pkt[IP].tos = 0
        exp_dod_pkt[DTEL_REPORT_HDR].congested_queue = 0

    def runTest(self):
        print "     Test INT Sink device with dod"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        prepare_int_l45_bindings()
        bind_mirror_on_drop_pkt()
        config = SwitchConfig(self, params)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)


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

        # upstream report packet
        exp_i2e_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
            dropped=0,
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=int_pkt)

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

        exp_int_pkt = int_l45_src_packet(
            test=self,
            int_inst_mask=0x8000,
            int_inst_cnt=1,
            max_hop_cnt=8,
            pkt=exp_pkt)

        exp_int_pkt = int_l45_packet_add_hop_info(
            Packet=exp_int_pkt, val=SID, incr_cnt=1)

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
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=exp_inte2e_inner_1)

        exp_mod_inner_1 = mod_report(
            packet=pkt,
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
            inner_frame=exp_mod_inner_1)

        exp_q_pkt = exp_e2e_pkt.copy()
        exp_q_pkt[DTEL_REPORT_HDR].congested_queue = 1
        exp_q_pkt[DTEL_REPORT_HDR].path_tracking_flow = 0

        # generate packets for the case that endpoint is disabled
        exp_pkt_ = int_pkt.copy()
        exp_pkt_[IP].ttl-=1
        exp_pkt_[Ether].src=exp_pkt[Ether].src
        exp_pkt_[Ether].dst=exp_pkt[Ether].dst

        exp_inte2e_inner_1 = postcard_report(
            packet=exp_pkt_,
            switch_id=SID,
            ingress_port=swports[0],
            egress_port=swports[1],
            queue_id=0,
            queue_depth=0,
            egress_tstamp=0)

        exp_q_pkt_ = ipv4_dtel_pkt(
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

        exp_mod_inner_1 = mod_report(
            packet=int_pkt,
            switch_id=SID,
            ingress_port=swports[0],
            egress_port=swports[1],
            queue_id=0,
            drop_reason=71)  # drop traffic manager

        exp_dod_pkt_ = ipv4_dtel_pkt(
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


        self.queue_report_enabled = False
        sink_port = False
        self.mod_watchlist = False
        try:
            # config MoD
            self.client.switch_api_dtel_drop_report_enable(device)
            # add INT sink port
            self.client.switch_api_dtel_int_sink_ports_add(
                device=device, port=swports[1])
            sink_port=True
            print "send int packet to sink without enabling endpoint"
            self.run_test_(int_pkt, exp_pkt_, None, None,
                           exp_q_pkt_, exp_dod_pkt_)

            exp_dod_pkt[DTEL_REPORT_HDR].path_tracking_flow = 1
            exp_q_pkt[DTEL_REPORT_HDR].path_tracking_flow = 1

            print "now enable end-point for sink"
            # enable int-ep
            self.client.switch_api_dtel_int_endpoint_enable(device=device)
            self.run_test_(int_pkt, exp_pkt, exp_i2e_pkt, exp_e2e_pkt,
                           exp_q_pkt, exp_dod_pkt)

            # now source
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

            # create an INT session
            self.client.switch_api_dtel_int_session_create(
                device=device, session_id=1,
                instruction=convert_int_instruction(0x8000), max_hop=8)

            # Add INT watchlist entry
            # session_id = 1, report_all_packets = true (no digest)
            ap = switcht_twl_int_params_t(
                session_id=1, report_all_packets=True, flow_sample_percent=100)
            self.client.switch_api_dtel_int_watchlist_entry_create(
                device, twl_kvp, priority=1, watch=True, action_params=ap)

            self.client.switch_api_dtel_int_sink_ports_delete(
                device=device, port=swports[1])
            sink_port=False

            print "Test the same for source"
            self.run_test_(pkt, exp_int_pkt, None, None,
                           exp_q_pkt, exp_dod_pkt)

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            split_mirror_on_drop_pkt()
            self.client.switch_api_dtel_int_endpoint_disable(device=device)
            if self.queue_report_enabled:
                self.client.switch_api_dtel_queue_report_delete(
                    device, swports[1], 0)
            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_DROP_REPORT, 0)
            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP, 0)
            self.client.switch_api_dtel_int_endpoint_disable(device=device)
            if sink_port:
              self.client.switch_api_dtel_int_sink_ports_delete(
                device=device, port=swports[1])
            else:
              self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
              self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            self.client.switch_api_dtel_drop_report_disable(device)
            if self.mod_watchlist:
              self.client.switch_api_dtel_drop_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            config.cleanup(self)

@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_Metadata_SinkTest(api_base_tests.ThriftInterfaceDataPlane,
                               pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45 TCP Sink with all metadata"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        int_inst=0xDC00

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(int_inst), max_hop=8)

        # Add INT watchlist entry
        # session_id = 1, report_all_packets = true (no digest)
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=True, flow_sample_percent=100)
        self.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        # enable int-ep
        self.client.switch_api_dtel_int_endpoint_enable(device=device)

        # add INT sink port
        self.client.switch_api_dtel_int_sink_ports_add(
            device=device, port=swports[1])

        # make input frame to inject to sink
        pkt = simple_tcp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_id=108,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            tcp_flags= None,
            ip_ttl=64,
            tcp_sport=101)

        int_pkt_orig = int_l45_src_packet(
            test=self,
            int_inst_mask=int_inst,
            int_inst_cnt=5,
            max_hop_cnt=8,
            pkt=pkt)

        # add 2 hop info to the packet
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt_orig, val=0x66666661, incr_cnt=1)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x66666662, incr_cnt=0)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x66666663, incr_cnt=0)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x66666664, incr_cnt=0)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x66666665, incr_cnt=0)

        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x66666666, incr_cnt=1)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x66666667, incr_cnt=0)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x66666668, incr_cnt=0)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x66666669, incr_cnt=0)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x6666666a, incr_cnt=0)

        # upstream report packet
        exp_i2e_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
            dropped=0,
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=int_pkt)

        exp_pkt = simple_tcp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            tcp_flags= None,
            ip_id=108,
            ip_ttl=63,
            tcp_sport=101)

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
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=exp_inte2e_inner_1)

        try:
            m = Mask(exp_pkt)
            m.set_do_not_care_scapy(TCP, 'chksum')
            # send a test pkt
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, m, swports[1])

            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)

            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)

            verify_no_other_packets(self)

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_endpoint_disable(device=device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            self.client.switch_api_dtel_int_sink_ports_delete(
                device=device, port=swports[1])
            config.cleanup(self)

@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_Corner_SinkTest(api_base_tests.ThriftInterfaceDataPlane,
                             pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45 instruction corner cases"
        print "transit didn't follow the instruction"
        print "lasthop different from transit"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x9000), max_hop=8)

        # Add INT watchlist entry
        # session_id = 1, report_all_packets = true (no digest)
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=True, flow_sample_percent=100)
        self.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        # enable int-ep
        self.client.switch_api_dtel_int_endpoint_enable(device=device)

        # add INT sink port
        self.client.switch_api_dtel_int_sink_ports_add(
            device=device, port=swports[1])

        # make input frame to inject to sink
        pkt = simple_tcp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_id=108,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            tcp_flags= None,
            ip_ttl=64,
            tcp_sport=101)

        int_pkt_orig = int_l45_src_packet(
            test=self,
            int_inst_mask=0x9000,  # swid and qdepth
            int_inst_cnt=2,
            max_hop_cnt=8,
            pkt=pkt)

        # add 2 hop info to the packet in an incorect way
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt_orig, val=0x66666666, incr_cnt=1)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222222, incr_cnt=1)

        # upstream report packet
        exp_i2e_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
            dropped=0,
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=int_pkt)

        exp_pkt = simple_tcp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            tcp_flags= None,
            ip_id=108,
            ip_ttl=63,
            tcp_sport=101)

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
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=exp_inte2e_inner_1)

        try:
            m = Mask(exp_pkt)
            m.set_do_not_care_scapy(TCP, 'chksum')
            # send a test pkt
            send_packet(self, swports[0], str(int_pkt))

            verify_packet(self, m, swports[1])

            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)

            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)

            verify_no_other_packets(self)

            # now send with the right instruction
            int_pkt[INT_META_HDR].int_inst_mask=0x8000
            int_pkt[INT_META_HDR].int_inst_cnt=1
            exp_i2e_pkt[INT_META_HDR].int_inst_mask=0x8000
            exp_i2e_pkt[INT_META_HDR].int_inst_cnt=1

            m = Mask(exp_pkt)
            m.set_do_not_care_scapy(TCP, 'chksum')
            # send a test pkt
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, m, swports[1])

            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)

            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)

            verify_no_other_packets(self)

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_endpoint_disable(device=device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            self.client.switch_api_dtel_int_sink_ports_delete(
                device=device, port=swports[1])
            config.cleanup(self)

@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_IngressMoDTest(api_base_tests.ThriftInterfaceDataPlane,
                            pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45 Sink device with mirror on drop at Ingress"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        prepare_int_l45_bindings()
        bind_mirror_on_drop_pkt()
        config = SwitchConfig(self, params)
        twl_kvp = []

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)

        # Add INT watchlist entry
        # session_id = 1, report_all_packets = true (no digest)
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=True, flow_sample_percent=100)
        self.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        # add INT sink port
        self.client.switch_api_dtel_int_sink_ports_add(
            device=device, port=swports[1])

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

        # upstream report packet
        exp_i2e_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
            dropped=0,
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=int_pkt)

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

        # local report packet
        exp_e2e_inner = postcard_report(
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
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=exp_e2e_inner)

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

        ep_enabled = False
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
            # dropped at ingress
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed INT + ingress MoD + endpoint disabled"

            # enable int-ep
            self.client.switch_api_dtel_int_endpoint_enable(device=device)
            exp_mod_pkt[DTEL_REPORT_HDR].path_tracking_flow = 1
            ep_enabled = True

            # send a test pkt
            send_packet(self, swports[1], str(int_pkt))
            # dropped at ingress
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed INT + ingress MoD + endpoint enabled"

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

            if ep_enabled:
                self.client.switch_api_dtel_int_endpoint_disable(device=device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            self.client.switch_api_dtel_int_sink_ports_delete(
                device=device, port=swports[1])
            self.client.switch_api_dtel_drop_report_disable(device)
            self.client.switch_api_dtel_drop_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            config.cleanup(self)


@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_SourceWatchlist(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 source watchlist"
        prepare_int_l45_bindings()
        p = params.report_ports;
        params.report_ports=None
        config = SwitchConfig(self, params)

        # create INT session 1, switch ID only
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)
        int_session_1_enabled = True

        # create INT session 2, switch ID and qdepth
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=2,
            instruction=convert_int_instruction(0x9000), max_hop=8)
        int_session_2_enabled = True

        # create INT session 3, all metadata
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=3,
            instruction=convert_int_instruction(0xDC00), max_hop=8)
        int_session_3_enabled = True

        try:
            # Add INT watchlist entry
            # dst_l4_port = 11, session_id = 1, report_all_packets = true
            twl_kvp = []
            kvp_val = switcht_twl_value_t(value_num=11)
            kvp_mask = switcht_twl_value_t(value_num=0xffff)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_L4_PORT_DST, kvp_val, kvp_mask))
            ap = switcht_twl_int_params_t(
                session_id=1, report_all_packets=True, flow_sample_percent=100)
            self.client.switch_api_dtel_int_watchlist_entry_create(
                device, twl_kvp, priority=1, watch=True, action_params=ap)

            # dst_l4_port = 22, session_id = 2, report_all_packets = true
            twl_kvp = []
            kvp_val = switcht_twl_value_t(value_num=22)
            kvp_mask = switcht_twl_value_t(value_num=0xffff)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_L4_PORT_DST, kvp_val, kvp_mask))
            ap = switcht_twl_int_params_t(
                session_id=2, report_all_packets=True, flow_sample_percent=100)
            self.client.switch_api_dtel_int_watchlist_entry_create(
                device, twl_kvp, priority=1, watch=True, action_params=ap)

            self.client.switch_api_dtel_int_endpoint_enable(device)

            pkt_in_11 = simple_udp_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                with_udp_chksum=False,
                ip_id=108,
                ip_ttl=64,
                udp_dport=11)

            pkt_out_11 = simple_udp_packet(
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                with_udp_chksum=False,
                ip_id=108,
                ip_ttl=63,
                udp_dport=11)

            exp_pkt_11 = int_l45_src_packet(
                test=self,
                int_inst_mask=0x8000,
                int_inst_cnt=1,
                max_hop_cnt=8,
                pkt=pkt_out_11)

            # switch id
            exp_pkt_11 = int_l45_packet_add_hop_info(
                Packet=exp_pkt_11, val=SID, incr_cnt=1)

            send_packet(self, swports[0], str(pkt_in_11))
            m = Mask(exp_pkt_11)
            if exp_pkt_11.haslayer(UDP_INTL45):
                m.set_do_not_care_scapy(UDP_INTL45, 'chksum')
            else:
                m.set_do_not_care_scapy(UDP, 'chksum')
            verify_packet(self, m, swports[1])
            verify_no_other_packets(self)
            print "pass 1st packet with INT session 1"

            pkt_in_22 = simple_udp_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                with_udp_chksum=False,
                ip_id=108,
                ip_ttl=64,
                udp_dport=22)

            pkt_out_22 = simple_udp_packet(
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=108,
                ip_ttl=63,
                with_udp_chksum=False,
                udp_dport=22)

            exp_pkt_22 = int_l45_src_packet(
                test=self,
                int_inst_mask=0x9000,
                int_inst_cnt=2,
                max_hop_cnt=8,
                pkt=pkt_out_22)

            # queue info + switch id
            exp_pkt_22 = int_l45_packet_add_hop_info(
                Packet=exp_pkt_22, val=0)
            exp_pkt_22 = int_l45_packet_add_hop_info(
                Packet=exp_pkt_22, val=SID, incr_cnt=1)

            send_packet(self, swports[0], str(pkt_in_22))
            verify_int_packet(
                self, exp_pkt_22, swports[1],
                digest=False, ignore_hop_indices=[2])
            verify_no_other_packets(self)
            print "pass 2nd packet with INT session 2"

            # Add INT watchlist entry to session_id that does not exist
            # dst_l4_port = 33, session_id = 4, report_all_packets = true
            twl_kvp = []
            kvp_val = switcht_twl_value_t(value_num=33)
            kvp_mask = switcht_twl_value_t(value_num=0xffff)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_L4_PORT_DST, kvp_val, kvp_mask))
            ap = switcht_twl_int_params_t(
                session_id=4, report_all_packets=True, flow_sample_percent=100)
            self.assertTrue(self.client.switch_api_dtel_int_watchlist_entry_create(
                device, twl_kvp, priority=1, watch=True, action_params=ap),
                "add watchlist entry to INT session that does not exist should fail")
            print "attempt to add watchlist entry to session_id that does not exist"
            print "  generates driver error including: INT session 4 does not exist"
            print "  followed by driver error including: Watchlist type 0 add failed"

            pkt_in_33 = simple_udp_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=108,
                ip_ttl=64,
                with_udp_chksum=False,
                udp_dport=33)

            pkt_out_33 = simple_udp_packet(
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=108,
                ip_ttl=63,
                with_udp_chksum=False,
                udp_dport=33)

            send_packet(self, swports[0], str(pkt_in_33))
            verify_packet(self, pkt_out_33, swports[1])
            verify_no_other_packets(self)
            print "pass 3rd packet with INT session that does not exist"

            # Add INT watchlist entry
            # dst_l4_port = 33, session_id = 3, report_all_packets = true
            twl_kvp = []
            kvp_val = switcht_twl_value_t(value_num=33)
            kvp_mask = switcht_twl_value_t(value_num=0xffff)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_L4_PORT_DST, kvp_val, kvp_mask))
            ap = switcht_twl_int_params_t(
                session_id=3, report_all_packets=True, flow_sample_percent=100)
            self.client.switch_api_dtel_int_watchlist_entry_create(
                device, twl_kvp, priority=1, watch=True, action_params=ap)

            exp_pkt_33 = int_l45_src_packet(
                test=self,
                int_inst_mask=0xDC00,
                int_inst_cnt=5,
                max_hop_cnt=8,
                pkt=pkt_out_33)

            # egress tstamp + ingress tstamp + queue info + port ids + switch id
            exp_pkt_33 = int_l45_packet_add_hop_info(
                Packet=exp_pkt_33, val=0x22222221)
            exp_pkt_33 = int_l45_packet_add_hop_info(
                Packet=exp_pkt_33, val=0x22222221)
            exp_pkt_33 = int_l45_packet_add_hop_info(
                Packet=exp_pkt_33, val=0)
            exp_pkt_33 = int_l45_packet_add_hop_info(
                Packet=exp_pkt_33,
                val=int_port_ids_pack(swports[0], swports[1]))
            exp_pkt_33 = int_l45_packet_add_hop_info(
                Packet=exp_pkt_33, val=SID, incr_cnt=1)

            send_packet(self, swports[0], str(pkt_in_33))
            #receive_print_packet(self, swports[1], exp_pkt_33, True)
            verify_int_packet(
                self, exp_pkt_33, swports[1],
                digest=False, ignore_hop_indices=[3, 4, 5])
            verify_no_other_packets(self)
            print "pass 4th packet with INT session 3"

            # update watchlist
            # dst_l4_port = 33, session_id = 1, report_all_packets = true
            twl_kvp = []
            kvp_val = switcht_twl_value_t(value_num=33)
            kvp_mask = switcht_twl_value_t(value_num=0xffff)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_L4_PORT_DST, kvp_val, kvp_mask))
            ap = switcht_twl_int_params_t(
                session_id=1, report_all_packets=True, flow_sample_percent=100)
            self.client.switch_api_dtel_int_watchlist_entry_update(
                device, twl_kvp, priority=1, watch=True, action_params=ap)

            pkt_out_33 = simple_udp_packet(
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                with_udp_chksum=False,
                ip_id=108,
                ip_ttl=63,
                udp_dport=33)

            exp_pkt_33 = int_l45_src_packet(
                test=self,
                int_inst_mask=0x8000,
                int_inst_cnt=1,
                max_hop_cnt=8,
                pkt=pkt_out_33)

            exp_pkt_33 = int_l45_packet_add_hop_info(
                Packet=exp_pkt_33, val=SID, incr_cnt=1)

            send_packet(self, swports[0], str(pkt_in_33))
            verify_packet(self, exp_pkt_33, swports[1])
            verify_no_other_packets(self)
            print "pass 5th packet with INT session updated to 1"

            self.client.switch_api_dtel_int_session_update(
                device=device, session_id=1,
                instruction=convert_int_instruction(0xD000), max_hop=8)
            exp_pkt_33 = int_l45_src_packet(
                test=self,
                int_inst_mask=0xD000,
                int_inst_cnt=3,
                max_hop_cnt=8,
                pkt=pkt_out_33)
            exp_pkt_33 = int_l45_packet_add_hop_info(
                Packet=exp_pkt_33, val=0)
            exp_pkt_33 = int_l45_packet_add_hop_info(
                Packet=exp_pkt_33,
                val=int_port_ids_pack(swports[0], swports[1]))
            exp_pkt_33 = int_l45_packet_add_hop_info(
                Packet=exp_pkt_33, val=SID, incr_cnt=1)

            send_packet(self, swports[0], str(pkt_in_33))
            verify_int_packet(
                self, exp_pkt_33, swports[1],
                digest=False, ignore_hop_indices=[3],
                ignore_chksum=True)
            verify_no_other_packets(self)
            print "pass 6th packet with updating instruction of session 1"

            # delete INT session 3
            self.assertFalse(self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=3),
                "delete INT session 3 should succeed after watchlist update")
            int_session_3_enabled = False

            # update watchlist to session_id that does not exist
            # dst_l4_port = 33, session_id = 3, report_all_packets = true
            twl_kvp = []
            kvp_val = switcht_twl_value_t(value_num=33)
            kvp_mask = switcht_twl_value_t(value_num=0xffff)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_L4_PORT_DST, kvp_val, kvp_mask))
            ap = switcht_twl_int_params_t(
                session_id=3, report_all_packets=True, flow_sample_percent=100)
            self.assertTrue(self.client.switch_api_dtel_int_watchlist_entry_update(
                device, twl_kvp, priority=1, watch=True, action_params=ap),
                "update watchlist entry to INT session that does not exist should fail")
            print "attempt to update watchlist entry to session_id that does not exist"
            print "  generates driver error including: INT session 3 does not exist"
            print "  followed by driver error including: Watchlist type 0 update failed"

            send_packet(self, swports[0], str(pkt_in_33))
            verify_int_packet(
                self, exp_pkt_33, swports[1],
                digest=False, ignore_hop_indices=[3],
                ignore_chksum=True)
            verify_no_other_packets(self)
            print "pass 7th packet with update to INT session that does not exist"

            # update first watchlist entry to not watch
            # dst_l4_port = 11, session_id = 1, report_all_packets = true
            twl_kvp = []
            kvp_val = switcht_twl_value_t(value_num=11)
            kvp_mask = switcht_twl_value_t(value_num=0xffff)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_L4_PORT_DST, kvp_val, kvp_mask))
            ap = switcht_twl_int_params_t(
                session_id=1, report_all_packets=True, flow_sample_percent=100)
            self.client.switch_api_dtel_int_watchlist_entry_update(
                device, twl_kvp, priority=1, watch=False, action_params=ap)

            # delete INT session 1 when still in use should fail
            # Will generate driver error:
            self.assertTrue(self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1),
                "delete INT session when still in use should fail")
            print "attempt to delete session_id still in use generates driver error"
            print "  including: 1 watchlist(s) still referring to session 1"

            send_packet(self, swports[0], str(pkt_in_33))
            verify_int_packet(
                self, exp_pkt_33, swports[1],
                digest=False, ignore_hop_indices=[3],
                ignore_chksum=True)
            verify_no_other_packets(self)
            print "pass 8th packet after trying to delete INT session still in use"

            # delete last watchlist entry with dst_l4_port = 33
            twl_kvp = []
            kvp_val = switcht_twl_value_t(value_num=33)
            kvp_mask = switcht_twl_value_t(value_num=0xffff)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_L4_PORT_DST, kvp_val, kvp_mask))
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device, twl_kvp)

            # delete INT session 1
            self.assertFalse(self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1),
                "delete INT session 1 should succeed after watchlist delete"
                " and not watch")
            int_session_1_enabled = False

            # recreate INT session 1, switch ID only
            self.client.switch_api_dtel_int_session_create(
                device=device, session_id=1,
                instruction=convert_int_instruction(0x8000), max_hop=8)
            int_session_1_enabled = True

            # add watchlist entry matching all dst_l4_port values
            # dst_l4_port = *, session_id = 1, report_all_packets = true
            twl_kvp = []
            kvp_val = switcht_twl_value_t(value_num=0)
            kvp_mask = switcht_twl_value_t(value_num=0x0)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_L4_PORT_DST, kvp_val, kvp_mask))
            ap = switcht_twl_int_params_t(
                session_id=1, report_all_packets=True, flow_sample_percent=100)
            self.client.switch_api_dtel_int_watchlist_entry_create(
                device, twl_kvp, priority=2, watch=True, action_params=ap)

            send_packet(self, swports[0], str(pkt_in_22))
            verify_int_packet(
                self, exp_pkt_22, swports[1],
                digest=False, ignore_hop_indices=[2])
            verify_no_other_packets(self)
            print "pass 9th packet unaffected by priority 2 watchlist entry"

            # update watchlist entry priority to 3
            # dst_l4_port = 22, session_id = 2, report_all_packets = true
            twl_kvp = []
            kvp_val = switcht_twl_value_t(value_num=22)
            kvp_mask = switcht_twl_value_t(value_num=0xffff)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_L4_PORT_DST, kvp_val, kvp_mask))
            ap = switcht_twl_int_params_t(
                session_id=2, report_all_packets=True, flow_sample_percent=100)
            self.client.switch_api_dtel_int_watchlist_entry_update(
                device, twl_kvp, priority=3, watch=True, action_params=ap)

            exp_pkt_22 = int_l45_src_packet(
                test=self,
                int_inst_mask=0x8000,
                int_inst_cnt=1,
                max_hop_cnt=8,
                pkt=pkt_out_22)

            # switch id
            exp_pkt_22 = int_l45_packet_add_hop_info(
                Packet=exp_pkt_22, val=SID, incr_cnt=1)

            send_packet(self, swports[0], str(pkt_in_22))
            verify_int_packet(
                self, exp_pkt_22, swports[1],
                digest=False)
            verify_no_other_packets(self)
            print "pass 10th packet after updating priority of watchlist entry"

            # update watchlist entry priority back to 1
            # dst_l4_port = 22, session_id = 2, report_all_packets = true
            twl_kvp = []
            kvp_val = switcht_twl_value_t(value_num=22)
            kvp_mask = switcht_twl_value_t(value_num=0xffff)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_L4_PORT_DST, kvp_val, kvp_mask))
            ap = switcht_twl_int_params_t(
                session_id=2, report_all_packets=True, flow_sample_percent=100)
            self.client.switch_api_dtel_int_watchlist_entry_update(
                device, twl_kvp, priority=1, watch=True, action_params=ap)

            exp_pkt_22 = int_l45_src_packet(
                test=self,
                int_inst_mask=0x9000,
                int_inst_cnt=2,
                max_hop_cnt=8,
                pkt=pkt_out_22)

            # queue info + switch id
            exp_pkt_22 = int_l45_packet_add_hop_info(
                Packet=exp_pkt_22, val=0)
            exp_pkt_22 = int_l45_packet_add_hop_info(
                Packet=exp_pkt_22, val=SID, incr_cnt=1)

            send_packet(self, swports[0], str(pkt_in_22))
            verify_int_packet(
                self, exp_pkt_22, swports[1],
                digest=False, ignore_hop_indices=[2])
            verify_no_other_packets(self)
            print "pass 11th packet after setting back priority of watchlist entry"

            # update watchlist entry to different session
            # dst_l4_port = 22, session_id = 1, report_all_packets = true
            twl_kvp = []
            kvp_val = switcht_twl_value_t(value_num=22)
            kvp_mask = switcht_twl_value_t(value_num=0xffff)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_L4_PORT_DST, kvp_val, kvp_mask))
            ap = switcht_twl_int_params_t(
                session_id=1, report_all_packets=True, flow_sample_percent=100)
            self.client.switch_api_dtel_int_watchlist_entry_update(
                device, twl_kvp, priority=1, watch=True, action_params=ap)

            # delete INT session 2
            self.assertFalse(self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=2),
                "delete INT session 2 should succeed after watchlist updates"
                " and not watch")
            int_session_2_enabled = False

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_endpoint_disable(
                device=device)
            self.client.switch_api_dtel_int_watchlist_clear(device)
            if int_session_1_enabled:
                self.client.switch_api_dtel_int_session_delete(
                    device=device, session_id=1)
            if int_session_2_enabled:
                self.client.switch_api_dtel_int_session_delete(
                    device=device, session_id=2)
            if int_session_3_enabled:
                self.client.switch_api_dtel_int_session_delete(
                    device=device, session_id=3)
            config.cleanup(self)
            params.report_ports = p

@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_Watchlist_Scale(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 source watchlist range match scale test"
        prepare_int_l45_bindings()
        p = params.report_ports;
        params.report_ports=None
        config = SwitchConfig(self, params)

        # values for large watchlist
        target_exact = 1023
        target_1sided_range = 512
        target_2sided_range = 256
        # values for smaller watchlist
        target_exact = 511
        target_1sided_range = 32
        target_2sided_range = 16

        target_exact_w_1sided = target_exact-target_1sided_range
        target_exact_w_2sided = target_exact-target_2sided_range
        # create INT session 1, switch ID only
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)

        try:
            self.client.switch_api_dtel_int_endpoint_enable(device)
            pkt_in_11 = simple_udp_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                with_udp_chksum=False,
                ip_id=108,
                ip_ttl=64,
                udp_dport=0x7FFF)

            pkt_out_11 = simple_udp_packet(
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                with_udp_chksum=False,
                ip_id=108,
                ip_ttl=63,
                udp_dport=0x7FFF)

            exp_pkt_11 = int_l45_src_packet(
                test=self,
                int_inst_mask=0x8000,
                int_inst_cnt=1,
                max_hop_cnt=8,
                pkt=pkt_out_11)

            # switch id
            exp_pkt_11 = int_l45_packet_add_hop_info(
                Packet=exp_pkt_11, val=SID, incr_cnt=1)

            # Add INT watchlist entry
            for i in range(1, target_exact):
                twl_kvp = []
                kvp_val = switcht_twl_value_t(value_num=i)
                kvp_mask = switcht_twl_value_t(value_num=0xffff)
                twl_kvp.append(switcht_twl_key_value_pair_t(
                    SWITCH_TWL_FIELD_L4_PORT_DST, kvp_val, kvp_mask))
                ap = switcht_twl_int_params_t(
                    session_id=1,
                    report_all_packets=True,
                    flow_sample_percent=100)
                self.client.switch_api_dtel_int_watchlist_entry_create(
                    device, twl_kvp, priority=1, watch=False, action_params=ap)

            twl_kvp = []
            kvp_val = switcht_twl_value_t(value_num=0x7FFF)
            kvp_mask = switcht_twl_value_t(value_num=0xffff)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_L4_PORT_DST, kvp_val, kvp_mask))
            ap = switcht_twl_int_params_t(
                session_id=1, report_all_packets=True, flow_sample_percent=100)
            self.client.switch_api_dtel_int_watchlist_entry_create(
                device, twl_kvp, priority=1, watch=True, action_params=ap)

            send_packet(self, swports[0], str(pkt_in_11))
            m = Mask(exp_pkt_11)
            if exp_pkt_11.haslayer(UDP_INTL45):
                m.set_do_not_care_scapy(UDP_INTL45, 'chksum')
            else:
                m.set_do_not_care_scapy(UDP, 'chksum')
            verify_packet(self, m, swports[1])
            verify_no_other_packets(self)

            print "passed adding %d int watchlist exact entries" % target_exact
            self.client.switch_api_dtel_int_watchlist_clear(device)

            # Add INT watchlist entry
            # make sure mask is as twl_value
            # dst port 0 to i
            # src port 0 to i*7
            for i in range(0, target_1sided_range):
                twl_kvp = []
                kvp_val = switcht_twl_value_t(value_num=0)
                kvp_mask = switcht_twl_value_t(value_num=0xffff)
                twl_kvp.append(switcht_twl_key_value_pair_t(
                    SWITCH_TWL_FIELD_L4_PORT_DST_START, kvp_val, kvp_mask))
                kvp_val = switcht_twl_value_t(value_num=i)
                kvp_mask = switcht_twl_value_t(value_num=0xffff)
                twl_kvp.append(switcht_twl_key_value_pair_t(
                    SWITCH_TWL_FIELD_L4_PORT_DST_END, kvp_val, kvp_mask))
                kvp_val = switcht_twl_value_t(value_num=0)
                kvp_mask = switcht_twl_value_t(value_num=0xffff)
                twl_kvp.append(switcht_twl_key_value_pair_t(
                    SWITCH_TWL_FIELD_L4_PORT_SRC_START, kvp_val, kvp_mask))
                kvp_val = switcht_twl_value_t(value_num=i*7)
                kvp_mask = switcht_twl_value_t(value_num=0xffff)
                twl_kvp.append(switcht_twl_key_value_pair_t(
                    SWITCH_TWL_FIELD_L4_PORT_SRC_END, kvp_val, kvp_mask))
                ap = switcht_twl_int_params_t(
                    session_id=1,
                    report_all_packets=True,
                    flow_sample_percent=100)
                self.client.switch_api_dtel_int_watchlist_entry_create(
                    device, twl_kvp, priority=1, watch=True, action_params=ap)

            for i in range(1, target_exact_w_1sided):
                twl_kvp = []
                kvp_val = switcht_twl_value_t(value_num=i)
                kvp_mask = switcht_twl_value_t(value_num=0xffff)
                twl_kvp.append(switcht_twl_key_value_pair_t(
                    SWITCH_TWL_FIELD_L4_PORT_DST, kvp_val, kvp_mask))
                ap = switcht_twl_int_params_t(
                    session_id=1,
                    report_all_packets=True,
                    flow_sample_percent=100)
                self.client.switch_api_dtel_int_watchlist_entry_create(
                    device, twl_kvp, priority=2, watch=False, action_params=ap)

            twl_kvp = []
            kvp_val = switcht_twl_value_t(value_num=0x7FFF)
            kvp_mask = switcht_twl_value_t(value_num=0xffff)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_L4_PORT_DST, kvp_val, kvp_mask))
            ap = switcht_twl_int_params_t(
                session_id=1, report_all_packets=True, flow_sample_percent=100)
            self.client.switch_api_dtel_int_watchlist_entry_create(
                device, twl_kvp, priority=1, watch=True, action_params=ap)

            send_packet(self, swports[0], str(pkt_in_11))
            m = Mask(exp_pkt_11)
            if exp_pkt_11.haslayer(UDP_INTL45):
                m.set_do_not_care_scapy(UDP_INTL45, 'chksum')
            else:
                m.set_do_not_care_scapy(UDP, 'chksum')
            verify_packet(self, m, swports[1])
            verify_no_other_packets(self)
            print "passed adding %d int watchlist 1sided range and %d exact entries" % (
                target_1sided_range, target_exact_w_1sided)
            self.client.switch_api_dtel_int_watchlist_clear(device)

            # Add INT watchlist entry
            # make sure mask is as twl_value
            # dst port i to i+1023
            # src port i*7 to i*7+1023
            for i in range(0, target_2sided_range):
                twl_kvp = []
                kvp_val = switcht_twl_value_t(value_num=i)
                kvp_mask = switcht_twl_value_t(value_num=0xffff)
                twl_kvp.append(switcht_twl_key_value_pair_t(
                    SWITCH_TWL_FIELD_L4_PORT_DST_START, kvp_val, kvp_mask))
                kvp_val = switcht_twl_value_t(value_num=i+1023)
                kvp_mask = switcht_twl_value_t(value_num=0xffff)
                twl_kvp.append(switcht_twl_key_value_pair_t(
                    SWITCH_TWL_FIELD_L4_PORT_DST_END, kvp_val, kvp_mask))
                kvp_val = switcht_twl_value_t(value_num=i*7)
                kvp_mask = switcht_twl_value_t(value_num=0xffff)
                twl_kvp.append(switcht_twl_key_value_pair_t(
                    SWITCH_TWL_FIELD_L4_PORT_SRC_START, kvp_val, kvp_mask))
                kvp_val = switcht_twl_value_t(value_num=i*7+1023)
                kvp_mask = switcht_twl_value_t(value_num=0xffff)
                twl_kvp.append(switcht_twl_key_value_pair_t(
                    SWITCH_TWL_FIELD_L4_PORT_SRC_END, kvp_val, kvp_mask))
                ap = switcht_twl_int_params_t(
                    session_id=1,
                    report_all_packets=True,
                    flow_sample_percent=100)
                self.client.switch_api_dtel_int_watchlist_entry_create(
                    device, twl_kvp, priority=1, watch=True, action_params=ap)

            for i in range(1, target_exact_w_2sided):
                twl_kvp = []
                kvp_val = switcht_twl_value_t(value_num=i)
                kvp_mask = switcht_twl_value_t(value_num=0xffff)
                twl_kvp.append(switcht_twl_key_value_pair_t(
                    SWITCH_TWL_FIELD_L4_PORT_DST, kvp_val, kvp_mask))
                ap = switcht_twl_int_params_t(
                    session_id=1,
                    report_all_packets=True,
                    flow_sample_percent=100)
                self.client.switch_api_dtel_int_watchlist_entry_create(
                    device, twl_kvp, priority=2, watch=False, action_params=ap)

            twl_kvp = []
            kvp_val = switcht_twl_value_t(value_num=0x7FFF)
            kvp_mask = switcht_twl_value_t(value_num=0xffff)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_L4_PORT_DST, kvp_val, kvp_mask))
            ap = switcht_twl_int_params_t(
                session_id=1, report_all_packets=True, flow_sample_percent=100)
            self.client.switch_api_dtel_int_watchlist_entry_create(
                device, twl_kvp, priority=1, watch=True, action_params=ap)

            send_packet(self, swports[0], str(pkt_in_11))
            m = Mask(exp_pkt_11)
            if exp_pkt_11.haslayer(UDP_INTL45):
                m.set_do_not_care_scapy(UDP_INTL45, 'chksum')
            else:
                m.set_do_not_care_scapy(UDP, 'chksum')
            verify_packet(self, m, swports[1])
            verify_no_other_packets(self)
            print "passed adding %d int watchlist 2sided range and %d exact entries" % (
                target_2sided_range, target_exact_w_2sided)

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_endpoint_disable(
                device=device)
            self.client.switch_api_dtel_int_watchlist_clear(device)
            self.client.switch_api_dtel_int_session_delete(
                    device=device, session_id=1)
            config.cleanup(self)
            params.report_ports = p


@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_SinkWatchlist(api_base_tests.ThriftInterfaceDataPlane,
                           pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45 sink watchlist and default session"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        # add INT sink port
        self.client.switch_api_dtel_int_sink_ports_add(
            device=device, port=swports[1])

        # enable int-ep
        self.client.switch_api_dtel_int_endpoint_enable(device)

        payload = 'int l45'
        # make input frame to inject to sink
        pkt = simple_udp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_id=108,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            with_udp_chksum=False,
            ip_ttl=64,
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

        # upstream report packet
        exp_i2e_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
            dropped=0,
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=int_pkt)

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
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=exp_inte2e_inner_1)

        try:

            # Add INT watchlist entry
            # session_id = 1, report_all_packets = true (no digest)
            ap = switcht_twl_int_params_t(
                session_id=1, report_all_packets=True, flow_sample_percent=100)
            self.client.switch_api_dtel_int_watchlist_entry_create(
                device, twl_kvp, priority=1, watch=True, action_params=ap)

            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass 1st packet with INT sink watchlist"

            self.client.switch_api_dtel_int_watchlist_clear(device)

            exp_inte2e_inner_1 = postcard_report(
                packet=exp_pkt,
                switch_id=SID,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_e2e_pkt_dc00 = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=exp_inte2e_inner_1)

            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt_dc00, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass 2nd packet w/o INT sink watchlist"

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_endpoint_disable(device)
            self.client.switch_api_dtel_int_watchlist_clear(device)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            self.client.switch_api_dtel_int_sink_ports_delete(
                device=device, port=swports[1])
            config.cleanup(self)

@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_DSCP_SourceTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 UDP Source device - changing INT L45 DSCP value"
        "diffserv value"
        if get_int_l45_encap() != "dscp":
            print "Not running with INT L45 encap using diffserv"
            print "Skipping this test"
            return

        prepare_int_l45_bindings()
        p = params.report_ports;
        params.report_ports=None
        config = SwitchConfig(self, params)

        current_dscp = 0x02
        prepare_int_l45_bindings(current_dscp, current_dscp)
        self.client.switch_api_dtel_int_dscp_value_set(
            device,
            current_dscp,
            current_dscp)

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)

        # Add INT watchlist entry
        # session_id = 1, report_all_packets = true (no digest)
        ap = switcht_twl_int_params_t(session_id=1,
            report_all_packets=True,
            flow_sample_percent=100)
        self.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        payload = 'int_l45'
        pkt = simple_udp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_id=108,
            ip_ttl=64,
            with_udp_chksum=False,
            udp_payload=payload)

        exp_pkt_ = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_id=108,
            ip_ttl=63,
            with_udp_chksum=False,
            udp_payload=payload)

        exp_pkt = int_l45_src_packet(
            test=self,
            int_inst_mask=0x8000,
            int_inst_cnt=1,
            max_hop_cnt=8,
            pkt=exp_pkt_)


        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=SID, incr_cnt=1)
        exp_pkt[IP].tos = current_dscp << 2
        try:
            self.client.switch_api_dtel_int_endpoint_enable(device)
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass dscp 0x%x mask 0x%x" %(current_dscp,
                                               current_dscp)

            current_dscp = 0x10
            prepare_int_l45_bindings(current_dscp, current_dscp)
            self.client.switch_api_dtel_int_dscp_value_set(
                device,
                current_dscp,
                current_dscp);
            exp_pkt[IP].tos = current_dscp << 2

            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass dscp 0x%x mask 0x%x" %(current_dscp,
                                               current_dscp)

            current_dscp = 0xB
            prepare_int_l45_bindings(current_dscp, 0x3f)
            self.client.switch_api_dtel_int_dscp_value_set(
                device,
                current_dscp,
                0x3f);
            exp_pkt[IP].tos = current_dscp << 2

            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass dscp 0x%x mask 0x%x" %(current_dscp,
                                               0x3f)

            self.client.switch_api_dtel_int_endpoint_disable(device)
            self.client.switch_api_dtel_int_endpoint_enable(device)
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass dscp 0x%x mask 0x%x after disable/enable" %(current_dscp,
                                               0x3f)

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_endpoint_disable(device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            config.cleanup(self)
            params.report_ports = p

@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_DSCP_SinkTest(api_base_tests.ThriftInterfaceDataPlane,
                          pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45 UDP Sink device when changing INT L45 DSCP value"
        if get_int_l45_encap() != "dscp":
            print "Not running with INT L45 encap using diffserv"
            print "Skipping this test"
            return

        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        config = SwitchConfig(self, params)

        # Original TOS bit value before applying INT
        # AF13 = 0x0e from RFC 2597
        original_tos = 0x0e << 2

        # INT L45 indicator DSCP
        current_dscp = 0x20
        prepare_int_l45_bindings(current_dscp, current_dscp)
        self.client.switch_api_dtel_int_dscp_value_set(
            device,
            current_dscp,
            current_dscp);

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)

        # Add INT watchlist entry
        # session_id = 1, report_all_packets = true (no digest)
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=True, flow_sample_percent=100)
        self.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        # add INT sink port
        self.client.switch_api_dtel_int_sink_ports_add(
            device=device, port=swports[1])

        payload = 'int l45'
        # make input frame to inject to sink
        pkt = simple_udp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_id=108,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=64,
            ip_tos=original_tos,
            udp_sport=101,
            with_udp_chksum=False,
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
        int_pkt[IP].tos = original_tos | (current_dscp << 2)

        # upstream report packet
        exp_i2e_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
            dropped=0,
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=int_pkt)

        exp_pkt = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            with_udp_chksum=False,
            ip_id=108,
            ip_ttl=63,
            ip_tos=original_tos,  # assume INT L45 DSCP bit was originally 0
            udp_sport=101,
            udp_payload=payload)

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
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=exp_inte2e_inner_1)

        try:
            # enable int-ep
            self.client.switch_api_dtel_int_endpoint_enable(device)
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify i2e mirrored packet
            verify_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass dscp 0x%x mask 0x%x" %(current_dscp,
                                               current_dscp)

            current_dscp = 0x10
            prepare_int_l45_bindings(current_dscp, current_dscp)
            self.client.switch_api_dtel_int_dscp_value_set(
                device,
                current_dscp,
                current_dscp);
            int_pkt[IP].tos = original_tos | (current_dscp << 2)
            # upstream report packet
            exp_i2e_pkt = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=int_pkt)

            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify i2e mirrored packet
            verify_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass dscp 0x%x mask 0x%x" %(current_dscp,
                                               current_dscp)

            # test the case when INT L45 DSCP is taking the entire 6b
            current_dscp = 0xB
            current_dscp_mask = 0x3f
            prepare_int_l45_bindings(current_dscp, current_dscp_mask)
            self.client.switch_api_dtel_int_dscp_value_set(
                device,
                current_dscp,
                current_dscp_mask);
            int_pkt[IP].tos = (original_tos & 0x3) | (current_dscp << 2)
            # upstream report packet
            exp_i2e_pkt = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=int_pkt)
            #downstream report packet
            exp_pkt[IP].tos = original_tos & 0x3  # dscp reset to 0
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
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=exp_inte2e_inner_1)

            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify i2e mirrored packet
            verify_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass dscp 0x%x mask 0x%x" %(current_dscp,
                                               current_dscp_mask)

            self.client.switch_api_dtel_int_endpoint_disable(device)
            self.client.switch_api_dtel_int_endpoint_enable(device)
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify i2e mirrored packet
            verify_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass dscp 0x%x mask 0x%x after disable/enable" %(current_dscp,
                                               current_dscp_mask)

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_endpoint_disable(device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            self.client.switch_api_dtel_int_sink_ports_delete(
                device=device, port=swports[1])
            config.cleanup(self)

@group('ep_l45_chksum')
class INTL45_CHKSUM_SourceTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 Source device with checksum"
        prepare_int_l45_bindings()
        p = params.report_ports;
        params.report_ports=None
        config = SwitchConfig(self, params)

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)

        # Add INT watchlist entry
        # session_id = 1, report_all_packets = true (no digest)
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=True, flow_sample_percent=100)
        self.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        self.client.switch_api_dtel_int_endpoint_enable(device)

        try:
            payload = 'int_l45'
            pkt = simple_udp_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=108,
                ip_ttl=64,
                with_udp_chksum=False, # udp checksum always 0
                udp_payload=payload)

            exp_pkt_ = simple_udp_packet(
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=108,
                ip_ttl=63,
                with_udp_chksum=False, # udp checksum always 0
                udp_payload=payload)

            exp_pkt = int_l45_src_packet(
                test=self,
                int_inst_mask=0x8000,
                int_inst_cnt=1,
                max_hop_cnt=8,
                pkt=exp_pkt_)

            exp_pkt = int_l45_packet_add_hop_info(
                Packet=exp_pkt, val=SID, incr_cnt=1)

            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "Passed UDP packet"

            pkt = simple_tcp_packet(
                eth_dst=params.mac_self,
                tcp_flags= None,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=108,
                ip_ttl=64)

            exp_pkt = simple_tcp_packet(
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                tcp_flags= None,
                ip_id=108,
                ip_ttl=63)

            exp_pkt = int_l45_src_packet(
                test=self,
                int_inst_mask=0x8000,
                int_inst_cnt=1,
                max_hop_cnt=8,
                pkt=exp_pkt)

            exp_pkt = int_l45_packet_add_hop_info(
                Packet=exp_pkt, val=SID, incr_cnt=1)
            if exp_pkt.haslayer(TCP_INTL45):
                del exp_pkt[TCP_INTL45].chksum
            else:
                del exp_pkt[TCP].chksum

            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "Passed TCP packet"

            pkt = simple_icmp_packet(
                pktlen=64,
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=108,
                ip_ttl=64)

            exp_pkt = simple_icmp_packet(
                pktlen=64,
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=108,
                ip_ttl=63)

            exp_pkt = int_l45_src_packet(
                test=self,
                int_inst_mask=0x8000,
                int_inst_cnt=1,
                max_hop_cnt=8,
                pkt=exp_pkt)

            exp_pkt = int_l45_packet_add_hop_info(
                Packet=exp_pkt, val=SID, incr_cnt=1)
            if exp_pkt.haslayer(ICMP_INTL45):
                del exp_pkt[ICMP_INTL45].chksum
            else:
                del exp_pkt[ICMP].chksum

            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "Passed ICMP packet"

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_endpoint_disable(device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            config.cleanup(self)
            params.report_ports = p

@group('ep_l45_chksum')
class INTL45_CHKSUM_SinkTest(api_base_tests.ThriftInterfaceDataPlane,
                             pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45 Sink device with checksum"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)

        # Add INT watchlist entry
        # session_id = 1, report_all_packets = true (no digest)
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=True, flow_sample_percent=100)
        self.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        # add INT sink port
        self.client.switch_api_dtel_int_sink_ports_add(
            device=device, port=swports[1])

        # enable int-ep
        self.client.switch_api_dtel_int_endpoint_enable(device)

        try:
            payload = 'int l45'
            # make input frame to inject to sink
            pkt = simple_udp_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_id=108,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_ttl=64,
                udp_sport=101,
                with_udp_chksum=False, # udp checksum always 0
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

            # upstream report packet
            exp_i2e_pkt = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=int_pkt)

            exp_pkt = simple_udp_packet(
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=108,
                ip_ttl=63,
                udp_sport=101,
                with_udp_chksum=False, # udp checksum always 0
                udp_payload=payload)

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
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=exp_inte2e_inner_1)

            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify i2e mirrored packet
            verify_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass UDP checksum"

            pkt = simple_tcp_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_id=108,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                tcp_flags= None,
                ip_ttl=64,
                tcp_sport=101)


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

            # upstream report packet
            exp_i2e_pkt = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=int_pkt)

            exp_pkt = simple_tcp_packet(
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                tcp_flags= None,
                ip_id=108,
                ip_ttl=63,
                tcp_sport=101)

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
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=exp_inte2e_inner_1)

            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify i2e mirrored packet
            verify_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass TCP checksum"

            # make input frame to inject to sink
            pkt = simple_icmp_packet(
                pktlen=64,
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_id=108,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_ttl=64)


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

            # bring back the checksum
            if int_pkt.haslayer(ICMP_INTL45):
              del int_pkt[ICMP_INTL45].chksum
            elif int_pkt.haslayer(ICMP):
              del int_pkt[ICMP].chksum

            # upstream report packet
            exp_i2e_pkt = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=int_pkt)

            exp_pkt = simple_icmp_packet(
                pktlen=64,
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=108,
                ip_ttl=63)

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
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=exp_inte2e_inner_1)
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify i2e mirrored packet
            verify_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass ICMP checksum"

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_endpoint_disable(device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            self.client.switch_api_dtel_int_sink_ports_delete(
                device=device, port=swports[1])
            config.cleanup(self)

@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_ReportPortSet_Test(api_base_tests.ThriftInterfaceDataPlane,
                             pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45 Change DTel Report UDP port"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)

        # Add INT watchlist entry
        # session_id = 1, report_all_packets = true (no digest)
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=True, flow_sample_percent=100)
        self.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        # add INT sink port
        self.client.switch_api_dtel_int_sink_ports_add(
            device=device, port=swports[1])

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
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt_orig, val=0x66666666, incr_cnt=1)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222222, incr_cnt=1)

        # upstream report packet
        exp_i2e_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
            dropped=0,
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=int_pkt)

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
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=exp_inte2e_inner_1)

        int_enabled = False

        try:
            # send a test pkt
            self.client.switch_api_dtel_report_udp_dstport_set(
                device, UDP_PORT_DTEL_REPORT^0x1111);
            bind_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT^0x1111)
            split_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT)

            exp_i2e_pkt[UDP].dport = UDP_PORT_DTEL_REPORT^0x1111
            exp_e2e_pkt[UDP].dport = UDP_PORT_DTEL_REPORT^0x1111
            # enable int-ep
            self.client.switch_api_dtel_int_endpoint_enable(device)
            int_enabled = True
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass 1st packet"

            self.client.switch_api_dtel_report_udp_dstport_set(
                device, UDP_PORT_DTEL_REPORT);
            split_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT^0x1111)
            bind_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT)
            exp_i2e_pkt[UDP].dport = UDP_PORT_DTEL_REPORT
            exp_e2e_pkt[UDP].dport = UDP_PORT_DTEL_REPORT
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass 2nd packet"

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_report_udp_dstport_set(
                device, UDP_PORT_DTEL_REPORT);
            split_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT^0x1111)
            bind_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT)
            if int_enabled:
                self.client.switch_api_dtel_int_endpoint_disable(device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            self.client.switch_api_dtel_int_sink_ports_delete(
                device=device, port=swports[1])
            config.cleanup(self)


@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_ReportSeq_Test(api_base_tests.ThriftInterfaceDataPlane,
                             pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test DTel Report sequence number"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)

        # Add INT watchlist entry
        # session_id = 1, report_all_packets = true (no digest)
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=True, flow_sample_percent=100)
        self.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        # add INT sink port
        self.client.switch_api_dtel_int_sink_ports_add(
            device=device, port=swports[1])

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
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt_orig, val=0x66666666, incr_cnt=1)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222222, incr_cnt=1)

        initial_seq = 0xFFFFFFFD
        # upstream report packet
        exp_i2e_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
            dropped=0,
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            sequence_number=initial_seq,
            inner_frame=int_pkt)

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
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            sequence_number=initial_seq+1,
            inner_frame=exp_inte2e_inner_1)

        int_enabled = False

        try:
            if initial_seq & 0x80000000 > 0:
                # two's complement
                initial_seq2 = ~initial_seq + 1
                initial_seq2 = -(0xFFFFFFFF & initial_seq2)
            self.client.switch_api_dtel_report_sequence_number_set(
                device, params.mirror_ids[0], initial_seq2)
            seq_numbers = self.client.switch_api_dtel_report_sequence_number_get(
                device, params.mirror_ids[0], 4)
            for s in seq_numbers:
                self.assertTrue(s == initial_seq2,
                        "Could not get initial configure sequence number "
                                "%x vs %x" % (s,initial_seq2))
            # enable int-ep
            self.client.switch_api_dtel_int_endpoint_enable(device)
            int_enabled = True
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]],
                ignore_seq_num=False)
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]],
                ignore_seq_num=False)
            verify_no_other_packets(self)
            print "pass seq increment"


            seq_numbers = self.client.switch_api_dtel_report_sequence_number_get(
                device, params.mirror_ids[0], 4)
            for i in range(len(seq_numbers)):
                if i == hw_id:
                    self.assertTrue(seq_numbers[i] == (initial_seq2+2) ,
                        "Could not get updated sequence number "
                                    "%x vs %x"% (seq_numbers[i],
                                                 (initial_seq2+2)))
                else:
                    self.assertTrue(seq_numbers[i] == initial_seq2,
                        "Could not get initial configure sequence number")
            exp_i2e_pkt[DTEL_REPORT_HDR].sequence_number = (initial_seq+2)& 0xFFFFFFFF
            exp_e2e_pkt[DTEL_REPORT_HDR].sequence_number = (initial_seq+3)& 0xFFFFFFFF
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]],
                ignore_seq_num=False)
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]],
                ignore_seq_num=False)
            verify_no_other_packets(self)
            print "pass seq increment"

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            if int_enabled:
                self.client.switch_api_dtel_int_endpoint_disable(device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            self.client.switch_api_dtel_int_sink_ports_delete(
                device=device, port=swports[1])
            config.cleanup(self)

@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_Watchlist_SampleTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 UDP Source device - not VTEP-src, just INT-src"
        prepare_int_l45_bindings()
        p = params.report_ports;
        params.report_ports=None
        config = SwitchConfig(self, params)

        percent = 50
        pkts_num = 100
        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)

        # Add INT watchlist entry
        # session_id = 1, report_all_packets = true (no digest)
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=True,
                                      flow_sample_percent=percent)
        self.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        payload = 'int_l45'
        pkt = simple_udp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_id=108,
            ip_ttl=64,
            with_udp_chksum=False,
            udp_payload=payload)

        exp_pkt_ = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            with_udp_chksum=False,
            ip_id=108,
            ip_ttl=63,
            udp_payload=payload)

        exp_pkt = int_l45_src_packet(
            test=self,
            int_inst_mask=0x8000,
            int_inst_cnt=1,
            max_hop_cnt=8,
            pkt=exp_pkt_)

        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=SID, incr_cnt=1)

        self.client.switch_api_dtel_int_endpoint_enable(device)

        try:
            int_pkts=0
            for i in range(0, pkts_num):
                ip_src=params.ipaddr_nbr[0]
                ip_src = ip_src[0:(ip_src.rfind('.')+1)]+('%d'% i)
                pkt[IP].src=ip_src
                exp_pkt[IP].src=ip_src
                exp_pkt_[IP].src=ip_src
                send_packet(self, swports[0], str(pkt))
                rcv_pkt=receive_packet(self, swports[1])
                if str(rcv_pkt)==str(exp_pkt):
                    int_pkts+=1
                    print "%d: INT"%i
                else:
                    if str(rcv_pkt)!=str(exp_pkt_):
                        hexdump(exp_pkt)
                        hexdump(exp_pkt_)
                        hexdump(rcv_pkt)
                    self.assertTrue( str(rcv_pkt)==str(exp_pkt_),
                                   "Packet doesn't match any expected packet")
                    print "%d: no INT"%i
            self.assertTrue(int_pkts>=pkts_num*percent/100 *0.8 and
                            int_pkts<=pkts_num*percent/100 *1.2,
                            "Expected %f percent INT "
                            "packets but received %f percent" %(
                                percent, 100.0 * int_pkts / pkts_num))

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_endpoint_disable(device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            config.cleanup(self)
            params.report_ports = p

@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_SinkParserTest(api_base_tests.ThriftInterfaceDataPlane,
                          pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45 Sink device parsing stack of different sizes"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        self.client.switch_api_dtel_int_endpoint_enable(device)

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
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=exp_inte2e_inner_1)

        try:
          int_pkt = int_pkt_orig
          for stack_len in range(26):
            exp_i2e_pkt = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=int_pkt)
            # send a test pkt
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass a packet w/ %d metadata"%stack_len
            int_pkt = int_l45_packet_add_hop_info(
                Packet=int_pkt, val=0x22222222, incr_cnt=1)
        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_endpoint_disable(device)
            config.cleanup(self)

@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_Marker_SourceTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 Source device - Marker"
        if get_int_l45_encap() != "marker":
            print "Not running with INT L45 encap using marker"
            print "Skipping this test"
            return
        prepare_int_l45_bindings()
        p = params.report_ports;
        params.report_ports=None
        config = SwitchConfig(self, params)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        payload = 'int_l45'

        self.client.switch_api_dtel_int_endpoint_enable(device)
        session_enabled = False
        watchlist_enabled = False

        try:
            ##### UDP
            set_int_l45_marker(0xdeadbeefdeadbeef, 17)
            self.client.switch_api_dtel_int_marker_set(
                device, 17, hex_to_i64(0xdeadbeefdeadbeef))
            # create an INT session
            self.client.switch_api_dtel_int_session_create(
                device=device, session_id=1,
                instruction=convert_int_instruction(0x8000), max_hop=8)
            session_enabled = True

            # Add INT watchlist entry
            # session_id = 1, report_all_packets = true (no digest)
            ap = switcht_twl_int_params_t(
                session_id=1, report_all_packets=True, flow_sample_percent=100)
            self.client.switch_api_dtel_int_watchlist_entry_create(
                device, twl_kvp, priority=1, watch=True, action_params=ap)
            watchlist_enabled = True

            pkt = simple_udp_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=108,
                ip_ttl=64,
                with_udp_chksum=False,
                udp_payload=payload)

            exp_pkt_ = simple_udp_packet(
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                with_udp_chksum=False,
                ip_id=108,
                ip_ttl=63,
                udp_payload=payload)

            exp_pkt = int_l45_src_packet(
                test=self,
                int_inst_mask=0x8000,
                int_inst_cnt=1,
                max_hop_cnt=8,
                pkt=exp_pkt_)

            exp_pkt = int_l45_packet_add_hop_info(
                Packet=exp_pkt, val=SID, incr_cnt=1)

            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass packet with setting marker before session"

            set_int_l45_marker(INT_L45_MARKER, 17)
            exp_pkt[INTL45_MARKER].marker=INT_L45_MARKER
            self.client.switch_api_dtel_int_marker_set(
                device, 17, hex_to_i64(INT_L45_MARKER))
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass packet with setting marker after session"

            self.client.switch_api_dtel_int_marker_delete(
                device, 17)
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass packet after disabling marker"

            # changing for TCP doesn't affect UDP
            set_int_l45_marker(0xdeadbeefdeadbeef, 6)
            self.client.switch_api_dtel_int_marker_set(
                device, 6, hex_to_i64(0xdeadbeefdeadbeef))
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass changing marker for TCP doesn't change for UDP"
            set_int_l45_marker(INT_L45_MARKER, 6)
            self.client.switch_api_dtel_int_marker_set(
                device, 6, hex_to_i64(INT_L45_MARKER))
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            watchlist_enabled = False
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            session_enabled = False


            ##### TCP
            set_int_l45_marker(0xdeadbeefdeadbeef, 6)
            self.client.switch_api_dtel_int_marker_set(
                device, 6, hex_to_i64(0xdeadbeefdeadbeef))
            # create an INT session
            self.client.switch_api_dtel_int_session_create(
                device=device, session_id=1,
                instruction=convert_int_instruction(0x8000), max_hop=8)
            session_enabled = True
            self.client.switch_api_dtel_int_watchlist_entry_create(
                device, twl_kvp, priority=1, watch=True, action_params=ap)
            watchlist_enabled = True
            pkt = simple_tcp_packet(
                eth_dst=params.mac_self,
                tcp_flags= None,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=108,
                ip_ttl=64)

            exp_pkt = simple_tcp_packet(
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                tcp_flags= None,
                ip_id=108,
                ip_ttl=63)

            exp_pkt = int_l45_src_packet(
                test=self,
                int_inst_mask=0x8000,
                int_inst_cnt=1,
                max_hop_cnt=8,
                pkt=exp_pkt)

            exp_pkt = int_l45_packet_add_hop_info(
                Packet=exp_pkt, val=SID, incr_cnt=1)

            m = Mask(exp_pkt)
            m.set_do_not_care_scapy(TCP, 'chksum')
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, m, swports[1])
            verify_no_other_packets(self)
            print "pass packet with setting marker before session"

            set_int_l45_marker(INT_L45_MARKER, 6)
            exp_pkt[INTL45_MARKER].marker=INT_L45_MARKER
            m = Mask(exp_pkt)
            m.set_do_not_care_scapy(TCP, 'chksum')
            self.client.switch_api_dtel_int_marker_set(
                device, 6, hex_to_i64(INT_L45_MARKER))
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, m, swports[1])
            verify_no_other_packets(self)
            print "pass packet with setting marker after session"

            self.client.switch_api_dtel_int_marker_delete(
                device, 6)
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, m, swports[1])
            verify_no_other_packets(self)
            print "pass packet after disabling marker"
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            watchlist_enabled = False
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            session_enabled = False

            ##### ICMP
            set_int_l45_marker(0xdeadbeefdeadbeef, 1)
            self.client.switch_api_dtel_int_marker_set(
                device, 1, hex_to_i64(0xdeadbeefdeadbeef))
            # create an INT session
            self.client.switch_api_dtel_int_session_create(
                device=device, session_id=1,
                instruction=convert_int_instruction(0x8000), max_hop=8)
            session_enabled = True
            self.client.switch_api_dtel_int_watchlist_entry_create(
                device, twl_kvp, priority=1, watch=True, action_params=ap)
            watchlist_enabled = True
            pkt = simple_icmp_packet(
                pktlen=64,
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=108,
                ip_ttl=64)

            exp_pkt = simple_icmp_packet(
                pktlen=64,
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=108,
                ip_ttl=63)

            exp_pkt = int_l45_src_packet(
                test=self,
                int_inst_mask=0x8000,
                int_inst_cnt=1,
                max_hop_cnt=8,
                pkt=exp_pkt)

            exp_pkt = int_l45_packet_add_hop_info(
                Packet=exp_pkt, val=SID, incr_cnt=1)

            del exp_pkt[ICMP_INTL45].chksum
            m = Mask(exp_pkt)
            m.set_do_not_care_scapy(ICMP_INTL45, 'chksum')
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, m, swports[1])
            verify_no_other_packets(self)
            print "pass packet with setting marker before session"

            set_int_l45_marker(INT_L45_MARKER, 1)
            exp_pkt[INTL45_MARKER].marker=INT_L45_MARKER
            self.client.switch_api_dtel_int_marker_set(
                device, 1, hex_to_i64(INT_L45_MARKER))
            m = Mask(exp_pkt)
            m.set_do_not_care_scapy(ICMP_INTL45, 'chksum')
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, m, swports[1])
            verify_no_other_packets(self)
            print "pass packet with setting marker after session"

            self.client.switch_api_dtel_int_marker_delete(
                device, 1)
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, m, swports[1])
            verify_no_other_packets(self)
            print "pass packet after disabling marker"

            #changing for TCP doesn't affect ICMP
            set_int_l45_marker(0xdeadbeefdeadbeef, 6)
            self.client.switch_api_dtel_int_marker_set(
                device, 6, hex_to_i64(0xdeadbeefdeadbeef))
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, m, swports[1])
            verify_no_other_packets(self)
            print "pass changing marker for TCP doesn't change for ICMP"
            set_int_l45_marker(INT_L45_MARKER, 6)
            self.client.switch_api_dtel_int_marker_set(
                device, 6, hex_to_i64(INT_L45_MARKER))

            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            watchlist_enabled = False
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            session_enabled = False


        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_marker_set(
                device, 1, hex_to_i64(INT_L45_MARKER))
            self.client.switch_api_dtel_int_marker_set(
                device, 6, hex_to_i64(INT_L45_MARKER))
            self.client.switch_api_dtel_int_marker_set(
                device, 17, hex_to_i64(INT_L45_MARKER))
            self.client.switch_api_dtel_int_endpoint_disable(device)
            if watchlist_enabled:
                self.client.switch_api_dtel_int_watchlist_entry_delete(
                    device=device, twl_kvp=twl_kvp)
            if session_enabled:
                self.client.switch_api_dtel_int_session_delete(
                 device=device, session_id=1)
            set_int_l45_marker(INT_L45_MARKER)
            config.cleanup(self)
            params.report_ports = p

@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_Marker_SinkTest(api_base_tests.ThriftInterfaceDataPlane,
                          pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45 Sink device generating DTel report"
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

        # add INT sink port
        self.client.switch_api_dtel_int_sink_ports_add(
            device=device, port=swports[1])

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

        self.client.switch_api_dtel_int_endpoint_enable(device)

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
            print "pass UDP marker port is not configred"

            # now add the port
            self.client.switch_api_dtel_int_marker_port_add(
               device, 17, 81, hex_to_i16(0xffff))
            exp_pkt[UDP].dport=81

            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            (_, rcv_port, rcv_pkt, pkt_time) = \
                self.dataplane.poll(port_number=swports[params.report_ports[0]],
                                    timeout=2)
            (_, rcv_port, rcv_pkt, pkt_time) = \
                self.dataplane.poll(port_number=swports[params.report_ports[0]],
                                    timeout=2)
            verify_no_other_packets(self)
            print "pass UDP marker port is configured"

            set_int_l45_marker(0xdeadbeefdeadbeef, 17)
            self.client.switch_api_dtel_int_marker_set(
                device, 17, hex_to_i64(0xdeadbeefdeadbeef))
            int_pkt[INTL45_MARKER].marker=0xdeadbeefdeadbeef
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            (_, rcv_port, rcv_pkt, pkt_time) = \
                self.dataplane.poll(port_number=swports[params.report_ports[0]],
                                    timeout=2)
            (_, rcv_port, rcv_pkt, pkt_time) = \
                self.dataplane.poll(port_number=swports[params.report_ports[0]],
                                    timeout=2)
            verify_no_other_packets(self)

            # port 80 is there by SwitchConfig
            exp_pkt[UDP].dport=80
            int_pkt[UDP].dport=80
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            (_, rcv_port, rcv_pkt, pkt_time) = \
                self.dataplane.poll(port_number=swports[params.report_ports[0]],
                                    timeout=2)
            (_, rcv_port, rcv_pkt, pkt_time) = \
                self.dataplane.poll(port_number=swports[params.report_ports[0]],
                                    timeout=2)
            verify_no_other_packets(self)
            print "pass marker changes for all added ports"

            set_int_l45_marker(0xabcdabcdabcdabcd, 6)
            self.client.switch_api_dtel_int_marker_set(
                device, 6, hex_to_i64(0xabcdabcdabcdabcd))
            exp_pkt[UDP].dport=80
            int_pkt[UDP].dport=80
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            (_, rcv_port, rcv_pkt, pkt_time) = \
                self.dataplane.poll(port_number=swports[params.report_ports[0]],
                                    timeout=2)
            (_, rcv_port, rcv_pkt, pkt_time) = \
                self.dataplane.poll(port_number=swports[params.report_ports[0]],
                                    timeout=2)
            verify_no_other_packets(self)
            print "pass marker change for TCP doesn't affect UDP"

            self.client.switch_api_dtel_int_marker_delete(
                device, 6)
            exp_pkt[UDP].dport=80
            int_pkt[UDP].dport=80
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            (_, rcv_port, rcv_pkt, pkt_time) = \
                self.dataplane.poll(port_number=swports[params.report_ports[0]],
                                    timeout=2)
            (_, rcv_port, rcv_pkt, pkt_time) = \
                self.dataplane.poll(port_number=swports[params.report_ports[0]],
                                    timeout=2)
            verify_no_other_packets(self)
            print "pass disabling Marker for TCP doesn't affect UDP"

            self.client.switch_api_dtel_int_marker_port_delete(
                device, 17, 81, hex_to_i16(0xffff))
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            (_, rcv_port, rcv_pkt, pkt_time) = \
                self.dataplane.poll(port_number=swports[params.report_ports[0]],
                                    timeout=2)
            (_, rcv_port, rcv_pkt, pkt_time) = \
                self.dataplane.poll(port_number=swports[params.report_ports[0]],
                                    timeout=2)
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
            (_, rcv_port, rcv_pkt, pkt_time) = \
                self.dataplane.poll(port_number=swports[params.report_ports[0]],
                                    timeout=2)
            (_, rcv_port, rcv_pkt, pkt_time) = \
                self.dataplane.poll(port_number=swports[params.report_ports[0]],
                                    timeout=2)
            verify_no_other_packets(self)
            print "pass add port back after delete works"

            self.client.switch_api_dtel_int_marker_port_clear(device, 6)
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            (_, rcv_port, rcv_pkt, pkt_time) = \
                self.dataplane.poll(port_number=swports[params.report_ports[0]],
                                    timeout=2)
            (_, rcv_port, rcv_pkt, pkt_time) = \
                self.dataplane.poll(port_number=swports[params.report_ports[0]],
                                    timeout=2)
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
            (_, rcv_port, rcv_pkt, pkt_time) = \
                self.dataplane.poll(port_number=swports[params.report_ports[0]],
                                    timeout=2)
            (_, rcv_port, rcv_pkt, pkt_time) = \
                self.dataplane.poll(port_number=swports[params.report_ports[0]],
                                    timeout=2)
            verify_no_other_packets(self)

            int_pkt[UDP].dport = 81
            exp_pkt[UDP].dport = 81
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            (_, rcv_port, rcv_pkt, pkt_time) = \
                self.dataplane.poll(port_number=swports[params.report_ports[0]],
                                    timeout=2)
            (_, rcv_port, rcv_pkt, pkt_time) = \
                self.dataplane.poll(port_number=swports[params.report_ports[0]],
                                    timeout=2)
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
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            (_, rcv_port, rcv_pkt, pkt_time) = \
                self.dataplane.poll(port_number=swports[params.report_ports[0]],
                                    timeout=2)
            (_, rcv_port, rcv_pkt, pkt_time) = \
                self.dataplane.poll(port_number=swports[params.report_ports[0]],
                                    timeout=2)
            verify_no_other_packets(self)
            int_pkt[UDP].dport = 901
            exp_pkt[UDP].dport = 901
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            (_, rcv_port, rcv_pkt, pkt_time) = \
                self.dataplane.poll(port_number=swports[params.report_ports[0]],
                                    timeout=2)
            (_, rcv_port, rcv_pkt, pkt_time) = \
                self.dataplane.poll(port_number=swports[params.report_ports[0]],
                                    timeout=2)
            verify_no_other_packets(self)
            print "pass enable UDP marker after adding ports works for multiple port entries"

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_endpoint_disable(device)
            self.client.switch_api_dtel_int_sink_ports_delete(
                device=device, port=swports[1])
            config.cleanup(self)

@group('ep_l45')
@group('ep_l45_no_suppression')
class INTL45_IngressPortMirror_SinkTest(api_base_tests.ThriftInterfaceDataPlane,
                                        pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45 Sink generating DTel report + ingress port mirror"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)

        # Add INT watchlist entry
        # session_id = 1, report_all_packets = true (no digest)
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=True, flow_sample_percent=100)
        self.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        # add INT sink port
        self.client.switch_api_dtel_int_sink_ports_add(
            device=device, port=swports[1])

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

        # upstream report packet
        exp_i2e_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
            dropped=0,
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=int_pkt)

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
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=exp_inte2e_inner_1)

        int_enabled = False
        port_mirror_enabled = False

        exp_pkt_ = int_pkt.copy()
        exp_pkt_[IP].ttl-=1
        exp_pkt_[Ether].src=exp_pkt[Ether].src
        exp_pkt_[Ether].dst=exp_pkt[Ether].dst

        try:
            # enable int-ep
            self.client.switch_api_dtel_int_endpoint_enable(device)
            int_enabled = True
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass packet w/ INT enabled generating I2E + E2E reports"

            # create a mirror session
            ingress_port_hdl = self.client.switch_api_port_id_to_handle_get(
                device, swports[0])
            report_port_hdl = self.client.switch_api_port_id_to_handle_get(
                device, swports[params.report_ports[0]])

            port_mirror_info = switcht_mirror_info_t(
                session_id=1,
                direction=1,
                egress_port_handle=report_port_hdl,
                mirror_type=SWITCH_MIRROR_TYPE_LOCAL,
                cos=0,
                max_pkt_len=0,
                ttl=0,
                nhop_handle=0,
                session_type=0,
                span_mode=0)
            mirror1 = self.client.switch_api_mirror_session_create(
                device, port_mirror_info)
            self.client.switch_api_port_ingress_mirror_set(
                device, ingress_port_hdl, mirror1)
            port_mirror_enabled = True

            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify ingress port mirrored packet
            verify_packet(self, int_pkt, swports[params.report_ports[0]])
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass packet w/ INT enabled generating ingress port mirror + E2E report"

            # disable int-ep
            self.client.switch_api_dtel_int_endpoint_disable(device)
            int_enabled = False
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt_, swports[1])
            # verify ingress port mirrored packet
            verify_packet(self, int_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass packet w/ INT disabled generating ingress port mirror"

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            if port_mirror_enabled:
                self.client.switch_api_port_ingress_mirror_set(
                    device, ingress_port_hdl, 0)
                self.client.switch_api_mirror_session_delete(device, mirror1)
            if int_enabled:
                self.client.switch_api_dtel_int_endpoint_disable(device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            self.client.switch_api_dtel_int_sink_ports_delete(
                device=device, port=swports[1])
            config.cleanup(self)

@group('ep_l45_egress_port_mirror')
class INTL45_EgressPortMirror_SinkTest(api_base_tests.ThriftInterfaceDataPlane,
                                       pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45 Sink generating DTel report + egress port mirror"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)

        # Add INT watchlist entry
        # session_id = 1, report_all_packets = true (no digest)
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=True, flow_sample_percent=100)
        self.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        # add INT sink port
        self.client.switch_api_dtel_int_sink_ports_add(
            device=device, port=swports[1])

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

        # upstream report packet
        exp_i2e_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
            dropped=0,
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=int_pkt)

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
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=exp_inte2e_inner_1)

        int_enabled = False
        port_mirror_enabled = False

        exp_pkt_ = int_pkt.copy()
        exp_pkt_[IP].ttl-=1
        exp_pkt_[Ether].src=exp_pkt[Ether].src
        exp_pkt_[Ether].dst=exp_pkt[Ether].dst

        try:
            # enable int-ep
            self.client.switch_api_dtel_int_endpoint_enable(device)
            int_enabled = True
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass packet w/ INT enabled generating I2E + E2E reports"

            # create a mirror session
            egress_port_hdl = self.client.switch_api_port_id_to_handle_get(
                device, swports[1])
            report_port_hdl = self.client.switch_api_port_id_to_handle_get(
                device, swports[params.report_ports[0]])

            port_mirror_info = switcht_mirror_info_t(
                session_id=1,
                direction=2,
                egress_port_handle=report_port_hdl,
                mirror_type=SWITCH_MIRROR_TYPE_LOCAL,
                cos=0,
                max_pkt_len=0,
                ttl=0,
                nhop_handle=0,
                session_type=0,
                span_mode=0)
            mirror1 = self.client.switch_api_mirror_session_create(
                device, port_mirror_info)
            self.client.switch_api_port_egress_mirror_set(
                device, egress_port_hdl, mirror1)
            port_mirror_enabled = True

            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
            # verify egress port mirrored packet
            verify_packet(self, exp_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass packet w/ INT enabled generating egress port mirror + I2E report"

            # disable int-ep
            self.client.switch_api_dtel_int_endpoint_disable(device)
            int_enabled = False
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt_, swports[1])
            # verify egress port mirrored packet
            verify_packet(self, exp_pkt_, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass packet w/ INT disabled generating egress port mirror"

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            if port_mirror_enabled:
                self.client.switch_api_port_egress_mirror_set(
                    device, egress_port_hdl, 0)
                self.client.switch_api_mirror_session_delete(device, mirror1)
            if int_enabled:
                self.client.switch_api_dtel_int_endpoint_disable(device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            self.client.switch_api_dtel_int_sink_ports_delete(
                device=device, port=swports[1])
            config.cleanup(self)
