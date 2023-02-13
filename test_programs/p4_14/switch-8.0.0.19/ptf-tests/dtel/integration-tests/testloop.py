###############################################################################
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
Postcard tests
"""

import logging
import os
import random
import switchapi_thrift
import sys
import time
import unittest
import threading

import ptf.dataplane as dataplane
from scapy.all import *
from erspan3 import *
from ptf.testutils import *
from ptf.thriftutils import *
from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../base/api-tests'))
import api_base_tests

from constants import *
sys.path.append(os.path.join(this_dir, '..'))
from dtel_utils import *
#from switch_config import *

################################################################################
@group('loop_leaf')
class loop_LeafTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print 'test postcard on leaf of loop'
        c=ConfigLeafALoop()
        params=c.getParams()
        swports=params.swports;
        bind_postcard_pkt()
        pkt_send = simple_udp_packet(
            eth_dst = mac_h41_1,
            eth_src = mac_h41_0,
            ip_dst =  ip_h41_1,
            ip_src =  ip_h41_0,
            ip_id = 105,
            ip_ttl = 64,
            pktlen = 256,
            with_udp_chksum = False)

        exp_pkt_recv = simple_udp_packet(
            eth_dst = mac_h41_1, # l2 routing doesn't change mac
            eth_src = mac_h41_0, # l2 routing doesn't change mac
            ip_dst =  ip_h41_1,
            ip_src =  ip_h41_0,
            ip_id = 105,
            ip_ttl = 64,
            pktlen = 256,
            with_udp_chksum = False)

        exp_postcard_inner_1 = postcard_report(
                packet=exp_pkt_recv,
                switch_id=params.switch_id,
                ingress_port=swports[0],
                egress_port=swports[2],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

        exp_poscard_pkt_1 = ipv4_erspan_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=mac_s31_r,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                version=2,   # ERSPAN III
                mirror_id=params.erspan_span_id,
                sgt_other=ERSPAN_FT_D_OTHER_POSTCARD,
                inner_frame=exp_postcard_inner_1)

        send_packet(self, swports[0], str(pkt_send))
        verify_packet(self, exp_pkt_recv, swports[2])
        verify_postcard_packet(self, exp_poscard_pkt_1,
            swports[params.report_ports[0]])
        #receive_print_packet(self, swports[2], exp_pkt_recv, False, False)

        # reverse path
        exp_postcard_inner_1 = postcard_report(
                packet=exp_pkt_recv,
                switch_id=params.switch_id,
                ingress_port=swports[3],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

        exp_poscard_pkt_1 = ipv4_erspan_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=mac_s31_r,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                version=2,   # ERSPAN III
                mirror_id=params.erspan_span_id,
                sgt_other=ERSPAN_FT_D_OTHER_POSTCARD,
                inner_frame=exp_postcard_inner_1)

        send_packet(self, swports[3], str(pkt_send))
        verify_packet(self, exp_pkt_recv, swports[1])
        verify_postcard_packet(self, exp_poscard_pkt_1,
            swports[params.report_ports[0]])

################################################################################
@group('loop_spine')
class loop_SpineTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print 'test postcard on a spine of loop'
        c=ConfigSpineLoop()
        params=c.getParams()
        swports=params.swports;
        bind_postcard_pkt()
        pkt_send = simple_udp_packet(
            eth_dst = mac_h41_1,
            eth_src = mac_h41_0,
            ip_dst =  ip_h41_1,
            ip_src =  ip_h41_0,
            ip_id = 105,
            ip_ttl = 64,
            pktlen = 256,
            with_udp_chksum = False)

        exp_pkt_recv = simple_udp_packet(
            eth_dst = mac_h41_1, # l2 routing doesn't change mac
            eth_src = mac_h41_0, # l2 routing doesn't change mac
            ip_dst =  ip_h41_1,
            ip_src =  ip_h41_0,
            ip_id = 105,
            ip_ttl = 64,
            pktlen = 256,
            with_udp_chksum = False)

        exp_postcard_inner_1 = postcard_report(
                packet=exp_pkt_recv,
                switch_id=params.switch_id,
                ingress_port=swports[1],
                egress_port=swports[2],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

        exp_poscard_pkt_1 = ipv4_erspan_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=mac_s31_r,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                version=2,   # ERSPAN III
                mirror_id=params.erspan_span_id,
                sgt_other=ERSPAN_FT_D_OTHER_POSTCARD,
                inner_frame=exp_postcard_inner_1)

        send_packet(self, swports[1], str(pkt_send))
        verify_packet(self, exp_pkt_recv, swports[2])
        verify_postcard_packet(self, exp_poscard_pkt_1,
            swports[params.report_ports[0]])
        #receive_print_packet(self, swports[2], exp_pkt_recv, False, False)

        # verify routing erspan from leaf
        leaf_c=ConfigLeafALoop()
        leaf_params=leaf_c.getParams()

        exp_pkt_recv = simple_udp_packet(
            eth_dst = mac_h41_1,
            eth_src = mac_h41_0,
            ip_dst =  ip_h41_1,
            ip_src =  ip_h41_0,
            ip_id = 105,
            ip_ttl = 64,
            pktlen = 256,
            with_udp_chksum = False)

        exp_postcard_inner_1 = postcard_report(
                packet=exp_pkt_recv,
                switch_id=leaf_params.switch_id,
                ingress_port=leaf_params.swports[0],
                egress_port=leaf_params.swports[2],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

        exp_poscard_pkt_1 = ipv4_erspan_pkt(
                eth_dst=leaf_params.mac_nbr[leaf_params.report_ports[0]],
                eth_src=mac_s31_r,
                ip_src=leaf_params.ipaddr_report_src[0],
                ip_dst=leaf_params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                version=2,   # ERSPAN III
                mirror_id=leaf_params.erspan_span_id,
                sgt_other=ERSPAN_FT_D_OTHER_POSTCARD,
                inner_frame=exp_postcard_inner_1)

        exp_pkt = ipv4_erspan_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=mac_s31_r,
                ip_src=leaf_params.ipaddr_report_src[0],
                ip_dst=leaf_params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=63,
                version=2,   # ERSPAN III
                mirror_id=leaf_params.erspan_span_id,
                sgt_other=ERSPAN_FT_D_OTHER_POSTCARD,
                inner_frame=exp_postcard_inner_1)

        send_packet(self, swports[1], str(exp_poscard_pkt_1))
        print swports[params.report_ports[0]]
        verify_packet(self, exp_pkt, swports[params.report_ports[0]])
