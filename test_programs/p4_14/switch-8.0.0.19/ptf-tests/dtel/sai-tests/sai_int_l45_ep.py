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
@group('ep_l45_no_suppression')
class SAI_INTL45_UDP_SourceTest(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 Source"
        cleanup_int_l45_bindings()

        INT_DSCP = 0x10
        INT_DSCP_MASK=0x10
        prepare_int_l45_bindings(int_dscp=INT_DSCP, int_dscp_mask=INT_DSCP_MASK)

        # create SAI manager
        sai_mgr = SAIManager(self, params)

        sai_mgr.switch.dtel_int_l4_dscp = (INT_DSCP, INT_DSCP_MASK)

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

        WL_DSCP_VAL = 0x01
        WL_DSCP_MASK = 0x01
        flow_watchlist_entry = sai_mgr.create_dtel_watchlist_entry(
            watchlist=flow_watchlist,
            priority=10,
            ip_src=params.ipaddr_nbr[0],
            ip_src_mask='255.255.255.0',
            ip_dst=params.ipaddr_nbr[1],
            ip_dst_mask='255.255.255.0',
            dscp=WL_DSCP_VAL,
            dscp_mask=WL_DSCP_MASK,
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
            ip_tos=WL_DSCP_VAL<<2,
            with_udp_chksum=False,
            udp_payload=payload)

        exp_pkt_ = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            with_udp_chksum=False,
            ip_id=108,
            ip_ttl=63,
            ip_tos=WL_DSCP_VAL<<2,
            udp_payload=payload)

        exp_pkt = int_l45_src_packet(
            test=self,
            int_inst_mask=0x8000,
            int_inst_cnt=1,
            max_hop_cnt=8,
            dscp=INT_DSCP,
            dscp_mask=INT_DSCP_MASK,
            pkt=exp_pkt_)

        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=SID, incr_cnt=1)

        int_enabled = False
        try:
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt_, swports[1])
            verify_no_other_packets(self)
            print "pass 1st packet w/ INT disabled"

            sai_mgr.switch.dtel_int_endpoint_enable = True
            int_enabled = True
            send_packet(self, swports[0], str(pkt))
            #receive_print_packet(self, swports[1], exp_pkt, True)
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass 2nd packet w/ INT enabled"

            sai_mgr.switch.dtel_int_endpoint_enable = False
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt_, swports[1])
            verify_no_other_packets(self)
            print "pass 3rd packet w/ INT disabled"

            sai_mgr.switch.dtel_int_endpoint_enable = True
            int_enabled = True
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass 4th packet w/ INT enabled"

            nomatch_tos = 0xFC - ((WL_DSCP_VAL | INT_DSCP) << 2)
            pkt[IP].tos = nomatch_tos
            exp_pkt_[IP].tos = nomatch_tos
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt_, swports[1])
            verify_no_other_packets(self)
            print "pass 5th packet w/ DSCP no match"

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            sai_mgr.switch.dtel_int_endpoint_enable = False
            sai_mgr.switch.dtel_int_l4_dscp = (get_int_l45_dscp_value(),
                                               get_int_l45_dscp_mask())
            sai_mgr.cleanup()


@group('ep_l45')
@group('ep_l45_no_suppression')
class SAI_INTL45_UDP_SinkTest(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 Sink"

        prepare_int_l45_bindings()

        # create SAI manager
        sai_mgr = SAIManager(self, params)

        # create report session
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

        payload = 'int l45'
        # make input frame to inject to sink
        pkt = simple_udp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_id=108,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=64,
            with_udp_chksum=False,
            udp_sport=101,
            udp_payload=payload)

        int_pkt_orig = int_l45_src_packet(
            test=self,
            int_inst_mask=0x8000,  # swid
            int_inst_cnt=1,
            max_hop_cnt=8,
            pkt=pkt)

        # add 2 hop info to the packet
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt_orig, val=0x66666666, incr_cnt=1)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222222, incr_cnt=1)

        # upstream report packet
        exp_i2e_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.report_src,
            ip_dst=params.report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
            dropped=0,
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=get_pipeid(
                swport_to_devport(self, swports[params.report_ports[0]])),
            inner_frame=int_pkt)

        exp_pkt = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            with_udp_chksum=False,
            ip_id=108,
            ip_ttl=63,
            udp_sport=101,
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
            inner_frame=exp_inte2e_inner_1)

        exp_pkt_ = int_pkt.copy()
        exp_pkt_[IP].ttl-=1
        exp_pkt_[Ether].src=exp_pkt[Ether].src
        exp_pkt_[Ether].dst=exp_pkt[Ether].dst
        try:
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt_, swports[1])
            verify_no_other_packets(self)
            print "pass 1st packet w/ INT disabled"

            sai_mgr.switch.dtel_int_endpoint_enable = True
            int_enabled = True
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass 2nd packet w/ INT enabled"

            sai_mgr.switch.dtel_int_endpoint_enable = False
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt_, swports[1])
            verify_no_other_packets(self)
            print "pass 3rd packet w/ INT disabled"

            sai_mgr.switch.dtel_int_endpoint_enable = True
            int_enabled = True
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass 4th packet w/ INT enabled"

        finally:
            ### Cleanup
            sai_mgr.switch.dtel_int_endpoint_enable = False
            sai_mgr.cleanup()


