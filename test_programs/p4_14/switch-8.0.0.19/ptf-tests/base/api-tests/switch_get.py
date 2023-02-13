###############################################################################
# BAREFOOT NETWORKS CONFIDENTIAL & PROPRIETARY
#
# Copyright (c) 2015-2018 Barefoot Networks, Inc.

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
import pd_base_tests

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *

import os
from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

from erspan3 import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '..'))
from common.utils import *
from common.api_utils import *

device = 0
cpu_port = 64

swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)

if swports == []:
    swports = [x for x in range(65)]


###############################################################################
class PortGetTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Get port details for port %d" % swports[1]

        port1 = self.client.switch_api_port_id_to_handle_get(
            device, swports[1])
        queue_hdls = self.client.switch_api_queues_get(device, port1)

        print "Port handle         : 0x%x" % port1
        port_id = self.client.switch_api_port_handle_to_id_get(device, port1)
        print "Port ID             : %d" % port_id
        speed = self.client.switch_api_port_speed_get(device, port1)
        print "Speed               : %d" % speed
        an_mode = self.client.switch_api_port_auto_neg_get(device, port1)
        print "Auto neg            : %d" % an_mode
        admin_mode = self.client.switch_api_port_admin_state_get(device, port1)
        print "Admin state         : %s" % ('Enabled'
                                            if admin_mode else 'Disabled')
        oper_state = self.client.switch_api_port_oper_status_get(device, port1)
        print "Operational         : %d" % oper_state
        lb_mode = self.client.switch_api_port_loopback_mode_get(device, port1)
        print "Loopback mode       : %d" % lb_mode
        rx_mtu = self.client.switch_api_port_rx_mtu_get(device, port1)
        print "RX MTU              : %d" % rx_mtu
        tx_mtu = self.client.switch_api_port_tx_mtu_get(device, port1)
        print "TX MTU              : %d" % tx_mtu
        api_port_info = self.client.switch_api_port_get(device, swports[1])
        print "Api port info #not implemented"
        print "Port      : %d" % api_port_info.port
        print "Speed     : %d" % api_port_info.port_speed
        print "Admin st  : %d" % api_port_info.initial_admin_state
        print "Rx/Tx mtu : %d/%d" % (api_port_info.tx_mtu,
                                     api_port_info.rx_mtu)
        print "FEC       : %d" % api_port_info.fec_mode
        meter_handle = self.client.switch_api_port_storm_control_get(
            device, port1, 0)
        print "Meter handle        : %d" % meter_handle
        counters = self.client.switch_api_storm_control_counters_get(
            device, meter_handle, [0, 1, 2, 3])
        print "Meter handle cntrs  :"
        print counters
        acl_handle = self.client.switch_api_port_ingress_acl_group_get(device, port1)
        print "ACL group handle    : %d" % acl_handle
        label = self.client.switch_api_port_ingress_acl_label_get(device, port1)
        print "Port label          : %d" % label
        port_stats = self.client.switch_api_port_stats_get(
            device, port1, [1, 3])
        print "All Octets          : %d" % port_stats[0]
        print "All Packets         : %d" % port_stats[1]
        bind_mode = self.client.switch_api_port_bind_mode_get(device, port1)
        print "Bind mode           : %d" % bind_mode
        max_queues = self.client.switch_api_port_max_queues_get(device, port1)
        print "Total queues        : %d" % max_queues
        pfc_map = self.client.switch_api_port_pfc_get(device, port1)
        print "Port PFC            : %d" % pfc_map
        rx_pause = self.client.switch_api_port_link_rx_pause_get(device, port1)
        print "Rx flow control     : %s" % ('Enabled'
                                            if rx_pause else 'Disabled')
        tx_pause = self.client.switch_api_port_link_tx_pause_get(device, port1)
        print "Tx flow control     : %s" % ('Enabled'
                                            if tx_pause else 'Disabled')
        fec_mode = self.client.switch_api_port_fec_mode_get(device, port1)
        print "FEC                 : %d" % fec_mode
        ing_mirror = self.client.switch_api_port_ingress_mirror_get(
            device, port1)
        print "Ingress mirror      : 0x%x" % ing_mirror
        ing_sflow_handle = self.client.switch_api_port_ingress_sflow_handle_get(
            device, port1)
        print "Ing sflow handle    : 0x%x" % ing_sflow_handle
        eg_sflow_handle = self.client.switch_api_port_egress_sflow_handle_get(
            device, port1)
        print "Eg sflow handle     : 0x%x" % eg_sflow_handle
        ing_qos_handle = self.client.switch_api_port_ingress_qos_handle_get(
            device, port1)
        print "Ing qos handle      : 0x%x" % ing_qos_handle
        tc_q_handle = self.client.switch_api_port_tc_queue_handle_get(
            device, port1)
        print "tc queue handle     : 0x%x" % tc_q_handle
        tc_ppg_handle = self.client.switch_api_port_tc_ppg_handle_get(
            device, port1)
        print "tc ppg handle       : 0x%x" % tc_ppg_handle
        eg_qos_handle = self.client.switch_api_port_egress_qos_handle_get(
            device, port1)
        print "Eg qos handle       : 0x%x" % eg_qos_handle
        num_ppgs = self.client.switch_api_port_max_ppg_get(device, port1)
        print "Total PPGs          : %d" % num_ppgs
        ppg_handles = self.client.switch_api_port_ppg_get(device, port1)
        print "PPG Handles         :"
        for each in ppg_handles:
            print "    0x%x" % each
        qos_map_handle = self.client.switch_api_port_pfc_priority_to_ppg_get(
            device, port1)
        print "QoS map handle      : %d" % qos_map_handle
        group_handle_cnt = self.client.switch_api_port_queue_scheduler_group_handle_count_get(
            device, port1)
        print "Group handle cnt    : %d" % group_handle_cnt
        group_handles = self.client.switch_api_port_qos_scheduler_group_handles_get(
            device, port1)
        print "Group Handles       :"
        for each in group_handles:
            print "    0x%x" % each
        scheduler_handle = self.client.switch_api_port_scheduler_profile_get(
            device, port1)
        print "Scheduler handle    : %d" % scheduler_handle
        port_idx = self.client.switch_api_queue_index_get(
            device, queue_hdls[0])
        print "Port idx for queue 0: %d" % port_idx
        port_hdl = self.client.switch_api_queue_port_get(device, queue_hdls[0])
        print "Port hdl for queue 0: 0x%x" % port_hdl


