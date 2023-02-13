# Copyright 2013-present Barefoot Networks, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
Thrift SAI interface L3 tests
"""
import socket
import sys
from struct import pack, unpack

from switch_utils import *

import sai_base_test
from ptf.mask import Mask
from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *
from switchsai_thrift.sai_headers import  *
from switch_utils import *


@group('l3')
@group('ecmp')
@group('l3-ocp')
@group('l3-ocp-mts')
class L3IPv4EcmpHostTest(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        #Hashing not implemented for BMv2
        if (test_param_get('target') == 'bmv2'):
            return
        print
        print "Sending packet port 1 -> port 2 (192.168.0.1 -> 10.10.10.1 [id = 101])"
        switch_init(self.client)
        port1 = port_list[0]
        port2 = port_list[1]
        port3 = port_list[2]
        port4 = port_list[3]
        port5 = port_list[4]
        v4_enabled = 1
        v6_enabled = 1
        mac = ''

        addr_family = SAI_IP_ADDR_FAMILY_IPV4
        ip_addr1 = '10.10.10.2'
        ip_addr1_subnet = '10.10.10.0'
        ip_mask1 = '255.255.255.0'
        dmac1 = '00:11:22:33:44:55'
        dmac2 = '00:11:22:33:44:56'
        dmac2 = '00:11:22:33:44:56'
        dmac3 = '00:11:22:33:44:57'
        dmac4 = '00:11:22:33:44:58'

        vr1 = sai_thrift_create_virtual_router(self.client, v4_enabled, v6_enabled)

        sai_thrift_remove_default_bridge_ports(self.client, [port1, port2, port3, port4, port5])

        rif1 = sai_thrift_create_router_interface(self.client, vr1, 1, port1, 0, v4_enabled, v6_enabled, mac)
        rif2 = sai_thrift_create_router_interface(self.client, vr1, 1, port2, 0, v4_enabled, v6_enabled, mac)
        rif3 = sai_thrift_create_router_interface(self.client, vr1, 1, port3, 0, v4_enabled, v6_enabled, mac)
        rif4 = sai_thrift_create_router_interface(self.client, vr1, 1, port4, 0, v4_enabled, v6_enabled, mac)
        rif5 = sai_thrift_create_router_interface(self.client, vr1, 1, port5, 0, v4_enabled, v6_enabled, mac)

        sai_thrift_create_neighbor(self.client, addr_family, rif1, ip_addr1, dmac1)
        sai_thrift_create_neighbor(self.client, addr_family, rif2, ip_addr1, dmac2)
        sai_thrift_create_neighbor(self.client, addr_family, rif3, ip_addr1, dmac3)
        sai_thrift_create_neighbor(self.client, addr_family, rif4, ip_addr1, dmac4)

        nhop1 = sai_thrift_create_nhop(self.client, addr_family, ip_addr1, rif1)
        nhop2 = sai_thrift_create_nhop(self.client, addr_family, ip_addr1, rif2)
        nhop3 = sai_thrift_create_nhop(self.client, addr_family, ip_addr1, rif3)
        nhop4 = sai_thrift_create_nhop(self.client, addr_family, ip_addr1, rif4)

        nhop_group1 = sai_thrift_create_next_hop_group(self.client)

        nhop_gmember1 = sai_thrift_create_next_hop_group_member(self.client, nhop_group1, nhop1)
        nhop_gmember2 = sai_thrift_create_next_hop_group_member(self.client, nhop_group1, nhop2)
        nhop_gmember3 = sai_thrift_create_next_hop_group_member(self.client, nhop_group1, nhop3)
        nhop_gmember4 = sai_thrift_create_next_hop_group_member(self.client, nhop_group1, nhop4)

        sai_thrift_create_route(self.client, vr1, addr_family, ip_addr1_subnet, ip_mask1, nhop_group1)
        #sai_thrift_create_route(self.client, vr1, addr_family, ip_addr1_subnet, ip_mask1, rif2)

        default_attr = self.client.sai_thrift_get_switch_attribute_by_id(SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED)
        default_seed = default_attr.value.u64

        # send the test packet(s)
        try:
            pkt = simple_tcp_packet(eth_dst=router_mac,
                                eth_src='00:22:22:22:22:22',
                                ip_dst='10.10.10.1',
                                ip_src='192.168.0.1',
                                ip_id=106,
                                ip_ttl=64)

            exp_pkt1 = simple_tcp_packet(
                                eth_dst='00:11:22:33:44:55',
                                eth_src=router_mac,
                                ip_dst='10.10.10.1',
                                ip_src='192.168.0.1',
                                ip_id=106,
                                #ip_tos=3,
                                ip_ttl=63)
            exp_pkt2 = simple_tcp_packet(
                                eth_dst='00:11:22:33:44:56',
                                eth_src=router_mac,
                                ip_dst='10.10.10.1',
                                ip_src='192.168.0.1',
                                ip_id=106,
                                #ip_tos=3,
                                ip_ttl=63)
            exp_pkt3 = simple_tcp_packet(
                                eth_dst='00:11:22:33:44:57',
                                eth_src=router_mac,
                                ip_dst='10.10.10.1',
                                ip_src='192.168.0.1',
                                ip_id=106,
                                #ip_tos=3,
                                ip_ttl=63)
            exp_pkt4 = simple_tcp_packet(
                                eth_dst='00:11:22:33:44:58',
                                eth_src=router_mac,
                                ip_dst='10.10.10.1',
                                ip_src='192.168.0.1',
                                ip_id=106,
                                #ip_tos=3,
                                ip_ttl=63)

            send_packet(self, 4, str(pkt))
            verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2, exp_pkt3, exp_pkt4], [0, 1, 2, 3])

            pkt = simple_tcp_packet(eth_dst=router_mac,
                                    eth_src='00:22:22:22:22:22',
                                    ip_dst='10.10.10.1',
                                    ip_src='192.168.100.3',
                                    ip_id=106,
                                    ip_ttl=64)

            exp_pkt1 = simple_tcp_packet(
                                    eth_dst='00:11:22:33:44:55',
                                    eth_src=router_mac,
                                    ip_dst='10.10.10.1',
                                    ip_src='192.168.100.3',
                                    ip_id=106,
                                    #ip_tos=3,
                                    ip_ttl=63)
            exp_pkt2 = simple_tcp_packet(
                                    eth_dst='00:11:22:33:44:56',
                                    eth_src=router_mac,
                                    ip_dst='10.10.10.1',
                                    ip_src='192.168.100.3',
                                    ip_id=106,
                                    #ip_tos=3,
                                    ip_ttl=63)
            exp_pkt3 = simple_tcp_packet(
                                eth_dst='00:11:22:33:44:57',
                                eth_src=router_mac,
                                ip_dst='10.10.10.1',
                                ip_src='192.168.100.3',
                                ip_id=106,
                                #ip_tos=3,
                                ip_ttl=63)
            exp_pkt4 = simple_tcp_packet(
                                eth_dst='00:11:22:33:44:58',
                                eth_src=router_mac,
                                ip_dst='10.10.10.1',
                                ip_src='192.168.100.3',
                                ip_id=106,
                                #ip_tos=3,
                                ip_ttl=63)

            send_packet(self, 4, str(pkt))
            verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2, exp_pkt3, exp_pkt4], [0, 1, 2, 3])


            print 'Changing Hash seed'
            seed=137
            attr_value = sai_thrift_attribute_value_t(u64=seed)
            attr = sai_thrift_attribute_t(id=SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED, value=attr_value)
            self.client.sai_thrift_set_switch_attribute(attr)

            switch_attr = self.client.sai_thrift_get_switch_attribute_by_id(SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED)

            self.assertTrue(switch_attr.id ==SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED and switch_attr.value.u64==seed, "Seed set/get failed")

            port_set = set()
            for i in range(0,10):
		pkt = simple_tcp_packet(eth_dst=router_mac,
					eth_src='00:22:22:22:22:22',
					ip_dst='10.10.10.1',
					ip_src='192.168.100.3',
					ip_id=106,
					ip_ttl=64)

		exp_pkt1 = simple_tcp_packet(
					eth_dst='00:11:22:33:44:55',
					eth_src=router_mac,
					ip_dst='10.10.10.1',
					ip_src='192.168.100.3',
					ip_id=106,
					#ip_tos=3,
					ip_ttl=63)
		exp_pkt2 = simple_tcp_packet(
					eth_dst='00:11:22:33:44:56',
					eth_src=router_mac,
					ip_dst='10.10.10.1',
					ip_src='192.168.100.3',
					ip_id=106,
					#ip_tos=3,
					ip_ttl=63)
		exp_pkt3 = simple_tcp_packet(
				    eth_dst='00:11:22:33:44:57',
				    eth_src=router_mac,
				    ip_dst='10.10.10.1',
				    ip_src='192.168.100.3',
				    ip_id=106,
				    #ip_tos=3,
				    ip_ttl=63)
		exp_pkt4 = simple_tcp_packet(
				    eth_dst='00:11:22:33:44:58',
				    eth_src=router_mac,
				    ip_dst='10.10.10.1',
				    ip_src='192.168.100.3',
				    ip_id=106,
				    #ip_tos=3,
				    ip_ttl=63)
                send_packet(self, 4, str(pkt))
                rcv_idx = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2, exp_pkt3, exp_pkt4], [0, 1, 2, 3])
                port_set.add(rcv_idx)
                seed=seed+(i+1)*17
                attr_value = sai_thrift_attribute_value_t(u64=seed)
                attr = sai_thrift_attribute_t(id=SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED, value=attr_value)
                self.client.sai_thrift_set_switch_attribute(attr)
            self.assertTrue(len(port_set)!=1)

        finally:
            attr_value = sai_thrift_attribute_value_t(u64=default_seed)
            attr = sai_thrift_attribute_t(id=SAI_SWITCH_ATTR_ECMP_DEFAULT_HASH_SEED, value=attr_value)
            self.client.sai_thrift_set_switch_attribute(attr)

            sai_thrift_remove_route(self.client, vr1, addr_family, ip_addr1_subnet, ip_mask1, nhop_group1)
            #sai_thrift_remove_route(self.client, vr1, addr_family, ip_addr1_subnet, ip_mask1, rif2)

            self.client.sai_thrift_remove_next_hop_group_member(nhop_gmember1)
            self.client.sai_thrift_remove_next_hop_group_member(nhop_gmember2)
            self.client.sai_thrift_remove_next_hop_group_member(nhop_gmember3)
            self.client.sai_thrift_remove_next_hop_group_member(nhop_gmember4)

            self.client.sai_thrift_remove_next_hop_group(nhop_group1)

            sai_thrift_remove_neighbor(self.client, addr_family, rif1, ip_addr1, dmac1)
            sai_thrift_remove_neighbor(self.client, addr_family, rif2, ip_addr1, dmac2)
            sai_thrift_remove_neighbor(self.client, addr_family, rif3, ip_addr1, dmac3)
            sai_thrift_remove_neighbor(self.client, addr_family, rif4, ip_addr1, dmac4)

            self.client.sai_thrift_remove_next_hop(nhop1)
            self.client.sai_thrift_remove_next_hop(nhop2)
            self.client.sai_thrift_remove_next_hop(nhop3)
            self.client.sai_thrift_remove_next_hop(nhop4)

            self.client.sai_thrift_remove_router_interface(rif1)
            self.client.sai_thrift_remove_router_interface(rif2)
            self.client.sai_thrift_remove_router_interface(rif3)
            self.client.sai_thrift_remove_router_interface(rif4)
            self.client.sai_thrift_remove_router_interface(rif5)

            sai_thrift_create_default_bridge_ports(self.client, [port1, port2, port3, port4, port5])

            self.client.sai_thrift_remove_virtual_router(vr1)

@group('l3')
@group('lag')
@group('l3-ocp')
@group('l3-ocp-mts')
class L3IPv4LagTest(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        #Hashing not implemented for BMv2
        if (test_param_get('target') == 'bmv2'):
            return
        switch_init(self.client)
        port1 = port_list[0]
        port2 = port_list[1]
        port3 = port_list[2]
        port4 = port_list[3]
        port5 = port_list[4]
        v4_enabled = 1
        v6_enabled = 1
        mac = ''

        addr_family = SAI_IP_ADDR_FAMILY_IPV4
        ip_addr1 = '10.10.10.1'
        ip_addr1_subnet = '10.10.10.0'
        ip_mask1 = '255.255.255.0'
        dmac1 = '00:11:22:33:44:55'

        vr_id = sai_thrift_create_virtual_router(self.client, v4_enabled, v6_enabled)

        lag_id1 = self.client.sai_thrift_create_lag([])
        sai_thrift_vlan_remove_ports(self.client, switch.default_vlan.oid, [port1, port2, port3, port4])
        lag_member_id1 = sai_thrift_create_lag_member(self.client, lag_id1, port1)
        lag_member_id2 = sai_thrift_create_lag_member(self.client, lag_id1, port2)
        lag_member_id3 = sai_thrift_create_lag_member(self.client, lag_id1, port3)
        lag_member_id4 = sai_thrift_create_lag_member(self.client, lag_id1, port4)
        sai_thrift_remove_default_bridge_ports(self.client, [port5])

        rif_id1 = sai_thrift_create_router_interface(self.client, vr_id, 1, lag_id1, 0, v4_enabled, v6_enabled, mac)
        rif_id2 = sai_thrift_create_router_interface(self.client, vr_id, 1, port5, 0, v4_enabled, v6_enabled, mac)

        sai_thrift_create_neighbor(self.client, addr_family, rif_id1, ip_addr1, dmac1)
        nhop1 = sai_thrift_create_nhop(self.client, addr_family, ip_addr1, rif_id1)
        sai_thrift_create_route(self.client, vr_id, addr_family, ip_addr1_subnet, ip_mask1, rif_id1)
        default_attr = self.client.sai_thrift_get_switch_attribute_by_id(SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED)
        default_seed = default_attr.value.u64

        # send the test packet(s)
        try:
            pkt = simple_tcp_packet(eth_dst=router_mac,
                                    eth_src='00:22:22:22:22:22',
                                    ip_dst='10.10.10.1',
                                    ip_src='192.168.0.1',
                                    ip_id=110,
                                    ip_ttl=64)

            exp_pkt = simple_tcp_packet(
                                    eth_dst='00:11:22:33:44:55',
                                    eth_src=router_mac,
                                    ip_dst='10.10.10.1',
                                    ip_src='192.168.0.1',
                                    ip_id=110,
                                    ip_ttl=63)
            send_packet(self, 4, str(pkt))
            verify_packets_any(self, exp_pkt, [0, 1, 2, 3])

            print 'Changing Lag seed'
            seed=137
            attr_value = sai_thrift_attribute_value_t(u64=seed)
            attr = sai_thrift_attribute_t(id=SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED, value=attr_value)
            self.client.sai_thrift_set_switch_attribute(attr)

            switch_attr = self.client.sai_thrift_get_switch_attribute_by_id(SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED)

            self.assertTrue(switch_attr.id ==SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED and switch_attr.value.u64==seed, "Seed set/get failed")

            port_set = set()
            for i in range(0,10):
		pkt = simple_tcp_packet(eth_dst=router_mac,
					eth_src='00:22:22:22:22:22',
					ip_dst='10.10.10.1',
					ip_src='192.168.0.1',
					ip_id=110,
					ip_ttl=64)

		exp_pkt = simple_tcp_packet(
					eth_dst='00:11:22:33:44:55',
					eth_src=router_mac,
					ip_dst='10.10.10.1',
					ip_src='192.168.0.1',
					ip_id=110,
					ip_ttl=63)
                send_packet(self, 4, str(pkt))
                rcv_idx = verify_any_packet_any_port(self,
                                                     [exp_pkt],
                                                     [0, 1, 2, 3])
                port_set.add(rcv_idx)
                seed=seed+(i+1)*17
		attr_value = sai_thrift_attribute_value_t(u64=seed)
		attr = sai_thrift_attribute_t(id=SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED, value=attr_value)
		self.client.sai_thrift_set_switch_attribute(attr)
	    self.assertTrue(len(port_set)!=1)

        finally:
            attr_value = sai_thrift_attribute_value_t(u64=default_seed)
            attr = sai_thrift_attribute_t(id=SAI_SWITCH_ATTR_LAG_DEFAULT_HASH_SEED, value=attr_value)
            self.client.sai_thrift_set_switch_attribute(attr)
            sai_thrift_remove_route(self.client, vr_id, addr_family, ip_addr1_subnet, ip_mask1, rif_id1)
            sai_thrift_remove_neighbor(self.client, addr_family, rif_id1, ip_addr1, dmac1)
            self.client.sai_thrift_remove_next_hop(nhop1)

            self.client.sai_thrift_remove_router_interface(rif_id1)
            self.client.sai_thrift_remove_router_interface(rif_id2)

            sai_thrift_create_default_bridge_ports(self.client, [port5])

            sai_thrift_remove_lag_member(self.client, lag_member_id1)
            sai_thrift_remove_lag_member(self.client, lag_member_id2)
            sai_thrift_remove_lag_member(self.client, lag_member_id3)
            sai_thrift_remove_lag_member(self.client, lag_member_id4)
            self.client.sai_thrift_remove_lag(lag_id1)
            self.client.sai_thrift_remove_virtual_router(vr_id)

