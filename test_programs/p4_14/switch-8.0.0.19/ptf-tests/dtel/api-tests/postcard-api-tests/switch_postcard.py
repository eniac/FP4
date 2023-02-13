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

import ptf.dataplane as dataplane

from ptf.testutils import *
from ptf.thriftutils import *
from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../../base'))
from common.utils import *
sys.path.append(os.path.join(this_dir, '../../../base/api-tests'))
import api_base_tests

sys.path.append(os.path.join(this_dir, '../..'))
from dtel_utils import *

import pd_base_tests

device = 0
swports = [0, 1, 2]
devports = range(4)
#swports = [0, 4, 8, 256]
#devports = [188, 184, 180, 64]
switch_id = 0x11111111

params = SwitchConfig_Params()
params.switch_id = switch_id
params.mac_self = '00:77:66:55:44:33'
params.nports = 3
params.ipaddr_inf = ['10.1.0.1',  '10.2.0.1',  '10.3.0.1']
params.ipaddr_nbr = ['10.11.0.1', '10.12.0.1', '10.13.0.1']
params.mac_nbr = ['00:11:22:33:44:55', '00:11:22:33:44:56', '00:11:22:33:44:57']
params.report_ports = [2]
params.ipaddr_report_src = ['4.4.4.1']
params.ipaddr_report_dst = ['4.4.4.3']
params.mirror_ids = [555]
params.device = device
params.swports = swports

quantization_shift = MAX_QUANTIZATION

if test_param_get('target') == "asic-model":
    reset_cycle = 6
    min_sleeptime = 30
elif test_param_get('target') == "bmv2":
    reset_cycle = 1
    min_sleeptime = 5
else:
    reset_cycle = 1
    min_sleeptime = 1

twl_kvp = []
kvp_val = switcht_twl_value_t(value_num=ipv4Addr_to_i32(params.ipaddr_nbr[0]))
kvp_mask = switcht_twl_value_t(value_num=0xffffff00)
twl_kvp.append(switcht_twl_key_value_pair_t(
    SWITCH_TWL_FIELD_IPV4_SRC, kvp_val, kvp_mask))
kvp_val = switcht_twl_value_t(value_num=ipv4Addr_to_i32(params.ipaddr_nbr[1]))
kvp_mask = switcht_twl_value_t(value_num=0xffffff00)
twl_kvp.append(switcht_twl_key_value_pair_t(
    SWITCH_TWL_FIELD_IPV4_DST, kvp_val, kvp_mask))

################################################################################
@group('simple')
class PostcardSimpleTest(api_base_tests.ThriftInterfaceDataPlane,
                         pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print
        print "Test simple postcard"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(swport_to_devport(self, swports[params.report_ports[0]]))
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
                pktlen=128)

            exp_pkt_out = simple_tcp_packet(
                eth_dst=params.mac_nbr[1], # 00:11:22:33:44:56
                eth_src=params.mac_self,   # 00:77:66:55:44:33
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                tcp_flags=None,
                ip_ttl=63,
                pktlen=128)

            exp_postcard_inner = postcard_report(
                packet=exp_pkt_out,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_postcard_pkt = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=exp_postcard_inner)

            # send a test packet
            send_packet(self, swports[0], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[1])
            # verify potcard packet
            verify_postcard_packet(
                self, exp_postcard_pkt, swports[params.report_ports[0]])
            #receive_print_packet(
            #    self, swports[params.report_ports[0]], exp_postcard_pkt, True)
            verify_no_other_packets(self)
            print "Passed for the 1st pkt."

        # cleanup
        finally:
            #raw_input("press any key to cleanup ... ")
            split_postcard_pkt()
            self.client.switch_api_dtel_postcard_disable(device)
            self.client.switch_api_dtel_postcard_watchlist_clear(device)
            config.cleanup(self)

@group('vlan')
@group('postcard')
class Postcard_Over_VLAN_Test(api_base_tests.ThriftInterfaceDataPlane,
                              pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print
        print "Test Postcard generating DTel report over VLAN port"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        bind_postcard_pkt()

        vlan_id = 10
        # set DTEL report port as vlan port
        params.vlans = {vlan_id: {params.report_ports[0]:True}}
        config = SwitchConfig(self, params)

        self.client.switch_api_dtel_latency_quantization_shift(
            device, quantization_shift)

        # add flow space to postcard watchlist
        ap = switcht_twl_postcard_params_t(
            report_all_packets=True, flow_sample_percent=100)
        self.client.switch_api_dtel_postcard_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # run test
        try:
            pkt_in = simple_tcp_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[params.report_ports[0]],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                ip_ttl=64,
                tcp_flags=None,
                pktlen=128)

            exp_pkt_out = simple_tcp_packet(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[params.report_ports[0]],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                tcp_flags=None,
                ip_ttl=63,
                dl_vlan_enable=True,
                vlan_vid=vlan_id,
                pktlen=132)

            # send a test packet
            send_packet(self, swports[0], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[params.report_ports[0]])
            print "Passed for the test pkt to the vlan report port"

            # enable Postcard
            self.client.switch_api_dtel_postcard_enable(device)

            pkt_in = simple_tcp_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                ip_ttl=64,
                tcp_flags=None,
                pktlen=128)

            exp_pkt_out = simple_tcp_packet(
                eth_dst=params.mac_nbr[1], # 00:11:22:33:44:56
                eth_src=params.mac_self,   # 00:77:66:55:44:33
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                tcp_flags=None,
                ip_ttl=63,
                pktlen=128)

            exp_postcard_inner = postcard_report(
                packet=exp_pkt_out,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_postcard_pkt = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=exp_postcard_inner)

            # send a test packet
            send_packet(self, swports[0], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[1])
            verify_postcard_packet(
                self, exp_postcard_pkt, swports[params.report_ports[0]])
            #receive_print_packet(
            #    self, swports[params.report_ports[0]], exp_postcard_pkt, True)
            verify_no_other_packets(self)
            print "Passed for the postcard pkt."

        # cleanup
        finally:
            #raw_input("press any key to cleanup ... ")
            split_postcard_pkt()
            self.client.switch_api_dtel_postcard_disable(device)
            self.client.switch_api_dtel_postcard_watchlist_clear(device)
            config.cleanup(self)
            params.vlans=None

