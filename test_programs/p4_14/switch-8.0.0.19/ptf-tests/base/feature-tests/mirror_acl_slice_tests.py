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
##############################################################################
"""
Thrift API interface basic tests
"""

import os
import ptf.mask
import switchapi_thrift
import sys
import unittest


from ptf.testutils import *
from ptf.thriftutils import *
from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../base'))
sys.path.append(os.path.join(this_dir, '../../base/api-tests'))
sys.path.append(os.path.join(this_dir, '../../base/common'))

from common.utils import *
import api_base_tests
from api_utils import *
from common.api_utils import *
from api_adapter import ApiAdapter

device = 0
cpu_port = 64
swports = [x for x in range(65)]

###############################################################################
@group('mirror_acl')
class MirrorAclSliceTest_i2e(api_base_tests.ThriftInterfaceDataPlane):
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

        print "Create mirror session"
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
            nhop_handle=0)
        mirror1 = self.client.switch_api_mirror_session_create(0, minfo1)

        # setup a Mirror acl and program it in dedicated mirror acl slice
        #
        print "Create Mirror ACL to mirror i2e from 1->4"
        acl = self.client.switch_api_acl_list_create(
            0, SWITCH_API_DIRECTION_INGRESS, SWITCH_ACL_TYPE_IP_MIRROR_ACL, SWITCH_HANDLE_TYPE_PORT)
        # create kvp to match destination IP
        kvp = []
        kvp_val = switcht_acl_value_t(value_num=int("0a0a0a01", 16))
        kvp_mask = switcht_acl_value_t(value_num=int("ffffffff", 16))
        kvp.append(
            switcht_acl_key_value_pair_t(SWITCH_ACL_IP_MIRROR_ACL_FIELD_IPV4_DEST, kvp_val,
                                         kvp_mask))
        action = SWITCH_ACL_ACTION_SET_MIRROR
        action_params = switcht_acl_action_params_t()
        opt_action_params = switcht_acl_opt_action_params_t(
            mirror_handle=mirror1)
        ace = self.client.switch_api_acl_ip_mirror_rule_create(
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


@group('mirror_acl')
class IPv6MirrorAclSliceTest_i2e(api_base_tests.ThriftInterfaceDataPlane):
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

        # send the test packet(s)
        pkt = simple_tcpv6_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:22:22:22:22:22',
            ipv6_dst='1234:5678:9abc:def0:4422:1133:5577:99aa',
            ipv6_src='2000::1',
            ipv6_hlim=64)
        send_packet(self, swports[1], str(pkt))

        exp_pkt = simple_tcpv6_packet(
            eth_dst='00:11:22:33:44:55',
            eth_src='00:77:66:55:44:33',
            ipv6_dst='1234:5678:9abc:def0:4422:1133:5577:99aa',
            ipv6_src='2000::1',
            ipv6_hlim=63)
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
            nhop_handle=0)
        mirror1 = self.client.switch_api_mirror_session_create(0, minfo1)

        # setup a Mirror acl and program it in dedicated mirror acl slice
        #
        print "Create Mirror ACL to mirror i2e from 1->4"
        acl = self.client.switch_api_acl_list_create(
            0, SWITCH_API_DIRECTION_INGRESS, SWITCH_ACL_TYPE_IPV6_MIRROR_ACL, SWITCH_HANDLE_TYPE_PORT)
        # create kvp to match destination IP
        kvp = []
        kvp_val = switcht_acl_value_t(value_str="123456789abcdef044221133557799aa")
        kvp_mask = switcht_acl_value_t(value_str="ffffffffffffffffffffffffffffffff")
        kvp.append(
            switcht_acl_key_value_pair_t(SWITCH_ACL_IPV6_MIRROR_ACL_FIELD_IPV6_DEST, kvp_val,
                                         kvp_mask))
        action = SWITCH_ACL_ACTION_SET_MIRROR
        action_params = switcht_acl_action_params_t()
        opt_action_params = switcht_acl_opt_action_params_t(
            mirror_handle=mirror1)
        ace = self.client.switch_api_acl_ipv6_mirror_rule_create(
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


