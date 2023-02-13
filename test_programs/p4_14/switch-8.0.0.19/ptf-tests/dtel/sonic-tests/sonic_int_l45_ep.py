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
from copy import deepcopy

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
@group('int_ep')
@group('int_ep_no_suppression')
@group('int_ep_udp_src')
class SONiC_INT_UDP_SourceTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print 'INT Endpoint test'
        prepare_int_l45_bindings()

        switch = sonic_switch.SONiCSwitch(dtel_monitoring_type='int_endpoint',
                                          dtel_switch_id=switch_id,
                                          management_ip=switch_ip)

        try:
            rs = switch.create_dtel_report_session(dst_ip_list=report_dst)

            switch.dtel_int_sink_port_list = []
            switch.dtel_int_l4_dscp = {'value': get_int_l45_dscp_value(),
                                       'mask': get_int_l45_dscp_mask()}
            int_session = switch.create_dtel_int_session(
                max_hop_count=8,
                collect_switch_id=True,
                collect_switch_ports=False,
                collect_ig_timestamp=False,
                collect_eg_timestamp=False,
                collect_queue_info=False)

            time.sleep(min_sleeptime)

            flow_watchlist = switch.create_dtel_watchlist(watchlist_type='flow')

            flow_watchlist_entry = flow_watchlist.create_entry(
                priority=10,
                ether_type=0x800,
                src_ip=ipaddr_nbr[0],
                dst_ip=ipaddr_nbr[1],
                dtel_int_session=int_session,
                dtel_sample_percent=100,
                dtel_report_all=True)

            switch.dtel_int_endpoint_enable = True
            switch.dtel_latency_sensitivity = MAX_QUANTIZATION
            time.sleep(min_sleeptime)

            payload = 'int_l45'
            pkt = simple_udp_packet(
                eth_dst=mac_self,
                eth_src=mac_nbr[0],
                ip_dst=ipaddr_nbr[1],
                ip_src=ipaddr_nbr[0],
                ip_id=108,
                ip_ttl=64,
                with_udp_chksum=False,
                udp_payload=payload)

            exp_pkt_ = simple_udp_packet(
                eth_dst=mac_nbr[1],
                eth_src=mac_self,
                ip_dst=ipaddr_nbr[1],
                ip_src=ipaddr_nbr[0],
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
                Packet=exp_pkt, val=switch_id, incr_cnt=1)

            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            #receive_print_packet(self, swports[1], exp_pkt, True)
            verify_no_other_packets(self)
            print "pass 1st packet w/ INT enabled"

            switch.dtel_int_endpoint_enable = False
            time.sleep(min_sleeptime)
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt_, swports[1])
            verify_no_other_packets(self)
            print "pass 2nd packet w/ INT disabled"

            switch.dtel_int_endpoint_enable = True
            time.sleep(min_sleeptime)
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass 3rd packet w/ INT enabled"

        finally:
            if debug_mode:
                raw_input("press any key to cleanup...")
            switch.cleanup(purge=True)


