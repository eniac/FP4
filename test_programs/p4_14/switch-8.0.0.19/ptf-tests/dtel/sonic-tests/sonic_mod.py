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
MoD SONiC tests
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
from ptf.testutils import *
from ptf.thriftutils import *
from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *
import redis
from copy import deepcopy

from switch_ptf_config import *

mac_all_ospf_routers = '01:00:5e:00:00:05'
ipaddr_all_ospf_routers = '224.0.0.5'

min_sleeptime = 3

# Whether to wait for user input before cleaning up in each test case
debug_mode = False

################################################################################
@group('mod')
@group('postcard')
@group('int_ep')
@group('int_transit')
class SONiC_IngressMoDTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print 'Ingress MoD test with malformed packet'
        bind_mirror_on_drop_pkt()

        # Value of dtel_monitoring_type is irrelevant since neither postcard
        # or INT will be enabled, and no 'flow' watchlist will be configured
        switch = sonic_switch.SONiCSwitch(dtel_monitoring_type='postcard',
                                          dtel_switch_id=switch_id,
                                          management_ip=switch_ip)

        switch.dtel_int_l4_dscp = {'value': get_int_l45_dscp_value(),
                                   'mask': get_int_l45_dscp_mask()}

        try:
            rs = switch.create_dtel_report_session(dst_ip_list=report_dst)

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
                ip_ttl=63,
                tcp_flags=None,
                pktlen=128)

            pkt_in_malformed = simple_tcp_packet(
                eth_dst=mac_self,
                eth_src=mac_all_zeros,
                ip_dst=ipaddr_nbr[1],
                ip_src=ipaddr_nbr[0],
                ip_id=105,
                ip_ttl=64,
                tcp_flags=None,
                pktlen=128)

            exp_mod_inner = mod_report(
                packet=pkt_in_malformed,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=INVALID_PORT_ID,
                queue_id=0,
                drop_reason=10)  # outer source mac all zeros

            exp_mod_pkt = ipv4_dtel_pkt(
                eth_dst=mac_nbr[report_ports[0]],
                eth_src=mac_self,
                ip_src=report_src,
                ip_dst=report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_MOD,
                dropped=1,
                congested_queue=0,
                path_tracking_flow=0,
                hw_id=get_pipeid(devports[report_ports[0]]),
                inner_frame=exp_mod_inner)

            time.sleep(min_sleeptime)

            print "Start sending packets"

            # send a test packet
            send_packet(self, swports[0], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[1])
            verify_no_other_packets(self)
            print "Normal packet passed"

            # send malformed packet
            send_packet(self, swports[0], str(pkt_in_malformed))
            # verify no packets out
            verify_no_other_packets(self)
            print "No report for malformed packet before adding watchlist"

            # Create drop watchlist
            drop_watchlist = switch.create_dtel_watchlist(watchlist_type='drop')
            drop_watchlist_entry = drop_watchlist.create_entry(
                priority=10,
                ether_type=0x800,
                src_ip=ipaddr_nbr[0],
                dst_ip=ipaddr_nbr[1])

            switch.dtel_drop_report_enable = True

            time.sleep(min_sleeptime)

            # send a test packet
            send_packet(self, swports[0], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[1])
            verify_no_other_packets(self)
            print "Normal packet passed after adding drop watchlist"

            # send malformed packet
            send_packet(self, swports[0], str(pkt_in_malformed))
            # verify mod packet
            verify_dtel_packet(
                self, exp_mod_pkt, swports[report_ports[0]])
            verify_no_other_packets(self)
            print "Received drop report for malformed packet"

            # send the same malformed packet again
            send_packet(self, swports[0], str(pkt_in_malformed))
            # verify mod packet
            verify_dtel_packet(
                self, exp_mod_pkt, swports[report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for identical malformed pkt from the same port"

            drop_watchlist_entry.delete()
            print "delete watchlist entry"
            time.sleep(min_sleeptime)

            # send a malformed packet
            send_packet(self, swports[0], str(pkt_in_malformed))
            verify_no_other_packets(self)
            print "No report after deleting watchlist"

        finally:
            if debug_mode:
                raw_input("press any key to cleanup...")
            switch.cleanup(purge=True)


@group('mod')
@group('postcard')
@group('int_ep')
@group('int_transit')
class SONiC_MoDNonDefaultRuleTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print 'MoD non default rule test'
        bind_mirror_on_drop_pkt()

        # Value of dtel_monitoring_type is irrelevant since neither postcard
        # or INT will be enabled, and no 'flow' watchlist will be configured
        switch = sonic_switch.SONiCSwitch(dtel_monitoring_type='postcard',
                                          dtel_switch_id=switch_id,
                                          management_ip=switch_ip)
        switch.dtel_int_l4_dscp = {'value': get_int_l45_dscp_value(),
                                   'mask': get_int_l45_dscp_mask()}

        try:
            rs = switch.create_dtel_report_session(dst_ip_list=report_dst)

            # Add COPP rule for OSPF
            r = redis.StrictRedis(host=switch_ip, port=6379, db=0)
            original_entry = r.hgetall('COPP_TABLE:trap.group.arp')
            entry = deepcopy(original_entry)
            entry['trap_action'] = 'drop'
            entry['trap_ids'] = 'ospf'

            # Set
            r.sadd('COPP_TABLE_KEY_SET', 'trap.group.ospf')
            r.hmset('COPP_TABLE:trap.group.ospf', entry)
            r.publish('COPP_TABLE_CHANNEL', 'G')

            print "Added trap group ospf to database"

            pkt_in = simple_ip_packet(
                eth_dst=mac_all_ospf_routers,
                eth_src=mac_nbr[0],
                ip_dst=ipaddr_all_ospf_routers,
                ip_src=ipaddr_nbr[0],
                ip_ttl=1,
                ip_proto=89)

            exp_mod_inner = mod_report(
                packet=pkt_in,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=INVALID_PORT_ID,
                queue_id=0,
                drop_reason=254)

            exp_mod_pkt = ipv4_dtel_pkt(
                eth_dst=mac_nbr[report_ports[0]],
                eth_src=mac_self,
                ip_src=report_src,
                ip_dst=report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_MOD,
                dropped=1,
                congested_queue=0,
                path_tracking_flow=0,
                hw_id=get_pipeid(devports[report_ports[0]]),
                inner_frame=exp_mod_inner)

            time.sleep(min_sleeptime)

            print "Start sending packets"

            # send a test packet
            send_packet(self, swports[0], str(pkt_in))
            verify_no_other_packets(self)
            print "Dropped OSPF packet"

            # Create drop watchlist for OSPF packets
            drop_watchlist = switch.create_dtel_watchlist(watchlist_type='drop')
            drop_watchlist_entry = drop_watchlist.create_entry(
                priority=10,
                ip_proto=89)

            switch.dtel_drop_report_enable = True

            time.sleep(min_sleeptime)

            # send a test packet
            send_packet(self, swports[0], str(pkt_in))
            # verify mod packet
            verify_dtel_packet(
                self, exp_mod_pkt, swports[report_ports[0]])
            verify_no_other_packets(self)
            print "Received drop report for OSPF packet"

            # send the same test packet again
            send_packet(self, swports[0], str(pkt_in))
            # verify mod packet
            verify_dtel_packet(
                self, exp_mod_pkt, swports[report_ports[0]])
            verify_no_other_packets(self)
            print "Passed for identical pkt from the same port"

            drop_watchlist_entry.delete()
            print "delete watchlist entry"
            time.sleep(min_sleeptime)

            # send a test packet
            send_packet(self, swports[0], str(pkt_in))
            verify_no_other_packets(self)
            print "Dropped OSPF packet with no report after deleting watchlist"

        finally:
            if debug_mode:
                raw_input("press any key to cleanup...")

            # Delete CoPP rule for OSPF
            r.sadd('COPP_TABLE_KEY_SET', 'trap.group.ospf')
            r.delete('COPP_TABLE:trap.group.ospf')
            r.publish('COPP_TABLE_CHANNEL', 'G')

            switch.cleanup(purge=True)