@group('postcard')
class PostcardTest(api_base_tests.ThriftInterfaceDataPlane,
                    pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print
        print "Test postcard"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        bind_postcard_pkt()
        config = SwitchConfig(self, params)

        # enable Postcard
        self.client.switch_api_dtel_postcard_enable(device)
        self.client.switch_api_dtel_latency_quantization_shift(
            device, quantization_shift)

        # add flow space to postcard watchlist
        ap = switcht_twl_postcard_params_t(
            report_all_packets=False, flow_sample_percent=100)
        self.client.switch_api_dtel_postcard_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        print "Clear bloom filters, can take up to %d secs." %min_sleeptime
        self.client.switch_api_dtel_flow_state_clear_cycle(
            device, reset_cycle)
        time.sleep(min_sleeptime)
        print "Disable bloom filter clearing."
        self.client.switch_api_dtel_flow_state_clear_cycle(device, 0)
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
                eth_dst=params.mac_nbr[1], # 00:11:22:33:44:56
                eth_src=params.mac_self,   # 00:77:66:55:44:33
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                tcp_flags=None,
                ip_ttl=63,
                pktlen=256)

            exp_postcard_inner_1 = postcard_report(
                packet=exp_pkt_out,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_postcard_pkt_1 = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
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
                switch_id=switch_id,
                ingress_port=swports[2],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_poscard_pkt_2 = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=exp_postcard_inner_2)

            self.client.switch_api_dtel_postcard_watchlist_entry_delete(
                device, twl_kvp)
            # send the same test packet through different port
            send_packet(self, swports[2], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[1])
            verify_no_other_packets(self)
            print "Passed for watchlist_entry_delete api"
            self.client.switch_api_dtel_postcard_watchlist_entry_create(
                device, twl_kvp, priority=1, watch=True, action_params=ap)

            # send the same test packet through different port
            send_packet(self, swports[2], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[1])
            # verify potcard packet
            verify_postcard_packet(
                self, exp_poscard_pkt_2, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for identical pkt from a different port"

            vxlan_gpe_pkt = simple_vxlan_gpe_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_id=0,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_ttl=64,
                udp_sport=101,
                udp_dport=4790,
                with_udp_chksum=False,
                vxlan_vni=0xaaaa,
                inner_frame=pkt_in)

            exp_vxlan_pkt_out = simple_vxlan_gpe_packet(
                eth_dst=params.mac_nbr[1], # 00:11:22:33:44:56
                eth_src=params.mac_self,   # 00:77:66:55:44:33
                ip_id=0,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_ttl=63,
                udp_sport=101,
                udp_dport=4790,
                with_udp_chksum=False,
                vxlan_vni=0xaaaa,
                inner_frame=pkt_in)

            exp_postcard_vxlan_inner = postcard_report(
                packet=exp_vxlan_pkt_out,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_poscard_vxlan_pkt = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=exp_postcard_vxlan_inner)

            send_packet(self, swports[0], str(vxlan_gpe_pkt))
            # verify packet out
            verify_packet(self, exp_vxlan_pkt_out, swports[1])
            # verify potcard packet
            verify_postcard_packet(
                self, exp_poscard_vxlan_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for the vxlan_gpe packet."

            print "Enable bloom filter clearing"

            # first use a very large cycle value and change later
            # to make sure the change goes in immidiately not per cycle
            self.client.switch_api_dtel_flow_state_clear_cycle(
                device, 10000)
            time.sleep(2);

            self.client.switch_api_dtel_flow_state_clear_cycle(
                device, reset_cycle)
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
            self.client.switch_api_dtel_latency_quantization_shift(
                device, 0)
            self.client.switch_api_dtel_flow_state_clear_cycle(device, 0)
            time.sleep(2*reset_cycle)

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
                if receive_postcard_packet(self, exp_postcard_pkt_1,
                                           swports[params.report_ports[0]]):
                    num_rcvd_pkts += 1
                verify_no_other_packets(self)

            report_ratio = num_rcvd_pkts * 1.0 / num_send_pkts
            print "Report ratio is", report_ratio
            self.assertTrue(report_ratio > 0.0, "Report ratio is zero!")

            print "Passed for the 2nd pkt w/ zero quantization shift."

            self.client.switch_api_dtel_latency_quantization_shift(
                device, MAX_QUANTIZATION)
            send_packet(self, swports[0], str(pkt_in))
            verify_packet(self, exp_pkt_out, swports[1])
            # ignoring latency may change local digest
            receive_postcard_packet(self, exp_postcard_pkt_1,
                                           swports[params.report_ports[0]])
            verify_no_other_packets(self)
            num_rcvd_pkts = 0
            for i in range(num_send_pkts):
                send_packet(self, swports[0], str(pkt_in))
                verify_packet(self, exp_pkt_out, swports[1])
                if receive_postcard_packet(self, exp_postcard_pkt_1,
                                           swports[params.report_ports[0]]):
                    num_rcvd_pkts += 1
                verify_no_other_packets(self)

            self.assertTrue(num_rcvd_pkts == 0, "recevied at least one report packet!")

            print "Passed for disabling latency in flow change detection."

        # cleanup
        finally:
            split_postcard_pkt()
            self.client.switch_api_dtel_postcard_disable(device)
            self.client.switch_api_dtel_postcard_watchlist_clear(device)
            config.cleanup(self)

@group('postcard')
@group('mod')
class PostcardMoDTest(api_base_tests.ThriftInterfaceDataPlane,
                     pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])
    def runTest(self):
        print
        print "Test postcard with mirror on drop"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        bind_postcard_pkt()
        bind_mirror_on_drop_pkt()
        config = SwitchConfig(self, params)

        mtu_set = False
        system_acl_rule_created = False

        # enable Postcard and MoD
        self.client.switch_api_dtel_drop_report_enable(device)
        self.client.switch_api_dtel_postcard_enable(device)
        self.client.switch_api_dtel_latency_quantization_shift(
            device, quantization_shift)

        # add flow space to postcard and MoD watchlist
        ap = switcht_twl_postcard_params_t(
            report_all_packets=False, flow_sample_percent=100)
        self.client.switch_api_dtel_postcard_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        ap = switcht_twl_mod_params_t(report_queue_tail_drops=False)
        self.client.switch_api_dtel_drop_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # run test
        queue_report_port_1_enabled = False
        try:
            pkt_in = simple_tcp_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                tcp_flags=None,
                ip_ttl=64,
                pktlen=256)

            exp_pkt_out = simple_tcp_packet(
                eth_dst=params.mac_nbr[1], # 00:11:22:33:44:56
                eth_src=params.mac_self,   # 00:77:66:55:44:33
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                tcp_flags=None,
                ip_id=105,
                ip_ttl=63,
                pktlen=256)

            exp_postcard_inner_1 = postcard_report(
                packet=exp_pkt_out,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_postcard_pkt_1 = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=exp_postcard_inner_1)

            exp_mod_inner_1 = mod_report(
                packet=exp_pkt_out,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                drop_reason=70)

            exp_mod_pkt = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_MOD,
                dropped=1,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=exp_mod_inner_1)

            # create system_acl rule to drop packets exceeding MTU
            acl = self.client.switch_api_acl_list_create(
                device, SWITCH_API_DIRECTION_EGRESS,
                SWITCH_ACL_TYPE_EGRESS_SYSTEM, SWITCH_HANDLE_TYPE_NONE)
            system_acl_rule_created = True
            acl_kvp = []
            acl_kvp_val = switcht_acl_value_t(value_num=0)
            acl_kvp_mask = switcht_acl_value_t(value_num=1)
            acl_kvp.append(switcht_acl_key_value_pair_t(
                SWITCH_ACL_EGRESS_SYSTEM_FIELD_DEFLECT, acl_kvp_val, acl_kvp_mask))
            acl_kvp_val = switcht_acl_value_t(value_num=0)
            acl_kvp_mask = switcht_acl_value_t(value_num=0xffff)
            acl_kvp.append(switcht_acl_key_value_pair_t(
                SWITCH_ACL_EGRESS_SYSTEM_FIELD_L3_MTU_CHECK, acl_kvp_val, acl_kvp_mask))
            action = SWITCH_ACL_EGRESS_SYSTEM_ACTION_DROP
            action_params = switcht_acl_action_params_t(
                drop=switcht_acl_action_drop(reason_code=70))
            opt_action_params = switcht_acl_opt_action_params_t()
            ace = self.client.switch_api_acl_egress_system_rule_create(
                device, acl, 3000, 2, acl_kvp, action, action_params,
                opt_action_params)

            # mirror on drop vs. postcard
            mtu_200 = self.client.switch_api_l3_mtu_create(
                device, SWITCH_MTU_TYPE_IPV4, 200)
            self.client.switch_api_rif_mtu_set(
                device, config.rifs[1], mtu_200)
            mtu_set = True

            send_packet(self, swports[0], str(pkt_in))
            # verify mirror on drop packet
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for the pkt with mirror on drop"

            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_DROP_REPORT, 5)
            exp_mod_pkt[IP].tos = 5<<2
            send_packet(self, swports[0], str(pkt_in))
            # verify mirror on drop packet
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for the pkt with mirror on drop new dscp"

            bind_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT^0x1111)
            split_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT)
            self.client.switch_api_dtel_report_udp_dstport_set(
                device, UDP_PORT_DTEL_REPORT^0x1111);
            send_packet(self, swports[0], str(pkt_in))
            exp_mod_pkt[UDP].dport = UDP_PORT_DTEL_REPORT^0x1111
            # verify mirror on drop packet
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            self.client.switch_api_dtel_report_udp_dstport_set(
                device, UDP_PORT_DTEL_REPORT);
            split_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT^0x1111)
            bind_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT)
            exp_mod_pkt[UDP].dport = UDP_PORT_DTEL_REPORT
            print "Passed for the pkt with mirror on drop new report UDP port"

            # postcard + mod + qreport
            self.client.switch_api_dtel_queue_report_create(
                device, swports[1], 0, 0, 0, 1, False)
            queue_report_port_1_enabled = True
            exp_mod_pkt[DTEL_REPORT_HDR].congested_queue = 1
            send_packet(self, swports[0], str(pkt_in))
            # verify mirror on drop packet
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)

            # quota finished now
            exp_mod_pkt[DTEL_REPORT_HDR].congested_queue = 0
            send_packet(self, swports[0], str(pkt_in))
            # verify mirror on drop packet
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)

            self.client.switch_api_dtel_queue_report_delete(
                device, swports[1], 0)
            queue_report_port_1_enabled = False

            print "Passed for the pkt with mod and queue report"

        # cleanup
        finally:
            self.client.switch_api_dtel_report_udp_dstport_set(
                device, UDP_PORT_DTEL_REPORT);
            split_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT^0x1111)
            bind_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT)
            split_mirror_on_drop_pkt()
            split_postcard_pkt()
            if queue_report_port_1_enabled:
                self.client.switch_api_dtel_queue_report_delete(
                    device, swports[1], 0)
            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_DROP_REPORT, 0)
            if mtu_set:
                self.client.switch_api_l3_mtu_delete(device, mtu_200)
            self.client.switch_api_dtel_postcard_disable(device)
            self.client.switch_api_dtel_drop_report_disable(device)
            self.client.switch_api_dtel_postcard_watchlist_clear(device)
            self.client.switch_api_dtel_drop_watchlist_clear(device)
            if system_acl_rule_created:
                self.client.switch_api_acl_rule_delete(device, acl, ace)
                self.client.switch_api_acl_list_delete(device, acl)
            config.cleanup(self)

