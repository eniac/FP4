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

try:
  import pltfm_pm_rpc
  from pltfm_pm_rpc.ttypes import *
except ImportError:
  pass

import pdb

import ptf.dataplane as dataplane

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *

import os
import ptf.mask

from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

from erspan3 import *

this_dir = os.path.dirname(os.path.abspath(__file__))

sys.path.append(os.path.join(this_dir, '../../base'))
sys.path.append(os.path.join(this_dir, '../../base/api-tests'))
from common.utils import *
from common.api_utils import *
import api_base_tests
import pd_base_tests

device = 0
swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)

if swports == []:
    swports = [x for x in range(65)]


@group('sr')
class L3Srv6Test(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "IPv6 Segment Routing test"
        self.cpu_port = get_cpu_port(self)

        #api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.client.switch_api_init(device)
        vrf = self.client.switch_api_vrf_create(device, 2)
        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v6_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info)
        rif2 = self.client.switch_api_rif_create(0, rif_info)

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)
        i_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='2000::2',
            prefix_length=120)
        self.client.switch_api_l3_interface_address_add(device, rif1, vrf,
                                                        i_ip1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='3000::2',
            prefix_length=120)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf,
                                                        i_ip2)

        # Add a static route
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='2000::3',
            prefix_length=128)
        nhop, neighbor = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip3, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(device, vrf, i_ip3, nhop)

        # send the test packet(s)
        try:
            print "Transit packet with firstSeg = 2, segLeft = 2"
            seg_list = ['2000::4', '2000::3', '2000::2']
            pkt = simple_ipv6_sr_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ipv6_dst='2000::3',
                ipv6_src='2000::1',
                ipv6_hlim=64,
                srh_seg_left=2,
                srh_first_seg=2,
                srh_seg_list=seg_list)
            exp_pkt = simple_ipv6_sr_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='2000::3',
                ipv6_src='2000::1',
                ipv6_hlim=63,
                srh_seg_left=1,
                srh_first_seg=2,
                srh_seg_list=seg_list)

            send_packet(self, swports[1], str(pkt))
            verify_packets(self, exp_pkt, [swports[2]])

            print "Endpoint with firstSeg = 2, segLeft = 2"
            seg_list = ['2000::4', '2000::3', '2000::2']
            pkt = simple_ipv6_sr_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ipv6_dst='2000::2',
                ipv6_src='2000::1',
                ipv6_hlim=64,
                srh_seg_left=2,
                srh_first_seg=2,
                srh_seg_list=seg_list)
            exp_pkt = simple_ipv6_sr_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='2000::3',
                ipv6_src='2000::1',
                ipv6_hlim=63,
                srh_seg_left=1,
                srh_first_seg=2,
                srh_seg_list=seg_list)

            send_packet(self, swports[1], str(pkt))
            verify_packets(self, exp_pkt, [swports[2]])
            print "Endpoint with penultimate segment poping (PSP) and firstSeg = 2, segLeft = 1"
            seg_list = ['2000::3', '2000::2', '2000::1']
            udp_hdr = UDP(sport=1234, dport=80, chksum=47800)
            pkt = simple_ipv6_sr_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ipv6_dst='2000::2',
                ipv6_src='2000::1',
                ipv6_hlim=64,
                srh_seg_left=1,
                srh_first_seg=2,
                srh_flags=0x00,
                srh_nh=0x11,
                srh_seg_list=seg_list,
                inner_frame=udp_hdr)

            exp_pkt = simple_udpv6_packet(
                pktlen=62,
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='2000::3',
                ipv6_src='2000::1',
                ipv6_hlim=63,
                udp_sport=1234,
                udp_dport=80)

            send_packet(self, swports[1], str(pkt))
            verify_packets(self, exp_pkt, [swports[2]])

            print "Endpoint with firstSeg = 1, segLeft = 0"
            seg_list = ['2000::2', '2000::1']
            pkt = simple_ipv6_sr_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ipv6_dst='2000::2',
                ipv6_src='2000::1',
                ipv6_hlim=64,
                srh_seg_left=0,
                srh_first_seg=1,
                srh_seg_list=seg_list)
            ingress_ifindex = self.client.switch_api_interface_ifindex_get(
                device, if1)
            exp_pkt = simple_cpu_packet(
                header_version=0,
                packet_version=0,
                fabric_color=0,
                fabric_qos=0,
                dst_device=0,
                dst_port_or_group=0,
                ingress_ifindex=ingress_ifindex,
                ingress_bd=0,
                egress_queue=0,
                reason_code=0x400,
                tx_bypass=False,
                ingress_port=1,
                inner_pkt=pkt)

            send_packet(self, swports[1], str(pkt))
            verify_packets(
		self, cpu_packet_mask_ingress_bd(exp_pkt), [self.cpu_port])

            print "Transit packet with firstSeg = 0, segLeft = 0"
            seg_list = ['2000::3']
            pkt = simple_ipv6_sr_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ipv6_dst='2000::3',
                ipv6_src='2000::1',
                ipv6_hlim=64,
                srh_seg_left=0,
                srh_first_seg=0,
                srh_seg_list=seg_list)
            exp_pkt = simple_ipv6_sr_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ipv6_dst='2000::3',
                ipv6_src='2000::1',
                ipv6_hlim=63,
                srh_seg_left=0,
                srh_first_seg=0,
                srh_seg_list=seg_list)

            send_packet(self, swports[1], str(pkt))
            verify_packets(self, exp_pkt, [swports[2]])

        finally:
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

