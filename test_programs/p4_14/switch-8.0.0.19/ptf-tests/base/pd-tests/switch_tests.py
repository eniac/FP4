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
Thrift PD interface basic tests
"""

import time
import sys
import logging

import unittest
import random

import pd_base_tests

from ptf.testutils import *
from ptf.thriftutils import *

import os

from switch.p4_pd_rpc.ttypes import *

from res_pd_rpc.ttypes import *
from mc_pd_rpc.ttypes import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '..'))
from common.utils import *
from common.pd_utils import *

#global defaults
inner_rmac_group = 1
outer_rmac_group = 2
smac_index = 1
rewrite_index = 1
vrf = 1
rmac = '00:33:33:33:33:33'

g_num_pipes = 1
g_start_mcidx = 2000
g_flood_mcidx = 5000
g_chan_per_port = 1

g_fabric_mcast_device_id = 127
g_fabric_mgid = 3333
g_unicast_fabric_tunnel_rewrite_index = 66
g_multicast_fabric_tunnel_rewrite_index = 77
vlan1 = 10
vlan1_uuc_mc_index = 12345

#Enable features based on p4src/p4feature.h
tunnel_enabled = 1
mc_tunnel_enabled = 0
ipv6_enabled = 1
acl_enabled = 1
multicast_enabled = 1
if test_param_get('target') == 'bmv2':
    stats_enabled = 1
else:
    stats_enabled = 0
int_enabled = 1
learn_timeout = 6


#Basic L2 Test case
@group("L2Test")
@group('l2')
class L2Test(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        sess_hdl = self.conn_mgr.client_init()

        dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))
        device = 0

        client_init(self.client, sess_hdl, dev_tgt)

        #Add the default entries
        populate_default_entries(self.client, sess_hdl, dev_tgt, ipv6_enabled,
                                 acl_enabled, tunnel_enabled, mc_tunnel_enabled, multicast_enabled,
                                 int_enabled)
        ret_init = populate_init_entries(
            self.client, sess_hdl, dev_tgt, rewrite_index, rmac,
            inner_rmac_group, outer_rmac_group, ipv6_enabled, tunnel_enabled)

        #Create two ports
        ret_list = program_ports(self.client, sess_hdl, dev_tgt, 2)

        vlan = 10
        ifindex1 = 1
        ifindex2 = 2
        port1 = 1
        port2 = 2
        v4_enabled = 0
        v6_enabled = 0

        # Add bd entry
        vlan_hdl = program_bd(self.client, sess_hdl, dev_tgt, vlan, 0)

        #Add ports to vlan
        #port vlan able programs (port, vlan) mapping and derives the bd
        hdl1, mbr_hdl1 = program_vlan_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            vrf,
            vlan,
            port1,
            v4_enabled,
            v6_enabled,
            0,
            0,
            ctag=None,
            stag=None)
        if_hdl1 = program_pv_to_ifindex_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            port1,
            ifindex1,
            ctag=None,
            stag=None,
            rid=0)

        hdl2, mbr_hdl2 = program_vlan_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            vrf,
            vlan,
            port2,
            v4_enabled,
            v6_enabled,
            0,
            0,
            ctag=None,
            stag=None)
        if_hdl2 = program_pv_to_ifindex_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            port2,
            ifindex2,
            ctag=None,
            stag=None,
            rid=0)

        #Add static macs to ports. (vlan, mac -> port)
        dmac_hdl1, smac_hdl1 = program_mac(self.client, sess_hdl, dev_tgt, vlan,
                                           '00:11:11:11:11:11', port1, ifindex1)
        dmac_hdl2, smac_hdl2 = program_mac(self.client, sess_hdl, dev_tgt, vlan,
                                           '00:22:22:22:22:22', port2, ifindex2)

        self.conn_mgr.complete_operations(sess_hdl)

        print "Sending packet port 1 -> port 2 on vlan 10 (192.168.0.1 -> 10.0.0.1 [id = 101])"
        pkt = simple_tcp_packet(
            eth_dst='00:22:22:22:22:22',
            eth_src='00:11:11:11:11:11',
            ip_dst='10.0.0.1',
            ip_src='192.168.0.1',
            ip_id=101,
            ip_ttl=64,
            ip_ihl=5)
        try:
            send_packet(self, 1, str(pkt))
            verify_packets(self, pkt, [2])
        finally:
            delete_default_entries(self.client, sess_hdl, device)

            delete_mac(self.client, sess_hdl, device, dmac_hdl2, smac_hdl2)
            delete_mac(self.client, sess_hdl, device, dmac_hdl1, smac_hdl1)

            delete_vlan_mapping(self.client, sess_hdl, device, hdl2, mbr_hdl2)
            delete_vlan_mapping(self.client, sess_hdl, device, hdl1, mbr_hdl1)

            delete_pv_to_ifindex_mapping(self.client, sess_hdl, device, if_hdl1)
            delete_pv_to_ifindex_mapping(self.client, sess_hdl, device, if_hdl2)

            # delete BD
            delete_bd(self.client, sess_hdl, device, vlan_hdl)

            # delete ports
            delete_ports(self.client, sess_hdl, device, 2, ret_list)

            # delete  init and default entries
            delete_init_entries(self.client, sess_hdl, device, ret_init,
                                tunnel_enabled)

            self.conn_mgr.complete_operations(sess_hdl)
            self.conn_mgr.client_cleanup(sess_hdl)


#Basic L3 Test case
@group("IPv4Test")
@group('ipv4')
@group('l3')
class L3Ipv4Test(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print
        sess_hdl = self.conn_mgr.client_init()

        dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))
        device = 0

        client_init(self.client, sess_hdl, dev_tgt)

        #Add the default entries
        populate_default_entries(self.client, sess_hdl, dev_tgt, ipv6_enabled,
                                 acl_enabled, tunnel_enabled, mc_tunnel_enabled, multicast_enabled,
                                 int_enabled)
        ret_init = populate_init_entries(
            self.client, sess_hdl, dev_tgt, rewrite_index, rmac,
            inner_rmac_group, outer_rmac_group, ipv6_enabled, tunnel_enabled)

        #Create two ports
        ret_list = program_ports(self.client, sess_hdl, dev_tgt, 2)

        vlan1 = 10
        vlan2 = 11
        ifindex1 = 1
        ifindex2 = 2
        port1 = 1
        port2 = 2
        v4_enabled = 1
        v6_enabled = 0

        # Add bd entry
        vlan_hdl1 = program_bd(self.client, sess_hdl, dev_tgt, vlan1, 0)
        vlan_hdl2 = program_bd(self.client, sess_hdl, dev_tgt, vlan2, 0)

        #For every L3 port, an implicit vlan will be allocated
        #Add ports to vlan
        #Outer vlan table programs (port, vlan) mapping and derives the bd
        #Inner vlan table derives the bd state
        hdl1, mbr_hdl1 = program_vlan_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            vrf,
            vlan1,
            port1,
            v4_enabled,
            v6_enabled,
            inner_rmac_group,
            0,
            ctag=None,
            stag=None)
        if_hdl1 = program_pv_to_ifindex_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            port1,
            ifindex1,
            ctag=None,
            stag=None)

        hdl2, mbr_hdl2 = program_vlan_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            vrf,
            vlan2,
            port2,
            v4_enabled,
            v6_enabled,
            inner_rmac_group,
            0,
            ctag=None,
            stag=None)
        if_hdl2 = program_pv_to_ifindex_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            port2,
            ifindex2,
            ctag=None,
            stag=None,
            rid=0)

        #Create nexthop
        nhop1 = 1
        nhop_hdl1 = program_nexthop(self.client, sess_hdl, dev_tgt, nhop1,
                                    vlan1, port1, ifindex1, 0)
        #Add rewrite information (ARP info)
        arp_hdl1 = program_ipv4_unicast_rewrite(
            self.client, sess_hdl, dev_tgt, vlan1, nhop1, '00:11:11:11:11:11')
        egress_bd_hdl1 = program_egress_bd_properties(
            self.client, sess_hdl, dev_tgt, vlan1, rewrite_index)
        #Add route
        route_hdl1 = program_ipv4_route(self.client, sess_hdl, dev_tgt, vrf,
                                        0x0a0a0a01, 32, nhop1)
        #Create nexthop
        nhop2 = 2
        nhop_hdl2 = program_nexthop(self.client, sess_hdl, dev_tgt, nhop2,
                                    vlan2, port2, ifindex2, 0)
        #Add rewrite information (ARP info)
        arp_hdl2 = program_ipv4_unicast_rewrite(
            self.client, sess_hdl, dev_tgt, vlan2, nhop2, '00:22:22:22:22:22')
        egress_bd_hdl2 = program_egress_bd_properties(
            self.client, sess_hdl, dev_tgt, vlan2, rewrite_index)
        #Add route
        route_hdl2 = program_ipv4_route(self.client, sess_hdl, dev_tgt, vrf,
                                        0x14141401, 32, nhop2)

        print "Sending packet port 1 -> port 2 (10.10.10.1 -> 20.20.20.1 [id = 101])"
        self.conn_mgr.complete_operations(sess_hdl)

        pkt = simple_tcp_packet(
            eth_dst='00:33:33:33:33:33',
            eth_src='00:11:11:11:11:11',
            ip_dst='20.20.20.1',
            ip_src='10.10.10.1',
            ip_id=101,
            ip_ttl=64,
            ip_ihl=5)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:22:22:22:22:22',
            eth_src='00:33:33:33:33:33',
            ip_dst='20.20.20.1',
            ip_src='10.10.10.1',
            ip_id=101,
            ip_ttl=63,
            ip_ihl=5)
        try:
            send_packet(self, 1, str(pkt))
            verify_packets(self, exp_pkt, [2])
        finally:
            delete_default_entries(self.client, sess_hdl, device)
            delete_egress_bd_properties(self.client, sess_hdl, device,
                                        egress_bd_hdl1)
            delete_egress_bd_properties(self.client, sess_hdl, device,
                                        egress_bd_hdl2)

            delete_ipv4_route(self.client, sess_hdl, device, 32, route_hdl2)
            delete_ipv4_unicast_rewrite(self.client, sess_hdl, device, arp_hdl2)
            delete_nexthop(self.client, sess_hdl, device, nhop_hdl2)

            delete_ipv4_route(self.client, sess_hdl, device, 32, route_hdl1)
            delete_ipv4_unicast_rewrite(self.client, sess_hdl, device, arp_hdl1)
            delete_nexthop(self.client, sess_hdl, device, nhop_hdl1)

            delete_vlan_mapping(self.client, sess_hdl, device, hdl2, mbr_hdl2)
            delete_vlan_mapping(self.client, sess_hdl, device, hdl1, mbr_hdl1)

            delete_pv_to_ifindex_mapping(self.client, sess_hdl, device, if_hdl1)
            delete_pv_to_ifindex_mapping(self.client, sess_hdl, device, if_hdl2)

            delete_bd(self.client, sess_hdl, device, vlan_hdl1)
            delete_bd(self.client, sess_hdl, device, vlan_hdl2)

            # delete ports
            delete_ports(self.client, sess_hdl, device, 2, ret_list)

            # delete  init and default entries
            delete_init_entries(self.client, sess_hdl, device, ret_init,
                                tunnel_enabled)

            self.conn_mgr.complete_operations(sess_hdl)
            self.conn_mgr.client_cleanup(sess_hdl)


@group("IPv6Test")
@group('ipv6')
@group('l3')
class L3Ipv6Test(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print
        if ipv6_enabled == 0:
            print "ipv6 not enabled"
            return

        sess_hdl = self.conn_mgr.client_init()

        dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))
        device = 0

        client_init(self.client, sess_hdl, dev_tgt)

        #Add the default entries
        populate_default_entries(self.client, sess_hdl, dev_tgt, ipv6_enabled,
                                 acl_enabled, tunnel_enabled, mc_tunnel_enabled, multicast_enabled,
                                 int_enabled)
        ret_init = populate_init_entries(
            self.client, sess_hdl, dev_tgt, rewrite_index, rmac,
            inner_rmac_group, outer_rmac_group, ipv6_enabled, tunnel_enabled)

        #Create two ports
        ret_list = program_ports(self.client, sess_hdl, dev_tgt, 2)

        vlan1 = 10
        vlan2 = 11
        ifindex1 = 1
        ifindex2 = 2
        port1 = 1
        port2 = 2
        v4_enabled = 0
        v6_enabled = 1

        # Add bd entry
        vlan_hdl1 = program_bd(self.client, sess_hdl, dev_tgt, vlan1, 0)
        vlan_hdl2 = program_bd(self.client, sess_hdl, dev_tgt, vlan2, 0)

        #For every L3 port, an implicit vlan will be allocated
        #Add ports to vlan
        #Outer vlan table programs (port, vlan) mapping and derives the bd
        #Inner vlan table derives the bd state
        hdl1, mbr_hdl1 = program_vlan_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            vrf,
            vlan1,
            port1,
            v4_enabled,
            v6_enabled,
            inner_rmac_group,
            0,
            ctag=None,
            stag=None)
        if_hdl1 = program_pv_to_ifindex_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            port1,
            ifindex1,
            ctag=None,
            stag=None,
            rid=0)

        hdl2, mbr_hdl2 = program_vlan_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            vrf,
            vlan2,
            port2,
            v4_enabled,
            v6_enabled,
            inner_rmac_group,
            0,
            ctag=None,
            stag=None)
        if_hdl2 = program_pv_to_ifindex_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            port2,
            ifindex2,
            ctag=None,
            stag=None,
            rid=0)

        #Create nexthop
        nhop1 = 1
        nhop_hdl1 = program_nexthop(self.client, sess_hdl, dev_tgt, nhop1,
                                    vlan1, port1, ifindex1, 0)
        #Add rewrite information (ARP info)
        arp_hdl1 = program_ipv6_unicast_rewrite(
            self.client, sess_hdl, dev_tgt, vlan1, nhop1, '00:11:11:11:11:11',
            ipv6_enabled)
        egress_bd_hdl1 = program_egress_bd_properties(
            self.client, sess_hdl, dev_tgt, vlan1, rewrite_index)
        #Add route
        route_hdl1 = program_ipv6_route(self.client, sess_hdl, dev_tgt, vrf,
                                        '2000::1', 128, nhop1, ipv6_enabled)
        #Create nexthop
        nhop2 = 2
        nhop_hdl2 = program_nexthop(self.client, sess_hdl, dev_tgt, nhop2,
                                    vlan2, port2, ifindex2, 0)
        #Add rewrite information (ARP info)
        arp_hdl2 = program_ipv6_unicast_rewrite(
            self.client, sess_hdl, dev_tgt, vlan2, nhop2, '00:22:22:22:22:22',
            ipv6_enabled)
        egress_bd_hdl2 = program_egress_bd_properties(
            self.client, sess_hdl, dev_tgt, vlan2, rewrite_index)
        #Add route
        route_hdl2 = program_ipv6_route(self.client, sess_hdl, dev_tgt, vrf,
                                        '3000::1', 128, nhop2, ipv6_enabled)

        print "Sending packet port 1 -> port 2 (10.10.10.1 -> 20.20.20.1 [id = 101])"
        self.conn_mgr.complete_operations(sess_hdl)

        pkt = simple_tcpv6_packet(
            eth_dst='00:33:33:33:33:33',
            eth_src='00:11:11:11:11:11',
            ipv6_dst='3000::1',
            ipv6_src='2000::1',
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst='00:22:22:22:22:22',
            eth_src='00:33:33:33:33:33',
            ipv6_dst='3000::1',
            ipv6_src='2000::1',
            ipv6_hlim=63)
        try:
            send_packet(self, 1, str(pkt))
            verify_packets(self, exp_pkt, [2])
        finally:

            delete_default_entries(self.client, sess_hdl, device)

            delete_egress_bd_properties(self.client, sess_hdl, device,
                                        egress_bd_hdl1)
            delete_egress_bd_properties(self.client, sess_hdl, device,
                                        egress_bd_hdl2)

            delete_ipv6_route(self.client, sess_hdl, device, 128, route_hdl2,
                              ipv6_enabled)
            delete_ipv6_unicast_rewrite(self.client, sess_hdl, device, arp_hdl2,
                                        ipv6_enabled)
            delete_nexthop(self.client, sess_hdl, device, nhop_hdl2)

            delete_ipv6_route(self.client, sess_hdl, device, 128, route_hdl1,
                              ipv6_enabled)
            delete_ipv6_unicast_rewrite(self.client, sess_hdl, device, arp_hdl1,
                                        ipv6_enabled)
            delete_nexthop(self.client, sess_hdl, device, nhop_hdl1)

            delete_vlan_mapping(self.client, sess_hdl, device, hdl2, mbr_hdl2)
            delete_vlan_mapping(self.client, sess_hdl, device, hdl1, mbr_hdl1)

            delete_pv_to_ifindex_mapping(self.client, sess_hdl, device, if_hdl1)
            delete_pv_to_ifindex_mapping(self.client, sess_hdl, device, if_hdl2)

            delete_bd(self.client, sess_hdl, device, vlan_hdl1)
            delete_bd(self.client, sess_hdl, device, vlan_hdl2)

            # delete ports
            delete_ports(self.client, sess_hdl, device, 2, ret_list)

            # delete  init and default entries
            delete_init_entries(self.client, sess_hdl, device, ret_init,
                                tunnel_enabled)

            self.conn_mgr.complete_operations(sess_hdl)
            self.conn_mgr.client_cleanup(sess_hdl)


#Basic Vxlan Tunneling Test case
@group("TunnelTest")
@group('tunnel')
@group('l2')
class L2VxlanTunnelTest(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print
        print "L2VxlanTunnelTest Skipped"
        return
        if tunnel_enabled == 0:
            print "tunnel not enabled"
            return

        sess_hdl = self.conn_mgr.client_init()

        dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))
        device = 0

        client_init(self.client, sess_hdl, dev_tgt)

        #Add the default entries
        populate_default_entries(self.client, sess_hdl, dev_tgt, ipv6_enabled,
                                 acl_enabled, tunnel_enabled, mc_tunnel_enabled, multicast_enabled,
                                 int_enabled)
        ret_init = populate_init_entries(
            self.client, sess_hdl, dev_tgt, rewrite_index, rmac,
            inner_rmac_group, outer_rmac_group, ipv6_enabled, tunnel_enabled)

        #Create two ports
        ret_list = program_ports(self.client, sess_hdl, dev_tgt, 2)

        ifindex1 = 1
        ifindex2 = 2
        port1 = 1
        port2 = 2
        outer_v4_enabled = 1
        inner_v4_enabled = 0
        outer_v6_enabled = 0
        inner_v6_enabled = 0
        core_vlan = 10
        tenant_vlan = 1000
        vnid = 0x1234
        tunnel_index = 0
        sip_index = 0
        dip_index = 0
        smac_index = 0
        dmac_index = 0
        tunnel_type = 1  #vxlan

        #Indicates vxlan tunnel in Parser
        ingress_tunnel_type = 1
        egress_tunnel_type = 1

        # Add bd entry
        vlan_hdl1 = program_bd(self.client, sess_hdl, dev_tgt, core_vlan, 0)
        vlan_hdl2 = program_bd(self.client, sess_hdl, dev_tgt, tenant_vlan, 0)

        #Port2 belong to core vlan
        #Outer vlan table will derive core bd and the src vtep, dest vtep and vnid will derive the tenant bd
        hdl1, mbr_hdl1 = program_vlan_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            vrf,
            core_vlan,
            port2,
            outer_v4_enabled,
            outer_v6_enabled,
            outer_rmac_group,
            0,
            ctag=None,
            stag=None)
        if_hdl1 = program_pv_to_ifindex_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            port2,
            ifindex2,
            ctag=None,
            stag=None,
            rid=0)
        tun_hdl = program_tunnel_ethernet_vlan(
            self.client, sess_hdl, dev_tgt, vrf, tenant_vlan, port2, vnid,
            ingress_tunnel_type, inner_v4_enabled, 0)

        #Port1 belong to tenant vlan
        #Outer vlan table will derive tenant bd and inner bd table will derive bd state
        hdl2, mbr_hdl2 = program_vlan_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            vrf,
            tenant_vlan,
            port1,
            inner_v4_enabled,
            inner_v6_enabled,
            0,
            0,
            ctag=None,
            stag=None)
        if_hdl2 = program_pv_to_ifindex_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            port1,
            ifindex1,
            ctag=None,
            stag=None,
            rid=0)
        #Add static macs to ports. (vlan, mac -> port)
        #Nextop should be created during mac lookup when the destinaion interface is a tunnel.
        #Nexthop allocated will derive egress bd in the ingress and derive rewrite info
        # at egress
        nhop = 1
        dmac_hdl1, smac_hdl1 = program_mac(self.client, sess_hdl, dev_tgt,
                                           tenant_vlan, '00:11:11:11:11:11',
                                           port1, ifindex1)
        dmac_hdl2, smac_hdl2 = program_mac_with_nexthop(
            self.client, sess_hdl, dev_tgt, tenant_vlan, '00:22:22:22:22:22',
            port2, nhop)

        #add nexthop table
        nhop_hdl1 = program_nexthop(self.client, sess_hdl, dev_tgt, nhop,
                                    tenant_vlan, port2, ifindex2, 1)

        encap1, encap2 = program_tunnel_encap(self.client, sess_hdl, dev_tgt)
        decap1, decap2 = program_tunnel_decap(self.client, sess_hdl, dev_tgt)

        tun_src = program_tunnel_src_ipv4_rewrite(
            self.client, sess_hdl, dev_tgt, sip_index, 0x0a0a0a01)
        tun_dst = program_tunnel_dst_ipv4_rewrite(
            self.client, sess_hdl, dev_tgt, dip_index, 0x0a0a0a02)
        tun_smac = program_tunnel_src_mac_rewrite(
            self.client, sess_hdl, dev_tgt, smac_index, '00:33:33:33:33:33')
        tun_dmac = program_tunnel_dst_mac_rewrite(
            self.client, sess_hdl, dev_tgt, dmac_index, '00:55:55:55:55:55')
        tun_l2 = program_tunnel_l2_unicast_rewrite(self.client, sess_hdl,
                                                   dev_tgt, tunnel_index,
                                                   tunnel_type, nhop, core_vlan)
        tun_rewrite = program_tunnel_rewrite(self.client, sess_hdl, dev_tgt,
                                             tunnel_index, sip_index, dip_index,
                                             smac_index, dmac_index, core_vlan)
        tun_svtep = program_tunnel_ipv4_src_vtep(self.client, sess_hdl, dev_tgt,
                                                 vrf, 0x0a0a0a02, 1, 0)
        tun_dvtep = program_tunnel_ipv4_dst_vtep(self.client, sess_hdl, dev_tgt,
                                                 vrf, 0x0a0a0a01, 1)
        tun_vni = program_egress_vni(self.client, sess_hdl, dev_tgt,
                                     egress_tunnel_type, tenant_vlan, vnid)

        self.conn_mgr.complete_operations(sess_hdl)

        #Egress Tunnel Decap - Decapsulate the vxlan header

        print "Sending packet port 1 -> port 2 - Vxlan tunnel encap"
        print "Inner packet (192.168.10.1 -> 192.168.20.2 [id = 101])"
        print "Outer packet (10.10.10.1 -> 10.10.10.2 [vnid = 0x1234, id = 101])"
        pkt1 = simple_tcp_packet(
            eth_dst='00:22:22:22:22:22',
            eth_src='00:11:11:11:11:11',
            ip_dst='192.168.10.2',
            ip_src='192.168.10.1',
            ip_id=101,
            ip_ttl=64)
        udp_sport = entropy_hash(pkt1)
        vxlan_pkt1 = simple_vxlan_packet(
            eth_dst='00:55:55:55:55:55',
            eth_src='00:33:33:33:33:33',
            ip_id=0,
            ip_dst='10.10.10.2',
            ip_src='10.10.10.1',
            ip_ttl=64,
            ip_flags=0x2,
            udp_sport=udp_sport,
            with_udp_chksum=False,
            vxlan_vni=0x1234,
            inner_frame=pkt1)

        print "Sending packet port 2 -> port 1 - Vxlan tunnel decap"
        print "Inner packet (192.168.10.2 -> 192.168.20.1 [id = 101])"
        print "Outer packet (10.10.10.2 -> 10.10.10.1 [vnid = 0x1234, id = 101])"
        pkt2 = simple_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:22:22:22:22:22',
            ip_dst='192.168.10.1',
            ip_src='192.168.10.2',
            ip_id=101,
            ip_ttl=64)
        vxlan_pkt2 = simple_vxlan_packet(
            eth_dst='00:33:33:33:33:33',
            eth_src='00:55:55:55:55:55',
            ip_id=0,
            ip_dst='10.10.10.1',
            ip_src='10.10.10.2',
            ip_ttl=63,
            ip_flags=0x2,
            udp_sport=4966,
            with_udp_chksum=False,
            vxlan_vni=0x1234,
            inner_frame=pkt2)
        try:
            send_packet(self, 1, str(pkt1))
            verify_packets(self, vxlan_pkt1, [2])
            send_packet(self, 2, str(vxlan_pkt2))
            verify_packets(self, pkt2, [1])
        finally:
            delete_default_entries(self.client, sess_hdl, device)
            delete_egress_vni(self.client, sess_hdl, device, tun_vni)
            delete_tunnel_ipv4_dst_vtep(self.client, sess_hdl, device,
                                        tun_dvtep)
            delete_tunnel_ipv4_src_vtep(self.client, sess_hdl, device,
                                        tun_svtep)
            delete_tunnel_rewrite(self.client, sess_hdl, device, tun_rewrite)
            delete_tunnel_l2_unicast_rewrite(self.client, sess_hdl, device,
                                             tun_l2)
            delete_tunnel_dst_mac_rewrite(self.client, sess_hdl, device,
                                          tun_dmac)
            delete_tunnel_src_mac_rewrite(self.client, sess_hdl, device,
                                          tun_smac)
            delete_tunnel_dst_ipv4_rewrite(self.client, sess_hdl, device,
                                           tun_dst)
            delete_tunnel_src_ipv4_rewrite(self.client, sess_hdl, device,
                                           tun_src)

            delete_tunnel_decap(self.client, sess_hdl, device, decap1, decap2)
            delete_tunnel_encap(self.client, sess_hdl, device, encap1, encap2)

            delete_nexthop(self.client, sess_hdl, device, nhop_hdl1)

            delete_mac(self.client, sess_hdl, device, dmac_hdl2, smac_hdl2)
            delete_mac(self.client, sess_hdl, device, dmac_hdl1, smac_hdl1)

            delete_vlan_mapping(self.client, sess_hdl, device, hdl2, mbr_hdl2)
            delete_tunnel_ethernet_vlan(self.client, sess_hdl, device, tun_hdl)
            delete_vlan_mapping(self.client, sess_hdl, device, hdl1, mbr_hdl1)

            delete_pv_to_ifindex_mapping(self.client, sess_hdl, device, if_hdl1)
            delete_pv_to_ifindex_mapping(self.client, sess_hdl, device, if_hdl2)

            delete_bd(self.client, sess_hdl, device, vlan_hdl1)
            delete_bd(self.client, sess_hdl, device, vlan_hdl2)

            # delete ports
            delete_ports(self.client, sess_hdl, device, 2, ret_list)

            # delete  init and default entries
            delete_init_entries(self.client, sess_hdl, device, ret_init,
                                tunnel_enabled)

            self.conn_mgr.complete_operations(sess_hdl)
            self.conn_mgr.client_cleanup(sess_hdl)


@group("TunnelTest")
@group('tunnel')
@group('l3')
class L3VxlanTunnelTest(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print
        print "L3VxlanTunnelTest Skipped"
        return
        if tunnel_enabled == 0:
            print "tunnel not enabled"
            return
        sess_hdl = self.conn_mgr.client_init()

        dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))
        device = 0

        client_init(self.client, sess_hdl, dev_tgt)

        #Add the default entries
        populate_default_entries(self.client, sess_hdl, dev_tgt, ipv6_enabled,
                                 acl_enabled, tunnel_enabled, mc_tunnel_enabled, multicast_enabled,
                                 int_enabled)
        ret_init = populate_init_entries(
            self.client, sess_hdl, dev_tgt, rewrite_index, rmac,
            inner_rmac_group, outer_rmac_group, ipv6_enabled, tunnel_enabled)

        #Create two ports
        ret_list = program_ports(self.client, sess_hdl, dev_tgt, 2)
        ifindex1 = 1
        ifindex2 = 2
        port1 = 1
        port2 = 2
        outer_v4_enabled = 1
        inner_v4_enabled = 1
        outer_v6_enabled = 0
        inner_v6_enabled = 0
        core_vlan = 10
        tenant_vlan1 = 1000
        tenant_vlan2 = 2000
        vnid = 0x1234
        tunnel_index = 0
        sip_index = 0
        dip_index = 0
        smac_index = 0
        dmac_index = 0

        #Indicates vxlan tunnel in Parser
        ingress_tunnel_type = 1
        egress_tunnel_type = 1

        # Add bd entry
        vlan_hdl1 = program_bd(self.client, sess_hdl, dev_tgt, core_vlan, 0)
        vlan_hdl2 = program_bd(self.client, sess_hdl, dev_tgt, tenant_vlan1, 0)
        vlan_hdl3 = program_bd(self.client, sess_hdl, dev_tgt, tenant_vlan2, 0)

        #Port2 belong to core vlan
        #Outer vlan table will derive core bd and the src vtep, dest vtep and vnid will derive the tenant bd
        hdl1, mbr_hdl1 = program_vlan_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            vrf,
            core_vlan,
            port2,
            outer_v4_enabled,
            outer_v6_enabled,
            outer_rmac_group,
            0,
            ctag=None,
            stag=None)
        if_hdl1 = program_pv_to_ifindex_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            port2,
            ifindex2,
            ctag=None,
            stag=None,
            rid=0)

        tun_hdl = program_tunnel_ipv4_vlan(
            self.client, sess_hdl, dev_tgt, tenant_vlan2, port2, vnid,
            ingress_tunnel_type, inner_v4_enabled, inner_rmac_group, vrf)

        #Port1 belong to tenant vlan
        #Outer vlan table will derive tenant bd and inner bd table will derive bd state
        hdl2, mbr_hdl2 = program_vlan_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            vrf,
            tenant_vlan1,
            port1,
            inner_v4_enabled,
            inner_v6_enabled,
            inner_rmac_group,
            0,
            ctag=None,
            stag=None)
        if_hdl2 = program_pv_to_ifindex_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            port1,
            ifindex1,
            ctag=None,
            stag=None,
            rid=0)
        #Add egress bd properties
        egress_bd_hdl1 = program_egress_bd_properties(
            self.client, sess_hdl, dev_tgt, tenant_vlan1, rewrite_index)
        egress_bd_hdl2 = program_egress_bd_properties(
            self.client, sess_hdl, dev_tgt, tenant_vlan2, rewrite_index)
        #Add L3 routes
        nhop1 = 1
        nhop2 = 2
        route_hdl1 = program_ipv4_route(self.client, sess_hdl, dev_tgt, vrf,
                                        0x0aa80a01, 32, nhop1)
        route_hdl2 = program_ipv4_route(self.client, sess_hdl, dev_tgt, vrf,
                                        0x0aa80b01, 32, nhop2)

        #Add nexthop table
        nhop_hdl1 = program_nexthop(self.client, sess_hdl, dev_tgt, nhop1,
                                    tenant_vlan1, port1, ifindex1, 1)
        arp_hdl1 = program_ipv4_unicast_rewrite(self.client, sess_hdl, dev_tgt,
                                                tenant_vlan1, nhop1,
                                                '00:11:11:11:11:11')

        nhop_hdl2 = program_nexthop(self.client, sess_hdl, dev_tgt, nhop2,
                                    tenant_vlan2, port2, ifindex2, 1)

        encap1, encap2 = program_tunnel_encap(self.client, sess_hdl, dev_tgt)
        decap1, decap2 = program_tunnel_decap(self.client, sess_hdl, dev_tgt)
        tun_src = program_tunnel_src_ipv4_rewrite(
            self.client, sess_hdl, dev_tgt, sip_index, 0x0a0a0a01)
        tun_dst = program_tunnel_dst_ipv4_rewrite(
            self.client, sess_hdl, dev_tgt, dip_index, 0x0a0a0a02)
        tun_smac = program_tunnel_src_mac_rewrite(
            self.client, sess_hdl, dev_tgt, smac_index, '00:33:33:33:33:33')
        tun_dmac = program_tunnel_dst_mac_rewrite(
            self.client, sess_hdl, dev_tgt, dmac_index, '00:55:55:55:55:55')
        tun_l3 = program_tunnel_l3_unicast_rewrite(
            self.client, sess_hdl, dev_tgt, tunnel_index, egress_tunnel_type,
            nhop2, tenant_vlan2, '00:22:22:22:22:22')
        tun_rewrite = program_tunnel_rewrite(self.client, sess_hdl, dev_tgt,
                                             tunnel_index, sip_index, dip_index,
                                             smac_index, dmac_index, core_vlan)
        tun_svtep = program_tunnel_ipv4_src_vtep(self.client, sess_hdl, dev_tgt,
                                                 vrf, 0x0a0a0a02, 1, 0)
        tun_dvtep = program_tunnel_ipv4_dst_vtep(self.client, sess_hdl, dev_tgt,
                                                 vrf, 0x0a0a0a01, 1)
        tun_vni = program_egress_vni(self.client, sess_hdl, dev_tgt,
                                     egress_tunnel_type, tenant_vlan2, vnid)

        self.conn_mgr.complete_operations(sess_hdl)

        print "Sending packet port 1 -> port 2 - Vxlan tunnel encap"
        print "Inner packet (10.168.10.1 -> 10.168.11.1 [id = 101])"
        print "Outer packet (10.10.10.1 -> 10.10.10.2 [vnid = 0x1234, id = 101])"
        pkt1 = simple_tcp_packet(
            eth_dst='00:33:33:33:33:33',
            eth_src='00:11:11:11:11:11',
            ip_dst='10.168.11.1',
            ip_src='10.168.10.1',
            ip_id=101,
            ip_ttl=64)

        pkt2 = simple_tcp_packet(
            eth_dst='00:22:22:22:22:22',
            eth_src='00:33:33:33:33:33',
            ip_dst='10.168.11.1',
            ip_src='10.168.10.1',
            ip_id=101,
            ip_ttl=63)

        udp_sport = entropy_hash(pkt1)
        vxlan_pkt1 = simple_vxlan_packet(
            eth_dst='00:55:55:55:55:55',
            eth_src='00:33:33:33:33:33',
            ip_id=0,
            ip_dst='10.10.10.2',
            ip_src='10.10.10.1',
            ip_ttl=64,
            ip_flags=0x2,
            udp_sport=udp_sport,
            with_udp_chksum=False,
            vxlan_vni=0x1234,
            inner_frame=pkt2)

        print "Sending packet port 2 -> port 1 - Vxlan tunnel decap"
        print "Inner packet (10.168.11.1 -> 10.168.10.1 [id = 101])"
        print "Outer packet (10.10.10.2 -> 10.10.10.1 [vnid = 0x1234, id = 101])"
        pkt3 = simple_tcp_packet(
            eth_dst='00:33:33:33:33:33',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.168.10.1',
            ip_src='10.168.11.1',
            ip_id=101,
            ip_ttl=64)
        vxlan_pkt2 = simple_vxlan_packet(
            eth_dst='00:33:33:33:33:33',
            eth_src='00:55:55:55:55:55',
            ip_id=0,
            ip_dst='10.10.10.1',
            ip_src='10.10.10.2',
            ip_ttl=64,
            ip_flags=0x2,
            udp_sport=14479,
            with_udp_chksum=False,
            vxlan_vni=0x1234,
            inner_frame=pkt3)

        pkt4 = simple_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:33:33:33:33:33',
            ip_dst='10.168.10.1',
            ip_src='10.168.11.1',
            ip_id=101,
            ip_ttl=63)

        try:
            send_packet(self, 1, str(pkt1))
            verify_packets(self, vxlan_pkt1, [2])
            send_packet(self, 2, str(vxlan_pkt2))
            verify_packets(self, pkt4, [1])
        finally:
            delete_default_entries(self.client, sess_hdl, device)
            delete_egress_bd_properties(self.client, sess_hdl, device,
                                        egress_bd_hdl1)
            delete_egress_bd_properties(self.client, sess_hdl, device,
                                        egress_bd_hdl2)
            delete_egress_vni(self.client, sess_hdl, device, tun_vni)
            delete_tunnel_ipv4_dst_vtep(self.client, sess_hdl, device,
                                        tun_dvtep)
            delete_tunnel_ipv4_src_vtep(self.client, sess_hdl, device,
                                        tun_svtep)
            delete_tunnel_rewrite(self.client, sess_hdl, device, tun_rewrite)
            delete_tunnel_l3_unicast_rewrite(self.client, sess_hdl, device,
                                             tun_l3)
            delete_tunnel_dst_mac_rewrite(self.client, sess_hdl, device,
                                          tun_dmac)
            delete_tunnel_src_mac_rewrite(self.client, sess_hdl, device,
                                          tun_smac)
            delete_tunnel_dst_ipv4_rewrite(self.client, sess_hdl, device,
                                           tun_dst)
            delete_tunnel_src_ipv4_rewrite(self.client, sess_hdl, device,
                                           tun_src)

            delete_tunnel_decap(self.client, sess_hdl, device, decap1, decap2)
            delete_tunnel_encap(self.client, sess_hdl, device, encap1, encap2)

            delete_ipv4_route(self.client, sess_hdl, device, 32, route_hdl2)
            delete_nexthop(self.client, sess_hdl, device, nhop_hdl2)

            delete_ipv4_route(self.client, sess_hdl, device, 32, route_hdl1)
            delete_ipv4_unicast_rewrite(self.client, sess_hdl, device, arp_hdl1)
            delete_nexthop(self.client, sess_hdl, device, nhop_hdl1)

            delete_vlan_mapping(self.client, sess_hdl, device, hdl2, mbr_hdl2)
            delete_tunnel_ipv4_vlan(self.client, sess_hdl, device, tun_hdl)
            delete_vlan_mapping(self.client, sess_hdl, device, hdl1, mbr_hdl1)

            delete_pv_to_ifindex_mapping(self.client, sess_hdl, device, if_hdl1)
            delete_pv_to_ifindex_mapping(self.client, sess_hdl, device, if_hdl2)

            # delete BD
            delete_bd(self.client, sess_hdl, device, vlan_hdl3)
            delete_bd(self.client, sess_hdl, device, vlan_hdl2)
            delete_bd(self.client, sess_hdl, device, vlan_hdl1)

            # delete ports
            delete_ports(self.client, sess_hdl, device, 2, ret_list)

            # delete  init and default entries
            delete_init_entries(self.client, sess_hdl, device, ret_init,
                                tunnel_enabled)

            self.conn_mgr.complete_operations(sess_hdl)
            self.conn_mgr.client_cleanup(sess_hdl)


@group("L2Test")
@group('l2')
class L2LearningTest(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        sess_hdl = self.conn_mgr.client_init()

        dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))
        device = 0

        client_init(self.client, sess_hdl, dev_tgt)

        #Add the default entries
        populate_default_entries(self.client, sess_hdl, dev_tgt, ipv6_enabled,
                                 acl_enabled, tunnel_enabled, mc_tunnel_enabled, multicast_enabled,
                                 int_enabled)
        ret_init = populate_init_entries(
            self.client, sess_hdl, dev_tgt, rewrite_index, rmac,
            inner_rmac_group, outer_rmac_group, ipv6_enabled, tunnel_enabled)

        #Create two ports
        ret_list = program_ports(self.client, sess_hdl, dev_tgt, 2)

        vlan = 10
        ifindex1 = 1
        ifindex2 = 2
        port1 = 1
        port2 = 2
        v4_enabled = 0
        v6_enabled = 0

        # Add bd entry
        vlan_hdl = program_bd(self.client, sess_hdl, dev_tgt, vlan, 0)

        #Add ports to vlan
        #Outer vlan table programs (port, vlan) mapping and derives the bd
        #Inner vlan table derives the bd state
        hdl1, mbr_hdl1 = program_vlan_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            vrf,
            vlan,
            port1,
            v4_enabled,
            v6_enabled,
            0,
            1,
            ctag=None,
            stag=None)
        if_hdl1 = program_pv_to_ifindex_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            port1,
            ifindex1,
            ctag=None,
            stag=None,
            rid=0)

        hdl2, mbr_hdl2 = program_vlan_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            vrf,
            vlan,
            port2,
            v4_enabled,
            v6_enabled,
            0,
            1,
            ctag=None,
            stag=None)
        if_hdl2 = program_pv_to_ifindex_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            port2,
            ifindex2,
            ctag=None,
            stag=None,
            rid=0)

        dmac_hdl1, smac_hdl1 = program_mac(self.client, sess_hdl, dev_tgt, vlan,
                                           '00:44:44:44:44:44', port2, ifindex2)

        enable_learning(self.client, sess_hdl, dev_tgt)

        self.client.set_learning_timeout(sess_hdl, 0, learn_timeout * 1000)
        self.client.mac_learn_digest_register(sess_hdl, 0)

        self.conn_mgr.complete_operations(sess_hdl)

        pkt = simple_tcp_packet(
            eth_dst='00:44:44:44:44:44',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.168.10.1',
            ip_src='10.168.11.1',
            ip_id=101,
            ip_ttl=64)
        try:
            send_packet(self, 1, str(pkt))
            time.sleep(learn_timeout + 1)
            digests = self.client.mac_learn_digest_get_digest(sess_hdl)
            assert len(digests.msg) == 1
            mac_str = digests.msg[0].l2_metadata_lkp_mac_sa
            print "new mac learnt ", mac_str,
            print "on port ", digests.msg[0].ingress_metadata_ifindex
        finally:
            delete_default_entries(self.client, sess_hdl, device)
            self.client.mac_learn_digest_digest_notify_ack(sess_hdl,
                                                           digests.msg_ptr)
            self.client.mac_learn_digest_deregister(sess_hdl, 0)

            delete_mac(self.client, sess_hdl, device, dmac_hdl1, smac_hdl1)

            delete_vlan_mapping(self.client, sess_hdl, device, hdl2, mbr_hdl2)
            delete_vlan_mapping(self.client, sess_hdl, device, hdl1, mbr_hdl1)

            delete_pv_to_ifindex_mapping(self.client, sess_hdl, device, if_hdl1)
            delete_pv_to_ifindex_mapping(self.client, sess_hdl, device, if_hdl2)

            delete_bd(self.client, sess_hdl, device, vlan_hdl)

            # delete ports
            delete_ports(self.client, sess_hdl, device, 2, ret_list)

            # delete  init and default entries
            delete_init_entries(self.client, sess_hdl, device, ret_init,
                                tunnel_enabled)

            self.conn_mgr.complete_operations(sess_hdl)
            self.conn_mgr.client_cleanup(sess_hdl)


@group("L2FloodTest")
@group('l2')
class L2FloodTest(pd_base_tests.ThriftInterfaceDataPlane):
    def __str__(self):
        return self.id()

    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        # skip test for BMV2
        if test_param_get('target') == 'bmv2':
            print "Skipping test for BMV2"
            return

        sess_hdl = self.conn_mgr.client_init()

        mc_sess_hdl = self.mc.mc_create_session()
        dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))
        device = 0

        client_init(self.client, sess_hdl, dev_tgt)

        #Add the default entries
        populate_default_entries(self.client, sess_hdl, dev_tgt, ipv6_enabled,
                                 acl_enabled, tunnel_enabled, mc_tunnel_enabled, multicast_enabled,
                                 int_enabled)
        ret_init = populate_init_entries(
            self.client, sess_hdl, dev_tgt, rewrite_index, rmac,
            inner_rmac_group, outer_rmac_group, ipv6_enabled, tunnel_enabled)

        #Create ports
        ret_list = program_ports(self.client, sess_hdl, dev_tgt, 4)

        ports = [1, 2, 3, 4]
        ifindex = ports
        rid = ports

        vlan = 10
        v4_enabled = 0
        v6_enabled = 0
        mgid = 0x100

        # Add bd entry
        vlan_hdl = program_bd(self.client, sess_hdl, dev_tgt, vlan, mgid)

        #Add ports to vlan
        hdl = {}
        mbr_hdl = {}
        if_hdl = {}
        node_hdl = {}

        mgrp_hdl = self.mc.mc_mgrp_create(mc_sess_hdl, 0, mgid)

        for i in range(4):
            hdl[i], mbr_hdl[i] = program_vlan_mapping(
                self.client,
                sess_hdl,
                dev_tgt,
                vrf,
                vlan,
                ports[i],
                v4_enabled,
                v6_enabled,
                0,
                0,
                ctag=None,
                stag=None)

            if_hdl[i] = program_pv_to_ifindex_mapping(
                self.client,
                sess_hdl,
                dev_tgt,
                ports[i],
                ifindex[i],
                ctag=None,
                stag=None,
                rid=rid[i])

            prune_port_map = set_port_or_lag_bitmap(288, [ports[i]])
            self.mc.mc_update_port_prune_table(mc_sess_hdl, 0, ports[i],
                                               prune_port_map)

            port_map = set_port_or_lag_bitmap(288, [ports[i]])
            lag_map = set_port_or_lag_bitmap(256, [])
            node_hdl[i] = self.mc.mc_node_create(mc_sess_hdl, 0, rid[i],
                                                 port_map, lag_map)

            self.mc.mc_associate_node(mc_sess_hdl, 0, mgrp_hdl, node_hdl[i],
                                      ports[i], 1)

        self.mc.mc_complete_operations(mc_sess_hdl)
        self.conn_mgr.complete_operations(sess_hdl)

        pkt = simple_tcp_packet(
            eth_dst='00:44:44:44:44:44',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.168.10.1',
            ip_src='10.168.11.1',
            ip_id=101,
            ip_ttl=64)

        try:

            rx_ports = ports
            tx_port = rx_ports.pop()

            send_packet(self, tx_port, str(pkt))
            verify_packets(self, pkt, ports=rx_ports)
        finally:
            delete_default_entries(self.client, sess_hdl, device)

            # delete port_vlan entries
            for i in range(4):
                self.mc.mc_dissociate_node(mc_sess_hdl, device, mgrp_hdl,
                                           node_hdl[i])
                self.mc.mc_node_destroy(mc_sess_hdl, device, node_hdl[i])

                delete_vlan_mapping(self.client, sess_hdl, device, hdl[i],
                                    mbr_hdl[i])
                delete_pv_to_ifindex_mapping(self.client, sess_hdl, device,
                                             if_hdl[i])

            self.mc.mc_mgrp_destroy(mc_sess_hdl, device, mgrp_hdl)

            # delete BD
            delete_bd(self.client, sess_hdl, device, vlan_hdl)

            # delete ports
            delete_ports(self.client, sess_hdl, device, 4, ret_list)

            # delete  init and default entries
            delete_init_entries(self.client, sess_hdl, device, ret_init,
                                tunnel_enabled)

            self.mc.mc_destroy_session(mc_sess_hdl)
            self.conn_mgr.complete_operations(sess_hdl)
            self.conn_mgr.client_cleanup(sess_hdl)


#@group('l2')
#class L2QinQTest(pd_base_tests.ThriftInterfaceDataPlane):
#    def __init__(self):
#        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
#                                                        ["dc"])
#
#    def runTest(self):
#        print
#        sess_hdl = self.conn_mgr.client_init()
#
#        dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))
#        device = 0
#
#        client_init(self.client, sess_hdl, dev_tgt)
#
#        #Add the default entries
#        populate_default_entries(self.client, sess_hdl, dev_tgt, ipv6_enabled,
#                                 acl_enabled, tunnel_enabled, mc_tunnel_enabled, multicast_enabled,
#                                 int_enabled)
#        ret_init = populate_init_entries(
#            self.client, sess_hdl, dev_tgt, rewrite_index, rmac,
#            inner_rmac_group, outer_rmac_group, ipv6_enabled, tunnel_enabled)
#
#        #Create two ports
#        ret_list = program_ports(self.client, sess_hdl, dev_tgt, 2)
#
#        vlan = 10
#        ifindex1 = 1
#        ifindex2 = 2
#        port1 = 1
#        port2 = 2
#        v4_enabled = 0
#        v6_enabled = 0
#
#        # Add bd entry
#        vlan_hdl = program_bd(self.client, sess_hdl, dev_tgt, vlan, 0)
#
#        #Add ports to vlan
#        #port vlan able programs (port, vlan) mapping and derives the bd
#        hdl1, mbr_hdl1 = program_vlan_mapping(
#            self.client,
#            sess_hdl,
#            dev_tgt,
#            vrf,
#            vlan,
#            port1,
#            v4_enabled,
#            v6_enabled,
#            0,
#            0,
#            ctag=10,
#            stag=20)
#        if_hdl1 = program_pv_to_ifindex_mapping(
#            self.client,
#            sess_hdl,
#            dev_tgt,
#            port1,
#            ifindex1,
#            ctag=None,
#            stag=None,
#            rid=0)
#
#        hdl2, mbr_hdl2 = program_vlan_mapping(
#            self.client,
#            sess_hdl,
#            dev_tgt,
#            vrf,
#            vlan,
#            port2,
#            v4_enabled,
#            v6_enabled,
#            0,
#            0,
#            ctag=None,
#            stag=None)
#        if_hdl2 = program_pv_to_ifindex_mapping(
#            self.client,
#            sess_hdl,
#            dev_tgt,
#            port2,
#            ifindex2,
#            ctag=None,
#            stag=None,
#            rid=0)
#
#        xlate_hdl = program_egress_vlan_xlate(
#            self.client, sess_hdl, dev_tgt, port1, 10, ctag=10, stag=20)
#
#        #Add static macs to ports. (vlan, mac -> port)
#        dmac_hdl1, smac_hdl1 = program_mac(self.client, sess_hdl, dev_tgt, vlan,
#                                           '00:11:11:11:11:11', port1, ifindex1)
#        dmac_hdl2, smac_hdl2 = program_mac(self.client, sess_hdl, dev_tgt, vlan,
#                                           '00:22:22:22:22:22', port2, ifindex2)
#
#        self.conn_mgr.complete_operations(sess_hdl)
#
#        pkt = simple_qinq_tcp_packet(
#            eth_dst='00:22:22:22:22:22',
#            eth_src='00:11:11:11:11:11',
#            dl_vlan_outer=20,
#            dl_vlan_pcp_outer=0,
#            dl_vlan_cfi_outer=0,
#            vlan_vid=10,
#            vlan_pcp=0,
#            dl_vlan_cfi=0,
#            ip_dst='10.0.0.1',
#            ip_src='192.168.0.1',
#            ip_ttl=64,
#            pktlen=100)
#        exp_pkt = simple_tcp_packet(
#            eth_dst='00:22:22:22:22:22',
#            eth_src='00:11:11:11:11:11',
#            ip_dst='10.0.0.1',
#            ip_src='192.168.0.1',
#            ip_ttl=64,
#            pktlen=100 - 8)
#        pkt[Ether].type = 0x9100
#
#        pkt2 = simple_tcp_packet(
#            eth_dst='00:11:11:11:11:11',
#            eth_src='00:22:22:22:22:22',
#            ip_dst='10.0.0.1',
#            ip_src='192.168.0.1',
#            ip_ttl=64,
#            pktlen=100 - 8)
#        exp_pkt2 = simple_qinq_tcp_packet(
#            eth_dst='00:11:11:11:11:11',
#            eth_src='00:22:22:22:22:22',
#            dl_vlan_outer=20,
#            dl_vlan_pcp_outer=0,
#            dl_vlan_cfi_outer=0,
#            vlan_vid=10,
#            vlan_pcp=0,
#            dl_vlan_cfi=0,
#            ip_dst='10.0.0.1',
#            ip_src='192.168.0.1',
#            ip_ttl=64,
#            pktlen=100)
#        exp_pkt2[Ether].type = 0x9100
#        try:
#            print "Sending packet port 1 (QinQ) -> port 2 (Untagged)"
#            send_packet(self, 1, str(pkt))
#            verify_packets(self, exp_pkt, [2])
#            print "Sending packet port 2 (Untagged) -> port 2 (QinQ)"
#            send_packet(self, 2, str(pkt2))
#            verify_packets(self, exp_pkt2, [1])
#        finally:
#            delete_default_entries(self.client, sess_hdl, device)
#            delete_mac(self.client, sess_hdl, device, dmac_hdl2, smac_hdl2)
#            delete_mac(self.client, sess_hdl, device, dmac_hdl1, smac_hdl1)
#
#            delete_egress_vlan_xlate(self.client, sess_hdl, device, xlate_hdl)
#
#            delete_vlan_mapping(self.client, sess_hdl, device, hdl2, mbr_hdl2)
#            delete_vlan_mapping(self.client, sess_hdl, device, hdl1, mbr_hdl1)
#
#            delete_pv_to_ifindex_mapping(self.client, sess_hdl, device, if_hdl1)
#            delete_pv_to_ifindex_mapping(self.client, sess_hdl, device, if_hdl2)
#
#            # delete BD
#            delete_bd(self.client, sess_hdl, device, vlan_hdl)
#
#            # delete ports
#            delete_ports(self.client, sess_hdl, device, 2, ret_list)
#
#            # delete  init and default entries
#            delete_init_entries(self.client, sess_hdl, device, ret_init,
#                                tunnel_enabled)
#
#            self.conn_mgr.complete_operations(sess_hdl)
#            self.conn_mgr.client_cleanup(sess_hdl)
#
#
@group("emulator_test")
class EmulatorTest(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def setUp(self):
        print
        print 'Configuring the devices'

        # skip test for BMV2
        if test_param_get('target') == 'bmv2':
            print "Skipping test for BMV2"
            return

        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.sess_hdl = self.conn_mgr.client_init()

        self.dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))
        self.device = 0

        client_init(self.client, self.sess_hdl, self.dev_tgt)

        populate_default_entries(self.client, self.sess_hdl, self.dev_tgt,
                                 ipv6_enabled, acl_enabled, tunnel_enabled,
                                 mc_tunnel_enabled, multicast_enabled, int_enabled)
        self.ret_init = populate_init_entries(
            self.client, self.sess_hdl, self.dev_tgt, rewrite_index, rmac,
            inner_rmac_group, outer_rmac_group, ipv6_enabled, tunnel_enabled)

        j = 0
        portno = []
        for pipe in range(0, g_num_pipes):
            for port in range(0, 72):
                j += 1
                portno.append(j)

        # create ports
        self.ret_list = program_emulation_ports(self.client, self.sess_hdl,
                                                self.dev_tgt, j)

        # untagged vlan
        self.vlan = 5
        # tagged vlans
        self.trunk_vlans = [10]
        v4_enabled = 0
        v6_enabled = 0
        mgid = g_flood_mcidx
        rid = 0x200
        xid = portno[1]

        self.vlan_hdl = program_bd(self.client, self.sess_hdl, self.dev_tgt,
                                   self.vlan, mgid)

        self.trunk_vlan_hdl = {}
        self.trunk_vlan_ehdl = {}
        for i in self.trunk_vlans:
            self.trunk_vlan_hdl[i] = program_bd(self.client, self.sess_hdl,
                                                self.dev_tgt, i, mgid)
            self.trunk_vlan_ehdl[i] = \
                program_egress_bd_properties(self.client, self.sess_hdl,self.dev_tgt, i, rewrite_index)

        self.mc_sess_hdl = self.mc.mc_create_session()
        init_pre(self.mc, self.mc_sess_hdl, g_num_pipes, g_start_mcidx,
                 g_chan_per_port, g_flood_mcidx)
        #init_mac_table(self.client, self.sess_hdl)
        #init_qos_table(self.client, self.sess_hdl)

        self.hdl = {}
        self.vhdl = {}
        self.xlate_hdl = {}
        self.mbr_hdl = {}
        self.vmbr_hdl = {}
        self.dhdl = {}
        self.shdl = {}
        self.mc_dmac_hdl = {}
        self.if_hdl = {}
        self.vif_hdl = {}

        for pipe in range(0, g_num_pipes):
            for port in range(0, 72):
                asic_port = pipe_port_to_asic_port(pipe, port)
                ifindex = asic_port
                self.hdl[asic_port], self.mbr_hdl[asic_port] = \
                            program_vlan_mapping(self.client, self.sess_hdl,self.dev_tgt, vrf, self.vlan, asic_port,v4_enabled, v6_enabled,0, 0, ctag=None, stag=None)
                self.if_hdl[asic_port] = program_pv_to_ifindex_mapping(
                    self.client,
                    self.sess_hdl,
                    self.dev_tgt,
                    asic_port,
                    ifindex,
                    ctag=None,
                    stag=None,
                    rid=0)

                mac_addr = '00:00:00:01:' + hex(pipe)[2:].zfill(2)
                mac_addr = mac_addr + ':' + hex(port)[2:].zfill(2)
                self.dhdl[asic_port], self.shdl[asic_port] = \
                            program_mac(self.client, self.sess_hdl,self.dev_tgt, self.vlan,mac_addr, asic_port+1, ifindex)

                mac_addr = '01:00:5e:00:' + hex(pipe)[2:].zfill(2)
                mac_addr = mac_addr + ':' + hex(port)[2:].zfill(2)
                self.mc_dmac_hdl[asic_port] = \
                            program_multicast_mac(self.client, self.sess_hdl,self.dev_tgt, self.vlan,mac_addr, asic_port,g_start_mcidx)
                for i in self.trunk_vlans:
                    self.vhdl[asic_port], self.vmbr_hdl[asic_port] = \
                        program_vlan_mapping(self.client, self.sess_hdl,self.dev_tgt, vrf, i, asic_port,v4_enabled, v6_enabled, 0, 0,ctag=None, stag=i)
                    self.vif_hdl[asic_port] = \
                        program_pv_to_ifindex_mapping(self.client, self.sess_hdl,self.dev_tgt, asic_port, ifindex,ctag=None, stag=i, rid=0)
                    self.xlate_hdl[asic_port] = \
                        program_egress_vlan_xlate(self.client, self.sess_hdl,self.dev_tgt, asic_port,i, ctag=i, stag=None)
                    mac_addr = '0000' + hex(i)[2:].zfill(4) + hex(pipe)[
                        2:].zfill(2)
                    mac_addr = ':'.join(
                        s.encode('hex') for s in mac_addr.decode('hex'))
                    mac_addr = mac_addr + ':' + hex(port)[2:].zfill(2)
                    self.dhdl[asic_port], self.shdl[asic_port] = \
                            program_mac(self.client, self.sess_hdl,self.dev_tgt, i,mac_addr, asic_port+1, ifindex)
                    mac_addr = '01005e' + hex(i)[2:].zfill(3) + \
                                hex(pipe)[2:].zfill(1) + hex(port)[2:].zfill(2)
                    mac_addr = ':'.join(
                        s.encode('hex') for s in mac_addr.decode('hex'))
                    self.mc_dmac_hdl[asic_port] = \
                            program_multicast_mac(self.client, self.sess_hdl,self.dev_tgt, i,mac_addr, asic_port,g_start_mcidx)

        self.mc.mc_complete_operations(self.mc_sess_hdl)
        self.conn_mgr.complete_operations(self.sess_hdl)

    def runTest(self):
        """
        pass

        """
        # skip test for BMV2
        if test_param_get('target') == 'bmv2':
            print "Skipping test for BMV2"
            return

        print
        print 'Running test'
        for pipe in range(0, g_num_pipes):
            for port in range(0, 8, 4):

                exp_asic_port = pipe_port_to_asic_port(pipe, port)

                if exp_asic_port == 0:
                    continue

                # unicast packet
                print "Untagged unicast packet test"
                mac_addr = '00:00:00:01:' + hex(pipe)[2:].zfill(2)
                mac_addr = mac_addr + ':' + hex(port)[2:].zfill(2)
                pkt = simple_tcp_packet(
                    eth_dst=mac_addr,
                    eth_src='00:00:00:00:01:01',
                    ip_dst='10.10.3.3',
                    ip_src='10.10.1.1',
                    ip_id=105,
                    ip_ttl=4)
                exp_pkt = simple_tcp_packet(
                    eth_dst=mac_addr,
                    eth_src='00:00:00:00:01:01',
                    ip_dst='10.10.3.3',
                    ip_src='10.10.1.1',
                    ip_id=105,
                    ip_ttl=4)
                send_packet(self, 0, str(pkt))
                verify_packets(self, exp_pkt, [exp_asic_port])

                # multicast packet
                print "Untagged multicast packet test"
                mac_addr = '01:00:5e:00:' + hex(pipe)[2:].zfill(2)
                mac_addr = mac_addr + ':' + hex(port)[2:].zfill(2)
                pkt = simple_tcp_packet(
                    eth_dst=mac_addr,
                    eth_src='00:00:00:00:01:01',
                    ip_dst='10.10.3.3',
                    ip_src='10.10.1.1',
                    ip_id=105,
                    ip_ttl=4)
                exp_pkt = simple_tcp_packet(
                    eth_dst=mac_addr,
                    eth_src='00:00:00:00:01:01',
                    ip_dst='10.10.3.3',
                    ip_src='10.10.1.1',
                    ip_id=105,
                    ip_ttl=4)
                send_packet(self, 0, str(pkt))
                verify_packets(self, exp_pkt, [exp_asic_port])

                # tagged packets
                for i in self.trunk_vlans:
                    print "Tagged unicast packet vlan %d" % i
                    mac_addr = '0000' + hex(i)[2:].zfill(4) + hex(pipe)[
                        2:].zfill(2)
                    mac_addr = ':'.join(
                        s.encode('hex') for s in mac_addr.decode('hex'))
                    mac_addr = mac_addr + ':' + hex(port)[2:].zfill(2)
                    src_mac_addr = '0000' + hex(i)[2:].zfill(4) + \
                                   hex(pipe)[2:].zfill(2) + '00'
                    src_mac_addr = ':'.join(
                        s.encode('hex') for s in src_mac_addr.decode('hex'))

                    pkt = simple_tcp_packet(
                        eth_dst=mac_addr,
                        eth_src=src_mac_addr,
                        ip_dst='10.10.3.3',
                        dl_vlan_enable=True,
                        vlan_vid=i,
                        ip_id=102,
                        ip_ttl=64)
                    exp_pkt = simple_tcp_packet(
                        eth_dst=mac_addr,
                        eth_src=src_mac_addr,
                        ip_dst='10.10.3.3',
                        dl_vlan_enable=True,
                        vlan_vid=i,
                        ip_id=102,
                        ip_ttl=64)
                    send_packet(self, 0, str(pkt))
                    verify_packets(self, exp_pkt, [exp_asic_port])

                    print "Tagged multicast packet vlan %d" % i
                    mac_addr = '01005e' + hex(i)[2:].zfill(3) + \
                                hex(pipe)[2:].zfill(1) + hex(port)[2:].zfill(2)
                    mac_addr = ':'.join(
                        s.encode('hex') for s in mac_addr.decode('hex'))
                    pkt = simple_tcp_packet(
                        eth_dst=mac_addr,
                        eth_src='00:00:00:00:01:01',
                        ip_dst='10.10.3.3',
                        ip_src='10.10.1.1',
                        dl_vlan_enable=True,
                        vlan_vid=i,
                        ip_id=105,
                        ip_ttl=4)
                    exp_pkt = simple_tcp_packet(
                        eth_dst=mac_addr,
                        eth_src='00:00:00:00:01:01',
                        ip_dst='10.10.3.3',
                        ip_src='10.10.1.1',
                        dl_vlan_enable=True,
                        vlan_vid=i,
                        ip_id=105,
                        ip_ttl=4)
                    send_packet(self, 0, str(pkt))
                    verify_packets(self, exp_pkt, [exp_asic_port])

    def tearDown(self):
        """
        pass

        """

        # skip test for BMV2
        if test_param_get('target') == 'bmv2':
            print "Skipping test for BMV2"
            return

        delete_default_entries(self.client, self.sess_hdl, self.device)
        for pipe in range(0, g_num_pipes):
            for port in range(0, 72):
                asic_port = pipe_port_to_asic_port(pipe, port)
                delete_mac(self.client, self.sess_hdl, self.device,
                           self.dhdl[asic_port], self.shdl[asic_port])
                delete_dmac(self.client, self.sess_hdl, self.device,
                            self.mc_dmac_hdl[asic_port])

                delete_vlan_mapping(self.client, self.sess_hdl, self.device,
                                    self.hdl[asic_port],
                                    self.mbr_hdl[asic_port])
                delete_pv_to_ifindex_mapping(self.client, self.sess_hdl,
                                             self.device, self.if_hdl[port])

        delete_bd(self.client, self.sess_hdl, self.device, self.vlan_hdl)
        for i in self.trunk_vlans:
            delete_bd(self.client, self.sess_hdl, self.device,
                      self.trunk_vlan_hdl[i])
            delete_vlan_mapping(self.client, self.sess_hdl, self.device,
                                self.vhdl[asic_port], self.vmbr_hdl[asic_port])
            delete_pv_to_ifindex_mapping(self.client, self.sess_hdl,
                                         self.device, self.vif_hdl[port])

        # delete ports
        delete_ports(self.client, self.sess_hdl, self.device, 2, self.ret_list)

        # delete  init and default entries
        delete_init_entries(self.client, self.sess_hdl, self.device,
                            self.ret_init, tunnel_enabled)

        self.mc.mc_destroy_session(self.mc_sess_hdl)
        self.conn_mgr.complete_operations(self.sess_hdl)
        self.conn_mgr.client_cleanup(self.sess_hdl)

# Ingress Snapshot on Basic L2 Test case
class SnapshotIgL2Test(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print
        # skip test for BMV2
        if test_param_get('target') == 'bmv2':
            return
        sess_hdl = self.conn_mgr.client_init()

        dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))
        device = 0

        client_init(self.client, sess_hdl, dev_tgt)

        #Add the default entries
        populate_default_entries(self.client, sess_hdl, dev_tgt, ipv6_enabled,
                                 acl_enabled, tunnel_enabled, mc_tunnel_enabled, multicast_enabled,
                                 int_enabled)
        ret_init = populate_init_entries(
            self.client, sess_hdl, dev_tgt, rewrite_index, rmac,
            inner_rmac_group, outer_rmac_group, ipv6_enabled, tunnel_enabled)

        #Create two ports
        ret_list = program_ports(self.client, sess_hdl, dev_tgt, 2)

        vlan = 10
        ifindex1 = 1
        ifindex2 = 2
        port1 = 1
        port2 = 2
        v4_enabled = 0
        v6_enabled = 0

        # Add bd entry
        vlan_hdl = program_bd(self.client, sess_hdl, dev_tgt, vlan, 0)

        #Add ports to vlan
        #port vlan able programs (port, vlan) mapping and derives the bd
        hdl1, mbr_hdl1 = program_vlan_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            vrf,
            vlan,
            port1,
            v4_enabled,
            v6_enabled,
            0,
            0,
            ctag=None,
            stag=None)
        if_hdl1 = program_pv_to_ifindex_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            port1,
            ifindex1,
            ctag=None,
            stag=None,
            rid=0)

        hdl2, mbr_hdl2 = program_vlan_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            vrf,
            vlan,
            port2,
            v4_enabled,
            v6_enabled,
            0,
            0,
            ctag=None,
            stag=None)
        if_hdl2 = program_pv_to_ifindex_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            port2,
            ifindex2,
            ctag=None,
            stag=None,
            rid=0)

        #Add static macs to ports. (vlan, mac -> port)
        dmac_hdl1, smac_hdl1 = program_mac(self.client, sess_hdl, dev_tgt, vlan,
                                           '00:71:14:15:16:6a', port1, ifindex1)
        dmac_hdl2, smac_hdl2 = program_mac(self.client, sess_hdl, dev_tgt, vlan,
                                           '00:96:22:22:22:29', port2, ifindex2)

        self.conn_mgr.complete_operations(sess_hdl)

        # Create snapshot
        pipe_val = 0
        start_stage = 0
        cap_field_stage_0 = 0
        cap_field_stage_1 = 1
        end_stage = 6
        dir_val = 0
        usec_val = 0
        # Check for fields in scope
        field_exists = self.client.snapshot_field_in_scope(dev_tgt, start_stage, dir_val,
                                     "tunnel_metadata_tunnel_dst_index")
        assert field_exists == True
        print "creating snapshot"
        snap_hdl = self.client.snapshot_create(dev_tgt, start_stage, end_stage, dir_val)
        print "PIPE_MGR gave me snapshot handle:", snap_hdl
        assert (snap_hdl != 0)
        # Disable the snapshot state first
        self.client.snapshot_state_set(snap_hdl, 0, 0)

        # Set the snapshot trigger
        trig_spec = dc_snapshot_trig_spec_t("ipv6_valid", 1, 1)
        self.client.snapshot_capture_trigger_set(snap_hdl, trig_spec, trig_spec)

        # Enable the snapshot
        self.client.snapshot_state_set(snap_hdl, 1, usec_val)
        snap_state = self.client.snapshot_state_get(snap_hdl, pipe_val)
        assert (snap_state == 1)
        time.sleep(2)

        pkt = simple_tcp_packet(
            eth_dst='00:96:22:22:22:29',
            eth_src='00:71:14:15:16:6a',
            ip_dst='10.0.0.1',
            ip_src='192.168.0.1',
            ip_id=101,
            ip_ttl=70)

        try:
            print "Sending packet port 1 -> port 2 on vlan 10 (192.168.0.1 -> 10.0.0.1 [id = 101])"
            send_packet(self, 1, str(pkt))
            verify_packets(self, pkt, [2])
            time.sleep(2)

            # Verify that the snapshot was not taken
            snap_state = self.client.snapshot_state_get(snap_hdl, pipe_val)
            assert (snap_state == 1)
            print "Snapshot not triggered as expected"

            # Clear and Set the new snapshot trigger
            trig_spec = dc_snapshot_trig_spec_t("ipv4_valid", 1, 1)
            self.client.snapshot_capture_trigger_set(snap_hdl, trig_spec, trig_spec)
            snap_state = self.client.snapshot_state_get(snap_hdl, pipe_val)
            assert (snap_state == 1)
            time.sleep(2)

            print "Sending packet port 1 -> port 2 on vlan 10 (192.168.0.1 -> 10.0.0.1 [id = 101])"
            send_packet(self, 1, str(pkt))
            verify_packets(self, pkt, [2])
            time.sleep(2)

            # Verify that the snapshot was taken and state went to disabled
            snap_state = self.client.snapshot_state_get(snap_hdl, pipe_val)
            assert (snap_state == 0)
            print "Snapshot triggered as expected"

            print "Verifying captured fields from multiple stages"
            # Verify captured field values
            field_value = self.client.snapshot_capture_data_get(snap_hdl, pipe_val, cap_field_stage_0,
                    "ipv4_ttl")
            print "Captured field value (ipv4 ttl): ", hex(field_value)
            assert field_value == 70
            field_value = self.client.snapshot_capture_data_get(snap_hdl, pipe_val, cap_field_stage_0,
                    "ethernet_srcAddr")
            print "Captured field value (ethernet src-addr): ", hex(field_value)
            assert field_value == 0x711415166a
            field_value = self.client.snapshot_capture_data_get(snap_hdl, pipe_val, cap_field_stage_0,
                    "tcp_valid")
            print "Captured field value (tcp valid): ", hex(field_value)
            assert field_value == 1
            # Check on stage 1 also
            field_value = self.client.snapshot_capture_data_get(snap_hdl, pipe_val, cap_field_stage_1,
                    "ipv4_identification")
            print "Captured field value (ipv4 identification): ", hex(field_value)
            # assert field_value == 101

        finally:
            print "deleting snapshot"
            self.client.snapshot_delete(snap_hdl)
            delete_default_entries(self.client, sess_hdl, device)

            delete_mac(self.client, sess_hdl, device, dmac_hdl2, smac_hdl2)
            delete_mac(self.client, sess_hdl, device, dmac_hdl1, smac_hdl1)

            delete_vlan_mapping(self.client, sess_hdl, device, hdl2, mbr_hdl2)
            delete_vlan_mapping(self.client, sess_hdl, device, hdl1, mbr_hdl1)

            delete_pv_to_ifindex_mapping(self.client, sess_hdl, device, if_hdl1)
            delete_pv_to_ifindex_mapping(self.client, sess_hdl, device, if_hdl2)

            # delete BD
            delete_bd(self.client, sess_hdl, device, vlan_hdl)

            # delete ports
            delete_ports(self.client, sess_hdl, device, 2, ret_list)

            # delete  init and default entries
            delete_init_entries(self.client, sess_hdl, device, ret_init,
                                tunnel_enabled)

            self.conn_mgr.complete_operations(sess_hdl)
            self.conn_mgr.client_cleanup(sess_hdl)

# Egress Snapshot on Basic L3 Test case
class SnapshotEgL3Ipv4Test(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print
        # skip test for BMV2
        if test_param_get('target') == 'bmv2':
            return
        sess_hdl = self.conn_mgr.client_init()

        dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))
        dev_tgt_0 = DevTarget_t(0, 0)
        device = 0

        client_init(self.client, sess_hdl, dev_tgt)

        #Add the default entries
        populate_default_entries(self.client, sess_hdl, dev_tgt, ipv6_enabled,
                                 acl_enabled, tunnel_enabled, mc_tunnel_enabled, multicast_enabled,
                                 int_enabled)
        ret_init = populate_init_entries(
            self.client, sess_hdl, dev_tgt, rewrite_index, rmac,
            inner_rmac_group, outer_rmac_group, ipv6_enabled, tunnel_enabled)

        #Create two ports
        ret_list = program_ports(self.client, sess_hdl, dev_tgt, 2)

        vlan1 = 10
        vlan2 = 11
        ifindex1 = 1
        ifindex2 = 2
        port1 = 1
        port2 = 2
        v4_enabled = 1
        v6_enabled = 0

        # Add bd entry
        vlan_hdl1 = program_bd(self.client, sess_hdl, dev_tgt, vlan1, 0)
        vlan_hdl2 = program_bd(self.client, sess_hdl, dev_tgt, vlan2, 0)

        #For every L3 port, an implicit vlan will be allocated
        #Add ports to vlan
        #Outer vlan table programs (port, vlan) mapping and derives the bd
        #Inner vlan table derives the bd state
        hdl1, mbr_hdl1 = program_vlan_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            vrf,
            vlan1,
            port1,
            v4_enabled,
            v6_enabled,
            inner_rmac_group,
            0,
            ctag=None,
            stag=None)
        if_hdl1 = program_pv_to_ifindex_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            port1,
            ifindex1,
            ctag=None,
            stag=None)

        hdl2, mbr_hdl2 = program_vlan_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            vrf,
            vlan2,
            port2,
            v4_enabled,
            v6_enabled,
            inner_rmac_group,
            0,
            ctag=None,
            stag=None)
        if_hdl2 = program_pv_to_ifindex_mapping(
            self.client,
            sess_hdl,
            dev_tgt,
            port2,
            ifindex2,
            ctag=None,
            stag=None,
            rid=0)

        #Create nexthop
        nhop1 = 1
        nhop_hdl1 = program_nexthop(self.client, sess_hdl, dev_tgt, nhop1,
                                    vlan1, port1, ifindex1, 0)
        #Add rewrite information (ARP info)
        arp_hdl1 = program_ipv4_unicast_rewrite(
            self.client, sess_hdl, dev_tgt, vlan1, nhop1, '00:11:11:11:11:11')
        egress_bd_hdl1 = program_egress_bd_properties(
            self.client, sess_hdl, dev_tgt, vlan1, rewrite_index)
        #Add route
        route_hdl1 = program_ipv4_route(self.client, sess_hdl, dev_tgt, vrf,
                                        0x0a0a0a01, 32, nhop1)
        #Create nexthop
        nhop2 = 2
        nhop_hdl2 = program_nexthop(self.client, sess_hdl, dev_tgt, nhop2,
                                    vlan2, port2, ifindex2, 0)
        #Add rewrite information (ARP info)
        arp_hdl2 = program_ipv4_unicast_rewrite(
            self.client, sess_hdl, dev_tgt, vlan2, nhop2, '00:22:22:22:22:22')
        egress_bd_hdl2 = program_egress_bd_properties(
            self.client, sess_hdl, dev_tgt, vlan2, rewrite_index)
        #Add route
        route_hdl2 = program_ipv4_route(self.client, sess_hdl, dev_tgt, vrf,
                                        0x14141401, 32, nhop2)

        self.conn_mgr.complete_operations(sess_hdl)

        # Create snapshot
        pipe_val = 0
        start_stage = 2
        cap_field_stage_2 = 2
        cap_field_stage_3 = 3
        cap_field_stage_7 = 7
        end_stage = 7
        dir_val = 1
        usec_val = 0
        # Check for fields in scope
        field_exists = self.client.snapshot_field_in_scope(dev_tgt, start_stage, dir_val,
                                     "inner_ipv4_fragOffset")
        assert field_exists == True
        print "creating snapshot"
        snap_hdl = self.client.snapshot_create(dev_tgt_0, start_stage, end_stage, dir_val)
        print "PIPE_MGR gave me snapshot handle:", snap_hdl
        assert (snap_hdl != 0)
        # Disable the snapshot state first
        self.client.snapshot_state_set(snap_hdl, 0, 0)

        # Set the snapshot trigger
        trig_spec1 = dc_snapshot_trig_spec_t("ipv4_dstAddr", ipv4Addr_to_i32("20.20.20.1"),
                ipv4Addr_to_i32("20.20.20.1"))
        trig_spec2 = dc_snapshot_trig_spec_t("ipv4_ttl", 30, 30)
        self.client.snapshot_capture_trigger_set(snap_hdl, trig_spec1, trig_spec2)

        # Enable the snapshot
        self.client.snapshot_state_set(snap_hdl, 1, usec_val)
        snap_state = self.client.snapshot_state_get(snap_hdl, pipe_val)
        assert (snap_state == 1)
        time.sleep(2)

        pkt = simple_tcp_packet(
            eth_dst='00:33:33:33:33:33',
            eth_src='00:11:11:11:11:11',
            ip_dst='20.20.20.1',
            ip_src='10.10.10.1',
            ip_id=101,
            ip_ttl=30,
            ip_ihl=5)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:22:22:22:22:22',
            eth_src='00:33:33:33:33:33',
            ip_dst='20.20.20.1',
            ip_src='10.10.10.1',
            ip_id=101,
            ip_ttl=29,
            ip_ihl=5)
        try:
            print "Sending packet port 1 -> port 2 (10.10.10.1 -> 20.20.20.1 [id = 101])"
            send_packet(self, 1, str(pkt))
            verify_packets(self, exp_pkt, [2])
            time.sleep(2)

            # Verify that the snapshot was taken and state went to disabled
            snap_state = self.client.snapshot_state_get(snap_hdl, pipe_val)
            self.assertEqual(snap_state, 0)
            print "Snapshot triggered as expected"

            # Verify captured field values
            print "Verifying captured fields from multiple stages"
            field_value = self.client.snapshot_capture_data_get(snap_hdl, pipe_val, cap_field_stage_2,
                    "ipv4_valid")
            print "Captured field value (ipv4 valid): ", hex(field_value)
            self.assertEqual(field_value, 1)
            # In stage 3 the DMAC has not yet been rewritten so we expect it to
            # be 00:33:33:33:33:33
            field_value = self.client.snapshot_capture_data_get(snap_hdl, pipe_val, cap_field_stage_3,
                    "ethernet_dstAddr")
            print "Captured field value (eth dst): ", hex(field_value)
            self.assertEqual(field_value, 0x3333333333)
            # In stage 7 the DMAC has been rewritten so we expect it to be
            # 00:22:22:22:22:22
            field_value = self.client.snapshot_capture_data_get(snap_hdl, pipe_val, cap_field_stage_7,
                    "ethernet_dstAddr")
            print "Captured field value (eth dst): ", hex(field_value)
            self.assertEqual(field_value, 0x2222222222)

            print "Adding new trigger with zero mask"
            # Clear the snapshot trigger and add a new trigger
            # Set the snapshot trigger (match mask is set to zero)
            trig_spec = dc_snapshot_trig_spec_t("ipv4_srcAddr", ipv4Addr_to_i32("10.10.10.1"), 0)
            self.client.snapshot_capture_trigger_set(snap_hdl, trig_spec, trig_spec)

            # Enable the snapshot
            self.client.snapshot_state_set(snap_hdl, 1, usec_val)
            snap_state = self.client.snapshot_state_get(snap_hdl, pipe_val)
            self.assertEqual(snap_state, 1)
            time.sleep(2)

            # Send the packet again
            print "Sending packet port 1 -> port 2 (10.10.10.1 -> 20.20.20.1 [id = 101])"
            send_packet(self, 1, str(pkt))
            verify_packets(self, exp_pkt, [2])
            time.sleep(2)

            # Verify that the snapshot was taken and state went to disabled
            snap_state = self.client.snapshot_state_get(snap_hdl, pipe_val)
            self.assertEqual(snap_state, 0)
            print "Snapshot triggered as expected"

            # Verify captured field values
            print "Verifying captured fields from multiple stages"
            field_value = self.client.snapshot_capture_data_get(snap_hdl, pipe_val, cap_field_stage_7,
                    "ipv4_valid")
            print "Captured field value (ipv4 valid): ", hex(field_value)
            self.assertEqual(field_value, 1)
            field_value = self.client.snapshot_capture_data_get(snap_hdl, pipe_val, cap_field_stage_2,
                    "tcp_valid")
            print "Captured field value (tcp valid): ", hex(field_value)
            self.assertEqual(field_value, 1)

        finally:
            print "deleting snapshot"
            self.client.snapshot_delete(snap_hdl)
            delete_default_entries(self.client, sess_hdl, device)
            delete_egress_bd_properties(self.client, sess_hdl, device,
                                        egress_bd_hdl1)
            delete_egress_bd_properties(self.client, sess_hdl, device,
                                        egress_bd_hdl2)

            delete_ipv4_route(self.client, sess_hdl, device, 32, route_hdl2)
            delete_ipv4_unicast_rewrite(self.client, sess_hdl, device, arp_hdl2)
            delete_nexthop(self.client, sess_hdl, device, nhop_hdl2)

            delete_ipv4_route(self.client, sess_hdl, device, 32, route_hdl1)
            delete_ipv4_unicast_rewrite(self.client, sess_hdl, device, arp_hdl1)
            delete_nexthop(self.client, sess_hdl, device, nhop_hdl1)

            delete_vlan_mapping(self.client, sess_hdl, device, hdl2, mbr_hdl2)
            delete_vlan_mapping(self.client, sess_hdl, device, hdl1, mbr_hdl1)

            delete_pv_to_ifindex_mapping(self.client, sess_hdl, device, if_hdl1)
            delete_pv_to_ifindex_mapping(self.client, sess_hdl, device, if_hdl2)

            delete_bd(self.client, sess_hdl, device, vlan_hdl1)
            delete_bd(self.client, sess_hdl, device, vlan_hdl2)

            # delete ports
            delete_ports(self.client, sess_hdl, device, 2, ret_list)

            # delete  init and default entries
            delete_init_entries(self.client, sess_hdl, device, ret_init,
                                tunnel_enabled)

            self.conn_mgr.complete_operations(sess_hdl)
            self.conn_mgr.client_cleanup(sess_hdl)