@group('postcard')
@group('watchlist')
class PostcardWatchlistTest(api_base_tests.ThriftInterfaceDataPlane,
                     pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])
    def runTest(self):
        print
        print "Test postcard watchlist"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        bind_postcard_pkt()
        config = SwitchConfig(self, params)

        # enable Postcard
        self.client.switch_api_dtel_postcard_enable(device)

        # run test
        try:
            # add flow space to postcard watchlist
            # redefine these with /28 no suppression
            twl_kvp = []
            kvp_val = switcht_twl_value_t(
                value_num=ipv4Addr_to_i32(params.ipaddr_nbr[0]))
            kvp_mask = switcht_twl_value_t(value_num=0xfffffff0)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_IPV4_SRC, kvp_val, kvp_mask))
            kvp_val = switcht_twl_value_t(
                value_num=ipv4Addr_to_i32(params.ipaddr_nbr[1]))
            kvp_mask = switcht_twl_value_t(value_num=0xfffffff0)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_IPV4_DST, kvp_val, kvp_mask))
            ap = switcht_twl_postcard_params_t(
                report_all_packets=True, flow_sample_percent=100)
            self.client.switch_api_dtel_postcard_watchlist_entry_create(
                device, twl_kvp, priority=2, watch=True, action_params=ap)

            pkt_in_1 = simple_tcp_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                tcp_flags=None,
                ip_ttl=64,
                tcp_sport=8888,
                tcp_dport=9999)

            exp_pkt_out_1 = simple_tcp_packet(
                eth_dst=params.mac_nbr[1], # 00:11:22:33:44:56
                eth_src=params.mac_self,   # 00:77:66:55:44:33
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                tcp_flags=None,
                ip_id=105,
                ip_ttl=63,
                tcp_sport=8888,
                tcp_dport=9999)

            exp_postcard_inner_1 = postcard_report(
                packet=exp_pkt_out_1,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_postcard_pkt_1 = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=exp_postcard_inner_1)

            # send a test packet
            send_packet(self, swports[0], str(pkt_in_1))
            # verify packet out
            verify_packet(self, exp_pkt_out_1, swports[1])
            # verify potcard packet
            verify_postcard_packet(
                self, exp_postcard_pkt_1, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for the 1st pkt, in watchlist"
            ip2 = params.ipaddr_nbr[1]
            ip2 = ip2[:ip2.rfind('.')]+'.129'

            pkt_in_2 = simple_tcp_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[1],
                ip_src=ip2,
                ip_id=105,
                tcp_flags=None,
                ip_ttl=64,
                tcp_sport=8888,
                tcp_dport=9999)

            exp_pkt_2 = simple_tcp_packet(
                eth_dst=params.mac_nbr[1], # 00:11:22:33:44:56
                eth_src=params.mac_self,   # 00:77:66:55:44:33
                ip_dst=params.ipaddr_nbr[1],
                ip_src=ip2,
                ip_id=105,
                tcp_flags=None,
                ip_ttl=63,
                tcp_sport=8888,
                tcp_dport=9999)

            exp_postcard_inner_2 = postcard_report(
                packet=exp_pkt_2,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_poscard_pkt_2 = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=exp_postcard_inner_2)

            # send a test packet
            send_packet(self, swports[0], str(pkt_in_2))
            # verify packet out
            verify_packet(self, exp_pkt_2, swports[1])
            verify_no_other_packets(self)
            print "Passed for the 2nd pkt, not in watchlist"

            # now add the rule in watchlist and we should get postcard
            twl_kvp = []
            kvp_val = switcht_twl_value_t(
                value_num=ipv4Addr_to_i32(ip2))
            kvp_mask = switcht_twl_value_t(value_num=0xfffffff0)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_IPV4_SRC, kvp_val, kvp_mask))
            kvp_val = switcht_twl_value_t(
                value_num=ipv4Addr_to_i32(params.ipaddr_nbr[1]))
            kvp_mask = switcht_twl_value_t(value_num=0xfffffff0)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_IPV4_DST, kvp_val, kvp_mask))
            ap = switcht_twl_postcard_params_t(
                report_all_packets=True, flow_sample_percent=100)
            self.client.switch_api_dtel_postcard_watchlist_entry_create(
                device, twl_kvp, priority=3, watch=True, action_params=ap)

            # send a test packet
            send_packet(self, swports[0], str(pkt_in_2))
            # verify packet out
            verify_packet(self, exp_pkt_2, swports[1])
            # verify potcard packet
            verify_postcard_packet(
                self, exp_poscard_pkt_2, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for the 3rd pkt, in watchlist"

            self.client.switch_api_dtel_postcard_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            # send a test packet
            send_packet(self, swports[0], str(pkt_in_2))
            # verify packet out
            verify_packet(self, exp_pkt_2, swports[1])
            verify_no_other_packets(self)
            print "Passed for the 4th pkt, not in watchlist"

            # tcp: 8888 -> 9999, no suppress
            twl_kvp = []
            kvp_val = switcht_twl_value_t(value_num=6)
            kvp_mask = switcht_twl_value_t(value_num=0xff)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_IP_PROTO, kvp_val, kvp_mask))
            kvp_val = switcht_twl_value_t(value_num=8888)
            kvp_mask = switcht_twl_value_t(value_num=0xffff)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_L4_PORT_SRC_START, kvp_val, kvp_mask))
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_L4_PORT_SRC_END, kvp_val, kvp_mask))
            kvp_val = switcht_twl_value_t(value_num=9999)
            kvp_mask = switcht_twl_value_t(value_num=0xffff)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_L4_PORT_DST_START, kvp_val, kvp_mask))
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_L4_PORT_DST_END, kvp_val, kvp_mask))
            ap = switcht_twl_postcard_params_t(
                report_all_packets=True, flow_sample_percent=100)
            self.client.switch_api_dtel_postcard_watchlist_entry_create(
                device, twl_kvp, priority=10, watch=True, action_params=ap)

            # send a test packet
            send_packet(self, swports[0], str(pkt_in_2))
            # verify packet out
            verify_packet(self, exp_pkt_2, swports[1])
            # verify potcard packet
            verify_postcard_packet(
                self, exp_poscard_pkt_2, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for the 5th pkt, in watchlist"

            # * all flows: watch=False
            twl_kvp = []
            ap = switcht_twl_postcard_params_t(report_all_packets=True)
            self.client.switch_api_dtel_postcard_watchlist_entry_create(
                device, twl_kvp, priority=1, watch=False, action_params=ap)

            send_packet(self, swports[0], str(pkt_in_1))
            verify_packet(self, exp_pkt_out_1, swports[1])
            verify_no_other_packets(self)
            print "Passed for the 6st pkt, in watchlist, not watch"
            send_packet(self, swports[0], str(pkt_in_2))
            verify_packet(self, exp_pkt_2, swports[1])
            verify_no_other_packets(self)
            print "Passed for the 7th pkt, in watchlist, not watch"

            self.client.switch_api_dtel_postcard_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            send_packet(self, swports[0], str(pkt_in_1))
            verify_packet(self, exp_pkt_out_1, swports[1])
            verify_postcard_packet(
                self, exp_postcard_pkt_1, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for the 8st pkt, in watchlist"

            old_type = pkt_in_1[Ether].type
            pkt_in_1[Ether].type=0x1234
            send_packet(self, swports[0], str(pkt_in_1))
            # packet will be dropped
            verify_no_other_packets(self)
            pkt_in_1[Ether].type = old_type
            print "Passed for non-IP pkt in watchlist"

            print "Started flow sampling in watchlist"
            # now sampling
            percent = 50
            pkts_num = 100
            self.client.switch_api_dtel_postcard_watchlist_clear(device)
            # tcp: 8888 -> 9999, no suppress
            twl_kvp = []
            kvp_val = switcht_twl_value_t(value_num=6)
            kvp_mask = switcht_twl_value_t(value_num=0xff)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_IP_PROTO, kvp_val, kvp_mask))
            kvp_val = switcht_twl_value_t(value_num=8888)
            kvp_mask = switcht_twl_value_t(value_num=0xffff)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_L4_PORT_SRC, kvp_val, kvp_mask))
            kvp_val = switcht_twl_value_t(value_num=9999)
            kvp_mask = switcht_twl_value_t(value_num=0xffff)
            twl_kvp.append(switcht_twl_key_value_pair_t(
                SWITCH_TWL_FIELD_L4_PORT_DST, kvp_val, kvp_mask))
            ap = switcht_twl_postcard_params_t(
                report_all_packets=True, flow_sample_percent=percent)
            self.client.switch_api_dtel_postcard_watchlist_entry_create(
                device, twl_kvp, priority=10, watch=True, action_params=ap)

            postcard_pkts=0
            for i in range(0, pkts_num):
                ip_src=params.ipaddr_nbr[0]
                ip_src = ip_src[0:(ip_src.rfind('.')+1)]+('%d'% i)
                pkt_in_1[IP].src=ip_src
                exp_pkt_out_1[IP].src=ip_src
                exp_postcard_pkt_1.getlayer(IP,2).src=ip_src
                send_packet(self, swports[0], str(pkt_in_1))
                verify_packet(self, exp_pkt_out_1, swports[1])
                if receive_postcard_packet(self, exp_postcard_pkt_1,
                                           swports[params.report_ports[0]]):
                    postcard_pkts += 1
                verify_no_other_packets(self)
            self.assertTrue(postcard_pkts>=pkts_num*percent/100 *0.8 and
                            postcard_pkts<=pkts_num*percent/100 *1.2,
                            "Expected %f percent postcard "
                            "packets but received %f percent" %(
                                percent, 100.0 * postcard_pkts / pkts_num))
            print "Passed for flow sampling in watchlist"

        # cleanup
        finally:
            split_postcard_pkt()
            self.client.switch_api_dtel_postcard_disable(device)
            self.client.switch_api_dtel_postcard_watchlist_clear(device)
            config.cleanup(self)

