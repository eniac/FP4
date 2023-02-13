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
Flowlet swithcing tests
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
import ptf.mask

from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../base'))

from common.utils import *
from common.api_utils import *
sys.path.append(os.path.join(this_dir, '../../base/api-tests'))
import api_base_tests

device = 0
cpu_port = 64
swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)

if swports == []:
    swports = [x for x in range(65)]


###############################################################################
@group('perf')
class L3PerfTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        num_entries = 30000
        print
        print "Adding %d route entries" % num_entries

        vrf = self.client.switch_api_vrf_create(device, 0)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

        port = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            v4_unicast_enabled=True,
            vrf_handle=vrf,
            rmac_handle=rmac)
        rif = self.client.switch_api_rif_create(0, rif_info)

        intf_info = switcht_interface_info_t(
            handle=port, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=rif)
        intf = self.client.switch_api_interface_create(device, intf_info)

        nhop_ip = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr='11.11.11.11', prefix_length=32)
        nhop = switch_api_nhop_create(self, device, rif, nhop_ip)
        route_entries = []
        try:
            for idx in range(num_entries):
                ip = '192.168.%d.%d' % (idx / 256, idx % 256)
                ip_addr = switcht_ip_addr_t(addr_type=SWITCH_API_IP_ADDR_V4,
                                            ipaddr=ip,
                                            prefix_length=32)
                route_entries.append(switcht_route_entry_t(
                    ip_addr=ip_addr, vrf_handle=vrf, nhop_handle=nhop))

            rate = self.client.switch_api_route_entry_add_perf_test(
                device=device, route_entries=route_entries)

            print "Rate:", rate
            self.assertTrue(rate > 35000)

        finally:
            for idx in range(num_entries):
                self.client.switch_api_l3_route_delete(
                    device, vrf, route_entries[idx].ip_addr, nhop)
            self.client.switch_api_nhop_delete(device, nhop)
            self.client.switch_api_interface_delete(device, intf)
            self.client.switch_api_rif_delete(0, rif)
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)

@group('perf')
class L2PerfTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        num_entries = 30000
        print
        print "Adding %d route entries" % num_entries

        vlan = self.client.switch_api_vlan_create(device, 10)

        port = self.client.switch_api_port_id_to_handle_get(device, swports[1])

        intf_info = switcht_interface_info_t(
            handle=port, type=SWITCH_INTERFACE_TYPE_ACCESS)
        intf = self.client.switch_api_interface_create(device, intf_info)
        mac_entries = []
        try:
            for idx in range(num_entries):
                mac_addr = '00:00:00:00:%02x:%02x' % (idx//256, idx%256)
                mac_entries.append(switcht_api_mac_entry_t(
                    network_handle=vlan,
                    mac_addr=mac_addr,
                    entry_type=2,
                    handle=intf))

            rate = self.client.switch_api_mac_entry_add_perf_test(
                device=device,
                mac_entries=mac_entries)

            print "Rate:", rate
            self.assertTrue(rate > 20000)

        finally:
            self.client.switch_api_mac_table_entry_flush(
                device, SWITCH_MAC_FLUSH_TYPE_ALL, 0x0, 0x0)
            self.client.switch_api_interface_delete(device, intf)
            self.client.switch_api_vlan_delete(device, vlan)
