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

from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

from erspan3 import *

this_dir = os.path.dirname(os.path.abspath(__file__))

sys.path.append(os.path.join(this_dir, '../../base'))
from common.utils import *
from common.api_utils import *
sys.path.append(os.path.join(this_dir, '../../base/api-tests'))
import api_base_tests

device=0
cpu_port=64
swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)

if swports == []:
    swports = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]
invalid_hdl = -1


###############################################################################
@group('qos')
class L3IPv4QosDscpRewriteTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Sending packet port %d" % swports[1], "  -> port %d" % swports[
            2], "  (192.168.0.1 -> 10.0.0.1 [id = 101])"
        vrf = self.client.switch_api_vrf_create(0, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(0, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(0, i_info1)
        i_ip1 = switcht_ip_addr_t(ipaddr='192.168.0.2', prefix_length=16)
        self.client.switch_api_l3_interface_address_add(0, rif1, vrf, i_ip1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(ipaddr='10.0.0.2', prefix_length=16)
        self.client.switch_api_l3_interface_address_add(0, rif2, vrf, i_ip2)

        # Add a static route
        i_ip3 = switcht_ip_addr_t(ipaddr='10.10.10.1', prefix_length=32)
        nhop, neighbor = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip3, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(0, vrf, i_ip3, nhop)

        qos_map_configured = False

        try:
            # send test packet before qos maps are configured
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_tos=24,
                ip_ttl=64)
            send_packet(self, swports[1], str(pkt))

            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_tos=24,
                ip_ttl=63)
            verify_packets(self, exp_pkt, [swports[2]])
            print "pass packet before qos maps are configured"

            qos_map1 = switcht_qos_map_t(dscp=1, tc=20)
            qos_map2 = switcht_qos_map_t(dscp=2, tc=24)
            qos_map3 = switcht_qos_map_t(dscp=3, tc=28)
            qos_map4 = switcht_qos_map_t(dscp=4, tc=32)
            ingress_qos_map_list = [qos_map1, qos_map2, qos_map3, qos_map4]
            ingress_qos_handle = self.client.switch_api_qos_map_ingress_create(
                device=0,
                qos_map_type=SWITCH_QOS_MAP_INGRESS_DSCP_TO_TC,
                qos_map=ingress_qos_map_list)

            qos_map5 = switcht_qos_map_t(tc=20, icos=1)
            qos_map6 = switcht_qos_map_t(tc=24, icos=0)
            qos_map7 = switcht_qos_map_t(tc=28, icos=1)
            qos_map8 = switcht_qos_map_t(tc=32, icos=0)
            tc_qos_map_list = [qos_map5, qos_map6, qos_map7, qos_map8]
            tc_qos_handle = self.client.switch_api_qos_map_ingress_create(
                device=0,
                qos_map_type=SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS,
                qos_map=tc_qos_map_list)

            qos_map51 = switcht_qos_map_t(tc=20, qid=1)
            qos_map61 = switcht_qos_map_t(tc=24, qid=2)
            qos_map71 = switcht_qos_map_t(tc=28, qid=3)
            qos_map81 = switcht_qos_map_t(tc=32, qid=4)
            tc_queue_map_list = [qos_map51, qos_map61, qos_map71, qos_map81]
            tc_queue_handle = self.client.switch_api_qos_map_ingress_create(
                device=0,
                qos_map_type=SWITCH_QOS_MAP_INGRESS_TC_TO_QUEUE,
                qos_map=tc_queue_map_list)

            qos_map9 = switcht_qos_map_t(tc=20, dscp=9)
            qos_map10 = switcht_qos_map_t(tc=24, dscp=10)
            qos_map11 = switcht_qos_map_t(tc=28, dscp=11)
            qos_map12 = switcht_qos_map_t(tc=32, dscp=12)
            egress_qos_map_list = [qos_map9, qos_map10, qos_map11, qos_map12]
            egress_qos_handle = self.client.switch_api_qos_map_egress_create(
                device=0,
                qos_map_type=SWITCH_QOS_MAP_EGRESS_TC_TO_DSCP,
                qos_map=egress_qos_map_list)

            self.client.switch_api_port_qos_group_ingress_set(
                device=0, port_handle=port1, qos_handle=ingress_qos_handle)
            self.client.switch_api_port_qos_group_tc_set(
                device=0, port_handle=port1, qos_handle=tc_qos_handle)
            self.client.switch_api_port_qos_group_egress_set(
                device=0, port_handle=port1, qos_handle=egress_qos_handle)
            self.client.switch_api_port_trust_dscp_set(
                device=0, port_handle=port1, trust_dscp=True)

            self.client.switch_api_port_qos_group_ingress_set(
                device=0, port_handle=port2, qos_handle=ingress_qos_handle)
            self.client.switch_api_port_qos_group_tc_set(
                device=0, port_handle=port2, qos_handle=tc_qos_handle)
            self.client.switch_api_port_qos_group_egress_set(
                device=0, port_handle=port2, qos_handle=egress_qos_handle)
            self.client.switch_api_port_trust_dscp_set(
                device=0, port_handle=port2, trust_dscp=True)

            qos_map_configured = True

            # send the test packet(s)
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_tos=4,
                ip_ttl=64)
            send_packet(self, swports[1], str(pkt))

            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='192.168.0.1',
                ip_id=105,
                ip_tos=36,
                ip_ttl=63)
            verify_packets(self, exp_pkt, [swports[2]])
            print "pass packet w/ mapped dscp value 1 -> 9"

            # send test packet with different dscp value
            pkt[IP].tos = 12
            send_packet(self, swports[1], str(pkt))

            exp_pkt[IP].tos = 44
            verify_packets(self, exp_pkt, [swports[2]])
            print "pass packet w/ mapped dscp value 3 -> 11"

            # send test packet with unmapped dscp value
            pkt[IP].tos = 24
            send_packet(self, swports[1], str(pkt))

            exp_pkt[IP].tos = 24
            verify_packets(self, exp_pkt, [swports[2]])
            print "pass packet w/ unmapped dscp value 6"

        finally:
            #cleanup

            if qos_map_configured:
                self.client.switch_api_port_qos_group_ingress_set(
                    device=0, port_handle=port1, qos_handle=0)
                self.client.switch_api_port_qos_group_tc_set(
                    device=0, port_handle=port1, qos_handle=0)
                self.client.switch_api_port_qos_group_egress_set(
                    device=0, port_handle=port1, qos_handle=0)
                self.client.switch_api_port_trust_dscp_set(
                    device=0, port_handle=port1, trust_dscp=False)

                self.client.switch_api_port_qos_group_ingress_set(
                    device=0, port_handle=port2, qos_handle=0)
                self.client.switch_api_port_qos_group_tc_set(
                    device=0, port_handle=port2, qos_handle=0)
                self.client.switch_api_port_qos_group_egress_set(
                    device=0, port_handle=port2, qos_handle=0)
                self.client.switch_api_port_trust_dscp_set(
                    device=0, port_handle=port2, trust_dscp=False)

                self.client.switch_api_qos_map_ingress_delete(
                    device=0, qos_map_handle=ingress_qos_handle)
                self.client.switch_api_qos_map_ingress_delete(
                    device=0, qos_map_handle=tc_queue_handle)
                self.client.switch_api_qos_map_ingress_delete(
                    device=0, qos_map_handle=tc_qos_handle)
                self.client.switch_api_qos_map_egress_delete(
                    device=0, qos_map_handle=egress_qos_handle)

            self.client.switch_api_l3_route_delete(0, vrf, i_ip3, nhop)
            self.client.switch_api_neighbor_delete(0, neighbor)
            self.client.switch_api_nhop_delete(0, nhop)

            self.client.switch_api_l3_interface_address_delete(0, rif1, vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(0, rif2, vrf, i_ip2)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_interface_delete(0, if1)
            self.client.switch_api_interface_delete(0, if2)

            self.client.switch_api_router_mac_delete(0, rmac, '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(0, rmac)
            self.client.switch_api_vrf_delete(0, vrf)


###############################################################################
@group('qos')
class L3IPv4QosTosRewriteTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Sending packet port %d" % swports[1], "  -> port %d" % swports[
            2], "  (192.168.0.1 -> 10.0.0.1 [id = 101])"
        vrf = self.client.switch_api_vrf_create(0, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(0, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(0, i_info1)
        i_ip1 = switcht_ip_addr_t(ipaddr='192.168.0.2', prefix_length=16)
        self.client.switch_api_l3_interface_address_add(0, rif1, vrf, i_ip1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(ipaddr='10.0.0.2', prefix_length=16)
        self.client.switch_api_l3_interface_address_add(0, rif2, vrf, i_ip2)

        # Add a static route
        i_ip3 = switcht_ip_addr_t(ipaddr='10.10.10.1', prefix_length=32)
        nhop, neighbor = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip3, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(0, vrf, i_ip3, nhop)

        qos_map1 = switcht_qos_map_t(tos=1, tc=20)
        qos_map2 = switcht_qos_map_t(tos=2, tc=24)
        qos_map3 = switcht_qos_map_t(tos=3, tc=28)
        qos_map4 = switcht_qos_map_t(tos=4, tc=32)
        ingress_qos_map_list = [qos_map1, qos_map2, qos_map3, qos_map4]
        ingress_qos_handle = self.client.switch_api_qos_map_ingress_create(
            device=0,
            qos_map_type=SWITCH_QOS_MAP_INGRESS_TOS_TO_TC,
            qos_map=ingress_qos_map_list)

        qos_map5 = switcht_qos_map_t(tc=20, icos=1)
        qos_map6 = switcht_qos_map_t(tc=24, icos=0)
        qos_map7 = switcht_qos_map_t(tc=28, icos=1)
        qos_map8 = switcht_qos_map_t(tc=32, icos=0)
        tc_qos_map_list = [qos_map5, qos_map6, qos_map7, qos_map8]
        tc_qos_handle = self.client.switch_api_qos_map_ingress_create(
            device=0,
            qos_map_type=SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS,
            qos_map=tc_qos_map_list)

        qos_map9 = switcht_qos_map_t(tc=20, tos=9)
        qos_map10 = switcht_qos_map_t(tc=24, tos=10)
        qos_map11 = switcht_qos_map_t(tc=28, tos=11)
        qos_map12 = switcht_qos_map_t(tc=32, tos=12)
        egress_qos_map_list = [qos_map9, qos_map10, qos_map11, qos_map12]
        egress_qos_handle = self.client.switch_api_qos_map_egress_create(
            device=0,
            qos_map_type=SWITCH_QOS_MAP_EGRESS_TC_TO_TOS,
            qos_map=egress_qos_map_list)

        self.client.switch_api_port_qos_group_ingress_set(
            device=0, port_handle=port1, qos_handle=ingress_qos_handle)
        self.client.switch_api_port_qos_group_tc_set(
            device=0, port_handle=port1, qos_handle=tc_qos_handle)
        self.client.switch_api_port_qos_group_egress_set(
            device=0, port_handle=port1, qos_handle=egress_qos_handle)
        self.client.switch_api_port_trust_dscp_set(
            device=0, port_handle=port1, trust_dscp=True)

        self.client.switch_api_port_qos_group_ingress_set(
            device=0, port_handle=port2, qos_handle=ingress_qos_handle)
        self.client.switch_api_port_qos_group_tc_set(
            device=0, port_handle=port2, qos_handle=tc_qos_handle)
        self.client.switch_api_port_qos_group_egress_set(
            device=0, port_handle=port2, qos_handle=egress_qos_handle)
        self.client.switch_api_port_trust_dscp_set(
            device=0, port_handle=port2, trust_dscp=True)

        # send the test packet(s)
        # send the test packet(s)
        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_tos=4,
            ip_ttl=64)
        send_packet(self, swports[1], str(pkt))

        exp_pkt = simple_tcp_packet(
            eth_dst='00:11:22:33:44:55',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_tos=12,
            ip_ttl=63)
        verify_packets(self, exp_pkt, [swports[2]])

        #cleanup
        self.client.switch_api_port_qos_group_ingress_set(
            device=0, port_handle=port1, qos_handle=0)
        self.client.switch_api_port_qos_group_tc_set(
            device=0, port_handle=port1, qos_handle=0)
        self.client.switch_api_port_qos_group_egress_set(
            device=0, port_handle=port1, qos_handle=0)
        self.client.switch_api_port_trust_dscp_set(
            device=0, port_handle=port1, trust_dscp=False)

        self.client.switch_api_port_qos_group_ingress_set(
            device=0, port_handle=port2, qos_handle=0)
        self.client.switch_api_port_qos_group_tc_set(
            device=0, port_handle=port2, qos_handle=0)
        self.client.switch_api_port_qos_group_egress_set(
            device=0, port_handle=port2, qos_handle=0)
        self.client.switch_api_port_trust_dscp_set(
            device=0, port_handle=port2, trust_dscp=False)

        self.client.switch_api_qos_map_ingress_delete(
            device=0, qos_map_handle=ingress_qos_handle)
        self.client.switch_api_qos_map_ingress_delete(
            device=0, qos_map_handle=tc_qos_handle)
        self.client.switch_api_qos_map_egress_delete(
            device=0, qos_map_handle=egress_qos_handle)

        self.client.switch_api_l3_route_delete(0, vrf, i_ip3, nhop)
        self.client.switch_api_neighbor_delete(0, neighbor)
        self.client.switch_api_nhop_delete(0, nhop)

        self.client.switch_api_l3_interface_address_delete(0, rif1, vrf, i_ip1)
        self.client.switch_api_l3_interface_address_delete(0, rif2, vrf, i_ip2)

        self.client.switch_api_rif_delete(0, rif1)
        self.client.switch_api_rif_delete(0, rif2)

        self.client.switch_api_interface_delete(0, if1)
        self.client.switch_api_interface_delete(0, if2)

        self.client.switch_api_router_mac_delete(0, rmac, '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(0, rmac)
        self.client.switch_api_vrf_delete(0, vrf)


###############################################################################

@group('ent')
class L3IPv4QosDscpRewriteTest2(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        #QOS_CLASSIFICATION flag is not enabled for QOS_PROFILE, bypass
        #this test for now.
        return
        print "Sending packet port %d" % swports[1], "  -> port %d" % swports[
            2], "  (192.168.0.1 -> 10.0.0.1 [id = 101])"
        vrf = self.client.switch_api_vrf_create(0, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(0, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(0, i_info1)
        i_ip1 = switcht_ip_addr_t(ipaddr='192.168.0.2', prefix_length=16)
        self.client.switch_api_l3_interface_address_add(0, rif1, vrf, i_ip1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(device, i_info2)
        i_ip2 = switcht_ip_addr_t(ipaddr='10.0.0.2', prefix_length=16)
        self.client.switch_api_l3_interface_address_add(0, rif2, vrf, i_ip2)

        # Add a static route
        i_ip3 = switcht_ip_addr_t(ipaddr='10.10.10.1', prefix_length=32)
        nhop, neighbor = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip3, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(0, vrf, i_ip3, nhop)

        qos_map1 = switcht_qos_map_t(dscp=1,  qid=1, icos=1, tc=20)
        qos_map2 = switcht_qos_map_t(dscp=2,  qid=2, icos=0, tc=24)
        qos_map3 = switcht_qos_map_t(dscp=3,  qid=3, icos=1, tc=28)
        qos_map4 = switcht_qos_map_t(dscp=4,  qid=4, icos=0, tc=32)
        ingress_qos_map_list = [qos_map1, qos_map2, qos_map3, qos_map4]
        ingress_qos_handle = self.client.switch_api_qos_map_ingress_create(
            device=0,
            qos_map_type=SWITCH_QOS_MAP_INGRESS_DSCP_TO_QID_AND_TC,
            qos_map=ingress_qos_map_list)

        qos_map9 = switcht_qos_map_t(tc=20,  dscp=9)
        qos_map10 = switcht_qos_map_t(tc=24, dscp=10)
        qos_map11 = switcht_qos_map_t(tc=28, dscp=11)
        qos_map12 = switcht_qos_map_t(tc=32, dscp=12)
        egress_qos_map_list = [qos_map9, qos_map10, qos_map11, qos_map12]
        egress_qos_handle = self.client.switch_api_qos_map_egress_create(
            device=0,
            qos_map_type=SWITCH_QOS_MAP_EGRESS_TC_TO_DSCP,
            qos_map=egress_qos_map_list)

        self.client.switch_api_port_qos_group_ingress_set(
            device=0, port_handle=port1, qos_handle=ingress_qos_handle)
        self.client.switch_api_port_qos_group_egress_set(
            device=0, port_handle=port1, qos_handle=egress_qos_handle)
        self.client.switch_api_port_trust_dscp_set(
            device=0, port_handle=port1, trust_dscp=True)

        self.client.switch_api_port_qos_group_ingress_set(
            device=0, port_handle=port2, qos_handle=ingress_qos_handle)
        self.client.switch_api_port_qos_group_egress_set(
            device=0, port_handle=port2, qos_handle=egress_qos_handle)
        self.client.switch_api_port_trust_dscp_set(
            device=0, port_handle=port2, trust_dscp=True)

        # send the test packet(s)
        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_tos=4,
            ip_ttl=64)
        send_packet(self, swports[1], str(pkt))

        exp_pkt = simple_tcp_packet(
            eth_dst='00:11:22:33:44:55',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_tos=36,
            ip_ttl=63)
        verify_packets(self, exp_pkt, [swports[2]])

        #cleanup
        self.client.switch_api_port_qos_group_ingress_set(
            device=0, port_handle=port1, qos_handle=0)
        self.client.switch_api_port_qos_group_egress_set(
            device=0, port_handle=port1, qos_handle=0)
        self.client.switch_api_port_trust_dscp_set(
            device=0, port_handle=port1, trust_dscp=False)

        self.client.switch_api_port_qos_group_ingress_set(
            device=0, port_handle=port2, qos_handle=0)
        self.client.switch_api_port_qos_group_egress_set(
            device=0, port_handle=port2, qos_handle=0)
        self.client.switch_api_port_trust_dscp_set(
            device=0, port_handle=port2, trust_dscp=False)

        self.client.switch_api_qos_map_ingress_delete(
            device=0, qos_map_handle=ingress_qos_handle)
        self.client.switch_api_qos_map_egress_delete(
            device=0, qos_map_handle=egress_qos_handle)

        self.client.switch_api_neighbor_delete(0, neighbor)
        self.client.switch_api_nhop_delete(0, nhop)
        self.client.switch_api_l3_route_delete(0, vrf, i_ip3, if2)

        self.client.switch_api_l3_interface_address_delete(0, rif1, vrf, i_ip1)
        self.client.switch_api_l3_interface_address_delete(0, rif2, vrf, i_ip2)

        self.client.switch_api_rif_delete(0, rif1)
        self.client.switch_api_rif_delete(0, rif2)

        self.client.switch_api_interface_delete(0, if1)
        self.client.switch_api_interface_delete(0, if2)

        self.client.switch_api_router_mac_delete(0, rmac, '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(0, rmac)
        self.client.switch_api_vrf_delete(0, vrf)


###############################################################################

