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


###############################################################################
@group('nat')
class NatTest(api_base_tests.ThriftInterfaceDataPlane):
    def setUp(self):
        print
        print 'Configuring devices for NAT test cases'

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.port1 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[1])
        self.port2 = self.client.switch_api_port_id_to_handle_get(device,
                                                                  swports[2])

        self.vrf = self.client.switch_api_vrf_create(device, 2)
        self.rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, self.rmac,
                                              '00:77:66:55:44:33')

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            nat_mode=SWITCH_NAT_MODE_INNER,
            v4_unicast_enabled=True)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            nat_mode=SWITCH_NAT_MODE_OUTER,
            v4_unicast_enabled=True)

        self.rif1 = self.client.switch_api_rif_create(0, rif_info1)
        self.rif2 = self.client.switch_api_rif_create(0, rif_info2)

        # create l3 inside interface
        info = switcht_interface_info_t(
            handle=self.port1,
            type=SWITCH_INTERFACE_TYPE_PORT,
            rif_handle=self.rif1)
        self.if1 = self.client.switch_api_interface_create(device, info)

        self.ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.1.1',
            prefix_length=24)
        self.client.switch_api_l3_interface_address_add(device, self.rif1,
                                                        self.vrf, self.ip1)

        # create l3 outside interface
        info = switcht_interface_info_t(
            handle=self.port2,
            type=SWITCH_INTERFACE_TYPE_PORT,
            rif_handle=self.rif2)
        self.if2 = self.client.switch_api_interface_create(device, info)
        self.ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.0.2.1',
            prefix_length=24)
        self.client.switch_api_l3_interface_address_add(device, self.rif2,
                                                        self.vrf, self.ip2)

        # add ipv4 static route
        self.ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='10.10.10.1',
            prefix_length=32)
        self.nhop1, self.neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, self.rif2, self.ip3, '00:22:22:22:22:22')
        self.client.switch_api_l3_route_add(device, self.vrf, self.ip3,
                                            self.nhop1)

        # create nexthop for dst NAT
        self.nhop2, self.neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, self.rif1, self.ip3, '00:11:11:11:11:11')

        # create NAT ACL (permit all)
        self.acl1 = self.client.switch_api_acl_list_create(
            device, SWITCH_API_DIRECTION_INGRESS, 0, SWITCH_HANDLE_TYPE_PORT)
        kvp = []
        kvp_val = switcht_acl_value_t(value_num=int("0a0a0a00", 16))
        kvp_mask = switcht_acl_value_t(value_num=int("ffffff00", 16))
        kvp.append(
            switcht_acl_key_value_pair_t(SWITCH_ACL_IP_FIELD_IPV4_DEST, kvp_val,
                                         kvp_mask))
        acl_priority = 10
        action = 2
        action_params = switcht_acl_action_params_t()
        opt_action_params = switcht_acl_opt_action_params_t(
            nat_mode=SWITCH_NAT_MODE_INNER)
        self.ace1 = self.client.switch_api_acl_ip_rule_create(
            device, self.acl1, acl_priority,
            len(kvp), kvp, action, action_params, opt_action_params)
        self.client.switch_api_acl_reference(device, self.acl1, self.port1)

        self.acl2 = self.client.switch_api_acl_list_create(
            device, SWITCH_API_DIRECTION_INGRESS, 0, SWITCH_HANDLE_TYPE_PORT)
        kvp = []
        kvp_val = switcht_acl_value_t(value_num=int("0a0a0a00", 16))
        kvp_mask = switcht_acl_value_t(value_num=int("ffffff00", 16))
        kvp.append(
            switcht_acl_key_value_pair_t(SWITCH_ACL_IP_FIELD_IPV4_SRC, kvp_val,
                                         kvp_mask))
        acl_priority = 10
        action = 2
        action_params = switcht_acl_action_params_t()
        opt_action_params = switcht_acl_opt_action_params_t(
            nat_mode=SWITCH_NAT_MODE_OUTER)
        self.ace2 = self.client.switch_api_acl_ip_rule_create(
            device, self.acl2, acl_priority,
            len(kvp), kvp, action, action_params, opt_action_params)
        self.client.switch_api_acl_reference(device, self.acl2, self.port2)
        # create NAT bindings
        self.nat = []
        nat_bindings = [
            [
                self.vrf, 0, 0, '192.168.0.1', 0x0, '0.0.0.0', 0x0, 6,
                '172.16.55.56', 0x0, '0.0.0.0', 0x0
            ],
            [
                self.vrf, self.nhop2, 1, '0.0.0.0', 0x0, '172.16.55.56', 0x0, 6,
                '0.0.0.0', 0x0, '192.168.0.1', 0x0
            ],
            [
                self.vrf, self.nhop1, 2, '192.168.0.2', 0x0, '10.10.10.2', 0x0,
                6, '172.16.55.57', 0x0, '10.100.1.2', 0x0
            ],
            [
                self.vrf, 0, 6, '192.168.0.1', 0x1289, '0.0.0.0', 0x0, 6,
                '172.16.55.55', 0x3456, '0.0.0.0', 0x0
            ],
            [
                self.vrf, self.nhop2, 7, '0.0.0.0', 0x0, '172.16.55.55', 0x3456,
                6, '0.0.0.0', 0x0, '192.168.0.1', 0x1289
            ],
            [
                self.vrf, self.nhop1, 8, '192.168.0.2', 0x789a, '10.10.10.2',
                0xabcd, 6, '172.16.55.58', 0x5678, '10.100.1.3', 0x3489
            ],
        ]
        for b in nat_bindings:
            vrf_h = b[0]
            nhop_h = b[1]
            nat_type = b[2]
            sip = switcht_ip_addr_t(
                addr_type=SWITCH_API_IP_ADDR_V4, ipaddr=b[3], prefix_length=32)
            sport = b[4]
            dip = switcht_ip_addr_t(
                addr_type=SWITCH_API_IP_ADDR_V4, ipaddr=b[5], prefix_length=0)
            dport = b[6]
            proto = b[7]
            rw_sip = switcht_ip_addr_t(
                addr_type=SWITCH_API_IP_ADDR_V4, ipaddr=b[8], prefix_length=32)
            rw_sport = b[9]
            rw_dip = switcht_ip_addr_t(
                addr_type=SWITCH_API_IP_ADDR_V4, ipaddr=b[10], prefix_length=0)
            rw_dport = b[11]
            nb = switcht_nat_info_t(
                nat_rw_type=nat_type,
                src_ip=sip,
                dst_ip=dip,
                src_port=sport,
                dst_port=dport,
                protocol=proto,
                rw_src_ip=rw_sip,
                rw_dst_ip=rw_dip,
                rw_src_port=rw_sport,
                rw_dst_port=rw_dport,
                vrf_handle=vrf_h,
                nhop_handle=nhop_h)
            self.client.switch_api_nat_create(device, nb)
            self.nat.append(nb)

    def runTest(self):

        skip_checksum_validation = False
        print "Verifying Source NAT (ip and port)"
        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:11:11:11:11:11',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_ttl=64,
            tcp_sport=0x1289)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:22:22:22:22:22',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.10.10.1',
            ip_src='172.16.55.55',
            ip_id=105,
            ip_ttl=63,
            tcp_sport=0x3456)
        send_packet(self, swports[1], str(pkt))
        if (skip_checksum_validation):
            exp_pkt = ptf.mask.Mask(exp_pkt)
            exp_pkt.set_do_not_care_scapy(TCP, 'chksum')
        verify_packets(self, exp_pkt, [swports[2]])

        print "Verifying Source NAT (ip)"
        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:11:11:11:11:11',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:22:22:22:22:22',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.10.10.1',
            ip_src='172.16.55.56',
            ip_id=105,
            ip_ttl=63)
        send_packet(self, swports[1], str(pkt))
        if (skip_checksum_validation):
            exp_pkt = ptf.mask.Mask(exp_pkt)
            exp_pkt.set_do_not_care_scapy(TCP, 'chksum')
        verify_packets(self, exp_pkt, [swports[2]])

        print "Verifying Destination NAT (ip and port)"
        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:22:22:22:22:22',
            ip_dst='172.16.55.55',
            ip_src='10.10.10.1',
            ip_id=105,
            ip_ttl=64,
            tcp_dport=0x3456)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:77:66:55:44:33',
            ip_dst='192.168.0.1',
            ip_src='10.10.10.1',
            ip_id=105,
            ip_ttl=63,
            tcp_dport=0x1289)
        send_packet(self, swports[2], str(pkt))
        if (skip_checksum_validation):
            exp_pkt = ptf.mask.Mask(exp_pkt)
            exp_pkt.set_do_not_care_scapy(TCP, 'chksum')
        verify_packets(self, exp_pkt, [swports[1]])

        print "Verifying Destination NAT (ip)"
        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:22:22:22:22:22',
            ip_dst='172.16.55.56',
            ip_src='10.10.10.1',
            ip_id=105,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:11:11:11:11:11',
            eth_src='00:77:66:55:44:33',
            ip_dst='192.168.0.1',
            ip_src='10.10.10.1',
            ip_id=105,
            ip_ttl=63)
        send_packet(self, swports[2], str(pkt))
        if (skip_checksum_validation):
            exp_pkt = ptf.mask.Mask(exp_pkt)
            exp_pkt.set_do_not_care_scapy(TCP, 'chksum')
        verify_packets(self, exp_pkt, [swports[1]])

        print "Verifying Twice NAT (ip and port)"
        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:11:11:11:11:11',
            ip_dst='10.10.10.2',
            ip_src='192.168.0.2',
            ip_id=105,
            ip_ttl=64,
            tcp_sport=0x789a,
            tcp_dport=0xabcd)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:22:22:22:22:22',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.100.1.3',
            ip_src='172.16.55.58',
            ip_id=105,
            ip_ttl=63,
            tcp_sport=0x5678,
            tcp_dport=0x3489)
        send_packet(self, swports[1], str(pkt))
        if (skip_checksum_validation):
            exp_pkt = ptf.mask.Mask(exp_pkt)
            exp_pkt.set_do_not_care_scapy(TCP, 'chksum')
        verify_packets(self, exp_pkt, [swports[2]])

        print "Verifying Twice NAT (ip)"
        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:11:11:11:11:11',
            ip_dst='10.10.10.2',
            ip_src='192.168.0.2',
            ip_id=105,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:22:22:22:22:22',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.100.1.2',
            ip_src='172.16.55.57',
            ip_id=105,
            ip_ttl=63)
        send_packet(self, swports[1], str(pkt))
        if (skip_checksum_validation):
            exp_pkt = ptf.mask.Mask(exp_pkt)
            exp_pkt.set_do_not_care_scapy(TCP, 'chksum')
        verify_packets(self, exp_pkt, [swports[2]])

    def tearDown(self):
        self.client.switch_api_l3_route_delete(device, self.vrf, self.ip3,
                                               self.nhop1)
        self.client.switch_api_neighbor_delete(device, self.neighbor1)
        self.client.switch_api_neighbor_delete(device, self.neighbor2)
        self.client.switch_api_nhop_delete(device, self.nhop1)
        self.client.switch_api_nhop_delete(device, self.nhop2)

        for n in self.nat:
            self.client.switch_api_nat_delete(device, n)

        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.ip1)
        self.client.switch_api_l3_interface_address_delete(device, self.rif2,
                                                           self.vrf, self.ip2)

        self.client.switch_api_acl_dereference(device, self.acl1, self.port1)
        self.client.switch_api_acl_rule_delete(device, self.acl1, self.ace1)
        self.client.switch_api_acl_list_delete(device, self.acl1)

        self.client.switch_api_acl_dereference(device, self.acl2, self.port2)
        self.client.switch_api_acl_rule_delete(device, self.acl2, self.ace2)
        self.client.switch_api_acl_list_delete(device, self.acl2)

        self.client.switch_api_interface_delete(device, self.if1)
        self.client.switch_api_interface_delete(device, self.if2)

        self.client.switch_api_rif_delete(0, self.rif1)
        self.client.switch_api_rif_delete(0, self.rif2)

        self.client.switch_api_router_mac_delete(device, self.rmac,
                                                 '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(device, self.rmac)
        self.client.switch_api_vrf_delete(device, self.vrf)
        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)
