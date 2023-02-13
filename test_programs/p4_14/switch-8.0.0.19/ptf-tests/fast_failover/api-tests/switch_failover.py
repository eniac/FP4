"""
Flowlet swithcing tests
"""

import switchapi_thrift

import time
import sys
import logging

import unittest
import random

import pd_base_tests
try:
    import pltfm_pm_rpc
    from pltfm_pm_rpc.ttypes import *
except ImportError:
    pass

import ptf.dataplane as dataplane

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *

import os
import ptf.mask

from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

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


###############################################################################
@group('failover')
class L2LagFailoverTest(pd_base_tests.ThriftInterfaceDataPlane,
                        api_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print

        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        devport = []
        for i in range(0, 9):
            devport.append(swport_to_devport(self, swports[i]))
        cpu_port = get_cpu_port(self)

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
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

        pipe_id = get_pipeid(devport[5])
        port_num = devport[5]
        pkt = simple_pktgen_port_down_packet(
            app_id=0x1, pipe_id=pipe_id, port_num=port_num, packet_id=0)
        send_packet(self, swports[1], str(pkt))
        time.sleep(1)
        try:
            count = [0, 0, 0, 0]
            dst_ip = int(socket.inet_aton('10.10.10.1').encode('hex'), 16)
            max_itrs = 200
            random.seed(314259)
            for i in range(0, max_itrs):
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
                    self, [exp_pkt, exp_pkt, exp_pkt],
                    [swports[5], swports[6], swports[7], swports[8]])
                count[rcv_idx] += 1
                dst_ip += 1

            print 'L2LagTest:', count
            # No packet through the failed port
            self.assertTrue(count[0] == 0,
                            "Should not receive packet on the failed port")
            for i in range(1, 4):
                self.assertTrue((count[i] >= ((max_itrs / 3) * 0.7)),
                                "Not all paths are equally balanced")

            pkt = simple_tcp_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:22:22:22:22:22',
                ip_dst='10.0.0.1',
                ip_id=109,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_src='00:11:11:11:11:11',
                eth_dst='00:22:22:22:22:22',
                ip_dst='10.0.0.1',
                ip_id=109,
                ip_ttl=64)

            self.client.switch_api_lag_member_activate(device, lag, swports[5])

        finally:
            switch_api_mac_table_entry_delete(self, device, vlan,
                                                          '00:22:22:22:22:22')
            switch_api_mac_table_entry_delete(self, device, vlan,
                                                          '00:11:11:11:11:11')

            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)

            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port5)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port6)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port7)
            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port8)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)

            self.client.switch_api_lag_delete(device, lag)
            self.client.switch_api_vlan_delete(device, vlan)