@group('int_ep')
@group('int_ep_no_suppression')
@group('int_ep_udp_sink')
class SONiC_INT_UDP_SinkTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print 'INT Endpoint test'
        prepare_int_l45_bindings()

        switch = sonic_switch.SONiCSwitch(dtel_monitoring_type='int_endpoint',
                                          dtel_switch_id=switch_id,
                                          management_ip=switch_ip)

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
                collect_queue_info=False)

            time.sleep(min_sleeptime)

            flow_watchlist = switch.create_dtel_watchlist(watchlist_type='flow')

            flow_watchlist_entry = flow_watchlist.create_entry(
                priority=10,
                ether_type=0x800,
                src_ip=ipaddr_nbr[0],
                dst_ip=ipaddr_nbr[1],
                dtel_int_session=int_session,
                dtel_sample_percent=100,
                dtel_report_all=True)

            switch.dtel_int_endpoint_enable = True
            switch.dtel_latency_sensitivity = MAX_QUANTIZATION
            time.sleep(min_sleeptime)

            payload = 'int l45'
            # make input frame to inject to sink
            pkt = simple_udp_packet(
                eth_dst=mac_self,
                eth_src=mac_nbr[0],
                ip_id=108,
                ip_dst=ipaddr_nbr[1],
                ip_src=ipaddr_nbr[0],
                ip_ttl=64,
                with_udp_chksum=False,
                udp_sport=101,
                udp_payload=payload)

            int_pkt_orig = int_l45_src_packet(
                test=self,
                int_inst_mask=0x8000,  # swid
                int_inst_cnt=1,
                max_hop_cnt=8,
                dscp=get_int_l45_dscp_value(),
                dscp_mask=get_int_l45_dscp_mask(),
                pkt=pkt)

            # add 2 hop info to the packet
            int_pkt = int_l45_packet_add_hop_info(
                Packet=int_pkt_orig, val=0x66666666, incr_cnt=1)
            int_pkt = int_l45_packet_add_hop_info(
                Packet=int_pkt, val=0x22222222, incr_cnt=1)

            routed_int_pkt = deepcopy(int_pkt)
            routed_int_pkt.getlayer(Ether, 1).src=mac_self
            routed_int_pkt.getlayer(Ether, 1).dst=mac_nbr[1]
            routed_int_pkt.getlayer(IP, 1).ttl=63

            # upstream report packet
            exp_i2e_pkt = ipv4_dtel_pkt(
                eth_dst=mac_nbr[report_ports[0]],
                eth_src=mac_self,
                ip_src=report_src,
                ip_dst=report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=get_pipeid(devports[report_ports[0]]),
                inner_frame=int_pkt)

            exp_pkt = simple_udp_packet(
                eth_dst=mac_nbr[1],
                eth_src=mac_self,
                ip_dst=ipaddr_nbr[1],
                ip_src=ipaddr_nbr[0],
                with_udp_chksum=False,
                ip_id=108,
                ip_ttl=63,
                udp_sport=101,
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


            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[report_ports[0]])
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[report_ports[0]])
            #receive_print_packet(
            #    self, swports[report_ports[0]], exp_e2e_pkt, True)
            verify_no_other_packets(self)
            print "pass 1st packet w/ INT enabled"

            switch.dtel_int_endpoint_enable = False
            time.sleep(min_sleeptime)
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, routed_int_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass 2nd packet w/ INT disabled"

            switch.dtel_int_endpoint_enable = True
            time.sleep(min_sleeptime)
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[report_ports[0]])
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[report_ports[0]])
            verify_no_other_packets(self)
            print "pass 3rd packet w/ INT enabled"

        finally:
            if debug_mode:
                raw_input("press any key to cleanup...")
            switch.cleanup(purge=True)


@group('int_ep')
@group('int_ep_no_suppression')
class SONiC_INT_1Hop_WatchlistLoopTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print 'INT 1 hop watchlist loop test'
        prepare_int_l45_bindings()

        switch = sonic_switch.SONiCSwitch(dtel_monitoring_type='int_endpoint',
                                          dtel_switch_id=switch_id,
                                          management_ip=switch_ip)
        switch.dtel_int_l4_dscp = {'value': get_int_l45_dscp_value(),
                                   'mask': get_int_l45_dscp_mask()}

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

            exp_e2e_inner_1 = postcard_report(
                packet=exp_pkt_out,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_e2e_pkt_1 = ipv4_dtel_pkt(
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
                inner_frame=exp_e2e_inner_1)

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

            switch.dtel_int_sink_port_list = fpports

            int_session = switch.create_dtel_int_session(
                max_hop_count=8,
                collect_switch_id=True,
                collect_switch_ports=False,
                collect_ig_timestamp=False,
                collect_eg_timestamp=False,
                collect_queue_info=False)

            time.sleep(min_sleeptime)

            switch.dtel_int_endpoint_enable = True
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
                    dtel_int_session=int_session,
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
                verify_int_lasthop_dtel_report_packet(
                    self, exp_e2e_pkt_1, swports[report_ports[0]])
                verify_no_other_packets(self)
                print "Passed for the 1st pkt in loop %d" % i

                # send the same test packet again
                send_packet(self, swports[0], str(pkt_in))
                # verify packet out
                verify_packet(self, exp_pkt_out, swports[1])
                # verify postcard packet
                verify_int_lasthop_dtel_report_packet(
                    self, exp_e2e_pkt_1, swports[report_ports[0]])
                verify_no_other_packets(self)
                print "Passed for identical pkt in loop %d" % i

                # send malformed packet
                send_packet(self, swports[0], str(pkt_in_malformed))
                # verify mod packet
                verify_dtel_packet(
                    self, exp_mod_pkt_1, swports[report_ports[0]])
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
