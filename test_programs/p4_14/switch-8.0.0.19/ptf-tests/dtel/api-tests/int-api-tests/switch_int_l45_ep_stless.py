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
INT L45 source and sink endpoint tests for stateless suppression
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
high_quantization_shift = TM_SHIFT+14
low_quantization_shift = 0

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

if test_param_get('target') == "asic-model":
    reset_cycle = 6
    min_sleeptime = 75
elif test_param_get('target') == "bmv2":
    reset_cycle = 1
    min_sleeptime = 5
else:
    reset_cycle = 1
    min_sleeptime = 1

DEBUG = False
#DEBUG = True

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

@group('ep_l45')
class INTL45_StLess_SourceTest(api_base_tests.ThriftInterfaceDataPlane,
                               pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45 State less source device"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

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
        exp_pkt_simple = simple_udp_packet(
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
            pkt=exp_pkt_simple)

        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=SID, incr_cnt=1)

        exp_e2e_inner_1 = postcard_report(
            packet=exp_pkt_simple,
            switch_id=SID,
            ingress_port=swports[0],
            egress_port=swports[1],
            queue_id=0,
            queue_depth=0,
            egress_tstamp=0)

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
            path_tracking_flow=0,
            hw_id=hw_id,
            inner_frame=exp_e2e_inner_1)

        exp_e2e_pkt_int = ipv4_dtel_pkt(
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
        endpoint_enabled=False
        try:

            print "disable queue report"
            # disable queue alsert
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt_simple, swports[1])
            verify_no_other_packets(self)

            print "add flow for INT Source"
            # create an INT session
            self.client.switch_api_dtel_int_session_create(
                device=device, session_id=1,
                instruction=convert_int_instruction(0x8000), max_hop=8)

            # Add INT watchlist entry
            ap = switcht_twl_int_params_t(
                session_id=1, report_all_packets=True, flow_sample_percent=100)
            self.client.switch_api_dtel_int_watchlist_entry_create(
                device, twl_kvp, priority=1, watch=True, action_params=ap)

            # set quantization shift
            self.client.switch_api_dtel_latency_quantization_shift(
                device=device, quant_shift=high_quantization_shift)

            # enable int-ep
            self.client.switch_api_dtel_int_endpoint_enable(device=device)
            endpoint_enabled=True

            print "enable queue report threshold: high threshold"
            # set queue report with max latency threshold
            self.client.switch_api_dtel_queue_report_create(
                device, swports[1], 0,
                hex_to_i32(0xfff), hex_to_i32(0xffffffff), 1024, False)
            queue_report_enabled = True

            # high threshould should not generate report
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)

            print "enable queue report threshold: low threshold"
            # set queue report with latency threshold 0
            self.client.switch_api_dtel_queue_report_update(
                device, swports[1], 0, 0, 0, 1024, False)
            # low threshold generates report
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt_int, swports[params.report_ports[0]])
            verify_no_other_packets(self)

            print "disable queue report"
            # disable queue alsert
            self.client.switch_api_dtel_queue_report_delete(
                device, swports[1], 0)
            queue_report_enabled = False
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)

            print "disable INT endpoint"
            self.client.switch_api_dtel_int_endpoint_disable(device=device)
            endpoint_enabled=False
            print "enable queue report threshold: high threshold"
            # set queue report with max latency threshold
            self.client.switch_api_dtel_queue_report_create(
                device, swports[1], 0,
                hex_to_i32(0xfff), hex_to_i32(0xffffffff), 1024, False)
            queue_report_enabled = True

            # high threshould should not generate report
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt_simple, swports[1])
            verify_no_other_packets(self)

            print "enable queue report threshold: low threshold"
            # set queue report with latency threshold 0
            self.client.switch_api_dtel_queue_report_update(
                device, swports[1], 0, 0, 0, 1024, False)
            # low threshold generates report
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt_simple, swports[1])
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)

            print "disable queue report"
            # disable queue alsert
            self.client.switch_api_dtel_queue_report_delete(
                device, swports[1], 0)
            queue_report_enabled = False
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt_simple, swports[1])
            verify_no_other_packets(self)

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            if queue_report_enabled:
                self.client.switch_api_dtel_queue_report_delete(
                    device, swports[1], 0)
            if endpoint_enabled:
                self.client.switch_api_dtel_int_endpoint_disable(device=device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            config.cleanup(self)

@group('ep_l45')
class INTL45_StLess_SinkTest(api_base_tests.ThriftInterfaceDataPlane,
                                  pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45 Sink device - INT/StLess"
        print "local queue report"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=high_quantization_shift)


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

        # add Digest headers
        digest = 0x66666666 ^ 0x22222222  # assume zero latency
        int_digest_pkt = int_l45_packet_add_update_digest(
            Packet=int_pkt, encoding=digest)

        # upstream report packet
        # add INT meta header and 2 hop info to the inner frame
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

        # local report packet
        pkt = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_id=108,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=63,
            udp_sport=101,
            with_udp_chksum=False,
            udp_payload=payload)

        exp_pkt = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_id=108,
            ip_ttl=63,
            udp_sport=101,
            with_udp_chksum=False,
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
            path_tracking_flow=0,
            hw_id=hw_id,
            inner_frame=exp_inte2e_inner_1)

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

        exp_e2e_pkt_ = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
            dropped=0,
            congested_queue=0,
            path_tracking_flow=0,
            hw_id=hw_id,
            inner_frame=exp_inte2e_inner_1)

        print "Clear bloom filters. Can take up to %d secs." % min_sleeptime
        self.client.switch_api_dtel_flow_state_clear_cycle(
            device=device, cycle=reset_cycle)
        time.sleep(min_sleeptime)

        if DEBUG: raw_input("Press Enter to continue...")
        print "Disable bloom filter clearing."
        self.client.switch_api_dtel_flow_state_clear_cycle(
                device=device, cycle=0)
        time.sleep(2*reset_cycle)

        queue_report_enabled = False
        endpoint_enabled = False
        try:
            ######## simple packet at end-point is tested in source test case
            ######## int packet without  enabling end-point
            # send a test pkt
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt_, swports[1])
            verify_no_other_packets(self)

            # send the packet again but now it should only have downstream
            print "enable queue report threshold: high threshold"
            # set queue report with max latency threshold
            self.client.switch_api_dtel_queue_report_create(
                device, swports[1], 0,
                hex_to_i32(0xfff), hex_to_i32(0xffffffff), 1024, False)
            queue_report_enabled = True

            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt_, swports[1])

            verify_no_other_packets(self)

            print "enable queue report threshold: low threshold"
            # set queue report with latency threshold 0
            self.client.switch_api_dtel_queue_report_update(
                device, swports[1], 0, 0, 0, 1024, False)

            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt_, swports[1])

            exp_e2e_pkt_[DTEL_REPORT_HDR].congested_queue = 1
            # verify e2e mirrored packet
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt_, swports[params.report_ports[0]])

            verify_no_other_packets(self)
            print "Passed for queue report for an INT packet w/o enable int endpoint"

            ######## an INT packet without digest
            # reset quota
            self.client.switch_api_dtel_queue_report_update(
                device, swports[1], 0, 0, 0, 1024, False)
            # enable int-ep
            exp_e2e_pkt[DTEL_REPORT_HDR].congested_queue = 1
            exp_e2e_pkt[DTEL_REPORT_HDR].path_tracking_flow = 1
            self.client.switch_api_dtel_int_endpoint_enable(device=device)
            # add INT sink port
            self.client.switch_api_dtel_int_sink_ports_add(
                device=device, port=swports[1])
            endpoint_enabled=True

            # send a test pkt
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])

            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])

            # verify e2e mirrored packet
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)

            # if queue >= threshold and we generate INT report, we still
            # update the quota
            quota = self.client.switch_api_dtel_queue_remaining_report_quota_during_breach_get(
                device, swports[1], 0);
            self.assertTrue(quota == 1023, "Remaining quota is not decremented")

            # send the packet again but now it should only have downstream
            print "enable queue report threshold: high threshold"
            exp_e2e_pkt[DTEL_REPORT_HDR].congested_queue = 0
            # set queue report with max latency threshold
            self.client.switch_api_dtel_queue_report_update(
                device, swports[1], 0,
                hex_to_i32(0xfff), hex_to_i32(0xffffffff), 1024, False)
            queue_report_enabled = True

            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])

            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])

            # verify e2e mirrored packet
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])

            verify_no_other_packets(self)

            print "enable queue report threshold: low threshold"
            # set queue report with latency threshold 0
            self.client.switch_api_dtel_queue_report_update(
                device, swports[1], 0, 0, 0, 1024, False)

            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])

            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])

            exp_e2e_pkt[DTEL_REPORT_HDR].congested_queue = 1
            # verify e2e mirrored packet
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])

            verify_no_other_packets(self)
            print "Passed for local stateless suppression for packet w/o digest"

            ####################################################################
            # int packet with digest
            # -> no upstream but downstream for stless with low threshold
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
                inner_frame=int_digest_pkt)

            print "Cleanup and disable the bloom filters"
            # wait to clear up the filter
            self.client.switch_api_dtel_flow_state_clear_cycle(
                device=device, cycle=reset_cycle)
            time.sleep(min_sleeptime)
            self.client.switch_api_dtel_flow_state_clear_cycle(
                device=device, cycle=0)
            time.sleep(2*reset_cycle)

            send_packet(self, swports[0], str(int_digest_pkt))
            verify_packet(self, exp_pkt, swports[1])

            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])

            # verify e2e mirrored packet
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)

            # send the packet again but now it should only have downstream
            # if threshold is low
            print "enable queue report threshold: high threshold"
            exp_e2e_pkt[DTEL_REPORT_HDR].congested_queue = 0
            # set queue report with max latency threshold
            self.client.switch_api_dtel_queue_report_update(
                device, swports[1], 0,
                hex_to_i32(0xfff), hex_to_i32(0xffffffff), 1024, False)

            send_packet(self, swports[0], str(int_digest_pkt))
            verify_packet(self, exp_pkt, swports[1])

            # no upstream or downstream report

            verify_no_other_packets(self)

            print "enable queue report threshold: low threshold"
            # set queue report with latency threshold 0
            self.client.switch_api_dtel_queue_report_update(
                device, swports[1], 0, 0, 0, 1024, False)

            send_packet(self, swports[0], str(int_digest_pkt))
            verify_packet(self, exp_pkt, swports[1])

            # only downstream report

            # verify e2e mirrored packet
            exp_e2e_pkt[DTEL_REPORT_HDR].congested_queue = 1
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])

            verify_no_other_packets(self)

            print "disable queue report"
            # disable queue alsert
            self.client.switch_api_dtel_queue_report_delete(
                device, swports[1], 0)
            queue_report_enabled = False
            send_packet(self, swports[0], str(int_digest_pkt))
            verify_packet(self, exp_pkt, swports[1])

            # no upstream or downstream as suppressed by stful suppression

            verify_no_other_packets(self)
            print "Passed for local stateless suppression for packet w/ digest"

        finally:
            ### Cleanup
            print "Test Cleanup"
            cleanup_int_l45_bindings()
            if queue_report_enabled:
                self.client.switch_api_dtel_queue_report_delete(
                    device, swports[1], 0)
            if endpoint_enabled:
              self.client.switch_api_dtel_int_sink_ports_delete(
                device=device, port=swports[1])
              self.client.switch_api_dtel_int_endpoint_disable(device=device)
            config.cleanup(self)