@group('sr')
@group('tunnel')
class L3Srv6TunnelTest(api_base_tests.ThriftInterfaceDataPlane):
    def setUp(self):
        print
        print 'Configuring device for IPv6 SR packet test cases'
        return
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.client.switch_api_init(device)

        self.vrf = self.client.switch_api_vrf_create(device, 2)
        self.rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(
            device, self.rmac, '00:77:66:55:44:33')

        self.port1 = self.client.switch_api_port_id_to_handle_get(
            device, swports[0])
        self.port2 = self.client.switch_api_port_id_to_handle_get(
            device, swports[1])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=self.port1,
            type=SWITCH_INTERFACE_TYPE_PORT,
            rif_handle=self.rif1)
        self.if1 = self.client.switch_api_interface_create(device, i_info1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=self.port2,
            type=SWITCH_INTERFACE_TYPE_PORT,
            rif_handle=self.rif2)
        self.if2 = self.client.switch_api_interface_create(device, i_info2)

        # Create an SR IPv6 tunnel interface
        sid1 = switcht_srv6_segment_t(sid='3ffe::4')
        sid2 = switcht_srv6_segment_t(sid='3ffe::3')
        sid3 = switcht_srv6_segment_t(sid='3ffe::2')
        seg_list = [sid1, sid2, sid3]
        self.src_ip = switcht_ip_addr_t(addr_type=SWITCH_API_IP_ADDR_V6,
                                   ipaddr='3ffe::1',
                                   prefix_length=128)
        self.dst_ip = switcht_ip_addr_t(addr_type=SWITCH_API_IP_ADDR_V6,
                                   ipaddr='3ffe::4',
                                   prefix_length=128)

        # Add a static route to tunnel destination
        tun_nhop_key = switcht_nhop_key_t(intf_handle=self.rif2, ip_addr_valid=0)
        self.tun_nhop = self.client.switch_api_nhop_create(0, tun_nhop_key)
        neighbor_entry1 = switcht_neighbor_info_t(nhop_handle=self.tun_nhop,
                                                 interface_handle=self.rif2,
                                                 mac_addr='00:55:55:55:55:55',
                                                 ip_addr=self.dst_ip,
                                                 rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L3)
        self.neighbor1 = self.client.switch_api_neighbor_entry_add(0, neighbor_entry1)

        self.client.switch_api_l3_route_add(0, self.vrf, self.dst_ip, self.tun_nhop)

        tunnel_info = switcht_tunnel_info_t(
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            tunnel_type=SWITCH_TUNNEL_TYPE_SRV6,
            src_ip=self.src_ip,
            dst_ip=self.dst_ip,
            srv6_seg_list=seg_list,
            egress_rif_handle=self.rif2)
        self.ift1 = self.client.switch_api_tunnel_interface_create(
            device, SWITCH_API_DIRECTION_BOTH, tunnel_info)

        # Create nexthop over SR IPv6 tunnel interface
        nhop_key = switcht_nhop_key_t(intf_handle=self.ift1, ip_addr_valid=0)
        self.nhop1 = self.client.switch_api_nhop_create(device, nhop_key)
        neighbor_entry = switcht_neighbor_info_t(
            nhop_handle=self.nhop1,
            interface_handle=self.ift1,
            mac_addr='00:44:44:44:44:44',
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L3,
            neigh_type=SWITCH_API_NEIGHBOR_IPV6_TUNNEL)
        self.neigh1 = self.client.switch_api_neighbor_entry_add(
            device, neighbor_entry)

        # Add route 2ffe::4/128 => IPV6SR tunnel ift1
        self.ip1 = switcht_ip_addr_t(addr_type=SWITCH_API_IP_ADDR_V6,
                                     ipaddr='2ffe::4',
                                     prefix_length=128)
        self.client.switch_api_l3_route_add(
            device, self.vrf, self.ip1, self.nhop1)

        # Add route 10.10.10.1/32 => IPv6SR tunnel ift1
        self.ip2 = switcht_ip_addr_t(addr_type=SWITCH_API_IP_ADDR_V4,
                                     ipaddr='10.10.10.4',
                                     prefix_length=32)
        self.client.switch_api_l3_route_add(
            device, self.vrf, self.ip2, self.nhop1)

        # Create nexthop over normal interface
        nhop_key = switcht_nhop_key_t(intf_handle=self.if1, ip_addr_valid=0)
        self.nhop2 = self.client.switch_api_nhop_create(device, nhop_key)
        neighbor_entry = switcht_neighbor_info_t(
            nhop_handle=self.nhop2,
            interface_handle=self.rif1,
            mac_addr='00:11:11:11:11:11',
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L3,
            neigh_type=SWITCH_API_NEIGHBOR_NONE)
        self.neigh3 = self.client.switch_api_neighbor_entry_add(
            device, neighbor_entry)

        # Add route 10.20.10.1/32 => if1
        self.ip3 = switcht_ip_addr_t(addr_type=SWITCH_API_IP_ADDR_V4,
                                     ipaddr='10.20.10.1',
                                     prefix_length=32)
        self.client.switch_api_l3_route_add(
            device, self.vrf, self.ip3, self.nhop2)

        # Add route 2000::1/128 => if1
        self.ip4 = switcht_ip_addr_t(addr_type=SWITCH_API_IP_ADDR_V6,
                                     ipaddr='2000::1',
                                     prefix_length=128)
        self.client.switch_api_l3_route_add(
            device, self.vrf, self.ip4, self.nhop2)

    def runTest(self):
        return
        try:
            print "Verifying ipv4 in srv6 (encap)"
            seg_list = ['3ffe::4', '3ffe::3', '3ffe::2']
            pkt = simple_tcp_packet(eth_src='00:11:11:11:11:11',
                                    eth_dst='00:77:66:55:44:33',
                                    ip_dst='10.10.10.4',
                                    ip_src='10.20.10.1',
                                    ip_ttl=64)
            inner_pkt = simple_tcp_packet(ip_dst='10.10.10.4',
                                          ip_src='10.20.10.1',
                                          ip_ttl=63)
            exp_pkt = simple_ipv6_sr_packet(eth_src='00:77:66:55:44:33',
                                            eth_dst='00:55:55:55:55:55',
                                            ipv6_dst='3ffe::2',
                                            ipv6_src='3ffe::1',
                                            ipv6_hlim=64,
                                            srh_seg_left=2,
                                            srh_first_seg=2,
                                            srh_flags=0x00,
                                            srh_nh=0x04,
                                            srh_seg_list=seg_list,
                                            inner_frame=inner_pkt['IP'])

            send_packet(self, swports[0], str(pkt))
            verify_any_packet_any_port(self, exp_pkt, [swports[1]])

            print "Verifying ipv6 in ipv6sr (encap)"
            pkt = simple_tcpv6_packet(eth_src='00:11:11:11:11:11',
                                      eth_dst='00:77:66:55:44:33',
                                      ipv6_dst='2ffe::4',
                                      ipv6_src='2000::1',
                                      ipv6_hlim=64)
            inner_pkt = simple_tcpv6_packet(eth_src='00:77:66:55:44:33',
                                            eth_dst='00:44:44:44:44:44',
                                            ipv6_dst='2ffe::4',
                                            ipv6_src='2000::1',
                                            ipv6_hlim=63)
            exp_pkt = simple_ipv6_sr_packet(eth_src='00:77:66:55:44:33',
                                            eth_dst='00:55:55:55:55:55',
                                            ipv6_dst='3ffe::2',
                                            ipv6_src='3ffe::1',
                                            ipv6_hlim=64,
                                            srh_seg_left=2,
                                            srh_first_seg=2,
                                            srh_flags=0x00,
                                            srh_nh=0x29,
                                            srh_seg_list=seg_list,
                                            inner_frame=inner_pkt['IPv6'])

            send_packet(self, swports[0], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

            print "Verifying ipv6 in ipv6sr (decap)"
            inner_pkt = simple_tcpv6_packet(eth_src='00:44:44:44:44:44',
                                       eth_dst='00:77:66:55:44:33',
                                       ipv6_dst='2000::1',
                                       ipv6_src='2ffe::1',
                                       ipv6_hlim=64)
            seg_list = ['3ffe::4', '3ffe::3', '3ffe::2']
            pkt = simple_ipv6_sr_packet(eth_dst='00:77:66:55:44:33',
                                        eth_src='00:22:22:22:22:22',
                                        ipv6_dst='3ffe::4',
                                        ipv6_src='3ffe::1',
                                        ipv6_hlim=64,
                                        srh_seg_left=0,
                                        srh_first_seg=2,
                                        srh_flags=0x00,
                                        srh_nh=0x29,
                                        srh_seg_list=seg_list,
                                        inner_frame=inner_pkt['IPv6'])

            exp_pkt = simple_tcpv6_packet(eth_src='00:77:66:55:44:33',
                                          eth_dst='00:11:11:11:11:11',
                                          ipv6_dst='2000::1',
                                          ipv6_src='2ffe::1',
                                          ipv6_hlim=63)
            send_packet(self, swports[1], str(pkt))
            verify_packets(self, exp_pkt, [swports[0]])

            print "Verifying ipv4 in ipv6sr (decap)"
            inner_pkt = simple_tcp_packet(eth_src='00:44:44:44:44:44',
                                         eth_dst='00:77:66:55:44:33',
                                         ip_dst='10.20.10.1',
                                         ip_src='10.10.10.1',
                                         ip_id=108,
                                         ip_ttl=64)
            pkt = simple_ipv6_sr_packet(eth_dst='00:77:66:55:44:33',
                                        eth_src='00:22:22:22:22:22',
                                        ipv6_dst='3ffe::4',
                                        ipv6_src='3ffe::1',
                                        ipv6_hlim=64,
                                        srh_seg_left=0,
                                        srh_first_seg=2,
                                        srh_flags=0x00,
                                        srh_nh=0x04,
                                        srh_seg_list=seg_list,
                                        inner_frame=inner_pkt['IP'])
            exp_pkt = simple_tcp_packet(eth_src='00:77:66:55:44:33',
                                     eth_dst='00:11:11:11:11:11',
                                     ip_dst='10.20.10.1',
                                     ip_src='10.10.10.1',
                                     ip_id=108,
                                     ip_ttl=63)
            send_packet(self, swports[1], str(pkt))
            verify_packets(self, exp_pkt, [swports[0]])


        finally:
            self.client.switch_api_l3_route_delete(
                device, self.vrf, self.ip1, self.nhop1)
            self.client.switch_api_l3_route_delete(
                device, self.vrf, self.ip2, self.nhop1)
            self.client.switch_api_l3_route_delete(
                device, self.vrf, self.ip3, self.nhop2)
            self.client.switch_api_l3_route_delete(
                device, self.vrf, self.ip4, self.nhop2)

            self.client.switch_api_neighbor_entry_remove(device, self.neigh1)
            self.client.switch_api_neighbor_entry_remove(device, self.neigh3)

            self.client.switch_api_nhop_delete(device, self.nhop1)
            self.client.switch_api_nhop_delete(device, self.nhop2)

            self.client.switch_api_tunnel_interface_delete(device, self.ift1)

            self.client.switch_api_l3_route_delete(0, self.vrf,
                                                self.dst_ip, self.tun_nhop)
            self.client.switch_api_neighbor_entry_remove(0, self.neighbor1)
            self.client.switch_api_nhop_delete(0, self.tun_nhop)

            self.client.switch_api_interface_delete(device, self.if1)
            self.client.switch_api_interface_delete(device, self.if2)

            self.client.switch_api_rif_delete(0, self.rif1)
            self.client.switch_api_rif_delete(0, self.rif2)

            self.client.switch_api_router_mac_delete(
                device, self.rmac, '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, self.rmac)
            self.client.switch_api_vrf_delete(device, self.vrf)
