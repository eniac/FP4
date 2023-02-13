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
INT endpoint SONiC test
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

import pdb
from switch_ptf_config import *

reset_cycle = 1
min_sleeptime = 2

################################################################################
@group('int_ep')
@group('int_1hopsink')
class SONiC_INT_L4_Stful_1HopSink_Test(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print 'INT Endpoint test'
        prepare_int_l45_bindings()

        switch = sonic_switch.SONiCSwitch(dtel_monitoring_type='int_endpoint',
                                          dtel_switch_id=switch_id+5,
                                          management_ip=switch_ip)
        # Testing for modify
        switch.dtel_switch_id = switch_id

        try:
            rs = switch.create_dtel_report_session(dst_ip_list=report_dst)

            switch.dtel_int_sink_port_list = fpports
            switch.dtel_int_l4_dscp = {'value': get_int_l45_dscp_value(),
                                       'mask': get_int_l45_dscp_mask()}

            int_session = switch.create_dtel_int_session(
                max_hop_count=8,
                collect_switch_id=True,
                collect_switch_ports=False,
                collect_ig_timestamp=False,
                collect_eg_timestamp=False,
                collect_queue_info=True)
            # Test for modification
            int_session.collect_queue_info = False

            time.sleep(min_sleeptime)

            flow_watchlist = switch.create_dtel_watchlist(watchlist_type='flow')

            flow_watchlist_entry = flow_watchlist.create_entry(
                priority=10,
                ether_type=0x888,
                src_ip=ipaddr_nbr[0],
                src_ip_mask=24,
                dst_ip=ipaddr_nbr[1],
                dst_ip_mask=24,
                dtel_int_session=int_session,
                dtel_sample_percent=100,
                dtel_report_all=False)

            # Testing if modification works
            flow_watchlist_entry.ether_type = 0x0800
            switch.dtel_int_endpoint_enable = True
            switch.dtel_latency_sensitivity = MAX_QUANTIZATION

            payload = 'int l45'
            # make input frame to inject to sink
            pkt = simple_udp_packet(
                eth_dst=mac_self,
                eth_src=mac_nbr[0],
                ip_dst=ipaddr_nbr[1],
                ip_src=ipaddr_nbr[0],
                ip_id=108,
                ip_ttl=64,
                udp_sport=101,
                with_udp_chksum=False,
                udp_payload=payload)

            exp_pkt = simple_udp_packet(
                eth_dst=mac_nbr[1],
                eth_src=mac_self,
                ip_dst=ipaddr_nbr[1],
                ip_src=ipaddr_nbr[0],
                ip_id=108,
                ip_ttl=63,
                udp_sport=101,
                with_udp_chksum=False,
                udp_payload=payload)

            exp_inte2e_inner_1 = postcard_report(
                packet=exp_pkt,
                switch_id=switch_id,
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
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=get_pipeid(devports[report_ports[0]]),
                inner_frame=exp_inte2e_inner_1)

            ip_src=ipaddr_nbr[0]
            ip_src = ip_src[0:(ip_src.rfind('.')+1)]+'10'
            pkt2=pkt.copy()
            exp_pkt2=exp_pkt.copy()
            pkt2[IP].src=ip_src
            exp_pkt2[IP].src=ip_src

            exp_inte2e_inner_1 = postcard_report(
                packet=exp_pkt2,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_e2e_pkt2 = ipv4_dtel_pkt(
                eth_dst=mac_nbr[report_ports[0]],
                eth_src=mac_self,
                ip_src=report_src,
                ip_dst=report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=get_pipeid(devports[report_ports[0]]),
                inner_frame=exp_inte2e_inner_1)


            print "Clear bloom filters. Can take up to %d secs." % min_sleeptime
            switch.dtel_flow_state_clear_cycle = reset_cycle
            time.sleep(min_sleeptime)

            print "Disable bloom filter clearing."
            switch.dtel_flow_state_clear_cycle = 0
            time.sleep(2*reset_cycle + min_sleeptime)

            # send a test pkt
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])

            # e2e mirror will have random latency. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[report_ports[0]])

            verify_no_other_packets(self)
            print "Passed for the 1st pkt."

            # send a test pkt from another flow
            send_packet(self, swports[0], str(pkt2))
            verify_packet(self, exp_pkt2, swports[1])

            # e2e mirror will have random latency. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt2, swports[report_ports[0]])
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
            switch.dtel_flow_state_clear_cycle = reset_cycle
            time.sleep(min_sleeptime)

            for count in range(0, 3):
                sleep_sec = max(min_sleeptime, reset_cycle * 2)
                time.sleep(sleep_sec)

                # send the same pkt again
                send_packet(self, swports[0], str(pkt))
                verify_packet(self, exp_pkt, swports[1])
                verify_int_lasthop_dtel_report_packet(
                    self, exp_e2e_pkt, swports[report_ports[0]])
                verify_no_other_packets(self)

                print "Passed bloom filter clearing test %d" % (count + 1)

            time.sleep(min_sleeptime)

            # change quanization shift and re-run 1st and 2nd pkts
            print "Re-setting quantization shift to zero"

            time.sleep(reset_cycle)

            switch.dtel_latency_sensitivity = 0

            print "clear bloom filter and disable clearing"
            switch.dtel_flow_state_clear_cycle = 0
            time.sleep(2*reset_cycle)

            print "Send 1st pkt"
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[report_ports[0]])
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
                    self, exp_e2e_pkt, swports[report_ports[0]])
                if nrcv:
                    num_rcvd_pkts += 1
                verify_no_other_packets(self)

            report_ratio = num_rcvd_pkts * 1.0 / num_send_pkts
            print "Report ratio is", report_ratio
            self.assertTrue(report_ratio > 0.0, "Lasthop report ratio is zero!")

            print "Passed for the 2nd identical pkt w/ zero quantization shift."

        finally:
            #raw_input("press any key to cleanup...")
            switch.cleanup(purge=True)
