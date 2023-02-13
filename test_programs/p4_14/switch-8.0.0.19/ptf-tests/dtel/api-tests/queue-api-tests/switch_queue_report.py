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
Common tests for queue report
"""

import switchapi_thrift

import time
import sys
import logging

import unittest
import random

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *

import os

from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

import pdb

import pd_base_tests

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../../base'))
from common.utils import *
sys.path.append(os.path.join(this_dir, '../../../base/api-tests'))
import api_base_tests

sys.path.append(os.path.join(this_dir, '../..'))
from dtel_utils import *

device = 0
SID = 0x0ACEFACE
high_quantization_shift = MAX_QUANTIZATION
low_quantization_shift = 0

swports = [0, 1, 2]

params = SwitchConfig_Params()
params.switch_id = SID
params.mac_self = '00:77:66:55:44:33'
params.nports = 3
params.ipaddr_inf = ['2.2.0.1',  '1.1.0.1', '10.1.0.4']
params.ipaddr_nbr = ['2.2.0.200', '1.1.0.100', '10.1.0.1']
params.mac_nbr = ['00:11:22:33:44:54', '00:11:22:33:44:55', '00:11:22:33:44:56']
params.report_ports = [2]
params.ipaddr_report_src = ['4.4.4.1']
params.ipaddr_report_dst = ['4.4.4.3']
params.mirror_ids = [555]
params.device = device
params.swports = swports

if test_param_get('target') == "asic-model":
    reset_cycle = 6
    min_sleeptime = 75
elif test_param_get('target') == "bmv2":
    reset_cycle = 1
    min_sleeptime = 5
else:
    reset_cycle = 1
    min_sleeptime = 1

DEBUG = False
#DEBUG = True

twl_kvp = []
kvp_val = switcht_twl_value_t(
    value_num=ipv4Addr_to_i32(params.ipaddr_nbr[0]))
kvp_mask = switcht_twl_value_t(value_num=0xffffff00)
twl_kvp.append(switcht_twl_key_value_pair_t(
    SWITCH_TWL_FIELD_IPV4_SRC, kvp_val, kvp_mask))
kvp_val = switcht_twl_value_t(
    value_num=ipv4Addr_to_i32(params.ipaddr_nbr[1]))
kvp_mask = switcht_twl_value_t(value_num=0xffffff00)
twl_kvp.append(switcht_twl_key_value_pair_t(
    SWITCH_TWL_FIELD_IPV4_DST, kvp_val, kvp_mask))

###############################################################################

@group('ep_l45')
@group('transit_l45')
@group('postcard')
class QueueReport_Quota_Test(api_base_tests.ThriftInterfaceDataPlane,
                                  pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test queue report Quota"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        config = SwitchConfig(self, params)
        bind_postcard_pkt() # jsut to parse report

        payload = 'qreport'
        # make input frame to inject to sink
        pkt = simple_udp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_id=108,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=64,
            udp_sport=101,
            with_udp_chksum=False,
            pktlen=256,
            udp_payload=payload)

        exp_pkt = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
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
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
            dropped=0,
            congested_queue=1,
            path_tracking_flow=0,
            hw_id=hw_id,
            inner_frame=exp_e2e_inner_1)

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
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_MOD,
            dropped=1,
            congested_queue=1,
            path_tracking_flow=0,
            hw_id=hw_id,
            inner_frame=exp_mod_inner_1)

        queue_report_enabled = False
        mod_enabled = False
        acl_enabled = False
        try:

            # quota cannot be 0
            self.client.switch_api_dtel_queue_report_create(
                device, swports[1], 0, 0, 0, 0, False)
            queue_report_enabled = True
            # verify that it is not programmed!
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            queue_report_enabled = False
            print "Passed quota cannot be 0"

            # default quota cannot be 0: tested as quota!=0 when qreport=false
            # and we didn't get a report

            # don't generate report if queue<threshold even if quota is there
            # if queue < threshold, remaining quota = remaining quota
            self.client.switch_api_dtel_queue_report_create(
                device, swports[1], 0, hex_to_i32(0xfff),
                hex_to_i32(0xffffffff), 1024, False)
            queue_report_enabled = True
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)

            quota = self.client.switch_api_dtel_queue_remaining_report_quota_during_breach_get(
                device, swports[1], 0);
            self.assertTrue(quota == 1024, "Remaining quota is not correct")
            print "Passed no report if queue < threshold"
            print "Passed remaining quota doesn't change if no report"

            # make sure the latency higher bits are zeroed
            self.client.switch_api_dtel_queue_report_update(
                device, swports[1], 0, hex_to_i32(0xfff),
                hex_to_i32(0x000fffff), 1024, False)
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "Passed no report if latency < threshold"

            # if queue >= threshold, generate report if remaining quota > 0.
            # remaining quota should be updated

            self.client.switch_api_dtel_queue_report_update(
                device, swports[1], 0, 0, 0, 1024, False)

            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])

            # verify e2e mirrored packet
            verify_postcard_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)

            quota = self.client.switch_api_dtel_queue_remaining_report_quota_during_breach_get(
                device, swports[1], 0);
            self.assertTrue(quota == 1023, "Remaining quota is not decremented"
                            " (%d)" % quota)
            print "Passed remaining quota decrements per report"

            self.client.switch_api_dtel_report_udp_dstport_set(
                device, UDP_PORT_DTEL_REPORT^0x1111);
            bind_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT^0x1111)
            split_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT)
            exp_e2e_pkt[UDP].dport = UDP_PORT_DTEL_REPORT^0x1111
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])

            # verify e2e mirrored packet
            verify_postcard_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            split_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT^0x1111)
            bind_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT)
            self.client.switch_api_dtel_report_udp_dstport_set(
                device, UDP_PORT_DTEL_REPORT);
            exp_e2e_pkt[UDP].dport = UDP_PORT_DTEL_REPORT
            print "Passed queue report + Report UDP port"

            # if queue >= threshold, generate report if remaining quota > 0.
            # remaining quota should be updated, even if big change
            # no harm to run in model
            # set quantization shift
            self.client.switch_api_dtel_latency_quantization_shift(
                device=device, quant_shift=low_quantization_shift)
            self.client.switch_api_dtel_queue_report_update(
                device, swports[1], 0, 0, 0, 1024, False)
            num = 10
            for i in range(num):
                send_packet(self, swports[0], str(pkt))
                verify_packet(self, exp_pkt, swports[1])

                # verify e2e mirrored packet
                verify_postcard_packet(
                    self, exp_e2e_pkt, swports[params.report_ports[0]])
            quota = self.client.switch_api_dtel_queue_remaining_report_quota_during_breach_get(
                device, swports[1], 0);
            self.assertTrue(quota == (1024-num), "Remaining quota is not decremented"
                            " (%d)" % quota)
            # set quantization shift
            self.client.switch_api_dtel_latency_quantization_shift(
                device=device, quant_shift=high_quantization_shift)
            print "Passed remaining quota decrements per report with change"

            # if queue >= threshold, don't generate report if remaining quota==0
            self.client.switch_api_dtel_queue_report_update(
                device, swports[1], 0, 0, 0, 1, False)
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_postcard_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)

            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "Passed don't generate report if quota = 0"

            if test_param_get('target') != 'asic-model':
                # (HARDWARE) if queue >= threshold and quota is finished, and now queue <
                # threshold for the first time, genereate 1 report (even if it is
                # not a big change)
                # software queue size is == 0
                self.client.switch_api_dtel_latency_quantization_shift(
                    device=device, quant_shift=high_quantization_shift)
                # send large packet first.
                # send small packet later it will be smaller than threshold!
                small_pkt = simple_udp_packet(
                    eth_dst=params.mac_self,
                    eth_src=params.mac_nbr[0],
                    ip_id=108,
                    ip_dst=params.ipaddr_nbr[1],
                    ip_src=params.ipaddr_nbr[0],
                    ip_ttl=64,
                    udp_sport=101,
                    with_udp_chksum=False,
                    pktlen=64,
                    udp_payload=payload)
                exp_small_pkt = simple_udp_packet(
                    eth_dst=params.mac_nbr[1],
                    eth_src=params.mac_self,
                    ip_dst=params.ipaddr_nbr[1],
                    ip_src=params.ipaddr_nbr[0],
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
                    eth_dst=params.mac_nbr[params.report_ports[0]],
                    eth_src=params.mac_self,
                    ip_src=params.ipaddr_report_src[0],
                    ip_dst=params.ipaddr_report_dst[0],
                    ip_id=0,
                    ip_ttl=64,
                    next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
                    dropped=0,
                    congested_queue=1,
                    path_tracking_flow=0,
                    hw_id=hw_id,
                    inner_frame=exp_small_e2e_inner_1)

                # each cell is 80B, so 10 should be at > one packet
                q_threshold = len(str(pkt))/80
                self.client.switch_api_dtel_queue_report_update(
                    device, swports[1], 0, q_threshold, hex_to_i32(0xffffffff), 1, False)

                # send large packet
                # quota of 1 should be finished here
                send_packet(self, swports[0], str(pkt))
                verify_packet(self, exp_pkt, swports[1])
                verify_postcard_packet(
                    self, exp_e2e_pkt, swports[params.report_ports[0]])

                # small packet should be below the threshold but still report
                # as quota is finsihed
                send_packet(self, swports[0], str(small_pkt))
                verify_packet(self, exp_small_pkt, swports[1])
                verify_postcard_packet(
                    self, exp_small_e2e_pkt, swports[params.report_ports[0]])

                print "Passed received packet below threshold when quota is finished"

            # if queue >= threshold and we drop packet at egress, we still
            # update the quota and generate report if MoD is disabled/enabled
            acl = self.client.switch_api_acl_list_create(
                device, SWITCH_API_DIRECTION_EGRESS, SWITCH_ACL_TYPE_EGRESS_SYSTEM,
                SWITCH_HANDLE_TYPE_PORT)
            # create kvp to match egress port and deflect bit
            kvp = []
            port = self.client.switch_api_port_id_to_handle_get(device, swports[1])
            kvp_val = switcht_acl_value_t(value_num=port)
            kvp_mask = switcht_acl_value_t(value_num=0xff)
            kvp.append(
                switcht_acl_key_value_pair_t(SWITCH_ACL_EGRESS_SYSTEM_FIELD_DEST_PORT,
                                             kvp_val, kvp_mask))
            action = SWITCH_ACL_EGRESS_SYSTEM_ACTION_DROP
            action_params = switcht_acl_action_params_t(
                drop=switcht_acl_action_drop(reason_code=92))  # egress acl deny
            opt_action_params = switcht_acl_opt_action_params_t()
            ace = self.client.switch_api_acl_egress_system_rule_create(
                device, acl, 11, 1, kvp, action, action_params, opt_action_params)
            self.client.switch_api_acl_reference(device, acl, port)
            acl_enabled = True
            self.client.switch_api_dtel_queue_report_update(
                device, swports[1], 0, 0, 0, 1024, False)

            send_packet(self, swports[0], str(pkt))

            # verify e2e mirrored packet
            verify_postcard_packet(
                self, exp_e2e_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            quota = self.client.switch_api_dtel_queue_remaining_report_quota_during_breach_get(
                device, swports[1], 0);
            self.assertTrue(quota == 1023, "Remaining quota is not decremented")
            print "Passed update quota even if original packet dropped"

            # now test with MoD
            # Add MoD watchlist
            ap = switcht_twl_mod_params_t(report_queue_tail_drops=False)
            self.client.switch_api_dtel_drop_watchlist_entry_create(
                device, twl_kvp, priority=1, watch=True, action_params=ap)
            self.client.switch_api_dtel_drop_report_enable(device)
            mod_enabled = True

            self.client.switch_api_dtel_queue_report_update(
                device, swports[1], 0, 0, 0, 1024, False)

            exp_e2e_pkt[DTEL_REPORT_HDR].dropped = 1
            send_packet(self, swports[0], str(pkt))
            # verify e2e mirrored packet
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            quota = self.client.switch_api_dtel_queue_remaining_report_quota_during_breach_get(
                device, swports[1], 0);
            self.assertTrue(quota == 1023, "Remaining quota is not decremented")
            print "Passed update quota even if original packet dropped+mod"

            # qbit is not set for MoD packets when quota is finished even though
            # queue>=threshold
            self.client.switch_api_dtel_queue_report_update(
                device, swports[1], 0, 0, 0, 1, False)
            send_packet(self, swports[0], str(pkt))
            # verify e2e mirrored packet
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)

            # quota finished now
            exp_mod_pkt[DTEL_REPORT_HDR].congested_queue = 0
            send_packet(self, swports[0], str(pkt))
            # verify e2e mirrored packet
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            exp_e2e_pkt[DTEL_REPORT_HDR].congested_queue = 1
            exp_e2e_pkt[DTEL_REPORT_HDR].dropped = 0

            self.client.switch_api_acl_dereference(device, acl, port)
            self.client.switch_api_acl_rule_delete(device, acl, ace)
            self.client.switch_api_acl_list_delete(device, acl)
            acl_enabled = False
            self.client.switch_api_dtel_drop_report_disable(device)
            self.client.switch_api_dtel_drop_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            mod_enabled = False


            # disable queue report
            self.client.switch_api_dtel_queue_report_delete(
                device, swports[1], 0)
            queue_report_enabled = False
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "Passed disable queue report"

        finally:
            print "Test Cleanup"
            split_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT^0x1111)
            bind_layers(UDP, DTEL_REPORT_HDR,
                        dport=UDP_PORT_DTEL_REPORT)
            self.client.switch_api_dtel_report_udp_dstport_set(
                device, UDP_PORT_DTEL_REPORT);
            split_mirror_on_drop_pkt()
            split_postcard_pkt()
            if queue_report_enabled:
                self.client.switch_api_dtel_queue_report_delete(
                    device, swports[1], 0)
            if acl_enabled:
                self.client.switch_api_acl_dereference(device, acl, port)
                self.client.switch_api_acl_rule_delete(device, acl, ace)
                self.client.switch_api_acl_list_delete(device, acl)
            if mod_enabled:
                self.client.switch_api_dtel_drop_report_disable(device)
                self.client.switch_api_dtel_drop_watchlist_entry_delete(
                    device=device, twl_kvp=twl_kvp)
            config.cleanup(self)

@group('ep_l45_dod')
@group('transit_l45_dod')
@group('postcard_dod')
# make sure at the end of the test case dod counter is 0
class QueueReport_DoD_Test(api_base_tests.ThriftInterfaceDataPlane,
                                  pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])
    def runTest(self):
        print "Test queue report Quota"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        config = SwitchConfig(self, params)
        bind_postcard_pkt() # jsut to parse report
        bind_mirror_on_drop_pkt()

        pkt_in = simple_tcp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_id=105,
            ip_ttl=64,
            pktlen=256)

        exp_pkt_out = simple_tcp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_id=105,
            ip_ttl=63,
            pktlen=256)

        input_port=swports[0]

        exp_dod_inner = mod_report(
            packet=pkt_in,
            switch_id=SID,
            ingress_port=swports[0],
            egress_port=swports[1],
            queue_id=0,
            drop_reason=71)  # drop traffic manager

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
            path_tracking_flow=0,
            hw_id=hw_id,
            inner_frame=exp_dod_inner)

        exp_e2e_inner_1 = postcard_report(
            packet=exp_pkt_out,
            switch_id=SID,
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
            congested_queue=1,
            path_tracking_flow=0,
            hw_id=hw_id,
            inner_frame=exp_e2e_inner_1)

        queue_report_enabled=False
        mod_enabled=False
        # ignore changes for this test
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=high_quantization_shift)
        try:
            # enable dod
            # Don't generate DoD on queue we don't monitor and is not in MoD
            # watchlist
            dtel_checkDoD(self, swports[0], swports[1], swports[params.report_ports[0]], pkt_in, exp_pkt_out, False)
            print "Pass no DoD if no queue report or MoD"

            # enable mod
            self.client.switch_api_dtel_drop_report_enable(device)
            dtel_checkDoD(self, swports[0], swports[1], swports[params.report_ports[0]], pkt_in, exp_pkt_out, False)
            print "Pass no DoD if no queue report or MoD"

            # add entry without dod
            ap = switcht_twl_mod_params_t(report_queue_tail_drops=False)
            self.client.switch_api_dtel_drop_watchlist_entry_create(
                device, twl_kvp, priority=1, watch=True, action_params=ap)
            mod_enabled=True
            dtel_checkDoD(self, swports[0], swports[1], swports[params.report_ports[0]], pkt_in, exp_pkt_out, False)
            print "Pass no DoD if no queue report and Dod=false in MoD watchlist"

            # add entry with dod
            ap = switcht_twl_mod_params_t(report_queue_tail_drops=True)
            self.client.switch_api_dtel_drop_watchlist_entry_update(
                device, twl_kvp, priority=1, watch=True, action_params=ap)
            dtel_checkDoD(self, swports[0], swports[1], swports[params.report_ports[0]], pkt_in, exp_pkt_out, True, exp_dod_pkt)
            print "Pass get DoD if no queue report and Dod in MoD watchlist"

            if test_param_get('target') == 'asic-model':
              self.client.switch_api_dtel_queue_report_create(
                device, swports[1], 0, hex_to_i32(0xfff),
                hex_to_i32(0xffffffff), 1024, True);
              queue_report_enabled=True
              exp_dod_pkt[DTEL_REPORT_HDR].congested_queue = 1
              dtel_checkDoD(self, swports[0], swports[1], swports[params.report_ports[0]], pkt_in,exp_pkt_out, True, exp_dod_pkt)
              print "Pass get DoD if queue report and Dod in MoD watchlist"
              quota = self.client.switch_api_dtel_queue_remaining_report_quota_during_breach_get(
                  device, swports[1], 0);
              self.assertTrue(quota == 1023, "Remaining quota is not correct")

              ap = switcht_twl_mod_params_t(report_queue_tail_drops=False)
              self.client.switch_api_dtel_drop_watchlist_entry_update(
                device, twl_kvp, priority=1, watch=True, action_params=ap)
              dtel_checkDoD(self, swports[0], swports[1], swports[params.report_ports[0]], pkt_in,exp_pkt_out, True, exp_dod_pkt)
              print "Pass get DoD if queue report and Dod=false in MoD watchlist"

              self.client.switch_api_dtel_event_set_dscp(
                    device, SWITCH_DTEL_EVENT_TYPE_DROP_REPORT, 5)
              dtel_checkDoD(self, swports[0], swports[1], swports[params.report_ports[0]], pkt_in,exp_pkt_out, True, exp_dod_pkt)
              print "Pass change MoD DSCP"

              self.client.switch_api_dtel_event_set_dscp(
                    device,
                  SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP, 3)
              exp_dod_pkt[IP].tos = 3<<2
              dtel_checkDoD(self, swports[0], swports[1], swports[params.report_ports[0]], pkt_in,exp_pkt_out, True, exp_dod_pkt)
              print "Pass change DoD DSCP"

              exp_dod_pkt[IP].tos = 0
              self.client.switch_api_dtel_event_set_dscp(
                    device,
                  SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP, 0)
              self.client.switch_api_dtel_event_set_dscp(
                    device, SWITCH_DTEL_EVENT_TYPE_DROP_REPORT, 0)

              self.client.switch_api_dtel_queue_report_update(
                device, swports[1], 0, hex_to_i32(0xfff),
                hex_to_i32(0xffffffff), 1, True);
              dtel_checkDoD(self, swports[0], swports[1], swports[params.report_ports[0]], pkt_in,exp_pkt_out, True, exp_dod_pkt)
              send_packet(self, swports[0], str(pkt_in))
              verify_packet(self, exp_pkt_out, swports[1])
              verify_postcard_packet(self, exp_e2e_pkt,
                                        swports[params.report_ports[0]])
              verify_no_other_packets(self)
              print "Pass queue report if quota is finished after prvious DoD and DoD=false in MOD watchlist"

              self.client.switch_api_dtel_queue_report_update(
                device, swports[1], 0, 0, 0, 1, True);
              send_packet(self, swports[0], str(pkt_in))
              verify_packet(self, exp_pkt_out, swports[1])
              verify_postcard_packet(
                    self, exp_e2e_pkt, swports[params.report_ports[0]])
              verify_no_other_packets(self)
              dtel_checkDoD(self, swports[0], swports[1], swports[params.report_ports[0]], pkt_in,exp_pkt_out, True, None, 8)
              print "Pass no DoD if quota is finished and DoD=false in MOD watchlist"

              ap = switcht_twl_mod_params_t(report_queue_tail_drops=True)
              self.client.switch_api_dtel_drop_watchlist_entry_update(
                device, twl_kvp, priority=1, watch=True, action_params=ap)
              exp_dod_pkt[DTEL_REPORT_HDR].congested_queue = 0
              dtel_checkDoD(self, swports[0], swports[1], swports[params.report_ports[0]], pkt_in,exp_pkt_out, True, exp_dod_pkt)
              print "Pass get DoD if quota is finished and DoD in MOD watchlist"

              self.client.switch_api_dtel_event_set_dscp(
                    device,
                  SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP, 3)
              dtel_checkDoD(self, swports[0], swports[1], swports[params.report_ports[0]], pkt_in,exp_pkt_out, True, exp_dod_pkt)
              print "Pass change DoD DSCP"

              self.client.switch_api_dtel_event_set_dscp(
                    device, SWITCH_DTEL_EVENT_TYPE_DROP_REPORT, 5)
              exp_dod_pkt[IP].tos = 5<<2
              dtel_checkDoD(self, swports[0], swports[1], swports[params.report_ports[0]], pkt_in,exp_pkt_out, True, exp_dod_pkt)
              print "Pass change MoD DSCP"
              exp_dod_pkt[IP].tos = 0
              self.client.switch_api_dtel_event_set_dscp(
                    device,
                  SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP, 0)
              self.client.switch_api_dtel_event_set_dscp(
                    device, SWITCH_DTEL_EVENT_TYPE_DROP_REPORT, 0)

              self.client.switch_api_dtel_queue_report_update(
                device, swports[1], 0, hex_to_i32(0xfff),
                hex_to_i32(0xffffffff), 1, False);
              dtel_checkDoD(self, swports[0], swports[1], swports[params.report_ports[0]], pkt_in,exp_pkt_out, True, exp_dod_pkt)
              quota = self.client.switch_api_dtel_queue_remaining_report_quota_during_breach_get(
                  device, swports[1], 0);
              self.assertTrue(quota == 1, "Remaining quota is not correct")
              print "Pass get DoD if DoD=false in qreport and DoD in MOD watchlist"

              mod_enabled=False
              self.client.switch_api_dtel_drop_watchlist_entry_delete(
                    device=device, twl_kvp=twl_kvp)
              dtel_checkDoD(self, swports[0], swports[1], swports[params.report_ports[0]], pkt_in,exp_pkt_out, False)
              print "Pass no DoD if DoD=false in qreport and DoD=false in MOD watchlist"

              self.client.switch_api_dtel_queue_report_update(
                device, swports[1], 0, hex_to_i32(0xfff),
                hex_to_i32(0xffffffff), 1024, True);
              exp_dod_pkt[DTEL_REPORT_HDR].congested_queue = 1
              dtel_checkDoD(self, swports[0], swports[1], swports[params.report_ports[0]], pkt_in,exp_pkt_out, True, exp_dod_pkt)
              quota = self.client.switch_api_dtel_queue_remaining_report_quota_during_breach_get(
                  device, swports[1], 0);
              self.assertTrue(quota == 1023, "Remaining quota is not correct")
              print "Pass get DoD if renable DoD in qreport and no MOD watchlist"

              self.client.switch_api_dtel_queue_report_delete(
                    device, swports[1], 0)
              queue_report_enabled=False
              dtel_checkDoD(self, swports[0], swports[1], swports[params.report_ports[0]], pkt_in,exp_pkt_out, False)
              print "Pass no DoD if remove queue report and no mod in MOD watchlist"

        finally:
            print "Test Cleanup"
            split_mirror_on_drop_pkt()
            split_postcard_pkt()
            if queue_report_enabled:
                self.client.switch_api_dtel_queue_report_delete(
                    device, swports[1], 0)
            self.client.switch_api_dtel_drop_report_disable(device)
            self.client.switch_api_dtel_event_set_dscp(
                device,
                SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP, 0)
            self.client.switch_api_dtel_event_set_dscp(
                device, SWITCH_DTEL_EVENT_TYPE_DROP_REPORT, 0)
            if mod_enabled:
                self.client.switch_api_dtel_drop_watchlist_entry_delete(
                    device=device, twl_kvp=twl_kvp)
            config.cleanup(self)

@group('ep_l45')
@group('transit_l45')
@group('postcard')
class QueueReport_Change_Test(api_base_tests.ThriftInterfaceDataPlane,
                                  pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print "Test queue report Change"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(
            swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        config = SwitchConfig(self, params)
        bind_postcard_pkt() # jsut to parse report

        payload = 'int l45'
        # make input frame to inject to sink
        pkt = simple_udp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_id=108,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=64,
            udp_sport=101,
            with_udp_chksum=False,
            pktlen=256,
            udp_payload=payload)

        exp_pkt = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
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
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
            dropped=0,
            congested_queue=1,
            path_tracking_flow=0,
            hw_id=hw_id,
            inner_frame=exp_e2e_inner_1)

        exp_mod_inner_1 = mod_report(
            packet=exp_pkt,
            switch_id=SID,
            ingress_port=swports[0],
            egress_port=swports[1],
            queue_id=0,
            drop_reason=70)  # drop mtu check fail

        exp_mod_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_MOD,
            dropped=1,
            congested_queue=1,
            path_tracking_flow=0,
            hw_id=hw_id,
            inner_frame=exp_mod_inner_1)

        small_pkt = simple_udp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_id=108,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=64,
            udp_sport=101,
            with_udp_chksum=False,
            pktlen=64,
            udp_payload=payload)
        exp_small_pkt = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
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
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.ipaddr_report_src[0],
            ip_dst=params.ipaddr_report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_SWITCH_LOCAL,
            dropped=0,
            congested_queue=1,
            path_tracking_flow=0,
            hw_id=hw_id,
            inner_frame=exp_small_e2e_inner_1)

        queue_report_enabled = False
        try:
            #  Don't generate queue report if queue<threshold even if there is
            #  a change
            self.client.switch_api_dtel_latency_quantization_shift(
                device=device, quant_shift=low_quantization_shift)
            self.client.switch_api_dtel_queue_report_create(
                device, swports[1], 0, hex_to_i32(0xfff),
                hex_to_i32(0xffffffff), 1024, False)
            queue_report_enabled = True

            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "Passed below threshold change don't make a report"
            # Don't generate queue report if queue >= threshold but no change
            # and quota is finished (done in QueueReport_Quota_Test)

            if test_param_get('target') != 'asic-model':
                # Generate queue report if queue >= threshold and change if quota
                # is not finished and if finished
                self.client.switch_api_dtel_queue_report_update(
                    device, swports[1], 0, 0, 0, 2, False)
                self.client.switch_api_dtel_latency_quantization_shift(
                    device=device, quant_shift=low_quantization_shift)
                send_packet(self, swports[0], str(pkt))
                verify_packet(self, exp_pkt, swports[1])

                # verify e2e mirrored packet
                verify_postcard_packet(
                        self, exp_e2e_pkt, swports[params.report_ports[0]])
                verify_no_other_packets(self)

                send_packet(self, swports[0], str(small_pkt))
                verify_packet(self, exp_small_pkt, swports[1])

                # verify e2e mirrored packet
                verify_postcard_packet(
                        self, exp_small_e2e_pkt, swports[params.report_ports[0]])
                verify_no_other_packets(self)
                quota = self.client.switch_api_dtel_queue_remaining_report_quota_during_breach_get(
                    device, swports[1], 0);
                self.assertTrue(quota == 0, "Remaining quota is not correct")

                received=False
                for i in range(20):
                    # first two use quota
                    # later should just go because of change (small vs large cause
                    # chagne)
                    send_packet(self, swports[0], str(pkt))
                    verify_packet(self, exp_pkt, swports[1])

                    # verify e2e mirrored packet
                    (_, rcv_port, rcv_pkt, pkt_time) = \
                            self.dataplane.poll(port_number=swports[params.report_ports[0]], timeout=1, exp_pkt=None)
                    if rcv_pkt!=None:
                        print "received a change report after sending %d packets"%i 
                        received=True
                        break;
                self.assertTrue(received, "Didn't receive any report because of change!")

                print "Passed generate queue reports regardless of quota"

        finally:
            print "Test Cleanup"
            split_postcard_pkt()
            if queue_report_enabled:
                self.client.switch_api_dtel_queue_report_delete(
                    device, swports[1], 0)
            config.cleanup(self)

