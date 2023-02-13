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
Postcard SONiC test
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
min_sleeptime = 3

# Whether to wait for user input before cleaning up in each test case
debug_mode = False

################################################################################
@group('postcard')
@group('postcard_no_suppression')
class SONiC_PostcardTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print 'postcard test'
        bind_postcard_pkt()

        switch = sonic_switch.SONiCSwitch(dtel_monitoring_type='postcard',
                                          dtel_switch_id=switch_id,
                                          management_ip=switch_ip)

        try:
            rs = switch.create_dtel_report_session(dst_ip_list=report_dst)

            flow_watchlist = switch.create_dtel_watchlist(watchlist_type='flow')

            # Temporary entry to test delete after creating another entry
            flow_watchlist_entry_temp = flow_watchlist.create_entry(
                priority=10,
                ether_type=0x9000,
                src_ip=ipaddr_nbr[0],
                dst_ip=ipaddr_nbr[1],
                l4_src_port_range='900-1100',
                dtel_sample_percent=100,
                dtel_report_all=True)

            flow_watchlist_entry = flow_watchlist.create_entry(
                priority=10,
                ether_type=0x800,
                src_ip=ipaddr_nbr[0],
                dst_ip=ipaddr_nbr[1],
                l4_src_port_range='900-1100',
                dtel_sample_percent=100,
                dtel_report_all=True)

            flow_watchlist_entry_temp.delete()

            switch.dtel_postcard_enable = True
            switch.dtel_latency_sensitivity = MAX_QUANTIZATION

            time.sleep(min_sleeptime)

            pkt_in_1 = simple_tcp_packet(
                eth_dst=mac_self,
                eth_src=mac_nbr[0],
                ip_dst=ipaddr_nbr[1],
                ip_src=ipaddr_nbr[0],
                tcp_sport=1001,
                ip_id=105,
                ip_ttl=64,
                tcp_flags=None,
                pktlen=128)

            exp_pkt_out_1 = simple_tcp_packet(
                eth_dst=mac_nbr[1],
                eth_src=mac_self,
                ip_dst=ipaddr_nbr[1],
                ip_src=ipaddr_nbr[0],
                tcp_sport=1001,
                ip_id=105,
                tcp_flags=None,
                ip_ttl=63,
                pktlen=128)

            exp_postcard_inner_1 = postcard_report(
                packet=exp_pkt_out_1,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_postcard_pkt_1 = ipv4_dtel_pkt(
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
                inner_frame=exp_postcard_inner_1)

            # send a test packet
            send_packet(self, swports[0], str(pkt_in_1))

            # verify packet out
            verify_packet(self, exp_pkt_out_1, swports[1])
            # verify postcard packet
            verify_postcard_packet(
                self, exp_postcard_pkt_1, swports[report_ports[0]])
            #receive_print_packet(
            #    self, swports[report_ports[0]], exp_postcard_pkt_1, True)
            verify_no_other_packets(self)
            print "Passed for the 1st pkt with sport 1001."

            pkt_in_2 = simple_tcp_packet(
                eth_dst=mac_self,
                eth_src=mac_nbr[0],
                ip_dst=ipaddr_nbr[1],
                ip_src=ipaddr_nbr[0],
                tcp_sport=5005,
                ip_id=105,
                ip_ttl=64,
                tcp_flags=None,
                pktlen=128)

            exp_pkt_out_2 = simple_tcp_packet(
                eth_dst=mac_nbr[1],
                eth_src=mac_self,
                ip_dst=ipaddr_nbr[1],
                ip_src=ipaddr_nbr[0],
                tcp_sport=5005,
                ip_id=105,
                tcp_flags=None,
                ip_ttl=63,
                pktlen=128)

            exp_postcard_inner_2 = postcard_report(
                packet=exp_pkt_out_2,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_postcard_pkt_2 = ipv4_dtel_pkt(
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
                inner_frame=exp_postcard_inner_2)

            # send a test packet
            send_packet(self, swports[0], str(pkt_in_2))
            # verify packet out
            verify_packet(self, exp_pkt_out_2, swports[1])
            verify_no_other_packets(self)
            print "Passed for the 2nd pkt with sport 5005."

            flow_watchlist_entry.delete()
            time.sleep(min_sleeptime)
            # send a test packet
            send_packet(self, swports[0], str(pkt_in_1))
            # verify packet out
            verify_packet(self, exp_pkt_out_1, swports[1])
            verify_no_other_packets(self)
            print "Passed for watchlist_delete api"

            flow_watchlist_entry = flow_watchlist.create_entry(
                priority=10,
                ether_type=0x800,
                src_ip=ipaddr_nbr[0],
                dst_ip=ipaddr_nbr[1],
                l4_src_port=5005,
                dtel_sample_percent=100,
                dtel_report_all=True)

            time.sleep(min_sleeptime)

            # send a test packet
            send_packet(self, swports[0], str(pkt_in_2))
            # verify packet out
            verify_packet(self, exp_pkt_out_2, swports[1])
            # verify potcard packet
            verify_postcard_packet(
                self, exp_postcard_pkt_2, swports[report_ports[0]])
            #receive_print_packet(
                #self, swports[report_ports[0]], exp_postcard_pkt_1, True)
            verify_no_other_packets(self)
            print "Passed for the 3rd pkt with sport 5005."

        finally:
            if debug_mode:
                raw_input("press any key to cleanup...")
            switch.cleanup(purge=True)

@group('postcard')
class SONiC_PostcardStfulTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print 'postcard test'
        bind_postcard_pkt()

        switch = sonic_switch.SONiCSwitch(dtel_monitoring_type='postcard',
                                          dtel_switch_id=switch_id,
                                          management_ip=switch_ip)

        try:
            rs = switch.create_dtel_report_session(dst_ip_list=report_dst)

            flow_watchlist = switch.create_dtel_watchlist(watchlist_type='flow')

            flow_watchlist_entry = flow_watchlist.create_entry(
                priority=10,
                ether_type=0x800,
                src_ip=ipaddr_nbr[0],
                dst_ip=ipaddr_nbr[1],
                dtel_sample_percent=100,
                dtel_report_all=False)

            switch.dtel_postcard_enable = True
            switch.dtel_latency_sensitivity = MAX_QUANTIZATION

            print "Clearing bloom filters ..."
            switch.dtel_flow_state_clear_cycle = reset_cycle
            #print switch.dtel_flow_state_clear_cycle
            time.sleep(min_sleeptime)
            # Test set
            assert switch.dtel_flow_state_clear_cycle == reset_cycle

            print "Disable bloom filter clearing."
            switch.dtel_flow_state_clear_cycle = 0
            time.sleep(2*reset_cycle)

            pkt_in = simple_tcp_packet(
                eth_dst=mac_self,
                eth_src=mac_nbr[0],
                ip_dst=ipaddr_nbr[1],
                ip_src=ipaddr_nbr[0],
                ip_id=105,
                ip_ttl=64,
                tcp_flags=None,
                pktlen=128)

            exp_pkt_out = simple_tcp_packet(
                eth_dst=mac_nbr[1],
                eth_src=mac_self,
                ip_dst=ipaddr_nbr[1],
                ip_src=ipaddr_nbr[0],
                ip_id=105,
                tcp_flags=None,
                ip_ttl=63,
                pktlen=128)

            exp_postcard_inner_1 = postcard_report(
                packet=exp_pkt_out,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_postcard_pkt_1 = ipv4_dtel_pkt(
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
                inner_frame=exp_postcard_inner_1)

            time.sleep(min_sleeptime)

            # send a test packet
            send_packet(self, swports[0], str(pkt_in))

            # verify packet out
            verify_packet(self, exp_pkt_out, swports[1])
            # verify postcard packet
            verify_postcard_packet(
                self, exp_postcard_pkt_1, swports[report_ports[0]])
            #receive_print_packet(
            #    self, swports[report_ports[0]], exp_postcard_pkt_1, True)
            verify_no_other_packets(self)
            print "Passed for the 1st pkt."

            # send the same test packet again
            send_packet(self, swports[0], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[1])
            verify_no_other_packets(self)
            print "Passed for identical pkt from the same port"

            exp_postcard_inner_2 = postcard_report(
                packet=exp_pkt_out,
                switch_id=switch_id,
                ingress_port=swports[2],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_poscard_pkt_2 = ipv4_dtel_pkt(
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
                inner_frame=exp_postcard_inner_2)

            flow_watchlist_entry.delete()
            print "delete watchlist entry"
            time.sleep(min_sleeptime)
            # send the same test packet through different port
            send_packet(self, swports[2], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[1])
            verify_no_other_packets(self)
            print "Passed for watchlist delete"

            flow_watchlist_entry = flow_watchlist.create_entry(
                priority=10,
                ether_type=0x800,
                src_ip=ipaddr_nbr[0],
                dst_ip=ipaddr_nbr[1],
                dtel_sample_percent=100,
                dtel_report_all=False)

            time.sleep(min_sleeptime)
            # send the same test packet through a different port
            send_packet(self, swports[2], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[1])
            # verify postcard packet
            verify_postcard_packet(
                self, exp_poscard_pkt_2, swports[report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for identical pkt from a different port"

            print "Enable bloom filter clearing."
            switch.dtel_flow_state_clear_cycle = reset_cycle
            time.sleep(2*reset_cycle)

            for count in range(0,3):
                sleep_sec = max(min_sleeptime, reset_cycle * 2)
                time.sleep(sleep_sec)
                # send the same pkt again
                send_packet(self, swports[0], str(pkt_in))
                verify_packet(self, exp_pkt_out, swports[1])
                verify_postcard_packet(
                    self, exp_postcard_pkt_1, swports[report_ports[0]])
                verify_no_other_packets(self)
                print "Passed bloom filter clearing test %d" % (count + 1)

            time.sleep(min_sleeptime)

            # change quanization shift and re-run 1st and 2nd pkts
            print "Re-setting quant shift to zero, high latency sensitivity"
            switch.dtel_latency_sensitivity = 0
            switch.dtel_flow_state_clear_cycle = 0
            time.sleep(min_sleeptime)

            send_packet(self, swports[0], str(pkt_in))
            verify_packet(self, exp_pkt_out, swports[1])
            verify_postcard_packet(
                self, exp_postcard_pkt_1, swports[report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for the 1st pkt."

            num_send_pkts = 20
            num_rcvd_pkts = 0
            for i in range(num_send_pkts):
                send_packet(self, swports[0], str(pkt_in))
                verify_packet(self, exp_pkt_out, swports[1])
                if receive_postcard_packet(
                        self, exp_postcard_pkt_1, swports[report_ports[0]]):
                    num_rcvd_pkts += 1
                verify_no_other_packets(self)

            report_ratio = num_rcvd_pkts * 1.0 / num_send_pkts
            print "Report ratio is", report_ratio
            self.assertTrue(report_ratio > 0.0, "Report ratio is zero!")

            print "Passed for the 2nd pkt w/ zero quantization shift."

        finally:
            if debug_mode:
                raw_input("press any key to cleanup...")
            switch.cleanup(purge=True)


@group('postcard')
class SONiC_PostcardWatchlistLoopTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print 'postcard test'
        bind_postcard_pkt()

        switch = sonic_switch.SONiCSwitch(dtel_monitoring_type='postcard',
                                          dtel_switch_id=switch_id,
                                          management_ip=switch_ip)

        loop_count = 10

        try:
            pkt_in = simple_tcp_packet(
                eth_dst=mac_self,
                eth_src=mac_nbr[0],
                ip_dst=ipaddr_nbr[1],
                ip_src=ipaddr_nbr[0],
                ip_id=105,
                ip_ttl=64,
                tcp_flags=None,
                pktlen=128)

            exp_pkt_out = simple_tcp_packet(
                eth_dst=mac_nbr[1],
                eth_src=mac_self,
                ip_dst=ipaddr_nbr[1],
                ip_src=ipaddr_nbr[0],
                ip_id=105,
                tcp_flags=None,
                ip_ttl=63,
                pktlen=128)

            exp_postcard_inner_1 = postcard_report(
                packet=exp_pkt_out,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_postcard_pkt_1 = ipv4_dtel_pkt(
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
                inner_frame=exp_postcard_inner_1)

            pkt_in_malformed = simple_tcp_packet(
                eth_dst=mac_self,
                eth_src=mac_all_zeros,
                ip_dst=ipaddr_nbr[1],
                ip_src=ipaddr_nbr[0],
                ip_id=105,
                ip_ttl=64,
                tcp_flags=None,
                pktlen=128)

            exp_mod_inner_1 = mod_report(
                packet=pkt_in_malformed,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=INVALID_PORT_ID,
                queue_id=0,
                drop_reason=10)  # outer source mac all zeros

            exp_mod_pkt_1 = ipv4_dtel_pkt(
                eth_dst=mac_nbr[report_ports[0]],
                eth_src=mac_self,
                ip_src=report_src,
                ip_dst=report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_MOD,
                dropped=1,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=get_pipeid(devports[report_ports[0]]),
                inner_frame=exp_mod_inner_1)

            rs = switch.create_dtel_report_session(dst_ip_list=report_dst)

            switch.dtel_postcard_enable = True
            switch.dtel_drop_report_enable = True
            switch.dtel_latency_sensitivity = MAX_QUANTIZATION

            for i in range(loop_count):
                flow_watchlist = switch.create_dtel_watchlist(
                    watchlist_type='flow')

                flow_watchlist_entry = flow_watchlist.create_entry(
                    priority=10,
                    ether_type=0x800,
                    src_ip=ipaddr_nbr[0],
                    dst_ip=ipaddr_nbr[1],
                    dtel_sample_percent=100,
                    dtel_report_all=True)

                drop_watchlist = switch.create_dtel_watchlist(
                    watchlist_type='drop')

                drop_watchlist_entry = drop_watchlist.create_entry(
                    priority=10,
                    ether_type=0x800,
                    src_ip=ipaddr_nbr[0],
                    dst_ip=ipaddr_nbr[1])

                time.sleep(min_sleeptime)

                # send a test packet
                send_packet(self, swports[0], str(pkt_in))

                # verify packet out
                verify_packet(self, exp_pkt_out, swports[1])
                # verify postcard packet
                verify_postcard_packet(
                    self, exp_postcard_pkt_1, swports[report_ports[0]])
                #receive_print_packet(
                #    self, swports[report_ports[0]], exp_postcard_pkt_1, True)
                verify_no_other_packets(self)
                print "Passed for the 1st pkt in loop %d" % i

                # send the same test packet again
                send_packet(self, swports[0], str(pkt_in))
                # verify packet out
                verify_packet(self, exp_pkt_out, swports[1])
                # verify postcard packet
                verify_postcard_packet(
                    self, exp_postcard_pkt_1, swports[report_ports[0]])
                #receive_print_packet(
                #    self, swports[report_ports[0]], exp_postcard_pkt_1, True)
                verify_no_other_packets(self)
                print "Passed for identical pkt in loop %d" % i

                # send malformed packet
                send_packet(self, swports[0], str(pkt_in_malformed))
                # verify mod packet
                verify_dtel_packet(
                    self, exp_mod_pkt_1, swports[report_ports[0]])
                #receive_print_packet(
                #    self, swports[report_ports[0]], exp_postcard_pkt_1, True)
                verify_no_other_packets(self)
                print "Passed for malformed pkt in loop %d" % i

                # flow_watchlist_entry.delete()
                # drop_watchlist_entry.delete()
                flow_watchlist.delete()
                drop_watchlist.delete()
                print "delete watchlist entries in loop %d" % i
                time.sleep(min_sleeptime)

                # send the same test packet
                send_packet(self, swports[0], str(pkt_in))
                # verify packet out
                verify_packet(self, exp_pkt_out, swports[1])
                verify_no_other_packets(self)
                print "Passed for watchlist delete in loop %d" % i

                # send malformed packet
                send_packet(self, swports[0], str(pkt_in_malformed))
                verify_no_other_packets(self)
                print "Passed for drop watchlist delete in loop %d" % i

        finally:
            if debug_mode:
                raw_input("press any key to cleanup...")
            switch.cleanup(purge=True)
