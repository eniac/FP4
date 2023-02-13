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
Thrift API interface basic tests
"""

import switchapi_thrift
import time
import sys
import logging

import unittest
import random

import ptf.dataplane as dataplane

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *

import os
import ptf.mask

from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../api-tests'))
sys.path.append(os.path.join(this_dir, '..'))
import api_base_tests
from common.utils import *

device = 0
cpu_port = 64
swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)

if swports == []:
    swports = [x for x in range(65)]


###############################################################################
@group('mpls_udp')
class L2MplsUdpOuterIPV4InnerIPV4PopTest(
        api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print

        vrf = self.client.switch_api_vrf_create(device, 2)
        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, vrf_handle=vrf, rmac_handle=rmac)
        rif1 = self.client.switch_api_rif_create(0, rif_info)

        i_info1 = switcht_interface_info_t(
            type=SWITCH_INTERFACE_TYPE_PORT, handle=port1, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            type=SWITCH_INTERFACE_TYPE_ACCESS,
            handle=port2)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        lognet_info = switcht_logical_network_t()
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)
        udp = switcht_udp_t(src_port=1234, dst_port=6635)
        src_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.1', prefix_length=32)
        dst_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.3', prefix_length=32)
        udp_tcp = switcht_udp_tcp_t(udp=udp)
        mpls_tag1 = switcht_mpls_t(label=0xabcde, exp=0x5, ttl=0x30, bos=0)
        mpls_tag2 = switcht_mpls_t(label=0x54321, exp=0x2, ttl=0x40, bos=1)
        pop_tag = [mpls_tag1, mpls_tag2]
        tun_info = switcht_tunnel_info_t(
            tunnel_type=SWITCH_TUNNEL_TYPE_MPLS_UDP,
            mpls_type=SWITCH_MPLS_TYPE_EOMPLS,
            mpls_mode=SWITCH_MPLS_MODE_TERMINATE,
            pop_tag=pop_tag,
            vrf_handle=vrf,
            src_ip=src_ip,
            dst_ip=dst_ip,
            u=udp_tcp,
            egress_rif_handle=rif1)
        ift3 = self.client.switch_api_tunnel_interface_create(device, 0,
                                                              tun_info)
        self.client.switch_api_logical_network_member_add(device, ln1, if2)
        self.client.switch_api_logical_network_member_add(device, ln1, ift3)
        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:22:22:22:22:22', 2, if2)

        try:
            tag1 = {'label': 0xabcde, 'tc': 0x5, 'ttl': 0xAA, 's': 0x0}
            tag2 = {'label': 0x54321, 'tc': 0x2, 'ttl': 0xBB, 's': 0x1}
            mpls_tags = [tag1, tag2]
            pkt = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63)
            udp_sport = entropy_hash(pkt)
            mpls_pkt = mpls_udp_inner_packet(
                mpls_tags=mpls_tags, inner_frame=pkt)
            mpls_udp_pkt = simple_udp_packet(
                eth_src='00:44:44:44:44:44',
                eth_dst='00:77:66:55:44:33',
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                udp_sport=udp_sport,
                udp_dport=6635,
                with_udp_chksum=False,
                udp_payload=mpls_pkt)

            send_packet(self, swports[0], str(mpls_udp_pkt))
            verify_packets(self, pkt, [swports[1]])
        finally:
            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:22:22:22:22:22')
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if2)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift3)
            self.client.switch_api_logical_network_delete(device, ln1)

            self.client.switch_api_tunnel_interface_delete(device, ift3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('mpls_udp')
class L2MplsUdpOuterIPV4InnerIPV6PopTest(
        api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print

        vrf = self.client.switch_api_vrf_create(device, 2)
        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, vrf_handle=vrf, rmac_handle=rmac)
        rif1 = self.client.switch_api_rif_create(0, rif_info)

        i_info1 = switcht_interface_info_t(
            rif_handle=rif1, type=SWITCH_INTERFACE_TYPE_PORT, handle=port1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            type=SWITCH_INTERFACE_TYPE_ACCESS,
            handle=port2)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        lognet_info = switcht_logical_network_t()
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)
        udp = switcht_udp_t(src_port=1234, dst_port=6635)
        src_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.1', prefix_length=32)
        dst_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.3', prefix_length=32)
        udp_tcp = switcht_udp_tcp_t(udp=udp)

        mpls_tag1 = switcht_mpls_t(label=0xabcde, exp=0x5, ttl=0x30, bos=0)
        mpls_tag2 = switcht_mpls_t(label=0x54321, exp=0x2, ttl=0x40, bos=1)
        pop_tag = [mpls_tag1, mpls_tag2]
        tun_info = switcht_tunnel_info_t(
            tunnel_type=SWITCH_TUNNEL_TYPE_MPLS_UDP,
            mpls_type=SWITCH_MPLS_TYPE_EOMPLS,
            mpls_mode=SWITCH_MPLS_MODE_TERMINATE,
            pop_tag=pop_tag,
            vrf_handle=vrf,
            src_ip=src_ip,
            dst_ip=dst_ip,
            u=udp_tcp,
            egress_rif_handle=rif1)

        ift3 = self.client.switch_api_tunnel_interface_create(device, 0,
                                                              tun_info)
        self.client.switch_api_logical_network_member_add(device, ln1, if2)
        self.client.switch_api_logical_network_member_add(device, ln1, ift3)
        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:22:22:22:22:22', 2, if2)

        try:
            tag1 = {'label': 0xabcde, 'tc': 0x5, 'ttl': 0xAA, 's': 0x0}
            tag2 = {'label': 0x54321, 'tc': 0x2, 'ttl': 0xBB, 's': 0x1}
            mpls_tags = [tag1, tag2]
            pkt = simple_tcpv6_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                ipv6_dst='4000::1',
                ipv6_src='5000::1')
            udp_sport = entropy_hash(pkt, layer='ipv6')
            mpls_pkt = mpls_udp_inner_packet(
                mpls_tags=mpls_tags, inner_frame=pkt)
            mpls_udp_pkt = simple_udp_packet(
                eth_src='00:44:44:44:44:44',
                eth_dst='00:77:66:55:44:33',
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                udp_sport=udp_sport,
                udp_dport=6635,
                with_udp_chksum=False,
                udp_payload=mpls_pkt)

            send_packet(self, swports[0], str(mpls_udp_pkt))
            verify_packets(self, pkt, [swports[1]])
        finally:
            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:22:22:22:22:22')

            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if2)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift3)

            self.client.switch_api_logical_network_delete(device, ln1)

            self.client.switch_api_tunnel_interface_delete(device, ift3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('mpls_udp')
class L2MplsUdpOuterIPV6InnerIPV4PopTest(
        api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print

        if test_param_get('target') != 'bmv2':
            return

        vrf = self.client.switch_api_vrf_create(device, 2)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, vrf_handle=vrf, rmac_handle=rmac)
        rif1 = self.client.switch_api_rif_create(0, rif_info)

        i_info1 = switcht_interface_info_t(
            rif_handle=rif1, type=SWITCH_INTERFACE_TYPE_PORT, handle=port1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            rmac_handle=rmac,
            type=SWITCH_INTERFACE_TYPE_ACCESS,
            handle=port2,
            vrf_handle=vrf)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        lognet_info = switcht_logical_network_t()
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)
        udp = switcht_udp_t(src_port=1234, dst_port=6635)
        src_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='2000::2',
            prefix_length=128)
        dst_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='3000::3',
            prefix_length=128)
        udp_tcp = switcht_udp_tcp_t(udp=udp)

        mpls_tag1 = switcht_mpls_t(label=0xabcde, exp=0x5, ttl=0x30, bos=0)
        mpls_tag2 = switcht_mpls_t(label=0x54321, exp=0x2, ttl=0x40, bos=1)
        pop_tag = [mpls_tag1, mpls_tag2]
        tun_info = switcht_tunnel_info_t(
            tunnel_type=SWITCH_TUNNEL_TYPE_MPLS_UDP,
            mpls_type=SWITCH_MPLS_TYPE_EOMPLS,
            mpls_mode=SWITCH_MPLS_MODE_TERMINATE,
            pop_tag=pop_tag,
            vrf_handle=vrf,
            src_ip=src_ip,
            dst_ip=dst_ip,
            u=udp_tcp,
            egress_rif_handle=rif1)

        ift3 = self.client.switch_api_tunnel_interface_create(device, 0,
                                                              tun_info)
        self.client.switch_api_logical_network_member_add(device, ln1, if2)
        self.client.switch_api_logical_network_member_add(device, ln1, ift3)

        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:22:22:22:22:22', 2, if2)

        try:
            tag1 = {'label': 0xabcde, 'tc': 0x5, 'ttl': 0xAA, 's': 0x0}
            tag2 = {'label': 0x54321, 'tc': 0x2, 'ttl': 0xBB, 's': 0x1}
            mpls_tags = [tag1, tag2]
            pkt = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63)
            udp_sport = entropy_hash(pkt)
            mpls_pkt = mpls_udp_inner_packet(
                mpls_tags=mpls_tags, inner_frame=pkt)
            mpls_udp_pkt = simple_udpv6_packet(
                eth_src='00:44:44:44:44:44',
                eth_dst='00:77:66:55:44:33',
                ipv6_dst='3000::3',
                ipv6_src='2000::2',
                udp_sport=udp_sport,
                udp_dport=6635,
                with_udp_chksum=False,
                udp_payload=mpls_pkt)

            send_packet(self, swports[0], str(mpls_udp_pkt))
            verify_packets(self, pkt, [swports[1]])
        finally:
            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:22:22:22:22:22')

            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if2)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift3)

            self.client.switch_api_logical_network_delete(device, ln1)

            self.client.switch_api_tunnel_interface_delete(device, ift3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('mpls_udp')
class L2MplsUdpOuterIPV6InnerIPV6PopTest(
        api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        if test_param_get('target') != 'bmv2':
            return

        vrf = self.client.switch_api_vrf_create(device, 2)
        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, vrf_handle=vrf, rmac_handle=rmac)
        rif1 = self.client.switch_api_rif_create(0, rif_info)

        i_info1 = switcht_interface_info_t(
            rif_handle=rif1, type=SWITCH_INTERFACE_TYPE_PORT, handle=port1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            rmac_handle=rmac,
            type=SWITCH_INTERFACE_TYPE_ACCESS,
            handle=port2,
            vrf_handle=vrf)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        lognet_info = switcht_logical_network_t()
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)
        udp = switcht_udp_t(src_port=1234, dst_port=6635)
        src_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='2000::2',
            prefix_length=128)
        dst_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='3000::3',
            prefix_length=128)
        udp_tcp = switcht_udp_tcp_t(udp=udp)

        mpls_tag1 = switcht_mpls_t(label=0xabcde, exp=0x5, ttl=0x30, bos=0)
        mpls_tag2 = switcht_mpls_t(label=0x54321, exp=0x2, ttl=0x40, bos=1)
        pop_tag = [mpls_tag1, mpls_tag2]
        tun_info = switcht_tunnel_info_t(
            tunnel_type=SWITCH_TUNNEL_TYPE_MPLS_UDP,
            mpls_type=SWITCH_MPLS_TYPE_EOMPLS,
            mpls_mode=SWITCH_MPLS_MODE_TERMINATE,
            pop_tag=pop_tag,
            vrf_handle=vrf,
            src_ip=src_ip,
            dst_ip=dst_ip,
            u=udp_tcp,
            egress_rif_handle=rif1)
        ift3 = self.client.switch_api_tunnel_interface_create(device, 0,
                                                              tun_info)
        self.client.switch_api_logical_network_member_add(device, ln1, if2)
        self.client.switch_api_logical_network_member_add(device, ln1, ift3)

        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:22:22:22:22:22', 2, if2)

        try:
            tag1 = {'label': 0xabcde, 'tc': 0x5, 'ttl': 0xAA, 's': 0x0}
            tag2 = {'label': 0x54321, 'tc': 0x2, 'ttl': 0xBB, 's': 0x1}
            mpls_tags = [tag1, tag2]
            pkt = simple_tcpv6_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                ipv6_dst='4000::1',
                ipv6_src='5000::1')
            udp_sport = entropy_hash(pkt, layer='ipv6')
            mpls_pkt = mpls_udp_inner_packet(
                mpls_tags=mpls_tags, inner_frame=pkt)
            mpls_udp_pkt = simple_udpv6_packet(
                eth_src='00:44:44:44:44:44',
                eth_dst='00:77:66:55:44:33',
                ipv6_dst='3000::3',
                ipv6_src='2000::2',
                udp_sport=udp_sport,
                udp_dport=6635,
                with_udp_chksum=False,
                udp_payload=mpls_pkt)

            send_packet(self, swports[0], str(mpls_udp_pkt))
            verify_packets(self, pkt, [swports[1]])
        finally:
            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:22:22:22:22:22')

            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if2)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift3)

            self.client.switch_api_logical_network_delete(device, ln1)

            self.client.switch_api_tunnel_interface_delete(device, ift3)
            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('mpls_udp')
class L2MplsUdpOuterIPV4InnerIPV4PushTest(
        api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print

        vrf = self.client.switch_api_vrf_create(device, 2)
        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, vrf_handle=vrf, rmac_handle=rmac)
        rif1 = self.client.switch_api_rif_create(0, rif_info)

        i_info1 = switcht_interface_info_t(
            rif_handle=rif1, type=SWITCH_INTERFACE_TYPE_PORT, handle=port1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            type=SWITCH_INTERFACE_TYPE_ACCESS,
            handle=port2)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        lognet_info = switcht_logical_network_t()
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)
        udp = switcht_udp_t(src_port=1234, dst_port=6635)
        src_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.1', prefix_length=32)
        dst_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.3', prefix_length=32)
        udp_tcp = switcht_udp_tcp_t(udp=udp)
        mpls_tag1 = switcht_mpls_t(label=0xabcde, exp=0x5, ttl=0x30, bos=0)
        mpls_tag2 = switcht_mpls_t(label=0x54321, exp=0x2, ttl=0x40, bos=1)
        push_tag = [mpls_tag1, mpls_tag2]
        tun_info = switcht_tunnel_info_t(
            tunnel_type=SWITCH_TUNNEL_TYPE_MPLS_UDP,
            mpls_type=SWITCH_MPLS_TYPE_EOMPLS,
            mpls_mode=SWITCH_MPLS_MODE_INITIATE,
            push_tag=push_tag,
            vrf_handle=vrf,
            src_ip=src_ip,
            dst_ip=dst_ip,
            u=udp_tcp,
            egress_rif_handle=rif1)

        ift3 = self.client.switch_api_tunnel_interface_create(device, 0,
                                                              tun_info)

        self.client.switch_api_logical_network_member_add(device, ln1, if2)
        self.client.switch_api_logical_network_member_add(device, ln1, ift3)

        nhop_key1 = switcht_nhop_key_t(intf_handle=ift3, ln_handle=ln1, ip_addr_valid=0)
        nhop1 = self.client.switch_api_nhop_create(device, nhop_key1)

        #neighbor type 5 is push l2vpn
        neighbor_entry1 = switcht_neighbor_info_t(
            nhop_handle=nhop1,
            neigh_type=SWITCH_API_NEIGHBOR_MPLS_IPV4_UDP_PUSH_L2VPN,
            interface_handle=ift3,
            mpls_label=0,
            header_count=2)
        neighbor1 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry1)

        nhop_key2 = switcht_nhop_key_t(intf_handle=rif1, ip_addr_valid=0)
        nhop2 = self.client.switch_api_nhop_create(device, nhop_key2)

        neighbor_entry2 = switcht_neighbor_info_t(
            nhop_handle=nhop2, interface_handle=rif1, mac_addr='00:44:44:44:44:44')
        neighbor2 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry2)

        self.client.switch_api_l3_route_add(device, vrf, dst_ip, nhop2)

        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:11:11:11:11:11', 2, if2)
        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:22:22:22:22:22', 2, nhop1)

        try:
            tag1 = {'label': 0xabcde, 'tc': 0x5, 'ttl': 0x30, 's': 0x0}
            tag2 = {'label': 0x54321, 'tc': 0x2, 'ttl': 0x40, 's': 0x1}
            mpls_tags = [tag1, tag2]
            pkt = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            udp_sport = entropy_hash(pkt)
            mpls_pkt = mpls_udp_inner_packet(
                mpls_tags=mpls_tags, inner_frame=pkt)
            mpls_udp_pkt = simple_udp_packet(
                eth_dst='00:44:44:44:44:44',
                eth_src='00:77:66:55:44:33',
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                udp_sport=udp_sport,
                udp_dport=6635,
                ip_id=0,
                ip_flag=0x2,
                with_udp_chksum=False,
                udp_payload=mpls_pkt)

            send_packet(self, swports[1], str(pkt))
            verify_packets(self, mpls_udp_pkt, [swports[0]])
        finally:
            self.client.switch_api_l3_route_delete(device, vrf, dst_ip, nhop2)

            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:11:11:11:11:11')
            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:22:22:22:22:22')

            self.client.switch_api_neighbor_entry_remove(device, neighbor1)
            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_neighbor_entry_remove(device, neighbor2)
            self.client.switch_api_nhop_delete(device, nhop2)

            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if2)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift3)

            self.client.switch_api_logical_network_delete(device, ln1)

            self.client.switch_api_tunnel_interface_delete(device, ift3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('mpls_udp')
class L2MplsUdpOuterIPV4InnerIPV6PushTest(
        api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        vrf = self.client.switch_api_vrf_create(device, 2)
        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, vrf_handle=vrf, rmac_handle=rmac)
        rif1 = self.client.switch_api_rif_create(0, rif_info)

        i_info1 = switcht_interface_info_t(
            rif_handle=rif1, type=SWITCH_INTERFACE_TYPE_PORT, handle=port1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            type=SWITCH_INTERFACE_TYPE_ACCESS,
            handle=port2)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        lognet_info = switcht_logical_network_t()
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)
        udp = switcht_udp_t(src_port=1234, dst_port=6635)
        src_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.1', prefix_length=32)
        dst_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.3', prefix_length=32)
        udp_tcp = switcht_udp_tcp_t(udp=udp)
        mpls_tag1 = switcht_mpls_t(label=0xabcde, exp=0x5, ttl=0x30, bos=0)
        mpls_tag2 = switcht_mpls_t(label=0x54321, exp=0x2, ttl=0x40, bos=1)
        push_tag = [mpls_tag1, mpls_tag2]
        tun_info = switcht_tunnel_info_t(
            tunnel_type=SWITCH_TUNNEL_TYPE_MPLS_UDP,
            mpls_type=SWITCH_MPLS_TYPE_EOMPLS,
            mpls_mode=SWITCH_MPLS_MODE_INITIATE,
            push_tag=push_tag,
            vrf_handle=vrf,
            src_ip=src_ip,
            dst_ip=dst_ip,
            u=udp_tcp,
            egress_rif_handle=rif1)

        ift3 = self.client.switch_api_tunnel_interface_create(device, 0,
                                                              tun_info)

        self.client.switch_api_logical_network_member_add(device, ln1, if2)
        self.client.switch_api_logical_network_member_add(device, ln1, ift3)

        nhop_key1 = switcht_nhop_key_t(intf_handle=ift3, ln_handle=ln1, ip_addr_valid=0)
        nhop1 = self.client.switch_api_nhop_create(device, nhop_key1)

        #neighbor type 5 is push l2vpn
        neighbor_entry1 = switcht_neighbor_info_t(
            nhop_handle=nhop1,
            neigh_type=SWITCH_API_NEIGHBOR_MPLS_IPV4_UDP_PUSH_L2VPN,
            interface_handle=ift3,
            mpls_label=0,
            header_count=2)
        neighbor1 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry1)

        nhop_key2 = switcht_nhop_key_t(intf_handle=rif1, ip_addr_valid=0)
        nhop2 = self.client.switch_api_nhop_create(device, nhop_key2)
        neighbor_entry2 = switcht_neighbor_info_t(
            nhop_handle=nhop2, interface_handle=rif1, mac_addr='00:44:44:44:44:44')
        neighbor2 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry2)

        self.client.switch_api_l3_route_add(device, vrf, dst_ip, nhop2)

        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:11:11:11:11:11', 2, if2)
        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:22:22:22:22:22', 2, nhop1)

        try:
            tag1 = {'label': 0xabcde, 'tc': 0x5, 'ttl': 0x30, 's': 0x0}
            tag2 = {'label': 0x54321, 'tc': 0x2, 'ttl': 0x40, 's': 0x1}
            mpls_tags = [tag1, tag2]
            pkt = simple_tcpv6_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                ipv6_dst='4000::1',
                ipv6_src='5000::1')
            udp_sport = entropy_hash(pkt, layer='ipv6')
            mpls_pkt = mpls_udp_inner_packet(
                mpls_tags=mpls_tags, inner_frame=pkt)
            mpls_udp_pkt = simple_udp_packet(
                eth_dst='00:44:44:44:44:44',
                eth_src='00:77:66:55:44:33',
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                udp_sport=udp_sport,
                udp_dport=6635,
                ip_id=0,
                ip_flag=0x2,
                with_udp_chksum=False,
                udp_payload=mpls_pkt)

            send_packet(self, swports[1], str(pkt))
            verify_packets(self, mpls_udp_pkt, [swports[0]])
        finally:
            self.client.switch_api_l3_route_delete(device, vrf, dst_ip, nhop2)

            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:11:11:11:11:11')
            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:22:22:22:22:22')

            self.client.switch_api_neighbor_entry_remove(device, neighbor1)
            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_neighbor_entry_remove(device, neighbor2)
            self.client.switch_api_nhop_delete(device, nhop2)

            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if2)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift3)

            self.client.switch_api_logical_network_delete(device, ln1)

            self.client.switch_api_tunnel_interface_delete(device, ift3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('mpls_udp')
class L2MplsUdpOuterIPV6InnerIPV4PushTest(
        api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print

        vrf = self.client.switch_api_vrf_create(device, 2)
        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, vrf_handle=vrf, rmac_handle=rmac)
        rif1 = self.client.switch_api_rif_create(0, rif_info)

        i_info1 = switcht_interface_info_t(
            rif_handle=rif1, type=SWITCH_INTERFACE_TYPE_PORT, handle=port1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            type=SWITCH_INTERFACE_TYPE_ACCESS,
            handle=port2)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        lognet_info = switcht_logical_network_t()
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)
        udp = switcht_udp_t(src_port=1234, dst_port=6635)

        src_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='2000::2',
            prefix_length=128)
        dst_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='3000::3',
            prefix_length=128)
        udp_tcp = switcht_udp_tcp_t(udp=udp)
        mpls_tag1 = switcht_mpls_t(label=0xabcde, exp=0x5, ttl=0x30, bos=0)
        mpls_tag2 = switcht_mpls_t(label=0x54321, exp=0x2, ttl=0x40, bos=1)
        push_tag = [mpls_tag1, mpls_tag2]
        tun_info = switcht_tunnel_info_t(
            tunnel_type=SWITCH_TUNNEL_TYPE_MPLS_UDP,
            mpls_type=SWITCH_MPLS_TYPE_EOMPLS,
            mpls_mode=SWITCH_MPLS_MODE_INITIATE,
            push_tag=push_tag,
            vrf_handle=vrf,
            src_ip=src_ip,
            dst_ip=dst_ip,
            u=udp_tcp,
            egress_rif_handle=rif1)

        ift3 = self.client.switch_api_tunnel_interface_create(device, 0,
                                                              tun_info)

        self.client.switch_api_logical_network_member_add(device, ln1, if2)
        self.client.switch_api_logical_network_member_add(device, ln1, ift3)

        nhop_key1 = switcht_nhop_key_t(intf_handle=ift3, ln_handle=ln1, ip_addr_valid=0)
        nhop1 = self.client.switch_api_nhop_create(device, nhop_key1)

        #neighbor type 5 is push l2vpn
        neighbor_entry1 = switcht_neighbor_info_t(
            nhop_handle=nhop1,
            neigh_type=SWITCH_API_NEIGHBOR_MPLS_IPV6_UDP_PUSH_L2VPN,
            interface_handle=ift3,
            mpls_label=0,
            header_count=2)
        neighbor1 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry1)

        nhop_key2 = switcht_nhop_key_t(intf_handle=rif1, ip_addr_valid=0)
        nhop2 = self.client.switch_api_nhop_create(device, nhop_key2)

        neighbor_entry2 = switcht_neighbor_info_t(
            nhop_handle=nhop2, interface_handle=rif1, mac_addr='00:44:44:44:44:44')
        neighbor2 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry2)

        self.client.switch_api_l3_route_add(device, vrf, dst_ip, nhop2)

        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:11:11:11:11:11', 2, if2)
        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:22:22:22:22:22', 2, nhop1)

        try:
            tag1 = {'label': 0xabcde, 'tc': 0x5, 'ttl': 0x30, 's': 0x0}
            tag2 = {'label': 0x54321, 'tc': 0x2, 'ttl': 0x40, 's': 0x1}
            mpls_tags = [tag1, tag2]
            pkt = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            udp_sport = entropy_hash(pkt)
            mpls_pkt = mpls_udp_inner_packet(
                mpls_tags=mpls_tags, inner_frame=pkt)
            mpls_udp_pkt = simple_udpv6_packet(
                eth_dst='00:44:44:44:44:44',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='3000::3',
                ipv6_src='2000::2',
                udp_sport=udp_sport,
                udp_dport=6635,
                with_udp_chksum=False,
                udp_payload=mpls_pkt)

            send_packet(self, swports[1], str(pkt))
            verify_packets(self, mpls_udp_pkt, [swports[0]])
        finally:
            self.client.switch_api_l3_route_delete(device, vrf, dst_ip, nhop2)

            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:11:11:11:11:11')
            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:22:22:22:22:22')

            self.client.switch_api_neighbor_entry_remove(device, neighbor1)
            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_neighbor_entry_remove(device, neighbor2)
            self.client.switch_api_nhop_delete(device, nhop2)

            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if2)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift3)

            self.client.switch_api_logical_network_delete(device, ln1)

            self.client.switch_api_tunnel_interface_delete(device, ift3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('mpls_udp')
class L2MplsUdpOuterIPV6InnerIPV6PushTest(
        api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print

        vrf = self.client.switch_api_vrf_create(device, 2)
        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF, vrf_handle=vrf, rmac_handle=rmac)
        rif1 = self.client.switch_api_rif_create(0, rif_info)

        i_info1 = switcht_interface_info_t(
            rif_handle=rif1, type=SWITCH_INTERFACE_TYPE_PORT, handle=port1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            type=SWITCH_INTERFACE_TYPE_ACCESS,
            handle=port2)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        lognet_info = switcht_logical_network_t()
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)
        udp = switcht_udp_t(src_port=1234, dst_port=6635)

        src_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='2000::2',
            prefix_length=128)
        dst_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='3000::3',
            prefix_length=128)
        udp_tcp = switcht_udp_tcp_t(udp=udp)
        mpls_tag1 = switcht_mpls_t(label=0xabcde, exp=0x5, ttl=0x30, bos=0)
        mpls_tag2 = switcht_mpls_t(label=0x54321, exp=0x2, ttl=0x40, bos=1)
        push_tag = [mpls_tag1, mpls_tag2]
        tun_info = switcht_tunnel_info_t(
            tunnel_type=SWITCH_TUNNEL_TYPE_MPLS_UDP,
            mpls_type=SWITCH_MPLS_TYPE_EOMPLS,
            mpls_mode=SWITCH_MPLS_MODE_INITIATE,
            push_tag=push_tag,
            vrf_handle=vrf,
            src_ip=src_ip,
            dst_ip=dst_ip,
            u=udp_tcp,
            egress_rif_handle=rif1)

        ift3 = self.client.switch_api_tunnel_interface_create(device, 0,
                                                              tun_info)

        self.client.switch_api_logical_network_member_add(device, ln1, if2)
        self.client.switch_api_logical_network_member_add(device, ln1, ift3)

        nhop_key1 = switcht_nhop_key_t(intf_handle=ift3, ln_handle=ln1, ip_addr_valid=0)
        nhop1 = self.client.switch_api_nhop_create(device, nhop_key1)

        #neighbor type 5 is push l2vpn
        neighbor_entry1 = switcht_neighbor_info_t(
            nhop_handle=nhop1,
            neigh_type=SWITCH_API_NEIGHBOR_MPLS_IPV6_UDP_PUSH_L2VPN,
            interface_handle=ift3,
            mpls_label=0,
            header_count=2)
        neighbor1 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry1)

        nhop_key2 = switcht_nhop_key_t(intf_handle=rif1, ip_addr_valid=0)
        nhop2 = self.client.switch_api_nhop_create(device, nhop_key2)

        neighbor_entry2 = switcht_neighbor_info_t(
            nhop_handle=nhop2, interface_handle=rif1, mac_addr='00:44:44:44:44:44')
        neighbor2 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry2)

        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:11:11:11:11:11', 2, if2)
        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:22:22:22:22:22', 2, nhop1)

        self.client.switch_api_l3_route_add(device, vrf, dst_ip, nhop2)

        try:
            tag1 = {'label': 0xabcde, 'tc': 0x5, 'ttl': 0x30, 's': 0x0}
            tag2 = {'label': 0x54321, 'tc': 0x2, 'ttl': 0x40, 's': 0x1}
            mpls_tags = [tag1, tag2]
            pkt = simple_tcpv6_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                ipv6_dst='4000::1',
                ipv6_src='5000::1')
            udp_sport = entropy_hash(pkt, layer='ipv6')

            mpls_pkt = mpls_udp_inner_packet(
                mpls_tags=mpls_tags, inner_frame=pkt)
            mpls_udp_pkt = simple_udpv6_packet(
                eth_dst='00:44:44:44:44:44',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='3000::3',
                ipv6_src='2000::2',
                udp_sport=udp_sport,
                udp_dport=6635,
                with_udp_chksum=False,
                udp_payload=mpls_pkt)

            send_packet(self, swports[1], str(pkt))
            verify_packets(self, mpls_udp_pkt, [swports[0]])
        finally:
            self.client.switch_api_l3_route_delete(device, vrf, dst_ip, nhop2)

            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:11:11:11:11:11')
            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:22:22:22:22:22')

            self.client.switch_api_neighbor_entry_remove(device, neighbor1)
            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_neighbor_entry_remove(device, neighbor2)
            self.client.switch_api_nhop_delete(device, nhop2)

            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if2)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift3)

            self.client.switch_api_logical_network_delete(device, ln1)

            self.client.switch_api_tunnel_interface_delete(device, ift3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('mpls_udp')
class L2MplsUdpPopPushTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        #pop push not supported right now
        return

        vrf = self.client.switch_api_vrf_create(device, 2)
        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        i_info1 = switcht_interface_info_t(
            rmac_handle=rmac,
            type=SWITCH_INTERFACE_TYPE_ACCESS,
            handle=port1,
            vrf_handle=vrf)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            rmac_handle=rmac,
            type=SWITCH_INTERFACE_TYPE_ACCESS,
            handle=port2,
            vrf_handle=vrf)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        lognet_info = switcht_logical_network_t()
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)
        #stuff for pop
        udp = switcht_udp_t(src_port=1234, dst_port=6635)
        src_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.1', prefix_length=32)
        dst_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.3', prefix_length=32)
        udp_tcp = switcht_udp_tcp_t(udp=udp)

        mpls_tag1 = switcht_mpls_t(label=0xabcde, exp=0x5, ttl=0x30, bos=0)
        mpls_tag2 = switcht_mpls_t(label=0x54321, exp=0x2, ttl=0x40, bos=1)
        pop_tag = [mpls_tag1, mpls_tag2]
        tun_info = switcht_tunnel_info_t(
            tunnel_type=SWITCH_TUNNEL_TYPE_MPLS_UDP,
            mpls_type=SWITCH_MPLS_TYPE_EOMPLS,
            mpls_mode=SWITCH_MPLS_MODE_TERMINATE,
            pop_tag=pop_tag,
            vrf_handle=vrf,
            src_ip=src_ip,
            dst_ip=dst_ip,
            u=udp_tcp,
            egress_rif_handle=rif1)
        ift3 = self.client.switch_api_tunnel_interface_create(device, 0,
                                                              tun_info)
        #stuff for the push stuff
        udp = switcht_udp_t(src_port=1234, dst_port=6635)
        src_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='2.2.2.2', prefix_length=32)
        dst_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='3.3.3.3', prefix_length=32)
        udp_tcp = switcht_udp_tcp_t(udp=udp)
        mpls_tag1 = switcht_mpls_t(label=0x99999, exp=0x5, ttl=0x30, bos=0)
        mpls_tag2 = switcht_mpls_t(label=0x56789, exp=0x2, ttl=0x40, bos=1)
        push_tag = [mpls_tag1, mpls_tag2]

        tun_info2 = switcht_tunnel_info_t(
            tunnel_type=SWITCH_TUNNEL_TYPE_MPLS_UDP,
            mpls_type=SWITCH_MPLS_TYPE_EOMPLS,
            mpls_mode=SWITCH_MPLS_MODE_INITIATE,
            push_tag=push_tag,
            vrf_handle=vrf,
            src_ip=src_ip,
            dst_ip=dst_ip,
            u=udp_tcp,
            egress_rif_handle=rif1)

        ift4 = self.client.switch_api_tunnel_interface_create(device, 0,
                                                              tun_info2)
        self.client.switch_api_logical_network_member_add(device, ln1, if2)
        self.client.switch_api_logical_network_member_add(device, ln1, ift3)
        self.client.switch_api_logical_network_member_add(device, ln1, ift4)
        nhop_key1 = switcht_nhop_key_t(intf_handle=ift3, ip_addr_valid=0)
        nhop1 = self.client.switch_api_nhop_create(device, nhop_key1)
        #neighbor type 5 is push l2vpn
        neighbor_entry1 = switcht_neighbor_info_t(
            nhop_handle=nhop1,
            neigh_type=SWITCH_API_NEIGHBOR_MPLS_IPV4_UDP_PUSH_L2VPN,
            interface_handle=ift4,
            mpls_label=0,
            header_count=2)
        neighbor1 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry1)
        neighbor_entry2 = switcht_neighbor_info_t(
            nhop_handle=0, interface_handle=ift4, mac_addr='00:44:44:44:44:44')
        neighbor2 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry2)
        self.client.switch_api_mac_table_entry_create(
            device, ln1, '00:22:22:22:22:22', 2, nhop1)

        try:
            tag1 = {'label': 0xabcde, 'tc': 0x5, 'ttl': 0x30, 's': 0x0}
            tag2 = {'label': 0x54321, 'tc': 0x2, 'ttl': 0x40, 's': 0x1}
            mpls_tags_1 = [tag1, tag2]
            tag3 = {'label': 0x99999, 'tc': 0x5, 'ttl': 0x30, 's': 0x0}
            tag4 = {'label': 0x56789, 'tc': 0x2, 'ttl': 0x40, 's': 0x1}
            mpls_tags_2 = [tag3, tag4]
            pkt = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            udp_sport_1 = entropy_hash(pkt)
            mpls_pkt_1 = mpls_udp_inner_packet(
                mpls_tags=mpls_tags_1, inner_frame=pkt)
            mpls_udp_pkt_1 = simple_udp_packet(
                eth_src='00:44:44:44:44:44',
                eth_dst='00:77:66:55:44:33',
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                udp_sport=udp_sport_1,
                udp_dport=6635,
                with_udp_chksum=False,
                udp_payload=mpls_pkt_1)
            mpls_pkt_1 = mpls_udp_inner_packet(
                mpls_tags=mpls_tags_2, inner_frame=pkt)
            udp_sport_2 = entropy_hash(pkt)
            mpls_udp_pkt_2 = simple_udp_packet(
                eth_dst='00:44:44:44:44:44',
                eth_src='00:77:66:55:44:33',
                ip_dst='3.3.3.3',
                ip_src='2.2.2.2',
                udp_sport=udp_sport_2,
                udp_dport=6635,
                ip_id=0,
                with_udp_chksum=False,
                udp_payload=mpls_pkt_1)

            send_packet(self, swports[1], str(mpls_udp_pkt_1))
            verify_packets(self, mpls_udp_pkt_2, [swports[0]])
        finally:
            self.client.switch_api_mac_table_entry_delete(device, ln1,
                                                          '00:22:22:22:22:22')

            self.client.switch_api_neighbor_entry_remove(device, neighbor1)
            self.client.switch_api_neighbor_entry_remove(device, neighbor2)
            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if2)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift3)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift4)

            self.client.switch_api_logical_network_delete(device, ln1)

            self.client.switch_api_tunnel_interface_delete(device, ift3)
            self.client.switch_api_tunnel_interface_delete(device, ift4)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('mpls_udp')
class L2MplsUdpSwapTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        #swap is not supported
        return

        vrf = self.client.switch_api_vrf_create(device, 2)
        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        i_info1 = switcht_interface_info_t(
            rmac_handle=rmac,
            type=SWITCH_INTERFACE_TYPE_ACCESS,
            handle=port1,
            vrf_handle=vrf)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            rmac_handle=rmac,
            type=SWITCH_INTERFACE_TYPE_ACCESS,
            handle=port2,
            vrf_handle=vrf)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        lognet_info = switcht_logical_network_t()
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)
        udp = switcht_udp_t(src_port=1234, dst_port=6635)
        src_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.1', prefix_length=32)
        dst_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.3', prefix_length=32)
        udp_tcp = switcht_udp_tcp_t(udp=udp)
        old_mpls_tag = switcht_mpls_t(label=0xabcde, exp=0x5, ttl=0x30, bos=0)
        new_mpls_tag = switcht_mpls_t(label=0x98765, exp=0x9, ttl=0x30, bos=0)
        inner_tag = switcht_mpls_t(label=0x54321, exp=0x2, ttl=0x40, bos=1)
        swap_info = switcht_mpls_swap_t(
            old_tag=old_mpls_tag, new_tag=new_mpls_tag)
        mpls_encap = switcht_mpls_encap_t(
            mpls_type=SWITCH_MPLS_TYPE_EOMPLS,
            mpls_mode=SWITCH_MPLS_MODE_TRANSIT,
            bd_handle=ln1)
        ip_encap = switcht_ip_encap_t(
            vrf_handle=vrf,
            src_ip=src_ip,
            dst_ip=dst_ip,
            u=udp_tcp,
            mpls_encap=mpls_encap)
        tunnel_encap = switcht_tunnel_encap_t(ip_encap=ip_encap)
        flags = switcht_interface_flags(flood_enabled=0, core_intf=0)
        tun_info = switcht_tunnel_info_t(
            tunnel_type=SWITCH_TUNNEL_TYPE_MPLS_UDP,
            egress_rif_handle=rif1,
            flags=flags)
        ift3 = self.client.switch_api_tunnel_interface_create(device, 0,
                                                              tun_info)
        nhop_key1 = switcht_nhop_key_t(intf_handle=ift3, ip_addr_valid=0)
        nhop1 = self.client.switch_api_nhop_create(device, nhop_key1)
        self.client.switch_api_logical_network_member_add(device, ln1, if2)
        self.client.switch_api_logical_network_member_add(device, ln1, ift3)
        #neighbor type 5 is push l2vpn
        neighbor_entry1 = switcht_neighbor_info_t(
            nhop_handle=nhop1,
            neigh_type=SWITCH_API_NEIGHBOR_MPLS_SWAP_L3VPN,
            interface_handle=ift3,
            mpls_label=0x98765,
            mac_addr='00:44:44:44:44:44',
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L2)
        neighbor1 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry1)

        try:
            old_tag = {'label': 0xabcde, 'tc': 0x5, 'ttl': 0x30, 's': 0x0}
            new_tag = {'label': 0x98765, 'tc': 0x5, 'ttl': 0x2f, 's': 0x0}
            inner_tag = {'label': 0x54321, 'tc': 0x2, 'ttl': 0x40, 's': 0x1}
            mpls_tags1 = [old_tag, inner_tag]
            mpls_tags2 = [new_tag, inner_tag]
            pkt = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            udp_sport = entropy_hash(pkt)
            mpls_pkt = mpls_udp_inner_packet(
                mpls_tags=mpls_tags1, inner_frame=pkt)
            mpls_pkt2 = simple_mpls_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:44:44:44:44:44',
                mpls_tags=mpls_tags2,
                inner_frame=pkt)
            mpls_udp_pkt = simple_udp_packet(
                eth_src='00:44:44:44:44:44',
                eth_dst='00:77:66:55:44:33',
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                udp_sport=udp_sport,
                udp_dport=6635,
                with_udp_chksum=False,
                udp_payload=mpls_pkt)

            send_packet(self, swports[1], str(mpls_udp_pkt))
            verify_packets(self, mpls_pkt2, [swports[0]])
        finally:
            self.client.switch_api_neighbor_entry_remove(device, neighbor1)
            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if2)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift3)

            self.client.switch_api_logical_network_delete(device, ln1)

            self.client.switch_api_tunnel_interface_delete(device, ift3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('mpls_udp')
class L2MplsUdpSwapTest2(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        #swap tests are not supported
        return

        vrf = self.client.switch_api_vrf_create(device, 2)
        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        i_info1 = switcht_interface_info_t(
            rmac_handle=rmac,
            type=SWITCH_INTERFACE_TYPE_ACCESS,
            handle=port1,
            vrf_handle=vrf)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            rmac_handle=rmac,
            type=SWITCH_INTERFACE_TYPE_ACCESS,
            handle=port2,
            vrf_handle=vrf)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        lognet_info = switcht_logical_network_t()
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)
        udp = switcht_udp_t(src_port=1234, dst_port=6635)
        src_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.1', prefix_length=32)
        dst_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.3', prefix_length=32)
        udp_tcp = switcht_udp_tcp_t(udp=udp)
        old_mpls_tag = switcht_mpls_t(label=0xabcde, exp=0x5, ttl=0x30, bos=0)
        new_mpls_tag = switcht_mpls_t(label=0x98765, exp=0x9, ttl=0x30, bos=0)
        inner_tag = switcht_mpls_t(label=0x54321, exp=0x2, ttl=0x40, bos=1)
        swap_info = switcht_mpls_swap_t(
            old_tag=old_mpls_tag, new_tag=new_mpls_tag)
        mpls_encap = switcht_mpls_encap_t(
            mpls_type=SWITCH_MPLS_TYPE_EOMPLS,
            mpls_mode=SWITCH_MPLS_MODE_TRANSIT,
            bd_handle=ln1)
        ip_encap = switcht_ip_encap_t(
            vrf_handle=vrf,
            src_ip=src_ip,
            dst_ip=dst_ip,
            u=udp_tcp,
            mpls_encap=mpls_encap)
        tunnel_encap = switcht_tunnel_encap_t(ip_encap=ip_encap)
        flags = switcht_interface_flags(flood_enabled=0, core_intf=0)
        tun_info = switcht_tunnel_info_t(
            tunnel_type=SWITCH_TUNNEL_TYPE_MPLS_UDP,
            egress_rif_handle=rif1,
            flags=flags)
        ift3 = self.client.switch_api_tunnel_interface_create(device, 0,
                                                              tun_info)
        nhop_key1 = switcht_nhop_key_t(intf_handle=ift3, ip_addr_valid=0)
        nhop1 = self.client.switch_api_nhop_create(device, nhop_key1)
        self.client.switch_api_logical_network_member_add(device, ln1, if2)
        self.client.switch_api_logical_network_member_add(device, ln1, ift3)
        #neighbor type 5 is push l2vpn
        neighbor_entry1 = switcht_neighbor_info_t(
            nhop_handle=nhop1,
            neigh_type=SWITCH_API_NEIGHBOR_MPLS_IPV4_UDP_SWAP_L3VPN,
            interface_handle=ift3,
            mpls_label=0x98765,
            mac_addr='00:44:44:44:44:44',
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L2)
        neighbor1 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry1)
        neighbor_entry2 = switcht_neighbor_info_t(
            nhop_handle=0, interface_handle=ift3, mac_addr='00:44:44:44:44:44')
        neighbor2 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry2)

        try:
            old_tag = {'label': 0xabcde, 'tc': 0x5, 'ttl': 0x30, 's': 0x0}
            new_tag = {'label': 0x98765, 'tc': 0x5, 'ttl': 0x2f, 's': 0x0}
            inner_tag = {'label': 0x54321, 'tc': 0x2, 'ttl': 0x40, 's': 0x1}
            mpls_tags1 = [old_tag, inner_tag]
            mpls_tags2 = [new_tag, inner_tag]
            pkt = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            mpls_pkt1 = simple_mpls_packet(
                eth_src='00:33:33:33:33:33',
                eth_dst='00:77:66:55:44:33',
                mpls_tags=mpls_tags1,
                inner_frame=pkt)

            udp_sport = entropy_hash(mpls_pkt1, layer='ether', ifindex=3)
            mpls_pkt2 = mpls_udp_inner_packet(
                mpls_tags=mpls_tags2, inner_frame=pkt)
            mpls_udp_pkt = simple_udp_packet(
                eth_dst='00:44:44:44:44:44',
                eth_src='00:77:66:55:44:33',
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                udp_sport=udp_sport,
                udp_dport=6635,
                ip_id=0,
                with_udp_chksum=False,
                udp_payload=mpls_pkt2)

            send_packet(self, swports[1], str(mpls_pkt1))
            verify_packets(self, mpls_udp_pkt, [swports[0]])
        finally:
            self.client.switch_api_neighbor_entry_remove(device, neighbor1)
            self.client.switch_api_neighbor_entry_remove(device, neighbor2)
            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if2)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift3)

            self.client.switch_api_logical_network_delete(device, ln1)

            self.client.switch_api_tunnel_interface_delete(device, ift3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('mpls_udp')
class L2MplsUdpSwapPushTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        #swap tests are not supported
        return

        vrf = self.client.switch_api_vrf_create(device, 2)
        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        i_info1 = switcht_interface_info_t(
            rmac_handle=rmac,
            type=SWITCH_INTERFACE_TYPE_ACCESS,
            handle=port1,
            vrf_handle=vrf)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            rmac_handle=rmac,
            type=SWITCH_INTERFACE_TYPE_ACCESS,
            handle=port2,
            vrf_handle=vrf)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        lognet_info = switcht_logical_network_t()
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)
        udp = switcht_udp_t(src_port=1234, dst_port=6635)
        src_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.1', prefix_length=32)
        dst_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.3', prefix_length=32)
        udp_tcp = switcht_udp_tcp_t(udp=udp)
        old_mpls_tag = switcht_mpls_t(label=0xabcde, exp=0x5, ttl=0x30, bos=0)
        new_mpls_tag_top = switcht_mpls_t(
            vrf_handle=vrfx98765, exp=0x5, ttl=0x40, bos=0)
        new_mpls_tag_bottom = switcht_mpls_t(
            vrf_handle=vrfx66666, exp=0x5, ttl=0x40, bos=0)
        inner_tag = switcht_mpls_t(label=0x54321, exp=0x2, ttl=0x40, bos=1)
        swap_push_info = switcht_mpls_swap_push_t(
            old_tag=old_mpls_tag,
            new_tag=[new_mpls_tag_top, new_mpls_tag_bottom])
        mpls_encap = switcht_mpls_encap_t(
            mpls_type=SWITCH_MPLS_TYPE_EOMPLS,
            mpls_mode=SWITCH_MPLS_MODE_TRANSIT,
            bd_handle=ln1)
        ip_encap = switcht_ip_encap_t(
            vrf_handle=vrf,
            src_ip=src_ip,
            dst_ip=dst_ip,
            u=udp_tcp,
            mpls_encap=mpls_encap)
        tunnel_encap = switcht_tunnel_encap_t(ip_encap=ip_encap)
        flags = switcht_interface_flags(flood_enabled=0, core_intf=0)
        tun_info = switcht_tunnel_info_t(
            tunnel_type=SWITCH_TUNNEL_TYPE_MPLS_UDP,
            egress_rif_handle=rif1,
            flags=flags)
        ift3 = self.client.switch_api_tunnel_interface_create(device, 0,
                                                              tun_info)
        nhop_key1 = switcht_nhop_key_t(intf_handle=ift3, ip_addr_valid=0)
        nhop1 = self.client.switch_api_nhop_create(device, nhop_key1)
        self.client.switch_api_logical_network_member_add(device, ln1, if2)
        self.client.switch_api_logical_network_member_add(device, ln1, ift3)
        #neighbor type 5 is push l2vpn
        neighbor_entry1 = switcht_neighbor_info_t(
            nhop_handle=nhop1,
            neigh_type=SWITCH_API_NEIGHBOR_MPLS_SWAP_PUSH_L3VPN,
            interface_handle=ift3,
            mpls_label=0x98765,
            mac_addr='00:44:44:44:44:44',
            header_count=1,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L2)
        neighbor1 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry1)
        neighbor_entry2 = switcht_neighbor_info_t(
            nhop_handle=0, interface_handle=ift3, mac_addr='00:44:44:44:44:44')
        neighbor2 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry2)

        try:
            old_tag = {'label': 0xabcde, 'tc': 0x5, 'ttl': 0x30, 's': 0x0}
            new_tag_top = {'label': 0x98765, 'tc': 0x5, 'ttl': 0x40, 's': 0x0}
            new_tag_bottom = {
                'label': 0x66666,
                'tc': 0x5,
                'ttl': 0x40,
                's': 0x0
            }
            inner_tag = {'label': 0x54321, 'tc': 0x2, 'ttl': 0x40, 's': 0x1}
            mpls_tags1 = [old_tag, inner_tag]
            mpls_tags2 = [new_tag_top, new_tag_bottom, inner_tag]
            pkt = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            udp_sport = entropy_hash(pkt)
            mpls_pkt = mpls_udp_inner_packet(
                mpls_tags=mpls_tags1, inner_frame=pkt)
            mpls_pkt2 = simple_mpls_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:44:44:44:44:44',
                mpls_tags=mpls_tags2,
                inner_frame=pkt)
            mpls_udp_pkt = simple_udp_packet(
                eth_src='00:44:44:44:44:44',
                eth_dst='00:77:66:55:44:33',
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                udp_sport=udp_sport,
                udp_dport=6635,
                with_udp_chksum=False,
                udp_payload=mpls_pkt)

            send_packet(self, swports[1], str(mpls_udp_pkt))
            verify_packets(self, mpls_pkt2, [swports[0]])
        finally:
            self.client.switch_api_neighbor_entry_remove(device, neighbor1)
            self.client.switch_api_neighbor_entry_remove(device, neighbor2)
            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 if2)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift3)

            self.client.switch_api_logical_network_delete(device, ln1)

            self.client.switch_api_tunnel_interface_delete(device, ift3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('mpls_udp')
class L3MplsUdpPopTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        #L3 is not supported
        return

        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')
        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True, )

        rif1 = self.client.switch_api_rif_create(0, rif_info)
        rif2 = self.client.switch_api_rif_create(0, rif_info)

        i_info1 = switcht_interface_info_t(
            rif_handle=rif1, type=SWITCH_INTERFACE_TYPE_PORT, handle=port1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        i_info2 = switcht_interface_info_t(
            rif_handle=rif2, type=SWITCH_INTERFACE_TYPE_PORT, handle=port2)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        lognet_info = switcht_logical_network_t(
            vrf_handle=vrf, rmac_handle=rmac, ipv4_unicast_enabled=True)
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)
        udp = switcht_udp_t(src_port=1234, dst_port=6635)
        src_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.1', prefix_length=32)
        dst_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.3', prefix_length=32)
        udp_tcp = switcht_udp_tcp_t(udp=udp)

        mpls_tag1 = switcht_mpls_t(label=0xabcde, exp=0x5, ttl=0x30, bos=0)
        mpls_tag2 = switcht_mpls_t(label=0x54321, exp=0x2, ttl=0x40, bos=1)
        pop_tag = [mpls_tag1, mpls_tag2]
        tun_info = switcht_tunnel_info_t(
            tunnel_type=SWITCH_TUNNEL_TYPE_MPLS_UDP,
            mpls_type=SWITCH_MPLS_TYPE_IPV4_MPLS,
            mpls_mode=SWITCH_MPLS_MODE_TERMINATE,
            pop_tag=pop_tag,
            vrf_handle=vrf,
            src_ip=src_ip,
            dst_ip=dst_ip,
            u=udp_tcp,
            egress_rif_handle=rif1)
        ift3 = self.client.switch_api_tunnel_interface_create(device, 0,
                                                              tun_info)
        self.client.switch_api_logical_network_member_add(device, ln1, if1)
        self.client.switch_api_logical_network_member_add(device, ln1, ift3)

        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='20.20.20.1',
            prefix_length=32)
        nhop_key = switcht_nhop_key_t(intf_handle=rif2, ip_addr_valid=0)
        nhop = self.client.switch_api_nhop_create(device, nhop_key)
        neighbor_entry = switcht_neighbor_info_t(
            nhop_handle=nhop,
            interface_handle=if2,
            mac_addr='00:33:33:33:33:33',
            ip_addr=i_ip3,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L3)
        neighbor = self.client.switch_api_neighbor_entry_add(device,
                                                             neighbor_entry)
        self.client.switch_api_l3_route_add(device, vrf, i_ip3, nhop)

        try:
            tag1 = {'label': 0xabcde, 'tc': 0x5, 'ttl': 0xAA, 's': 0x0}
            tag2 = {'label': 0x54321, 'tc': 0x2, 'ttl': 0xBB, 's': 0x1}
            mpls_tags = [tag1, tag2]
            pkt1 = simple_ip_only_packet(
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64,
                pktlen=86)
            udp_sport = entropy_hash(pkt1)
            pkt = simple_tcp_packet(
                eth_dst='00:33:33:33:33:33',
                eth_src='00:77:66:55:44:33',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63)

            mpls_pkt = mpls_udp_inner_packet(
                mpls_tags=mpls_tags, inner_frame=pkt1)
            mpls_udp_pkt = simple_udp_packet(
                eth_src='00:44:44:44:44:44',
                eth_dst='00:77:66:55:44:33',
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                udp_sport=udp_sport,
                udp_dport=6635,
                with_udp_chksum=False,
                udp_payload=mpls_pkt)

            send_packet(self, swports[0], str(mpls_udp_pkt))
            verify_packets(self, pkt, [swports[1]])
        finally:
            self.client.switch_api_neighbor_entry_remove(device, neighbor)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip3, nhop)
            self.client.switch_api_nhop_delete(device, nhop)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift3)
            self.client.switch_api_logical_network_delete(device, ln1)
            self.client.switch_api_tunnel_interface_delete(device, ift3)
            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
@group('mpls_udp')
class L3MplsUdpPushTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        #L3 is not supported
        return

        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=true)
        rif1 = self.client.switch_api_rif_create(0, rif_info)

        i_info1 = switcht_interface_info_t(
            rif_handle=rif1, type=SWITCH_INTERFACE_TYPE_PORT, handle=port1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        lognet_info = switcht_logical_network_t(
            vrf_handle=vrf, rmac_handle=rmac, ipv4_unicast_enabled=True)
        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)
        udp = switcht_udp_t(src_port=1234, dst_port=6635)
        src_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.1', prefix_length=32)
        dst_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='1.1.1.3', prefix_length=32)
        udp_tcp = switcht_udp_tcp_t(udp=udp)
        mpls_tag1 = switcht_mpls_t(label=0xabcde, exp=0x5, ttl=0x30, bos=0)
        mpls_tag2 = switcht_mpls_t(label=0x54321, exp=0x2, ttl=0x40, bos=1)
        tun_info = switcht_tunnel_info_t(
            tunnel_type=SWITCH_TUNNEL_TYPE_MPLS_UDP,
            mpls_type=SWITCH_MPLS_TYPE_EOMPLS,
            mpls_mode=SWITCH_MPLS_MODE_INITIATE,
            push_tag=push_tag,
            vrf_handle=vrf,
            src_ip=src_ip,
            dst_ip=dst_ip,
            u=udp_tcp,
            egress_rif_handle=rif1)

        ift3 = self.client.switch_api_tunnel_interface_create(device, 0,
                                                              tun_info)

        self.client.switch_api_logical_network_member_add(device, ln1, ift3)
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='20.20.20.1',
            prefix_length=32)

        nhop_key1 = switcht_nhop_key_t(intf_handle=ift3, ip_addr_valid=0)
        nhop1 = self.client.switch_api_nhop_create(device, nhop_key1)

        #neighbor type 5 is push l2vpn
        neighbor_entry1 = switcht_neighbor_info_t(
            nhop_handle=nhop1,
            neigh_type=SWITCH_API_NEIGHBOR_MPLS_IPV4_UDP_PUSH_L3VPN,
            interface_handle=ift3,
            mpls_label=0,
            header_count=2)
        neighbor1 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry1)

        neighbor_entry2 = switcht_neighbor_info_t(
            nhop_handle=0, interface_handle=ift3, mac_addr='00:44:44:44:44:44')
        neighbor2 = self.client.switch_api_neighbor_entry_add(device,
                                                              neighbor_entry2)
        self.client.switch_api_l3_route_add(device, vrf, i_ip3, nhop1)

        try:
            tag1 = {'label': 0xabcde, 'tc': 0x5, 'ttl': 0x30, 's': 0x0}
            tag2 = {'label': 0x54321, 'tc': 0x2, 'ttl': 0x40, 's': 0x1}
            mpls_tags = [tag1, tag2]
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)

            pkt2 = simple_ip_only_packet(
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63,
                pktlen=86)

            udp_sport = entropy_hash(pkt)
            mpls_pkt = mpls_udp_inner_packet(
                mpls_tags=mpls_tags, inner_frame=pkt2)
            mpls_udp_pkt = simple_udp_packet(
                eth_dst='00:44:44:44:44:44',
                eth_src='00:77:66:55:44:33',
                ip_dst='1.1.1.3',
                ip_src='1.1.1.1',
                udp_sport=udp_sport,
                udp_dport=6635,
                ip_id=0,
                with_udp_chksum=False,
                udp_payload=mpls_pkt)

            send_packet(self, swports[1], str(pkt))
            verify_packets(self, mpls_udp_pkt, [swports[0]])
        finally:
            self.client.switch_api_neighbor_entry_remove(device, neighbor1)
            self.client.switch_api_neighbor_entry_remove(device, neighbor2)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip3, nhop1)
            self.client.switch_api_nhop_delete(device, nhop1)
            self.client.switch_api_logical_network_member_remove(device, ln1,
                                                                 ift3)
            self.client.switch_api_logical_network_delete(device, ln1)
            self.client.switch_api_tunnel_interface_delete(device, ift3)
            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_rif_delete(0, rif1)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)