@group('postcard')
class PostcardQueueReportTest(api_base_tests.ThriftInterfaceDataPlane,
                     pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])
    def runTest(self):
        print
        print "Test postcard queue report"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        bind_postcard_pkt()
        config = SwitchConfig(self, params)

        # enable Postcard
        self.client.switch_api_dtel_postcard_enable(device)

        # add flow space to postcard watchlist
        ap = switcht_twl_postcard_params_t(
            report_all_packets=False, flow_sample_percent=100)
        self.client.switch_api_dtel_postcard_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        print "Clear bloom filters, can take up to %d secs." %min_sleeptime
        self.client.switch_api_dtel_flow_state_clear_cycle(
            device, reset_cycle)
        time.sleep(min_sleeptime)
        print "Disable bloom filter clearing."
        self.client.switch_api_dtel_flow_state_clear_cycle(device, 0)
        time.sleep(2*reset_cycle)

        queue_report_port_1_enabled = False
        queue_report_ports_0_2_enabled = False

        try:
            pkt_in = simple_tcp_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                tcp_flags=None,
                ip_ttl=64,
                pktlen=256)

            exp_pkt_out = simple_tcp_packet(
                eth_dst=params.mac_nbr[1], # 00:11:22:33:44:56
                eth_src=params.mac_self,   # 00:77:66:55:44:33
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                tcp_flags=None,
                ip_id=105,
                ip_ttl=63,
                pktlen=256)

            exp_postcard_inner = postcard_report(
                packet=exp_pkt_out,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_poscard_pkt = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=exp_postcard_inner)

            send_packet(self, swports[0], str(pkt_in))
            verify_packet(self, exp_pkt_out, swports[1])
            verify_postcard_packet(
                self, exp_poscard_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for the 1st pkt, normal postcard"

            send_packet(self, swports[0], str(pkt_in))
            verify_packet(self, exp_pkt_out, swports[1])
            verify_no_other_packets(self)
            print "Passed for identical pkt from same port, postcard suppress"

            # set queue report
            self.client.switch_api_dtel_queue_report_create(
                device, swports[1], 0, 0, 0, 1024, False)
            queue_report_port_1_enabled = True
            exp_poscard_pkt[DTEL_REPORT_HDR].congested_queue = 1

            send_packet(self, swports[0], str(pkt_in))
            verify_packet(self, exp_pkt_out, swports[1])
            verify_postcard_packet(
                self, exp_poscard_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)

            quota = self.client.switch_api_dtel_queue_remaining_report_quota_during_breach_get(
                device, swports[1], 0);
            self.assertTrue(quota == 1023, "Remaining quota is not decremented")
            print "Passed for identical pkt with queue report"

            # disable queue report
            self.client.switch_api_dtel_queue_report_delete(
                device, swports[1], 0)
            queue_report_port_1_enabled = False
            send_packet(self, swports[0], str(pkt_in))
            verify_packet(self, exp_pkt_out, swports[1])
            verify_no_other_packets(self)
            print "Passed for identical pkt without queue report"


            self.client.switch_api_dtel_queue_report_create(
                device, swports[1], 0, 0, 0, 1024, False)
            queue_report_port_1_enabled = True
            self.client.switch_api_dtel_queue_report_create(
                device, swports[2], 0, 0, 0, 1024, False)
            self.client.switch_api_dtel_queue_report_create(
                device, swports[0], 0, 0, 0, 1024, False)
            queue_report_ports_0_2_enabled = True

            send_packet(self, swports[0], str(pkt_in))
            verify_packet(self, exp_pkt_out, swports[1])
            verify_postcard_packet(
                self, exp_poscard_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for identical pkt with queue report on multiple ports"

            self.client.switch_api_dtel_queue_report_delete(
                device, swports[2], 0)
            self.client.switch_api_dtel_queue_report_delete(
                device, swports[0], 0)
            queue_report_ports_0_2_enabled = False

            send_packet(self, swports[0], str(pkt_in))
            verify_packet(self, exp_pkt_out, swports[1])
            verify_postcard_packet(
                self, exp_poscard_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for identical pkt with queue report on one port"

            # set queue report with max threshold
            self.client.switch_api_dtel_queue_report_update(
                device, swports[1], 0,
                hex_to_i32(0xfff), hex_to_i32(0xffffffff), 4, False)
            send_packet(self, swports[0], str(pkt_in))
            verify_packet(self, exp_pkt_out, swports[1])
            verify_no_other_packets(self)
            print "Passed for identical pkt with high queue threshold"

            # set queue report
            self.client.switch_api_dtel_queue_report_update(
                device, swports[1], 0, 0, 0, 4, False)

            print "Clear bloom filters, can take up to %d secs." %min_sleeptime
            self.client.switch_api_dtel_flow_state_clear_cycle(
                device, reset_cycle)
            time.sleep(min_sleeptime)

            send_packet(self, swports[0], str(pkt_in))
            verify_packet(self, exp_pkt_out, swports[1])
            verify_postcard_packet(
                self, exp_poscard_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for identical pkt, postcard + q report"

            send_packet(self, swports[0], str(pkt_in))
            verify_packet(self, exp_pkt_out, swports[1])
            verify_postcard_packet(
                self, exp_poscard_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for identical pkt with queue report"

            # should add no watchlist cases to test path_tracking_flow=0

        # cleanup
        finally:
            split_postcard_pkt()
            if queue_report_port_1_enabled:
                self.client.switch_api_dtel_queue_report_delete(
                    device, swports[1], 0)
            if queue_report_ports_0_2_enabled:
                self.client.switch_api_dtel_queue_report_delete(
                    device, swports[2], 1)
                self.client.switch_api_dtel_queue_report_delete(
                    device, swports[0], 3)
            self.client.switch_api_dtel_postcard_disable(device)
            self.client.switch_api_dtel_postcard_watchlist_clear(device)
            config.cleanup(self)

@group('postcard')
class PostcardDSCPTest(api_base_tests.ThriftInterfaceDataPlane,
                     pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])
    def runTest(self):
        print
        print "Test postcard while changing DSCP of reports"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        bind_postcard_pkt()
        config = SwitchConfig(self, params)

        # enable Postcard
        self.client.switch_api_dtel_postcard_enable(device)
        self.client.switch_api_dtel_latency_quantization_shift(
            device, quantization_shift)

        # add flow space to postcard watchlist
        ap = switcht_twl_postcard_params_t(
            report_all_packets=False, flow_sample_percent=100)
        self.client.switch_api_dtel_postcard_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        print "Clear bloom filters, can take up to %d secs." %min_sleeptime
        self.client.switch_api_dtel_flow_state_clear_cycle(
            device, reset_cycle)
        time.sleep(min_sleeptime)
        print "Disable bloom filter clearing."
        self.client.switch_api_dtel_flow_state_clear_cycle(device, 0)
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
                tcp_flags="S",
                pktlen=256)

            exp_pkt_out = simple_tcp_packet(
                eth_dst=params.mac_nbr[1], # 00:11:22:33:44:56
                eth_src=params.mac_self,   # 00:77:66:55:44:33
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                tcp_flags="S",
                ip_ttl=63,
                pktlen=256)

            exp_postcard_inner_1 = postcard_report(
                packet=exp_pkt_out,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_postcard_pkt_1 = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=exp_postcard_inner_1)

            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_FLOW_STATE_CHANGE, 5)
            self.assertTrue(self.client.switch_api_dtel_event_get_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_FLOW_STATE_CHANGE)==5)
            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_FLOW_TCPFLAG, 3)

            exp_postcard_pkt_1[IP].tos = 5 <<2
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

            exp_postcard_pkt_1[IP].tos = 3 <<2
            # send the same test packet again now reported becasue of TCP
            send_packet(self, swports[0], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[1])
            verify_postcard_packet(
                self, exp_postcard_pkt_1, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for identical pkt from the same flow"

            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_FLOW_TCPFLAG, 6)
            exp_postcard_pkt_1[IP].tos = 6 <<2
            # send the same test packet again now reported becasue of TCP
            send_packet(self, swports[0], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[1])
            verify_postcard_packet(
                self, exp_postcard_pkt_1, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for identical pkt from the same flow change dscp"

            self.client.switch_api_dtel_postcard_watchlist_entry_delete(
                device, twl_kvp)
            # add flow space to postcard watchlist
            ap = switcht_twl_postcard_params_t(
                report_all_packets=True, flow_sample_percent=100)
            self.client.switch_api_dtel_postcard_watchlist_entry_create(
                device, twl_kvp, priority=1, watch=True, action_params=ap)
            # send the same test packet again now reported becasue of TCP
            send_packet(self, swports[0], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[1])
            verify_postcard_packet(
                self, exp_postcard_pkt_1, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for identical pkt TCP and report all"

            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS, 32)
            exp_postcard_pkt_1[IP].tos = 32 <<2
            # send the same test packet again now reported becasue report all
            send_packet(self, swports[0], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[1])
            verify_postcard_packet(
                self, exp_postcard_pkt_1, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for identical pkt TCP and report all new dscp"

        # cleanup
        finally:
            split_postcard_pkt()
            self.client.switch_api_dtel_postcard_disable(device)
            self.client.switch_api_dtel_postcard_watchlist_clear(device)
            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_FLOW_TCPFLAG, 0)
            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS, 0)
            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_FLOW_STATE_CHANGE, 0)
            config.cleanup(self)

