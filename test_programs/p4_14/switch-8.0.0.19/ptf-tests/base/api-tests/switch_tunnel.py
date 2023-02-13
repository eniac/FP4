################################################################################
# BAREFOOT NETWORKS CONFIDENTIAL & PROPRIETARY
#
# Copyright (c) 2015-2016 Barefoot Networks, Inc.

# All Rights Reserved.
#
# NOTICE: All information contained herein is, and remains the property of
# Barefoot Networks, Inc. and its suppliers, if any. The intellectual and
# technical concepts contained herein are proprietary to Barefoot Networks, # Inc.
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
import api_base_tests
import pd_base_tests
try:
    import pltfm_pm_rpc
    from pltfm_pm_rpc.ttypes import *
except ImportError:
    pass

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *

import os
import ptf.mask

from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '..'))
from common.utils import *
from common.api_utils import *
from common.api_adapter import ApiAdapter

device = 0
cpu_port = 64

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)

if swports == []:
    swports = [x for x in range(65)]

###############################################################################

@group('l3')
@group('tunnel')
@group('maxsizes')
@group('2porttests')
@group('ent')
@group('mcast')
class L3VxlanUnicastTunnelSMTest(ApiAdapter):
    def runTest(self):
        print
        print "L3 Vxlan test to verify tunnel object state machine"

        # Underlay/Provider Port
        vrf2 = self.client.switch_api_vrf_create(device, 2)

        rmac2 = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac2, '00:AA:BB:CC:DD:EE')
        self.client.switch_api_vrf_rmac_handle_set(device, vrf2, rmac2)

        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf2,
            rmac_handle=rmac2,
            v4_unicast_enabled=True)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        # Overlay/Customer Port
        vrf1 = self.client.switch_api_vrf_create(device, 101)

        rmac1 = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac1, '00:77:66:55:44:33')
        self.client.switch_api_vrf_rmac_handle_set(device, vrf1, rmac1)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf1,
            rmac_handle=rmac1,
            v4_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        imapper_h = self.create_tunnel_mapper(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VNI_TO_VRF_HANDLE,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN)
        imapper_entry_h = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VNI_TO_VRF_HANDLE,
                              mapper=imapper_h,
                              handle=vrf1,
                              vni=0x1234)
        emapper_h = self.create_tunnel_mapper(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VRF_HANDLE_TO_VNI,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN)
        emapper_entry_h = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VRF_HANDLE_TO_VNI,
                              mapper=emapper_h,
                              handle=vrf1,
                              vni=0x1234)

        underlay_lb_h = self.create_loopback_rif(device, vrf2, rmac2)
        overlay_lb_h = self.create_loopback_rif(device, vrf1, rmac1)

        # Case 1: Route to tunnel destination present before tunnel creation
        # Add a static route to tunnel destination
        i_ip3 = switcht_ip_addr_t(ipaddr='1.1.1.3', prefix_length=24)
        tun_nhop, neighbor3 = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip3, '00:44:44:44:44:44')
        self.client.switch_api_l3_route_add(device, vrf2, i_ip3, tun_nhop)

        tunnel_h = self.create_tunnel_table(
                              device=device,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
                              src_ip='1.1.1.3',
                              imapper_h=imapper_h,
                              emapper_h=emapper_h,
                              urif=underlay_lb_h,
                              orif=overlay_lb_h)

        tunnel_term_h = self.create_tunnel_term(
                              device=device,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
                              vrf=vrf2,
                              tunnel=tunnel_h,
                              entry_type=SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P,
                              src_ip='1.1.1.1',
                              dst_ip='1.1.1.3')

        tunnel_if_h = self.create_tunnel_interface(
                              device=device,
                              tunnel=tunnel_h)

        # route for customer packet
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='20.20.20.1',
            prefix_length=32)
        nhop2  = self.add_nhop_tunnel(
                              device,
                              SWITCH_NHOP_TUNNEL_TYPE_VRF,
                              vrf1,
                              tunnel_h,
                              '1.1.1.1',
                              rw_type=SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L3,
                              mac_addr='00:55:55:55:55:55',
                              v4=True)
        self.client.switch_api_l3_route_add(device, vrf1, i_ip2, nhop2)

        try:
            print "Sending packet from Access port1 to Vxlan port2"
            pkt1 = simple_tcp_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:77:66:55:44:33',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            pkt2 = simple_tcp_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:55:55:55:55:55',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63)
            udp_sport = entropy_hash(pkt1)
            vxlan_pkt = simple_vxlan_packet(
                eth_src='00:AA:BB:CC:DD:EE',
                eth_dst='00:44:44:44:44:44',
                ip_id=0,
                ip_dst='1.1.1.1',
                ip_src='1.1.1.3',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt2)
            send_packet(self, swports[0], str(pkt1))
            verify_packet(self, vxlan_pkt, swports[1])

            # Case 2: Remove route to tunnel destination
            self.client.switch_api_l3_route_delete(device, vrf2, i_ip3, tun_nhop)
            print "Sending packet after route remove"
            send_packet(self, swports[0], str(pkt1))
            verify_no_packet(self, vxlan_pkt, swports[1])

            # Case 3: Delete tunnel nhop
            self.client.switch_api_l3_route_delete(device, vrf1, i_ip2, nhop2)
            self.no_nhop(device, nhop2)

            # Case 4: Create tunnel nhop again with no route
            nhop2  = self.add_nhop_tunnel(
                              device,
                              SWITCH_NHOP_TUNNEL_TYPE_VRF,
                              vrf1,
                              tunnel_h,
                              '1.1.1.1',
                              rw_type=SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L3,
                              mac_addr='00:55:55:55:55:55',
                              v4=True)
            self.client.switch_api_l3_route_add(device, vrf1, i_ip2, nhop2)

            print "Sending packet after tunnel create and no route"
            send_packet(self, swports[0], str(pkt1))
            verify_no_packet(self, vxlan_pkt, swports[1])

            time.sleep(1)

            # Case 5: Add route back and verify that packet is routed
            self.client.switch_api_l3_route_add(device, vrf2, i_ip3, tun_nhop)

            print "Sending packet after route add"
            send_packet(self, swports[0], str(pkt1))
            verify_packet(self, vxlan_pkt, swports[1])


        finally:
            self.client.switch_api_l3_route_delete(device, vrf1, i_ip2, nhop2)
            self.client.switch_api_l3_route_delete(device, vrf2, i_ip3, tun_nhop)

            self.client.switch_api_neighbor_delete(device, neighbor3)
            self.client.switch_api_nhop_delete(device, tun_nhop)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.cleanup()

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_router_mac_delete(device, rmac1,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac1)
            self.client.switch_api_vrf_delete(device, vrf1)

            self.client.switch_api_router_mac_delete(device, rmac2,
                                                     '00:AA:BB:CC:DD:EE')
            self.client.switch_api_router_mac_group_delete(device, rmac2)
            self.client.switch_api_vrf_delete(device, vrf2)

