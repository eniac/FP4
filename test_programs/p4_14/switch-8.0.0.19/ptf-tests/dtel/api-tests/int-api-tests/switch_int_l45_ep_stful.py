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
INT L45 source and sink endpoint tests, optionally with Digest.
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

import ptf.mask

device = 0
SID = 0x0ACEFACE

swports = [0, 1, 2]

params = SwitchConfig_Params()
params.switch_id = SID
params.mac_self = '00:77:66:55:44:33'
params.nports = 3
params.ipaddr_inf = ['2.2.0.1',  '1.1.0.1', '10.1.0.4']
params.ipaddr_nbr = ['2.2.0.2', '1.1.0.2', '10.1.0.1']
params.mac_nbr = ['00:11:22:33:44:54', '00:11:22:33:44:55', '00:11:22:33:44:56']
params.report_ports = [2]
params.ipaddr_report_src = ['4.4.4.1']
params.ipaddr_report_dst = ['4.4.4.3']
params.mirror_ids = [555]
params.device = device
params.swports = swports

quantization_shift = MAX_QUANTIZATION

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
kvp_mask = switcht_twl_value_t(value_num=0xfffffff0)
twl_kvp.append(switcht_twl_key_value_pair_t(
    SWITCH_TWL_FIELD_IPV4_SRC, kvp_val, kvp_mask))
kvp_val = switcht_twl_value_t(
    value_num=ipv4Addr_to_i32(params.ipaddr_nbr[1]))
kvp_mask = switcht_twl_value_t(value_num=0xfffffff0)
twl_kvp.append(switcht_twl_key_value_pair_t(
    SWITCH_TWL_FIELD_IPV4_DST, kvp_val, kvp_mask))

###############################################################################

