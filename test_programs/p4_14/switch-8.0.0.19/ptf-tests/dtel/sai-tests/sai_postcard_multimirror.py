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
Thrift SAI interface INT EP tests
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

SID = 0x0ACEFACE
swports = range(3)
devports = range(3)

params = SwitchSAIConfig_Params()
params.swports = swports
params.switch_id = SID
params.mac_self = '00:77:66:55:44:33'
params.nports = 3
params.ipaddr_inf = ['2.2.0.1',  '1.1.0.1', '10.1.0.4']
params.ipaddr_nbr = ['2.2.0.200', '1.1.0.100', '10.1.0.1']
params.mac_nbr = ['00:11:22:33:44:54', '00:11:22:33:44:55', '00:11:22:33:44:56']
params.report_ports = [2, 2, 2, 2]
params.report_src = '4.4.4.1'
params.report_dst = ['4.4.4.2', '4.4.4.3', '4.4.4.4', '4.4.4.5']
params.report_udp_port = UDP_PORT_DTEL_REPORT
params.report_truncate_size = 512
params.configure_routes = True

if test_param_get('target') == "asic-model":
    reset_cycle = 6
    min_sleeptime = 75
elif test_param_get('target') == "bmv2":
    reset_cycle = 1
    min_sleeptime = 5
else:
    reset_cycle = 1
    min_sleeptime = 1

@group('postcard_multimirror')
class SAI_Postcard_MultiMiror_Test(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Test postcard"
        bind_postcard_pkt()

        # create SAI manager
        sai_mgr = SAIManager(self, params)

        # create report session
        sai_mgr.create_dtel_report_session()

        flow_watchlist = sai_mgr.create_dtel_watchlist('Flow')

        flow_watchlist_entry = sai_mgr.create_dtel_watchlist_entry(
            watchlist=flow_watchlist,
            priority=10,
            ip_src=params.ipaddr_nbr[0],
            ip_src_mask='255.255.255.0',
            ip_dst=params.ipaddr_nbr[1],
            ip_dst_mask='255.255.255.0',
            dtel_postcard_enable=True,
            dtel_report_all=True,
            dtel_sample_percent=100)

        sai_mgr.switch.dtel_postcard_enable = True
        sai_mgr.switch.dtel_latency_sensitivity = MAX_QUANTIZATION
        sai_mgr.switch.dtel_flow_state_clear_cycle = 0

	    # run test
        try:
            max_itrs = 100
            random.seed(314159)
            mirror_sessions_num = len(params.report_ports)
            count = [0] * mirror_sessions_num
            mirror_ports = [0] * mirror_sessions_num
            selected_mirrors=[]
            exp_postcard_pkt = [None] * mirror_sessions_num
            for i in range(0, mirror_sessions_num):
                mirror_ports[i] = swports[params.report_ports[i]]

            payload = 'postcard multi-mirror'
            for i in range(0, max_itrs):
                src_port = i + 10000
                dst_port = i + 10001

                pkt_in = simple_udp_packet(
                    eth_dst=params.mac_self,
                    eth_src=params.mac_nbr[0],
                    ip_dst=params.ipaddr_nbr[1],
                    ip_src=params.ipaddr_nbr[0],
                    ip_id=105,
                    ip_ttl=64,
                    udp_sport=src_port,
                    udp_dport=dst_port,
                    with_udp_chksum=False,
                    udp_payload=payload)

                exp_pkt_out = simple_udp_packet(
                    eth_dst=params.mac_nbr[1],
                    eth_src=params.mac_self,
                    ip_dst=params.ipaddr_nbr[1],
                    ip_src=params.ipaddr_nbr[0],
                    ip_id=105,
                    ip_ttl=63,
                    udp_sport=src_port,
                    udp_dport=dst_port,
                    with_udp_chksum=False,
                    udp_payload=payload)

                exp_postcard_inner = postcard_report(
                    packet=exp_pkt_out,
                    switch_id=SID,
                    ingress_port=swports[0],
                    egress_port=swports[1],
                    queue_id=0,
                    queue_depth=0,
                    egress_tstamp=0)

                for j in range (0, len(params.report_ports)):
                    devport = swport_to_devport(
                        self, swports[params.report_ports[j]])
                    exp_postcard_pkt[j] = ipv4_dtel_pkt(
                        eth_dst=params.mac_nbr[params.report_ports[j]],
                        eth_src=params.mac_self,
                        ip_src=params.report_src,
                        ip_dst=params.report_dst[j],
                        ip_id=0,
                        ip_ttl=64,
                        next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                        dropped=0,
                        congested_queue=0,
                        path_tracking_flow=1,
                        hw_id=get_pipeid(devport),
                        inner_frame=exp_postcard_inner)

                # send a test packet
                send_packet(self, swports[0], str(pkt_in))
                # verify packet out
                verify_packet(self, exp_pkt_out, swports[1])
                # verify potcard packet
                rcv_idx = verify_any_dtel_packet_any_port(
                    self, exp_postcard_pkt, mirror_ports)
                #receive_print_packet(
                #    self, swports[params.report_ports[0]], exp_postcard_pkt[0], True)
                print ("%d %d" % (i, rcv_idx))
                selected_mirrors.append(rcv_idx)
                #verify_no_other_packets(self, timeout=1)
                count[rcv_idx] += 1

            for i in range(0, mirror_sessions_num):
                self.assertTrue((count[i] >= ((max_itrs / 4.0) * 0.50)),
                                "Not all mirror sessions are equally balanced"
                                " (%f %% < %f) for %d" %
                                (count[i], ((max_itrs / 4.0) * 0.50), i))

            print "passed balancing the load among telemery mirror sessions"

        # cleanup
        finally:
            split_postcard_pkt()
            sai_mgr.switch.dtel_postcard_enable = False
            sai_mgr.cleanup()