@group('postcard')
class Postcard_ReportPortSet_Test(api_base_tests.ThriftInterfaceDataPlane,
                     pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])
    def runTest(self):
        print
        print "Test postcard with differnt report ports"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(swport_to_devport(self, swports[params.report_ports[0]]))
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

        print "Disable bloom filter clearing."
        self.client.switch_api_dtel_flow_state_clear_cycle(device, 0)
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
                pktlen=256)

            exp_pkt_out = simple_tcp_packet(
                eth_dst=params.mac_nbr[1], # 00:11:22:33:44:56
                eth_src=params.mac_self,   # 00:77:66:55:44:33
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                ip_ttl=63,
                pktlen=256)

            exp_postcard_inner_1 = postcard_report(
                packet=exp_pkt_out,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_postcard_pkt_1 = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=exp_postcard_inner_1)

            self.client.switch_api_dtel_report_udp_dstport_set(
                device, UDP_PORT_DTEL_REPORT^0x1111);
            bind_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT^0x1111)
            split_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT)

            exp_postcard_pkt_1[UDP].dport = UDP_PORT_DTEL_REPORT^0x1111

            # send a test packet
            send_packet(self, swports[0], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[1])
            # verify potcard packet
            verify_postcard_packet(
                self, exp_postcard_pkt_1, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for the 1st pkt."

            self.client.switch_api_dtel_report_udp_dstport_set(
                device, UDP_PORT_DTEL_REPORT);
            split_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT^0x1111)
            bind_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT)
            exp_postcard_pkt_1[UDP].dport = UDP_PORT_DTEL_REPORT

            # send a test packet
            send_packet(self, swports[0], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[1])
            # verify potcard packet
            verify_postcard_packet(
                self, exp_postcard_pkt_1, swports[params.report_ports[0]])
            print "Passed for the 2nd pkt."

        # cleanup
        finally:
            self.client.switch_api_dtel_report_udp_dstport_set(
                device, UDP_PORT_DTEL_REPORT);
            split_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT^0x1111)
            bind_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT)
            split_postcard_pkt()
            self.client.switch_api_dtel_postcard_disable(device)
            self.client.switch_api_dtel_postcard_watchlist_clear(device)
            config.cleanup(self)

