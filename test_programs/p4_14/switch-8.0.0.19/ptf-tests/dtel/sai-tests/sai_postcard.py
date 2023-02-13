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

@group('postcard')
@group('postcard_no_suppression')
class SAI_PostcardTest(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Test postcard"
        bind_postcard_pkt()

        # create SAI manager
        sai_mgr = SAIManager(self, params)

        # create report session
        sai_mgr.create_dtel_report_session()

        flow_watchlist = sai_mgr.create_dtel_watchlist('Flow')

        u32range = sai_thrift_range_t(min=900, max=1100)
        acl_range_id = sai_thrift_create_acl_range(
            self.client, SAI_ACL_RANGE_TYPE_L4_SRC_PORT_RANGE, u32range)
        range_list = [acl_range_id]

        flow_watchlist_entry = sai_mgr.create_dtel_watchlist_entry(
            watchlist=flow_watchlist,
            priority=10,
            ip_src=params.ipaddr_nbr[0],
            ip_src_mask='255.255.255.0',
            ip_dst=params.ipaddr_nbr[1],
            ip_dst_mask='255.255.255.0',
            range_list=range_list,
            dtel_postcard_enable=True,
            dtel_report_all=True,
            dtel_sample_percent=100)

        sai_mgr.switch.dtel_postcard_enable = True

        # run test
        try:
            pkt_in = simple_tcp_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                tcp_sport=1001,
                ip_id=105,
                ip_ttl=64,
                tcp_flags=None,
                pktlen=256)

            exp_pkt_out = simple_tcp_packet(
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                tcp_sport=1001,
                ip_id=105,
                tcp_flags=None,
                ip_ttl=63,
                pktlen=256)

            exp_postcard_inner = postcard_report(
                packet=exp_pkt_out,
                switch_id=SID,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_postcard_pkt = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.report_src,
                ip_dst=params.report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=get_pipeid(
                    swport_to_devport(self, swports[params.report_ports[0]])),
                inner_frame=exp_postcard_inner)

            # send a test packet
            send_packet(self, swports[0], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[1])
            # verify potcard packet
            verify_postcard_packet(
                self, exp_postcard_pkt, swports[params.report_ports[0]])
            #receive_print_packet(
                #self, swports[params.report_ports[0]], exp_postcard_pkt_1, True)
            verify_no_other_packets(self)
            print "Passed for the 1st pkt with sport 1001."

            pkt_in = simple_tcp_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                tcp_sport=5005,
                ip_id=105,
                ip_ttl=64,
                tcp_flags=None,
                pktlen=256)

            exp_pkt_out = simple_tcp_packet(
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                tcp_sport=5005,
                ip_id=105,
                tcp_flags=None,
                ip_ttl=63,
                pktlen=256)

            exp_postcard_inner = postcard_report(
                packet=exp_pkt_out,
                switch_id=SID,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_postcard_pkt = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.report_src,
                ip_dst=params.report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=get_pipeid(
                    swport_to_devport(self, swports[params.report_ports[0]])),
                inner_frame=exp_postcard_inner)

            # send a test packet
            send_packet(self, swports[0], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[1])
            verify_no_other_packets(self)
            print "Passed for the 2nd pkt with sport 5005."

            flow_watchlist.delete()
            # send the same test packet through different port
            send_packet(self, swports[2], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[1])
            verify_no_other_packets(self)
            print "Passed for watchlist_delete api"

            flow_watchlist = sai_mgr.create_dtel_watchlist('Flow')
            flow_watchlist_entry = sai_mgr.create_dtel_watchlist_entry(
                watchlist=flow_watchlist,
                priority=10,
                ip_src=params.ipaddr_nbr[0],
                ip_src_mask='255.255.255.0',
                ip_dst=params.ipaddr_nbr[1],
                ip_dst_mask='255.255.255.0',
                l4_src_port=5005,
                l4_src_port_mask=0xffff,
                dtel_postcard_enable=True,
                dtel_report_all=True,
                dtel_sample_percent=100)
            # send a test packet
            send_packet(self, swports[0], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[1])
            # verify potcard packet
            verify_postcard_packet(
                self, exp_postcard_pkt, swports[params.report_ports[0]])
            #receive_print_packet(
                #self, swports[params.report_ports[0]], exp_postcard_pkt_1, True)
            verify_no_other_packets(self)
            print "Passed for the 3rd pkt with sport 5005."

        # cleanup
        finally:
            split_postcard_pkt()
            sai_mgr.switch.dtel_postcard_enable = False
            sai_mgr.cleanup()