@group('ep_l45')
class INTL45_StFull_SourceTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 StFull Source device"
        prepare_int_l45_bindings()
        p = params.report_ports;
        config = SwitchConfig(self, params)

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(0x8000), max_hop=8)

        # Add INT watchlist
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=False, flow_sample_percent=100)
        self.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        # enable int-ep
        self.client.switch_api_dtel_int_endpoint_enable(device=device)

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
        exp_pkt = simple_udp_packet(
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
            pkt=exp_pkt)

        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=SID, incr_cnt=1)

        # switch_id minus quantized_latency, where the latter is random
        q_latency = 0
        digest = 0xffff & (SID - q_latency)
        exp_pkt = int_l45_packet_add_update_digest(
            Packet=exp_pkt, encoding=digest)
        try:
            send_packet(self, swports[0], str(pkt))

            verify_int_packet(test=self,
                              pkt=exp_pkt,
                              port=swports[1],
                              digest=True)

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
class INTL45_StFull_SinkTest(api_base_tests.ThriftInterfaceDataPlane,
                             pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45/StFull Sink device generating DTel report"
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

        # Add INT watchlist
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=False, flow_sample_percent=100)
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
            int_inst_mask=0x9000,  # swid + qid
            int_inst_cnt=2,
            max_hop_cnt=8,
            pkt=pkt)

        # add 2 hop info to the packet, just random qids
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt_orig, val=0x66666666, incr_cnt=1)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x12345678, incr_cnt=0)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222222, incr_cnt=1)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x87654321, incr_cnt=0)

        # add Digest headers
        digest = 0x6666 ^ 0x2222  # assume zero latency
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
            inner_frame=int_digest_pkt)  # int i2e doesn't remove int_digest

        exp_i2e_pkt_ = ipv4_dtel_pkt(
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

        ip_src=params.ipaddr_nbr[0]
        ip_src = ip_src[0:(ip_src.rfind('.')+1)]+'10'
        int_digest_pkt2=int_digest_pkt.copy()
        exp_pkt2=exp_pkt.copy()
        int_digest_pkt2[IP].src=ip_src
        exp_pkt2[IP].src=ip_src

        exp_i2e_pkt2 = ipv4_dtel_pkt(
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
            inner_frame=int_digest_pkt2)  # int i2e doesn't remove int_digest

        exp_inte2e_inner_1 = postcard_report(
            packet=exp_pkt2,
            switch_id=SID,
            ingress_port=swports[0],
            egress_port=swports[1],
            queue_id=0,
            queue_depth=0,
            egress_tstamp=0)

        exp_e2e_pkt2 = ipv4_dtel_pkt(
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

        exp_pkt = Mask(exp_pkt)
        exp_pkt.set_do_not_care_scapy(TCP, 'chksum')
        exp_pkt2 = Mask(exp_pkt2)
        exp_pkt2.set_do_not_care_scapy(TCP, 'chksum')

        print "Clear bloom filters. Can take up to %d secs." % min_sleeptime
        self.client.switch_api_dtel_flow_state_clear_cycle(
            device=device, cycle=reset_cycle)
        time.sleep(min_sleeptime)

        if DEBUG: raw_input("Press Enter to continue...")
        print "Disable bloom filter clearing."
        self.client.switch_api_dtel_flow_state_clear_cycle(
            device=device, cycle=0)
        time.sleep(2*reset_cycle)

        try:
            # send a test pkt
            send_packet(self, swports[0], str(int_digest_pkt))

            verify_packet(self, exp_pkt, swports[1])

            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)

            # e2e mirror will have random latency. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)

            verify_no_other_packets(self)
            print "Passed for the 1st pkt."

            # send a test pkt
            send_packet(self, swports[0], str(int_digest_pkt2))
            verify_packet(self, exp_pkt2, swports[1])

            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt2, swports[params.report_ports[0]],
                ignore_chksum=True)

            # e2e mirror will have random latency. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt2, swports[params.report_ports[0]],
                ignore_chksum=True)

            verify_no_other_packets(self)
            print "Passed for the 1st pkt of 2nd flow."

            # send the same pkt again
            send_packet(self, swports[0], str(int_digest_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)

            # send the same pkt again
            send_packet(self, swports[0], str(int_digest_pkt2))
            verify_packet(self, exp_pkt2, swports[1])
            verify_no_other_packets(self)

            print "Passed for the 2nd identical pkt w/ high quantization shift."
            # send a packet without digest. It should get both i2e and e2e
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt_, swports[params.report_ports[0]],
                ignore_chksum=True)

            # e2e mirror will have random latency. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)
            verify_no_other_packets(self)
            print "Passed for the 2nd identical pkt without digest."

            # change Digest encoding value to test upstream bloom filer.
            # no actual change in the INT hop info.
            digest = 0x7777 ^ 0x2222  # assume zero latency
            int_digest_pkt = int_l45_packet_add_update_digest(
                Packet=int_pkt, encoding=digest)

            exp_i2e_pkt[INT_META_HDR].rsvd2_digest = int_digest_pkt[INT_META_HDR].rsvd2_digest
            send_packet(self, swports[0], str(int_digest_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)
            verify_no_other_packets(self)

            print "Passed for the 3rd pkt with upstream change in digest."

            # enable bloom filter clearing
            print "Enable bloom filter clearing."
            self.client.switch_api_dtel_flow_state_clear_cycle(
                device=device, cycle=reset_cycle)

            for count in range(0, 3):
                sleep_sec = max(min_sleeptime, reset_cycle * 2)
                time.sleep(sleep_sec)

                # send the same pkt again
                send_packet(self, swports[0], str(int_digest_pkt))
                verify_packet(self, exp_pkt, swports[1])
                verify_int_l45_dtel_packet(
                    self, exp_i2e_pkt, swports[params.report_ports[0]],
                    ignore_chksum=True)
                verify_int_lasthop_dtel_report_packet(
                    self, exp_e2e_pkt, swports[params.report_ports[0]],
                    ignore_chksum=True)
                verify_no_other_packets(self)

                print "Passed bloom filter clearing test %d" % (count + 1)

            time.sleep(min_sleeptime)

            # change quanization shift and re-run 1st and 2nd pkts
            print "Re-setting quantization shift to zero"
            self.client.switch_api_dtel_latency_quantization_shift(
                device=device, quant_shift=0)

            print "clear bloom filter and disable clearing"
            self.client.switch_api_dtel_flow_state_clear_cycle(
                device=device, cycle=0)
            time.sleep(2*reset_cycle)

            print "Send 1st pkt"
            send_packet(self, swports[0], str(int_digest_pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)
            verify_no_other_packets(self)

            print "Passed for the 1st pkt."

            # send the same pkt again
            num_send_pkts = 20
            num_rcvd_pkts = 0
            for i in range(num_send_pkts):
                send_packet(self, swports[0], str(int_digest_pkt))
                verify_packet(self, exp_pkt, swports[1])
                # For high latency sentitivity (zero quantization shift)
                # we will receive last-hop report but no upstream report
                nrcv = receive_int_lasthop_dtel_report_packet(
                    self, exp_e2e_pkt, swports[params.report_ports[0]],
                    ignore_chksum=True)
                if nrcv:
                    num_rcvd_pkts += 1
                verify_no_other_packets(self)

            report_ratio = num_rcvd_pkts * 1.0 / num_send_pkts
            print "Report ratio is", report_ratio
            self.assertTrue(report_ratio > 0.0, "Lasthop report ratio is zero!")

            print "Passed for the 2nd identical pkt w/ zero quantization shift."

            self.client.switch_api_dtel_latency_quantization_shift(
                device=device, quant_shift=MAX_QUANTIZATION)
            print "Send 1st pkt"
            # latency = 0 may cause a new packet
            send_packet(self, swports[0], str(int_digest_pkt))
            verify_packet(self, exp_pkt, swports[1])
            nrcv = receive_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)
            num_rcvd_pkts = 0
            for i in range(num_send_pkts):
                send_packet(self, swports[0], str(int_digest_pkt))
                verify_packet(self, exp_pkt, swports[1])
                # for disabling the latency in report, we should not
                # receive anything (we already got first above)
                nrcv = receive_int_lasthop_dtel_report_packet(
                    self, exp_e2e_pkt, swports[params.report_ports[0]],
                    ignore_chksum=True)
                if nrcv:
                    num_rcvd_pkts += 1
                verify_no_other_packets(self)

            self.assertTrue(num_rcvd_pkts == 0, "disabling latency "
                            "didn't work! I got %d reports" % num_rcvd_pkts)
            print "Passed for disabling latency in flow change detection"

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
class INTL45_StFull_1hopSinkTest(api_base_tests.ThriftInterfaceDataPlane,
                                 pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45/StFull one-hop Sink device generating "
        print "DTel report"
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

        # Add INT watchlist
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=False, flow_sample_percent=100)
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

        payload = 'int l45'
        # make input frame to inject to sink
        pkt = simple_udp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_id=108,
            ip_ttl=64,
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
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=exp_inte2e_inner_1)

        ip_src=params.ipaddr_nbr[0]
        ip_src = ip_src[0:(ip_src.rfind('.')+1)]+'10'
        pkt2=pkt.copy()
        exp_pkt2=exp_pkt.copy()
        pkt2[IP].src=ip_src
        exp_pkt2[IP].src=ip_src

        exp_inte2e_inner_1 = postcard_report(
            packet=exp_pkt2,
            switch_id=SID,
            ingress_port=swports[0],
            egress_port=swports[1],
            queue_id=0,
            queue_depth=0,
            egress_tstamp=0)

        exp_e2e_pkt2 = ipv4_dtel_pkt(
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

        print "Clear bloom filters. Can take up to %d secs." % min_sleeptime
        self.client.switch_api_dtel_flow_state_clear_cycle(
            device=device, cycle=reset_cycle)
        time.sleep(min_sleeptime)

        if DEBUG: raw_input("Press Enter to continue...")
        print "Disable bloom filter clearing."
        self.client.switch_api_dtel_flow_state_clear_cycle(
            device=device, cycle=0)
        time.sleep(2*reset_cycle)

        try:
            # send a test pkt
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])

            # e2e mirror will have random latency. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])

            verify_no_other_packets(self)
            print "Passed for the 1st pkt."

            # send a test pkt from another flow
            send_packet(self, swports[0], str(pkt2))
            verify_packet(self, exp_pkt2, swports[1])

            # e2e mirror will have random latency. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt2, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for the 1st pkt of 2nd flow."

            # send the same pkt again
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)

            # send a test pkt from another flow
            send_packet(self, swports[0], str(pkt2))
            verify_packet(self, exp_pkt2, swports[1])
            verify_no_other_packets(self)

            print "Passed for the 2nd identical pkt w/ high quantization shift."

            # enable bloom filter clearing
            print "Enable bloom filter clearing."

            # first use a very large cycle value and change later
            # to make sure the change goes in immidiately not per cycle
            self.client.switch_api_dtel_flow_state_clear_cycle(
                device, 10000)
            time.sleep(2);

            self.client.switch_api_dtel_flow_state_clear_cycle(
                device=device, cycle=reset_cycle)

            for count in range(0, 3):
                sleep_sec = max(min_sleeptime, reset_cycle * 2)
                time.sleep(sleep_sec)

                # send the same pkt again
                send_packet(self, swports[0], str(pkt))
                verify_packet(self, exp_pkt, swports[1])
                verify_int_lasthop_dtel_report_packet(
                    self, exp_e2e_pkt, swports[params.report_ports[0]])
                verify_no_other_packets(self)

                print "Passed bloom filter clearing test %d" % (count + 1)

            time.sleep(min_sleeptime)

            # change quanization shift and re-run 1st and 2nd pkts
            print "Re-setting quantization shift to zero"

            time.sleep(reset_cycle)

            self.client.switch_api_dtel_latency_quantization_shift(
                device=device, quant_shift=0)

            print "clear bloom filter and disable clearing"
            self.client.switch_api_dtel_flow_state_clear_cycle(
                device=device, cycle=0)
            time.sleep(2*reset_cycle)

            print "Send 1st pkt"
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)

            print "Passed for the 1st pkt."

            # send the same pkt again
            num_send_pkts = 20
            num_rcvd_pkts = 0
            for i in range(num_send_pkts):
                send_packet(self, swports[0], str(pkt))
                verify_packet(self, exp_pkt, swports[1])
                # For high latency sentitivity (zero quantization shift)
                # we will receive last-hop report but no upstream report
                nrcv = receive_int_lasthop_dtel_report_packet(
                    self, exp_e2e_pkt, swports[params.report_ports[0]])
                if nrcv:
                    num_rcvd_pkts += 1
                verify_no_other_packets(self)

            report_ratio = num_rcvd_pkts * 1.0 / num_send_pkts
            print "Report ratio is", report_ratio
            self.assertTrue(report_ratio > 0.0, "Lasthop report ratio is zero!")

            print "Passed for the 2nd identical pkt w/ zero quantization shift."

        finally:
            print "Test Cleanup"
            if DEBUG: raw_input("Before cleanup. Press Enter to continue...")
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


