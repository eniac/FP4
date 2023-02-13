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
SONiC tests for queue report
"""


import logging
import os
import random
import switchapi_thrift
import sys
import time
import unittest
import threading

# import ptf.dataplane as dataplane
# from scapy.all import *
# from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *
from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

import pdb

from switch_ptf_config import *

SID = 0x0ACEFACE
switch_id = SID

reset_cycle = 1
min_sleeptime = 1

# Whether to wait for user input before cleaning up in each test case
debug_mode = False

###############################################################################

@group('queue')
@group('int_ep')
@group('int_transit')
@group('postcard')
class SONiC_QueueReport_Quota_Test(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test queue alert quota"
        bind_postcard_pkt() # just to parse report

        # Value of dtel_monitoring_type is irrelevant since neither postcard
        # nor INT will be enabled, and no 'flow' watchlist will be configured
        switch = sonic_switch.SONiCSwitch(dtel_monitoring_type='postcard',
                                          dtel_switch_id=switch_id,
                                          management_ip=switch_ip)
        switch.dtel_int_l4_dscp = {'value': get_int_l45_dscp_value(),
                                   'mask': get_int_l45_dscp_mask()}
        payload = 'qreport'
        # make input frame to inject to sink
        pkt = simple_udp_packet(
            eth_dst=mac_self,
            eth_src=mac_nbr[0],
            ip_id=108,
            ip_dst=ipaddr_nbr[1],
            ip_src=ipaddr_nbr[0],
            ip_ttl=64,
            udp_sport=101,
            with_udp_chksum=False,
            pktlen=256,
            udp_payload=payload)

        exp_pkt = simple_udp_packet(
            eth_dst=mac_nbr[1],
            eth_src=mac_self,
            ip_dst=ipaddr_nbr[1],
            ip_src=ipaddr_nbr[0],
            ip_id=108,
            ip_ttl=63,
            pktlen=256,
            udp_sport=101,
            with_udp_chksum=False,
            udp_payload=payload)

        exp_e2e_inner_1 = postcard_report(
            packet=exp_pkt,
            switch_id=SID,
            ingress_port=swports[0],
            egress_port=swports[1],
            queue_id=0,
            queue_depth=0,
            egress_tstamp=0)

        exp_e2e_pkt = ipv4_dtel_pkt(
            eth_dst=mac_nbr[report_ports[0]],
            eth_src=mac_self,
            ip_src=report_src,
            ip_dst=report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
            dropped=0,
            congested_queue=1,
            path_tracking_flow=0,
            hw_id=get_pipeid(devports[report_ports[0]]),
            inner_frame=exp_e2e_inner_1)

        exp_mod_inner_1 = mod_report(
            packet=exp_pkt,
            switch_id=SID,
            ingress_port=swports[0],
            egress_port=swports[1],
            queue_id=0,
            drop_reason=92)  # drop egress acl deny

        exp_mod_pkt = ipv4_dtel_pkt(
            eth_dst=mac_nbr[report_ports[0]],
            eth_src=mac_self,
            ip_src=report_src,
            ip_dst=report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_MOD,
            dropped=1,
            congested_queue=1,
            path_tracking_flow=0,
            hw_id=get_pipeid(devports[report_ports[0]]),
            inner_frame=exp_mod_inner_1)

        mod_enabled = False
        acl_enabled = False
        try:
            rs = switch.create_dtel_report_session(
                dst_ip_list=report_dst, truncate_size=report_truncate_size)

            switch.dtel_queue_report_enable = True

            # don't generate report if queue<threshold even if quota is there
            # if queue < threshold, remaining quota = remaining quota
            queue_report = switch.create_dtel_queue_report(
                port=swport_to_fpport(swports[1]),
                queue_id=0,
                depth_threshold=0xfff,
                latency_threshold=0xffffffff,
                breach_quota=1024,
                report_tail_drop=False)

            try:
                # Testing if allowed
                queue_report.port = '4/0'
            except ValueError:
                # Correct behavior
                assert queue_report.port == swports[1]

            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "Passed no report if queue < threshold"

            # make sure the latency higher bits are zeroed
            queue_report.latency_threshold = 0x000fffff
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "Passed no report if latency < threshold"

            # if queue >= threshold, generate report if remaining quota > 0
            queue_report.depth_threshold = 0
            queue_report.latency_threshold = 0
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])

            # verify e2e mirrored packet
            verify_postcard_packet(
                self, exp_e2e_pkt, swports[report_ports[0]])
            verify_no_other_packets(self)
            print "Passed queue report when remaining quota > 0"

            rs.udp_port = UDP_PORT_DTEL_REPORT^0x1111
            time.sleep(min_sleeptime)
            bind_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT^0x1111)
            split_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT)
            exp_e2e_pkt[UDP].dport = UDP_PORT_DTEL_REPORT^0x1111
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])

            # verify e2e mirrored packet
            verify_postcard_packet(
                self, exp_e2e_pkt, swports[report_ports[0]])
            verify_no_other_packets(self)
            split_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT^0x1111)
            bind_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT)
            rs.udp_port = UDP_PORT_DTEL_REPORT
            time.sleep(min_sleeptime)
            exp_e2e_pkt[UDP].dport = UDP_PORT_DTEL_REPORT
            print "Passed queue report + Report UDP port"

            # if queue >= threshold, generate report if remaining quota > 0.
            # set latency sensitivity
            switch.dtel_latency_sensitivity = low_latency_sensitivity
            num = 10
            for i in range(num):
                send_packet(self, swports[0], str(pkt))
                verify_packet(self, exp_pkt, swports[1])

                # verify e2e mirrored packet
                verify_postcard_packet(
                    self, exp_e2e_pkt, swports[report_ports[0]])

            # set quantization shift
            switch.dtel_latency_sensitivity = high_latency_sensitivity
            print "Passed queue report with change when quota > 0"

            # if queue >= threshold, don't generate report if remaining quota==0
            queue_report.breach_quota = 1
            time.sleep(min_sleeptime)
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_postcard_packet(
                self, exp_e2e_pkt, swports[report_ports[0]])
            verify_no_other_packets(self)

            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "Passed don't generate report if quota = 0"

            if test_param_get('target') != 'asic-model':
                # (HARDWARE) if queue >= threshold and quota is finished, and now queue <
                # threshold for the first time, generate 1 report (even if it is
                # not a big change)
                # software queue size is == 0
                # send large packet first.
                # send small packet later it will be smaller than threshold!
                small_pkt = simple_udp_packet(
                    eth_dst=mac_self,
                    eth_src=mac_nbr[0],
                    ip_id=108,
                    ip_dst=ipaddr_nbr[1],
                    ip_src=ipaddr_nbr[0],
                    ip_ttl=64,
                    udp_sport=101,
                    with_udp_chksum=False,
                    pktlen=64,
                    udp_payload=payload)
                exp_small_pkt = simple_udp_packet(
                    eth_dst=mac_nbr[1],
                    eth_src=mac_self,
                    ip_dst=ipaddr_nbr[1],
                    ip_src=ipaddr_nbr[0],
                    ip_id=108,
                    ip_ttl=63,
                    pktlen=64,
                    udp_sport=101,
                    with_udp_chksum=False,
                    udp_payload=payload)

                exp_small_e2e_inner_1 = postcard_report(
                    packet=exp_small_pkt,
                    switch_id=SID,
                    ingress_port=swports[0],
                    egress_port=swports[1],
                    queue_id=0,
                    queue_depth=0,
                    egress_tstamp=0)

                exp_small_e2e_pkt = ipv4_dtel_pkt(
                    eth_dst=mac_nbr[report_ports[0]],
                    eth_src=mac_self,
                    ip_src=report_src,
                    ip_dst=report_dst[0],
                    ip_id=0,
                    ip_ttl=64,
                    next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                    dropped=0,
                    congested_queue=1,
                    path_tracking_flow=0,
                    hw_id=get_pipeid(devports[report_ports[0]]),
                    inner_frame=exp_small_e2e_inner_1)

                # each cell is 80B, so 10 should be at > one packet
                q_threshold = len(str(pkt))/80
                queue_report.depth_threshold = q_threshold
                queue_report.latency_threshold = 0xffffffff
                time.sleep(min_sleeptime)

                # send large packet
                # quota of 1 should be finished here
                send_packet(self, swports[0], str(pkt))
                verify_packet(self, exp_pkt, swports[1])
                verify_postcard_packet(
                    self, exp_e2e_pkt, swports[report_ports[0]])

                # small packet should be below the threshold but still report
                # as quota is finished
                send_packet(self, swports[0], str(small_pkt))
                verify_packet(self, exp_small_pkt, swports[1])
                verify_postcard_packet(
                    self, exp_small_e2e_pkt, swports[report_ports[0]])

                print "Passed received packet below threshold when quota is finished"

            # disable queue alert
            queue_report.delete()
            time.sleep(min_sleeptime)
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "Passed disable queue alert"

        finally:
            if debug_mode:
                raw_input("press any key to cleanup ...")
            else:
                print "Test Cleanup"
            split_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT^0x1111)
            bind_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT)
            split_mirror_on_drop_pkt()
            split_postcard_pkt()
            switch.cleanup(purge=True)

@group('queue')
@group('int_ep')
@group('int_transit')
@group('postcard')
class SONiC_QueueReport_Change_Test(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test queue alert change"
        bind_postcard_pkt() # just to parse report

        # Value of dtel_monitoring_type is irrelevant since neither postcard
        # nor INT will be enabled, and no 'flow' watchlist will be configured
        switch = sonic_switch.SONiCSwitch(dtel_monitoring_type='postcard',
                                          dtel_switch_id=switch_id,
                                          management_ip=switch_ip)
        switch.dtel_int_l4_dscp = {'value': get_int_l45_dscp_value(),
                                   'mask': get_int_l45_dscp_mask()}
        payload = 'qreport'
        # make input frame to inject to sink
        pkt = simple_udp_packet(
            eth_dst=mac_self,
            eth_src=mac_nbr[0],
            ip_id=108,
            ip_dst=ipaddr_nbr[1],
            ip_src=ipaddr_nbr[0],
            ip_ttl=64,
            udp_sport=101,
            with_udp_chksum=False,
            pktlen=256,
            udp_payload=payload)

        exp_pkt = simple_udp_packet(
            eth_dst=mac_nbr[1],
            eth_src=mac_self,
            ip_dst=ipaddr_nbr[1],
            ip_src=ipaddr_nbr[0],
            ip_id=108,
            ip_ttl=63,
            pktlen=256,
            udp_sport=101,
            with_udp_chksum=False,
            udp_payload=payload)

        exp_e2e_inner_1 = postcard_report(
            packet=exp_pkt,
            switch_id=SID,
            ingress_port=swports[0],
            egress_port=swports[1],
            queue_id=0,
            queue_depth=0,
            egress_tstamp=0)

        exp_e2e_pkt = ipv4_dtel_pkt(
            eth_dst=mac_nbr[report_ports[0]],
            eth_src=mac_self,
            ip_src=report_src,
            ip_dst=report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
            dropped=0,
            congested_queue=1,
            path_tracking_flow=0,
            hw_id=get_pipeid(devports[report_ports[0]]),
            inner_frame=exp_e2e_inner_1)

        exp_mod_inner_1 = mod_report(
            packet=exp_pkt,
            switch_id=SID,
            ingress_port=swports[0],
            egress_port=swports[1],
            queue_id=0,
            drop_reason=70)  # drop mtu check fail

        exp_mod_pkt = ipv4_dtel_pkt(
            eth_dst=mac_nbr[report_ports[0]],
            eth_src=mac_self,
            ip_src=report_src,
            ip_dst=report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_MOD,
            dropped=1,
            congested_queue=1,
            path_tracking_flow=0,
            hw_id=get_pipeid(devports[report_ports[0]]),
            inner_frame=exp_mod_inner_1)

        small_pkt = simple_udp_packet(
            eth_dst=mac_self,
            eth_src=mac_nbr[0],
            ip_id=108,
            ip_dst=ipaddr_nbr[1],
            ip_src=ipaddr_nbr[0],
            ip_ttl=64,
            udp_sport=101,
            with_udp_chksum=False,
            pktlen=64,
            udp_payload=payload)
        exp_small_pkt = simple_udp_packet(
            eth_dst=mac_nbr[1],
            eth_src=mac_self,
            ip_dst=ipaddr_nbr[1],
            ip_src=ipaddr_nbr[0],
            ip_id=108,
            ip_ttl=63,
            pktlen=64,
            udp_sport=101,
            with_udp_chksum=False,
            udp_payload=payload)

        exp_small_e2e_inner_1 = postcard_report(
            packet=exp_small_pkt,
            switch_id=SID,
            ingress_port=swports[0],
            egress_port=swports[1],
            queue_id=0,
            queue_depth=0,
            egress_tstamp=0)

        exp_small_e2e_pkt = ipv4_dtel_pkt(
            eth_dst=mac_nbr[report_ports[0]],
            eth_src=mac_self,
            ip_src=report_src,
            ip_dst=report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
            dropped=0,
            congested_queue=1,
            path_tracking_flow=0,
            hw_id=get_pipeid(devports[report_ports[0]]),
            inner_frame=exp_small_e2e_inner_1)

        try:
            rs = switch.create_dtel_report_session(
                dst_ip_list=report_dst, truncate_size=report_truncate_size)

            switch.dtel_queue_report_enable = True

            #  Don't generate queue report if queue<threshold even if there is
            #  a change
            switch.dtel_latency_sensitivity = low_latency_sensitivity
            queue_report = switch.create_dtel_queue_report(
                port=swport_to_fpport(swports[1]),
                queue_id=0,
                depth_threshold=0xfff,
                latency_threshold=0xffffffff,
                breach_quota=1024,
                report_tail_drop=False)

            time.sleep(min_sleeptime)
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "Passed below threshold change don't make a report"

            # Don't generate queue report if queue >= threshold but no change
            # and quota is finished (done in QueueReport_Quota_Test)
            if test_param_get('target') != 'asic-model':
                # Generate queue report if queue >= threshold and change
                # if quota is not finished and if finished
                queue_report.depth_threshold = 0
                queue_report.latency_threshold = 0
                queue_report.breach_quota = 2
                time.sleep(min_sleeptime)

                send_packet(self, swports[0], str(pkt))
                verify_packet(self, exp_pkt, swports[1])

                # verify e2e mirrored packet
                verify_postcard_packet(
                        self, exp_e2e_pkt, swports[report_ports[0]])
                verify_no_other_packets(self)

                send_packet(self, swports[0], str(small_pkt))
                verify_packet(self, exp_small_pkt, swports[1])

                # verify e2e mirrored packet
                verify_postcard_packet(
                        self, exp_small_e2e_pkt, swports[report_ports[0]])
                verify_no_other_packets(self)

                received=False
                for i in range(20):
                    # first two use quota
                    # later should just go because of change
                    # small vs large change)
                    send_packet(self, swports[0], str(pkt))
                    verify_packet(self, exp_pkt, swports[1])

                    # verify e2e mirrored packet
                    (_, rcv_port, rcv_pkt, pkt_time) = \
                        self.dataplane.poll(
                            port_number=swports[report_ports[0]], timeout=1, exp_pkt=None)
                    nrcv = exp_pkt.__class__(rcv_pkt)
                    while nrcv[Ether].type == 0x88cc: # ignore LLDP packets
                        (_, rcv_port, rcv_pkt, pkt_time) = \
                            self.dataplane.poll(
                                port_number=swports[report_ports[0]], timeout=1, exp_pkt=None)
                        nrcv = exp_pkt.__class__(rcv_pkt)
                    if rcv_pkt!=None:
                        print "received a change report after sending %d packets"%(i+1)
                        verify_postcard_dtel_packet(
                                self, rcv_pkt, exp_e2e_pkt)
                        received=True
                        break;
                self.assertTrue(received, "Didn't receive any report because of change!")

                print "Passed generate queue reports regardless of quota"

        finally:
            if debug_mode:
                raw_input("press any key to cleanup ...")
            else:
                print "Test Cleanup"
            split_postcard_pkt()
            switch.cleanup(purge=True)