class L3EcmpFailoverTest(api_base_tests.ThriftInterfaceDataPlane):
    def add_interface(self, port_num, ip_addr, rmac, vrf):
        port = self.client.switch_api_port_id_to_handle_get(device, port_num)
        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif = self.client.switch_api_rif_create(0, rif_info)
        info = switcht_interface_info_t(
            handle=port, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif)
        interface = self.client.switch_api_interface_create(device, info)
        ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr=ip_addr, prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif, vrf, ip)
        return ip, interface, rif, port

    def runTest(self):
        print
        print "Sending packet port %d" % swports[1], " -> port %d" % swports[
            2], " (192.168.0.1 -> 10.0.0.1 [id = 101])"

        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        i_ip1, if1, rif1, port1 = self.add_interface(swports[0], '192.168.0.2',
                                                     rmac, vrf)
        i_ip2, if2, rif2, port2 = self.add_interface(swports[1], '10.0.0.2',
                                                     rmac, vrf)
        i_ip3, if3, rif3, port3 = self.add_interface(swports[2], '11.0.0.2',
                                                     rmac, vrf)

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

        ecmp = self.client.switch_api_ecmp_create(device)
        self.client.switch_api_ecmp_member_add(device, ecmp, 2, [nhop1, nhop2])

        r_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.0.0',
            prefix_length=16)
        self.client.switch_api_l3_route_add(device, vrf, r_ip, ecmp)

        pkt = simple_pktgen_recirc_packet(
            app_id=0x2, pipe_id=0, key=nhop1, packet_id=0x0)
        send_packet(self, swports[1], str(pkt))
        time.sleep(1)

        try:
            count = [0, 0]
            dst_ip = int(socket.inet_aton('10.10.10.1').encode('hex'), 16)
            max_itrs = 20
            random.seed(314259)
            for i in range(0, max_itrs):
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
                send_packet(self, swports[0], str(pkt))
                rcv_idx = verify_any_packet_any_port(self, [exp_pkt1, exp_pkt2],
                                                     [swports[1], swports[2]])
                count[rcv_idx] += 1
                dst_ip += 1

            print "ECMP load balancing result ", count, "(first member should not be active)"
            self.assertTrue(count[0] == 0)
            self.assertTrue(count[1] == max_itrs)

        finally:
            self.client.switch_api_l3_route_delete(device, vrf, r_ip, ecmp)

            self.client.switch_api_ecmp_member_delete(device, ecmp, 2,
                                                      [nhop1, nhop2])
            self.client.switch_api_ecmp_delete(device, ecmp)

            self.client.switch_api_neighbor_delete(device, neighbor1)
            self.client.switch_api_nhop_delete(device, nhop1)

            self.client.switch_api_neighbor_delete(device, neighbor2)
            self.client.switch_api_nhop_delete(device, nhop2)

            self.client.switch_api_l3_interface_address_delete(device, rif1,
                                                               vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(device, rif2,
                                                               vrf, i_ip2)
            self.client.switch_api_l3_interface_address_delete(device, rif3,
                                                               vrf, i_ip3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


class L3EcmpLagFailoverTest(pd_base_tests.ThriftInterfaceDataPlane,
                            api_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print
        print "Sending packet port %d" % swports[1], " -> ecmp -> lag"

        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        devport = []
        for i in range(0, 7):
            devport.append(swport_to_devport(self, swports[i]))
        cpu_port = get_cpu_port(self)

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port5 = self.client.switch_api_port_id_to_handle_get(device, swports[5])
        port6 = self.client.switch_api_port_id_to_handle_get(device, swports[6])

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info)
        rif3 = self.client.switch_api_rif_create(0, rif_info)
        rif4 = self.client.switch_api_rif_create(0, rif_info)

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)
        i_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='192.168.0.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif1, vrf,
                                                        i_ip1)

        lag1 = self.client.switch_api_lag_create(device)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag1, side=SWITCH_API_DIRECTION_BOTH, port=port5)
        i_info3 = switcht_interface_info_t(
            handle=lag1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
        if3 = self.client.switch_api_interface_create(device, i_info3)
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.3.2',
            prefix_length=24)
        self.client.switch_api_l3_interface_address_add(device, rif3, vrf,
                                                        i_ip3)

        i_info4 = switcht_interface_info_t(
            handle=port6, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif4)
        if4 = self.client.switch_api_interface_create(device, i_info4)
        i_ip4 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.4.2',
            prefix_length=24)
        self.client.switch_api_l3_interface_address_add(device, rif4, vrf,
                                                        i_ip4)

        i_ip5 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.100.0.0',
            prefix_length=16)
        nhop2, neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, rif3, i_ip5, '00:11:22:33:44:56')
        nhop3, neighbor3 = switch_api_l3_nhop_neighbor_create(self, device, rif4, i_ip5, '00:11:22:33:44:57')

        ecmp = self.client.switch_api_ecmp_create(device)
        self.client.switch_api_ecmp_member_add(device, ecmp, 2, [nhop2, nhop3])

        self.client.switch_api_l3_route_add(device, vrf, i_ip5, ecmp)

        pipe_id = get_pipeid(devport[5])
        port_num = devport[5]
        pkt = simple_pktgen_port_down_packet(
            app_id=0x1, pipe_id=pipe_id, port_num=port_num, packet_id=0)
        send_packet(self, swports[1], str(pkt))
        time.sleep(1)

        try:
            count = [0, 0]
            dst_ip = int(socket.inet_aton('10.100.10.4').encode('hex'), 16)
            max_itrs = 50
            random.seed(314259)
            member_is_active = True
            for i in range(0, max_itrs):
                dst_ip_addr = socket.inet_ntoa(
                    hex(dst_ip)[2:].zfill(8).decode('hex'))
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src='192.168.8.1',
                    ip_id=106,
                    ip_ttl=64)

                exp_pkt1 = simple_tcp_packet(
                    eth_dst='00:11:22:33:44:56',
                    eth_src='00:77:66:55:44:33',
                    ip_dst=dst_ip_addr,
                    ip_src='192.168.8.1',
                    ip_id=106,
                    ip_ttl=63)
                exp_pkt2 = simple_tcp_packet(
                    eth_dst='00:11:22:33:44:57',
                    eth_src='00:77:66:55:44:33',
                    ip_dst=dst_ip_addr,
                    ip_src='192.168.8.1',
                    ip_id=106,
                    ip_ttl=63)

                send_packet(self, swports[1], str(pkt))
                if member_is_active:
                    (_, rcv_port, rcv_pkt, _) = dp_poll(
                        self, device_number=device, timeout=1)

                    if rcv_pkt != None:
                        self.assertTrue(rcv_port == swports[6])
                        count[1] += 1
                    else:
                        # Packet that deactivates the ECMP member will get dropped
                        member_is_active = False
                else:
                    rcv_idx = verify_any_packet_any_port(
                        self, [exp_pkt1, exp_pkt2], [swports[5], swports[6]])
                    count[rcv_idx] += 1
                dst_ip += 1

            print 'ECMP-count:', count
            self.assertTrue(count[0] == 0,
                            "Should not any packets on the failed member")
            self.assertTrue(count[1] == max_itrs - 1,
                            "Firs packet should've dropped")

        finally:
            self.client.switch_api_l3_route_delete(device, vrf, i_ip5, ecmp)

            self.client.switch_api_ecmp_member_delete(device, ecmp, 2,
                                                      [nhop2, nhop3])
            self.client.switch_api_ecmp_delete(device, ecmp)

            self.client.switch_api_neighbor_delete(device, neighbor2)
            self.client.switch_api_nhop_delete(device, nhop2)

            self.client.switch_api_neighbor_delete(device, neighbor3)
            self.client.switch_api_nhop_delete(device, nhop3)

            self.client.switch_api_lag_member_delete(
                device,
                lag_handle=lag1,
                side=SWITCH_API_DIRECTION_BOTH,
                port=port5)

            self.client.switch_api_l3_interface_address_delete(device, rif1,
                                                               vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(device, rif3,
                                                               vrf, i_ip3)
            self.client.switch_api_l3_interface_address_delete(device, rif4,
                                                               vrf, i_ip4)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if3)
            self.client.switch_api_interface_delete(device, if4)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif3)
            self.client.switch_api_rif_delete(0, rif4)

            self.client.switch_api_lag_delete(device, lag1)

            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)