@group('postcard')
class PostcardTCPTest(api_base_tests.ThriftInterfaceDataPlane,
                     pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])
    def runTest(self):
        print
        print "Test postcard with TCP flags"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        bind_postcard_pkt()
        config = SwitchConfig(self, params)

        # enable Postcard
        self.client.switch_api_dtel_postcard_enable(device)
        self.client.switch_api_dtel_latency_quantization_shift(
            device, quantization_shift)

        # add flow space to postcard watchlist
        ap = switcht_twl_postcard_params_t(
            report_all_packets=False, flow_sample_percent=100)
        self.client.switch_api_dtel_postcard_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        print "Clear bloom filters, can take up to %d secs." %min_sleeptime
        self.client.switch_api_dtel_flow_state_clear_cycle(
            device, reset_cycle)
        time.sleep(min_sleeptime)
        print "Disable bloom filter clearing."
        self.client.switch_api_dtel_flow_state_clear_cycle(device, 0)
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
                tcp_flags="S",
                pktlen=256)

            exp_pkt_out = simple_tcp_packet(
                eth_dst=params.mac_nbr[1], # 00:11:22:33:44:56
                eth_src=params.mac_self,   # 00:77:66:55:44:33
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                tcp_flags="S",
                ip_ttl=63,
                pktlen=256)

            exp_postcard_inner_1 = postcard_report(
                packet=exp_pkt_out,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_postcard_pkt_1 = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
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
            # verify potcard packet
            verify_postcard_packet(
                self, exp_postcard_pkt_1, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for identical pkt from the same flow outer SYN"

            pkt_in[TCP].flags="F"
            exp_pkt_out[TCP].flags="F"
            exp_postcard_pkt_1[TCP].flags="F"
            # send the same test packet again
            send_packet(self, swports[0], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[1])
            # verify potcard packet
            verify_postcard_packet(
                self, exp_postcard_pkt_1, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for identical pkt from the same flow outer FIN"

        # cleanup
        finally:
            split_postcard_pkt()
            self.client.switch_api_dtel_postcard_disable(device)
            self.client.switch_api_dtel_postcard_watchlist_clear(device)
            config.cleanup(self)

@group('postcard_inner')
class PostcardTCP_inner_Test(api_base_tests.ThriftInterfaceDataPlane,
                     pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])
    def runTest(self):
        print
        print "Test postcard with TCP flags"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        bind_postcard_pkt()
        config = SwitchConfig(self, params)

        # enable Postcard
        self.client.switch_api_dtel_postcard_enable(device)
        self.client.switch_api_dtel_latency_quantization_shift(
            device, quantization_shift)

        # add flow space to postcard watchlist
        ap = switcht_twl_postcard_params_t(
            report_all_packets=False, flow_sample_percent=100)
        self.client.switch_api_dtel_postcard_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        print "Clear bloom filters, can take up to %d secs." %min_sleeptime
        self.client.switch_api_dtel_flow_state_clear_cycle(
            device, reset_cycle)
        time.sleep(min_sleeptime)
        print "Disable bloom filter clearing."
        self.client.switch_api_dtel_flow_state_clear_cycle(device, 0)
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
                tcp_flags="S",
                pktlen=256)

            vxlan_pkt = simple_vxlan_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_id=0,
                ip_dst=params.ipaddr_nbr[1],
                ip_src='10.1.1.11',
                ip_ttl=64,
                udp_sport=101,
                with_udp_chksum=False,
                vxlan_vni=0xaaaa,
                inner_frame=pkt_in)

            exp_vxlan_pkt_out = simple_vxlan_packet(
                eth_dst=params.mac_nbr[1], # 00:11:22:33:44:56
                eth_src=params.mac_self,   # 00:77:66:55:44:33
                ip_id=0,
                ip_dst=params.ipaddr_nbr[1],
                ip_src='10.1.1.11',
                ip_ttl=63,
                udp_sport=101,
                with_udp_chksum=False,
                vxlan_vni=0xaaaa,
                inner_frame=pkt_in)

            exp_postcard_vxlan_inner = postcard_report(
                packet=exp_vxlan_pkt_out,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                queue_depth=0,
                egress_tstamp=0)

            exp_poscard_vxlan_pkt = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                dropped=0,
                congested_queue=0,
                path_tracking_flow=1,
                hw_id=hw_id,
                inner_frame=exp_postcard_vxlan_inner)

            send_packet(self, swports[0], str(vxlan_pkt))
            # verify packet out
            verify_packet(self, exp_vxlan_pkt_out, swports[1])
            # verify potcard packet
            verify_postcard_packet(
                self, exp_poscard_vxlan_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for the vxlan packet 1st time"

            send_packet(self, swports[0], str(vxlan_pkt))
            # verify packet out
            verify_packet(self, exp_vxlan_pkt_out, swports[1])
            # verify potcard packet
            verify_postcard_packet(
                self, exp_poscard_vxlan_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for the vxlan packet 2nd time because of inner TCP"

        # cleanup
        finally:
            split_postcard_pkt()
            self.client.switch_api_dtel_postcard_disable(device)
            self.client.switch_api_dtel_postcard_watchlist_clear(device)
            config.cleanup(self)

@group('postcard_dod')
class Postcard_DoDTest(api_base_tests.ThriftInterfaceDataPlane,
                           pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])
    def run_test_(self, pkt_in, exp_pkt, exp_i2e_pkt, exp_e2e_pkt,
                  exp_q_pkt, exp_dod_pkt):
        dtel_checkDoD(self, swports[0], swports[1],
                           swports[params.report_ports[0]],
                           pkt_in, exp_pkt, drop=False,
                           exp_i2e_pkt=exp_i2e_pkt, exp_e2e_pkt=exp_e2e_pkt)
        print "Passed for postcard egress + without DoD"

        ap = switcht_twl_mod_params_t(report_queue_tail_drops=True)
        self.client.switch_api_dtel_drop_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)
        self.mod_watchlist=True
        dtel_checkDoD(self, swports[0], swports[1],
                           swports[params.report_ports[0]],
                           pkt_in, exp_pkt,
                           True, exp_dod_pkt,
                           exp_i2e_pkt=exp_i2e_pkt, exp_e2e_pkt=exp_e2e_pkt)
        print "Passed for postcard egress + DOD"

        self.client.switch_api_dtel_event_set_dscp(
            device, SWITCH_DTEL_EVENT_TYPE_DROP_REPORT, 8)
        self.assertTrue(self.client.switch_api_dtel_event_get_dscp(
            device, SWITCH_DTEL_EVENT_TYPE_DROP_REPORT)==8)
        exp_dod_pkt[IP].tos = 8<<2
        dtel_checkDoD(self, swports[0], swports[1],
                           swports[params.report_ports[0]],
                           pkt_in, exp_pkt,
                           True, exp_dod_pkt,
                           exp_i2e_pkt=exp_i2e_pkt, exp_e2e_pkt=exp_e2e_pkt)
        print "Passed for postcard egress + DOD + new DSCP"

        self.client.switch_api_dtel_queue_report_create(
            device, swports[1], 0, 0, 0, 1024, False)
        self.queue_report_enabled = True
        dtel_checkDoD(self, swports[0], swports[1],
                           swports[params.report_ports[0]],
                           pkt_in, exp_pkt,
                           True, exp_dod_pkt,
                           exp_i2e_pkt=exp_i2e_pkt, exp_e2e_pkt=exp_q_pkt)
        print "Passed for postcard egress + DOD + Q"

        self.client.switch_api_dtel_queue_report_update(
            device, swports[1], 0, 0, 0, 1024, True)
        exp_dod_pkt[DTEL_REPORT_HDR].congested_queue = 1
        dtel_checkDoD(self, swports[0], swports[1],
                           swports[params.report_ports[0]],
                           pkt_in, exp_pkt,
                           True, exp_dod_pkt,
                           exp_i2e_pkt=exp_i2e_pkt, exp_e2e_pkt=exp_q_pkt)
        print "Passed for postcard egress + DOD + QDoD"

        self.client.switch_api_dtel_event_set_dscp(
            device, SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP, 6)
        self.assertTrue(self.client.switch_api_dtel_event_get_dscp(
            device, SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP)==6)
        dtel_checkDoD(self, swports[0], swports[1],
                           swports[params.report_ports[0]],
                           pkt_in, exp_pkt, True, exp_dod_pkt,
                           exp_i2e_pkt=exp_i2e_pkt, exp_e2e_pkt=exp_q_pkt)
        print "Passed for postcard egress + DOD + QDoD + new DSCP"

        ap = switcht_twl_mod_params_t(report_queue_tail_drops=False)
        self.client.switch_api_dtel_drop_watchlist_entry_update(
            device, twl_kvp, priority=1, watch=True, action_params=ap)
        # no dod in mod watchlist thus get dscp of q report
        exp_dod_pkt[IP].tos = 6<<2
        dtel_checkDoD(self, swports[0], swports[1],
                           swports[params.report_ports[0]],
                           pkt_in, exp_pkt,
                           True, exp_dod_pkt,
                           exp_i2e_pkt=exp_i2e_pkt, exp_e2e_pkt=exp_q_pkt)
        print "Passed for postcard egress + QDoD + new DSCP"


        self.client.switch_api_dtel_drop_watchlist_entry_delete(
          device=device, twl_kvp=twl_kvp)
        self.mod_watchlist=False
        exp_dod_pkt[IP].tos = 6<<2
        dtel_checkDoD(self, swports[0], swports[1],
                           swports[params.report_ports[0]],
                           pkt_in, exp_pkt,
                           True, exp_dod_pkt,
                           exp_i2e_pkt=exp_i2e_pkt, exp_e2e_pkt=exp_q_pkt)
        print "Passed for postcard egress + QDoD + new DSCP"

        self.client.switch_api_dtel_queue_report_delete(
            device, swports[1], 0)
        self.queue_report_enabled = False
        self.client.switch_api_dtel_event_set_dscp(
            device, SWITCH_DTEL_EVENT_TYPE_DROP_REPORT, 0)
        self.client.switch_api_dtel_event_set_dscp(
            device, SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP, 0)
        exp_dod_pkt[IP].tos = 0
        exp_dod_pkt[DTEL_REPORT_HDR].congested_queue = 0

    def runTest(self):
        print "Test Postcard device with dod"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        bind_postcard_pkt()
        bind_mirror_on_drop_pkt()
        config = SwitchConfig(self, params)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        self.client.switch_api_dtel_postcard_enable(device)

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
            udp_sport=101)

        exp_pkt = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_id=108,
            ip_ttl=63,
            pktlen=256,
            with_udp_chksum=False,
            udp_sport=101)

        exp_inte2e_inner_1 = postcard_report(
            packet=exp_pkt,
            switch_id=switch_id,
            ingress_port=swports[0],
            egress_port=swports[1],
            queue_id=0,
            queue_depth=0,
            egress_tstamp=0)

        exp_e2e_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
            dropped=0,
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=exp_inte2e_inner_1)

        exp_mod_inner_1 = mod_report(
            packet=pkt,
            switch_id=switch_id,
            ingress_port=swports[0],
            egress_port=swports[1],
            queue_id=0,
            drop_reason=71)

        exp_dod_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_MOD,
            dropped=1,
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            inner_frame=exp_mod_inner_1)

        self.queue_report_enabled = False
        self.mod_watchlist=False
        exp_q_pkt = exp_e2e_pkt.copy()
        exp_q_pkt[DTEL_REPORT_HDR].congested_queue = 1

        # add flow space to postcard watchlist
        ap = switcht_twl_postcard_params_t(
            report_all_packets=True, flow_sample_percent=100)
        self.client.switch_api_dtel_postcard_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # config MoD
        self.client.switch_api_dtel_drop_report_enable(device)

        try:
            self.run_test_(pkt, exp_pkt, None, exp_e2e_pkt,
                           exp_q_pkt, exp_dod_pkt)
        finally:
            ### Cleanup
            split_postcard_pkt()
            split_mirror_on_drop_pkt()
            if self.queue_report_enabled:
                self.client.switch_api_dtel_queue_report_delete(
                    device, swports[1], 0)
            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_DROP_REPORT, 0)
            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP, 0)
            self.client.switch_api_dtel_postcard_disable(device)
            self.client.switch_api_dtel_postcard_watchlist_clear(device)
            self.client.switch_api_dtel_drop_report_disable(device)
            if self.mod_watchlist:
              self.client.switch_api_dtel_drop_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            config.cleanup(self)