class INTL45_EgressBFilter(api_base_tests.ThriftInterfaceDataPlane,
                           pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):

        print "Test INT L45/StFull Bloom Filter"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        timeout=0.4
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        twl_kvp = []
        int_inst = 0x8000

        # create an INT session
        self.client.switch_api_dtel_int_session_create(
            device=device, session_id=1,
            instruction=convert_int_instruction(int_inst), max_hop=8)

        # Add INT watchlist entry
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=False, flow_sample_percent=100)
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

        payload = 'int l45'
        # make input frame to inject to sink
        init_sport=101
        init_dport=202
        pkt = simple_udp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_id=108,
            ip_ttl=64,
            udp_sport=init_sport,
            udp_dport=init_dport,
            with_udp_chksum=False,
            udp_payload=payload)

        exp_pkt = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_id=108,
            ip_ttl=63,
            udp_sport=init_sport,
            udp_dport=init_dport,
            with_udp_chksum=False,
            udp_payload=payload)

        try:
          for max_iter in [2000, 4000, 8000]:
            print "for %d flows in one-hop Sink device"%max_iter
            print "Clear bloom filters. Can take up to %d secs." % min_sleeptime
            self.client.switch_api_dtel_flow_state_clear_cycle(
              device=device, cycle=reset_cycle)
            time.sleep(min_sleeptime)
            self.client.switch_api_dtel_flow_state_clear_cycle(
              device=device, cycle=0)
            time.sleep(2*reset_cycle)

            # false negative is practically zero (should send but not send)
            # because we keep flow_hash inside entries (16bit) and have 64k
            # entries
            num_rcvd_pkts=0
            fp_indices=[]
            sport=init_sport
            dport=init_dport
            for i in range(max_iter):
                sport=(sport+1) & 0xffff
                if sport==init_sport:
                    dport=(dport+1) & 0xffff
                    sys.stdout.write('/')
                pkt[UDP].sport=sport
                exp_pkt[UDP].sport=sport
                pkt[UDP].dport=dport
                exp_pkt[UDP].dport=dport

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

                send_packet(self, swports[0], str(pkt))
                verify_packet(self, exp_pkt, swports[1])
                nrcv = receive_int_lasthop_dtel_report_packet(
                    self, exp_e2e_pkt,
                    swports[params.report_ports[0]],
                    timeout=timeout)
                verify_no_other_packets(self)
                if nrcv:
                    sys.stdout.write('-')
                else:
                    sys.stdout.write('|')
                    fp_indices.append(i)
                    num_rcvd_pkts += 1
                sys.stdout.flush()
                if (i+1) % 1000==0:
                    print i+1
            print
            print fp_indices

            print "false positive = %d / %d = %f" % \
                (num_rcvd_pkts, max_iter, float(num_rcvd_pkts)/max_iter)
            print "Passed false negative test successuflly"

            # send again and count the # false positive (no change but report)
            num_rcvd_pkts=0
            dup_indices=[]
            sport=init_sport
            dport=init_dport
            for i in range(max_iter):
                sport=(sport+1) & 0xffff
                if sport==init_sport:
                    dport=(dport+1) & 0xffff
                    sys.stdout.write('/')
                pkt[UDP].sport=sport
                exp_pkt[UDP].sport=sport
                pkt[UDP].dport=dport
                exp_pkt[UDP].dport=dport

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

                send_packet(self, swports[0], str(pkt))
                verify_packet(self, exp_pkt, swports[1])
                nrcv = receive_int_lasthop_dtel_report_packet(
                    self, exp_e2e_pkt,
                    swports[params.report_ports[0]],
                    timeout=timeout)
               # verify_no_other_packets(self)
                if nrcv:
                    sys.stdout.write('|')
                    dup_indices.append(i)
                    num_rcvd_pkts += 1
                else:
                    sys.stdout.write('-')
                sys.stdout.flush()
                if (i+1) % 1000==0:
                    print i+1
            print
            print dup_indices

            print "false negative = %d / %d = %f" % \
                (num_rcvd_pkts, max_iter, float(num_rcvd_pkts)/max_iter)

            print "Passed false positive test successuflly"
        finally:
            print "Test Cleanup"
            if DEBUG: raw_input("Before cleanup. Press Enter to continue...")
            ### Cleanup
            cleanup_int_l45_bindings()
            # wait to clear up the filter
            self.client.switch_api_dtel_int_endpoint_disable(device=device)
            self.client.switch_api_dtel_int_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_dtel_int_session_delete(
                device=device, session_id=1)
            self.client.switch_api_dtel_int_sink_ports_delete(
                device=device, port=swports[1])
            config.cleanup(self)