###############################################################################

@group('l3')
@group('tunnel')
@group('maxsizes')
@group('2porttests')
@group('ent')
@group('mcast')
class L3VxlanUnicastTunnelSMSVITest(ApiAdapter):
    def runTest(self):
        return
        print
        print "L3 Vxlan test to verify tunnel object state machine with SVI"

        # SVI provider port
        vrf2 = self.client.switch_api_vrf_create(device, 2)

        rmac2 = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac2, '00:AA:BB:CC:DD:EE')
        self.client.switch_api_vrf_rmac_handle_set(device, vrf2, rmac2)

        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        vlan = self.client.switch_api_vlan_create(device, 2)
        self.client.switch_api_vlan_member_add(device, vlan, if2)

        switch_api_mac_table_entry_create(
            self, device, vlan, '00:44:44:44:44:44', 2, if2)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=2,
            vrf_handle=vrf2,
            rmac_handle=rmac2,
            v4_unicast_enabled=1,
            v6_unicast_enabled=1)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='4.4.4.0',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device,
                                                        rif2, vrf2, i_ip1)

        # Overlay/Customer Port
        vrf1 = self.client.switch_api_vrf_create(device, 101)

        rmac1 = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac1, '00:77:66:55:44:33')
        self.client.switch_api_vrf_rmac_handle_set(device, vrf1, rmac1)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf1,
            rmac_handle=rmac1,
            v4_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        imapper_h = self.create_tunnel_mapper(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VNI_TO_VRF_HANDLE,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN)
        imapper_entry_h = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VNI_TO_VRF_HANDLE,
                              mapper=imapper_h,
                              handle=vrf1,
                              vni=0x1234)
        emapper_h = self.create_tunnel_mapper(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VRF_HANDLE_TO_VNI,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN)
        emapper_entry_h = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VRF_HANDLE_TO_VNI,
                              mapper=emapper_h,
                              handle=vrf1,
                              vni=0x1234)

        underlay_lb_h = self.create_loopback_rif(device, vrf2, rmac2)
        overlay_lb_h = self.create_loopback_rif(device, vrf1, rmac1)

        # Case 1: Route to tunnel destination present before tunnel creation
        # Add a static route to tunnel destination
        i_ip3 = switcht_ip_addr_t(ipaddr='1.1.1.1', prefix_length=24)
        tun_nhop, neighbor3 = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip3, '00:44:44:44:44:44')
        self.client.switch_api_l3_route_add(device, vrf2, i_ip3, tun_nhop)

        tunnel_h = self.create_tunnel_table(
                              device=device,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
                              src_ip='1.1.1.3',
                              imapper_h=imapper_h,
                              emapper_h=emapper_h,
                              urif=underlay_lb_h,
                              orif=overlay_lb_h)

        tunnel_if_h = self.create_tunnel_interface(
                              device=device,
                              tunnel=tunnel_h)

        # route for customer packet
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='20.20.20.1',
            prefix_length=32)
        nhop2 = self.add_nhop_tunnel(
                              device,
                              SWITCH_NHOP_TUNNEL_TYPE_VRF,
                              vrf1,
                              tunnel_h,
                              '1.1.1.1',
                              rw_type=SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L3,
                              mac_addr='00:55:55:55:55:55',
                              v4=True)
        self.client.switch_api_l3_route_add(device, vrf1, i_ip2, nhop2)

        try:
            print "Sending packet from Access port1 to Vxlan port2"
            pkt1 = simple_tcp_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:77:66:55:44:33',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            pkt2 = simple_tcp_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:55:55:55:55:55',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63)
            udp_sport = entropy_hash(pkt1)
            vxlan_pkt = simple_vxlan_packet(
                eth_src='00:AA:BB:CC:DD:EE',
                eth_dst='00:44:44:44:44:44',
                ip_id=0,
                ip_dst='1.1.1.1',
                ip_src='1.1.1.3',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt2)
            send_packet(self, swports[0], str(pkt1))
            verify_packet(self, vxlan_pkt, swports[1])

            # Case 2: Remove route to tunnel destination
            self.client.switch_api_l3_route_delete(device, vrf2, i_ip3, tun_nhop)
            time.sleep(2)
            print "Sending packet after route remove"
            send_packet(self, swports[0], str(pkt1))
            verify_no_packet(self, vxlan_pkt, swports[1], timeout=3)

            # Case 3: Remove the MAC address and add tunnel route
            switch_api_mac_table_entry_delete(self, device, vlan, '00:44:44:44:44:44')
            self.client.switch_api_l3_route_add(device, vrf2, i_ip3, tun_nhop)
            time.sleep(2)
            print "Sending packet after MAC remove and tunnel route add"
            send_packet(self, swports[0], str(pkt1))
            verify_no_packet(self, vxlan_pkt, swports[1], timeout=3)
            self.client.switch_api_l3_route_delete(device, vrf2, i_ip3, tun_nhop)

            # Case 4: Add MAC and route
            switch_api_mac_table_entry_create(
                self, device, vlan, '00:44:44:44:44:44', 2, if2)
            self.client.switch_api_l3_route_add(device, vrf2, i_ip3, tun_nhop)
            time.sleep(2)
            print "Sending packet after re-adding MAC and route"
            send_packet(self, swports[0], str(pkt1))
            verify_packet(self, vxlan_pkt, swports[1])

            # Case 5: Delete tunnel nhop
            self.client.switch_api_l3_route_delete(device, vrf1, i_ip2, nhop2)
            self.no_nhop(device, nhop2)

            # Case 6: Create tunnel nhop again with no route
            nhop2  = self.add_nhop_tunnel(
                              device,
                              SWITCH_NHOP_TUNNEL_TYPE_VRF,
                              vrf1,
                              tunnel_h,
                              '1.1.1.1',
                              rw_type=SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L3,
                              mac_addr='00:55:55:55:55:55',
                              v4=True)
            self.client.switch_api_l3_route_add(device, vrf1, i_ip2, nhop2)

            time.sleep(5)
            print "Sending packet after tunnel create and no route"
            send_packet(self, swports[0], str(pkt1))
            verify_no_packet(self, vxlan_pkt, swports[1])

            # Case 7: Add route back and verify that packet is routed
            self.client.switch_api_l3_route_add(device, vrf2, i_ip3, tun_nhop)
            time.sleep(2)

            print "Sending packet after route add"
            send_packet(self, swports[0], str(pkt1))
            verify_packet(self, vxlan_pkt, swports[1])


        finally:
            self.client.switch_api_l3_route_delete(device, vrf1, i_ip2, nhop2)
            self.client.switch_api_l3_route_delete(device, vrf2, i_ip3, tun_nhop)

            self.client.switch_api_neighbor_delete(device, neighbor3)
            self.client.switch_api_nhop_delete(device, tun_nhop)

            switch_api_mac_table_entry_delete(self, device, vlan,
                                                     '00:44:44:44:44:44')

            self.client.switch_api_vlan_member_remove(device, vlan, if2)
            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.cleanup()

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_router_mac_delete(device, rmac1,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac1)
            self.client.switch_api_vrf_delete(device, vrf1)

            self.client.switch_api_router_mac_delete(device, rmac2,
                                                     '00:AA:BB:CC:DD:EE')
            self.client.switch_api_router_mac_group_delete(device, rmac2)
            self.client.switch_api_vrf_delete(device, vrf2)
            self.client.switch_api_vlan_delete(device, vlan)