@group('postcard')
class Postcard_ReportSeq_Test(api_base_tests.ThriftInterfaceDataPlane,
                             pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test DTel Report sequence number"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        bind_postcard_pkt()
        config = SwitchConfig(self, params)

        ap = switcht_twl_postcard_params_t(
            report_all_packets=True, flow_sample_percent=100)
        self.client.switch_api_dtel_postcard_watchlist_entry_create(
            device, twl_kvp, priority=1, watch=True, action_params=ap)

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=quantization_shift)

        # make input frame to inject to sink
        pkt = simple_udp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_id=108,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=64,
            with_udp_chksum=False,
            udp_sport=101)

        initial_seq = 0xFFFFFFFD

        exp_pkt = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            with_udp_chksum=False,
            ip_id=108,
            ip_ttl=63,
            udp_sport=101)

        exp_inte2e_inner_1 = postcard_report(
            packet=exp_pkt,
            switch_id=params.switch_id,
            ingress_port=swports[0],
            egress_port=swports[1],
            queue_id=0,
            queue_depth=0,
            egress_tstamp=0)

        exp_e2e_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
            dropped=0,
            congested_queue=0,
            path_tracking_flow=1,
            hw_id=hw_id,
            sequence_number=initial_seq,
            inner_frame=exp_inte2e_inner_1)

        postcard_enabled = False

        try:
            initial_seq2=hex_to_i32(initial_seq)
            self.client.switch_api_dtel_report_sequence_number_set(
                device, params.mirror_ids[0], initial_seq2)
            seq_numbers = self.client.switch_api_dtel_report_sequence_number_get(
                device, params.mirror_ids[0], 4)
            for s in seq_numbers:
                self.assertTrue(s == initial_seq2,
                        "Could not get initial configure sequence number "
                                "%x vs %x" % (s,initial_seq2))
            # enable int-ep
            self.client.switch_api_dtel_postcard_enable(device)
            postcard_enabled = True
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_postcard_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]],
                ignore_seq_num=False)
            verify_no_other_packets(self)
            print "pass seq increment"

            seq_numbers = self.client.switch_api_dtel_report_sequence_number_get(
                device, params.mirror_ids[0], 4)
            for i in range(len(seq_numbers)):
                if i == hw_id:
                    self.assertTrue(seq_numbers[i] == (initial_seq2+1) ,
                        "Could not get updated sequence number "
                                    "%x vs %x"% (seq_numbers[i],
                                                 (initial_seq+1)))
                else:
                    self.assertTrue(seq_numbers[i] == initial_seq2,
                        "Could not get initial configure sequence number")
            exp_e2e_pkt[DTEL_REPORT_HDR].sequence_number = (initial_seq+1)& 0xFFFFFFFF
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            # e2e mirror will have qdepth value. Ignore qdepth and compare.
            verify_postcard_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]],
                ignore_seq_num=False)
            verify_no_other_packets(self)
            print "pass seq increment"

        finally:
            ### Cleanup
            split_postcard_pkt()
            if postcard_enabled:
                self.client.switch_api_dtel_postcard_disable(device)
            self.client.switch_api_dtel_postcard_watchlist_clear(device)
            config.cleanup(self)