@group('ep_l45')
class INTL45_TCPFLAG_Test(api_base_tests.ThriftInterfaceDataPlane,
                          pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45/StFull Sink device looking at TCP flags"
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

        # Add INT watchlist
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=False, flow_sample_percent=100)
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
            tcp_flags= "S",
            ip_ttl=64,
            tcp_sport=101)
        int_pkt_orig = int_l45_src_packet(
            test=self,
            int_inst_mask=0x9000,  # swid + qid
            int_inst_cnt=2,
            max_hop_cnt=8,
            pkt=pkt)

        # add 2 hop info to the packet, just random qids
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt_orig, val=0x66666666, incr_cnt=1)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x12345678, incr_cnt=0)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222222, incr_cnt=1)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x87654321, incr_cnt=0)

        # add Digest headers
        digest = 0x6666 ^ 0x2222  # assume zero latency
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
            inner_frame=int_digest_pkt)  # int i2e doesn't remove int_digest

        exp_pkt = simple_tcp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            tcp_flags= "S",
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

        print "Clear bloom filters. Can take up to %d secs." % min_sleeptime
        self.client.switch_api_dtel_flow_state_clear_cycle(
            device=device, cycle=reset_cycle)
        time.sleep(min_sleeptime)

        if DEBUG: raw_input("Press Enter to continue...")
        print "Disable bloom filter clearing."
        self.client.switch_api_dtel_flow_state_clear_cycle(
            device=device, cycle=0)
        time.sleep(2*reset_cycle)

        try:
            # send a test pkt
            send_packet(self, swports[0], str(int_digest_pkt))

            m = Mask(exp_pkt)
            m.set_do_not_care_scapy(TCP, 'chksum')
            verify_packet(self, m, swports[1])

            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)

            # e2e mirror will have random latency. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)

            verify_no_other_packets(self)
            print "Passed for the 1st pkt."

            # send the same pkt again
            send_packet(self, swports[0], str(int_digest_pkt))
            verify_packet(self, m, swports[1])
            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]],
                    ignore_chksum=True)

            # e2e mirror will have random latency. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]],
                    ignore_chksum=True)
            verify_no_other_packets(self)
            print "Passed for the 2nd pkt (TCP Syn)"

            if int_digest_pkt.haslayer(TCP_INTL45):
                int_digest_pkt[TCP_INTL45].flags="F"
                exp_i2e_pkt[TCP_INTL45].flags="F"
            else:
                int_digest_pkt[TCP].flags="F"
                exp_i2e_pkt[TCP].flags="F"
            exp_pkt[TCP].flags="F"
            exp_e2e_pkt[TCP].flags="F"
            send_packet(self, swports[0], str(int_digest_pkt))
            m = Mask(exp_pkt)
            m.set_do_not_care_scapy(TCP, 'chksum')
            verify_packet(self, m, swports[1])
            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]],
                    ignore_chksum=True)

            # e2e mirror will have random latency. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]],
                    ignore_chksum=True)
            verify_no_other_packets(self)
            print "Passed for the 2nd pkt (TCP Fin)"

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