###############################################################################
class VlanGetTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Get vlan details for vlan id 10"

        try:
            vlan = self.client.switch_api_vlan_create(device, 10)
            port1 = self.client.switch_api_port_id_to_handle_get(
                device, swports[1])
            port2 = self.client.switch_api_port_id_to_handle_get(
                device, swports[2])

            i_info1 = switcht_interface_info_t(
                handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
            if1 = self.client.switch_api_interface_create(device, i_info1)
            i_info2 = switcht_interface_info_t(
                handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
            if2 = self.client.switch_api_interface_create(device, i_info2)

            self.client.switch_api_vlan_member_add(device, vlan, if1)
            self.client.switch_api_vlan_member_add(device, vlan, if2)

            stp = self.client.switch_api_stp_group_create(device, 1)
            self.client.switch_api_stp_group_member_add(device, stp, vlan)

            vlan_handle = self.client.switch_api_vlan_id_to_handle_get(
                device, 10)
            self.assertEqual(vlan, vlan_handle)
            print "Vlan handle         : 0x%x" % vlan_handle
            vlan_id = self.client.switch_api_vlan_handle_to_id_get(
                device, vlan)
            print "Vlan ID             : %d" % vlan_id
            bd = self.client.switch_api_vlan_bd_get(device, vlan)
            print "BD value            : %d" % bd
            vlan_info = self.client.switch_api_vlan_attribute_get(
                device, vlan, 0xFF)
            print "Vlan Attributes     :"
            print "Learning   : %d" % vlan_info.learning_enabled
            print "IGMP snoop : %d" % vlan_info.igmp_snooping_enabled
            print "MDL snoop  : %d" % vlan_info.mld_snooping_enabled
            print "Aging int  : %d" % vlan_info.aging_interval
            print "STP        : 0x%x" % vlan_info.stp_handle
            print "Mrpf grp   : %d" % vlan_info.mrpf_group
            learning = self.client.switch_api_vlan_learning_get(device, vlan)
            print "Learning enabled    : %s" % ('Enabled'
                                                if learning else 'Disabled')
            igmp_snooping = self.client.switch_api_vlan_igmp_snooping_get(
                device, vlan)
            print "IGMP snoop enabled  : %s" % (
                'Enabled' if igmp_snooping else 'Disabled')
            mld_snooping = self.client.switch_api_vlan_mld_snooping_get(
                device, vlan)
            print "MLD snoop enabled   : %s" % (
                'Enabled' if mld_snooping else 'Disabled')
            aging_int = self.client.switch_api_vlan_aging_interval_get(
                device, vlan)
            print "Aging interval      : %d" % aging_int
            stp_handle = self.client.switch_api_vlan_stp_handle_get(
                device, vlan)
            print "STP handle          : 0x%x" % stp_handle
            mrpf_group = self.client.switch_api_vlan_mrpf_group_get(
                device, vlan)
            print "Mrpf group          : %d" % mrpf_group
            acl_group_handle = self.client.switch_api_vlan_ingress_acl_group_get(
                device, vlan)
            print "ACL grp handle      : %d" % acl_group_handle
            label = self.client.switch_api_vlan_ingress_acl_label_get(device, vlan)
            print "Vlan acl Label      : %d" % label
            stats = self.client.switch_api_vlan_stats_get(device, vlan, [1, 3])
            for each in stats:
                print each
            mbrs = self.client.switch_api_vlan_interfaces_get(device, vlan)
            for mbr in mbrs:
                print "Vlan Member     : 0x%x" % mbr
                id = self.client.switch_api_vlan_member_vlan_id_get(
                    device, mbr)
                tag = self.client.switch_api_vlan_member_vlan_tagging_mode_get(
                    device, mbr)
                intf = self.client.switch_api_vlan_member_intf_handle_get(
                    device, mbr)
                self.assertEqual(id, vlan_id)
                print "Id : %d Tag mode : %d Intf handle : 0x%x" % (id, tag,
                                                                    intf)
            stp_mbrs = self.client.switch_api_stp_group_members_get(
                device, stp)
            for mbr in stp_mbrs:
                print "STP group member: 0x%x" % mbr

        finally:
            self.client.switch_api_stp_group_member_remove(device, stp, vlan)
            self.client.switch_api_stp_group_delete(device, stp)
            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_vlan_member_remove(device, vlan, if2)
            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_vlan_delete(device, vlan)


###############################################################################
class AclGetTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Get ACL details"

        #acl
        acl = self.client.switch_api_acl_list_create(
            0, SWITCH_API_DIRECTION_INGRESS, SWITCH_ACL_TYPE_IP,
            SWITCH_HANDLE_TYPE_PORT)
        # create kvp to match destination IP
        kvp = []
        kvp_val = switcht_acl_value_t(value_num=int("0a0a0a01", 16))
        kvp_mask = switcht_acl_value_t(value_num=int("ffffffff", 16))
        kvp.append(
            switcht_acl_key_value_pair_t(SWITCH_ACL_IP_FIELD_IPV4_DEST,
                                         kvp_val, kvp_mask))
        kvp_val = switcht_acl_value_t(value_num=int("2", 16))
        kvp_mask = switcht_acl_value_t(value_num=int("ff", 16))
        kvp.append(
            switcht_acl_key_value_pair_t(SWITCH_ACL_IP_FIELD_TCP_FLAGS,
                                         kvp_val, kvp_mask))
        action = 1
        action_params = switcht_acl_action_params_t(
            redirect=switcht_acl_action_redirect(handle=0))
        opt_action_params = switcht_acl_opt_action_params_t()
        ace = self.client.switch_api_acl_ip_rule_create(
            0, acl, 10, 2, kvp, action, action_params, opt_action_params)

        acl_type = self.client.switch_api_acl_type_get(device, acl)
        self.assertEqual(acl_type, SWITCH_ACL_TYPE_IP)
        action_spec = self.client.switch_api_acl_entry_action_get(device, ace)
        print "Action                : %d" % action_spec.action
        action_params = action_spec.action_params
        print "CPU Redirect reason code: %d" % action_params.cpu_redirect.reason_code
        print "Redirect handle       : 0x%x " % action_params.redirect.handle
        opt_action_params = action_spec.opt_action_params
        print "Mirror handle         : 0x%x" % opt_action_params.mirror_handle
        print "Meter handle          : 0x%x" % opt_action_params.meter_handle
        print "Counter handle        : 0x%x" % opt_action_params.counter_handle
        print "NAT mode              : %d" % opt_action_params.nat_mode
        field_count = self.client.switch_api_acl_entry_rules_count_get(
            device, ace)
        self.assertEqual(field_count, 2)
        acl_handle = self.client.switch_api_acl_entry_acl_table_get(
            device, ace)
        self.assertEqual(acl_handle, acl)

        #range acl
        if (test_param_get('target') != 'bmv2'):
            switch_range = switcht_range_t(start_value=1000, end_value=2000)
            acl_range_handle = self.client.switch_api_acl_range_create(
                0, SWITCH_API_DIRECTION_INGRESS, SWITCH_RANGE_TYPE_SRC_PORT,
                switch_range)

            range_type = self.client.switch_api_acl_range_type_get(
                device, acl_range_handle)
            self.assertEqual(range_type, SWITCH_RANGE_TYPE_SRC_PORT)
            range = self.client.switch_api_acl_range_get(device, acl_range_handle)
            self.assertEqual(range.start_value, switch_range.start_value)
            self.assertEqual(range.end_value, switch_range.end_value)
            self.client.switch_api_acl_range_delete(device, acl_range_handle)

        #cleanup
        self.client.switch_api_acl_rule_delete(device, acl, ace)
        self.client.switch_api_acl_list_delete(device, acl)


###############################################################################
class RifGetTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Get RIF details"

        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac,
                                              '00:77:66:55:44:33')

        port1 = self.client.switch_api_port_id_to_handle_get(
            device, swports[0])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=True)
        rif1 = self.client.switch_api_rif_create(device, rif_info1)
        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        vrf_handle = self.client.switch_api_rif_vrf_handle_get(device, rif1)
        self.assertEqual(vrf_handle, vrf)
        print "VRF Handle        : 0x%x" % vrf_handle
        intf_handle = self.client.switch_api_rif_intf_handle_get(device, rif1)
        self.assertEqual(if1, intf_handle)
        print "Intf Handle       : 0x%x" % intf_handle
        ipv4_unicast = self.client.switch_api_rif_ipv4_unicast_get(
            device, rif1)
        print "IPv4 unicast en   : %d" % ipv4_unicast
        ipv6_unicast = self.client.switch_api_rif_ipv6_unicast_get(
            device, rif1)
        print "IPv6 mcast en     : %d" % ipv6_unicast
        ipv4_mcast = self.client.switch_api_rif_ipv4_multicast_get(
            device, rif1)
        print "IPv4 unicast en   : %d" % ipv4_mcast
        ipv6_mcast = self.client.switch_api_rif_ipv6_multicast_get(
            device, rif1)
        print "IPv6 mcast en     : %d" % ipv6_mcast
        mtu_handle = self.client.switch_api_rif_mtu_get(device, rif1)
        print "MTU handle        : 0x%x" % mtu_handle
        rif_type = self.client.switch_api_rif_type_get(device, rif1)
        print "RIF type          : %d" % rif_type
        rif_info = self.client.switch_api_rif_attribute_get(device, rif1, 0)
        rmac_handle = self.client.switch_api_rif_rmac_handle_get(device, rif1)
        print "RMAC handle       : 0x%x" % rmac_handle
        acl_handle = self.client.switch_api_rif_ingress_acl_group_get(device, rif1)
        print "ACL handle        : 0x%x" % acl_handle
        label = self.client.switch_api_rif_ingress_acl_label_get(device, rif1)
        print "VLAN label        : %d" % label
        bd = self.client.switch_api_rif_bd_get(device, rif1)
        print "BD value          : %d" % bd

        self.client.switch_api_interface_delete(device, if1)
        self.client.switch_api_rif_delete(device, rif1)
        self.client.switch_api_router_mac_delete(device, rmac,
                                                 '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(device, rmac)
        self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
class LagGetTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Get LAG details"

        port1 = self.client.switch_api_port_id_to_handle_get(
            device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(
            device, swports[2])

        lag = self.client.switch_api_lag_create(device)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port1)
        self.client.switch_api_lag_member_add(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port2)

        count = self.client.switch_api_lag_member_count_get(device, lag)
        print "LAG Member Count   : %d" % count
        mbrs = self.client.switch_api_lag_members_get(device, lag)
        for mbr in mbrs:
            hdl = self.client.switch_api_lag_member_port_handle_get(
                device, mbr)
            print "LAG Member     : 0x%x" % mbr
            print "Intf handle    : 0x%x" % hdl
        acl_hdl = self.client.switch_api_lag_ingress_acl_group_get(device, lag)
        print "ACL group handle   : 0x%x" % acl_hdl
        label = self.client.switch_api_lag_ingress_acl_label_get(device, lag)
        print "Port label         : %d" % label
        bind_mode = self.client.switch_api_lag_bind_mode_get(device, lag)
        print "Bind mode          : %d" % bind_mode

        self.client.switch_api_lag_member_delete(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port1)
        self.client.switch_api_lag_member_delete(
            device, lag_handle=lag, side=SWITCH_API_DIRECTION_BOTH, port=port2)
        self.client.switch_api_lag_delete(device, lag)


###############################################################################
class InterfaceGetTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Get interface details"

        port1 = self.client.switch_api_port_id_to_handle_get(
            device, swports[1])

        i_info1 = switcht_interface_info_t(
            handle=port1, type=SWITCH_INTERFACE_TYPE_PORT)
        if1 = self.client.switch_api_interface_create(device, i_info1)

        ifidx = self.client.switch_api_interface_ifindex_get(device, if1)
        print "IF index          : %d" % ifidx
        port_hdl = self.client.switch_api_interface_handle_get(device, if1)
        self.assertEqual(port1, port_hdl)
        print "Port handle       : 0x%x" % port_hdl
        intf_hdl = self.client.switch_api_interface_by_type_get(
            device, port1, SWITCH_INTERFACE_TYPE_PORT)
        self.assertEqual(intf_hdl, if1)
        print "Intf handle       : 0x%x" % intf_hdl
        vlan_hdl = self.client.switch_api_interface_native_vlan_get(
            device, if1)
        print "Vlan handle       : 0x%x" % vlan_hdl
        vlan_id = self.client.switch_api_interface_native_vlan_id_get(
            device, if1)
        print "Vlan ID           : %d" % vlan_id
        intf_info = self.client.switch_api_interface_attribute_get(
            device, if1, 0)
        print "Interface info    :"
        print "Type           : %d" % intf_info.type
        print "Handle         : 0x%x" % intf_info.handle
        print "RIF            : 0x%x" % intf_info.rif_handle
        print "VLAN ID        : %d" % intf_info.vlan
        print "VLAN handle    : %d" % intf_info.native_vlan_handle
        print "Flood enabled  : %d" % intf_info.flood_enabled
        ln_hdl = self.client.switch_api_interface_ln_handle_get(device, if1)
        print "LN handle         : %d" % ln_hdl

        self.client.switch_api_interface_delete(device, if1)


###############################################################################
class DeviceGetTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Get device details"

        device_info = self.client.switch_api_device_attribute_get(
            device, 0xFFFFFFFF)
        print "Device info         :"
        print "Default VRF         : %d" % device_info.default_vrf
        print "Default VRF handle  : 0x%x" % device_info.vrf_handle
        print "Default vlan        : %d" % device_info.default_vlan
        print "Default vlan handle : 0x%x" % device_info.vlan_handle
        print "RMAC handle         : 0x%x" % device_info.rmac_handle
        print "MAX lag groups      : %d" % device_info.max_lag_groups
        print "MAX lag members     : %d" % device_info.max_lag_members
        print "MAX ecmp groups     : %d" % device_info.max_ecmp_groups
        print "MAX ecmp members    : %d" % device_info.max_ecmp_members
        print "LAG hash algo       : %d" % device_info.lag_hash_algorithm
        print "LAG hash flags      : 0x%x" % device_info.lag_hash_flags
        print "ECMP hash algo      : %d" % device_info.ecmp_hash_algorithm
        print "ECMP hash flags     : 0x%x" % device_info.ecmp_hash_flags
        print "Default log level   : %d" % device_info.default_log_level
        print "Install DMAC        : %d" % device_info.install_dmac
        print "MAX vrf             : %d" % device_info.max_vrf
        print "MAX ports           : %d" % device_info.max_ports
        print "Eth CPU port        : %d" % device_info.eth_cpu_port
        print "PCIE CPU port       : %d" % device_info.pcie_cpu_port
        print "Refresh interval    : %d" % device_info.refresh_interval
        print "Aging interval      : %d" % device_info.aging_interval
        print "Num active ports    : %d" % device_info.num_active_ports
        print "MAX port mtu        : %d" % device_info.max_port_mtu

        print "Port List (top 5) :"
        for each in device_info.port_list[:5]:
            print "0x%x" % each
        rmac_hdl = self.client.switch_api_device_default_rmac_handle_get(
            device)
        print "RMAC handle      : 0x%x" % rmac_hdl
        vrf_hdl = self.client.switch_api_device_default_vrf_handle_get(device)
        print "Default VRF hdl  : 0x%x" % vrf_hdl
        vrf_id = self.client.switch_api_device_default_vrf_id_get(device)
        print "Default VRF      : %d" % vrf_id
        vlan_hdl = self.client.switch_api_device_default_vlan_handle_get(
            device)
        print "Default vlan hdl : 0x%x" % vlan_hdl
        vlan_id = self.client.switch_api_device_default_vlan_id_get(device)
        print "Default vlan     : %d" % vlan_id
        cpu_port_hdl = self.client.switch_api_device_cpu_port_handle_get(
            device)
        cpu_eth_port = self.client.switch_api_device_cpu_eth_port_get(device)
        print "Eth CPU port     : %d" % cpu_eth_port
        cpu_pcie_port = self.client.switch_api_device_cpu_pcie_port_get(device)
        print "PCIE CPU port    : %d" % cpu_pcie_port
        refr = self.client.switch_api_device_counter_refresh_interval_get(
            device)
        print "Refresh interval : %d" % refr
        aging = self.client.switch_api_device_mac_aging_interval_get(device)
        print "Aging internval  : %d" % aging
        recirc_port_hdl = self.client.switch_api_device_recirc_port_get(
            device, 0)
        print "Recirc port hdl  : 0x%x" % recirc_port_hdl
        max_recirc_ports = self.client.switch_api_device_max_recirc_ports_get(
            device)
        print "MAX recirc ports : %d" % max_recirc_ports
        action = self.client.switch_api_device_dmac_miss_packet_action_get(
            device, 2)
        print "DMAC miss action : %d" % action
        cutthru = self.client.switch_api_device_cut_through_mode_get(device)
        print "Cut thru mode    : %d" % cutthru

        handles = self.client.switch_api_handles_get(device, 1)
        print "Port handles (top 5):"
        for each in handles[:5]:
            print "0x%x" % each
        max_queues = self.client.switch_api_max_queues_get(device)
        print "MAX queues       : %d" % max_queues
        max_cpu_queues = self.client.switch_api_max_cpu_queues_get(device)
        print "MAX CPU queues   : %d" % max_cpu_queues
        traffic_class = self.client.switch_api_max_traffic_class_get(device)
        print "Traffic classes  : %d" % traffic_class


###############################################################################
class HostIfGetTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Get hostif details"

        try:
            vrf = self.client.switch_api_vrf_create(device, 2)
            rmac = self.client.switch_api_router_mac_group_create(
                device, SWITCH_RMAC_TYPE_INNER)
            self.client.switch_api_router_mac_add(device, rmac,
                                                  '00:11:22:33:44:55')
            self.cpu_port = get_cpu_port(self)

            port1 = self.client.switch_api_port_id_to_handle_get(
                device, swports[1])

            rif_info1 = switcht_rif_info_t(
                rif_type=SWITCH_RIF_TYPE_INTF,
                vrf_handle=vrf,
                rmac_handle=rmac,
                v4_unicast_enabled=True)
            rif1 = self.client.switch_api_rif_create(0, rif_info1)
            i_info1 = switcht_interface_info_t(
                handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
            if1 = self.client.switch_api_interface_create(device, i_info1)
            i_ip1 = switcht_ip_addr_t(
                addr_type=SWITCH_API_IP_ADDR_V4,
                ipaddr='192.168.0.2',
                prefix_length=16)
            self.client.switch_api_l3_interface_address_add(
                device, rif1, vrf, i_ip1)

            cpu_port_handle = self.client.switch_api_port_id_to_handle_get(
                device, self.cpu_port)
            queue_handles = self.client.switch_api_queues_get(
                device, cpu_port_handle)

            hostif_group1 = switcht_hostif_group_t(
                queue_handles[0], policer_handle=0)
            hostif_group_id1 = self.client.switch_api_hostif_group_create(
                device, hostif_group1)

            flags = 0
            flags |= SWITCH_HOSTIF_RCODE_ATTR_REASON_CODE
            flags |= SWITCH_HOSTIF_RCODE_ATTR_PACKET_ACTION
            flags |= SWITCH_HOSTIF_RCODE_ATTR_HOSTIF_GROUP

            arp_req_rcode_info = switcht_hostif_rcode_info_t(
                reason_code=SWITCH_HOSTIF_REASON_CODE_ARP_REQUEST,
                action=SWITCH_ACL_ACTION_COPY_TO_CPU,
                hostif_group_id=hostif_group_id1)
            rcode_handle1 = self.client.switch_api_hostif_reason_code_create(
                device, flags, arp_req_rcode_info)

            hostif_name1 = "rif1"
            hostif1 = switcht_hostif_t(
                intf_name=hostif_name1,
                handle=rif1,
                mac='00:11:22:33:44:55',
                v4addr=i_ip1,
                operstatus=True,
                admin_state=True)
            hostif_flags = 0
            hostif_flags |= SWITCH_HOSTIF_ATTR_INTERFACE_NAME
            hostif_flags |= SWITCH_HOSTIF_ATTR_HANDLE
            hostif_flags |= SWITCH_HOSTIF_ATTR_MAC_ADDRESS
            hostif_flags |= SWITCH_HOSTIF_ATTR_IPV4_ADDRESS
            hostif_flags |= SWITCH_HOSTIF_ATTR_OPER_STATUS
            hostif_flags |= SWITCH_HOSTIF_ATTR_ADMIN_STATE
            hostif_id1 = self.client.switch_api_hostif_create(
                device, hostif_flags, hostif1)

            hostif_hdl = self.client.switch_api_hostif_handle_get(
                device, "rif1")
            self.assertEqual(hostif_hdl, hostif_id1)
            print "Hostif handle       : 0x%x" % hostif_hdl
            hostif_grp = self.client.switch_api_hostif_group_get(
                device, hostif_group_id1)
            self.assertEqual(hostif_grp.queue_handle, queue_handles[0])
            oper_state = self.client.switch_api_hostif_oper_state_get(
                device, hostif_id1)
            print "Hostif oper state   : %d" % oper_state
            nhop_hdl = self.client.switch_api_hostif_nhop_get(
                device, SWITCH_HOSTIF_REASON_CODE_GLEAN)
            print "NHOP handle         : 0x%x" % nhop_hdl

        finally:
            self.client.switch_api_hostif_reason_code_delete(
                device, rcode_handle1)
            self.client.switch_api_hostif_delete(device, hostif_id1)
            self.client.switch_api_hostif_group_delete(device,
                                                       hostif_group_id1)

            self.client.switch_api_l3_interface_address_delete(
                device, rif1, vrf, i_ip1)
            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:11:22:33:44:55')
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
class L2GetTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Get L2 details"

        try:
            vlan = self.client.switch_api_vlan_create(device, 10)

            port1 = self.client.switch_api_port_id_to_handle_get(
                device, swports[0])
            i_info1 = switcht_interface_info_t(
                handle=port1, type=SWITCH_INTERFACE_TYPE_ACCESS)
            if1 = self.client.switch_api_interface_create(device, i_info1)
            self.client.switch_api_vlan_member_add(device, vlan, if1)

            switch_api_mac_table_entry_create(
                self, device, vlan, '00:22:22:22:22:22', 2, if1)

            mac_hdl = self.client.switch_api_mac_entry_handle_get(
                device, vlan, '00:22:22:22:22:22')
            print "MAC handle          : 0x%x" % mac_hdl
            intf_hdl = self.client.switch_api_mac_entry_intf_handle_get(
                device, vlan, '00:22:22:22:22:22')
            self.assertEqual(intf_hdl, if1)
            print "Interface handle    : 0x%x" % intf_hdl
            entry_type = self.client.switch_api_mac_entry_type_get(
                device, vlan, '00:22:22:22:22:22')
            self.assertEqual(entry_type, 2)
            print "Entry type          : %d" % entry_type
            mac_action = self.client.switch_api_mac_entry_packet_action_get(
                device, vlan, '00:22:22:22:22:22')
            self.assertEqual(mac_action, 1)  # 1 is SWITCH_MAC_ACTION_FORWARD
            print "MAC action          : %d" % mac_action

        finally:
            switch_api_mac_table_entry_delete(
                self, device, vlan, '00:22:22:22:22:22')
            self.client.switch_api_vlan_member_remove(device, vlan, if1)
            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_vlan_delete(device, vlan)


###############################################################################
class BufferGetTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Get buffer pool details"

        try:
            port1 = self.client.switch_api_port_id_to_handle_get(
                device, swports[1])
            ppg_handles = self.client.switch_api_port_ppg_get(device, port1)
            queue_handles = self.client.switch_api_queues_get(device, port1)

            pool_info = switcht_buffer_pool_t(
                dir=1, pool_size=1000, threshold=2, xoff_size=500)
            ing_pool_handle = self.client.switch_api_buffer_pool_create(
                device, pool_info)

            buffer_info = switcht_buffer_profile_t(
                threshold_mode=2,
                threshold=0xFF,
                pool_handle=ing_pool_handle,
                buffer_size=1000,
                xoff_threshold=500,
                xon_threshold=800)
            ing_profile_handle = self.client.switch_api_buffer_profile_create(
                device, buffer_info)

            #self.client.switch_api_ppg_buffer_profile_set(
            #    device, ppg_handles[0], ing_profile_handle)
            #self.client.switch_api_queue_buffer_profile_set(
            #    device, queue_handles[0], ing_profile_handle)

            info = self.client.switch_api_buffer_profile_info_get(
                device, ing_profile_handle)
            thres_mode = self.client.switch_api_buffer_pool_threshold_mode_get(
                device, ing_pool_handle)
            self.assertEqual(thres_mode, 2)
            print "Pool threshold mode     : %d" % thres_mode
            size = self.client.switch_api_buffer_pool_size_get(
                device, ing_pool_handle)
            self.assertEqual(size, 1000)
            print "Pool size               : %d" % size
            type = self.client.switch_api_buffer_pool_type_get(
                device, ing_pool_handle)
            self.assertEqual(type, 1)
            print "Pool direction          : %d" % type
            xoff_size = self.client.switch_api_buffer_pool_xoff_size_get(
                device, ing_pool_handle)
            self.assertEqual(xoff_size, 500)
            print "Pool Xoff size          : %d" % xoff_size
            profile_hdl = self.client.switch_api_ppg_buffer_profile_get(
                device, ppg_handles[0])
            #self.assertEqual(profile_hdl, ing_profile_handle)
            print "PG buffer profile handle      : 0x%x" % profile_hdl
            profile_hdl = self.client.switch_api_queue_buffer_profile_get(
                device, queue_handles[0])
            #self.assertEqual(profile_hdl, ing_profile_handle)
            print "Queue buffer profile handle   : 0x%x" % profile_hdl
            port_hdl = self.client.switch_api_priority_group_port_get(
                device, ppg_handles[0])
            self.assertEqual(port_hdl, port1)
            print "Pool port handle        : 0x%x" % port_hdl
            ppg_idx = self.client.switch_api_priority_group_index_get(
                device, ppg_handles[0])
            print "Ppg index               : %d" % ppg_idx
        finally:
            #self.client.switch_api_ppg_buffer_profile_set(
            #    device, ppg_handles[0], 0)
            #self.client.switch_api_queue_buffer_profile_set(
            #    device, queue_handles[0], 0)

            self.client.switch_api_buffer_profile_delete(
                device, ing_profile_handle)
            self.client.switch_api_buffer_pool_delete(device, ing_pool_handle)


###############################################################################
class VrfRmacGetTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Get VRF details"

        try:
            vrf = self.client.switch_api_vrf_create(device, 2)
            rmac = self.client.switch_api_router_mac_group_create(
                device, SWITCH_RMAC_TYPE_INNER)
            self.client.switch_api_router_mac_add(device, rmac,
                                                  '00:77:66:55:44:33')
            self.client.switch_api_router_mac_add(device, rmac,
                                                  '00:77:66:55:44:34')

            vrf_hdl = self.client.switch_api_vrf_id_to_handle_get(device, 2)
            self.assertEqual(vrf, vrf_hdl)
            vrf_id = self.client.switch_api_vrf_handle_to_id_get(device, vrf)
            self.assertEqual(vrf_id, 2)
            macs = self.client.switch_api_rmac_macs_get(device, rmac)
            for mac in macs:
                print "%s" % mac
        finally:
            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:34')
            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
class NhopGetTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Get NHOP details"

        try:
            vrf = self.client.switch_api_vrf_create(device, 2)

            rmac = self.client.switch_api_router_mac_group_create(
                device, SWITCH_RMAC_TYPE_INNER)
            self.client.switch_api_router_mac_add(device, rmac,
                                                  '00:77:66:55:44:33')

            port1 = self.client.switch_api_port_id_to_handle_get(
                device, swports[0])

            rif_info1 = switcht_rif_info_t(
                rif_type=SWITCH_RIF_TYPE_INTF,
                vrf_handle=vrf,
                rmac_handle=rmac,
                v4_urpf_mode=1,
                v4_unicast_enabled=1)
            rif1 = self.client.switch_api_rif_create(0, rif_info1)
            i_info1 = switcht_interface_info_t(
                handle=port1, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif1)
            if1 = self.client.switch_api_interface_create(device, i_info1)
            intf_ip1 = switcht_ip_addr_t(
                addr_type=SWITCH_API_IP_ADDR_V4,
                ipaddr='192.168.0.2',
                prefix_length=16)
            self.client.switch_api_l3_interface_address_add(
                device, rif1, vrf, intf_ip1)

            i_ip1 = switcht_ip_addr_t(
                addr_type=SWITCH_API_IP_ADDR_V4,
                ipaddr='192.168.0.1',
                prefix_length=32)
            nhop1, neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, rif1, i_ip1, '00:11:22:33:44:55')

            nhop_key1 = switcht_nhop_key_t(handle=rif1, ip_addr=i_ip1)
            nhop_hdl = self.client.switch_api_nhop_handle_get(
                device, nhop_key1)
            print "NHOP handle       : 0x%x" % nhop_hdl
            neigh_hdl = self.client.switch_api_neighbor_handle_get(
                device, nhop1)
            self.assertEqual(neigh_hdl, neighbor1)
            print "NHOP neigh handle : 0x%x" % neigh_hdl
            type = self.client.switch_api_nhop_id_type_get(device, nhop1)
            print "NHOP id type      : %d" % type
            size = self.client.switch_api_nhop_table_size_get(device)
            print "NHOP table size   : %d" % size
            mac = self.client.switch_api_neighbor_entry_rewrite_mac_get(
                device, neighbor1)
            print "MAC               : %s" % mac

        finally:
            self.client.switch_api_neighbor_delete(device, neighbor1)
            self.client.switch_api_nhop_delete(device, nhop1)
            self.client.switch_api_l3_interface_address_delete(
                device, rif1, vrf, intf_ip1)
            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_router_mac_delete(device, rmac,
                                                     '00:77:66:55:44:33')
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
class MtreeGetTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Get mroute tree details"

        try:
            vrf = self.client.switch_api_vrf_create(device, 2)
            vlan = self.client.switch_api_vlan_create(device, 10)
            print "Vlan 0x%x" % vlan
            mtree = self.client.switch_api_multicast_tree_create(device)
            rpf = self.client.switch_api_rpf_create(device, 2, 1)
            src_ip = switcht_ip_addr_t(
                addr_type=SWITCH_API_IP_ADDR_V4,
                ipaddr='10.0.10.5',
                prefix_length=32)
            grp_ip = switcht_ip_addr_t(
                addr_type=SWITCH_API_IP_ADDR_V4,
                ipaddr='230.1.1.5',
                prefix_length=32)
            self.client.switch_api_multicast_mroute_add(
                device, mtree, rpf, vrf, src_ip, grp_ip, 1)
            self.client.switch_api_multicast_l2route_add(
                device, mtree, vlan, src_ip, grp_ip)

            mroute_tree = self.client.switch_api_multicast_mroute_tree_get(
                device, vrf, src_ip, grp_ip)
            self.assertEqual(mroute_tree.mgid_handle, mtree)
            self.assertEqual(mroute_tree.rpf_handle, rpf)
            print "Multicast Tree      : 0x%x" % mtree
            print "RPF                 : 0x%x" % rpf
            mgid_hdl = self.client.switch_api_multicast_l2route_tree_get(
                device, vlan, src_ip, grp_ip)
            self.assertEqual(mgid_hdl, mtree)
            print "L2MC tree           : 0x%x" % mgid_hdl

        finally:
            self.client.switch_api_multicast_mroute_delete(
                device, vrf, src_ip, grp_ip)
            self.client.switch_api_multicast_l2route_delete(
                device, vlan, src_ip, grp_ip)
            self.client.switch_api_rpf_delete(device, rpf)
            self.client.switch_api_multicast_tree_delete(device, mtree)
            self.client.switch_api_vlan_delete(device, vlan)
            self.client.switch_api_vrf_delete(device, vrf)


###############################################################################
class MirrorGetTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Get mirror details"

        try:
            port1 = self.client.switch_api_port_id_to_handle_get(
                device, swports[1])
            mirror_info = switcht_mirror_info_t(
                session_id=1,
                direction=1,
                egress_port_handle=port1,
                mirror_type=SWITCH_MIRROR_TYPE_LOCAL,
                session_type=0,
                cos=0,
                max_pkt_len=0,
                ttl=0,
                nhop_handle=0)
            mirror = self.client.switch_api_mirror_session_create(
                device, mirror_info)
            type = self.client.switch_api_mirror_session_type_get(
                device, mirror)
            self.assertEqual(type, SWITCH_MIRROR_TYPE_LOCAL)
            minfo = self.client.switch_api_mirror_session_info_get(
                device, mirror)
            print "Mirror type         : %d" % minfo.mirror_type
            print "Mirror session id   : %d" % minfo.session_id
            print "Mirror direction    : %d" % minfo.direction

        finally:
            self.client.switch_api_mirror_session_delete(device, mirror)


###############################################################################
class QosGetTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print "Get qos map details"

        try:
            qos_map1 = switcht_qos_map_t(dscp=1, tc=20)
            qos_map2 = switcht_qos_map_t(dscp=2, tc=24)
            qos_map3 = switcht_qos_map_t(dscp=3, tc=28)
            qos_map4 = switcht_qos_map_t(dscp=4, tc=32)
            ingress_qos_map_list = [qos_map1, qos_map2, qos_map3, qos_map4]
            ingress_qos_handle = self.client.switch_api_qos_map_ingress_create(
                device,
                qos_map_type=SWITCH_QOS_MAP_INGRESS_DSCP_TO_TC,
                qos_map=ingress_qos_map_list)

            dir = self.client.switch_api_qos_map_dir_get(
                device, ingress_qos_handle)
            print "Direction        : %d" % dir
            ig_map_type = self.client.switch_api_qos_map_ig_map_type_get(
                device, ingress_qos_handle)
            self.assertEqual(ig_map_type, SWITCH_QOS_MAP_INGRESS_DSCP_TO_TC)
            print "Ingress map type : %d" % ig_map_type
            map_list = self.client.switch_api_qos_map_list_get(
                device, ingress_qos_handle)
            for map in map_list:
                print "DSCP %d TC %d" % (map.dscp, map.tc)

        finally:
            self.client.switch_api_qos_map_ingress_delete(
                device, qos_map_handle=ingress_qos_handle)
