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

device = 0
cpu_port = 64
swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)

if swports == []:
    swports = [x for x in range(65)]
################################################################################


###############################################################################
@group('resilient')
class L3IPv4EcmpResilientTest(api_base_tests.ThriftInterfaceDataPlane):
    def add_interface(self, port_handle, ip_addr, rmac, vrf):
        port = self.client.switch_api_port_id_to_handle_get(device, port_handle)
        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        rif = self.client.switch_api_rif_create(0, rif_info)
        info = switcht_interface_info_t(
            type=SWITCH_INTERFACE_TYPE_PORT,
            handle=port,
            rif_handle=rif)
        interface = self.client.switch_api_interface_create(device, info)
        ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr=ip_addr, prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif, vrf, ip)
        return ip, interface, rif

    def exp_pkts(self, dst_ip_addr, src_port, dst_port):
        exp_pkt1 = simple_tcp_packet(
            eth_dst='00:11:22:33:44:55',
            eth_src='00:77:66:55:44:33',
            ip_dst=dst_ip_addr,
            ip_src='192.168.8.1',
            ip_id=106,
            ip_ttl=63,
            tcp_sport=src_port,
            tcp_dport=dst_port)
        exp_pkt2 = simple_tcp_packet(
            eth_dst='00:11:22:33:44:56',
            eth_src='00:77:66:55:44:33',
            ip_dst=dst_ip_addr,
            ip_src='192.168.8.1',
            ip_id=106,
            ip_ttl=63,
            tcp_sport=src_port,
            tcp_dport=dst_port)
        exp_pkt3 = simple_tcp_packet(
            eth_dst='00:11:22:33:44:57',
            eth_src='00:77:66:55:44:33',
            ip_dst=dst_ip_addr,
            ip_src='192.168.8.1',
            ip_id=106,
            ip_ttl=63,
            tcp_sport=src_port,
            tcp_dport=dst_port)
        exp_pkt4 = simple_tcp_packet(
            eth_dst='00:11:22:33:44:58',
            eth_src='00:77:66:55:44:33',
            ip_dst=dst_ip_addr,
            ip_src='192.168.8.1',
            ip_id=106,
            ip_ttl=63,
            tcp_sport=src_port,
            tcp_dport=dst_port)
        return [exp_pkt1, exp_pkt2, exp_pkt3, exp_pkt4]

    def runTest(self):
        print
        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        i_ip1, if1, rif1 = self.add_interface(swports[0], '192.168.0.2', rmac,
                                              vrf)
        i_ip2, if2, rif2 = self.add_interface(swports[1], '10.0.0.2', rmac, vrf)
        i_ip3, if3, rif3 = self.add_interface(swports[2], '11.0.0.2', rmac, vrf)
        i_ip4, if4, rif4 = self.add_interface(swports[3], '12.0.0.2', rmac, vrf)
        i_ip5, if5, rif5 = self.add_interface(swports[4], '13.0.0.2', rmac, vrf)

        n_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.0.100',
            prefix_length=32)
        nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, rif2, n_ip1, '00:11:22:33:44:55')

        n_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='11.0.0.100',
            prefix_length=32)
        nhop2, neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, rif3, n_ip2, '00:11:22:33:44:56')

        n_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='12.0.0.101',
            prefix_length=32)
        nhop3, neighbor3 = switch_api_l3_nhop_neighbor_create(self, device, rif4, n_ip3, '00:11:22:33:44:57')

        n_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='13.0.0.101',
            prefix_length=32)
        nhop3, neighbor3 = switch_api_l3_nhop_neighbor_create(self, device, rif5, n_ip4, '00:11:22:33:44:58')

        ecmp = self.client.switch_api_ecmp_create(device)
        self.client.switch_api_ecmp_member_add(device, ecmp, 4,
                                               [nhop1, nhop2, nhop3, nhop4])

        r_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.0.0',
            prefix_length=16)
        self.client.switch_api_l3_route_add(device, vrf, r_ip, ecmp)

        try:
            pkt_to_idx = {}
            count = [0, 0, 0, 0]
            dst_ip = int(socket.inet_aton('10.10.10.1').encode('hex'), 16)
            random.seed(314159)
            num_sample_pkts = 200
            # Send few sample pkts and store the received index for each packets
            for i in range(0, num_sample_pkts):
                dst_ip_addr = socket.inet_ntoa(
                    hex(dst_ip)[2:].zfill(8).decode('hex'))
                src_port = random.randint(0, 65535)
                dst_port = random.randint(0, 65535)
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src='192.168.8.1',
                    ip_id=106,
                    ip_ttl=64,
                    tcp_sport=src_port,
                    tcp_dport=dst_port)
                send_packet(self, swports[0], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self,
                    self.exp_pkts(dst_ip_addr, src_port, dst_port),
                    [swports[1], swports[2], swports[3], swports[4]])
                pkt_to_idx[(src_port, dst_port, dst_ip_addr)] = rcv_idx
                count[rcv_idx] += 1
                dst_ip += 1
            print "Initial ECMP load balancing result", count

            # Deactivate one path and send the same set of packets again
            self.client.switch_api_l3_ecmp_member_deactivate(device, ecmp, 1,
                                                             [nhop1])
            count = [0, 0, 0, 0]
            dst_ip = int(socket.inet_aton('10.10.10.1').encode('hex'), 16)

            random.seed(314159)
            for i in range(0, num_sample_pkts):
                dst_ip_addr = socket.inet_ntoa(
                    hex(dst_ip)[2:].zfill(8).decode('hex'))
                src_port = random.randint(0, 65535)
                dst_port = random.randint(0, 65535)
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src='192.168.8.1',
                    ip_id=106,
                    ip_ttl=64,
                    tcp_sport=src_port,
                    tcp_dport=dst_port)
                send_packet(self, swports[0], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self,
                    self.exp_pkts(dst_ip_addr, src_port, dst_port),
                    [swports[1], swports[2], swports[3], swports[4]])
                # Make sure the sampled packets are received on the same path
                if (i < num_sample_pkts and
                        pkt_to_idx[(src_port, dst_port, dst_ip_addr)] != 0):
                    self.assertTrue(rcv_idx == pkt_to_idx[(src_port, dst_port,
                                                           dst_ip_addr)])
                count[rcv_idx] += 1
                dst_ip += 1
            # Make sure the live paths are equally balanced after one path is
            # failed
            print "ECMP load balancing result after one path is failed", count
            self.assertTrue(count[0] == 0)
            for i in range(1, 4):
                self.assertTrue((count[i] >= ((num_sample_pkts / 3) * 0.7)),
                                "Not all paths are equally balanced")

            # Deactivate 2nd path and send the same set of packets again
            self.client.switch_api_l3_ecmp_member_deactivate(device, ecmp, 1,
                                                             [nhop2])
            count = [0, 0, 0, 0]
            dst_ip = int(socket.inet_aton('10.10.10.1').encode('hex'), 16)

            random.seed(314159)
            for i in range(0, num_sample_pkts):
                dst_ip_addr = socket.inet_ntoa(
                    hex(dst_ip)[2:].zfill(8).decode('hex'))
                src_port = random.randint(0, 65535)
                dst_port = random.randint(0, 65535)
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src='192.168.8.1',
                    ip_id=106,
                    ip_ttl=64,
                    tcp_sport=src_port,
                    tcp_dport=dst_port)
                send_packet(self, swports[0], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self,
                    self.exp_pkts(dst_ip_addr, src_port, dst_port),
                    [swports[1], swports[2], swports[3], swports[4]])
                # Make sure the sampled packets are received on the same path
                if (i < num_sample_pkts and
                        pkt_to_idx[(src_port, dst_port, dst_ip_addr)] > 1):
                    self.assertTrue(rcv_idx == pkt_to_idx[(src_port, dst_port,
                                                           dst_ip_addr)])
                count[rcv_idx] += 1
                dst_ip += 1
            # Make sure the live paths are equally balanced after 2nd path failed
            print "ECMP load balancing result after two paths failed", count
            self.assertTrue(count[0] == 0)
            for i in range(2, 4):
                self.assertTrue((count[i] >= ((num_sample_pkts / 2) * 0.7)),
                                "Not all paths are equally balanced")

            # Activate the previously failed path 2
            self.client.switch_api_l3_ecmp_member_activate(device, ecmp, 2,
                                                           [nhop2])
            count = [0, 0, 0, 0]
            for i in range(0, num_sample_pkts):
                dst_ip_addr = socket.inet_ntoa(
                    hex(dst_ip)[2:].zfill(8).decode('hex'))
                src_port = random.randint(0, 65535)
                dst_port = random.randint(0, 65535)
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src='192.168.8.1',
                    ip_id=106,
                    ip_ttl=64,
                    tcp_sport=src_port,
                    tcp_dport=dst_port)
                send_packet(self, swports[0], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self,
                    self.exp_pkts(dst_ip_addr, src_port, dst_port),
                    [swports[1], swports[2], swports[3], swports[4]])
                count[rcv_idx] += 1
                dst_ip += 1
            # Make sure the live paths are equally balanced after the 2nd failed
            # path is activated again
            print "ECMP load balancing result after failed path 2 is activated", count
            for i in range(1, 4):
                self.assertTrue((count[i] >= ((num_sample_pkts / 3) * 0.7)),
                                "Not all paths are equally balanced")

            # Activate the previously failed path 1
            self.client.switch_api_l3_ecmp_member_activate(device, ecmp, 1,
                                                           [nhop1])
            count = [0, 0, 0, 0]
            for i in range(0, num_sample_pkts):
                dst_ip_addr = socket.inet_ntoa(
                    hex(dst_ip)[2:].zfill(8).decode('hex'))
                src_port = random.randint(0, 65535)
                dst_port = random.randint(0, 65535)
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src='192.168.8.1',
                    ip_id=106,
                    ip_ttl=64,
                    tcp_sport=src_port,
                    tcp_dport=dst_port)
                send_packet(self, swports[0], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self,
                    self.exp_pkts(dst_ip_addr, src_port, dst_port),
                    [swports[1], swports[2], swports[3], swports[4]])
                count[rcv_idx] += 1
                dst_ip += 1
            # Make sure the live paths are equally balanced after the failed
            # path 1 is activated again
            print "ECMP load balancing result after failed path 1 is activated", count
            for i in range(0, 4):
                self.assertTrue((count[i] >= ((num_sample_pkts / 4) * 0.7)),
                                "Not all paths are equally balanced")

        finally:
            self.client.switch_api_l3_route_delete(device, vrf, r_ip, ecmp)

            self.client.switch_api_ecmp_member_delete(
                device, ecmp, 4, [nhop1, nhop2, nhop3, nhop4])
            self.client.switch_api_ecmp_delete(device, ecmp)

            self.client.switch_api_neighbor_delete(device, neighbor1)
            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_neighbor_delete(device, neighbor2)
            self.client.switch_api_nhop_delete(device, nhop2)

            self.client.switch_api_neighbor_delete(device, neighbor3)
            self.client.switch_api_nhop_delete(device, nhop3)

            self.client.switch_api_neighbor_delete(device, neighbor4)
            self.client.switch_api_nhop_delete(device, nhop4)

            self.client.switch_api_l3_interface_address_delete(device, rif1,
                                                               vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(device, rif2,
                                                               vrf, i_ip2)
            self.client.switch_api_l3_interface_address_delete(device, rif3,
                                                               vrf, i_ip3)
            self.client.switch_api_l3_interface_address_delete(device, rif4,
                                                               vrf, i_ip4)
            self.client.switch_api_l3_interface_address_delete(device, rif5,
                                                               vrf, i_ip5)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)
            self.client.switch_api_interface_delete(device, if4)
            self.client.switch_api_interface_delete(device, if5)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)
            self.client.switch_api_rif_delete(0, rif3)
            self.client.switch_api_rif_delete(0, rif4)
            self.client.switch_api_rif_delete(0, rif5)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


