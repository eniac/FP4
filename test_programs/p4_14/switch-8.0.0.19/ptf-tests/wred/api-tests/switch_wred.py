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
Thrift API interface ACL tests
"""

import switchapi_thrift

import time
import sys
import logging

import unittest
import random
import pdb

import ptf.dataplane as dataplane

from ptf.testutils import *
from ptf.thriftutils import *

import os
import ptf.mask

from switchapi_thrift.ttypes import  *
from switchapi_thrift.switch_api_headers import  *

from erspan3 import *

this_dir = os.path.dirname(os.path.abspath(__file__))

sys.path.append(os.path.join(this_dir, '../../base'))
sys.path.append(os.path.join(this_dir, '../../base/api-tests'))
from common.utils import *
from common.api_utils import *
import api_base_tests

device = 0
cpu_port = 64
swports = [x for x in range(65)]

@group('wred')
@group('wred_drop')
class WredLookupTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "WRED lookup test in switchAPI."
        # The lookup is purely done in control plane (switchAPI), not using
        # dataplane tables."

        try:
            # Add a new wred profile
            wred_info = switcht_wred_info_t(enable=True,
                                            ecn_mark=True,
                                            min_threshold=0,
                                            max_threshold=100,
                                            max_probability=.5,
                                            time_constant=100)
            wred_handle = self.client.switch_api_wred_create(device, wred_info)
            info = self.client.switch_api_wred_get(device, wred_handle)
            self.assertTrue(info.max_threshold == 100)
            self.assertTrue(info.max_probability == .5)

            wred_info.max_threshold = 200
            status = self.client.switch_api_wred_update(
                device, wred_handle, wred_info)
            self.assertTrue(status == 0) # SUCCESS

            port = self.client.switch_api_port_id_to_handle_get(
                device, swports[0])

            queue_handles = self.client.switch_api_queues_get(device, port)
            status = self.client.switch_api_wred_attach(
                device, queue_handles[0], 1, wred_handle)
            self.assertTrue(status == 0) # SUCCESS

            status = self.client.switch_api_wred_detach(
                device, queue_handles[0], 1)
            self.assertTrue(status == 0) # SUCCESS

            status = self.client.switch_api_wred_delete(device, wred_handle)
            self.assertTrue(status == 0) # SUCCESS

            status = self.client.switch_api_wred_delete(device, wred_handle)
            self.assertTrue(status == 7) # ITEM NOT FOUND


        finally:
            pass


@group('wred')
class WredIpv4Test(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "WRED test"

        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)

        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)
        i_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='192.168.0.1',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(
            device, rif1, vrf, i_ip1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(
            device, rif2, vrf, i_ip2)

        # Add a static route
        i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.1',
            prefix_length=32)
        nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip4, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(device, vrf, i_ip4, nhop1)

        try:
            pkt1 = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)

            # ECN-capable transport
            pkt2 = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_tos=1,
                ip_id=105,
                ip_ttl=64)

            exp_pkt1 = simple_tcp_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=63)

            exp_pkt2 = simple_tcp_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_tos=3,
                ip_id=105,
                ip_ttl=63)

            exp_pkt3 = simple_tcp_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_tos=1,
                ip_id=105,
                ip_ttl=63)

            # Add a new wred profile to mark packets.
            wred_info = switcht_wred_info_t(enable=True,
                                            ecn_mark=True,
                                            min_threshold=0,
                                            max_threshold=50,
                                            max_probability=1,
                                            time_constant=100)
            wred_handle = self.client.switch_api_wred_create(device, wred_info)
            queue_handles = self.client.switch_api_queues_get(
                device, port2)

            # Attach the wred profile to the correct queue. Packet should get
            # marked.
            self.client.switch_api_wred_attach(
                device, queue_handles[0], 0, wred_handle)

            # ECT is set.
            send_packet(self, swports[0], str(pkt2))
            verify_packets(self, exp_pkt2, [swports[1]])

            # ECT is NOT set.
            send_packet(self, swports[0], str(pkt1))
            verify_packets(self, exp_pkt1, [swports[1]])

            # Detach the profile. Packet should NOT get marked.
            self.client.switch_api_wred_detach(
                device, queue_handles[0], 0)

            send_packet(self, swports[0], str(pkt2))
            verify_packets(self, exp_pkt3, [swports[1]])

        finally:
            self.client.switch_api_wred_delete(device, wred_handle)

            self.client.switch_api_neighbor_delete(device, neighbor1)

            self.client.switch_api_l3_route_delete(device, vrf, i_ip4, nhop1)

            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_l3_interface_address_delete(
                device, rif1, vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(
                device, rif2, vrf, i_ip2)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_router_mac_delete(
                device, rmac, '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)

@group('wred')
@group('wred_drop')
class WredStatsTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "WRED test"

        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)

        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)
        i_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='192.168.0.1',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(
            device, rif1, vrf, i_ip1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(
            device, rif2, vrf, i_ip2)

        # Add a static route
        i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.1',
            prefix_length=32)
        nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip4, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(device, vrf, i_ip4, nhop1)

        try:
            pkt1 = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)

            # ECN-capable transport
            pkt2 = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_tos=2,
                ip_id=105,
                ip_ttl=64)

            exp_pkt1 = simple_tcp_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=63)

            exp_pkt2 = simple_tcp_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_tos=3,
                ip_id=105,
                ip_ttl=63)

            # Add a new wred profile to mark packets.
            wred_info = switcht_wred_info_t(enable=True,
                                            ecn_mark=True,
                                            min_threshold=0,
                                            max_threshold=50,
                                            max_probability=1,
                                            time_constant=100)
            wred_handle = self.client.switch_api_wred_create(device, wred_info)
            queue_handles = self.client.switch_api_queues_get(
                device, port2)

            # Attach the wred profile to the correct queue. Packet should get
            # marked.
            self.client.switch_api_wred_attach(
                device, queue_handles[0], 0, wred_handle)

            # Clear WRED stats
            wred_stats = self.client.switch_api_wred_stats_clear(
                device, queue_handles[0], [SWITCH_WRED_STATS_GREEN_ECN_MARKED])

            # ECT is set.
            num_packets = 5
            for i in range(num_packets):
                send_packet(self, swports[0], str(pkt2))
                verify_packets(self, exp_pkt2, [swports[1]])

            # ECT is NOT set.
            for i in range(num_packets):
                send_packet(self, swports[0], str(pkt1))

            # Get WRED stats
            wred_stats = self.client.switch_api_wred_stats_get(
                device, queue_handles[0], [SWITCH_WRED_STATS_GREEN_ECN_MARKED])
            print "Number of marked packets:"
            print "- Green: %d, (expected %d)" % (wred_stats[0].num_packets, num_packets)
            self.assertTrue(wred_stats[0].num_packets == 5)
            wred_stats = self.client.switch_api_wred_stats_get(
                device, queue_handles[0], [SWITCH_WRED_STATS_RED_ECN_MARKED])
            print "- Red: %d, (expected %d)" % (wred_stats[0].num_packets, 0)
            self.assertTrue(wred_stats[0].num_packets == 0)
            wred_stats = self.client.switch_api_wred_stats_get(
                device, queue_handles[7], [SWITCH_WRED_STATS_RED_ECN_MARKED])
            print "- Red: %d, (expected %d)" % (wred_stats[0].num_packets, 0)
            self.assertTrue(wred_stats[0].num_packets == 0)



            # Get WRED stats
            wred_stats = self.client.switch_api_wred_stats_clear(
                device, queue_handles[0], [SWITCH_WRED_STATS_GREEN_ECN_MARKED])
            wred_stats = self.client.switch_api_wred_stats_get(
                device, queue_handles[0], [SWITCH_WRED_STATS_GREEN_ECN_MARKED])
            self.assertTrue(wred_stats[0].num_packets == 0)


        finally:
            self.client.switch_api_wred_delete(device, wred_handle)

            self.client.switch_api_neighbor_delete(device, neighbor1)

            self.client.switch_api_l3_route_delete(device, vrf, i_ip4, nhop1)

            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_l3_interface_address_delete(
                device, rif1, vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(
                device, rif2, vrf, i_ip2)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_router_mac_delete(
                device, rmac, '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


@group('wred_drop')
class WredDropIpv4Test(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "WRED test"

        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)

        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[2])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)
        i_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='192.168.0.1',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(
            device, rif1, vrf, i_ip1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(
            device, rif2, vrf, i_ip2)

        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif3 = self.client.switch_api_rif_create(0, rif_info3)
        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.3',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(
            device, rif3, vrf, i_ip3)

        # Add a static route
        i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.1',
            prefix_length=32)
        nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip4, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(device, vrf, i_ip4, nhop1)

        i_ip5 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.2',
            prefix_length=32)
        nhop2, neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, rif3, i_ip5, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(device, vrf, i_ip5, nhop2)

        try:
            pkt1 = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=64)

            pkt2 = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.2',
                ip_src='192.168.0.1',
                ip_tos=1,
                ip_id=105,
                ip_ttl=64)

            # ECN-capable transport
            pkt3 = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_tos=1,
                ip_id=105,
                ip_ttl=64)

            # send the test packet(s)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=63)
            # Add a new wred profile to drop a packet.
            wred_info = switcht_wred_info_t(enable=True,
                                            ecn_mark=False,
                                            min_threshold=0,
                                            max_threshold=50,
                                            max_probability=1,
                                            time_constant=100)
            wred_handle = self.client.switch_api_wred_create(device, wred_info)
            queue_handles = self.client.switch_api_queues_get(
                device, port2)
            # Attach the wred profile to a wrong queue. Packet should NOT get
            # dropped.
            self.client.switch_api_wred_attach(
                device, queue_handles[1], 0, wred_handle)

            send_packet(self, swports[0], str(pkt1))
            verify_packets(self, exp_pkt, [swports[1]])

            # Attach the wred profile to the correct queue. Packet should get
            # dropped.
            self.client.switch_api_wred_attach(
                device, queue_handles[0], 0, wred_handle)

            send_packet(self, swports[0], str(pkt1))
            verify_no_other_packets(self, timeout=1)

            # Detach the profile. Packet should NOT get dropped.
            self.client.switch_api_wred_detach(
                device, queue_handles[0], 0)

            send_packet(self, swports[0], str(pkt1))
            verify_packets(self, exp_pkt, [swports[1]])

            # Turn on ECN marking.
            wred_info.ecn_mark = True
            self.client.switch_api_wred_update(device, wred_handle, wred_info)
            self.client.switch_api_wred_attach(
                device, queue_handles[0], 0, wred_handle)

            # Send the test packet to port 1. Should get marked.
            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_tos=3,
                ip_id=105,
                ip_ttl=63)
            # ECT is NOT set.
            send_packet(self, swports[0], str(pkt1))
            verify_no_other_packets(self, timeout=1)

            # ECT is set.
            send_packet(self, swports[0], str(pkt3))
            verify_packets(self, exp_pkt, [swports[1]])

            # Send a packet to different port. Should NOT get marked.
            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.2',
                ip_src='192.168.0.1',
                ip_tos=1,
                ip_id=105,
                ip_ttl=63)
            send_packet(self, swports[0], str(pkt2))
            verify_packets(self, exp_pkt, [swports[2]])

            # Delete the profile
            self.client.switch_api_wred_delete(device, wred_handle)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_ttl=63)
            send_packet(self, swports[0], str(pkt1))
            verify_packets(self, exp_pkt, [swports[1]])

        finally:


            self.client.switch_api_neighbor_delete(device, neighbor1)
            self.client.switch_api_neighbor_delete(device, neighbor2)

            self.client.switch_api_l3_route_delete(device, vrf, i_ip4, nhop1)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip5, nhop2)

            self.client.switch_api_nhop_delete(device, nhop1)
            self.client.switch_api_nhop_delete(device, nhop2)

            self.client.switch_api_l3_interface_address_delete(
                device, rif1, vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(
                device, rif2, vrf, i_ip2)
            self.client.switch_api_l3_interface_address_delete(
                device, rif3, vrf, i_ip3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)
            self.client.switch_api_rif_delete(0, rif3)

            self.client.switch_api_router_mac_delete(
                device, rmac, '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


@group('wred_drop')
class WredIpv6Test(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "WRED test"

        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)

        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            v6_unicast_enabled=True,
            rmac_handle=rmac)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)
        i_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='2000::2',
            prefix_length=120)
        self.client.switch_api_l3_interface_address_add(
            device, rif1, vrf, i_ip1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            v6_unicast_enabled=True,
            rmac_handle=rmac)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='3000::2',
            prefix_length=120)
        self.client.switch_api_l3_interface_address_add(
            device, rif2, vrf, i_ip2)

        # Add a static route
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='4000::2',
            prefix_length=128)
        nhop, neighbor = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip3, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(device, vrf, i_ip3, nhop)


        pkt = simple_tcpv6_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:22:22:22:22:22',
            ipv6_dst='4000::2',
            ipv6_src='2000::1',
            ipv6_ecn=1,
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst='00:11:22:33:44:55',
            eth_src='00:77:66:55:44:33',
            ipv6_dst='4000::2',
            ipv6_src='2000::1',
            ipv6_ecn=3,
            ipv6_hlim=63)


        try:
            # Add a new wred profile to drop a packet.
            wred_info = switcht_wred_info_t(enable=True,
                                            ecn_mark=True,
                                            min_threshold=0,
                                            max_threshold=50,
                                            max_probability=1,
                                            time_constant=100)
            wred_handle = self.client.switch_api_wred_create(device, wred_info)
            queue_handles = self.client.switch_api_queues_get(
                device, port2)

            self.client.switch_api_wred_attach(
                device, queue_handles[0], 0, wred_handle)

            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

        finally:
            self.client.switch_api_wred_delete(device, wred_handle)

            self.client.switch_api_neighbor_delete(device, neighbor)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip3, nhop)
            self.client.switch_api_nhop_delete(device, nhop)

            self.client.switch_api_l3_interface_address_delete(
                device, rif1, vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(
                device, rif2, vrf, i_ip2)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_router_mac_delete(
                device, rmac, '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)