###############################################################################

@group('l3')
@group('tunnel')
@group('maxsizes')
@group('2porttests')
@group('mcast')
@group('non-vxlan-tunnel')
class L3VxlanUnicastMultiTunnelSMTest(ApiAdapter):
    def runTest(self):
        print
        print "L3 Vxlan test to verify tunnel object state machine"
        print "with multiple tunnels and routes"

        vrf = self.client.switch_api_vrf_create(device, 2)
        vrf1 = self.client.switch_api_vrf_create(device, 10)
        vrf2 = self.client.switch_api_vrf_create(device, 20)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')
        self.client.switch_api_vrf_rmac_handle_set(device, vrf, rmac)
        self.client.switch_api_vrf_rmac_handle_set(device, vrf1, rmac)
        self.client.switch_api_vrf_rmac_handle_set(device, vrf2, rmac)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[2])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf1,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf2,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif3 = self.client.switch_api_rif_create(0, rif_info3)
        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)

        emapper_h1 = self.create_tunnel_mapper(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VRF_HANDLE_TO_VNI,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN)
        emapper_entry_h1 = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VRF_HANDLE_TO_VNI,
                              mapper=emapper_h1,
                              handle=vrf1,
                              vni=0x1234)
        emapper_h2 = self.create_tunnel_mapper(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VRF_HANDLE_TO_VNI,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN)
        emapper_entry_h2 = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VRF_HANDLE_TO_VNI,
                              mapper=emapper_h2,
                              handle=vrf2,
                              vni=0x2345)

        # Case 1: Route to tunnel destination present before tunnel creation
        # Add two static routes to tunnel destination
        i_ip1 = switcht_ip_addr_t(ipaddr='1.1.1.1', prefix_length=32)
        i_ip2 = switcht_ip_addr_t(ipaddr='1.1.1.2', prefix_length=32)
        i_ip3 = switcht_ip_addr_t(ipaddr='1.1.1.7', prefix_length=32)
        tun_nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, rif3, i_ip1, '00:44:44:44:44:44')
        tun_nhop2, neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, rif3, i_ip2, '00:44:44:44:44:44')
        tun_nhop3, neighbor3 = switch_api_l3_nhop_neighbor_create(self, device, rif3, i_ip3, '00:44:44:44:44:44')

        self.client.switch_api_l3_route_add(device, vrf, i_ip1, tun_nhop1)
        self.client.switch_api_l3_route_add(device, vrf, i_ip2, tun_nhop2)
        self.client.switch_api_l3_route_add(device, vrf, i_ip3, tun_nhop3)

        underlay_lb_h = self.create_loopback_rif(device, vrf, rmac)
        overlay_lb_h1 = self.create_loopback_rif(device, vrf1, rmac)
        overlay_lb_h2 = self.create_loopback_rif(device, vrf2, rmac)

        tunnel_h1 = self.create_tunnel_table(
                              device=device,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
                              src_ip='1.1.1.3',
                              emapper_h=emapper_h1,
                              urif=underlay_lb_h,
                              orif=overlay_lb_h1)

        tunnel_if_h1 = self.create_tunnel_interface(
                              device=device,
                              tunnel=tunnel_h1)

        tunnel_h2 = self.create_tunnel_table(
                              device=device,
                              tunnel_type=SWITCH_TUNNEL_TYPE_NVGRE,
                              src_ip='1.1.1.3',
                              emapper_h=emapper_h1,
                              urif=underlay_lb_h,
                              orif=overlay_lb_h1)

        tunnel_if_h2 = self.create_tunnel_interface(
                              device=device,
                              tunnel=tunnel_h2)

        tunnel_h3 = self.create_tunnel_table(
                              device=device,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
                              src_ip='1.1.1.5',
                              emapper_h=emapper_h2,
                              urif=underlay_lb_h,
                              orif=overlay_lb_h2)

        tunnel_if_h3 = self.create_tunnel_interface(
                              device=device,
                              tunnel=tunnel_h3)

        tunnel_h4 = self.create_tunnel_table(
                              device=device,
                              tunnel_type=SWITCH_TUNNEL_TYPE_GENEVE,
                              src_ip='1.1.1.5',
                              emapper_h=emapper_h2,
                              urif=underlay_lb_h,
                              orif=overlay_lb_h2)

        tunnel_if_h4 = self.create_tunnel_interface(
                              device=device,
                              tunnel=tunnel_h4)

        i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='20.20.20.1',
            prefix_length=32)
        nhop2 = self.add_nhop_tunnel(
                              device,
                              SWITCH_NHOP_TUNNEL_TYPE_VRF,
                              vrf1,
                              tunnel_h1,
                              '1.1.1.1',
                              rw_type=SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L3,
                              mac_addr='00:55:55:55:55:55',
                              v4=True)
        self.client.switch_api_l3_route_add(device, vrf1, i_ip4, nhop2)

        i_ip5 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='30.30.30.1',
            prefix_length=32)
        nhop3 = self.add_nhop_tunnel(
                              device,
                              SWITCH_NHOP_TUNNEL_TYPE_VRF,
                              vrf1,
                              tunnel_h2,
                              '1.1.1.1',
                              rw_type=SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L3,
                              mac_addr='00:66:66:66:66:66',
                              v4=True)
        self.client.switch_api_l3_route_add(device, vrf1, i_ip5, nhop3)

        i_ip6 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='40.40.40.1',
            prefix_length=32)
        nhop4 = self.add_nhop_tunnel(
                              device,
                              SWITCH_NHOP_TUNNEL_TYPE_VRF,
                              vrf2,
                              tunnel_h3,
                              '1.1.1.2',
                              rw_type=SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L3,
                              mac_addr='00:77:77:77:77:77',
                              v4=True)
        self.client.switch_api_l3_route_add(device, vrf2, i_ip6, nhop4)

        i_ip7 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='50.50.50.1',
            prefix_length=32)
        nhop5 = self.add_nhop_tunnel(
                              device,
                              SWITCH_NHOP_TUNNEL_TYPE_VRF,
                              vrf2,
                              tunnel_h4,
                              '1.1.1.2',
                              rw_type=SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L3,
                              mac_addr='00:88:88:88:88:88',
                              v4=True)
        self.client.switch_api_l3_route_add(device, vrf2, i_ip7, nhop5)

        try:
            pkt1 = simple_tcp_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:77:66:55:44:33',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            pkt2 = simple_tcp_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:55:55:55:55:55',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63)
            udp_sport1 = entropy_hash(pkt1)
            vxlan_pkt1 = simple_vxlan_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:44:44:44:44:44',
                ip_id=0,
                ip_dst='1.1.1.1',
                ip_src='1.1.1.3',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport1,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt2)
            print "Sending packet from Access port1 to Vxlan port2"
            send_packet(self, swports[0], str(pkt1))
            verify_packet(self, vxlan_pkt1, swports[2])

            pkt3 = simple_tcp_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:77:66:55:44:33',
                ip_dst='30.30.30.1',
                ip_id=108,
                ip_ttl=64)
            pkt4 = simple_tcp_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:66:66:66:66:66',
                ip_dst='30.30.30.1',
                ip_id=108,
                ip_ttl=63)
            nvgre_flowid1 = entropy_hash(pkt3) & 0xFF
            nvgre_pkt1 = simple_nvgre_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:44:44:44:44:44',
                ip_id=0,
                ip_dst='1.1.1.1',
                ip_src='1.1.1.3',
                ip_flags=0x2,
                ip_ttl=64,
                nvgre_tni=0x1234,
                nvgre_flowid=nvgre_flowid1,
                inner_frame=pkt4)
            print "Sending packet from Access port1 to Nvgre port2"
            send_packet(self, swports[0], str(pkt3))
            verify_packet(self, nvgre_pkt1, swports[2])

            pkt5 = simple_tcp_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:77:66:55:44:33',
                ip_dst='40.40.40.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            pkt6 = simple_tcp_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:77:77:77:77:77',
                ip_dst='40.40.40.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63)
            udp_sport2 = entropy_hash(pkt5)
            vxlan_pkt2 = simple_vxlan_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:44:44:44:44:44',
                ip_id=0,
                ip_dst='1.1.1.2',
                ip_src='1.1.1.5',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport2,
                with_udp_chksum=False,
                vxlan_vni=0x2345,
                inner_frame=pkt6)
            print "Sending packet from Access port1 to Vxlan port2"
            send_packet(self, swports[1], str(pkt5))
            verify_packet(self, vxlan_pkt2, swports[2])

            pkt7 = simple_tcp_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:77:66:55:44:33',
                ip_dst='50.50.50.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            pkt8 = simple_tcp_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:88:88:88:88:88',
                ip_dst='50.50.50.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63)
            udp_sport3 = entropy_hash(pkt7)
            geneve_pkt2 = simple_geneve_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:44:44:44:44:44',
                ip_id=0,
                ip_dst='1.1.1.2',
                ip_src='1.1.1.5',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport3,
                with_udp_chksum=False,
                geneve_vni=0x2345,
                inner_frame=pkt8)
            print "Sending packet from Access port1 to Geneve port2"
            send_packet(self, swports[1], str(pkt7))
            verify_packet(self, geneve_pkt2, swports[2])

            # Case 2: Remove routes to tunnel destinations
            self.client.switch_api_l3_route_delete(device, vrf, i_ip1, tun_nhop1)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip2, tun_nhop2)
            print "Sending packets after route remove"
            send_packet(self, swports[0], str(pkt1))
            verify_no_packet(self, vxlan_pkt1, swports[2])
            send_packet(self, swports[0], str(pkt3))
            verify_no_packet(self, nvgre_pkt1, swports[2])
            send_packet(self, swports[1], str(pkt5))
            verify_no_packet(self, vxlan_pkt2, swports[2])
            send_packet(self, swports[2], str(pkt7))
            verify_no_packet(self, geneve_pkt2, swports[2])

            # Case 3: Delete tunnel nhop
            self.client.switch_api_l3_route_delete(device, vrf1, i_ip4, nhop2)
            self.no_nhop(device, nhop2)
            self.client.switch_api_l3_route_delete(device, vrf1, i_ip5, nhop3)
            self.no_nhop(device, nhop3)
            self.client.switch_api_l3_route_delete(device, vrf2, i_ip6, nhop4)
            self.no_nhop(device, nhop4)
            self.client.switch_api_l3_route_delete(device, vrf2, i_ip7, nhop5)
            self.no_nhop(device, nhop5)

            # Case 4: Create tunnel interface again with no route
            nhop2 = self.add_nhop_tunnel(
                                  device,
                                  SWITCH_NHOP_TUNNEL_TYPE_VRF,
                                  vrf1,
                                  tunnel_h1,
                                  '1.1.1.1',
                                  rw_type=SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L3,
                                  mac_addr='00:55:55:55:55:55',
                                  v4=True)
            self.client.switch_api_l3_route_add(device, vrf1, i_ip4, nhop2)

            nhop3 = self.add_nhop_tunnel(
                                  device,
                                  SWITCH_NHOP_TUNNEL_TYPE_VRF,
                                  vrf1,
                                  tunnel_h2,
                                  '1.1.1.1',
                                  rw_type=SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L3,
                                  mac_addr='00:66:66:66:66:66',
                                  v4=True)
            self.client.switch_api_l3_route_add(device, vrf1, i_ip5, nhop3)

            nhop4 = self.add_nhop_tunnel(
                                  device,
                                  SWITCH_NHOP_TUNNEL_TYPE_VRF,
                                  vrf2,
                                  tunnel_h3,
                                  '1.1.1.2',
                                  rw_type=SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L3,
                                  mac_addr='00:77:77:77:77:77',
                                  v4=True)
            self.client.switch_api_l3_route_add(device, vrf2, i_ip6, nhop4)

            nhop5 = self.add_nhop_tunnel(
                                  device,
                                  SWITCH_NHOP_TUNNEL_TYPE_VRF,
                                  vrf2,
                                  tunnel_h4,
                                  '1.1.1.2',
                                  rw_type=SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L3,
                                  mac_addr='00:88:88:88:88:88',
                                  v4=True)
            self.client.switch_api_l3_route_add(device, vrf2, i_ip7, nhop5)

            print "Sending packets after tunnel create and no route"
            send_packet(self, swports[0], str(pkt1))
            verify_no_packet(self, vxlan_pkt1, swports[1])
            send_packet(self, swports[0], str(pkt3))
            verify_no_packet(self, nvgre_pkt1, swports[1])
            send_packet(self, swports[0], str(pkt5))
            verify_no_packet(self, vxlan_pkt2, swports[1])
            send_packet(self, swports[0], str(pkt7))
            verify_no_packet(self, geneve_pkt2, swports[1])

            # Case 5: Add routes back and verify that packets are routed
            self.client.switch_api_l3_route_add(device, vrf, i_ip1, tun_nhop1)
            self.client.switch_api_l3_route_add(device, vrf, i_ip2, tun_nhop2)

            time.sleep(4)
            print "Sending packet after route add"
            send_packet(self, swports[0], str(pkt1))
            verify_packet(self, vxlan_pkt1, swports[2])
            send_packet(self, swports[0], str(pkt3))
            verify_packet(self, nvgre_pkt1, swports[2])
            send_packet(self, swports[1], str(pkt5))
            verify_packet(self, vxlan_pkt2, swports[2])
            send_packet(self, swports[1], str(pkt7))
            verify_packet(self, geneve_pkt2, swports[2])

        finally:
            self.client.switch_api_l3_route_delete(device, vrf, i_ip1, tun_nhop1)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip2, tun_nhop2)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip3, tun_nhop3)

            self.client.switch_api_l3_route_delete(device, vrf1, i_ip4, nhop2)
            self.client.switch_api_l3_route_delete(device, vrf1, i_ip5, nhop3)
            self.client.switch_api_l3_route_delete(device, vrf2, i_ip6, nhop4)
            self.client.switch_api_l3_route_delete(device, vrf2, i_ip7, nhop5)

            self.cleanup()

            self.client.switch_api_neighbor_delete(device, neighbor1)
            self.client.switch_api_nhop_delete(device, tun_nhop1)
            self.client.switch_api_neighbor_delete(device, neighbor2)
            self.client.switch_api_nhop_delete(device, tun_nhop2)
            self.client.switch_api_neighbor_delete(device, neighbor3)
            self.client.switch_api_nhop_delete(device, tun_nhop3)


            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)
            self.client.switch_api_rif_delete(0, rif3)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)
            self.client.switch_api_vrf_delete(device, vrf1)
            self.client.switch_api_vrf_delete(device, vrf2)

