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
import pd_base_tests

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *

from switch.p4_pd_rpc.ttypes import *

from res_pd_rpc.ttypes import *
from mc_pd_rpc.ttypes import *

import os
from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

from erspan3 import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../api-tests'))
sys.path.append(os.path.join(this_dir, '..'))
import api_base_tests
from common.api_utils import *

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)

if swports == []:
    swports = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15]

sess_hdl = 0


###############################################################################
@group('acl')
@group('meters')
class MeterTest(pd_base_tests.ThriftInterfaceDataPlane,
                api_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        if test_param_get('target') == "bmv2":
            pd_base_tests.ThriftInterfaceDataPlane.__init__(self, [""], ["dc"])
        else:
            pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                            ["dc"])

    def set_meter_time(self, time, enable):
        if test_param_get('target') == "bmv2":
            dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))
            self.conn_mgr.set_meter_time(sess_hdl, dev_tgt, enable)
        else:
            dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))
            self.conn_mgr.set_meter_time(sess_hdl, 0, hex_to_i16(0xFFFF), time)
            self.conn_mgr.complete_operations(sess_hdl)

    def runTest(self):
        print
        sess_hdl = self.conn_mgr.client_init()
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.set_meter_time(0, 1)
        vrf = self.client.switch_api_vrf_create(0, 1)

        rmac = self.client.switch_api_router_mac_group_create(0)
        self.client.switch_api_router_mac_add(0, rmac, '00:77:66:55:44:33')

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            v4_unicast_enabled=True,
            rmac_handle=rmac)
        rif1 = self.client.switch_api_rif_create(0, rif_info)
        rif2 = self.client.switch_api_rif_create(0, rif_info)

        port1 = self.client.switch_api_port_id_to_handle_get(0, swports[1])
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(0, i_info1)
        i_ip1 = switcht_ip_addr_t(ipaddr='192.168.0.2', prefix_length=16)
        self.client.switch_api_l3_interface_address_add(0, rif1, vrf, i_ip1)

        port2 = self.client.switch_api_port_id_to_handle_get(0, swports[2])
        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif2)
        if2 = self.client.switch_api_interface_create(0, i_info2)
        i_ip2 = switcht_ip_addr_t(ipaddr='10.0.0.2', prefix_length=16)
        self.client.switch_api_l3_interface_address_add(0, rif2, vrf, i_ip2)
        # Add a static route
        i_ip3 = switcht_ip_addr_t(ipaddr='10.10.10.1', prefix_length=32)
        nhop, neighbor = switch_api_l3_nhop_neighbor_create(self, device, rif2, i_ip3, '00:11:22:33:44:55')
        self.client.switch_api_l3_route_add(0, vrf, i_ip3, nhop)

        api_meter_info = switcht_meter_info_t(
            meter_mode=1,
            color_source=1,
            meter_type=2,
            cbs=8,
            cir=1000,
            pbs=16,
            pir=2000,
            green_action=2,
            yellow_action=2,
            red_action=1)
        meter = self.client.switch_api_meter_create(0, api_meter_info)

        # setup a deny ACL to verify that the same packet does not make it
        # ip acl
        acl = self.client.switch_api_acl_list_create(0, 1, 0,
                                                     SWITCH_HANDLE_TYPE_PORT)
        # create kvp to match destination IP
        kvp = []
        kvp_val = switcht_acl_value_t(value_num=int("0a0a0a01", 16))
        kvp_mask = switcht_acl_value_t(value_num=int("ffffffff", 16))
        kvp.append(switcht_acl_key_value_pair_t(1, kvp_val, kvp_mask))
        action = 2
        action_params = switcht_acl_action_params_t(
            redirect=switcht_acl_action_redirect(handle=0))
        opt_action_params = switcht_acl_opt_action_params_t(meter_handle=meter)
        ace = self.client.switch_api_acl_ip_rule_create(
            0, acl, 10, 1, kvp, action, action_params, opt_action_params)
        self.client.switch_api_acl_reference(0, acl, port1)

        pkt = simple_tcp_packet(
            eth_dst='00:77:66:55:44:33',
            eth_src='00:22:22:22:22:22',
            ip_dst='10.10.10.1',
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:11:22:33:44:55',
            eth_src='00:77:66:55:44:33',
            ip_dst='10.10.10.1',
            ip_id=101,
            ip_ttl=63)

        try:
            counter_ids = [0, 1, 2]
            counter1 = self.client.switch_api_meter_stats_get(0, meter,
                                                              counter_ids)

            if test_param_get('target') == "bmv2":
                num_of_green_pkts = 10
            else:
                num_of_green_pkts = 11

            self.set_meter_time(0, 1)

            for i in range(0, num_of_green_pkts):
                send_packet(self, swports[1], str(pkt))
                verify_packets(self, exp_pkt, [swports[2]])

            self.set_meter_time(20, 1)

            num_of_yellow_pkts = 10
            for i in range(0, num_of_yellow_pkts):
                send_packet(self, swports[1], str(pkt))
                verify_packets(self, exp_pkt, [swports[2]])

            self.set_meter_time(40, 1)

            num_of_red_pkts = 0
            if test_param_get('target') != "bmv2":
                send_packet(self, swports[1], str(pkt))
                verify_no_other_packets(self, timeout=1)
                num_of_red_pkts += 1

            self.set_meter_time(25000000, 1)

            num_of_red_pkts += 1
            send_packet(self, swports[1], str(pkt))
            verify_no_other_packets(self, timeout=1)

            self.set_meter_time(0, 0)

            num_of_green_pkts += 1
            send_packet(self, swports[1], str(pkt))
            verify_packets(self, exp_pkt, [swports[2]])

            counter2 = self.client.switch_api_meter_stats_get(0, meter,
                                                              counter_ids)

            for i in range(0, 3):
                counter2[i].num_bytes -= counter1[i].num_bytes
                counter2[i].num_packets -= counter1[i].num_packets

            self.assertEqual(counter2[0].num_packets, num_of_green_pkts)
            self.assertEqual(counter2[1].num_packets, num_of_yellow_pkts)
            self.assertEqual(counter2[2].num_packets, num_of_red_pkts)

            print "green pkts: ", num_of_green_pkts
            print "yellow pkts: ", num_of_yellow_pkts
            print "red pkts: ", num_of_red_pkts

        finally:
            # ip_acl
            self.client.switch_api_acl_dereference(0, acl, port1)
            self.client.switch_api_acl_rule_delete(0, acl, ace)
            self.client.switch_api_acl_list_delete(0, acl)

            self.client.switch_api_meter_delete(0, meter)
            #cleanup
            self.client.switch_api_neighbor_delete(0, neighbor)
            self.client.switch_api_nhop_delete(0, nhop)
            self.client.switch_api_l3_route_delete(0, vrf, i_ip3, if2)

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
@group('acl')
@group('meters')
@group('ent')
class StormControlTest(pd_base_tests.ThriftInterfaceDataPlane,
                       api_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        if test_param_get('target') == "bmv2":
            pd_base_tests.ThriftInterfaceDataPlane.__init__(self, [""], ["dc"])
        else:
            pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                            ["dc"])

    def set_meter_time(self, time, enable):
        if test_param_get('target') == "bmv2":
            sess_hdl = self.conn_mgr.client_init()
            dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))
            self.conn_mgr.set_meter_time(sess_hdl, dev_tgt, enable)
        else:
            sess_hdl = self.conn_mgr.client_init()
            dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))
            self.conn_mgr.set_meter_time(sess_hdl, 0, hex_to_i16(0xFFFF), time)
            self.conn_mgr.complete_operations(sess_hdl)

    def runTest(self):
        print
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.set_meter_time(0, 1)
        vlan = self.client.switch_api_vlan_create(0, 10)

        port1 = self.client.switch_api_port_id_to_handle_get(0, swports[1])
        i_info1 = switcht_interface_info_t(handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if1 = self.client.switch_api_interface_create(0, i_info1)

        port2 = self.client.switch_api_port_id_to_handle_get(0, swports[2])
        i_info2 = switcht_interface_info_t(handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(0, i_info2)

        self.client.switch_api_vlan_member_add(0, vlan, if1)
        self.client.switch_api_vlan_member_add(0, vlan, if2)

        switch_api_mac_table_entry_create(self,
            0, vlan, '00:11:11:11:11:11', 2, if1)
        switch_api_mac_table_entry_create(self,
            0, vlan, '00:22:22:22:22:22', 2, if2)

        api_meter_info = switcht_meter_info_t(
            meter_mode=2,
            color_source=1,
            meter_type=2,
            cbs=8,
            cir=1000,
            green_action=2,
            red_action=1)
        meter = self.client.switch_api_meter_create(0, api_meter_info)
        self.client.switch_api_port_storm_control_set(0, port1, 1, meter)

        pkt = simple_tcp_packet(
            eth_dst='00:22:22:22:22:22',
            eth_src='00:11:11:11:11:11',
            ip_dst='10.10.10.1',
            ip_id=101,
            ip_ttl=64)

        try:
            counter_ids = [0, 1, 2]
            counter1 = self.client.switch_api_meter_stats_get(0, meter,
                                                              counter_ids)

            if test_param_get('target') == "bmv2":
                num_of_green_pkts = 10
            else:
                num_of_green_pkts = 11

            for i in range(0, num_of_green_pkts):
                send_packet(self, swports[1], str(pkt))
                verify_packets(self, pkt, [swports[2]])

            self.set_meter_time(20, 1)

            num_of_red_pkts = 1
            send_packet(self, swports[1], str(pkt))
            verify_no_other_packets(self, timeout=1)

            self.set_meter_time(25000000, 1)

            if test_param_get('target') != "bmv2":
                num_of_red_pkts += 1
                send_packet(self, swports[1], str(pkt))
                verify_no_other_packets(self, timeout=1)

            self.set_meter_time(0, 0)

            num_of_green_pkts += 1
            send_packet(self, swports[1], str(pkt))
            verify_packets(self, pkt, [swports[2]])

            counter2 = self.client.switch_api_meter_stats_get(0, meter,
                                                              counter_ids)

            for i in range(0, 3):
                counter2[i].num_bytes -= counter1[i].num_bytes
                counter2[i].num_packets -= counter1[i].num_packets

            self.assertEqual(counter2[0].num_packets, num_of_green_pkts)
            self.assertEqual(counter2[2].num_packets, num_of_red_pkts)

            print "green pkts: ", num_of_green_pkts
            print "red pkts: ", num_of_red_pkts

        finally:
            switch_api_mac_table_entry_delete(self, 0, vlan, '00:11:11:11:11:11')
            switch_api_mac_table_entry_delete(self, 0, vlan, '00:22:22:22:22:22')
            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)
            self.client.switch_api_meter_delete(0, meter)
            self.client.switch_api_interface_delete(0, if1)
            self.client.switch_api_interface_delete(0, if2)