@group('ep_l45')
@group('ep_l45_no_suppression')
class SAI_INTL45_TCP_SourceTest(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 TCP Source device - not VTEP-src, just INT-src"
        prepare_int_l45_bindings()

        # create SAI panager
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

        pkt = simple_tcp_packet(
            eth_dst=params.mac_self,
            tcp_flags= None,
            eth_src=params.mac_nbr[0],
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_id=108,
            ip_ttl=64)

        exp_pkt = simple_tcp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            tcp_flags= None,
            ip_id=108,
            ip_ttl=63)

        exp_pkt = int_l45_src_packet(
            test=self,
            int_inst_mask=0x8000,
            int_inst_cnt=1,
            max_hop_cnt=8,
            pkt=exp_pkt)

        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=SID, incr_cnt=1)

        m = Mask(exp_pkt)
        m.set_do_not_care_scapy(TCP_INTL45, 'chksum')

        try:
            sai_mgr.switch.dtel_int_endpoint_enable = True
            int_enabled = True

            send_packet(self, swports[0], str(pkt))
            verify_packet(self, m, swports[1])
            verify_no_other_packets(self)

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            sai_mgr.switch.dtel_int_endpoint_enable = False
	    sai_mgr.cleanup()

@group('ep_l45')
@group('ep_l45_no_suppression')
class SAI_INTL45_TCP_SinkTest(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 TCP Sink device generating DTel report"
        prepare_int_l45_bindings()

        # create SAI manager
        sai_mgr = SAIManager(self, params)

        # create report session
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

        payload = 'int l45'

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
        # force checksum calculation
        #del pkt[TCP].chksum
        #pkt[TCP] = pkt[TCP].__class__(str(pkt[TCP]))
        #chksum = pkt[TCP].chksum


        int_pkt_orig = int_l45_src_packet(
            test=self,
            int_inst_mask=0x8000,  # swid
            int_inst_cnt=1,
            max_hop_cnt=8,
            pkt=pkt)
        # add 2 hop info to the packet
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt_orig, val=0x66666666, incr_cnt=1)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222222, incr_cnt=1)

        # bring back the checksum
        #int_pkt[TCP_INTL45].chksum = chksum

        # upstream report packet
        exp_i2e_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.report_src,
            ip_dst=params.report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
            dropped=0,
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=get_pipeid(
                swport_to_devport(self, swports[params.report_ports[0]])),
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
            inner_frame=exp_inte2e_inner_1)


        try:
            sai_mgr.switch.dtel_int_endpoint_enable = True
            int_enabled = True
            m = Mask(exp_pkt)
            m.set_do_not_care_scapy(TCP, 'chksum')

            # send a test pkt
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, m, swports[1])

            # verify i2e mirrored packet
            verify_int_l45_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)

            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_int_lasthop_dtel_report_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]],
                ignore_chksum=True)

            verify_no_other_packets(self)

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            sai_mgr.switch.dtel_int_endpoint_enable = False
            sai_mgr.cleanup()

