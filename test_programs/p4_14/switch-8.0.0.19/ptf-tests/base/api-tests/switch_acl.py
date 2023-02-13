###############################################################################
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
import api_base_tests

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *

import os
from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *
this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '..'))
from common.api_utils import *
from common.api_adapter import ApiAdapter

from erspan3 import *

device = 0

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)

if swports == []:
    swports = [x for x in range(65)]


###############################################################################
@group('l3')
@group('acl')
@group('ent')
class IPAclTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Sending packet port %d" % swports[1], "  -> port %d" % swports[
            2], "  (192.168.0.1 -> 10.0.0.1 [id = 101])"
        vrf = self.client.switch_api_vrf_create(device, 2)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(0, rmac, '00:77:66:55:44:33')

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
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
            0, SWITCH_API_DIRECTION_INGRESS, 0, SWITCH_HANDLE_TYPE_PORT)
        # create kvp to match destination IP
        kvp = []
        kvp_val = switcht_acl_value_t(value_num=int("0a0a0a01", 16))
        kvp_mask = switcht_acl_value_t(value_num=int("ffffffff", 16))
        kvp.append(
            switcht_acl_key_value_pair_t(SWITCH_ACL_IP_FIELD_IPV4_DEST, kvp_val,
                                         kvp_mask))
        kvp_val = switcht_acl_value_t(value_num=int("2", 16))
        kvp_mask = switcht_acl_value_t(value_num=int("ff", 16))
        kvp.append(
            switcht_acl_key_value_pair_t(SWITCH_ACL_IP_FIELD_TCP_FLAGS, kvp_val,
                                         kvp_mask))
        action = 1
        action_params = switcht_acl_action_params_t(
            redirect=switcht_acl_action_redirect(handle=0))
        opt_action_params = switcht_acl_opt_action_params_t()
        ace = self.client.switch_api_acl_ip_rule_create(
            0, acl, 10, 2, kvp, action, action_params, opt_action_params)
        port = self.client.switch_api_port_id_to_handle_get(0, swports[1])
        self.client.switch_api_acl_reference(0, acl, port)
        send_packet(self, swports[1], str(pkt))

        # check for absence of packet here!
        try:
            verify_packets(self, exp_pkt, [swports[2]])
            print 'FAILED - did not expect packet'
        except:
            print 'Success'

        # ip_acl
        self.client.switch_api_acl_dereference(0, acl, port)
        self.client.switch_api_acl_rule_delete(0, acl, ace)
        self.client.switch_api_acl_list_delete(0, acl)

        #cleanup
        self.client.switch_api_l3_route_delete(0, vrf, i_ip3, nhop)
        self.client.switch_api_neighbor_delete(0, neighbor)
        self.client.switch_api_nhop_delete(0, nhop)

        self.client.switch_api_l3_interface_address_delete(0, rif1, vrf, i_ip1)
        self.client.switch_api_l3_interface_address_delete(0, rif2, vrf, i_ip2)

        self.client.switch_api_interface_delete(0, if1)
        self.client.switch_api_interface_delete(0, if2)

        self.client.switch_api_rif_delete(0, rif1)
        self.client.switch_api_rif_delete(0, rif2)

        self.client.switch_api_router_mac_delete(0, rmac, '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(0, rmac)
        self.client.switch_api_vrf_delete(0, vrf)


###############################################################################
@group('l3')
@group('acl')
@group('mirror')
@group('ent')
class MirrorAclTest_i2e(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Sending packet port %d" % swports[1], "  -> port %d" % swports[
            2], "  (192.168.0.1 -> 10.0.0.1 [id = 101])"
        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(0, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[4])

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

        # create a mirror session
        minfo1 = switcht_mirror_info_t(
            session_id=1,
            direction=1,
            egress_port_handle=port4,
            mirror_type=SWITCH_MIRROR_TYPE_LOCAL,
            session_type=0,
            cos=0,
            max_pkt_len=0,
            ttl=0,
            nhop_handle=0,
            span_mode=0)
        mirror1 = self.client.switch_api_mirror_session_create(0, minfo1)

        # setup a Mirror acl
        # ip acl
        print "Create Mirror ACL to mirror i2e from 1->4"
        acl = self.client.switch_api_acl_list_create(
            0, SWITCH_API_DIRECTION_INGRESS, 0, SWITCH_HANDLE_TYPE_PORT)
        # create kvp to match destination IP
        kvp = []
        kvp_val = switcht_acl_value_t(value_num=int("0a0a0a01", 16))
        kvp_mask = switcht_acl_value_t(value_num=int("ffffffff", 16))
        kvp.append(
            switcht_acl_key_value_pair_t(SWITCH_ACL_IP_FIELD_IPV4_DEST, kvp_val,
                                         kvp_mask))
        action = SWITCH_ACL_ACTION_SET_MIRROR
        action_params = switcht_acl_action_params_t()
        opt_action_params = switcht_acl_opt_action_params_t(
            mirror_handle=mirror1)
        ace = self.client.switch_api_acl_ip_rule_create(
            0, acl, 10, 1, kvp, action, action_params, opt_action_params)
        port = self.client.switch_api_port_id_to_handle_get(0, swports[1])
        self.client.switch_api_acl_reference(0, acl, port)

        # send the test packet(s)
        send_packet(self, swports[1], str(pkt))
        verify_packet(self, exp_pkt, swports[2])
        # verify mirrored packet
        verify_packet(self, pkt, swports[4])
        verify_no_other_packets(self)

        # delete the mirror sesion
        print "Delete Mirror ACL"
        self.client.switch_api_mirror_session_delete(0, mirror1)
        # clean-up test, make sure pkt is not mirrored after session is deleted
        send_packet(self, swports[1], str(pkt))
        verify_packet(self, exp_pkt, swports[2])
        verify_no_other_packets(self)
        # ip_acl cleanup
        self.client.switch_api_acl_dereference(0, acl, port)
        self.client.switch_api_acl_rule_delete(0, acl, ace)
        self.client.switch_api_acl_list_delete(0, acl)
        #cleanup
        self.client.switch_api_l3_route_delete(0, vrf, i_ip3, nhop)
        self.client.switch_api_neighbor_delete(0, neighbor)
        self.client.switch_api_nhop_delete(0, nhop)

        self.client.switch_api_l3_interface_address_delete(0, rif1, vrf, i_ip1)
        self.client.switch_api_l3_interface_address_delete(0, rif2, vrf, i_ip2)

        self.client.switch_api_interface_delete(0, if1)
        self.client.switch_api_interface_delete(0, if2)

        self.client.switch_api_rif_delete(0, rif1)
        self.client.switch_api_rif_delete(0, rif2)

        self.client.switch_api_router_mac_delete(0, rmac, '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(0, rmac)
        self.client.switch_api_vrf_delete(0, vrf)


###############################################################################
@group('acl')
@group('mirror')
@group('ent')
class MirrorSessionTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "create mirror sessions"
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])
        minfo1 = switcht_mirror_info_t(
            session_id=1,
            direction=1,
            egress_port_handle=port3,
            mirror_type=SWITCH_MIRROR_TYPE_LOCAL,
            session_type=0,
            cos=0,
            max_pkt_len=0,
            ttl=0,
            nhop_handle=0)
        mirror1 = self.client.switch_api_mirror_session_create(0, minfo1)
        minfo2 = switcht_mirror_info_t(
            session_id=101,
            direction=2,
            egress_port_handle=port3,
            mirror_type=SWITCH_MIRROR_TYPE_LOCAL,
            session_type=0,
            cos=0,
            max_pkt_len=0,
            ttl=0,
            nhop_handle=0)
        mirror2 = self.client.switch_api_mirror_session_create(0, minfo2)
        minfo3 = switcht_mirror_info_t(
            session_id=201,
            direction=3,
            egress_port_handle=port3,
            mirror_type=SWITCH_MIRROR_TYPE_LOCAL,
            session_type=0,
            cos=0,
            max_pkt_len=0,
            ttl=0,
            nhop_handle=0)
        mirror3 = self.client.switch_api_mirror_session_create(0, minfo3)
        print "delete mirror sessions"
        self.client.switch_api_mirror_session_delete(0, mirror1)
        self.client.switch_api_mirror_session_delete(0, mirror2)
        self.client.switch_api_mirror_session_delete(0, mirror3)
        # delete again -ve test
        self.client.switch_api_mirror_session_delete(0, mirror3)


