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

import ptf.dataplane as dataplane

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *

import os
from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../base'))
from common.utils import *
from common.api_utils import *
sys.path.append(os.path.join(this_dir, '../../base/api-tests'))
import api_base_tests

from erspan3 import *

this_dir = os.path.dirname(os.path.abspath(__file__))

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)

if swports == []:
    swports = [x for x in range(0,16, 1)]

################################################################################
@group('egress_acl')
@group('acl')
@group('maxsizes')
@group('ent')
class IPEgressAclTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):

        print "Sending packet port %d" % swports[1], \
            "  -> port %d" % swports[2],"  (192.168.0.1 -> 10.0.0.1 [id = 101])"
        vrf = self.client.switch_api_vrf_create(0, 2)

        #import pdb;
        #pdb.set_trace()

        rmac = self.client.switch_api_router_mac_group_create(
            0, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(0, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(0, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(0, swports[2])

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info)
        rif2 = self.client.switch_api_rif_create(0, rif_info)

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(0, i_info1)
        i_ip1 = switcht_ip_addr_t(ipaddr='192.168.0.2', prefix_length=16)
        self.client.switch_api_l3_interface_address_add(0, rif1, vrf, i_ip1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(0, i_info2)
        i_ip2 = switcht_ip_addr_t(ipaddr='10.0.0.2', prefix_length=16)
        self.client.switch_api_l3_interface_address_add(0, rif2, vrf, i_ip2)

        # Add a static route
        i_ip3 = switcht_ip_addr_t(ipaddr='10.10.10.1', prefix_length=32)
        nhop, neighbor = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip3, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(0, vrf, i_ip3, nhop)

        # send the test packet(s)
        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_ttl=64)
        send_packet(self, swports[1], str(pkt))

        exp_pkt = simple_tcp_packet(
            eth_dst='00:11:22:33:44:55',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_ttl=63)
        verify_packets(self, exp_pkt, [swports[2]])

        # setup a deny ACL to verify that the same packet does not make it
        # ip acl
        acl = self.client.switch_api_acl_list_create(
            0, SWITCH_API_DIRECTION_EGRESS, SWITCH_ACL_TYPE_EGRESS_IP_ACL,
            SWITCH_HANDLE_TYPE_PORT)
        # create kvp to match destination IP
        kvp = []
        kvp_val = switcht_acl_value_t(value_num=int("0a0a0a01", 16))
        kvp_mask = switcht_acl_value_t(value_num=int("ffffffff", 16))
        kvp.append(
            switcht_acl_key_value_pair_t(SWITCH_ACL_IP_FIELD_IPV4_DEST, kvp_val,
                                         kvp_mask))
        action = SWITCH_ACL_ACTION_DROP
        action_params = switcht_acl_action_params_t(
            redirect=switcht_acl_action_drop(reason_code=0))
        opt_action_params = switcht_acl_opt_action_params_t()
        ace = self.client.switch_api_acl_ip_rule_create(
            0, acl, 10, 1, kvp, action, action_params, opt_action_params)
        self.client.switch_api_acl_reference(0, acl, port2)
        send_packet(self, swports[1], str(pkt))

        # check for absence of packet here!
        try:
            verify_packets(self, exp_pkt, [swports[2]])
            print 'FAILED - did not expect packet'
        except:
            print 'Success'

        # ip_acl
        self.client.switch_api_acl_dereference(0, acl, port2)
        self.client.switch_api_acl_rule_delete(0, acl, ace)
        self.client.switch_api_acl_list_delete(0, acl)

        #cleanup
        self.client.switch_api_neighbor_delete(0, neighbor)
        self.client.switch_api_nhop_delete(0, nhop)
        self.client.switch_api_l3_route_delete(0, vrf, i_ip3, if2)

        self.client.switch_api_l3_interface_address_delete(0, rif1, vrf, i_ip1)
        self.client.switch_api_l3_interface_address_delete(0, rif2, vrf, i_ip2)

        self.client.switch_api_interface_delete(0, if1)
        self.client.switch_api_interface_delete(0, if2)

        self.client.switch_api_rif_delete(0, rif1)
        self.client.switch_api_rif_delete(0, rif2)

        self.client.switch_api_router_mac_delete(0, rmac, '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(0, rmac)
        self.client.switch_api_vrf_delete(0, vrf)


################################################################################
@group('egress_acl')
@group('acl')
@group('maxsizes')
class IPEgressAclRangeTcamTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):

        print "Sending packet port %d" % swports[1], \
            "  -> port %d" % swports[2], " (192.168.0.1 -> 10.0.0.1 [id = 101])"
        vrf = self.client.switch_api_vrf_create(0, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            0, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(0, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(0, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(0, swports[2])

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info)
        rif2 = self.client.switch_api_rif_create(0, rif_info)

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(0, i_info1)
        i_ip1 = switcht_ip_addr_t(ipaddr='192.168.0.2', prefix_length=16)
        self.client.switch_api_l3_interface_address_add(0, rif1, vrf, i_ip1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(0, i_info2)
        i_ip2 = switcht_ip_addr_t(ipaddr='10.0.0.2', prefix_length=16)
        self.client.switch_api_l3_interface_address_add(0, rif2, vrf, i_ip2)

        # Add a static route
        i_ip3 = switcht_ip_addr_t(ipaddr='10.10.10.1', prefix_length=32)
        nhop, neighbor = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip3, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(0, vrf, i_ip3, nhop)

        # send the test packet(s)
        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            tcp_dport=1500,
            ip_id=105,
            ip_ttl=64)
        send_packet(self, swports[1], str(pkt))

        exp_pkt = simple_tcp_packet(
            eth_dst='00:11:22:33:44:55',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            tcp_dport=1500,
            ip_ttl=63)
        verify_packets(self, exp_pkt, [swports[2]])

        switch_range = switcht_range_t(start_value=1000, end_value=2000)
        acl_range_handle = self.client.switch_api_acl_range_create(
            0, SWITCH_API_DIRECTION_EGRESS, SWITCH_RANGE_TYPE_DST_PORT,
            switch_range)

        # setup a deny ACL to verify that the same packet does not make it
        # ip acl
        acl = self.client.switch_api_acl_list_create(
            0, SWITCH_API_DIRECTION_EGRESS, SWITCH_ACL_TYPE_EGRESS_IP_ACL,
            SWITCH_HANDLE_TYPE_PORT)
        # create kvp to match destination IP
        kvp = []
        kvp_val = switcht_acl_value_t(value_num=acl_range_handle)
        kvp_mask = switcht_acl_value_t(value_num=0xffffffff)
        kvp.append(
            switcht_acl_key_value_pair_t(SWITCH_ACL_IP_FIELD_L4_DEST_PORT_RANGE,
                                         kvp_val, kvp_mask))
        action = SWITCH_ACL_ACTION_DROP
        action_params = switcht_acl_action_params_t(
            redirect=switcht_acl_action_redirect(handle=0))
        opt_action_params = switcht_acl_opt_action_params_t()
        ace = self.client.switch_api_acl_ip_rule_create(
            0, acl, 10, 1, kvp, action, action_params, opt_action_params)
        self.client.switch_api_acl_reference(0, acl, port2)
        send_packet(self, swports[1], str(pkt))

        # check for absence of packet here!
        try:
            verify_packets(self, exp_pkt, [swports[2]])
            print 'FAILED - did not expect packet'
        except:
            print 'Success'

        # ip_acl
        self.client.switch_api_acl_dereference(0, acl, port2)
        self.client.switch_api_acl_rule_delete(0, acl, ace)
        self.client.switch_api_acl_list_delete(0, acl)
        self.client.switch_api_acl_range_delete(0, acl_range_handle)

        #cleanup
        self.client.switch_api_neighbor_delete(0, neighbor)
        self.client.switch_api_nhop_delete(0, nhop)
        self.client.switch_api_l3_route_delete(0, vrf, i_ip3, if2)

        self.client.switch_api_l3_interface_address_delete(0, rif1, vrf, i_ip1)
        self.client.switch_api_l3_interface_address_delete(0, rif2, vrf, i_ip2)

        self.client.switch_api_interface_delete(0, if1)
        self.client.switch_api_interface_delete(0, if2)

        self.client.switch_api_rif_delete(0, rif1)
        self.client.switch_api_rif_delete(0, rif2)

        self.client.switch_api_router_mac_delete(0, rmac, '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(0, rmac)
        self.client.switch_api_vrf_delete(0, vrf)