@group('resilient')
class L2LagResilientTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        vlan = self.client.switch_api_vlan_create(device, 10)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port5 = self.client.switch_api_port_id_to_handle_get(device, swports[5])
        port6 = self.client.switch_api_port_id_to_handle_get(device, swports[6])
        port7 = self.client.switch_api_port_id_to_handle_get(device, swports[7])
        port8 = self.client.switch_api_port_id_to_handle_get(device, swports[8])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        lag = self.client.switch_api_lag_create(device)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port5)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port6)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port7)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port8)
        i_info2 = switcht_interface_info_t(
            handle=lag, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        self.client.switch_api_vlan_member_add(device, vlan, if1)
        self.client.switch_api_vlan_member_add(device, vlan, if2)

        switch_api_mac_table_entry_create(
            self, device, vlan, '00:11:11:11:11:11', 2, if2)
        switch_api_mac_table_entry_create(
            self, device, vlan, '00:22:22:22:22:22', 2, if1)

        self.client.switch_api_lag_member_deactivate(device, lag, swports[5])

        try:
            count = [0, 0, 0, 0]
            dst_ip = int(socket.inet_aton('10.10.10.1').encode('hex'), 16)
            num_sample_pkts = 200
            random.seed(314159)
            for i in range(0, num_sample_pkts):
                dst_ip_addr = socket.inet_ntoa(
                    hex(dst_ip)[2:].zfill(8).decode('hex'))
                pkt = simple_tcp_packet(
                    eth_dst='00:11:11:11:11:11',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src='192.168.8.1',
                    ip_id=109,
                    ip_ttl=64)

                exp_pkt = simple_tcp_packet(
                    eth_dst='00:11:11:11:11:11',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src='192.168.8.1',
                    ip_id=109,
                    ip_ttl=64)

                send_packet(self, swports[1], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self, [exp_pkt, exp_pkt, exp_pkt, exp_pkt],
                    [swports[5], swports[6], swports[7], swports[8]])
                count[rcv_idx] += 1
                dst_ip += 1

            print 'L2LagTest with 3 member ports:', count
            self.assertTrue(count[0] == 0)
            for i in range(1, 4):
                self.assertTrue((count[i] >= ((num_sample_pkts / 3) * 0.7)),
                                "Not all paths are equally balanced")

            self.client.switch_api_lag_member_activate(device, lag, port5)
            count = [0, 0, 0, 0]
            dst_ip = int(socket.inet_aton('10.10.10.1').encode('hex'), 16)
            for i in range(0, num_sample_pkts):
                dst_ip_addr = socket.inet_ntoa(
                    hex(dst_ip)[2:].zfill(8).decode('hex'))
                pkt = simple_tcp_packet(
                    eth_dst='00:11:11:11:11:11',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src='192.168.8.1',
                    ip_id=109,
                    ip_ttl=64)

                exp_pkt = simple_tcp_packet(
                    eth_dst='00:11:11:11:11:11',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src='192.168.8.1',
                    ip_id=109,
                    ip_ttl=64)

                send_packet(self, swports[1], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self, [exp_pkt, exp_pkt, exp_pkt, exp_pkt],
                    [swports[5], swports[6], swports[7], swports[8]])
                count[rcv_idx] += 1
                dst_ip += 1

            print 'L2LagTest: after adding another port', count
            for i in range(0, 4):
                self.assertTrue((count[i] >= ((num_sample_pkts / 4) * 0.7)),
                                "Not all paths are equally balanced")

        finally:
            switch_api_mac_table_entry_delete(self, device, vlan,
                                                          '00:22:22:22:22:22')
            switch_api_mac_table_entry_delete(self, device, vlan,
                                                          '00:11:11:11:11:11')
            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)

            self.client.switch_api_lag_member_delete(
                device, lag_handle=lag, side=0, port=port5)
            self.client.switch_api_lag_member_delete(
                device, lag_handle=lag, side=0, port=port6)
            self.client.switch_api_lag_member_delete(
                device, lag_handle=lag, side=0, port=port7)
            self.client.switch_api_lag_member_delete(
                device, lag_handle=lag, side=0, port=port8)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_lag_delete(device, lag)
            self.client.switch_api_vlan_delete(device, vlan)