###############################################################################
@group('l3')
@group('acl')
@group('mirror')
class MirrorAclTest_e2e(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test e2e Mirror packet port %d" % swports[
            1], "  -> port %d" % swports[
                2], "  (192.168.0.1 -> 10.0.0.1 [id = 101])"
        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(0, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])
        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[4])

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

        exp_pkt = simple_tcp_packet(
            eth_dst='00:11:22:33:44:55',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_ttl=63)
        # create a mirror session
        minfo1 = switcht_mirror_info_t(
            session_id=1,
            direction=2,
            egress_port_handle=port4,
            mirror_type=SWITCH_MIRROR_TYPE_LOCAL,
            session_type=0,
            cos=0,
            max_pkt_len=0x600,
            ttl=0,
            nhop_handle=0)
        mirror1 = self.client.switch_api_mirror_session_create(0, minfo1)

        # setup a egress Mirror acl
        print "Create Egress Mirror ACL to mirror e2e from %d -> %d" % (
            swports[2], swports[4])
        acl = self.client.switch_api_acl_list_create(
            0, SWITCH_API_DIRECTION_EGRESS, SWITCH_ACL_TYPE_EGRESS_SYSTEM,
            SWITCH_HANDLE_TYPE_PORT)
        # create kvp to match egress port and deflect bit
        kvp = []
        kvp_val = switcht_acl_value_t(value_num=port2)
        kvp_mask = switcht_acl_value_t(value_num=0xff)
        kvp.append(
            switcht_acl_key_value_pair_t(SWITCH_ACL_EGRESS_SYSTEM_FIELD_DEST_PORT,
                                         kvp_val, kvp_mask))
        kvp_val = switcht_acl_value_t(value_num=0)
        kvp_mask = switcht_acl_value_t(value_num=0xff)
        kvp.append(
            switcht_acl_key_value_pair_t(SWITCH_ACL_EGRESS_SYSTEM_FIELD_DEFLECT, kvp_val,
                                         kvp_mask))
        action = 1
        action_params = switcht_acl_action_params_t()
        opt_action_params = switcht_acl_opt_action_params_t(
            mirror_handle=mirror1)
        ace = self.client.switch_api_acl_egress_system_rule_create(
            0, acl, 11, 2, kvp, action, action_params, opt_action_params)
        send_packet(self, swports[1], str(pkt))
        verify_packet(self, exp_pkt, swports[2])
        verify_packet(self, exp_pkt, swports[4])
        verify_no_other_packets(self)

        # update the mirror sesion to different port
        print "Update Egress Mirror Session's egr_port to 3 and test packet again"
        minfo1 = switcht_mirror_info_t(
            session_id=1,
            direction=2,
            egress_port_handle=port3,
            mirror_type=SWITCH_MIRROR_TYPE_LOCAL,
            session_type=0,
            cos=0,
            max_pkt_len=0x600,
            ttl=0,
            nhop_handle=0)
        self.client.switch_api_mirror_session_update(0, mirror1, minfo1)
        send_packet(self, swports[1], str(pkt))
        verify_packet(self, exp_pkt, swports[2])
        verify_packet(self, exp_pkt, swports[3])
        verify_no_other_packets(self)

        print "Delete Mirror Session"
        self.client.switch_api_mirror_session_delete(0, mirror1)
        # clean-up test, make sure pkt is not mirrored after session is deleted
        send_packet(self, swports[1], str(pkt))
        verify_packet(self, exp_pkt, swports[2])
        verify_no_other_packets(self)
        # ip_acl cleanup
        self.client.switch_api_acl_rule_delete(0, acl, ace)
        self.client.switch_api_acl_list_delete(0, acl)
        #cleanup
        self.client.switch_api_l3_route_delete(0, vrf, i_ip3, nhop)
        self.client.switch_api_neighbor_delete(0, neighbor)
        self.client.switch_api_nhop_delete(0, nhop)

        self.client.switch_api_l3_interface_address_delete(0, rif1, vrf, i_ip1)
        self.client.switch_api_l3_interface_address_delete(0, rif2, vrf, i_ip2)

        self.client.switch_api_interface_delete(0, if1)
        self.client.switch_api_interface_delete(0, if2)

        self.client.switch_api_rif_delete(0, rif1)
        self.client.switch_api_rif_delete(0, rif2)

        self.client.switch_api_router_mac_delete(0, rmac, '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(0, rmac)
        self.client.switch_api_vrf_delete(0, vrf)


###############################################################################
#@group('l3')
#@group('acl')
#@group('mirror')
#class MirrorAclTest_i2e_erspan(api_base_tests.ThriftInterfaceDataPlane):
#    def runTest(self):
#        return
#
#        if (test_param_get('target') == 'bmv2'):
#            return
#
#        print "Test i2e Erspan Mirror packet port %d" % swports[
#            1], "  -> port %d" % swports[
#                2], "  (192.168.0.1 -> 10.0.0.1 [id = 101])"
#        vrf = self.client.switch_api_vrf_create(device, 2)
#
#        rmac = self.client.switch_api_router_mac_group_create(
#            device, SWITCH_RMAC_TYPE_ALL)
#        self.client.switch_api_router_mac_add(0, rmac, '00:77:66:55:44:33')
#
#        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
#        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
#        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[4])
#
#        rif_info = switcht_rif_info_t(
#            rif_type=SWITCH_RIF_TYPE_INTF,
#            vrf_handle=vrf,
#            rmac_handle=rmac,
#            v4_unicast_enabled=True)
#        rif1 = self.client.switch_api_rif_create(0, rif_info)
#        rif2 = self.client.switch_api_rif_create(0, rif_info)
#        rif4 = self.client.switch_api_rif_create(0, rif_info)
#
#        i_info1 = switcht_interface_info_t(
#            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
#        if1 = self.client.switch_api_interface_create(0, i_info1)
#        i_ip1 = switcht_ip_addr_t(ipaddr='192.168.0.2', prefix_length=16)
#        self.client.switch_api_l3_interface_address_add(0, rif1, vrf, i_ip1)
#
#        i_info2 = switcht_interface_info_t(
#            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
#        if2 = self.client.switch_api_interface_create(0, i_info2)
#        i_ip2 = switcht_ip_addr_t(ipaddr='10.0.0.2', prefix_length=16)
#        self.client.switch_api_l3_interface_address_add(0, rif2, vrf, i_ip2)
#
#        # Add a static route
#        i_ip3 = switcht_ip_addr_t(ipaddr='10.10.10.1', prefix_length=32)
#        nhop_key = switcht_nhop_key_t(intf_handle=rif2, ip_addr_valid=0)
#        nhop = self.client.switch_api_nhop_create(0, nhop_key)
#        neighbor_entry = switcht_neighbor_info_t(
#            nhop_handle=nhop,
#            interface_handle=rif2,
#            mac_addr='00:11:22:33:44:55',
#            ip_addr=i_ip3,
#            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L3)
#        neighbor = self.client.switch_api_neighbor_entry_add(0, neighbor_entry)
#        self.client.switch_api_l3_route_add(0, vrf, i_ip3, nhop)
#
#        # send the test packet(s)
#        pkt = simple_tcp_packet(
#            eth_dst='00:77:66:55:44:33',
#            eth_src='00:22:22:22:22:22',
#            ip_dst='10.10.10.1',
#            ip_src='192.168.0.1',
#            ip_id=105,
#            ip_ttl=64)
#
#        exp_pkt = simple_tcp_packet(
#            eth_dst='00:11:22:33:44:55',
#            eth_src='00:77:66:55:44:33',
#            ip_dst='10.10.10.1',
#            ip_src='192.168.0.1',
#            ip_id=105,
#            ip_ttl=63)
#
#        i_info4 = switcht_interface_info_t(
#            handle=port4, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif4)
#        if4 = self.client.switch_api_interface_create(0, i_info4)
#        i_ip4 = switcht_ip_addr_t(ipaddr='10.1.0.4', prefix_length=16)
#        self.client.switch_api_l3_interface_address_add(0, rif4, vrf, i_ip4)
#
#         # Add a static route to tunnel destination
#        i_ip5 = switcht_ip_addr_t(ipaddr='4.4.4.0', prefix_length=24)
#        tun_nhop_key = switcht_nhop_key_t(intf_handle=rif4, ip_addr_valid=0)
#        tun_nhop = self.client.switch_api_nhop_create(0, tun_nhop_key)
#        neighbor_entry1 = switcht_neighbor_info_t(nhop_handle=tun_nhop,
#                                                 interface_handle=rif4,
#                                                 mac_addr='00:44:44:44:44:44',
#                                                 ip_addr=i_ip5,
#                                                 rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L3)
#        neighbor1 = self.client.switch_api_neighbor_entry_add(0, neighbor_entry1)
#
#        self.client.switch_api_l3_route_add(0, vrf, i_ip5, tun_nhop)
#
#        # Create a logical network (LN)
#        lognet_info = switcht_logical_network_t()
#        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)
#
#        tunnel_mapper = switcht_tunnel_mapper_t(
#            tunnel_vni=0x1234, ln_handle=ln1)
#        mapper_handle = self.client.switch_api_tunnel_mapper_create(
#            device,
#            direction=SWITCH_API_DIRECTION_BOTH,
#            tunnel_type=SWITCH_TUNNEL_TYPE_ERSPAN_T3,
#            tunnel_mapper_list=[tunnel_mapper])
#
#        # Create a tunnel interface
#        src_ip = switcht_ip_addr_t(
#            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='4.4.4.1', prefix_length=32)
#        dst_ip = switcht_ip_addr_t(
#            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='4.4.4.3', prefix_length=32)
#        tunnel_info = switcht_tunnel_info_t(
#            vrf_handle=vrf,
#            tunnel_type=SWITCH_TUNNEL_TYPE_ERSPAN_T3,
#	    erspan_span_id=85,
#            src_ip=src_ip,
#            dst_ip=dst_ip,
#            decap_mapper_handle=mapper_handle,
#            encap_mapper_handle=mapper_handle,
#            egress_rif_handle=rif4)
#        ift = self.client.switch_api_tunnel_interface_create(
#            device, SWITCH_API_DIRECTION_BOTH, tunnel_info)
#        self.client.switch_api_logical_network_member_add(0, ln1, ift)
#
#	# create erspan tunnel nexthop
#        nhop_key1 = switcht_nhop_key_t(intf_handle=ift, ln_handle=ln1,
#                                            ip_addr_valid=0)
#        nhop1 = self.client.switch_api_nhop_create(0, nhop_key1)
#        neighbor_entry3 = switcht_neighbor_info_t(
#            nhop_handle=nhop1,
#            interface_handle=ift,
#            mac_addr='00:44:44:44:44:44',
#            ip_addr=src_ip,
#            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L2,
#            neigh_type=SWITCH_API_NEIGHBOR_IPV4_TUNNEL)
#        neighbor3 = self.client.switch_api_neighbor_entry_add(0,
#                                                               neighbor_entry3)
#        # create a mirror session
#        minfo1 = switcht_mirror_info_t(session_id=85, direction=SWITCH_API_DIRECTION_INGRESS,
#				      egress_port=port4,
#                                      mirror_type=SWITCH_MIRROR_TYPE_ENHANCED_REMOTE,
#                                      session_type=SWITCH_MIRROR_SESSION_TYPE_SIMPLE,
#                                      cos=0, max_pkt_len=0,
#                                      tun_info=tunnel_info,
#                                      tunnel_info_valid=True,
#                                      ttl=0, enable=1,
#				      nhop_handle=nhop1,
#                                      span_mode=SWITCH_MIRROR_SPAN_MODE_TUNNEL_NHOP)
#
#        mirror1 = self.client.switch_api_mirror_session_create(0, minfo1)
#
#        print "Create Mirror ACL to mirror i2e from 1->4"
#        acl = self.client.switch_api_acl_list_create(
#            0, SWITCH_API_DIRECTION_INGRESS, 0, SWITCH_HANDLE_TYPE_PORT)
#        # create kvp to match destination IP
#        kvp = []
#        kvp_val = switcht_acl_value_t(value_num=int("0a0a0a01", 16))
#        kvp_mask = switcht_acl_value_t(value_num=int("ffffffff", 16))
#        kvp.append(
#            switcht_acl_key_value_pair_t(SWITCH_ACL_IP_FIELD_IPV4_DEST, kvp_val,
#                                         kvp_mask))
#        action = SWITCH_ACL_ACTION_SET_MIRROR
#        action_params = switcht_acl_action_params_t()
#        opt_action_params = switcht_acl_opt_action_params_t(
#            mirror_handle=mirror1)
#        ace = self.client.switch_api_acl_ip_rule_create(
#            0, acl, 10, 1, kvp, action, action_params, opt_action_params)
#        port = self.client.switch_api_port_id_to_handle_get(0, swports[1])
#        self.client.switch_api_acl_reference(0, acl, port)
#
#        # egress interface if4
#        send_packet(self, swports[1], str(pkt))
#        # verify mirrored packet
#        exp_mirrored_pkt = ipv4_erspan_pkt(
#            eth_dst='00:44:44:44:44:44',
#            eth_src='00:77:66:55:44:33',
#            ip_src='4.4.4.1',
#            ip_dst='4.4.4.3',
#            ip_id=0,
#            ip_ttl=64,
#            ip_flags=0x2,
#            version=2,
#            mirror_id=85,
#            inner_frame=pkt)
#        # verify mirrored and original pkts
#        time.sleep(1)
#
#        verify_erspan3_packet(self, exp_mirrored_pkt, swports[4])
#        verify_packet(self, exp_pkt, swports[2])
#        verify_no_other_packets(self)
#
#        # delete the mirror sesion
#        print "Delete Egress Mirror Session and test packet again"
#        self.client.switch_api_mirror_session_delete(0, mirror1)
#        # clean-up test, make sure pkt is not mirrored after session is deleted
#        send_packet(self, swports[1], str(pkt))
#        verify_packet(self, exp_pkt, swports[2])
#        verify_no_other_packets(self)
#        # ip_acl cleanup
#        self.client.switch_api_acl_dereference(0, acl, port)
#        self.client.switch_api_acl_rule_delete(0, acl, ace)
#        self.client.switch_api_acl_list_delete(0, acl)
#        #cleanup
#
#        self.client.switch_api_neighbor_delete(0, neighbor2)
#        self.client.switch_api_neighbor_delete(0, neighbor3)
#        self.client.switch_api_nhop_delete(0, nhop1)
#
#        self.client.switch_api_logical_network_member_remove(0, ln1, ift)
#        self.client.switch_api_tunnel_interface_delete(0, ift)
#        self.client.switch_api_logical_network_delete(0, ln1)
#
#        self.client.switch_api_l3_route_delete(0, vrf, i_ip5, tun_nhop)
#        self.client.switch_api_neighbor_delete(0, neighbor1)
#        self.client.switch_api_nhop_delete(0, tun_nhop)
#        self.client.switch_api_l3_interface_address_delete(0, rif4, vrf, i_ip4)
#
#        self.client.switch_api_l3_route_delete(0, vrf, i_ip3, nhop)
#
#        self.client.switch_api_neighbor_delete(0, neighbor)
#        self.client.switch_api_nhop_delete(0, nhop)
#
#        self.client.switch_api_l3_interface_address_delete(0, rif1, vrf, i_ip1)
#        self.client.switch_api_l3_interface_address_delete(0, rif2, vrf, i_ip2)
#
#
#        self.client.switch_api_interface_delete(0, if1)
#        self.client.switch_api_interface_delete(0, if2)
#        self.client.switch_api_interface_delete(0, if4)
#
#        self.client.switch_api_rif_delete(0, rif1)
#        self.client.switch_api_rif_delete(0, rif2)
#        self.client.switch_api_rif_delete(0, rif4)
#
#        self.client.switch_api_router_mac_delete(0, rmac, '00:77:66:55:44:33')
#        self.client.switch_api_router_mac_group_delete(0, rmac)
#        self.client.switch_api_vrf_delete(0, vrf)
#
################################################################################
#@group('l3')
#@group('acl')
#@group('mirror')
#@group('mcast')
#class MirrorAclTest_i2e_erspan_with_mgid(ApiAdapter):
#    def setUp(self):
#        super(self.__class__, self).setUp()
#    def runTest(self):
#        if (test_param_get('target') == 'bmv2'):
#            return
#
#        print "Test i2e Erspan Mirror packet port %d" % swports[
#            1], "  -> port %d" % swports[
#                2], "  (192.168.0.1 -> 10.0.0.1 [id = 101])"
#        vrf = self.client.switch_api_vrf_create(device, 2)
#
#        rmac = self.client.switch_api_router_mac_group_create(
#            device, SWITCH_RMAC_TYPE_ALL)
#        self.client.switch_api_router_mac_add(0, rmac, '00:77:66:55:44:33')
#
#        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
#        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
#        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[4])
#
#        rif_info = switcht_rif_info_t(
#            rif_type=SWITCH_RIF_TYPE_INTF,
#            vrf_handle=vrf,
#            rmac_handle=rmac,
#            v4_unicast_enabled=True)
#        rif1 = self.client.switch_api_rif_create(0, rif_info)
#        rif2 = self.client.switch_api_rif_create(0, rif_info)
#        rif4 = self.client.switch_api_rif_create(0, rif_info)
#
#        i_info1 = switcht_interface_info_t(
#            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
#        if1 = self.client.switch_api_interface_create(0, i_info1)
#        i_ip1 = switcht_ip_addr_t(ipaddr='192.168.0.2', prefix_length=16)
#        self.client.switch_api_l3_interface_address_add(0, rif1, vrf, i_ip1)
#
#        i_info2 = switcht_interface_info_t(
#            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
#        if2 = self.client.switch_api_interface_create(0, i_info2)
#
#        i_ip2 = switcht_ip_addr_t(ipaddr='10.0.0.2', prefix_length=16)
#        self.client.switch_api_l3_interface_address_add(0, rif2, vrf, i_ip2)
#
#        # Add a static route
#        i_ip3 = switcht_ip_addr_t(ipaddr='10.10.10.1', prefix_length=32)
#        nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip3, '00:11:22:33:44:55')
#        self.client.switch_api_l3_route_add(0, vrf, i_ip3, nhop1)
#
#        i_info4 = switcht_interface_info_t(
#            handle=port4, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif4)
#        if4 = self.client.switch_api_interface_create(0, i_info4)
#        i_ip4 = switcht_ip_addr_t(ipaddr='10.1.0.4', prefix_length=16)
#        self.client.switch_api_l3_interface_address_add(0, rif4, vrf, i_ip4)
#
#         # Add a static route to tunnel destination
#        i_ip5 = switcht_ip_addr_t(ipaddr='4.4.4.0', prefix_length=24)
#        nhop2, neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, rif4, i_ip5, '00:44:44:44:44:44')
#        self.client.switch_api_l3_route_add(0, vrf, i_ip5, nhop2)
#
#        # Create a logical network (LN)
#        lognet_info = switcht_logical_network_t()
#        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)
#
#        rif_info5 = switcht_rif_info_t(
#            rif_type=SWITCH_RIF_TYPE_LN,
#            vrf_handle=vrf,
#            rmac_handle=rmac,
#            ln_handle=ln1,
#            v4_unicast_enabled=True)
#        rif5 = self.client.switch_api_rif_create(0, rif_info5)
#
#        underlay_lb_h = self.create_loopback_rif(device, vrf, rmac)
#        overlay_lb_h = self.create_loopback_rif(device, vrf, rmac)
#
#        tunnel = self.create_tunnel(
#                              device=device,
#                              underlay_vrf=vrf,
#                              tunnel_type=SWITCH_TUNNEL_TYPE_ERSPAN_T3,
#                              src_ip="4.4.4.1",
#                              dst_ip="4.4.4.3",
#                              entry_type=SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P,
#                              urif=underlay_lb_h,
#                              orif=overlay_lb_h,
#                              tunnel_ip_type=SWITCH_TUNNEL_IP_ADDR_TYPE_IPV4)
#        tunnel_nhop = self.add_nhop_tunnel(
#                              device,
#                              SWITCH_NHOP_TUNNEL_TYPE_LN,
#                              ln1,
#                              tunnel,
#                              "4.4.4.1",
#                              rw_type=SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L2_MIRROR)
#
#        # create a mirror session
#        minfo1 = switcht_mirror_info_t(session_id=85, direction=SWITCH_API_DIRECTION_INGRESS,
#                                      mirror_type=SWITCH_MIRROR_TYPE_ENHANCED_REMOTE,
#                                      session_type=SWITCH_MIRROR_SESSION_TYPE_SIMPLE,
#                                      cos=0, max_pkt_len=0,
#                                      nhop_handle=tunnel_nhop,
#                                      span_mode=SWITCH_MIRROR_SPAN_MODE_TUNNEL_NHOP,
#                                      ttl=0)
#
#        mirror1 = self.client.switch_api_mirror_session_create(0, minfo1)
#
#        print "Create Mirror ACL to mirror i2e from 1->4"
#        acl = self.client.switch_api_acl_list_create(
#            0, SWITCH_API_DIRECTION_INGRESS, 0, SWITCH_HANDLE_TYPE_PORT)
#        # create kvp to match destination IP
#        kvp = []
#        kvp_val = switcht_acl_value_t(value_num=int("0a0a0a01", 16))
#        kvp_mask = switcht_acl_value_t(value_num=int("ffffffff", 16))
#        kvp.append(
#            switcht_acl_key_value_pair_t(SWITCH_ACL_IP_FIELD_IPV4_DEST, kvp_val,
#                                         kvp_mask))
#        action = SWITCH_ACL_ACTION_SET_MIRROR
#        action_params = switcht_acl_action_params_t()
#        opt_action_params = switcht_acl_opt_action_params_t(
#            mirror_handle=mirror1)
#        ace = self.client.switch_api_acl_ip_rule_create(
#            0, acl, 10, 1, kvp, action, action_params, opt_action_params)
#        port = self.client.switch_api_port_id_to_handle_get(0, swports[1])
#        self.client.switch_api_acl_reference(0, acl, port)
#
#        # send the test packet(s)
#        pkt = simple_tcp_packet(
#            eth_dst='00:77:66:55:44:33',
#            eth_src='00:22:22:22:22:22',
#            ip_dst='10.10.10.1',
#            ip_src='192.168.0.1',
#            ip_id=105,
#            ip_ttl=64)
#
#        exp_pkt = simple_tcp_packet(
#            eth_dst='00:11:22:33:44:55',
#            eth_src='00:77:66:55:44:33',
#            ip_dst='10.10.10.1',
#            ip_src='192.168.0.1',
#            ip_id=105,
#            ip_ttl=63)
#
#        exp_mirrored_pkt = ipv4_erspan_pkt(
#            eth_dst='00:44:44:44:44:44',
#            eth_src='00:77:66:55:44:33',
#            ip_dst='4.4.4.1',
#            ip_src='4.4.4.3',
#            ip_id=0,
#            ip_ttl=64,
#            ip_flags=0x2,
#            version=2,
#            mirror_id=85,
#            inner_frame=pkt)
#
#        # Case 1: verify mirrored and original pkts
#        send_packet(self, swports[1], str(pkt))
#        time.sleep(1)
#        verify_erspan3_packet(self, exp_mirrored_pkt, swports[4])
#        verify_packet(self, exp_pkt, swports[2])
#        verify_no_other_packets(self)
#
#        # Case 2: Remove route to tunnel destination
#        self.client.switch_api_l3_route_delete(device, vrf, i_ip5, nhop2)
#        print "Sending packet after route remove"
#        send_packet(self, swports[1], str(pkt))
#        time.sleep(1)
#        verify_no_packet(self, exp_mirrored_pkt, swports[4])
#        verify_packet(self, exp_pkt, swports[2])
#        verify_no_other_packets(self)
#
#        # Case 3: Remove mirror and add them back
#        self.client.switch_api_mirror_session_delete(0, mirror1)
#        mirror1 = self.client.switch_api_mirror_session_create(0, minfo1)
#
#        # Case 4: Add route back and verify
#        self.client.switch_api_l3_route_add(0, vrf, i_ip5, nhop2)
#
#        print "Sending packet after route add"
#        send_packet(self, swports[1], str(pkt))
#        time.sleep(1)
#        verify_erspan3_packet(self, exp_mirrored_pkt, swports[4])
#        verify_packet(self, exp_pkt, swports[2])
#        verify_no_other_packets(self)
#
#        # delete the mirror sesion
#        print "Delete Egress Mirror Session and test packet again"
#        self.client.switch_api_mirror_session_delete(0, mirror1)
#
#        # clean-up test, make sure pkt is not mirrored after session is deleted
#        send_packet(self, swports[1], str(pkt))
#        verify_packet(self, exp_pkt, swports[2])
#        verify_no_other_packets(self)
#
#        # ip_acl cleanup
#        self.client.switch_api_acl_dereference(0, acl, port)
#        self.client.switch_api_acl_rule_delete(0, acl, ace)
#        self.client.switch_api_acl_list_delete(0, acl)
#        #cleanup
#        self.client.switch_api_rif_delete(0, rif5)
#
#        self.client.switch_api_logical_network_delete(0, ln1)
#
#        self.cleanup()
#
#        self.client.switch_api_l3_route_delete(0, vrf, i_ip5, nhop2)
#        self.client.switch_api_neighbor_delete(0, neighbor2)
#        self.client.switch_api_nhop_delete(0, nhop2)
#        self.client.switch_api_l3_interface_address_delete(0, rif4, vrf, i_ip4)
#
#        self.client.switch_api_l3_route_delete(0, vrf, i_ip3, nhop1)
#
#        self.client.switch_api_neighbor_delete(0, neighbor1)
#        self.client.switch_api_nhop_delete(0, nhop1)
#
#        self.client.switch_api_l3_interface_address_delete(0, rif1, vrf, i_ip1)
#        self.client.switch_api_l3_interface_address_delete(0, rif2, vrf, i_ip2)
#
#        self.client.switch_api_interface_delete(0, if1)
#        self.client.switch_api_interface_delete(0, if2)
#        self.client.switch_api_interface_delete(0, if4)
#
#        self.client.switch_api_rif_delete(0, rif1)
#        self.client.switch_api_rif_delete(0, rif2)
#        self.client.switch_api_rif_delete(0, rif4)
#
#        self.client.switch_api_router_mac_delete(0, rmac, '00:77:66:55:44:33')
#        self.client.switch_api_router_mac_group_delete(0, rmac)
#        self.client.switch_api_vrf_delete(0, vrf)
#
################################################################################
#@group('l3')
#@group('acl')
#@group('mirror')
#@group('mcast')
#class MirrorAclTest_i2e_erspan_with_ecmp_mgid(ApiAdapter):
#    def setUp(self):
#        super(self.__class__, self).setUp()
#    def runTest(self):
#        if (test_param_get('target') == 'bmv2'):
#            return
#
#        print "Test i2e Erspan Mirror packet port %d" % swports[
#            1], "  -> port %d" % swports[
#                2], "  (192.168.0.1 -> 10.0.0.1 [id = 101])"
#        self.client.switch_api_init(0)
#        vrf = self.client.switch_api_vrf_create(device, 2)
#
#        rmac = self.client.switch_api_router_mac_group_create(
#            device, SWITCH_RMAC_TYPE_ALL)
#        self.client.switch_api_router_mac_add(0, rmac, '00:77:66:55:44:33')
#
#        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
#        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
#        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])
#        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[4])
#
#        rif_info = switcht_rif_info_t(
#            rif_type=SWITCH_RIF_TYPE_INTF,
#            vrf_handle=vrf,
#            rmac_handle=rmac,
#            v4_unicast_enabled=True)
#        rif1 = self.client.switch_api_rif_create(0, rif_info)
#        rif2 = self.client.switch_api_rif_create(0, rif_info)
#        rif3 = self.client.switch_api_rif_create(0, rif_info)
#        rif4 = self.client.switch_api_rif_create(0, rif_info)
#
#        i_info1 = switcht_interface_info_t(
#            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
#        if1 = self.client.switch_api_interface_create(0, i_info1)
#        i_ip1 = switcht_ip_addr_t(ipaddr='192.168.0.2', prefix_length=16)
#        self.client.switch_api_l3_interface_address_add(0, rif1, vrf, i_ip1)
#
#        i_info2 = switcht_interface_info_t(
#            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
#        if2 = self.client.switch_api_interface_create(0, i_info2)
#        i_ip2 = switcht_ip_addr_t(ipaddr='10.0.0.2', prefix_length=16)
#        self.client.switch_api_l3_interface_address_add(0, rif2, vrf, i_ip2)
#
#        # Add a static route
#        i_ip3 = switcht_ip_addr_t(ipaddr='10.10.10.1', prefix_length=32)
#        nhop, neighbor = switch_api_l3_nhop_neighbor_create(
#            self, device, rif2, i_ip3, '00:11:22:33:44:55')
#        self.client.switch_api_l3_route_add(0, vrf, i_ip3, nhop)
#
#        # Add a static ecmp route to tunnel destination
#        i_ip6 = switcht_ip_addr_t(ipaddr='4.4.4.0', prefix_length=24)
#
#        i_info3 = switcht_interface_info_t(
#            handle=port3, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif3)
#        if3 = self.client.switch_api_interface_create(0, i_info3)
#        i_ip4 = switcht_ip_addr_t(ipaddr='10.1.0.3', prefix_length=16)
#        self.client.switch_api_l3_interface_address_add(0, rif3, vrf, i_ip4)
#
#        tun_nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(
#            self, device, rif3, i_ip6, '00:44:44:44:44:44')
#
#        i_info4 = switcht_interface_info_t(
#            handle=port4, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif4)
#        if4 = self.client.switch_api_interface_create(0, i_info4)
#        i_ip5 = switcht_ip_addr_t(ipaddr='10.2.0.4', prefix_length=16)
#        self.client.switch_api_l3_interface_address_add(0, rif4, vrf, i_ip5)
#
#        tun_nhop2, neighbor2 = switch_api_l3_nhop_neighbor_create(
#            self, device, rif4, i_ip6, '00:44:44:44:44:44')
#
#        ecmp = self.client.switch_api_ecmp_create(device)
#        self.client.switch_api_ecmp_member_add(device, ecmp, 2,
#                                               [tun_nhop1, tun_nhop2])
#
#        self.client.switch_api_l3_route_add(0, vrf, i_ip6, ecmp)
#
#        # Create a logical network (LN)
#        lognet_info = switcht_logical_network_t()
#        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)
#
#        rif_info5 = switcht_rif_info_t(
#            rif_type=SWITCH_RIF_TYPE_LN,
#            vrf_handle=vrf,
#            rmac_handle=rmac,
#            ln_handle=ln1,
#            v4_unicast_enabled=True)
#        rif5 = self.client.switch_api_rif_create(0, rif_info5)
#
#        underlay_lb_h = self.create_loopback_rif(device, vrf, rmac)
#        overlay_lb_h = self.create_loopback_rif(device, vrf, rmac)
#
#        # Create a tunnel interface
#        tunnel = self.create_tunnel(
#                              device=device,
#                              underlay_vrf=vrf,
#                              tunnel_type=SWITCH_TUNNEL_TYPE_ERSPAN_T3,
#                              src_ip="4.4.4.1",
#                              dst_ip="4.4.4.3",
#                              entry_type=SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P,
#                              urif=underlay_lb_h,
#                              orif=overlay_lb_h,
#                              tunnel_ip_type=SWITCH_TUNNEL_IP_ADDR_TYPE_IPV4)
#        tunnel_nhop = self.add_nhop_tunnel(
#                              device,
#                              SWITCH_NHOP_TUNNEL_TYPE_LN,
#                              ln1,
#                              tunnel,
#                              "4.4.4.1",
#                              rw_type=SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L2_MIRROR)
#
#        # create a mirror session
#        minfo1 = switcht_mirror_info_t(session_id=85, direction=SWITCH_API_DIRECTION_INGRESS,
#                                      mirror_type=SWITCH_MIRROR_TYPE_ENHANCED_REMOTE,
#                                      session_type=SWITCH_MIRROR_SESSION_TYPE_SIMPLE,
#                                      cos=0, max_pkt_len=0,
#                                      nhop_handle=tunnel_nhop,
#                                      span_mode=SWITCH_MIRROR_SPAN_MODE_TUNNEL_NHOP,
#                                      ttl=0)
#
#        mirror1 = self.client.switch_api_mirror_session_create(0, minfo1)
#
#        print "Create Mirror ACL to mirror i2e from 1->4"
#        acl = self.client.switch_api_acl_list_create(
#            0, SWITCH_API_DIRECTION_INGRESS, 0, SWITCH_HANDLE_TYPE_PORT)
#        # create kvp to match destination IP
#        kvp = []
#        kvp_val = switcht_acl_value_t(value_num=int("0a0a0a01", 16))
#        kvp_mask = switcht_acl_value_t(value_num=int("ffffffff", 16))
#        kvp.append(
#            switcht_acl_key_value_pair_t(SWITCH_ACL_IP_FIELD_IPV4_DEST, kvp_val,
#                                         kvp_mask))
#        action = SWITCH_ACL_ACTION_SET_MIRROR
#        action_params = switcht_acl_action_params_t()
#        opt_action_params = switcht_acl_opt_action_params_t(
#            mirror_handle=mirror1)
#        ace = self.client.switch_api_acl_ip_rule_create(
#            0, acl, 10, 1, kvp, action, action_params, opt_action_params)
#        port = self.client.switch_api_port_id_to_handle_get(0, swports[1])
#        self.client.switch_api_acl_reference(0, acl, port)
#
#        # send the test packet(s)
#        pkt = simple_tcp_packet(
#            eth_dst='00:77:66:55:44:33',
#            eth_src='00:22:22:22:22:22',
#            ip_dst='10.10.10.1',
#            ip_src='192.168.0.1',
#            ip_id=105,
#            ip_ttl=64)
#
#        exp_pkt = simple_tcp_packet(
#            eth_dst='00:11:22:33:44:55',
#            eth_src='00:77:66:55:44:33',
#            ip_dst='10.10.10.1',
#            ip_src='192.168.0.1',
#            ip_id=105,
#            ip_ttl=63)
#
#        exp_mirrored_pkt = ipv4_erspan_pkt(
#            eth_dst='00:44:44:44:44:44',
#            eth_src='00:77:66:55:44:33',
#            ip_dst='4.4.4.1',
#            ip_src='4.4.4.3',
#            ip_id=0,
#            ip_ttl=64,
#            ip_flags=0x2,
#            version=2,
#            mirror_id=85,
#            inner_frame=pkt)
#
#        # egress interface if4
#        send_packet(self, swports[1], str(pkt))
#
#        # Case 1: verify mirrored and original pkts
#        time.sleep(2)
#
#        verify_erspan3_any_packet_any_port(self, [exp_mirrored_pkt],
#                                        [swports[3], swports[4]])
#        verify_packet(self, exp_pkt, swports[2])
#        verify_no_other_packets(self)
#
#        # Case 2: Remove route to tunnel destination
#        self.client.switch_api_l3_route_delete(0, vrf, i_ip6, ecmp)
#        print "Sending packet after route remove"
#        send_packet(self, swports[1], str(pkt))
#        time.sleep(1)
#        verify_packet(self, exp_pkt, swports[2])
#        verify_no_other_packets(self)
#
#        # Case 3: Remove mirror and add them back
#        self.client.switch_api_mirror_session_delete(0, mirror1)
#        mirror1 = self.client.switch_api_mirror_session_create(0, minfo1)
#
#        # Case 4: Add route back and verify
#        self.client.switch_api_l3_route_add(0, vrf, i_ip6, ecmp)
#
#        print "Sending packet after route add"
#        send_packet(self, swports[1], str(pkt))
#        time.sleep(1)
#        verify_erspan3_any_packet_any_port(self, [exp_mirrored_pkt],
#                                        [swports[3], swports[4]])
#        verify_packet(self, exp_pkt, swports[2])
#        verify_no_other_packets(self)
#
#        # Case 5: Remove ECMP members and see that mirroring works
#        self.client.switch_api_ecmp_member_delete(device, ecmp, 1,
#                                [tun_nhop2])
#        send_packet(self, swports[1], str(pkt))
#        time.sleep(1)
#        verify_erspan3_packet(self, exp_mirrored_pkt, swports[3])
#        verify_packet(self, exp_pkt, swports[2])
#        verify_no_other_packets(self)
#
#        self.client.switch_api_ecmp_member_delete(device, ecmp, 1,
#                                [tun_nhop1])
#        send_packet(self, swports[1], str(pkt))
#        time.sleep(1)
#        verify_packet(self, exp_pkt, swports[2])
#        verify_no_other_packets(self)
#
#        # Case 6: Add ECMP members and see that mirroring works
#        self.client.switch_api_ecmp_member_add(device, ecmp, 1,
#                                        [tun_nhop2])
#        send_packet(self, swports[1], str(pkt))
#        time.sleep(1)
#        verify_erspan3_packet(self, exp_mirrored_pkt, swports[4])
#        verify_packet(self, exp_pkt, swports[2])
#        verify_no_other_packets(self)
#
#        self.client.switch_api_ecmp_member_add(device, ecmp, 1,
#                                        [tun_nhop1])
#        send_packet(self, swports[1], str(pkt))
#        time.sleep(1)
#        verify_erspan3_any_packet_any_port(self, [exp_mirrored_pkt],
#                                        [swports[3], swports[4]])
#        verify_packet(self, exp_pkt, swports[2])
#        verify_no_other_packets(self)
#
#        # delete the mirror sesion
#        print "Delete Egress Mirror Session and test packet again"
#        self.client.switch_api_mirror_session_delete(0, mirror1)
#        # clean-up test, make sure pkt is not mirrored after session is deleted
#        send_packet(self, swports[1], str(pkt))
#        verify_packet(self, exp_pkt, swports[2])
#        verify_no_other_packets(self)
#
#        # ip_acl cleanup
#        self.client.switch_api_acl_dereference(0, acl, port)
#        self.client.switch_api_acl_rule_delete(0, acl, ace)
#        self.client.switch_api_acl_list_delete(0, acl)
#        #cleanup
#        self.client.switch_api_rif_delete(0, rif5)
#        self.client.switch_api_logical_network_delete(0, ln1)
#        self.cleanup()
#
#        self.client.switch_api_l3_route_delete(0, vrf, i_ip6, ecmp)
#        self.client.switch_api_ecmp_member_delete(device, ecmp, 2,
#                                                 [tun_nhop1, tun_nhop2])
#        self.client.switch_api_ecmp_delete(device, ecmp)
#
#        self.client.switch_api_neighbor_delete(0, neighbor1)
#        self.client.switch_api_nhop_delete(0, tun_nhop1)
#        self.client.switch_api_l3_interface_address_delete(0, rif3, vrf, i_ip4)
#
#        self.client.switch_api_neighbor_delete(0, neighbor2)
#        self.client.switch_api_nhop_delete(0, tun_nhop2)
#        self.client.switch_api_l3_interface_address_delete(0, rif4, vrf, i_ip5)
#
#        self.client.switch_api_l3_route_delete(0, vrf, i_ip3, nhop)
#        self.client.switch_api_neighbor_delete(0, neighbor)
#        self.client.switch_api_nhop_delete(0, nhop)
#
#        self.client.switch_api_l3_interface_address_delete(0, rif1, vrf, i_ip1)
#        self.client.switch_api_l3_interface_address_delete(0, rif2, vrf, i_ip2)
#
#        self.client.switch_api_interface_delete(0, if1)
#        self.client.switch_api_interface_delete(0, if2)
#        self.client.switch_api_interface_delete(0, if3)
#        self.client.switch_api_interface_delete(0, if4)
#
#        self.client.switch_api_rif_delete(0, rif1)
#        self.client.switch_api_rif_delete(0, rif2)
#        self.client.switch_api_rif_delete(0, rif3)
#        self.client.switch_api_rif_delete(0, rif4)
#
#        self.client.switch_api_router_mac_delete(0, rmac, '00:77:66:55:44:33')
#        self.client.switch_api_router_mac_group_delete(0, rmac)
#        self.client.switch_api_vrf_delete(0, vrf)
#
################################################################################
#
#@group('l3')
#@group('acl')
#@group('mirror')
#@group('mcast')
#class MirrorAclTest_i2e_erspan_lag_mgid(ApiAdapter):
#    def setUp(self):
#        super(self.__class__, self).setUp()
#    def runTest(self):
#        if (test_param_get('target') == 'bmv2'):
#            return
#
#        print "Test i2e Erspan Mirror packet port %d" % swports[
#            1], "  -> port %d" % swports[
#                2], "  (192.168.0.1 -> 10.0.0.1 [id = 101])"
#        self.client.switch_api_init(0)
#        vrf = self.client.switch_api_vrf_create(device, 2)
#
#        rmac = self.client.switch_api_router_mac_group_create(
#            device, SWITCH_RMAC_TYPE_ALL)
#        self.client.switch_api_router_mac_add(0, rmac, '00:77:66:55:44:33')
#
#        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
#        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
#        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])
#        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[4])
#
#        rif_info = switcht_rif_info_t(
#            rif_type=SWITCH_RIF_TYPE_INTF,
#            vrf_handle=vrf,
#            rmac_handle=rmac,
#            v4_unicast_enabled=True)
#        rif1 = self.client.switch_api_rif_create(0, rif_info)
#        rif2 = self.client.switch_api_rif_create(0, rif_info)
#        rif3 = self.client.switch_api_rif_create(0, rif_info)
#
#        i_info1 = switcht_interface_info_t(
#            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
#        if1 = self.client.switch_api_interface_create(0, i_info1)
#        i_ip1 = switcht_ip_addr_t(ipaddr='192.168.0.2', prefix_length=16)
#        self.client.switch_api_l3_interface_address_add(0, rif1, vrf, i_ip1)
#
#        i_info2 = switcht_interface_info_t(
#            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
#        if2 = self.client.switch_api_interface_create(0, i_info2)
#        i_ip2 = switcht_ip_addr_t(ipaddr='10.0.0.2', prefix_length=16)
#        self.client.switch_api_l3_interface_address_add(0, rif2, vrf, i_ip2)
#
#        lag1 = self.client.switch_api_lag_create(device)
#        self.client.switch_api_lag_member_add(
#            device, lag_handle=lag1, side=SWITCH_API_DIRECTION_BOTH, port=port3)
#        self.client.switch_api_lag_member_add(
#            device, lag_handle=lag1, side=SWITCH_API_DIRECTION_BOTH, port=port4)
#        i_info3 = switcht_interface_info_t(
#            handle=lag1, type=SWITCH_INTERFACE_TYPE_ACCESS, rif_handle=rif3)
#        if3 = self.client.switch_api_interface_create(device, i_info3)
#
#        i_ip4 = switcht_ip_addr_t(ipaddr='10.1.0.3', prefix_length=16)
#        self.client.switch_api_l3_interface_address_add(0, rif3, vrf, i_ip4)
#
#        # Add a static route
#        i_ip3 = switcht_ip_addr_t(ipaddr='10.10.10.1', prefix_length=32)
#        nhop, neighbor = switch_api_l3_nhop_neighbor_create(
#            self, device, rif2, i_ip3, '00:11:22:33:44:55')
#        self.client.switch_api_l3_route_add(0, vrf, i_ip3, nhop)
#
#        # Add a static ecmp route to tunnel destination
#        i_ip6 = switcht_ip_addr_t(ipaddr='4.4.4.0', prefix_length=24)
#
#        tun_nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(
#            self, device, rif1, i_ip6, '00:44:44:44:44:44')
#
#        tun_nhop2, neighbor2 = switch_api_l3_nhop_neighbor_create(
#            self, device, rif3, i_ip6, '00:44:44:44:44:44')
#
#        ecmp = self.client.switch_api_ecmp_create(device)
#        self.client.switch_api_ecmp_member_add(device, ecmp, 2,
#                                               [tun_nhop1, tun_nhop2])
#
#        self.client.switch_api_l3_route_add(0, vrf, i_ip6, ecmp)
#
#        # Create a logical network (LN)
#        lognet_info = switcht_logical_network_t()
#        ln1 = self.client.switch_api_logical_network_create(device, lognet_info)
#
#        rif_info5 = switcht_rif_info_t(
#            rif_type=SWITCH_RIF_TYPE_LN,
#            vrf_handle=vrf,
#            rmac_handle=rmac,
#            ln_handle=ln1,
#            v4_unicast_enabled=True)
#        rif5 = self.client.switch_api_rif_create(0, rif_info5)
#
#        # Create a tunnel interface
#        underlay_lb_h = self.create_loopback_rif(device, vrf, rmac)
#        overlay_lb_h = self.create_loopback_rif(device, vrf, rmac)
#
#        tunnel = self.create_tunnel(
#                              device=device,
#                              underlay_vrf=vrf,
#                              tunnel_type=SWITCH_TUNNEL_TYPE_ERSPAN_T3,
#                              src_ip="4.4.4.1",
#                              dst_ip="4.4.4.3",
#                              entry_type=SWITCH_TUNNEL_TERM_ENTRY_TYPE_P2P,
#                              urif=underlay_lb_h,
#                              orif=overlay_lb_h,
#                              tunnel_ip_type=SWITCH_TUNNEL_IP_ADDR_TYPE_IPV4)
#        tunnel_nhop = self.add_nhop_tunnel(
#                              device,
#                              SWITCH_NHOP_TUNNEL_TYPE_LN,
#                              ln1,
#                              tunnel,
#                              "4.4.4.1",
#                              rw_type=SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L2_MIRROR)
#
#        # create a mirror session
#        minfo1 = switcht_mirror_info_t(session_id=85, direction=SWITCH_API_DIRECTION_INGRESS,
#                                      mirror_type=SWITCH_MIRROR_TYPE_ENHANCED_REMOTE,
#                                      session_type=SWITCH_MIRROR_SESSION_TYPE_SIMPLE,
#                                      cos=0, max_pkt_len=0,
#                                      nhop_handle=tunnel_nhop,
#                                      span_mode=SWITCH_MIRROR_SPAN_MODE_TUNNEL_NHOP,
#                                      ttl=0)
#
#        mirror1 = self.client.switch_api_mirror_session_create(0, minfo1)
#
#        print "Create Mirror ACL to mirror i2e from 1->4"
#        acl = self.client.switch_api_acl_list_create(
#            0, SWITCH_API_DIRECTION_INGRESS, 0, SWITCH_HANDLE_TYPE_PORT)
#        # create kvp to match destination IP
#        kvp = []
#        kvp_val = switcht_acl_value_t(value_num=int("0a0a0a01", 16))
#        kvp_mask = switcht_acl_value_t(value_num=int("ffffffff", 16))
#        kvp.append(
#            switcht_acl_key_value_pair_t(SWITCH_ACL_IP_FIELD_IPV4_DEST, kvp_val,
#                                         kvp_mask))
#        action = SWITCH_ACL_ACTION_SET_MIRROR
#        action_params = switcht_acl_action_params_t()
#        opt_action_params = switcht_acl_opt_action_params_t(
#            mirror_handle=mirror1)
#        ace = self.client.switch_api_acl_ip_rule_create(
#            0, acl, 10, 1, kvp, action, action_params, opt_action_params)
#        port = self.client.switch_api_port_id_to_handle_get(0, swports[1])
#        self.client.switch_api_acl_reference(0, acl, port)
#
#        # send the test packet(s)
#        pkt = simple_tcp_packet(
#            eth_dst='00:77:66:55:44:33',
#            eth_src='00:22:22:22:22:22',
#            ip_dst='10.10.10.1',
#            ip_src='192.168.0.1',
#            ip_id=105,
#            ip_ttl=64)
#
#        exp_pkt = simple_tcp_packet(
#            eth_dst='00:11:22:33:44:55',
#            eth_src='00:77:66:55:44:33',
#            ip_dst='10.10.10.1',
#            ip_src='192.168.0.1',
#            ip_id=105,
#            ip_ttl=63)
#
#        exp_mirrored_pkt = ipv4_erspan_pkt(
#            eth_dst='00:44:44:44:44:44',
#            eth_src='00:77:66:55:44:33',
#            ip_dst='4.4.4.1',
#            ip_src='4.4.4.3',
#            ip_id=0,
#            ip_ttl=64,
#            ip_flags=0x2,
#            version=2,
#            mirror_id=85,
#            inner_frame=pkt)
#
#        # egress interface if4
#        send_packet(self, swports[1], str(pkt))
#
#        # Case 1: verify mirrored and original pkts
#        time.sleep(1)
#
#        verify_erspan3_any_packet_any_port(self, [exp_mirrored_pkt],
#                                [swports[1], swports[3], swports[4]])
#        verify_packet(self, exp_pkt, swports[2])
#        verify_no_other_packets(self)
#
#        # Case 2: Remove ECMP members and see that mirroring works
#        # Also, check if reflection works
#        self.client.switch_api_ecmp_member_delete(device, ecmp, 1,
#                                [tun_nhop2])
#        send_packet(self, swports[1], str(pkt))
#        time.sleep(1)
#        verify_erspan3_packet(self, exp_mirrored_pkt, swports[1])
#        verify_packet(self, exp_pkt, swports[2])
#        verify_no_other_packets(self)
#
#        self.client.switch_api_ecmp_member_delete(device, ecmp, 1,
#                                [tun_nhop1])
#        send_packet(self, swports[1], str(pkt))
#        time.sleep(1)
#        verify_packet(self, exp_pkt, swports[2])
#        verify_no_other_packets(self)
#
#        # Case 3: Add ECMP members and see that mirroring works
#        self.client.switch_api_ecmp_member_add(device, ecmp, 1,
#                                        [tun_nhop2])
#        send_packet(self, swports[1], str(pkt))
#        time.sleep(1)
#        verify_erspan3_any_packet_any_port(self, [exp_mirrored_pkt],
#                                [swports[3], swports[4]])
#        verify_packet(self, exp_pkt, swports[2])
#        verify_no_other_packets(self)
#
#        # Case 4: Churn the lag
#        self.client.switch_api_lag_member_delete(
#            device,
#            lag_handle=lag1,
#            side=SWITCH_API_DIRECTION_BOTH,
#            port=port3)
#        send_packet(self, swports[1], str(pkt))
#        time.sleep(1)
#        verify_erspan3_packet(self, exp_mirrored_pkt, swports[4])
#        verify_packet(self, exp_pkt, swports[2])
#        verify_no_other_packets(self)
#
#        self.client.switch_api_lag_member_delete(
#            device,
#            lag_handle=lag1,
#            side=SWITCH_API_DIRECTION_BOTH,
#            port=port4)
#        send_packet(self, swports[1], str(pkt))
#        time.sleep(1)
#        verify_packet(self, exp_pkt, swports[2])
#        verify_no_other_packets(self, timeout=4)
#
#        self.client.switch_api_lag_member_add(
#            device, lag_handle=lag1, side=SWITCH_API_DIRECTION_BOTH,
#            port=port3)
#        send_packet(self, swports[1], str(pkt))
#        time.sleep(1)
#        verify_erspan3_packet(self, exp_mirrored_pkt, swports[3])
#        verify_packet(self, exp_pkt, swports[2])
#        verify_no_other_packets(self)
#
#        self.client.switch_api_lag_member_add(
#            device, lag_handle=lag1, side=SWITCH_API_DIRECTION_BOTH,
#            port=port4)
#        send_packet(self, swports[1], str(pkt))
#        time.sleep(1)
#        verify_erspan3_any_packet_any_port(self, [exp_mirrored_pkt],
#                                [swports[3], swports[4]])
#        verify_packet(self, exp_pkt, swports[2])
#        verify_no_other_packets(self)
#
#        self.client.switch_api_ecmp_member_add(device, ecmp, 1,
#                                        [tun_nhop1])
#        send_packet(self, swports[1], str(pkt))
#        time.sleep(1)
#        verify_erspan3_any_packet_any_port(self, [exp_mirrored_pkt],
#                                [swports[1], swports[3], swports[4]])
#        verify_packet(self, exp_pkt, swports[2])
#        verify_no_other_packets(self)
#
#        # delete the mirror sesion
#        print "Delete Egress Mirror Session and test packet again"
#        self.client.switch_api_mirror_session_delete(0, mirror1)
#        # clean-up test, make sure pkt is not mirrored after session is deleted
#        send_packet(self, swports[1], str(pkt))
#        verify_packet(self, exp_pkt, swports[2])
#        verify_no_other_packets(self)
#
#        # ip_acl cleanup
#        self.client.switch_api_acl_dereference(0, acl, port)
#        self.client.switch_api_acl_rule_delete(0, acl, ace)
#        self.client.switch_api_acl_list_delete(0, acl)
#        #cleanup
#        self.client.switch_api_rif_delete(0, rif5)
#        self.client.switch_api_logical_network_delete(0, ln1)
#        self.cleanup()
#
#        self.client.switch_api_l3_route_delete(0, vrf, i_ip6, ecmp)
#        self.client.switch_api_ecmp_member_delete(device, ecmp, 2,
#                                                 [tun_nhop1, tun_nhop2])
#        self.client.switch_api_ecmp_delete(device, ecmp)
#
#        self.client.switch_api_neighbor_delete(0, neighbor1)
#        self.client.switch_api_nhop_delete(0, tun_nhop1)
#
#        self.client.switch_api_neighbor_delete(0, neighbor2)
#        self.client.switch_api_nhop_delete(0, tun_nhop2)
#
#        self.client.switch_api_l3_route_delete(0, vrf, i_ip3, nhop)
#        self.client.switch_api_neighbor_delete(0, neighbor)
#        self.client.switch_api_nhop_delete(0, nhop)
#
#        self.client.switch_api_l3_interface_address_delete(0, rif1, vrf, i_ip1)
#        self.client.switch_api_l3_interface_address_delete(0, rif2, vrf, i_ip2)
#        self.client.switch_api_l3_interface_address_delete(0, rif3, vrf, i_ip4)
#
#        self.client.switch_api_interface_delete(0, if1)
#        self.client.switch_api_interface_delete(0, if2)
#        self.client.switch_api_interface_delete(0, if3)
#
#        self.client.switch_api_rif_delete(0, rif1)
#        self.client.switch_api_rif_delete(0, rif2)
#        self.client.switch_api_rif_delete(0, rif3)
#
#        self.client.switch_api_lag_member_delete(
#            device,
#            lag_handle=lag1,
#            side=SWITCH_API_DIRECTION_BOTH,
#            port=port3)
#        self.client.switch_api_lag_member_delete(
#            device,
#            lag_handle=lag1,
#            side=SWITCH_API_DIRECTION_BOTH,
#            port=port4)
#
#        self.client.switch_api_lag_delete(device, lag1)
#
#        self.client.switch_api_router_mac_delete(0, rmac, '00:77:66:55:44:33')
#        self.client.switch_api_router_mac_group_delete(0, rmac)
#        self.client.switch_api_vrf_delete(0, vrf)
#
###############################################################################

@group('l3')
@group('acl')
@group('stats')
@group('ent')
class IPAclStatsTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Sending packet port %d" % swports[1], "  -> port %d" % swports[
            2], "  (192.168.0.1 -> 10.0.0.1 [id = 101])"
        vrf = self.client.switch_api_vrf_create(device, 2)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(0, rmac, '00:77:66:55:44:33')

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info1)

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)

        if1 = self.client.switch_api_interface_create(0, i_info1)
        i_ip1 = switcht_ip_addr_t(ipaddr='192.168.0.2', prefix_length=16)
        self.client.switch_api_l3_interface_address_add(0, rif1, vrf, i_ip1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(0, i_info2)
        i_ip2 = switcht_ip_addr_t(ipaddr='10.0.0.2', prefix_length=16)
        self.client.switch_api_l3_interface_address_add(0, rif2, vrf, i_ip2)

        # Add a static route
        i_ip3 = switcht_ip_addr_t(ipaddr='10.10.0.0', prefix_length=16)
        nhop_ip3 = switcht_ip_addr_t(ipaddr='10.10.10.10', prefix_length=32)
        nhop, neighbor = switch_api_l3_nhop_neighbor_create(self, device, rif2, nhop_ip3, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(0, vrf, i_ip3, nhop)

        counter = self.client.switch_api_acl_counter_create(0)

        acl = self.client.switch_api_acl_list_create(
            0, SWITCH_API_DIRECTION_INGRESS, 0, SWITCH_HANDLE_TYPE_PORT)
        kvp = []
        kvp_val1 = switcht_acl_value_t(value_num=int("0a0a0a01", 16))
        kvp_mask1 = switcht_acl_value_t(value_num=int("ffffffff", 16))
        kvp.append(
            switcht_acl_key_value_pair_t(SWITCH_ACL_IP_FIELD_IPV4_DEST,
                                         kvp_val1, kvp_mask1))
        action = 2
        action_params = switcht_acl_action_params_t(
            redirect=switcht_acl_action_redirect(handle=0))
        opt_action_params = switcht_acl_opt_action_params_t(
            counter_handle=counter)
        ace1 = self.client.switch_api_acl_ip_rule_create(
            0, acl, 10, 1, kvp, action, action_params, opt_action_params)
        kvp = []
        kvp_val2 = switcht_acl_value_t(value_num=int("0a0a0a02", 16))
        kvp_mask2 = switcht_acl_value_t(value_num=int("ffffffff", 16))
        kvp.append(
            switcht_acl_key_value_pair_t(SWITCH_ACL_IP_FIELD_IPV4_DEST,
                                         kvp_val2, kvp_mask2))
        action = 1
        action_params = switcht_acl_action_params_t(
            redirect=switcht_acl_action_redirect(handle=0))
        opt_action_params = switcht_acl_opt_action_params_t(
            counter_handle=counter)
        ace2 = self.client.switch_api_acl_ip_rule_create(
            0, acl, 10, 1, kvp, action, action_params, opt_action_params)
        port = self.client.switch_api_port_id_to_handle_get(0, swports[1])
        self.client.switch_api_acl_reference(0, acl, port)

        ba = 0
        if test_param_get('target') == "hw" or test_param_get('target') == "asic-model":
            ba = 4

        # send the test packet(s)
        try:
            num_bytes = 0
            num_packets = 0
            random.seed(314159)
            stats0 = self.client.switch_api_acl_stats_get(0, counter)
            for i in range(0, 10):
                pktlen = random.randint(100, 250)
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst='10.10.10.1',
                    ip_src='192.168.0.1',
                    ip_id=105,
                    ip_ttl=64,
                    pktlen=pktlen)

                exp_pkt = simple_tcp_packet(
                    eth_dst='00:11:22:33:44:55',
                    eth_src='00:77:66:55:44:33',
                    ip_dst='10.10.10.1',
                    ip_src='192.168.0.1',
                    ip_id=105,
                    ip_ttl=63,
                    pktlen=pktlen)
                send_packet(self, swports[1], str(pkt))
                verify_packets(self, exp_pkt, [swports[2]])
                num_bytes += pktlen
                num_packets += 1

            for i in range(0, 10):
                pktlen = random.randint(100, 250)
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst='10.10.10.2',
                    ip_src='192.168.0.1',
                    ip_id=105,
                    ip_ttl=64,
                    pktlen=pktlen)

                send_packet(self, swports[1], str(pkt))
                num_bytes += pktlen
                num_packets += 1

            verify_no_other_packets(self, timeout=30)

            stats = self.client.switch_api_acl_stats_get(0, counter)
            stats.num_packets = stats.num_packets - stats0.num_packets
            stats.num_bytes = stats.num_bytes - stats0.num_bytes
            self.assertEqual(stats.num_packets, num_packets)
            self.assertEqual(stats.num_bytes, num_bytes + ba * num_packets)

        finally:
            # ip_acl
            self.client.switch_api_acl_dereference(0, acl, port)
            self.client.switch_api_acl_counter_delete(0, counter)
            self.client.switch_api_acl_rule_delete(0, acl, ace1)
            self.client.switch_api_acl_rule_delete(0, acl, ace2)
            self.client.switch_api_acl_list_delete(0, acl)

            #cleanup
            self.client.switch_api_l3_route_delete(0, vrf, i_ip3, nhop)
            self.client.switch_api_neighbor_delete(0, neighbor)
            self.client.switch_api_nhop_delete(0, nhop)

            self.client.switch_api_l3_interface_address_delete(0, rif1, vrf,
                                                               i_ip1)
            self.client.switch_api_l3_interface_address_delete(0, rif2, vrf,
                                                               i_ip2)

            self.client.switch_api_interface_delete(0, if1)
            self.client.switch_api_interface_delete(0, if2)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_router_mac_delete(0, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(0, rmac)
            self.client.switch_api_vrf_delete(0, vrf)


###############################################################################
@group('l3')
@group('acl')
@group('ipv6')
@group('racl')
class IPv6RAclTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Sending packet port %d" % swports[1], "  -> port %d" % swports[
            2], "  (2000::1 -> 3000::2)"
        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(0, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[4])

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v6_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info)
        rif2 = self.client.switch_api_rif_create(0, rif_info)

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(0, i_info1)
        i_ip1 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='2000::2',
            prefix_length=120)
        self.client.switch_api_l3_interface_address_add(0, rif1, vrf, i_ip1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(0, i_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='3000::2',
            prefix_length=120)
        self.client.switch_api_l3_interface_address_add(0, rif2, vrf, i_ip2)

        # Add a static route
        i_ip3 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6,
            ipaddr='1234:5678:9abc:def0:4422:1133:5577:99aa',
            prefix_length=128)
        nhop, neighbor = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip3, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(0, vrf, i_ip3, nhop)

        # create ace
        counter = self.client.switch_api_racl_counter_create(0)

        racl = self.client.switch_api_acl_list_create(
            0, SWITCH_API_DIRECTION_INGRESS, SWITCH_ACL_TYPE_IPV6_RACL, SWITCH_HANDLE_TYPE_PORT)

        kvp = []
        kvp_val = switcht_acl_value_t(value_str="123456789abcdef044221133557799aa")
        kvp_mask = switcht_acl_value_t(value_str="ffffffffffffffffffffffffffffffff")
        kvp.append(
            switcht_acl_key_value_pair_t(SWITCH_ACL_IPV6_FIELD_IPV6_DEST, kvp_val,
                                         kvp_mask))
        action = SWITCH_ACL_ACTION_PERMIT
        action_params = switcht_acl_action_params_t(
            redirect=switcht_acl_action_redirect(handle=0))
        opt_action_params = switcht_acl_opt_action_params_t(
            counter_handle=counter)
        ace1 = self.client.switch_api_acl_ipv6racl_rule_create(
            0, racl, 10, 1, kvp, action, action_params, opt_action_params)

        kvp = []
        kvp_val = switcht_acl_value_t(value_str="123456789abcdef044221133557799ab")
        kvp_mask = switcht_acl_value_t(value_str="ffffffffffffffffffffffffffffffff")
        kvp.append(
            switcht_acl_key_value_pair_t(SWITCH_ACL_IPV6_FIELD_IPV6_DEST, kvp_val,
                                         kvp_mask))
        action = SWITCH_ACL_ACTION_DROP
        action_params = switcht_acl_action_params_t(
            redirect=switcht_acl_action_redirect(handle=0))
        opt_action_params = switcht_acl_opt_action_params_t(
            counter_handle=counter)
        ace2 = self.client.switch_api_acl_ipv6racl_rule_create(
            0, racl, 10, 1, kvp, action, action_params, opt_action_params)
        port = self.client.switch_api_port_id_to_handle_get(0, swports[1])
        self.client.switch_api_acl_reference(0, racl, port)

        ba = 0
        if test_param_get('target') == "hw" or test_param_get('target') == "asic-model":
            ba = 4

        # send the test packet(s)
        try:
            num_bytes = 0
            num_packets = 0
            random.seed(314159)
            stats0 = self.client.switch_api_racl_stats_get(0, counter)
            for i in range(0, 10):
                pktlen = random.randint(100, 250)
                pkt = simple_tcpv6_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ipv6_dst='1234:5678:9abc:def0:4422:1133:5577:99aa',
                    ipv6_src='2000::1',
                    ipv6_hlim=64,
                    pktlen=pktlen)

                exp_pkt = simple_tcpv6_packet(
                    eth_dst='00:11:22:33:44:55',
                    eth_src='00:77:66:55:44:33',
                    ipv6_dst='1234:5678:9abc:def0:4422:1133:5577:99aa',
                    ipv6_src='2000::1',
                    ipv6_hlim=63,
                    pktlen=pktlen)

                send_packet(self, swports[1], str(pkt))
                verify_packets(self, exp_pkt, [swports[2]])
                num_bytes += pktlen
                num_packets += 1

            for i in range(0, 10):
                pktlen = random.randint(100, 250)
                pkt = simple_tcpv6_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ipv6_dst='1234:5678:9abc:def0:4422:1133:5577:99ab',
                    ipv6_src='2000::1',
                    ipv6_hlim=64,
                    pktlen=pktlen)

                send_packet(self, swports[1], str(pkt))
                num_bytes += pktlen
                num_packets += 1

            verify_no_other_packets(self, timeout=30)

            stats = self.client.switch_api_racl_stats_get(0, counter)
            stats.num_packets = stats.num_packets - stats0.num_packets
            stats.num_bytes = stats.num_bytes - stats0.num_bytes
            self.assertEqual(stats.num_packets, num_packets)
            self.assertEqual(stats.num_bytes, num_bytes + ba * num_packets)

        finally:
            # ip_acl
            self.client.switch_api_acl_dereference(0, racl, port)
            self.client.switch_api_racl_counter_delete(0, counter)
            self.client.switch_api_acl_rule_delete(0, racl, ace1)
            self.client.switch_api_acl_rule_delete(0, racl, ace2)
            self.client.switch_api_acl_list_delete(0, racl)

            #cleanup
            self.client.switch_api_l3_route_delete(0, vrf, i_ip3, nhop)
            self.client.switch_api_neighbor_delete(0, neighbor)
            self.client.switch_api_nhop_delete(0, nhop)

            self.client.switch_api_l3_interface_address_delete(0, rif1, vrf,
                                                               i_ip1)
            self.client.switch_api_l3_interface_address_delete(0, rif2, vrf,
                                                               i_ip2)

            self.client.switch_api_interface_delete(0, if1)
            self.client.switch_api_interface_delete(0, if2)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_router_mac_delete(0, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(0, rmac)
            self.client.switch_api_vrf_delete(0, vrf)


###############################################################################
@group('l3')
@group('acl')
@group('ent')
class IPIngressAclRangeTcamTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Sending packet port %d" % swports[1], "  -> port %d" % swports[
            2], "  (192.168.0.1 -> 10.0.0.1 [id = 101])"
        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(0, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
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
            v4_unicast_enabled=True)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)

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

        switch_range = switcht_range_t(start_value=1000, end_value=2000)
        acl_range_handle = self.client.switch_api_acl_range_create(
            0, SWITCH_API_DIRECTION_INGRESS, SWITCH_RANGE_TYPE_SRC_PORT,
            switch_range)

        # setup a deny ACL to verify that the same packet does not make it
        # ip acl
        acl = self.client.switch_api_acl_list_create(
            0, SWITCH_API_DIRECTION_INGRESS, SWITCH_ACL_TYPE_IP,
            SWITCH_HANDLE_TYPE_PORT)
        # create kvp to match destination IP
        kvp = []
        kvp_val = switcht_acl_value_t(value_num=acl_range_handle)
        kvp_mask = switcht_acl_value_t(value_num=0xffffffff)
        kvp.append(
            switcht_acl_key_value_pair_t(
                SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT_RANGE, kvp_val, kvp_mask))
        action = SWITCH_ACL_ACTION_DROP
        action_params = switcht_acl_action_params_t(
            redirect=switcht_acl_action_redirect(handle=0))
        opt_action_params = switcht_acl_opt_action_params_t()
        ace = self.client.switch_api_acl_ip_rule_create(
            0, acl, 10, 1, kvp, action, action_params, opt_action_params)
        port = self.client.switch_api_port_id_to_handle_get(0, swports[1])
        self.client.switch_api_acl_reference(0, acl, port)
        send_packet(self, swports[1], str(pkt))

        # check for absence of packet here!
        try:
            verify_packets(self, exp_pkt, [swports[2]])
            print 'FAILED - did not expect packet'
        except:
            print 'Success'

        # ip_acl
        self.client.switch_api_acl_dereference(0, acl, port1)
        self.client.switch_api_acl_rule_delete(0, acl, ace)
        self.client.switch_api_acl_list_delete(0, acl)
        self.client.switch_api_acl_range_delete(0, acl_range_handle)

        #cleanup
        self.client.switch_api_l3_route_delete(0, vrf, i_ip3, nhop)
        self.client.switch_api_neighbor_delete(0, neighbor)
        self.client.switch_api_nhop_delete(0, nhop)

        self.client.switch_api_l3_interface_address_delete(0, rif1, vrf, i_ip1)
        self.client.switch_api_l3_interface_address_delete(0, rif2, vrf, i_ip2)

        self.client.switch_api_interface_delete(0, if1)
        self.client.switch_api_interface_delete(0, if2)

        self.client.switch_api_rif_delete(0, rif1)
        self.client.switch_api_rif_delete(0, rif2)

        self.client.switch_api_router_mac_delete(0, rmac, '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(0, rmac)
        self.client.switch_api_vrf_delete(0, vrf)


###############################################################################

@group('l3')
@group('acl')
@group('stats')
@group('ent')
class AclLabelTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        vrf = self.client.switch_api_vrf_create(device, 2)
        vlan = self.client.switch_api_vlan_create(device, 100)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])
        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[4])

        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if3 = self.client.switch_api_interface_create(device, i_info3)

        i_info4 = switcht_interface_info_t(
            handle=port4, type=SWITCH_INTERFACE_TYPE_TRUNK)
        if4 = self.client.switch_api_interface_create(device, i_info4)

        self.client.switch_api_vlan_member_add(device, vlan, if3)
        self.client.switch_api_vlan_member_add(device, vlan, if4)

        switch_api_mac_table_entry_create(
            self, device, vlan, '00:33:33:33:33:33', 2, if3)
        switch_api_mac_table_entry_create(
            self, device, vlan, '00:44:44:44:44:44', 2, if4)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(0, rmac, '00:77:66:55:44:33')

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
        i_ip1 = switcht_ip_addr_t(ipaddr='11.0.0.2', prefix_length=16)
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
        if2 = self.client.switch_api_interface_create(0, i_info2)
        i_ip2 = switcht_ip_addr_t(ipaddr='10.0.0.2', prefix_length=16)
        self.client.switch_api_l3_interface_address_add(0, rif2, vrf, i_ip2)

        # Add a static route
        i_ip3 = switcht_ip_addr_t(ipaddr='10.10.0.0', prefix_length=16)
        nhop_ip3 = switcht_ip_addr_t(ipaddr='10.10.10.10', prefix_length=32)
        nhop2, neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, rif2, nhop_ip3, '00:11:11:11:11:55')
        self.client.switch_api_l3_route_add(0, vrf, i_ip3, nhop2)

        # Add a static route
        i_ip4 = switcht_ip_addr_t(ipaddr='11.11.0.0', prefix_length=16)
        nhop_ip4 = switcht_ip_addr_t(ipaddr='11.11.11.11', prefix_length=32)
        nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, rif1, nhop_ip4, '00:22:22:22:22:55')
        self.client.switch_api_l3_route_add(0, vrf, i_ip4, nhop1)

        counter = self.client.switch_api_acl_counter_create(0)

        #ip_acl
        ip_acl = self.client.switch_api_acl_list_create(
            0, SWITCH_API_DIRECTION_INGRESS, 0, SWITCH_HANDLE_TYPE_NONE)
        kvp = []
        kvp_count = 0
        kvp_val1 = switcht_acl_value_t(value_num=int("0a0a0a01", 16))
        kvp_mask1 = switcht_acl_value_t(value_num=int("ffffffff", 16))
        kvp.append(
            switcht_acl_key_value_pair_t(SWITCH_ACL_IP_FIELD_IPV4_DEST,
                                         kvp_val1, kvp_mask1))
        kvp_count += 1
        kvp_val1 = switcht_acl_value_t(value_num=111)
        kvp_mask1 = switcht_acl_value_t(value_num=int("ffff", 16))
        kvp.append(
            switcht_acl_key_value_pair_t(SWITCH_ACL_IP_FIELD_PORT_LAG_LABEL,
                                         kvp_val1, kvp_mask1))
        kvp_count += 1
        action = 1
        action_params = switcht_acl_action_params_t(
            redirect=switcht_acl_action_redirect(handle=0))
        opt_action_params = switcht_acl_opt_action_params_t(
            counter_handle=counter)
        ip_ace = self.client.switch_api_acl_ip_rule_create(0, ip_acl, 10,
                kvp_count, kvp, action, action_params, opt_action_params)
        self.client.switch_api_port_ingress_acl_label_set(0, port1, 111)

        #mac_acl
        mac_acl = self.client.switch_api_acl_list_create(
            0, SWITCH_API_DIRECTION_INGRESS, 1, SWITCH_HANDLE_TYPE_NONE)
        kvp = []
        kvp_count = 0
        mac_addr = "004444444444"
        kvp_val1 = switcht_acl_value_t(value_str=mac_addr)
        kvp_mask1 = switcht_acl_value_t(value_str=("ffffffffffff"))
        kvp.append(
            switcht_acl_key_value_pair_t(SWITCH_ACL_MAC_FIELD_DEST_MAC,
                                         kvp_val1, kvp_mask1))
        kvp_count += 1
        kvp_val1 = switcht_acl_value_t(value_num=222)
        kvp_mask1 = switcht_acl_value_t(value_num=int("ffff", 16))
        kvp.append(
            switcht_acl_key_value_pair_t(SWITCH_ACL_MAC_FIELD_VLAN_RIF_LABEL,
                                         kvp_val1, kvp_mask1))
        kvp_count += 1
        action = 1
        action_params = switcht_acl_action_params_t(
            redirect=switcht_acl_action_redirect(handle=0))
        opt_action_params = switcht_acl_opt_action_params_t(
            counter_handle=counter)
        mac_ace = self.client.switch_api_acl_mac_rule_create(0, mac_acl, 10,
                kvp_count, kvp, action, action_params, opt_action_params)
        self.client.switch_api_vlan_ingress_acl_label_set(0, vlan, 222)

        try:
            print "Sending packet port %d" % swports[1], "  -> port %d" % swports[
                2], "  (11.11.11.1 -> 10.10.10.1) ip_acl hit"
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:11:11:11:11:11',
                ip_dst='10.10.10.1',
                ip_src='11.11.11.1',
                ip_id=105,
                ip_ttl=64,
                pktlen=150)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:55',
                eth_src='00:77:66:55:44:33',
                ip_dst='10.10.10.1',
                ip_src='11.11.11.1',
                ip_id=105,
                ip_ttl=63,
                pktlen=150)
            send_packet(self, swports[1], str(pkt))
            verify_no_other_packets(self, timeout=2)

            print "Sending packet port %d" % swports[2], "  -> port %d" % swports[
                1], "  (10.10.10.1 -> 11.11.11.1) ip_acl miss"
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='11.11.11.1',
                ip_src='10.10.10.1',
                ip_id=105,
                ip_ttl=64,
                pktlen=150)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:22:22:22:22:55',
                eth_src='00:77:66:55:44:33',
                ip_dst='11.11.11.1',
                ip_src='10.10.10.1',
                ip_id=105,
                ip_ttl=63,
                pktlen=150)
            send_packet(self, swports[2], str(pkt))
            verify_packets(self, exp_pkt, [swports[1]])

            print "Sending packet port %d" % swports[3], "  -> port %d" % swports[
                4], "  (00:33:33:33:33:33 -> 00:44:44:44:44:44) mac_acl hit"
            pkt = simple_eth_packet(
                eth_dst='00:44:44:44:44:44',
                eth_src='00:33:33:33:33:33')
            send_packet(self, swports[3], str(pkt))
            verify_no_other_packets(self, timeout=2)

        finally:
            # acl cleanup
            self.client.switch_api_acl_counter_delete(0, counter)
            self.client.switch_api_acl_rule_delete(0, mac_acl, mac_ace)
            self.client.switch_api_acl_list_delete(0, mac_acl)
            self.client.switch_api_acl_rule_delete(0, ip_acl, ip_ace)
            self.client.switch_api_acl_list_delete(0, ip_acl)

            #cleanup
            self.client.switch_api_l3_route_delete(0, vrf, i_ip3, nhop2)
            self.client.switch_api_neighbor_delete(0, neighbor2)
            self.client.switch_api_nhop_delete(0, nhop2)
            self.client.switch_api_l3_route_delete(0, vrf, i_ip4, nhop1)
            self.client.switch_api_neighbor_delete(0, neighbor1)
            self.client.switch_api_nhop_delete(0, nhop1)

            self.client.switch_api_l3_interface_address_delete(0, rif1, vrf,
                                                               i_ip1)
            self.client.switch_api_l3_interface_address_delete(0, rif2, vrf,
                                                               i_ip2)

            switch_api_mac_table_entry_delete(self, 0, vlan,
                                                          '00:33:33:33:33:33')
            switch_api_mac_table_entry_delete(self, 0, vlan,
                                                          '00:44:44:44:44:44')
            self.client.switch_api_vlan_member_remove(0, vlan, if3)
            self.client.switch_api_vlan_member_remove(0, vlan, if4)

            self.client.switch_api_interface_delete(0, if1)
            self.client.switch_api_interface_delete(0, if2)
            self.client.switch_api_interface_delete(0, if3)
            self.client.switch_api_interface_delete(0, if4)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_vlan_delete(0, vlan)
            self.client.switch_api_router_mac_delete(0, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(0, rmac)
            self.client.switch_api_vrf_delete(0, vrf)

###############################################################################

@group('acl')
@group('mirror')
class Acl_i2e_ErspanRewriteTest(ApiAdapter):
    def setUp(self):
        super(self.__class__, self).setUp()
    def runTest(self):
        if (test_param_get('target') == 'bmv2'):
            return

        print "Test i2e Erspan Mirror packet port %d" % swports[
            1], "  -> port %d" % swports[
                2], "  (192.168.0.1 -> 10.0.0.1 [id = 101])"
        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(0, rmac, '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port4 = self.client.switch_api_port_id_to_handle_get(device, swports[4])

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(0, rif_info)
        rif2 = self.client.switch_api_rif_create(0, rif_info)
        rif4 = self.client.switch_api_rif_create(0, rif_info)

        # Src Port
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(0, i_info1)
        i_ip1 = switcht_ip_addr_t(ipaddr='192.168.0.2', prefix_length=16)
        self.client.switch_api_l3_interface_address_add(0, rif1, vrf, i_ip1)

        # Dest Port
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(0, i_info2)
        i_ip2 = switcht_ip_addr_t(ipaddr='10.0.0.2', prefix_length=16)
        self.client.switch_api_l3_interface_address_add(0, rif2, vrf, i_ip2)

        # Add a static route
        i_ip3 = switcht_ip_addr_t(ipaddr='10.10.10.1', prefix_length=32)
        nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip3, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(0, vrf, i_ip3, nhop1)

        # Monitor Port
        i_info4 = switcht_interface_info_t(
            handle=port4, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif4)
        if4 = self.client.switch_api_interface_create(0, i_info4)
        i_ip4 = switcht_ip_addr_t(ipaddr='10.1.0.4', prefix_length=16)
        self.client.switch_api_l3_interface_address_add(0, rif4, vrf, i_ip4)

        # create a mirror session
        minfo1 = switcht_mirror_info_t(session_id=85, direction=SWITCH_API_DIRECTION_INGRESS,
                                       egress_port_handle=port4,
                                       mirror_type=SWITCH_MIRROR_TYPE_ENHANCED_REMOTE,
                                       cos=0, max_pkt_len=0, ttl=32,
                                       session_type=SWITCH_MIRROR_SESSION_TYPE_SIMPLE,
                                       span_mode=SWITCH_MIRROR_SPAN_MODE_TUNNEL_REWRITE,
                                       src_ip=switcht_ip_addr_t(
                                                        addr_type=SWITCH_API_IP_ADDR_V4,
                                                        ipaddr='4.4.4.3',
                                                        prefix_length=32),
                                       dst_ip=switcht_ip_addr_t(
                                                        addr_type=SWITCH_API_IP_ADDR_V4,
                                                        ipaddr='4.4.4.1',
                                                        prefix_length=32),
                                       src_mac='00:77:66:55:44:33',
                                       dst_mac='00:44:44:44:44:44')

        mirror1 = self.client.switch_api_mirror_session_create(0, minfo1)

        print "Create Mirror ACL to mirror i2e from 1->4"
        acl = self.client.switch_api_acl_list_create(
            0, SWITCH_API_DIRECTION_INGRESS, 0, SWITCH_HANDLE_TYPE_PORT)
        # create kvp to match destination IP
        kvp = []
        kvp_val = switcht_acl_value_t(value_num=int("0a0a0a01", 16))
        kvp_mask = switcht_acl_value_t(value_num=int("ffffffff", 16))
        kvp.append(
            switcht_acl_key_value_pair_t(SWITCH_ACL_IP_FIELD_IPV4_DEST, kvp_val,
                                         kvp_mask))
        action = SWITCH_ACL_ACTION_SET_MIRROR
        action_params = switcht_acl_action_params_t()
        opt_action_params = switcht_acl_opt_action_params_t(
            mirror_handle=mirror1)
        ace = self.client.switch_api_acl_ip_rule_create(
            0, acl, 10, 1, kvp, action, action_params, opt_action_params)
        port = self.client.switch_api_port_id_to_handle_get(0, swports[1])
        self.client.switch_api_acl_reference(0, acl, port)

        # send the test packet(s)
        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_ttl=64)

        exp_pkt = simple_tcp_packet(
            eth_dst='00:11:22:33:44:55',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.10.10.1',
            ip_src='192.168.0.1',
            ip_id=105,
            ip_ttl=63)

        exp_mirrored_pkt = ipv4_erspan_pkt(
            eth_dst='00:44:44:44:44:44',
            eth_src='00:77:66:55:44:33',
            ip_dst='4.4.4.1',
            ip_src='4.4.4.3',
            ip_id=0,
            ip_ttl=32,
            ip_flags=0x2,
            version=2,
            mirror_id=85,
            inner_frame=pkt)

        mirror_created = True
        try:
            # Case 1: verify mirrored and original pkts
            send_packet(self, swports[1], str(pkt))
            time.sleep(1)
            verify_erspan3_packet(self, exp_mirrored_pkt, swports[4])
            verify_packet(self, exp_pkt, swports[2])
            verify_no_other_packets(self)

            # delete the mirror sesion
            print "Delete Egress Mirror Session and test packet again"
            self.client.switch_api_mirror_session_delete(0, mirror1)
            mirror_created = False

            # clean-up test, make sure pkt is not mirrored after session is deleted
            send_packet(self, swports[1], str(pkt))
            verify_packet(self, exp_pkt, swports[2])
            verify_no_other_packets(self)

        finally:
            if mirror_created:
                self.client.switch_api_mirror_session_delete(0, mirror1)

            # ip_acl cleanup
            self.client.switch_api_acl_dereference(0, acl, port)
            self.client.switch_api_acl_rule_delete(0, acl, ace)
            self.client.switch_api_acl_list_delete(0, acl)
            self.client.switch_api_l3_interface_address_delete(0, rif4, vrf, i_ip4)
            self.client.switch_api_l3_route_delete(0, vrf, i_ip3, nhop1)

            self.client.switch_api_neighbor_delete(0, neighbor1)
            self.client.switch_api_nhop_delete(0, nhop1)

            self.client.switch_api_l3_interface_address_delete(0, rif1, vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(0, rif2, vrf, i_ip2)

            self.client.switch_api_interface_delete(0, if1)
            self.client.switch_api_interface_delete(0, if2)
            self.client.switch_api_interface_delete(0, if4)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)
            self.client.switch_api_rif_delete(0, rif4)

            self.client.switch_api_router_mac_delete(0, rmac, '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(0, rmac)
            self.client.switch_api_vrf_delete(0, vrf)
