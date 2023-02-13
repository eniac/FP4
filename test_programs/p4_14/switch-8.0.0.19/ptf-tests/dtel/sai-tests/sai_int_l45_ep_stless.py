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
params.report_ports = [2]
params.report_src = '4.4.4.1'
params.report_dst = ['4.4.4.3']
params.report_udp_port = UDP_PORT_DTEL_REPORT
params.report_truncate_size = 256
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

###############################################################################

@group('ep_l45')
class SAI_INTL45_StLess_SourceTest(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 State less source device"
        prepare_int_l45_bindings()
        p = params.report_ports;
        # create SAI manager
        sai_mgr = SAIManager(self, params)

        # create report session according to params
        sai_mgr.create_dtel_report_session()

        # create INT session
        int_session = sai_mgr.create_dtel_int_session(
            max_hop_count=8,
            collect_switch_id=True,
            collect_switch_ports=False,
            collect_ig_timestamp=False,
            collect_eg_timestamp=False,
            collect_queue_info=False)

        flow_watchlist = sai_mgr.create_dtel_watchlist('Flow')

        flow_watchlist_entry = sai_mgr.create_dtel_watchlist_entry(
            watchlist=flow_watchlist,
            priority=10,
            ip_src=params.ipaddr_nbr[0],
            ip_src_mask='255.255.255.0',
            ip_dst=params.ipaddr_nbr[1],
            ip_dst_mask='255.255.255.0',
            dtel_int_enable=True,
            dtel_int_session=int_session,
            dtel_report_all=True)

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
            ip_src=params.report_src,
            ip_dst=params.report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
            dropped=0,
            congested_queue=1,
            path_tracking_flow=1,
            hw_id=get_pipeid(
                swport_to_devport(self, swports[params.report_ports[0]])),
            inner_frame=exp_inte2e_inner_1)

        queue_alert_enabled = False
        try:
            sai_mgr.switch.dtel_int_endpoint_enable = True
            print "enable queue alert threshold: high threshold"
            # set queue alert with max latency threshold
            queue_report = sai_mgr.create_dtel_queue_report(
            swports[1], 0, hex_to_i32(0xfff),
            hex_to_i32(0xffffffff), 1024, True)
            queue_alert_enabled = True

            # high threshould should not generate report
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)

            print "enable queue alert threshold: low threshold"
            # set queue alert with latency threshold 0
            queue_report.depth_threshold = 0
            queue_report.latency_threshold = 0
            # low threshold generates report
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)

            print "disable queue alert"
            # disable queue alsert
            queue_report.delete()
            queue_alert_enabled = False
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            sai_mgr.switch.dtel_int_endpoint_enable = False
            sai_mgr.cleanup()
            params.report_ports = p