@group('ep_l45X')
@group('ep_l45_no_suppressionX')
class SAI_INTL45_EgressMoDTest(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT Sink device with mirror on drop"
        print "Skip this test until egress ACLs are supported in SAI"
        return
        prepare_int_l45_bindings()
        bind_mirror_on_drop_pkt()

        # create SAI manager
        sai_mgr = SAIManager(self, params)

        # create report session
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

        drop_watchlist = sai_mgr.create_dtel_watchlist('Drop')

        drop_watchlist_entry = sai_mgr.create_dtel_watchlist_entry(
            watchlist=drop_watchlist,
            priority=10,
            ip_src=params.ipaddr_nbr[0],
            ip_src_mask='255.255.255.0',
            ip_dst=params.ipaddr_nbr[1],
            ip_dst_mask='255.255.255.0',
            dtel_drop_report_enable=True)

        payload = 'int l45'
        # make input frame to inject to sink
        pkt = simple_udp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_id=108,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=64,
            with_udp_chksum=False,
            pktlen=256,
            udp_sport=101,
            udp_payload=payload)

        int_pkt_orig = int_l45_src_packet(
            test=self,
            int_inst_mask=0x8000,  # swid
            int_inst_cnt=1,
            max_hop_cnt=8,
            pkt=pkt)

        # add 2 hop info to the packet
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt_orig, val=0x66666666, incr_cnt=1)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222222, incr_cnt=1)

        # upstream report packet
        exp_i2e_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.report_src,
            ip_dst=params.report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_ETHERNET,
            dropped=0,
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=get_pipeid(
                swport_to_devport(self, swports[params.report_ports[0]])),
            inner_frame=int_pkt)

        exp_pkt = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_id=108,
            ip_ttl=63,
            pktlen=256,
            with_udp_chksum=False,
            udp_sport=101,
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
            inner_frame=exp_inte2e_inner_1)

        exp_mod_inner_1 = mod_report(
            packet=exp_pkt,
            switch_id=SID,
            ingress_port=swports[0],
            egress_port=swports[1],
            queue_id=0,
            drop_reason=92)  # drop egress acl deny

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

        try:
            # config MoD
            sai_mgr.switch.dtel_int_endpoint_enable = True
            sai_mgr.switch.dtel_drop_report_enable = True

            # send a test pkt
            send_packet(self, swports[0], str(int_pkt))
            verify_packet(self, exp_pkt, swports[1])
            print "Initial packet was transmitted"

            # setup ACL to drop based on source IP
            action_list = [SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION]
            packet_action = SAI_PACKET_ACTION_DROP
            ip_src_mask = "255.255.255.0"

            # setup ACL to block based on Source IP
            table_stage = SAI_ACL_STAGE_EGRESS
            table_bind_point_list = [SAI_ACL_BIND_POINT_TYPE_PORT]
            entry_priority = 1000
            action = SAI_PACKET_ACTION_DROP
            addr_family = SAI_IP_ADDR_FAMILY_IPV4
            mac_src = None
            mac_dst = None
            mac_src_mask = None
            mac_dst_mask = None
            ip_src = params.ipaddr_nbr[0]
            ip_src_mask = "255.255.255.0"
            ip_dst = None
            ip_dst_mask = None
            ip_proto = None
            in_ports = None
            in_port = None
            out_port = None
            out_ports = [swports[1]]
            src_l4_port = None
            dst_l4_port = None
            ingress_mirror_id = None
            egress_mirror_id = None
            range_list = None

            acl_table_id = sai_thrift_create_acl_table(self.client,
                table_stage,
                table_bind_point_list,
                addr_family,
                mac_src,
                mac_dst,
                ip_src,
                ip_dst,
                ip_proto,
                in_ports,
                out_ports,
                in_port,
                out_port,
                src_l4_port,
                dst_l4_port,
                range_list)

            acl_entry_id = sai_thrift_create_acl_entry(self.client,
                acl_table_id,
                entry_priority,
                action, addr_family,
                mac_src, mac_src_mask,
                mac_dst, mac_dst_mask,
                ip_src, ip_src_mask,
                ip_dst, ip_dst_mask,
                ip_proto,
                in_ports, out_ports,
                in_port, out_port,
                src_l4_port, dst_l4_port,
                ingress_mirror_id,
                egress_mirror_id,
                range_list)

            # bind this ACL table to port's object id
            attr_value = sai_thrift_attribute_value_t(oid=acl_table_id)
            attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_EGRESS_ACL, value=attr_value)
            self.client.sai_thrift_set_port_attribute(swports[1], attr)

            # verify i2e mirrored packet
            # dropped at egress
            verify_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
            # verify mod packet as mod wins
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])

            verify_no_other_packets(self)
            print "DTel reports received for dropped packet"

            #self.client.switch_api_dtel_event_set_dscp(
            #    device, SWITCH_DTEL_EVENT_TYPE_DROP_REPORT, 5)
            #self.assertTrue(self.client.switch_api_dtel_event_get_dscp(
            #    device, SWITCH_DTEL_EVENT_TYPE_DROP_REPORT)==5)
            event = sai_mgr.create_dtel_event(
                SAI_DTEL_EVENT_TYPE_DROP_REPORT, 5);
            exp_mod_pkt[IP].tos = 5<<2
            send_packet(self, swports[0], str(int_pkt))
            # packet is dropped
            #verify_packet(self, exp_pkt, swports[1])

            # verify i2e mirrored packet
            # dropped at egress
            verify_dtel_packet(
                self, exp_i2e_pkt, swports[params.report_ports[0]])
            # verify mod packet as mod wins
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            print "Drop report received with modified DSCP"

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            split_mirror_on_drop_pkt()
            sai_mgr.switch.dtel_drop_report_enable = False
            sai_mgr.switch.dtel_int_endpoint_enable = False
            # unbind port
            sai_thrift_set_port_attribute(
                self.client,
                swports[1],
                SAI_PORT_ATTR_EGRESS_ACL,
                SAI_NULL_OBJECT_ID)
            # delete ACL
            self.client.sai_thrift_remove_acl_entry(acl_entry_id)
            self.client.sai_thrift_remove_acl_table(acl_table_id)
            sai_mgr.cleanup()