@group('postcard')
class SAI_PostcardStfulTest(sai_base_test.ThriftInterfaceDataPlane):
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
            dtel_report_all=False,
            dtel_sample_percent=100)

        sai_mgr.switch.dtel_postcard_enable = True
        sai_mgr.switch.dtel_latency_sensitivity = MAX_QUANTIZATION
        print "Clear bloom filters. Can take up to %d secs." % min_sleeptime
        sai_mgr.switch.dtel_flow_state_clear_cycle = reset_cycle
        time.sleep(min_sleeptime)

        print "Disable bloom filter clearing."
        sai_mgr.switch.dtel_flow_state_clear_cycle = 0
        time.sleep(2*reset_cycle)

        # run test
        try:
            pkt_in = simple_tcp_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                ip_ttl=64,
                tcp_flags=None,
                pktlen=256)

            exp_pkt_out = simple_tcp_packet(
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                tcp_flags=None,
                ip_ttl=63,
                pktlen=256)

            exp_postcard_inner_1 = postcard_report(
                packet=exp_pkt_out,
                switch_id=SID,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_postcard_pkt_1 = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.report_src,
                ip_dst=params.report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=get_pipeid(
                    swport_to_devport(self, swports[params.report_ports[0]])),
                inner_frame=exp_postcard_inner_1)

            # send a test packet
            send_packet(self, swports[0], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[1])
            # verify potcard packet
            verify_postcard_packet(
                self, exp_postcard_pkt_1, swports[params.report_ports[0]])
            #receive_print_packet(
                #self, swports[params.report_ports[0]], exp_postcard_pkt_1, True)
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
                switch_id=SID,
                ingress_port=swports[2],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_poscard_pkt_2 = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.report_src,
                ip_dst=params.report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=get_pipeid(
                    swport_to_devport(self, swports[params.report_ports[0]])),
                inner_frame=exp_postcard_inner_2)

            flow_watchlist.delete()
            # send the same test packet through different port
            send_packet(self, swports[2], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[1])
            verify_no_other_packets(self)
            print "Passed for watchlist_delete api"
            flow_watchlist = sai_mgr.create_dtel_watchlist('Flow')

            flow_watchlist_entry = sai_mgr.create_dtel_watchlist_entry(
                watchlist=flow_watchlist,
                priority=10,
                ip_src=params.ipaddr_nbr[0],
            	ip_src_mask='255.255.255.0',
            	ip_dst=params.ipaddr_nbr[1],
            	ip_dst_mask='255.255.255.0',
            	dtel_postcard_enable=True,
            	dtel_report_all=False,
                dtel_sample_percent=100)
            # send the same test packet through different port
            send_packet(self, swports[2], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[1])
            # verify potcard packet
            verify_postcard_packet(
                self, exp_poscard_pkt_2, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for identical pkt from a different port"

            print "Enable bloom filter clearing"
            sai_mgr.switch.dtel_flow_state_clear_cycle = reset_cycle
            for count in range(0,3):
                sleep_sec = max(min_sleeptime, reset_cycle * 2)
                time.sleep(sleep_sec)
                # send the same pkt again
                send_packet(self, swports[0], str(pkt_in))
                verify_packet(self, exp_pkt_out, swports[1])
                verify_postcard_packet(
                    self, exp_postcard_pkt_1, swports[params.report_ports[0]])
                verify_no_other_packets(self)
                print "Passed bloom filter clearing test %d" % (count + 1)

            time.sleep(min_sleeptime)

            # change quanization shift and re-run 1st and 2nd pkts
            print "Re-setting quant shift to zero, high latency sensitivity"
            sai_mgr.switch.dtel_latency_sensitivity = 0
            sai_mgr.switch.dtel_flow_state_clear_cycle = 0
            time.sleep(2)

            send_packet(self, swports[0], str(pkt_in))
            verify_packet(self, exp_pkt_out, swports[1])
            verify_postcard_packet(
                self, exp_postcard_pkt_1, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for the 1st pkt."

            num_send_pkts = 20
            num_rcvd_pkts = 0
            for i in range(num_send_pkts):
                send_packet(self, swports[0], str(pkt_in))
                verify_packet(self, exp_pkt_out, swports[1])
                if receive_postcard_packet(self,
                                           exp_postcard_pkt_1,
                                           swports[params.report_ports[0]]):
                    num_rcvd_pkts += 1
                verify_no_other_packets(self)

            report_ratio = num_rcvd_pkts * 1.0 / num_send_pkts
            print "Report ratio is", report_ratio
            self.assertTrue(report_ratio > 0.0, "Report ratio is zero!")

            print "Passed for the 2nd pkt w/ zero quantization shift."

        # cleanup
        finally:
            split_postcard_pkt()
            sai_mgr.switch.dtel_postcard_enable = False
            sai_mgr.cleanup()

@group('postcard2')
@group('mod')
class SAI_PostcardMoDTest(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Test postcard with mirror on drop"
        print "NOTE: MTU does not currently trigger MoD. Skip this test."
        return

        bind_postcard_pkt()
        bind_mirror_on_drop_pkt()
        sai_mgr = SAIManager(self, params)

    	# enable Postcard and MoD
        sai_mgr.switch.dtel_postcard_enable = True
        sai_mgr.switch.dtel_drop_report_enable = True
        sai_mgr.switch.dtel_latency_sensitivity = 0#latency_sensitivity

        # add flow space to postcard and MoD watchlist
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
            dtel_report_all=False,
            dtel_sample_percent=100)

        drop_watchlist = sai_mgr.create_dtel_watchlist('Drop')

        drop_watchlist_entry = sai_mgr.create_dtel_watchlist_entry(
            watchlist=drop_watchlist,
            priority=10,
            ip_src=params.ipaddr_nbr[0],
            ip_src_mask='255.255.255.0',
            ip_dst=params.ipaddr_nbr[1],
            ip_dst_mask='255.255.255.0',
            dtel_drop_report_enable=True)

    	# run test
        try:
    	    pkt_in = simple_tcp_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                ip_ttl=64,
                tcp_flags=None,
                pktlen=256)

            exp_pkt_out = simple_tcp_packet(
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                tcp_flags=None,
                ip_ttl=63,
                pktlen=256)

            exp_postcard_inner_1 = postcard_report(
                packet=exp_pkt_out,
                switch_id=SID,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_postcard_pkt_1 = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.report_src,
                ip_dst=params.report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=get_pipeid(
                    swport_to_devport(self, swports[params.report_ports[0]])),
                inner_frame=exp_postcard_inner_1)

            exp_mod_inner_1 = mod_report(
                packet=exp_pkt_out,
                switch_id=SID,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                drop_reason=70)

            exp_mod_pkt = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.report_src,
                ip_dst=params.report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_MOD,
                dropped=1,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=get_pipeid(
                    swport_to_devport(self, swports[params.report_ports[0]])),
                inner_frame=exp_mod_inner_1)

            # mirror on drop vs. postcard
            sai_mgr.router_interfaces[1].mtu = 200
            send_packet(self, swports[0], str(pkt_in))
            # verify mirror on drop packet
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            #receive_print_packet(
            #    self, swports[params.report_ports[0]], exp_mod_pkt, True)
            verify_no_other_packets(self)
            print "Passed for the pkt with mirror on drop"

            dtel_event = sai_mgr.create_dtel_event(
                SAI_DTEL_EVENT_TYPE_DROP_REPORT, 5)
            exp_mod_pkt[IP].tos = 5<<2
            send_packet(self, swports[0], str(pkt_in))
            # verify mirror on drop packet
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for the pkt with mirror on drop new dscp"

        # cleanup
        finally:
            split_mirror_on_drop_pkt()
            split_postcard_pkt()
            sai_mgr.switch.dtel_latency_sensitivity = 0
            sai_mgr.switch.dtel_postcard_enable = False
            sai_mgr.switch.dtel_drop_report_enable = False
            sai_mgr.cleanup()