@group('ep_l45_inner')
class INTL45_TCPFLAG_inner_Test(api_base_tests.ThriftInterfaceDataPlane,
                          pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45/StFull Sink device looking at TCP flags"
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

        # Add INT watchlist
        ap = switcht_twl_int_params_t(
            session_id=1, report_all_packets=False, flow_sample_percent=100)
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
            tcp_flags= "S",
            ip_ttl=64,
            tcp_sport=101)

        # add Digest headers
        digest = 0x6666 ^ 0x2222  # assume zero latency

        print "Clear bloom filters. Can take up to %d secs." % min_sleeptime
        self.client.switch_api_dtel_flow_state_clear_cycle(
            device=device, cycle=reset_cycle)
        time.sleep(min_sleeptime)

        if DEBUG: raw_input("Press Enter to continue...")
        print "Disable bloom filter clearing."
        self.client.switch_api_dtel_flow_state_clear_cycle(
            device=device, cycle=0)
        time.sleep(2*reset_cycle)

        try:

            pkt_in = pkt
            # make a vxlan packet with inner TCP
            pkt = simple_vxlan_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_id=108,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_ttl=64,
                udp_sport=101,
                with_udp_chksum=False,
                vxlan_vni=0xaaaa,
                inner_frame=pkt_in)
            int_pkt_orig = int_l45_src_packet(
                test=self,
                int_inst_mask=0x9000,  # swid + qid
                int_inst_cnt=2,
                max_hop_cnt=8,
                pkt=pkt)

            # add 2 hop info to the packet, just random qids
            int_pkt = int_l45_packet_add_hop_info(
                Packet=int_pkt_orig, val=0x66666666, incr_cnt=1)
            int_pkt = int_l45_packet_add_hop_info(
                Packet=int_pkt, val=0x12345678, incr_cnt=0)
            int_pkt = int_l45_packet_add_hop_info(
                Packet=int_pkt, val=0x22222222, incr_cnt=1)
            int_pkt = int_l45_packet_add_hop_info(
                Packet=int_pkt, val=0x87654321, incr_cnt=0)

            # add Digest headers
            digest = 0x6666 ^ 0x2222  # assume zero latency
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
                inner_frame=int_digest_pkt)  # int i2e doesn't remove int_digest

            exp_pkt = simple_vxlan_packet(
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_id=108,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_ttl=63,
                udp_sport=101,
                with_udp_chksum=False,
                vxlan_vni=0xaaaa,
                inner_frame=pkt_in)

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
            send_packet(self, swports[0], str(int_digest_pkt))
            m = Mask(exp_pkt)
            m.set_do_not_care_scapy(UDP, 'chksum')
            verify_packet(self, m, swports[1])
            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]],
                    ignore_chksum=True)

            # e2e mirror will have random latency. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]],
                    ignore_chksum=True)
            verify_no_other_packets(self)

            # send packet again
            send_packet(self, swports[0], str(int_digest_pkt))
            verify_packet(self, m, swports[1])
            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]],
                    ignore_chksum=True)

            # e2e mirror will have random latency. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]],
                    ignore_chksum=True)
            verify_no_other_packets(self)
            print "Passed for the 2nd pkt (Inner TCP Fin)"

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
class INTL45_REPORT_DSCP_Test(api_base_tests.ThriftInterfaceDataPlane,
                       pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test INT L45/StFull Sink device with different Report DSCP values"
        print "NOTE: Report DSCP should not overlap with INT L45 DSCP bit"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        config = SwitchConfig(self, params)

        prepare_int_l45_bindings(int_dscp=get_int_l45_dscp_value(),
                                 int_dscp_mask=get_int_l45_dscp_mask())

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
            tcp_flags= "S",
            ip_ttl=64,
            tcp_sport=101)
        int_pkt_orig = int_l45_src_packet(
            test=self,
            int_inst_mask=0x9000,  # swid + qid
            int_inst_cnt=2,
            max_hop_cnt=8,
            pkt=pkt)

        # add 2 hop info to the packet, just random qids
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt_orig, val=0x66666666, incr_cnt=1)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x12345678, incr_cnt=0)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222222, incr_cnt=1)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x87654321, incr_cnt=0)

        # add Digest headers
        digest = 0x6666 ^ 0x2222  # assume zero latency
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
            inner_frame=int_digest_pkt)  # int i2e doesn't remove int_digest

        exp_pkt = simple_tcp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            tcp_flags= "S",
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

        print "Clear bloom filters. Can take up to %d secs." % min_sleeptime
        self.client.switch_api_dtel_flow_state_clear_cycle(
            device=device, cycle=reset_cycle)
        time.sleep(min_sleeptime)

        if DEBUG: raw_input("Press Enter to continue...")
        print "Disable bloom filter clearing."
        self.client.switch_api_dtel_flow_state_clear_cycle(
            device=device, cycle=0)
        time.sleep(2*reset_cycle)

        try:
            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_FLOW_STATE_CHANGE, 5)
            self.assertTrue(self.client.switch_api_dtel_event_get_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_FLOW_STATE_CHANGE)==5)
            self.client.switch_api_dtel_event_set_dscp(device,
                 SWITCH_DTEL_EVENT_TYPE_FLOW_TCPFLAG, 3)
            exp_i2e_pkt[IP].tos = 5<<2
            exp_e2e_pkt[IP].tos = 5<<2
            # send a test pkt
            send_packet(self, swports[0], str(int_digest_pkt))
            m = Mask(exp_pkt)
            m.set_do_not_care_scapy(TCP, 'chksum')
            verify_packet(self, m, swports[1])
            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)
            # e2e mirror will have random latency. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)
            verify_no_other_packets(self)
            print "Passed for the 1st pkt."

            exp_i2e_pkt[IP].tos = 3<<2
            exp_e2e_pkt[IP].tos = 3<<2
            # send a test pkt
            send_packet(self, swports[0], str(int_digest_pkt))
            verify_packet(self, m, swports[1])
            # verify i2e mirrored packet because of TCP
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)
            # e2e mirror will have random latency. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)
            verify_no_other_packets(self)
            print "Passed for the identical pkt from the same flow."

            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_FLOW_TCPFLAG, 6)
            exp_i2e_pkt[IP].tos = 6<<2
            exp_e2e_pkt[IP].tos = 6<<2
            # send a test pkt
            send_packet(self, swports[0], str(int_digest_pkt))
            verify_packet(self, m, swports[1])
            # verify i2e mirrored packe because of TCP
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)
            # e2e mirror will have random latency. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)
            print "Passed for the identical pkt but change dscp."

            # make i2e without digest
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
            exp_i2e_pkt[IP].tos = 6<<2

            # send a test pkt
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, m, swports[1])
            # verify i2e mirrored packe because of TCP
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)
            # e2e mirror will have random latency. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)
            print "Passed for TCP and report all"

            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS, 31)
            exp_i2e_pkt[IP].tos = 31<<2
            exp_e2e_pkt[IP].tos = 31<<2
            # send a test pkt
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, m, swports[1])
            # verify i2e mirrored packet because of TCP
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)
            # e2e mirror will have random latency. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)
            print "Passed for TCP and report all changed dscp"


        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_endpoint_disable(device=device)
            self.client.switch_api_dtel_int_sink_ports_delete(
                device=device, port=swports[1])
            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_FLOW_TCPFLAG, 0)
            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS, 0)
            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_FLOW_STATE_CHANGE, 0)
            config.cleanup(self)
