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
INT L45 sink testing multiple mirror sessions
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
quantization_shift = MAX_QUANTIZATION

params = SwitchConfig_Params()
params.nports = 3
swports = [0, 1, 2]

params.switch_id = SID
params.mac_self = '00:77:66:55:44:33'
params.ipaddr_inf = ['2.2.0.1',  '1.1.0.1', '10.1.0.1']
params.ipaddr_nbr = ['2.2.0.200', '1.1.0.100', '10.1.0.4']
params.mac_nbr = ['00:11:22:33:44:54', '00:11:22:33:44:55', '00:11:22:33:44:56']
params.report_ports = [2, 2, 2, 2]
params.ipaddr_report_src = ['4.4.4.1', '4.4.4.1', '4.4.4.1', '4.4.4.1']
params.ipaddr_report_dst = ['4.4.4.3', '4.4.4.4', '4.4.4.5', '4.4.4.6']
params.mirror_ids = [555, 556, 557, 558]
params.device = device
params.swports = swports

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

@group('postcard')
class Postcard_MirrorTest(api_base_tests.ThriftInterfaceDataPlane,
                        pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test Postcard, sending reports to multiple destinations"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        bind_postcard_pkt()
        config = SwitchConfig(self, params)

        # enable Postcard
        self.client.switch_api_dtel_postcard_enable(device)
        self.client.switch_api_dtel_latency_quantization_shift(
            device, quantization_shift)

        # add flow space to postcard watchlist
        ap = switcht_twl_postcard_params_t(
            report_all_packets=True, flow_sample_percent=100)
        self.client.switch_api_dtel_postcard_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        try:
            max_itrs = 100
            random.seed(314159)
            mirror_sessions_num = len(params.report_ports)
            count = [0] * mirror_sessions_num
            mirror_ports = [0] * mirror_sessions_num
            selected_mirrors=[]
            exp_postcard_pkt_1 = [None] * mirror_sessions_num
            for i in range(0, mirror_sessions_num):
                self.client.switch_api_dtel_report_sequence_number_set(
                  device, params.mirror_ids[i], 0)
                mirror_ports[i] = swports[params.report_ports[i]]
            for i in range(0, max_itrs):
                src_port = i + 10000
                dst_port = i + 10001
                # make input frame to inject to sink
                pkt = simple_udp_packet(
                    eth_dst=params.mac_self,
                    eth_src=params.mac_nbr[0],
                    ip_id=108,
                    ip_dst=params.ipaddr_nbr[1],
                    ip_src=params.ipaddr_nbr[0],
                    ip_ttl=64,
                    udp_sport=src_port,
                    udp_dport=dst_port,
                    with_udp_chksum=False)
                exp_pkt = simple_udp_packet(
                    eth_dst=params.mac_nbr[1],
                    eth_src=params.mac_self,
                    ip_dst=params.ipaddr_nbr[1],
                    ip_src=params.ipaddr_nbr[0],
                    ip_id=108,
                    ip_ttl=63,
                    udp_sport=src_port,
                    udp_dport=dst_port,
                    with_udp_chksum=False)

                exp_postcard_inner_1 = postcard_report(
                    packet=exp_pkt,
                    switch_id=params.switch_id,
                    ingress_port=swports[0],
                    egress_port=swports[1],
                    queue_id=0,
                    queue_depth=0,
                    egress_tstamp=0)

                for j in range (0, len(params.report_ports)):
                    exp_postcard_pkt_1[j] = ipv4_dtel_pkt(
                        eth_dst=params.mac_nbr[params.report_ports[j]],
                        eth_src=params.mac_self,
                        ip_src=params.ipaddr_report_src[j],
                        ip_dst=params.ipaddr_report_dst[j],
                        ip_id=0,
                        ip_ttl=64,
                        next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                        dropped=0,
                        congested_queue=0,
                        path_tracking_flow=1,
                        sequence_number=count[j],
                        hw_id=hw_id,
                        inner_frame=exp_postcard_inner_1)

                send_packet(self, swports[0], str(pkt))
                verify_packet(self, exp_pkt, swports[1])

                rcv_idx1 = verify_any_dtel_packet_any_port(
                    self, exp_postcard_pkt_1,
                    mirror_ports, ignore_seq_num=False)
                print ("%d %d" % (i, rcv_idx1))
                selected_mirrors.append(rcv_idx1)
                #verify_no_other_packets(self, timeout=1)
                count[rcv_idx1] += 1

            for i in range(0, mirror_sessions_num):
                self.assertTrue((count[i] >= ((max_itrs / 4.0) * 0.50)),
                                "Not all mirror sessions are equally balanced"
                                " (%f %% < %f) for %d" %
                                (count[i], ((max_itrs / 4.0) * 0.50), i))

            print "passed balancing the load among telemery mirror sessions"

            # add mirror ids in reverse order
            for mirror_id in params.mirror_ids:
                self.client.switch_api_dtel_report_session_delete(
                    device,
                    mirror_id)

            for i in range(mirror_sessions_num - 1, -1, -1):
                self.client.switch_api_dtel_report_session_add(
                    device,
                    params.mirror_ids[i])

            for i in range(0, max_itrs):
                src_port = i + 10000
                dst_port = i + 10001

                pkt = simple_udp_packet(
                    eth_dst=params.mac_self,
                    eth_src=params.mac_nbr[0],
                    ip_id=108,
                    ip_dst=params.ipaddr_nbr[1],
                    ip_src=params.ipaddr_nbr[0],
                    ip_ttl=64,
                    udp_sport=src_port,
                    udp_dport=dst_port,
                    with_udp_chksum=False)
                exp_pkt = simple_udp_packet(
                    eth_dst=params.mac_nbr[1],
                    eth_src=params.mac_self,
                    ip_dst=params.ipaddr_nbr[1],
                    ip_src=params.ipaddr_nbr[0],
                    ip_id=108,
                    ip_ttl=63,
                    udp_sport=src_port,
                    udp_dport=dst_port,
                    with_udp_chksum=False)

                exp_postcard_inner_1 = postcard_report(
                    packet=exp_pkt,
                    switch_id=params.switch_id,
                    ingress_port=swports[0],
                    egress_port=swports[1],
                    queue_id=0,
                    queue_depth=0,
                    egress_tstamp=0)


                for j in range (0, len(params.report_ports)):
                    exp_postcard_pkt_1[j] = ipv4_dtel_pkt(
                        eth_dst=params.mac_nbr[params.report_ports[j]],
                        eth_src=params.mac_self,
                        ip_src=params.ipaddr_report_src[j],
                        ip_dst=params.ipaddr_report_dst[j],
                        ip_id=0,
                        ip_ttl=64,
                        next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                        dropped=0,
                        congested_queue=0,
                        path_tracking_flow=1,
                        hw_id=hw_id,
                        inner_frame=exp_postcard_inner_1)

                send_packet(self, swports[0], str(pkt))
                verify_packet(self, exp_pkt, swports[1])

                rcv_idx1 = verify_any_dtel_packet_any_port(
                    self, exp_postcard_pkt_1,
                    mirror_ports)
                self.assertTrue(rcv_idx1 == selected_mirrors[i],
                                "Did not recevie packet on the same mirror"
                                " session when changing the mirror add order"
                                " exp=%d, in=%d"%(selected_mirrors[i],
                                                  rcv_idx1))
                print ("%d %d" % (i, rcv_idx1))

            print "passed consistent mirroring with different order for adding mirror IDs"

        finally:
            split_postcard_pkt()
            self.client.switch_api_dtel_postcard_disable(device)
            self.client.switch_api_dtel_postcard_watchlist_clear(device)
            config.cleanup(self)