###############################################################################

@group('l3')
@group('tunnel')
@group('maxsizes')
@group('2porttests')
@group('ent')
@group('mcast')
class L3VxlanUnicastTunnelECMPSMTest(ApiAdapter):
    def runTest(self):
        print
        print "L3 Vxlan test to verify tunnel object state machine"
        if (test_param_get('target') == 'bmv2'):
            return

        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')
        self.client.switch_api_vrf_rmac_handle_set(device, vrf, rmac)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif3 = self.client.switch_api_rif_create(0, rif_info3)
        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)

        rif_info4 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif4 = self.client.switch_api_rif_create(0, rif_info4)
        i_info4 = switcht_interface_info_t(
            handle=port4, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif4)
        if4 = self.client.switch_api_interface_create(device, i_info4)

        emapper_h = self.create_tunnel_mapper(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VRF_HANDLE_TO_VNI,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN)
        emapper_entry_h = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VRF_HANDLE_TO_VNI,
                              mapper=emapper_h,
                              handle=vrf,
                              vni=0x1234)

        underlay_lb_h = self.create_loopback_rif(device, vrf, rmac)
        overlay_lb_h = self.create_loopback_rif(device, vrf, rmac)

        # Case 1: Route to tunnel destination present before tunnel creation
        # Add a static route to tunnel destination
        i_ip3 = switcht_ip_addr_t(ipaddr='1.1.1.1', prefix_length=24)
        tun_nhop1, neighbor3 = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip3, '00:44:44:44:44:44')
        tun_nhop2, neighbor4 = switch_api_l3_nhop_neighbor_create(self, device, rif3, i_ip3, '00:66:66:66:66:66')
        tun_nhop3, neighbor5 = switch_api_l3_nhop_neighbor_create(self, device, rif4, i_ip3, '00:77:77:77:77:77')
        ecmp = self.client.switch_api_ecmp_create(device)
        self.client.switch_api_ecmp_member_add(device, ecmp, 3, [tun_nhop1, tun_nhop2, tun_nhop3])
        self.client.switch_api_l3_route_add(device, vrf, i_ip3, ecmp)

        # Create a tunnel interface
        tunnel_h = self.create_tunnel_table(
                              device=device,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
                              src_ip='1.1.1.3',
                              imapper_h=0,
                              emapper_h=emapper_h,
                              urif=underlay_lb_h,
                              orif=overlay_lb_h)

        tunnel_if_h = self.create_tunnel_interface(
                              device=device,
                              tunnel=tunnel_h)

        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='20.20.20.1',
            prefix_length=32)
        nhop2 = self.add_nhop_tunnel(
                              device,
                              SWITCH_NHOP_TUNNEL_TYPE_VRF,
                              vrf,
                              tunnel_h,
                              '1.1.1.1',
                              rw_type=SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L3,
                              mac_addr='00:55:55:55:55:55',
                              v4=True)
        self.client.switch_api_l3_route_add(device, vrf, i_ip2, nhop2)

        try:
            print "Sending packet from Access port1 to Vxlan port2"
            pkt1 = simple_tcp_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:77:66:55:44:33',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            pkt2 = simple_tcp_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:55:55:55:55:55',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63)
            udp_sport = entropy_hash(pkt1)
            vxlan_pkt1 = simple_vxlan_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:44:44:44:44:44',
                ip_id=0,
                ip_dst='1.1.1.1',
                ip_src='1.1.1.3',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt2)
            vxlan_pkt2 = simple_vxlan_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:66:66:66:66:66',
                ip_id=0,
                ip_dst='1.1.1.1',
                ip_src='1.1.1.3',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt2)
            vxlan_pkt3 = simple_vxlan_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:77:77:77:77:77',
                ip_id=0,
                ip_dst='1.1.1.1',
                ip_src='1.1.1.3',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt2)
            send_packet(self, swports[0], str(pkt1))

            verify_any_packet_any_port(self,
                [vxlan_pkt1, vxlan_pkt2, vxlan_pkt3],
                [swports[1], swports[2], swports[3]], timeout=2)

            # Case 2: Remove members from ecmp group
            self.client.switch_api_ecmp_member_delete(device, ecmp, 1,
                                    [tun_nhop3])
            send_packet(self, swports[0], str(pkt1))
            verify_any_packet_any_port(self,
                [vxlan_pkt1, vxlan_pkt2],
                [swports[1], swports[2]], timeout=2)

            self.client.switch_api_ecmp_member_delete(device, ecmp, 1,
                                    [tun_nhop2])
            send_packet(self, swports[0], str(pkt1))
            verify_any_packet_any_port(self,
                [vxlan_pkt1],
                [swports[1]], timeout=2)

            self.client.switch_api_ecmp_member_delete(device, ecmp, 1,
                                    [tun_nhop1])
            send_packet(self, swports[0], str(pkt1))
            verify_no_other_packets(self)

            # Case 3: Add the ports back to the ecmp group
            self.client.switch_api_ecmp_member_add(device, ecmp, 1,
                                        [tun_nhop1])
            send_packet(self, swports[0], str(pkt1))
            verify_any_packet_any_port(self,
                [vxlan_pkt1],
                [swports[1]], timeout=2)

            self.client.switch_api_ecmp_member_add(device, ecmp, 1,
                                        [tun_nhop2])
            send_packet(self, swports[0], str(pkt1))
            verify_any_packet_any_port(self,
                [vxlan_pkt1, vxlan_pkt2],
                [swports[1], swports[2]], timeout=2)

            self.client.switch_api_ecmp_member_add(device, ecmp, 1,
                                        [tun_nhop3])
            send_packet(self, swports[0], str(pkt1))
            verify_any_packet_any_port(self,
                [vxlan_pkt1, vxlan_pkt2, vxlan_pkt3],
                [swports[1], swports[2], swports[3]], timeout=2)

            # Case 3: Remove route to tunnel destination
            self.client.switch_api_l3_route_delete(device, vrf, i_ip3, ecmp)
            print "Sending packet after route remove"
            send_packet(self, swports[0], str(pkt1))
            verify_no_other_packets(self)

            # Case 3: Delete tunnel interface
            self.client.switch_api_l3_route_delete(device, vrf, i_ip2, nhop2)
            self.no_nhop(device, nhop2)

            # Case 4: Create tunnel interface again with no route
            nhop2 = self.add_nhop_tunnel(
                              device,
                              SWITCH_NHOP_TUNNEL_TYPE_VRF,
                              vrf,
                              tunnel_h,
                              '1.1.1.1',
                              rw_type=SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L3,
                              mac_addr='00:55:55:55:55:55',
                              v4=True)
            self.client.switch_api_l3_route_add(device, vrf, i_ip2, nhop2)

            print "Sending packet after tunnel create and no route"
            send_packet(self, swports[0], str(pkt1))
            verify_no_other_packets(self)

            # Case 5: Add route back and verify that packet is routed
            self.client.switch_api_l3_route_add(device, vrf, i_ip3, ecmp)

            print "Sending packet after route add"
            send_packet(self, swports[0], str(pkt1))
            verify_any_packet_any_port(self,
                [vxlan_pkt1, vxlan_pkt2, vxlan_pkt3],
                [swports[1], swports[2], swports[3]], timeout=2)


        finally:
            self.client.switch_api_l3_route_delete(device, vrf, i_ip2, nhop2)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip3, ecmp)

            self.client.switch_api_ecmp_member_delete(device, ecmp, 3,
                                        [tun_nhop1, tun_nhop2, tun_nhop3])
            self.client.switch_api_ecmp_delete(device, ecmp)

            self.client.switch_api_neighbor_delete(device, neighbor5)
            self.client.switch_api_nhop_delete(device, tun_nhop3)

            self.client.switch_api_neighbor_delete(device, neighbor4)
            self.client.switch_api_nhop_delete(device, tun_nhop2)

            self.client.switch_api_neighbor_delete(device, neighbor3)
            self.client.switch_api_nhop_delete(device, tun_nhop1)

            self.cleanup()

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)
            self.client.switch_api_rif_delete(0, rif3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)
            self.client.switch_api_interface_delete(device, if4)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)

###############################################################################

@group('l3')
@group('tunnel')
@group('maxsizes')
@group('2porttests')
@group('ent')
@group('mcast')
class L3VxlanUnicastTunnelECMPLagReflectionSMTest(ApiAdapter):
    def runTest(self):
        print
        print "L3 Vxlan test to verify tunnel object state machine"
        if (test_param_get('target') == 'bmv2'):
            return

        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')
        self.client.switch_api_vrf_rmac_handle_set(device, vrf, rmac)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[0])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif3 = self.client.switch_api_rif_create(0, rif_info3)
        lag1 = self.client.switch_api_lag_create(device)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag1, side=SWITCH_API_DIRECTION_BOTH, port=port3)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag1, side=SWITCH_API_DIRECTION_BOTH, port=port4)
        i_info3 = switcht_interface_info_t(
            handle=lag1, type=SWITCH_INTERFACE_TYPE_ACCESS, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)

        emapper_h = self.create_tunnel_mapper(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VRF_HANDLE_TO_VNI,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN)
        emapper_entry_h = self.create_tunnel_mapper_entry(
                              device=device,
                              map_type=SWITCH_TUNNEL_MAP_TYPE_VRF_HANDLE_TO_VNI,
                              mapper=emapper_h,
                              handle=vrf,
                              vni=0x1234)

        underlay_lb_h = self.create_loopback_rif(device, vrf, rmac)
        overlay_lb_h = self.create_loopback_rif(device, vrf, rmac)

        # Case 1: Route to tunnel destination present before tunnel creation
        # Add a static route to tunnel destination
        i_ip3 = switcht_ip_addr_t(ipaddr='1.1.1.1', prefix_length=24)
        tun_nhop1, neighbor3 = switch_api_l3_nhop_neighbor_create(self, device, rif1, i_ip3, '00:44:44:44:44:44')
        tun_nhop2, neighbor4 = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip3, '00:66:66:66:66:66')
        tun_nhop3, neighbor5 = switch_api_l3_nhop_neighbor_create(self, device, rif3, i_ip3, '00:77:77:77:77:77')
        ecmp = self.client.switch_api_ecmp_create(device)
        self.client.switch_api_ecmp_member_add(device, ecmp, 3, [tun_nhop1, tun_nhop2, tun_nhop3])
        self.client.switch_api_l3_route_add(device, vrf, i_ip3, ecmp)

        # Create a tunnel interface
        tunnel_h = self.create_tunnel_table(
                              device=device,
                              tunnel_type=SWITCH_TUNNEL_TYPE_VXLAN,
                              src_ip='1.1.1.3',
                              imapper_h=0,
                              emapper_h=emapper_h,
                              urif=underlay_lb_h,
                              orif=overlay_lb_h)

        tunnel_if_h = self.create_tunnel_interface(
                              device=device,
                              tunnel=tunnel_h)

        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='20.20.20.1',
            prefix_length=32)
        nhop2 = self.add_nhop_tunnel(
                              device,
                              SWITCH_NHOP_TUNNEL_TYPE_VRF,
                              vrf,
                              tunnel_h,
                              '1.1.1.1',
                              rw_type=SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L3,
                              mac_addr='00:55:55:55:55:55',
                              v4=True)
        self.client.switch_api_l3_route_add(device, vrf, i_ip2, nhop2)
        try:
            print "Sending packet from Access port1 to Vxlan port2"
            pkt1 = simple_tcp_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:77:66:55:44:33',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            pkt2 = simple_tcp_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:55:55:55:55:55',
                ip_dst='20.20.20.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63)
            udp_sport = entropy_hash(pkt1)
            vxlan_pkt1 = simple_vxlan_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:44:44:44:44:44',
                ip_id=0,
                ip_dst='1.1.1.1',
                ip_src='1.1.1.3',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt2)
            vxlan_pkt2 = simple_vxlan_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:66:66:66:66:66',
                ip_id=0,
                ip_dst='1.1.1.1',
                ip_src='1.1.1.3',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt2)
            vxlan_pkt3 = simple_vxlan_packet(
                eth_src='00:77:66:55:44:33',
                eth_dst='00:77:77:77:77:77',
                ip_id=0,
                ip_dst='1.1.1.1',
                ip_src='1.1.1.3',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=0x1234,
                inner_frame=pkt2)
            send_packet(self, swports[0], str(pkt1))
            verify_any_packet_any_port(self,
                [vxlan_pkt1, vxlan_pkt2, vxlan_pkt3],
                [swports[0], swports[1], swports[2], swports[3]], timeout=2)

            # Case 2: Remove members from ecmp group
            self.client.switch_api_ecmp_member_delete(device, ecmp, 1,
                                    [tun_nhop3])
            send_packet(self, swports[0], str(pkt1))
            verify_any_packet_any_port(self,
                [vxlan_pkt1, vxlan_pkt2],
                [swports[0], swports[1]], timeout=2)

            #Test reflection on source port
            self.client.switch_api_ecmp_member_delete(device, ecmp, 1,
                                    [tun_nhop2])
            send_packet(self, swports[0], str(pkt1))
            verify_any_packet_any_port(self,
                [vxlan_pkt1],
                [swports[0]], timeout=2)

            self.client.switch_api_ecmp_member_delete(device, ecmp, 1,
                                    [tun_nhop1])
            send_packet(self, swports[0], str(pkt1))
            verify_no_other_packets(self, timeout=4)

            # Case 3: Add the ports back to the ecmp group
            self.client.switch_api_ecmp_member_add(device, ecmp, 1,
                                        [tun_nhop3])
            send_packet(self, swports[0], str(pkt1))
            verify_any_packet_any_port(self,
                [vxlan_pkt3],
                [swports[2], swports[3]], timeout=2)

            # Churn the lag
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag1,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port3)
            send_packet(self, swports[0], str(pkt1))
            verify_packet(self, vxlan_pkt3, swports[3])

            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag1,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port4)
            send_packet(self, swports[0], str(pkt1))
            verify_no_other_packets(self, timeout=4)

            self.client.switch_api_lag_member_add(
            device, lag_handle=lag1, side=SWITCH_API_DIRECTION_BOTH, port=port3)
            send_packet(self, swports[0], str(pkt1))
            verify_packet(self, vxlan_pkt3, swports[2])

            self.client.switch_api_lag_member_add(
            device, lag_handle=lag1, side=SWITCH_API_DIRECTION_BOTH, port=port4)
            send_packet(self, swports[0], str(pkt1))
            verify_any_packet_any_port(self,
                [vxlan_pkt3],
                [swports[2], swports[3]], timeout=2)

            self.client.switch_api_ecmp_member_add(device, ecmp, 1,
                                        [tun_nhop2])
            send_packet(self, swports[0], str(pkt1))
            verify_any_packet_any_port(self,
                [vxlan_pkt2, vxlan_pkt3],
                [swports[1], swports[2], swports[3]], timeout=2)


            self.client.switch_api_ecmp_member_add(device, ecmp, 1,
                                        [tun_nhop1])
            send_packet(self, swports[0], str(pkt1))
            verify_any_packet_any_port(self,
                [vxlan_pkt1, vxlan_pkt2, vxlan_pkt3],
                [swports[0], swports[1], swports[2], swports[3]], timeout=2)

            # Case 3: Remove route to tunnel destination
            self.client.switch_api_l3_route_delete(device, vrf, i_ip3, ecmp)
            print "Sending packet after route remove"
            send_packet(self, swports[0], str(pkt1))
            verify_no_other_packets(self)

            # Case 3: Delete tunnel interface
            self.client.switch_api_l3_route_delete(device, vrf, i_ip2, nhop2)
            self.no_nhop(device, nhop2)

            # Case 4: Create tunnel interface again with no route
            nhop2 = self.add_nhop_tunnel(
                              device,
                              SWITCH_NHOP_TUNNEL_TYPE_VRF,
                              vrf,
                              tunnel_h,
                              '1.1.1.1',
                              rw_type=SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L3,
                              mac_addr='00:55:55:55:55:55',
                              v4=True)
            self.client.switch_api_l3_route_add(device, vrf, i_ip2, nhop2)

            print "Sending packet after tunnel create and no route"
            send_packet(self, swports[0], str(pkt1))
            verify_no_other_packets(self)

            # Case 5: Add route back and verify that packet is routed
            self.client.switch_api_l3_route_add(device, vrf, i_ip3, ecmp)

            print "Sending packet after route add"
            send_packet(self, swports[0], str(pkt1))
            verify_any_packet_any_port(self,
                [vxlan_pkt1, vxlan_pkt2, vxlan_pkt3],
                [swports[0], swports[1], swports[2], swports[3]], timeout=2)


        finally:
            self.client.switch_api_l3_route_delete(device, vrf, i_ip2, nhop2)
            self.client.switch_api_l3_route_delete(device, vrf, i_ip3, ecmp)

            self.client.switch_api_ecmp_member_delete(device, ecmp, 3,
                                        [tun_nhop1, tun_nhop2, tun_nhop3])
            self.client.switch_api_ecmp_delete(device, ecmp)

            self.client.switch_api_neighbor_delete(device, neighbor5)
            self.client.switch_api_nhop_delete(device, tun_nhop3)

            self.client.switch_api_neighbor_delete(device, neighbor4)
            self.client.switch_api_nhop_delete(device, tun_nhop2)

            self.client.switch_api_neighbor_delete(device, neighbor3)
            self.client.switch_api_nhop_delete(device, tun_nhop1)

            self.cleanup()

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)
            self.client.switch_api_rif_delete(0, rif3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag1,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port3)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag1,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port4)

            self.client.switch_api_lag_delete(device, lag1)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)
