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
Thrift API interface negative tests
"""

import switchapi_thrift

import time
import sys
import os
import logging

import unittest
import random

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../'))
sys.path.append(os.path.join(this_dir, '../../api-tests/'))

import ptf.dataplane as dataplane
import api_base_tests
from api_utils import *

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *

from switchapi_thrift.ttypes import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '..'))
from common.utils import *
from common.api_utils import *

device=0
cpu_port=64
swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)

if swports == []:
    swports = [x for x in range(65)]

###############################################################################
@group('negative')
class L2DuplicateVlanAddTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print

        vlan = self.client.switch_api_vlan_create(device, 10)

        print "\nvlan handle = {0}".format(vlan)

        # TODO: JIRA TSW-68 Duplicate vlan create is allowed, causing handle memory leak
        # Need to pass / fail test case based on function return, or if same hanle is returned
        try:
            print "\nTry to add duplicate vlan 10"
            dup_vlan = self.client.switch_api_vlan_create(device, 10)
            print "\nvlan handle = {0}".format(dup_vlan)
        finally:
            status = self.client.switch_api_vlan_delete(device, vlan)
            print "\nOriginal vlan delete status = {0}".format(status)

            status = self.client.switch_api_vlan_delete(device, dup_vlan)
            print "\nDuplicate vlan delete status = {0}".format(status)


###############################################################################
@group('negative')
class L2InvalidVlanAddTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print


        # TODO: JIRA TSW-69 Vlan > SWITCH_API_MAX_VLANS is allowed in switch_api_vlan_create
        # Need to pass / fail test case based on function return, or if same handle is returned
        # TODO: JIRA TSW-70 switch_api_vlan_create rpc call asserts due to vlan integer type check

        invalid_vlan_list = [9999, 0x45, '!@#$%^&*()', 'VLAN10']
        for invalid_vlan in invalid_vlan_list:
            print "\nTry to add invalid vlan {0}".format(invalid_vlan)
            vlan = self.client.switch_api_vlan_create(device, invalid_vlan)
            print "\nVlan handle: {0}".format(vlan)
            self.client.switch_api_vlan_delete(device, vlan)


###############################################################################
@group('negative')
class L2DuplicateDynamicMacAddTest(api_base_tests.ThriftInterfaceDataPlane):
    def setUp(self):
        print "\nConfiguring different l2 interface type for duplicate dynamic mac add test\n"

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.vlan10 = Vlan(client=self.client, device=device, vlan_id=10)

        self.mac = [
            '', '00:11:11:11:11:11', '00:22:22:22:22:22', '00:33:33:33:33:33',
            '00:44:44:44:44:44'
        ]

        # mac_type: 0 => undefined, 1 => dynamic, 2 => static
        self.mac_type = 1

        ## L2 access
        self.if1 = L2Interface(
            client=self.client, device=device, intf_type=2, port=swports[1])
        ## L2 trunk
        self.if2 = L2Interface(
            client=self.client, device=device, intf_type=3, port=swports[2])
        ## L2 lag access
        self.lag3 = Lag(client=self.client, device=device)
        self.lag3.member_add([swports[3]])
        self.if3 = L2Interface(
            client=self.client,
            device=device,
            intf_type=2,
            port=self.lag3.lag_hdl)

        ## L2 lag trunk
        self.lag4 = Lag(client=self.client, device=device)
        self.lag4.member_add([swports[4]])
        self.if4 = L2Interface(
            client=self.client,
            device=device,
            intf_type=3,
            port=self.lag4.lag_hdl)

        self.vlan10.member_add([self.if1, self.if2, self.if3, self.if4])

        self.mac1 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac=self.mac[1],
            mac_type=self.mac_type,
            handle=self.if1.if_hdl)
        self.mac2 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac=self.mac[2],
            mac_type=self.mac_type,
            handle=self.if2.if_hdl)
        self.mac3 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac=self.mac[3],
            mac_type=self.mac_type,
            handle=self.if3.if_hdl)
        self.mac4 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac=self.mac[4],
            mac_type=self.mac_type,
            handle=self.if4.if_hdl)

    def runTest(self):
        self.fail_flag = False

        print "\nTry to add duplicate mac {0} on L2 access port {1}".format(
            self.mac[1], swports[1])
        status = switch_api_mac_table_entry_create(self,
            device, self.vlan10.vlan_hdl, self.mac[1], self.mac_type,
            self.if1.if_hdl)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Duplicate mac add {0} on port {1} unexpectedly passed".format(
                self.mac[1], swports[1])

        print "\nTry to add duplicate mac {0} on L2 trunk port {1}".format(
            self.mac[2], swports[2])
        status = switch_api_mac_table_entry_create(self,
            device, self.vlan10.vlan_hdl, self.mac[2], self.mac_type,
            self.if2.if_hdl)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Duplicate mac add {0} on port {1} unexpectedly passed".format(
                self.mac[2], swports[2])

        print "\nTry to add duplicate mac {0} on L2 lag access port {1}".format(
            self.mac[3], swports[3])
        status = switch_api_mac_table_entry_create(self,
            device, self.vlan10.vlan_hdl, self.mac[3], self.mac_type,
            self.if3.if_hdl)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Duplicate mac add {0} on port {1} unexpectedly passed".format(
                self.mac[3], swports[3])

        print "\nTry to add duplicate mac {0} on l2 lag trunk port {1}".format(
            self.mac[4], swports[4])
        status = switch_api_mac_table_entry_create(self,
            device, self.vlan10.vlan_hdl, self.mac[4], self.mac_type,
            self.if4.if_hdl)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Duplicate mac add {0} on port {1} unexpectedly passed".format(
                self.mac[4], swports[4])

        self.assertFalse(self.fail_flag, "Duplicate mac unexpectedly passed")

    def tearDown(self):
        self.client.switch_api_mac_table_entries_delete_all(device)

        self.vlan10.vlan_cleanup()

        self.if1.interface_delete()
        self.if2.interface_delete()
        self.if3.interface_delete()
        self.if4.interface_delete()

        self.lag3.member_delete([swports[3]])
        self.lag4.member_delete([swports[4]])

        self.lag3.lag_delete()
        self.lag4.lag_delete()

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('negative')
class L2DuplicateStaticMacAddTest(api_base_tests.ThriftInterfaceDataPlane):
    def setUp(self):
        print "\nConfiguring different l2 interface type for static mac add test\n"

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.vlan10 = Vlan(client=self.client, device=device, vlan_id=10)

        self.mac = [
            '', '00:11:11:11:11:11', '00:22:22:22:22:22', '00:33:33:33:33:33',
            '00:44:44:44:44:44'
        ]

        # mac_type: 0 => undefined, 1 => dynamic, 2 => static
        self.mac_type = 2

        ## L2 access
        self.if1 = L2Interface(
            client=self.client, device=device, intf_type=2, port=swports[1])
        ## L2 trunk
        self.if2 = L2Interface(
            client=self.client, device=device, intf_type=3, port=swports[2])
        ## L2 lag access
        self.lag3 = Lag(client=self.client, device=device)
        self.lag3.member_add([swports[3]])
        self.if3 = L2Interface(
            client=self.client,
            device=device,
            intf_type=2,
            port=self.lag3.lag_hdl)

        ## L2 lag trunk
        self.lag4 = Lag(client=self.client, device=device)
        self.lag4.member_add([swports[4]])
        self.if4 = L2Interface(
            client=self.client,
            device=device,
            intf_type=3,
            port=self.lag4.lag_hdl)

        self.vlan10.member_add([self.if1, self.if2, self.if3, self.if4])

        self.mac1 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac=self.mac[1],
            mac_type=self.mac_type,
            handle=self.if1.if_hdl)
        self.mac2 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac=self.mac[2],
            mac_type=self.mac_type,
            handle=self.if2.if_hdl)
        self.mac3 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac=self.mac[3],
            mac_type=self.mac_type,
            handle=self.if3.if_hdl)
        self.mac4 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac=self.mac[4],
            mac_type=self.mac_type,
            handle=self.if4.if_hdl)

    def runTest(self):
        self.fail_flag = False

        print "\nTry to add duplicate mac {0} on L2 access port {1}".format(
            self.mac[1], swports[1])
        status = switch_api_mac_table_entry_create(self,
            device, self.vlan10.vlan_hdl, self.mac[1], self.mac_type,
            self.if1.if_hdl)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Duplicate mac add {0} on port {1} unexpectedly passed".format(
                self.mac[1], swports[1])

        print "\nTry to add duplicate mac {0} on L2 trunk port {1}".format(
            self.mac[2], swports[2])
        status = switch_api_mac_table_entry_create(self,
            device, self.vlan10.vlan_hdl, self.mac[2], self.mac_type,
            self.if2.if_hdl)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Duplicate mac add {0} on port {1} unexpectedly passed".format(
                self.mac[2], swports[2])

        print "\nTry to add duplicate mac {0} on L2 lag access port {1}".format(
            self.mac[3], swports[3])
        status = switch_api_mac_table_entry_create(self,
            device, self.vlan10.vlan_hdl, self.mac[3], self.mac_type,
            self.if3.if_hdl)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Duplicate mac add {0} on port {1} unexpectedly passed".format(
                self.mac[3], swports[3])

        print "\nTry to add duplicate mac {0} on l2 lag trunk port {1}".format(
            self.mac[4], swports[4])
        status = switch_api_mac_table_entry_create(self,
            device, self.vlan10.vlan_hdl, self.mac[4], self.mac_type,
            self.if4.if_hdl)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Duplicate mac add {0} on port {1} unexpectedly passed".format(
                self.mac[4], swports[4])

        self.assertFalse(self.fail_flag, "Duplicate mac unexpectedly passed")

    def tearDown(self):
        self.client.switch_api_mac_table_entries_delete_all(device)

        self.vlan10.vlan_cleanup()

        self.if1.interface_delete()
        self.if2.interface_delete()
        self.if3.interface_delete()
        self.if4.interface_delete()

        self.lag3.member_delete([swports[3]])
        self.lag4.member_delete([swports[4]])

        self.lag3.lag_delete()
        self.lag4.lag_delete()

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('negative')
class L2InvalidDynamicMacAddTest(api_base_tests.ThriftInterfaceDataPlane):
    ## JIRA TSW-64 - Invalid mac string is accepted ##
    def setUp(self):
        print "\nConfiguring different l2 interface type for invalid dynamic mac add test\n"

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.vlan10 = Vlan(client=self.client, device=device, vlan_id=10)

        self.mac = [
            '', '00:11:11:11:11', '00:gg:12:34:56:78', '!#@$J%^&*',
            '0123456789abcdef'
        ]

        # mac_type: 0 => undefined, 1 => dynamic, 2 => static
        self.mac_type = 1

        ## L2 access
        self.if1 = L2Interface(
            client=self.client, device=device, intf_type=2, port=swports[1])
        ## L2 trunk
        self.if2 = L2Interface(
            client=self.client, device=device, intf_type=3, port=swports[2])
        ## L2 lag access
        self.lag3 = Lag(client=self.client, device=device)
        self.lag3.member_add([swports[3]])
        self.if3 = L2Interface(
            client=self.client,
            device=device,
            intf_type=2,
            port=self.lag3.lag_hdl)

        ## L2 lag trunk
        self.lag4 = Lag(client=self.client, device=device)
        self.lag4.member_add([swports[4]])
        self.if4 = L2Interface(
            client=self.client,
            device=device,
            intf_type=3,
            port=self.lag4.lag_hdl)

        self.vlan10.member_add([self.if1, self.if2, self.if3, self.if4])

    def runTest(self):
        self.fail_flag = False

        print "\nTry to add invalid mac {0} on L2 access port {1}".format(
            self.mac[1], swports[1])
        status = switch_api_mac_table_entry_create(self,
            device, self.vlan10.vlan_hdl, self.mac[1], self.mac_type,
            self.if1.if_hdl)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Invalid mac add {0} on port {1} unexpectedly passed".format(
                self.mac[1], swports[1])

        print "\nTry to add invalid mac {0} on L2 trunk port {1}".format(
            self.mac[2], swports[2])
        status = switch_api_mac_table_entry_create(self,
            device, self.vlan10.vlan_hdl, self.mac[2], self.mac_type,
            self.if2.if_hdl)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Invalid mac add {0} on port {1} unexpectedly passed".format(
                self.mac[2], swports[2])

        print "\nTry to add invalid mac {0} on L2 lag access port {1}".format(
            self.mac[3], swports[3])
        status = switch_api_mac_table_entry_create(self,
            device, self.vlan10.vlan_hdl, self.mac[3], self.mac_type,
            self.if3.if_hdl)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Invalid mac add {0} on port {1} unexpectedly passed".format(
                self.mac[3], swports[3])

        print "\nTry to add invalid mac {0} on l2 lag trunk port {1}".format(
            self.mac[4], swports[4])
        status = switch_api_mac_table_entry_create(self,
            device, self.vlan10.vlan_hdl, self.mac[4], self.mac_type,
            self.if4.if_hdl)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Invalid mac add {0} on port {1} unexpectedly passed".format(
                self.mac[4], swports[4])

        self.assertFalse(self.fail_flag, "Invalid mac unexpectedly passed")

    def tearDown(self):
        self.client.switch_api_mac_table_entries_delete_all(device)

        self.vlan10.vlan_cleanup()

        self.if1.interface_delete()
        self.if2.interface_delete()
        self.if3.interface_delete()
        self.if4.interface_delete()

        self.lag3.member_delete([swports[3]])
        self.lag4.member_delete([swports[4]])

        self.lag3.lag_delete()
        self.lag4.lag_delete()

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('negative')
class L2InvalidStaticMacAddTest(api_base_tests.ThriftInterfaceDataPlane):
    ## JIRA TSW-64 - Invalid mac string is accepted ##
    def setUp(self):
        print "\nConfiguring different l2 interface type for static mac add test\n"

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.vlan10 = Vlan(client=self.client, device=device, vlan_id=10)

        self.mac = [
            '', '00:11:11:11:11', '00:gg:12:34:56:78', '!#@$J%^&*',
            '0123456789abcdef'
        ]

        # mac_type: 0 => undefined, 1 => dynamic, 2 => static
        self.mac_type = 2

        ## L2 access
        self.if1 = L2Interface(
            client=self.client, device=device, intf_type=2, port=swports[1])
        ## L2 trunk
        self.if2 = L2Interface(
            client=self.client, device=device, intf_type=3, port=swports[2])
        ## L2 lag access
        self.lag3 = Lag(client=self.client, device=device)
        self.lag3.member_add([swports[3]])
        self.if3 = L2Interface(
            client=self.client,
            device=device,
            intf_type=2,
            port=self.lag3.lag_hdl)

        ## L2 lag trunk
        self.lag4 = Lag(client=self.client, device=device)
        self.lag4.member_add([swports[4]])
        self.if4 = L2Interface(
            client=self.client,
            device=device,
            intf_type=3,
            port=self.lag4.lag_hdl)

        self.vlan10.member_add([self.if1, self.if2, self.if3, self.if4])

    def runTest(self):
        self.fail_flag = False

        print "\nTry to add invalid mac {0} on L2 access port {1}".format(
            self.mac[1], swports[1])
        status = switch_api_mac_table_entry_create(self,
            device, self.vlan10.vlan_hdl, self.mac[1], self.mac_type,
            self.if1.if_hdl)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Invalid mac add {0} on port {1} unexpectedly passed".format(
                self.mac[1], swports[1])

        print "\nTry to add invalid mac {0} on L2 trunk port {1}".format(
            self.mac[2], swports[2])
        status = switch_api_mac_table_entry_create(self,
            device, self.vlan10.vlan_hdl, self.mac[2], self.mac_type,
            self.if2.if_hdl)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Invalid mac add {0} on port {1} unexpectedly passed".format(
                self.mac[2], swports[2])

        print "\nTry to add invalid mac {0} on L2 lag access port {1}".format(
            self.mac[3], swports[3])
        status = switch_api_mac_table_entry_create(self,
            device, self.vlan10.vlan_hdl, self.mac[3], self.mac_type,
            self.if3.if_hdl)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Invalid mac add {0} on port {1} unexpectedly passed".format(
                self.mac[3], swports[3])

        print "\nTry to add invalid mac {0} on l2 lag trunk port {1}".format(
            self.mac[4], swports[4])
        status = switch_api_mac_table_entry_create(self,
            device, self.vlan10.vlan_hdl, self.mac[4], self.mac_type,
            self.if4.if_hdl)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Invalid mac add {0} on port {1} unexpectedly passed".format(
                self.mac[4], swports[4])

        self.assertFalse(self.fail_flag, "Invalid mac unexpectedly passed")

    def tearDown(self):
        self.client.switch_api_mac_table_entries_delete_all(device)

        self.vlan10.vlan_cleanup()

        self.if1.interface_delete()
        self.if2.interface_delete()
        self.if3.interface_delete()
        self.if4.interface_delete()

        self.lag3.member_delete([swports[3]])
        self.lag4.member_delete([swports[4]])

        self.lag3.lag_delete()
        self.lag4.lag_delete()

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('negative')
class L2DeleteReaddSameDynamicMacTest(api_base_tests.ThriftInterfaceDataPlane):
    def setUp(self):
        print "\nConfiguring different l2 interface type for delete / readd same dynamic mac test"

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.vlan10 = Vlan(client=self.client, device=device, vlan_id=10)
        self.mac = [
            '', '00:11:11:11:11:11', '00:22:22:22:22:22', '00:33:33:33:33:33',
            '00:44:44:44:44:44'
        ]

        # mac_type: 0 => undefined, 1 => dynamic, 2 => static
        self.mac_type = 1

        ## L2 access
        self.if1 = L2Interface(
            client=self.client, device=device, intf_type=2, port=swports[1])
        ## L2 trunk
        self.if2 = L2Interface(
            client=self.client, device=device, intf_type=3, port=swports[2])
        ## L2 lag access
        self.lag3 = Lag(client=self.client, device=device)
        self.lag3.member_add([swports[3]])
        self.if3 = L2Interface(
            client=self.client,
            device=device,
            intf_type=2,
            port=self.lag3.lag_hdl)

        ## L2 lag trunk
        self.lag4 = Lag(client=self.client, device=device)
        self.lag4.member_add([swports[4]])
        self.if4 = L2Interface(
            client=self.client,
            device=device,
            intf_type=3,
            port=self.lag4.lag_hdl)

        self.vlan10.member_add([self.if1, self.if2, self.if3, self.if4])

        self.mac1 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac=self.mac[1],
            mac_type=self.mac_type,
            handle=self.if1.if_hdl)
        self.mac2 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac=self.mac[2],
            mac_type=self.mac_type,
            handle=self.if2.if_hdl)
        self.mac3 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac=self.mac[3],
            mac_type=self.mac_type,
            handle=self.if3.if_hdl)
        self.mac4 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac=self.mac[4],
            mac_type=self.mac_type,
            handle=self.if4.if_hdl)

    def runTest(self):
        print "\nTry to delete and readd same mac {0} on L2 access port {1}".format(
            self.mac[1], swports[1])
        self.mac1.mac_delete()
        self.mac1.mac_add()

        print "\nTry to delete and readd same mac {0} on L2 trunk port {1}".format(
            self.mac[2], swports[2])
        self.mac2.mac_delete()
        self.mac2.mac_add()

        print "\nTry to delete and readd same mac {0} on L2 lag access port {1}".format(
            self.mac[3], swports[3])
        self.mac3.mac_delete()
        self.mac3.mac_add()

        print "\nTry to delete and readd same mac {0} on L2 access port {1}".format(
            self.mac[4], swports[4])
        self.mac4.mac_delete()
        self.mac4.mac_add()

        print "\nSending L2 packet access port {0} -> trunk port {1} [vlan=10]".format(
            swports[1], swports[2])
        pkt = simple_tcp_packet(
            eth_dst=self.mac[2],
            eth_src=self.mac[1],
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.mac[2],
            eth_src=self.mac[1],
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64,
            pktlen=104)
        send_packet(self, swports[1], str(pkt))
        verify_packets(self, exp_pkt, [swports[2]])

        print "Sending L2 packet trunk port {0} -> lag access port {1} [vlan=10]".format(
            swports[2], swports[3])
        pkt = simple_tcp_packet(
            eth_dst=self.mac[3],
            eth_src=self.mac[2],
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64,
            pktlen=104)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.mac[3],
            eth_src=self.mac[2],
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64)
        send_packet(self, swports[2], str(pkt))
        verify_packets(self, exp_pkt, [swports[3]])

        print "Sending L2 packet lag access port {0} -> lag trunk port {1} [vlan=10]".format(
            swports[3], swports[4])
        pkt = simple_tcp_packet(
            eth_dst=self.mac[4],
            eth_src=self.mac[3],
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.mac[4],
            eth_src=self.mac[3],
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64,
            pktlen=104)
        send_packet(self, swports[3], str(pkt))
        verify_packets(self, exp_pkt, [swports[4]])

        print "Sending L2 packet lag trunk port {0} -> access port {1} [vlan=10]".format(
            swports[4], swports[1])
        pkt = simple_tcp_packet(
            eth_dst=self.mac[1],
            eth_src=self.mac[4],
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64,
            pktlen=104)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.mac[1],
            eth_src=self.mac[4],
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64)
        send_packet(self, swports[4], str(pkt))
        verify_packets(self, exp_pkt, [swports[1]])

    def tearDown(self):
        self.client.switch_api_mac_table_entries_delete_all(device)

        self.vlan10.vlan_cleanup()

        self.if1.interface_delete()
        self.if2.interface_delete()
        self.if3.interface_delete()
        self.if4.interface_delete()

        self.lag3.member_delete([swports[3]])
        self.lag4.member_delete([swports[4]])

        self.lag3.lag_delete()
        self.lag4.lag_delete()

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('negative')
class L2DeleteReaddSameStaticMacTest(api_base_tests.ThriftInterfaceDataPlane):
    def setUp(self):
        print "\nConfiguring different l2 interface type for delete / readd same static mac test"

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.vlan10 = Vlan(client=self.client, device=device, vlan_id=10)

        self.mac = [
            '', '00:11:11:11:11:11', '00:22:22:22:22:22', '00:33:33:33:33:33',
            '00:44:44:44:44:44'
        ]

        # mac_type: 0 => undefined, 1 => dynamic, 2 => static
        self.mac_type = 2

        ## L2 access
        self.if1 = L2Interface(
            client=self.client, device=device, intf_type=2, port=swports[1])
        ## L2 trunk
        self.if2 = L2Interface(
            client=self.client, device=device, intf_type=3, port=swports[2])
        ## L2 lag access
        self.lag3 = Lag(client=self.client, device=device)
        self.lag3.member_add([swports[3]])
        self.if3 = L2Interface(
            client=self.client,
            device=device,
            intf_type=2,
            port=self.lag3.lag_hdl)

        ## L2 lag trunk
        self.lag4 = Lag(client=self.client, device=device)
        self.lag4.member_add([swports[4]])
        self.if4 = L2Interface(
            client=self.client,
            device=device,
            intf_type=3,
            port=self.lag4.lag_hdl)

        self.vlan10.member_add([self.if1, self.if2, self.if3, self.if4])

        self.mac1 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac=self.mac[1],
            mac_type=self.mac_type,
            handle=self.if1.if_hdl)
        self.mac2 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac=self.mac[2],
            mac_type=self.mac_type,
            handle=self.if2.if_hdl)
        self.mac3 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac=self.mac[3],
            mac_type=self.mac_type,
            handle=self.if3.if_hdl)
        self.mac4 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac=self.mac[4],
            mac_type=self.mac_type,
            handle=self.if4.if_hdl)

    def runTest(self):
        print "\nTry to delete and readd same mac {0} on L2 access port {1}".format(
            self.mac[1], swports[1])
        self.mac1.mac_delete()
        self.mac1.mac_add()

        print "\nTry to delete and readd same mac {0} on L2 trunk port {1}".format(
            self.mac[2], swports[2])
        self.mac2.mac_delete()
        self.mac2.mac_add()

        print "\nTry to delete and readd same mac {0} on L2 lag access port {1}".format(
            self.mac[3], swports[3])
        self.mac3.mac_delete()
        self.mac3.mac_add()

        print "\nTry to delete and readd same mac {0} on L2 access port {1}".format(
            self.mac[4], swports[4])
        self.mac4.mac_delete()
        self.mac4.mac_add()

        print "\nSending L2 packet access port {0} -> trunk port {1} [vlan=10]".format(
            swports[1], swports[2])
        pkt = simple_tcp_packet(
            eth_dst=self.mac[2],
            eth_src=self.mac[1],
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.mac[2],
            eth_src=self.mac[1],
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64,
            pktlen=104)
        send_packet(self, swports[1], str(pkt))
        verify_packets(self, exp_pkt, [swports[2]])

        print "Sending L2 packet trunk port {0} -> lag access port {1} [vlan=10]".format(
            swports[2], swports[3])
        pkt = simple_tcp_packet(
            eth_dst=self.mac[3],
            eth_src=self.mac[2],
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64,
            pktlen=104)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.mac[3],
            eth_src=self.mac[2],
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64)
        send_packet(self, swports[2], str(pkt))
        verify_packets(self, exp_pkt, [swports[3]])

        print "Sending L2 packet lag access port {0} -> lag trunk port {1} [vlan=10]".format(
            swports[3], swports[4])
        pkt = simple_tcp_packet(
            eth_dst=self.mac[4],
            eth_src=self.mac[3],
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.mac[4],
            eth_src=self.mac[3],
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64,
            pktlen=104)
        send_packet(self, swports[3], str(pkt))
        verify_packets(self, exp_pkt, [swports[4]])

        print "Sending L2 packet lag trunk port {0} -> access port {1} [vlan=10]".format(
            swports[4], swports[1])
        pkt = simple_tcp_packet(
            eth_dst=self.mac[1],
            eth_src=self.mac[4],
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64,
            pktlen=104)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.mac[1],
            eth_src=self.mac[4],
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64)
        send_packet(self, swports[4], str(pkt))
        verify_packets(self, exp_pkt, [swports[1]])

    def tearDown(self):
        self.client.switch_api_mac_table_entries_delete_all(device)

        self.vlan10.vlan_cleanup()

        self.if1.interface_delete()
        self.if2.interface_delete()
        self.if3.interface_delete()
        self.if4.interface_delete()

        self.lag3.member_delete([swports[3]])
        self.lag4.member_delete([swports[4]])

        self.lag3.lag_delete()
        self.lag4.lag_delete()

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('negative')
class L2IncorrectMacTrafficTest(api_base_tests.ThriftInterfaceDataPlane):
    def setUp(self):
        print "\nL2 traffic across different vlan test"

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.vlan10 = Vlan(client=self.client, device=device, vlan_id=10)

        self.mac = [
            '', '00:11:11:11:11:11', '00:22:22:22:22:22', '00:33:33:33:33:33',
            '00:44:44:44:44:44'
        ]

        # mac_type: 0 => undefined, 1 => dynamic, 2 => static
        self.mac_type = 1

        ## L2 access
        self.if1 = L2Interface(
            client=self.client, device=device, intf_type=2, port=swports[1])
        ## L2 trunk
        self.if2 = L2Interface(
            client=self.client, device=device, intf_type=3, port=swports[2])
        ## L2 lag access
        self.lag3 = Lag(client=self.client, device=device)
        self.lag3.member_add([swports[3]])
        self.if3 = L2Interface(
            client=self.client,
            device=device,
            intf_type=2,
            port=self.lag3.lag_hdl)

        ## L2 lag trunk
        self.lag4 = Lag(client=self.client, device=device)
        self.lag4.member_add([swports[4]])
        self.if4 = L2Interface(
            client=self.client,
            device=device,
            intf_type=3,
            port=self.lag4.lag_hdl)

        self.vlan10.member_add([self.if1, self.if2, self.if3, self.if4])

    def runTest(self):
        print "\nSending L2 packet access vlan 10 port {0} -> trunk vlan 10 port {1}".format(
            swports[1], swports[2])
        pkt = simple_tcp_packet(
            eth_dst=self.mac[2],
            eth_src=self.mac[1],
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.mac[2],
            eth_src=self.mac[1],
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64)

        send_packet(self, swports[1], str(pkt))
        print "Verify packet is not flooded back to the src port..."
        verify_no_packet(self, exp_pkt, swports[1])
        print "Verify packet flooded on same vlan as mac entry not created...."
        verify_packet(self, exp_pkt, swports[3])
        ## Trunk ports verification
        exp_pkt = simple_tcp_packet(
            eth_dst=self.mac[2],
            eth_src=self.mac[1],
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64,
            pktlen=104)
        verify_packets(self, exp_pkt, [swports[2], swports[4]])

        print "\nSending L2 packet trunk vlan 10 port {0} -> lag access vlan 10 port {1}".format(
            swports[2], swports[3])
        pkt = simple_tcp_packet(
            eth_dst=self.mac[3],
            eth_src=self.mac[2],
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.mac[3],
            eth_src=self.mac[2],
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64)

        send_packet(self, swports[2], str(pkt))
        print "Verify packet is not flooded back to the src port..."
        verify_no_packet(self, exp_pkt, swports[2])
        print "Verify packet flooded on same vlan as mac entry not created...."
        verify_packet(self, exp_pkt, swports[4])
        ## Access ports verification
        exp_pkt = simple_tcp_packet(
            eth_dst=self.mac[3],
            eth_src=self.mac[2],
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64,
            pktlen=96)
        verify_packets(self, exp_pkt, [swports[1], swports[3]])

        print "\nSending L2 packet lag access vlan 10 port {0} -> lag trunk vlan 10 port {1}".format(
            swports[3], swports[4])
        pkt = simple_tcp_packet(
            eth_dst=self.mac[4],
            eth_src=self.mac[3],
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.mac[4],
            eth_src=self.mac[3],
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64)

        send_packet(self, swports[3], str(pkt))
        print "Verify packet is not flooded back to the src port..."
        verify_no_packet(self, exp_pkt, swports[3])
        print "Verify packet flooded on same vlan as mac entry not created...."
        verify_packet(self, exp_pkt, swports[1])
        ## Trunk ports verification
        exp_pkt = simple_tcp_packet(
            eth_dst=self.mac[4],
            eth_src=self.mac[3],
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64,
            pktlen=104)
        verify_packets(self, exp_pkt, [swports[2], swports[4]])

        ## Remove dynamic mac learned from port 1 in the first traffic stream
        self.client.switch_api_mac_table_entries_delete_all(device)

        print "\nSending L2 packet lag trunk vlan 10 port {0} -> access vlan 10 port".format(
            swports[4], swports[1])
        pkt = simple_tcp_packet(
            eth_dst=self.mac[1],
            eth_src=self.mac[4],
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.mac[1],
            eth_src=self.mac[4],
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64)

        send_packet(self, swports[4], str(pkt))
        print "Verify packet is not flooded back to the src port..."
        verify_no_packet(self, exp_pkt, swports[4])
        print "Verify packet flooded on same vlan as mac entry not created...."
        verify_packet(self, exp_pkt, swports[2])
        ## Access ports verification
        exp_pkt = simple_tcp_packet(
            eth_dst=self.mac[1],
            eth_src=self.mac[4],
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64,
            pktlen=96)
        verify_packets(self, exp_pkt, [swports[1], swports[3]])

    def tearDown(self):
        self.client.switch_api_mac_table_entries_delete_all(device)

        self.vlan10.vlan_cleanup()

        self.if1.interface_delete()
        self.if2.interface_delete()
        self.if3.interface_delete()
        self.if4.interface_delete()

        self.lag3.member_delete([swports[3]])
        self.lag4.member_delete([swports[4]])

        self.lag3.lag_delete()
        self.lag4.lag_delete()

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('negative')
class L2DifferentVlanTrafficTest(api_base_tests.ThriftInterfaceDataPlane):
    def setUp(self):
        print "\nL2 traffic across different vlan test"

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.vlan10 = Vlan(client=self.client, device=device, vlan_id=10)
        self.vlan20 = Vlan(client=self.client, device=device, vlan_id=20)

        self.mac = [
            '', '00:11:11:11:11:11', '00:22:22:22:22:22', '00:33:33:33:33:33',
            '00:44:44:44:44:44'
        ]

        # mac_type: 0 => undefined, 1 => dynamic, 2 => static
        self.mac_type = 1

        ## L2 access vlan 10
        self.if1 = L2Interface(
            client=self.client, device=device, intf_type=2, port=swports[1])
        ## L2 trunk vlan 20
        self.if2 = L2Interface(
            client=self.client, device=device, intf_type=3, port=swports[2])
        ## L2 lag access vlan 20
        self.lag3 = Lag(client=self.client, device=device)
        self.lag3.member_add([swports[3]])
        self.if3 = L2Interface(
            client=self.client,
            device=device,
            intf_type=2,
            port=self.lag3.lag_hdl)

        ## L2 lag trunk vlan 10
        self.lag4 = Lag(client=self.client, device=device)
        self.lag4.member_add([swports[4]])
        self.if4 = L2Interface(
            client=self.client,
            device=device,
            intf_type=3,
            port=self.lag4.lag_hdl)

        self.vlan10.member_add([self.if1, self.if4])
        self.vlan20.member_add([self.if2, self.if3])

        self.mac1 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac=self.mac[1],
            mac_type=self.mac_type,
            handle=self.if1.if_hdl)
        self.mac2 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan20,
            mac=self.mac[2],
            mac_type=self.mac_type,
            handle=self.if2.if_hdl)
        self.mac3 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan20,
            mac=self.mac[3],
            mac_type=self.mac_type,
            handle=self.if3.if_hdl)
        self.mac4 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac=self.mac[4],
            mac_type=self.mac_type,
            handle=self.if4.if_hdl)

    def runTest(self):
        print "\nSending L2 packet access vlan 10 port {0} -> trunk vlan 20 port {1}".format(
            swports[1], swports[2])
        pkt = simple_tcp_packet(
            eth_dst=self.mac[2],
            eth_src=self.mac[1],
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.mac[2],
            eth_src=self.mac[1],
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64,
            pktlen=104)
        send_packet(self, swports[1], str(pkt))
        print "Expecting packet not received on different vlan and flooded on same vlan as mac entry not created...."
        verify_packets(self, exp_pkt, [swports[4]])

        print "\nSending L2 packet trunk vlan 20 port {0} -> lag access vlan 20 port {1}".format(
            swports[2], swports[3])
        pkt = simple_tcp_packet(
            eth_dst=self.mac[3],
            eth_src=self.mac[2],
            dl_vlan_enable=True,
            vlan_vid=20,
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.mac[3],
            eth_src=self.mac[2],
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64,
            pktlen=96)
        send_packet(self, swports[2], str(pkt))
        print "Expecting packet received on same vlan and not flooded on same/different vlan...."
        verify_packets(self, exp_pkt, [swports[3]])

        print "\nSending L2 packet lag access vlan 20 port {0} -> lag trunk vlan 10 port {1}".format(
            swports[3], swports[4])
        pkt = simple_tcp_packet(
            eth_dst=self.mac[4],
            eth_src=self.mac[3],
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.mac[4],
            eth_src=self.mac[3],
            dl_vlan_enable=True,
            vlan_vid=20,
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64,
            pktlen=104)
        send_packet(self, swports[3], str(pkt))
        print "Expecting packet not received on different vlan and flooded on same vlan as mac entry not created...."
        verify_packets(self, exp_pkt, [swports[2]])

        print "\nSending L2 packet lag trunk vlan 10 port {0} -> access vlan 10 port".format(
            swports[4], swports[1])
        pkt = simple_tcp_packet(
            eth_dst=self.mac[1],
            eth_src=self.mac[4],
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.mac[1],
            eth_src=self.mac[4],
            ip_dst='10.0.0.1',
            ip_id=101,
            ip_ttl=64,
            pktlen=96)
        send_packet(self, swports[4], str(pkt))
        print "Expecting packet received on same vlan and not flooded on same/different vlan...."
        verify_packets(self, exp_pkt, [swports[1]])

    def tearDown(self):
        self.client.switch_api_mac_table_entries_delete_all(device)

        self.vlan10.vlan_cleanup()
        self.vlan20.vlan_cleanup()

        self.if1.interface_delete()
        self.if2.interface_delete()
        self.if3.interface_delete()
        self.if4.interface_delete()

        self.lag3.member_delete([swports[3]])
        self.lag4.member_delete([swports[4]])

        self.lag3.lag_delete()
        self.lag4.lag_delete()

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('negative')
class L3IPv4DuplicateAddTest(api_base_tests.ThriftInterfaceDataPlane):
    ## JIRA TSW-67 Duplicate or invalid IPv4 address create is not returning error ##
    def setUp(self):
        print "\nConfiguring different l3 interface type for duplicate ipv4 add test\n"
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.ip = ['', '1.1.1.1', '2.2.2.2', '3.3.3.3', '4.4.4.4']
        self.prefix_length = ['', 16, 16, 16, 16]

        self.vrf = self.client.switch_api_vrf_create(device, swports[1])
        self.rmac = self.client.switch_api_router_mac_group_create(device)
        self.client.switch_api_router_mac_add(device, self.rmac,
                                              '00:77:66:55:44:33')
        self.vlan10 = Vlan(client=self.client, device=device, vlan_id=10)

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif1 = self.client.switch_api_rif_create(0, rif_info)
        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif2 = self.client.switch_api_rif_create(0, rif_info2)
        self.rif3 = self.client.switch_api_rif_create(0, rif_info)
        self.rif4 = self.client.switch_api_rif_create(0, rif_info)

        ## L3 interface
        iu1 = interface_union(port_lag_handle=swports[1])
        i_info1 = switcht_interface_info_t(
            device=0,
            type=SWITCH_INTERFACE_TYPE_PORT,
            u=iu1,
            mac='00:77:66:55:44:33',
            label=0,
            rif_handle=self.rif1)
        self.if1 = self.client.switch_api_interface_create(device, i_info1)
        self.i_ip1 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.ip[1], prefix_length=self.prefix_length[1])
        self.client.switch_api_l3_interface_address_add(device, self.rif1,
                                                        self.vrf, self.i_ip1)

        ## L2 access vlan 10 interface
        self.if0 = L2Interface(
            client=self.client, device=device, intf_type=2, port=swports[2])
        self.vlan10.member_add([self.if0])
        self.mac1 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac='00:22:22:22:22:22',
            mac_type=2,
            handle=self.if0.if_hdl)
        ## L3 VI interface
        self.i_ip2 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.ip[2], prefix_length=self.prefix_length[2])
        self.client.switch_api_l3_interface_address_add(device, self.rif2,
                                                        self.vrf, self.i_ip2)

        ## L3 sub-interface
        pv3 = switcht_port_vlan_t(port_lag_handle=swports[3], vlan_id=10)
        iu3 = interface_union(port_vlan=pv3)
        i_info3 = switcht_interface_info_t(
            device=0,
            type=6,
            u=iu3,
            mac='00:77:66:55:44:33',
            label=0,
            rif_handle=self.rif3)
        self.if3 = self.client.switch_api_interface_create(device, i_info3)
        self.i_ip3 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.ip[3], prefix_length=self.prefix_length[3])
        self.client.switch_api_l3_interface_address_add(device, self.rif3,
                                                        self.vrf, self.i_ip3)

        ## L3 lag interface
        self.lag = Lag(client=self.client, device=device)
        self.lag.member_add([swports[4]])
        iu4 = interface_union(port_lag_handle=self.lag.lag_hdl)
        i_info4 = switcht_interface_info_t(
            device=0,
            type=SWITCH_INTERFACE_TYPE_PORT,
            u=iu4,
            mac='00:77:66:55:44:33',
            label=0,
            rif_handle=self.rif4)
        self.if4 = self.client.switch_api_interface_create(device, i_info4)
        self.i_ip4 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.ip[4], prefix_length=self.prefix_length[4])
        self.client.switch_api_l3_interface_address_add(device, self.rif4,
                                                        self.vrf, self.i_ip4)

    def runTest(self):
        self.fail_flag = False

        print "\nTry to add duplicate IPv4 {0}/{1} on L3 port {2}".format(
            self.ip[1], self.prefix_length[1], swports[1])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif1, self.vrf, self.i_ip1)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Duplicate IPv4 add {0}/{1} on L3 port {2} unexpectedly passed".format(
                self.ip[1], self.prefix_length[1], swports[1])

        print "\nTry to add duplicate IPv4 {0}/{1} on L3 VI port {2}".format(
            self.ip[2], self.prefix_length[2], swports[2])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif2, self.vrf, self.i_ip2)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Duplicate IPv4 add {0}/{1} on L3 VI port {2} unexpectedly passed".format(
                self.ip[2], self.prefix_length[2], swports[2])

        print "\nTry to add duplicate IPv4 {0}/{1} on L3 sub-interface port {2}".format(
            self.ip[3], self.prefix_length[3], swports[3])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif3, self.vrf, self.i_ip3)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Duplicate IPv4 add {0}/{1} on L3 sub-interface port {2} unexpectedly passed".format(
                self.ip[3], self.prefix_length[3], swports[3])

        print "\nTry to add duplicate IPv4 {0}/{1} on L3 lag port {2}".format(
            self.ip[4], self.prefix_length[4], swports[4])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif4, self.vrf, self.i_ip4)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Duplicate IPv4 add {0}/{1} on L3 port {2} unexpectedly passed".format(
                self.ip[4], self.prefix_length[4], swports[4])

        self.assertFalse(self.fail_flag,
                         "Duplicate IPv4 add unexpectedly passed")

    def tearDown(self):
        self.client.switch_api_mac_table_entries_delete_all(device)

        self.vlan10.vlan_cleanup()

        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip1)
        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip2)
        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip3)
        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip4)

        self.if0.interface_delete()

        self.client.switch_api_interface_delete(device, self.if1)
        self.client.switch_api_interface_delete(device, self.if3)
        self.client.switch_api_interface_delete(device, self.if4)

        self.client.switch_api_rif_delete(0, self.rif1)
        self.client.switch_api_rif_delete(0, self.rif2)
        self.client.switch_api_rif_delete(0, self.rif3)
        self.client.switch_api_rif_delete(0, self.rif4)

        self.lag.member_delete([swports[4]])
        self.lag.lag_delete()

        self.client.switch_api_router_mac_delete(device, self.rmac,
                                                 '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(device, self.rmac)
        self.client.switch_api_vrf_delete(device, self.vrf)

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('negative')
class L3IPv4InvalidAddTest(api_base_tests.ThriftInterfaceDataPlane):
    ## JIRA TSW-67 Duplicate or invalid IPv4 address create is not returning error ##
    def setUp(self):
        print "\nConfiguring different l3 interface type for invalid ipv4 add test\n"
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.ip = [
            '', '192.168.1', '1234.1234.1234.1234', 'abcedfg0123456789',
            '224.0.0.1'
        ]
        self.prefix_length = ['', 16, 16, 16, 16]

        self.vrf = self.client.switch_api_vrf_create(device, swports[1])
        self.rmac = self.client.switch_api_router_mac_group_create(device)
        self.client.switch_api_router_mac_add(device, self.rmac,
                                              '00:77:66:55:44:33')
        self.vlan10 = Vlan(client=self.client, device=device, vlan_id=10)

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif1 = self.client.switch_api_rif_create(0, rif_info)
        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif2 = self.client.switch_api_rif_create(0, rif_info2)
        self.rif3 = self.client.switch_api_rif_create(0, rif_info)
        self.rif4 = self.client.switch_api_rif_create(0, rif_info)

        ## L3 interface
        iu1 = interface_union(port_lag_handle=swports[1])
        i_info1 = switcht_interface_info_t(
            device=0,
            type=SWITCH_INTERFACE_TYPE_PORT,
            u=iu1,
            mac='00:77:66:55:44:33',
            label=0,
            rif_handle=self.rif1)
        self.if1 = self.client.switch_api_interface_create(device, i_info1)

        self.i_ip1 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.ip[1], prefix_length=self.prefix_length[1])
        self.client.switch_api_l3_interface_address_add(device, self.rif1,
                                                        self.vrf, self.i_ip1)

        ## L2 access vlan 10 interface
        self.if0 = L2Interface(
            client=self.client, device=device, intf_type=2, port=swports[2])
        self.vlan10.member_add([self.if0])
        self.mac1 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac='00:22:22:22:22:22',
            mac_type=2,
            handle=self.if0.if_hdl)
        ## L3 VI interface
        self.i_ip2 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.ip[2], prefix_length=self.prefix_length[2])
        self.client.switch_api_l3_interface_address_add(device, self.rif2,
                                                        self.vrf, self.i_ip2)

        ## L3 sub-interface
        pv3 = switcht_port_vlan_t(port_lag_handle=swports[3], vlan_id=10)
        iu3 = interface_union(port_vlan=pv3)
        i_info3 = switcht_interface_info_t(
            device=0,
            type=6,
            u=iu3,
            mac='00:77:66:55:44:33',
            label=0,
            rif_handle=self.rif3)
        self.if3 = self.client.switch_api_interface_create(device, i_info3)
        self.i_ip3 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.ip[3], prefix_length=self.prefix_length[3])
        self.client.switch_api_l3_interface_address_add(device, self.rif3,
                                                        self.vrf, self.i_ip3)

        ## L3 lag interface
        self.lag = Lag(client=self.client, device=device)
        self.lag.member_add([swports[4]])
        iu4 = interface_union(port_lag_handle=self.lag.lag_hdl)
        i_info4 = switcht_interface_info_t(
            device=0,
            type=SWITCH_INTERFACE_TYPE_PORT,
            u=iu4,
            mac='00:77:66:55:44:33',
            label=0,
            rif_handle=self.rif4)
        self.if4 = self.client.switch_api_interface_create(device, i_info4)
        self.i_ip4 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.ip[4], prefix_length=self.prefix_length[4])
        self.client.switch_api_l3_interface_address_add(device, self.rif4,
                                                        self.vrf, self.i_ip4)

    def runTest(self):
        self.fail_flag = False

        print "\nTry to add invalid IPv4 {0}/{1} on L3 port {2}".format(
            self.ip[1], self.prefix_length[1], swports[1])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif1, self.vrf, self.i_ip1)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Invalid IPv4 add {0}/{1} on L3 port {2} unexpectedly passed".format(
                self.ip[1], self.prefix_length[1], swports[1])

        print "\nTry to add invalid IPv4 {0}/{1} on L3 VI port {2}".format(
            self.ip[2], self.prefix_length[2], swports[2])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif2, self.vrf, self.i_ip2)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Invalid IPv4 add {0}/{1} on L3 VI port {2} unexpectedly passed".format(
                self.ip[2], self.prefix_length[2], swports[2])

        print "\nTry to add invalid IPv4 {0}/{1} on L3 sub-interface port {2}".format(
            self.ip[3], self.prefix_length[3], swports[3])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif3, self.vrf, self.i_ip3)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Invalid IPv4 add {0}/{1} on L3 sub-interface port {2} unexpectedly passed".format(
                self.ip[3], self.prefix_length[3], swports[3])

        print "\nTry to add invalid IPv4 {0}/{1} on L3 lag port {2}".format(
            self.ip[4], self.prefix_length[4], swports[4])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif4, self.vrf, self.i_ip4)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Invalid IPv4 add {0}/{1} on L3 port {2} unexpectedly passed".format(
                self.ip[4], self.prefix_length[4], swports[4])

        self.assertFalse(self.fail_flag, "Invalid IPv4 add unexpectedly passed")

    def tearDown(self):
        self.client.switch_api_mac_table_entries_delete_all(device)

        self.vlan10.vlan_cleanup()

        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip1)
        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip2)
        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip3)
        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip4)

        self.if0.interface_delete()

        self.client.switch_api_interface_delete(device, self.if1)
        self.client.switch_api_interface_delete(device, self.if3)
        self.client.switch_api_interface_delete(device, self.if4)

        self.client.switch_api_rif_delete(0, self.rif1)
        self.client.switch_api_rif_delete(0, self.rif2)
        self.client.switch_api_rif_delete(0, self.rif3)
        self.client.switch_api_rif_delete(0, self.rif4)

        self.lag.member_delete([swports[4]])
        self.lag.lag_delete()

        self.client.switch_api_router_mac_delete(device, self.rmac,
                                                 '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(device, self.rmac)
        self.client.switch_api_vrf_delete(device, self.vrf)

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('negative')
class L3IPv4DeleteReaddSameIPTest(api_base_tests.ThriftInterfaceDataPlane):
    def setUp(self):
        print "\nConfiguring different l3 interface type for ipv4 delete readd same ip test\n"
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.ip = ['', '1.1.1.1', '2.2.2.2', '3.3.3.3']
        self.ip_src = ['', '1.1.1.100', '2.2.2.100', '3.3.3.100']
        self.nh_ip = ['', '11.11.11.11', '22.22.22.22', '33.33.33.33']
        self.nh_mac = [
            '', '00:11:11:11:11:11', '00:22:22:22:22:22', '00:33:33:33:33:33'
        ]
        self.prefix_length = ['', 16, 16, 16]
        self.rmac_address = '00:77:66:55:44:33'

        self.vrf = self.client.switch_api_vrf_create(device, swports[1])
        self.rmac = self.client.switch_api_router_mac_group_create(device)
        self.client.switch_api_router_mac_add(device, self.rmac,
                                              self.rmac_address)
        self.vlan10 = Vlan(client=self.client, device=device, vlan_id=10)

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif1 = self.client.switch_api_rif_create(0, rif_info)
        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif2 = self.client.switch_api_rif_create(0, rif_info2)
        self.rif3 = self.client.switch_api_rif_create(0, rif_info)
        ## L3 interface
        iu1 = interface_union(port_lag_handle=swports[1])
        i_info1 = switcht_interface_info_t(
            device=0,
            type=SWITCH_INTERFACE_TYPE_PORT,
            u=iu1,
            mac=self.rmac_address,
            label=0,
            rif_handle=self.rif1)
        self.if1 = self.client.switch_api_interface_create(device, i_info1)
        self.i_ip1 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.ip[1], prefix_length=self.prefix_length[1])
        self.client.switch_api_l3_interface_address_add(device, self.rif1,
                                                        self.vrf, self.i_ip1)

        ## L2 access vlan 10 interface
        self.if0 = L2Interface(
            client=self.client, device=device, intf_type=2, port=swports[2])
        self.vlan10.member_add([self.if0])
        self.mac1 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac='00:22:22:22:22:22',
            mac_type=2,
            handle=self.if0.if_hdl)
        ## L3 VI interface
        self.i_ip2 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.ip[2], prefix_length=self.prefix_length[2])
        self.client.switch_api_l3_interface_address_add(device, self.rif2,
                                                        self.vrf, self.i_ip2)

        ## L3 lag interface
        self.lag = Lag(client=self.client, device=device)
        self.lag.member_add([swports[3]])
        iu3 = interface_union(port_lag_handle=self.lag.lag_hdl)
        i_info3 = switcht_interface_info_t(
            device=0,
            type=SWITCH_INTERFACE_TYPE_PORT,
            u=iu3,
            mac=self.rmac_address,
            label=0,
            rif_handle=self.rif3)
        self.if3 = self.client.switch_api_interface_create(device, i_info3)
        self.i_ip3 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.ip[3], prefix_length=self.prefix_length[3])
        self.client.switch_api_l3_interface_address_add(device, self.rif3,
                                                        self.vrf, self.i_ip3)

    def runTest(self):
        self.fail_flag = False

        print "\nTry to delete / readd same IPv4 {0}/{1} on L3 port {2}".format(
            self.ip[1], self.prefix_length[1], swports[1])
        status = self.client.switch_api_l3_interface_address_delete(
            device, self.rif1, self.vrf, self.i_ip1)
        print "Delete ip status: {0}".format(status)
        if (status != 0):
            self.fail_flag = True
            print "Delete ip {0}/{1} on L3 port {2} failed".format(
                self.ip[1], self.prefix_length[1], swports[1])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif1, self.vrf, self.i_ip1)
        print "Readd ip status: {0}".format(status)
        if (status != 0):
            self.fail_flag = True
            print "Readd same ip {0}/{1} on L3 port {2} failed".format(
                self.ip[1], self.prefix_length[1], swports[1])

        print "\nTry to delete / readd same IPv4 {0}/{1} on L3 VI port {2}".format(
            self.ip[2], self.prefix_length[2], swports[2])
        status = self.client.switch_api_l3_interface_address_delete(
            device, self.rif2, self.vrf, self.i_ip2)
        print "Delete ip tatus: {0}".format(status)
        if (status != 0):
            self.fail_flag = True
            print "Delete ip {0}/{1} on L3 VI port {2} failed".format(
                self.ip[2], self.prefix_length[2], swports[2])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif2, self.vrf, self.i_ip2)
        print "Readd ip status: {0}".format(status)
        if (status != 0):
            self.fail_flag = True
            print "Readd same ip {0}/{1} on L3 VI port {2} failed".format(
                self.ip[2], self.prefix_length[2], swports[2])

        print "\nTry to delete / readd IPv4 {0}/{1} on L3 lag port {2}".format(
            self.ip[3], self.prefix_length[3], swports[3])
        status = self.client.switch_api_l3_interface_address_delete(
            device, self.rif3, self.vrf, self.i_ip3)
        print "Delete ip tatus: {0}".format(status)
        if (status != 0):
            self.fail_flag = True
            print "Delete ip {0}/{1} on L3 lag port {2} failed".format(
                self.ip[3], self.prefix_length[3], swports[3])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif3, self.vrf, self.i_ip3)
        print "Readd ip status: {0}".format(status)
        if (status != 0):
            self.fail_flag = True
            print "Readd same ip {0}/{1} on L3 lag port {2} failed".format(
                self.ip[3], self.prefix_length[3], swports[3])

        self.assertFalse(self.fail_flag, "Delete / readd same IPv4 failed")

        # Add static routes
        self.nh_ip1 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.nh_ip[1], prefix_length=32)
        self.nhop1, self.neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, self.rif1, self.nh_ip1, self.nh_mac[1])
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip1,
                                            self.nhop1)

        self.nh_ip2 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.nh_ip[2], prefix_length=32)
        self.nhop2, self.neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, self.rif2, self.nh_ip2, self.nh_mac[2])
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip2,
                                            self.nhop2)

        self.nh_ip3 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.nh_ip[3], prefix_length=32)
        self.nhop3, self.neighbor3 = switch_api_l3_nhop_neighbor_create(self, device, self.rif3, self.nh_ip3, self.nh_mac[3])
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip3,
                                            self.nhop3)

        print "\nSending L3 packet L3 port {0} -> L3 VI [vlan=10] port {1}".format(
            swports[1], swports[2])
        pkt = simple_tcp_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[1],
            ip_dst=self.nh_ip[2],
            ip_src=self.ip_src[1],
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.nh_mac[2],
            eth_src=self.rmac_address,
            ip_dst=self.nh_ip[2],
            ip_src=self.ip_src[1],
            ip_id=101,
            ip_ttl=63)
        send_packet(self, swports[1], str(pkt))
        verify_packets(self, exp_pkt, [swports[2]])

        print "Sending L3 packet L3 VI port {0} -> L3 lag port {1}".format(
            swports[2], swports[3])
        pkt = simple_tcp_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[2],
            ip_dst=self.nh_ip[3],
            ip_src=self.ip_src[2],
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.nh_mac[3],
            eth_src=self.rmac_address,
            ip_dst=self.nh_ip[3],
            ip_src=self.ip_src[2],
            ip_id=101,
            ip_ttl=63)
        send_packet(self, swports[2], str(pkt))
        verify_packets(self, exp_pkt, [swports[3]])

        print "Sending L3 packet L3 lag port {0} -> L3 port {1}".format(
            swports[3], swports[1])
        pkt = simple_tcp_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[3],
            ip_dst=self.nh_ip[1],
            ip_src=self.ip_src[3],
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.nh_mac[1],
            eth_src=self.rmac_address,
            ip_dst=self.nh_ip[1],
            ip_src=self.ip_src[3],
            ip_id=101,
            ip_ttl=63)
        send_packet(self, swports[3], str(pkt))
        verify_packets(self, exp_pkt, [swports[1]])

    def tearDown(self):
        self.client.switch_api_mac_table_entries_delete_all(device)

        self.vlan10.vlan_cleanup()

        self.client.switch_api_neighbor_delete(device, self.neighbor1)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip1,
                                               self.nhop1)
        self.client.switch_api_nhop_delete(device, self.nhop1)

        self.client.switch_api_neighbor_delete(device, self.neighbor2)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip2,
                                               self.nhop2)
        self.client.switch_api_nhop_delete(device, self.nhop2)

        self.client.switch_api_neighbor_delete(device, self.neighbor3)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip3,
                                               self.nhop3)
        self.client.switch_api_nhop_delete(device, self.nhop3)

        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip1)
        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip2)
        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip3)

        self.if0.interface_delete()

        self.client.switch_api_interface_delete(device, self.if1)
        self.client.switch_api_interface_delete(device, self.if3)

        self.client.switch_api_rif_delete(0, self.rif1)
        self.client.switch_api_rif_delete(0, self.rif2)
        self.client.switch_api_rif_delete(0, self.rif3)

        self.lag.member_delete([swports[3]])
        self.lag.lag_delete()

        self.client.switch_api_router_mac_delete(device, self.rmac,
                                                 self.rmac_address)
        self.client.switch_api_router_mac_group_delete(device, self.rmac)
        self.client.switch_api_vrf_delete(device, self.vrf)

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('negative')
class L3IPv4AddDeleteUpdateTest(api_base_tests.ThriftInterfaceDataPlane):
    def setUp(self):
        print "\nConfiguring different l3 interface type for ipv4 add / delete / update different ip test\n"
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.old_ip = ['', '121.1.1.1', '121.2.2.2', '121.3.3.3']
        self.ip = ['', '1.1.1.1', '2.2.2.2', '3.3.3.3']
        self.ip_src = ['', '1.1.1.100', '2.2.2.100', '3.3.3.100']
        self.nh_ip = ['', '11.11.11.11', '22.22.22.22', '33.33.33.33']
        self.nh_mac = [
            '', '00:11:11:11:11:11', '00:22:22:22:22:22', '00:33:33:33:33:33'
        ]
        self.prefix_length = ['', 16, 16, 16]
        self.rmac_address = '00:77:66:55:44:33'

        self.vrf = self.client.switch_api_vrf_create(device, swports[1])
        self.rmac = self.client.switch_api_router_mac_group_create(device)
        self.client.switch_api_router_mac_add(device, self.rmac,
                                              self.rmac_address)
        self.vlan10 = Vlan(client=self.client, device=device, vlan_id=10)

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif1 = self.client.switch_api_rif_create(0, rif_info)
        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif2 = self.client.switch_api_rif_create(0, rif_info2)
        self.rif3 = self.client.switch_api_rif_create(0, rif_info)

        ## L3 interface
        iu1 = interface_union(port_lag_handle=swports[1])
        i_info1 = switcht_interface_info_t(
            device=0,
            type=SWITCH_INTERFACE_TYPE_PORT,
            u=iu1,
            mac=self.rmac_address,
            label=0,
            rif_handle=self.rif1)
        self.if1 = self.client.switch_api_interface_create(device, i_info1)
        self.i_ip1 = switcht_ip_addr_t(
            addr_type=0,
            ipaddr=self.old_ip[1],
            prefix_length=self.prefix_length[1])
        self.client.switch_api_l3_interface_address_add(device, self.rif1,
                                                        self.vrf, self.i_ip1)

        ## L2 access vlan 10 interface
        self.if0 = L2Interface(
            client=self.client, device=device, intf_type=2, port=swports[2])
        self.vlan10.member_add([self.if0])
        self.mac1 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac='00:22:22:22:22:22',
            mac_type=2,
            handle=self.if0.if_hdl)
        ## L3 VI interface
        self.i_ip2 = switcht_ip_addr_t(
            addr_type=0,
            ipaddr=self.old_ip[2],
            prefix_length=self.prefix_length[2])
        self.client.switch_api_l3_interface_address_add(device, self.rif2,
                                                        self.vrf, self.i_ip2)

        ## L3 lag interface
        self.lag = Lag(client=self.client, device=device)
        self.lag.member_add([swports[3]])
        iu3 = interface_union(port_lag_handle=self.lag.lag_hdl)
        i_info3 = switcht_interface_info_t(
            device=0,
            type=SWITCH_INTERFACE_TYPE_PORT,
            u=iu3,
            mac=self.rmac_address,
            label=0,
            rif_handle=self.rif3)
        self.if3 = self.client.switch_api_interface_create(device, i_info3)
        self.i_ip3 = switcht_ip_addr_t(
            addr_type=0,
            ipaddr=self.old_ip[3],
            prefix_length=self.prefix_length[3])
        self.client.switch_api_l3_interface_address_add(device, self.rif3,
                                                        self.vrf, self.i_ip3)

    def runTest(self):
        self.fail_flag = False

        print "\nTry to delete {0}/{1} update different IPv4 {2}/{1} on L3 port {3}".format(
            self.old_ip[1], self.prefix_length[1], self.ip[1], swports[1])
        status = self.client.switch_api_l3_interface_address_delete(
            device, self.rif1, self.vrf, self.i_ip1)
        print "Delete ip status: {0}".format(status)
        if (status != 0):
            self.fail_flag = True
            print "Delete ip {0}/{1} on L3 port {2} failed".format(
                self.old_ip[1], self.prefix_length[1], swports[1])
        self.i_ip1 = switcht_ip_addr_t(
            addr_type=0,
            ipaddr=self.old_ip[1],
            prefix_length=self.prefix_length[1])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif1, self.vrf, self.i_ip1)
        print "Add ip status: {0}".format(status)
        if (status != 0):
            self.fail_flag = True
            print "Update different ip {0}/{1} on L3 port {2} failed".format(
                self.ip[1], self.prefix_length[1], swports[1])

        print "\nTry to delete {0}/{1} update different IPv4 {2}/{1} on L3 VI port {3}".format(
            self.old_ip[2], self.prefix_length[2], self.ip[2], swports[2])
        status = self.client.switch_api_l3_interface_address_delete(
            device, self.rif2, self.vrf, self.i_ip2)
        print "Delete ip tatus: {0}".format(status)
        if (status != 0):
            self.fail_flag = True
            print "Delete ip {0}/{1} on L3 VI port {2} failed".format(
                self.old_ip[2], self.prefix_length[2], swports[2])
        self.i_ip2 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.ip[2], prefix_length=self.prefix_length[2])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif2, self.vrf, self.i_ip2)
        print "Add ip status: {0}".format(status)
        if (status != 0):
            self.fail_flag = True
            print "Update different ip {0}/{1} on L3 VI port {2} failed".format(
                self.ip[2], self.prefix_length[2], swports[2])

        print "\nTry to delete {0}/{1} update different IPv4 {2}/{1} on L3 lag port {3}".format(
            self.old_ip[3], self.prefix_length[3], self.ip[3], swports[3])
        status = self.client.switch_api_l3_interface_address_delete(
            device, self.rif3, self.vrf, self.i_ip3)
        print "Delete ip tatus: {0}".format(status)
        if (status != 0):
            self.fail_flag = True
            print "Delete ip {0}/{1} on L3 lag port {2} failed".format(
                self.old_ip[3], self.prefix_length[3], swports[3])
        self.i_ip3 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.ip[3], prefix_length=self.prefix_length[3])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif3, self.vrf, self.i_ip3)
        print "Add ip status: {0}".format(status)
        if (status != 0):
            self.fail_flag = True
            print "Update different ip {0}/{1} on L3 lag port {2} failed".format(
                self.ip[3], self.prefix_length[3], swports[3])

        self.assertFalse(self.fail_flag,
                         "Delete / update different IPv4 failed")

        # Add static routes
        self.nh_ip1 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.nh_ip[1], prefix_length=32)
        self.nhop1, self.neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, self.rif1, self.nh_ip1, self.nh_mac[1])
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip1,
                                            self.nhop1)

        self.nh_ip2 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.nh_ip[2], prefix_length=32)
        self.nhop2, self.neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, self.rif2, self.nh_ip2, self.nh_mac[2])
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip2,
                                            self.nhop2)

        self.nh_ip3 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.nh_ip[3], prefix_length=32)
        self.nhop3, self.neighbor3 = switch_api_l3_nhop_neighbor_create(self, device, self.rif3, self.nh_ip3, self.nh_mac[3])
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip3,
                                            self.nhop3)

        print "\nSending L3 packet L3 port {0} -> L3 VI [vlan=10] port {1}".format(
            swports[1], swports[2])
        pkt = simple_tcp_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[1],
            ip_dst=self.nh_ip[2],
            ip_src=self.ip_src[1],
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.nh_mac[2],
            eth_src=self.rmac_address,
            ip_dst=self.nh_ip[2],
            ip_src=self.ip_src[1],
            ip_id=101,
            ip_ttl=63)
        send_packet(self, swports[1], str(pkt))
        verify_packets(self, exp_pkt, [swports[2]])

        print "Sending L3 packet L3 VI port {0} -> L3 lag port {1}".format(
            swports[2], swports[3])
        pkt = simple_tcp_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[2],
            ip_dst=self.nh_ip[3],
            ip_src=self.ip_src[2],
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.nh_mac[3],
            eth_src=self.rmac_address,
            ip_dst=self.nh_ip[3],
            ip_src=self.ip_src[2],
            ip_id=101,
            ip_ttl=63)
        send_packet(self, swports[2], str(pkt))
        verify_packets(self, exp_pkt, [swports[3]])

        print "Sending L3 packet L3 lag port {0} -> L3 port {1}".format(
            swports[3], swports[1])
        pkt = simple_tcp_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[3],
            ip_dst=self.nh_ip[1],
            ip_src=self.ip_src[3],
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.nh_mac[1],
            eth_src=self.rmac_address,
            ip_dst=self.nh_ip[1],
            ip_src=self.ip_src[3],
            ip_id=101,
            ip_ttl=63)
        send_packet(self, swports[3], str(pkt))
        verify_packets(self, exp_pkt, [swports[1]])

    def tearDown(self):
        self.client.switch_api_mac_table_entries_delete_all(device)

        self.vlan10.vlan_cleanup()

        self.client.switch_api_neighbor_delete(device, self.neighbor1)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip1,
                                               self.nhop1)
        self.client.switch_api_nhop_delete(device, self.nhop1)

        self.client.switch_api_neighbor_delete(device, self.neighbor2)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip2,
                                               self.nhop2)
        self.client.switch_api_nhop_delete(device, self.nhop2)

        self.client.switch_api_neighbor_delete(device, self.neighbor3)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip3,
                                               self.nhop3)
        self.client.switch_api_nhop_delete(device, self.nhop3)

        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip1)
        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip2)
        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip3)

        self.if0.interface_delete()

        self.client.switch_api_interface_delete(device, self.if1)
        self.client.switch_api_interface_delete(device, self.if3)

        self.client.switch_api_rif_delete(0, self.rif1)
        self.client.switch_api_rif_delete(0, self.rif2)
        self.client.switch_api_rif_delete(0, self.rif3)

        self.lag.member_delete([swports[3]])
        self.lag.lag_delete()

        self.client.switch_api_router_mac_delete(device, self.rmac,
                                                 self.rmac_address)
        self.client.switch_api_router_mac_group_delete(device, self.rmac)
        self.client.switch_api_vrf_delete(device, self.vrf)

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('negative')
class L3IPv4MultipleVITest(api_base_tests.ThriftInterfaceDataPlane):
    def setUp(self):
        print "\nConfiguring multiple l3 VI interfaces for ipv4 add / delete / update different ip test\n"
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.ip = ['', '1.1.1.1', '2.2.2.2', '3.3.3.3']
        self.src_ip_vlan10 = ['', '1.1.1.101', '1.1.1.102']
        self.src_ip_vlan20 = ['', '2.2.2.101', '2.2.2.102']
        self.src_ip_vlan30 = ['', '3.3.3.101', '3.3.3.102']
        self.nh_ip_vlan10 = ['', '11.11.11.101', '11.11.11.102']
        self.nh_ip_vlan20 = ['', '22.22.22.101', '22.22.22.102']
        self.nh_ip_vlan30 = ['', '33.33.33.101', '33.33.33.102']
        self.nh_ip_vlan10_new = ['', '111.11.11.101', '111.11.11.102']
        self.nh_ip_vlan20_new = ['', '122.22.22.101', '122.22.22.102']
        self.nh_ip_vlan30_new = ['', '133.33.33.101', '133.33.33.102']
        self.nh_mac_vlan10 = ['', '00:10:11:11:11:11', '00:10:22:22:22:22']
        self.nh_mac_vlan20 = ['', '00:20:11:11:11:11', '00:20:22:22:22:22']
        self.nh_mac_vlan30 = ['', '00:30:11:11:11:11', '00:30:22:22:22:22']
        self.prefix_length = ['', 16, 16, 16]
        self.rmac_address = '00:77:66:55:44:33'

        self.vrf = self.client.switch_api_vrf_create(device, swports[1])
        self.rmac = self.client.switch_api_router_mac_group_create(device)
        self.client.switch_api_router_mac_add(device, self.rmac,
                                              self.rmac_address)
        self.vlan10 = Vlan(client=self.client, device=device, vlan_id=10)
        self.vlan20 = Vlan(client=self.client, device=device, vlan_id=20)
        self.vlan30 = Vlan(client=self.client, device=device, vlan_id=30)

        ## L2 access and trunk interface for vlan 10
        self.if11 = L2Interface(
            client=self.client, device=device, intf_type=2, port=swports[1])
        self.mac11 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac=self.nh_mac_vlan10[1],
            mac_type=2,
            handle=self.if11.if_hdl)
        self.if12 = L2Interface(
            client=self.client, device=device, intf_type=3, port=swports[2])
        self.mac12 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac=self.nh_mac_vlan10[2],
            mac_type=2,
            handle=self.if12.if_hdl)
        self.vlan10.member_add([self.if11, self.if12])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif1 = self.client.switch_api_rif_create(0, rif_info1)
        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=20,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif2 = self.client.switch_api_rif_create(0, rif_info2)
        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=30,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif3 = self.client.switch_api_rif_create(0, rif_info3)
        rif_info4 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif4 = self.client.switch_api_rif_create(0, rif_info4)

        ## L3 VI interface (vlan10)
        self.i_ip1 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.ip[1], prefix_length=self.prefix_length[1])
        self.client.switch_api_l3_interface_address_add(device, self.rif1,
                                                        self.vrf, self.i_ip1)

        ## L2 access and trunk interface for vlan 20
        self.if21 = L2Interface(
            client=self.client, device=device, intf_type=2, port=swports[3])
        self.mac21 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan20,
            mac=self.nh_mac_vlan20[1],
            mac_type=2,
            handle=self.if21.if_hdl)
        self.if22 = L2Interface(
            client=self.client, device=device, intf_type=3, port=swports[4])
        self.mac22 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan20,
            mac=self.nh_mac_vlan20[2],
            mac_type=2,
            handle=self.if22.if_hdl)
        self.vlan20.member_add([self.if21, self.if22])

        ## L3 VI interface (vlan20)
        self.i_ip2 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.ip[2], prefix_length=self.prefix_length[2])
        self.client.switch_api_l3_interface_address_add(device, self.rif2,
                                                        self.vrf, self.i_ip2)

        ## L2 access and trunk interface for vlan 30
        self.if31 = L2Interface(
            client=self.client, device=device, intf_type=2, port=swports[5])
        self.mac31 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan30,
            mac=self.nh_mac_vlan30[1],
            mac_type=2,
            handle=self.if31.if_hdl)
        self.if32 = L2Interface(
            client=self.client, device=device, intf_type=3, port=swports[6])
        self.mac32 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan30,
            mac=self.nh_mac_vlan30[2],
            mac_type=2,
            handle=self.if32.if_hdl)
        self.vlan30.member_add([self.if31, self.if32])

        ## L3 VI interface (vlan30)
        self.i_ip3 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.ip[3], prefix_length=self.prefix_length[3])
        self.client.switch_api_l3_interface_address_add(device, self.rif3,
                                                        self.vrf, self.i_ip3)

        ## L3 port
        iu4 = interface_union(port_lag_handle=swports[7])
        i_info4 = switcht_interface_info_t(
            device=0,
            type=SWITCH_INTERFACE_TYPE_PORT,
            u=iu4,
            mac=self.rmac_address,
            label=0,
            rif_handle=self.rif4)
        self.if4 = self.client.switch_api_interface_create(device, i_info4)
        self.i_ip4 = switcht_ip_addr_t(
            addr_type=0, ipaddr='4.4.4.4', prefix_length=self.prefix_length[3])
        self.client.switch_api_l3_interface_address_add(device, self.rif4,
                                                        self.vrf, self.i_ip4)

    def runTest(self):
        self.fail_flag = False

        # Add static routes L3 interface
        self.nh_ip4 = switcht_ip_addr_t(
            addr_type=0, ipaddr='4.4.4.10', prefix_length=32)
        self.nhop4, self.neighbor4 = switch_api_l3_nhop_neighbor_create(self, device, self.rif4, self.nh_ip4, '00:44:44:44:44:44')
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip4,
                                            self.nhop4)

        # Add static routes vlan10
        self.nh_ip11 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.nh_ip_vlan10[1], prefix_length=32)
        self.nhop11, self.neighbor11 = switch_api_l3_nhop_neighbor_create(self, device, self.rif1, self.nh_ip11, self.nh_mac_vlan10[1])
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip11,
                                            self.nhop11)

        self.nh_ip12 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.nh_ip_vlan10[2], prefix_length=32)
        self.nhop12, self.neighbor12 = switch_api_l3_nhop_neighbor_create(self, device, self.rif1, self.nh_ip12, self.nh_mac_vlan10[2])
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip12,
                                            self.nhop12)

        # Add static routes vlan20
        self.nh_ip21 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.nh_ip_vlan20[1], prefix_length=32)
        self.nhop21, self.neighbor21 = switch_api_l3_nhop_neighbor_create(self, device, self.rif2, self.nh_ip21, self.nh_mac_vlan20[1])
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip21,
                                            self.nhop21)

        self.nh_ip22 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.nh_ip_vlan20[2], prefix_length=32)
        self.nhop22, self.neighbor22 = switch_api_l3_nhop_neighbor_create(self, device, self.rif2, self.nh_ip22, self.nh_mac_vlan20[2])
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip22,
                                            self.nhop22)

        # Add static routes vlan10
        self.nh_ip31 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.nh_ip_vlan30[1], prefix_length=32)
        self.nhop31, self.neighbor31 = switch_api_l3_nhop_neighbor_create(self, device, self.rif3, self.nh_ip31, self.nh_mac_vlan30[1])
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip31,
                                            self.nhop31)

        self.nh_ip32 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.nh_ip_vlan30[2], prefix_length=32)
        self.nhop32, self.neighbor32 = switch_api_l3_nhop_neighbor_create(self, device, self.rif3, self.nh_ip32, self.nh_mac_vlan30[2])
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip32,
                                            self.nhop32)

        print "Sending L3 port {0} -> L3 VI 10 trunk port {1}".format(
            swports[7], swports[2])
        pkt = simple_tcp_packet(
            eth_dst=self.rmac_address,
            eth_src='00:44:44:44:44:44',
            ip_dst=self.nh_ip_vlan10[2],
            ip_src='4.4.4.10',
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.nh_mac_vlan10[2],
            eth_src=self.rmac_address,
            ip_dst=self.nh_ip_vlan10[2],
            ip_src='4.4.4.10',
            dl_vlan_enable=True,
            vlan_vid=10,
            pktlen=104,
            ip_id=101,
            ip_ttl=63)
        send_packet(self, swports[7], str(pkt))
        verify_packets(self, exp_pkt, [swports[2]])

        print "Sending L3 VI 10 trunk portt {0} -> L3 port {1}".format(
            swports[2], swports[7])
        pkt = simple_tcp_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac_vlan10[2],
            ip_dst='4.4.4.10',
            ip_src=self.nh_ip_vlan10[2],
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:44:44:44:44:44',
            eth_src=self.rmac_address,
            ip_dst='4.4.4.10',
            ip_src=self.nh_ip_vlan10[2],
            pktlen=96,
            ip_id=101,
            ip_ttl=63)
        send_packet(self, swports[2], str(pkt))
        verify_packets(self, exp_pkt, [swports[7]])

        print "Sending L3 port {0} -> L3 VI 10 access port {1}".format(
            swports[7], swports[1])
        pkt = simple_tcp_packet(
            eth_dst=self.rmac_address,
            eth_src='00:44:44:44:44:44',
            ip_dst=self.nh_ip_vlan10[1],
            ip_src='4.4.4.10',
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.nh_mac_vlan10[1],
            eth_src=self.rmac_address,
            ip_dst=self.nh_ip_vlan10[1],
            ip_src='4.4.4.10',
            ip_id=101,
            ip_ttl=63)
        send_packet(self, swports[7], str(pkt))
        verify_packets(self, exp_pkt, [swports[1]])

        print "Sending L3 VI 10 access portt {0} -> L3 port {1}".format(
            swports[1], swports[7])
        pkt = simple_tcp_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac_vlan10[1],
            ip_dst='4.4.4.10',
            ip_src=self.nh_ip_vlan10[1],
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst='00:44:44:44:44:44',
            eth_src=self.rmac_address,
            ip_dst='4.4.4.10',
            ip_src=self.nh_ip_vlan10[1],
            ip_id=101,
            ip_ttl=63)
        send_packet(self, swports[1], str(pkt))
        verify_packets(self, exp_pkt, [swports[7]])

        print "Sending L3 VI 10 access port {0} -> L3 VI 20 trunk port {1}".format(
            swports[1], swports[4])
        pkt = simple_tcp_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac_vlan10[1],
            ip_dst=self.nh_ip_vlan20[2],
            ip_src=self.src_ip_vlan10[1],
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.nh_mac_vlan20[2],
            eth_src=self.rmac_address,
            ip_dst=self.nh_ip_vlan20[2],
            ip_src=self.src_ip_vlan10[1],
            dl_vlan_enable=True,
            vlan_vid=20,
            pktlen=104,
            ip_id=101,
            ip_ttl=63)
        send_packet(self, swports[1], str(pkt))
        verify_packets(self, exp_pkt, [swports[4]])

        print "Sending L3 VI 10 trunk port {0} -> L3 VI 30 access port {1}".format(
            swports[2], swports[5])
        pkt = simple_tcp_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac_vlan10[2],
            ip_dst=self.nh_ip_vlan30[1],
            ip_src=self.src_ip_vlan10[2],
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.nh_mac_vlan30[1],
            eth_src=self.rmac_address,
            ip_dst=self.nh_ip_vlan30[1],
            ip_src=self.src_ip_vlan10[2],
            pktlen=96,
            ip_id=101,
            ip_ttl=63)
        send_packet(self, swports[2], str(pkt))
        verify_packets(self, exp_pkt, [swports[5]])

        print "Sending L3 VI 20 access port {0} -> L3 VI 30 trunk port {1}".format(
            swports[3], swports[6])
        pkt = simple_tcp_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac_vlan20[1],
            ip_dst=self.nh_ip_vlan30[2],
            ip_src=self.src_ip_vlan20[1],
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.nh_mac_vlan30[2],
            eth_src=self.rmac_address,
            ip_dst=self.nh_ip_vlan30[2],
            ip_src=self.src_ip_vlan20[1],
            dl_vlan_enable=True,
            vlan_vid=30,
            pktlen=104,
            ip_id=101,
            ip_ttl=63)
        send_packet(self, swports[3], str(pkt))
        verify_packets(self, exp_pkt, [swports[6]])

        print "Sending L3 VI 20 trunk port {0} -> L3 VI 10 trunk port {1}".format(
            swports[4], swports[2])
        pkt = simple_tcp_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac_vlan20[2],
            ip_dst=self.nh_ip_vlan10[2],
            ip_src=self.src_ip_vlan20[2],
            dl_vlan_enable=True,
            vlan_vid=20,
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.nh_mac_vlan10[2],
            eth_src=self.rmac_address,
            ip_dst=self.nh_ip_vlan10[2],
            ip_src=self.src_ip_vlan20[2],
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_id=101,
            ip_ttl=63)
        send_packet(self, swports[4], str(pkt))
        verify_packets(self, exp_pkt, [swports[2]])

        print "Sending L3 VI 30 access port {0} -> L3 VI 10 access port {1}".format(
            swports[5], swports[1])
        pkt = simple_tcp_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac_vlan30[1],
            ip_dst=self.nh_ip_vlan10[1],
            ip_src=self.src_ip_vlan30[1],
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.nh_mac_vlan10[1],
            eth_src=self.rmac_address,
            ip_dst=self.nh_ip_vlan10[1],
            ip_src=self.src_ip_vlan30[1],
            ip_id=101,
            ip_ttl=63)
        send_packet(self, swports[5], str(pkt))
        verify_packets(self, exp_pkt, [swports[1]])

        print "Sending L3 VI 30 trunk port {0} -> L3 VI 20 access port {1}".format(
            swports[6], swports[3])
        pkt = simple_tcp_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac_vlan30[2],
            ip_dst=self.nh_ip_vlan20[1],
            ip_src=self.src_ip_vlan30[2],
            dl_vlan_enable=True,
            vlan_vid=30,
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.nh_mac_vlan20[1],
            eth_src=self.rmac_address,
            ip_dst=self.nh_ip_vlan20[1],
            ip_src=self.src_ip_vlan30[2],
            pktlen=96,
            ip_id=101,
            ip_ttl=63)
        send_packet(self, swports[6], str(pkt))
        verify_packets(self, exp_pkt, [swports[3]])

    def tearDown(self):
        self.client.switch_api_mac_table_entries_delete_all(device)

        self.vlan10.vlan_cleanup()
        self.vlan20.vlan_cleanup()
        self.vlan30.vlan_cleanup()

        self.client.switch_api_neighbor_delete(device, self.neighbor11)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip11,
                                               self.nhop11)
        self.client.switch_api_nhop_delete(device, self.nhop11)
        self.client.switch_api_neighbor_delete(device, self.neighbor12)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip12,
                                               self.nhop12)
        self.client.switch_api_nhop_delete(device, self.nhop12)
        self.client.switch_api_neighbor_delete(device, self.neighbor21)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip21,
                                               self.nhop21)
        self.client.switch_api_nhop_delete(device, self.nhop21)
        self.client.switch_api_neighbor_delete(device, self.neighbor22)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip22,
                                               self.nhop22)
        self.client.switch_api_nhop_delete(device, self.nhop22)
        self.client.switch_api_neighbor_delete(device, self.neighbor31)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip31,
                                               self.nhop31)
        self.client.switch_api_nhop_delete(device, self.nhop31)
        self.client.switch_api_neighbor_delete(device, self.neighbor32)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip32,
                                               self.nhop32)
        self.client.switch_api_nhop_delete(device, self.nhop32)
        self.client.switch_api_neighbor_delete(device, self.neighbor4)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip4,
                                               self.nhop4)
        self.client.switch_api_nhop_delete(device, self.nhop4)

        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip1)
        self.client.switch_api_l3_interface_address_delete(device, self.rif2,
                                                           self.vrf, self.i_ip2)
        self.client.switch_api_l3_interface_address_delete(device, self.rif3,
                                                           self.vrf, self.i_ip3)
        self.client.switch_api_l3_interface_address_delete(device, self.rif4,
                                                           self.vrf, self.i_ip4)

        self.if11.interface_delete()
        self.if12.interface_delete()
        self.if21.interface_delete()
        self.if22.interface_delete()
        self.if31.interface_delete()
        self.if32.interface_delete()
        self.client.switch_api_interface_delete(device, self.if4)

        self.client.switch_api_rif_delete(0, self.rif1)
        self.client.switch_api_rif_delete(0, self.rif2)
        self.client.switch_api_rif_delete(0, self.rif3)
        self.client.switch_api_rif_delete(0, self.rif4)

        self.client.switch_api_router_mac_delete(device, self.rmac,
                                                 self.rmac_address)
        self.client.switch_api_router_mac_group_delete(device, self.rmac)

        self.client.switch_api_vrf_delete(device, self.vrf)

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('negative')
class L3NonExistentIPv4TrafficTest(api_base_tests.ThriftInterfaceDataPlane):
    def setUp(self):
        print "\nConfiguring different l3 interface type for non existent ipv4 traffic test\n"
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.ip = ['', '1.1.1.1', '2.2.2.2', '3.3.3.3']
        self.ip_src = ['', '1.1.1.100', '2.2.2.100', '3.3.3.100']
        self.nh_ip = ['', '11.11.11.11', '22.22.22.22', '33.33.33.33']
        self.nh_mac = [
            '', '00:11:11:11:11:11', '00:22:22:22:22:22', '00:33:33:33:33:33'
        ]
        self.prefix_length = ['', 16, 16, 16]
        self.rmac_address = '00:77:66:55:44:33'

        self.vrf = self.client.switch_api_vrf_create(device, swports[1])
        self.rmac = self.client.switch_api_router_mac_group_create(device)
        self.client.switch_api_router_mac_add(device, self.rmac,
                                              self.rmac_address)
        self.vlan10 = Vlan(client=self.client, device=device, vlan_id=10)

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif1 = self.client.switch_api_rif_create(0, rif_info1)
        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif2 = self.client.switch_api_rif_create(0, rif_info2)
        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif3 = self.client.switch_api_rif_create(0, rif_info3)

        ## L3 interface
        iu1 = interface_union(port_lag_handle=swports[1])
        i_info1 = switcht_interface_info_t(
            device=0,
            type=SWITCH_INTERFACE_TYPE_PORT,
            u=iu1,
            mac=self.rmac_address,
            label=0,
            rif_handle=self.rif1)
        self.if1 = self.client.switch_api_interface_create(device, i_info1)
        self.i_ip1 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.ip[1], prefix_length=self.prefix_length[1])
        self.client.switch_api_l3_interface_address_add(device, self.rif1,
                                                        self.vrf, self.i_ip1)

        ## L3 VI interface
        self.i_ip2 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.ip[2], prefix_length=self.prefix_length[2])
        self.client.switch_api_l3_interface_address_add(device, self.rif2,
                                                        self.vrf, self.i_ip2)

        ## L2 access interface in vlan 10 for flood test
        self.if11 = L2Interface(
            client=self.client, device=device, intf_type=2, port=swports[4])
        self.mac11 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac=self.nh_mac[2],
            mac_type=2,
            handle=self.if11.if_hdl)

        ## L2 trunk interface in vlan 10 for flood test
        self.if12 = L2Interface(
            client=self.client, device=device, intf_type=3, port=swports[5])

        ## L2 lag access interface in vlan 10 for flood test
        self.lag6 = Lag(client=self.client, device=device)
        self.lag6.member_add([swports[6]])
        self.if13 = L2Interface(
            client=self.client,
            device=device,
            intf_type=2,
            port=self.lag6.lag_hdl)

        ## L2 lag trunk interface in vlan 10 for flood test
        self.lag7 = Lag(client=self.client, device=device)
        self.lag7.member_add([swports[7]])
        self.if14 = L2Interface(
            client=self.client,
            device=device,
            intf_type=3,
            port=self.lag7.lag_hdl)

        self.vlan10.member_add([self.if11, self.if12, self.if13, self.if14])

        ## L3 lag interface
        self.lag = Lag(client=self.client, device=device)
        self.lag.member_add([swports[3]])
        iu3 = interface_union(port_lag_handle=self.lag.lag_hdl)
        i_info3 = switcht_interface_info_t(
            device=0,
            type=SWITCH_INTERFACE_TYPE_PORT,
            u=iu3,
            mac=self.rmac_address,
            label=0,
            rif_handle=self.rif3)
        self.if3 = self.client.switch_api_interface_create(device, i_info3)
        self.i_ip3 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.ip[3], prefix_length=self.prefix_length[3])
        self.client.switch_api_l3_interface_address_add(device, self.rif3,
                                                        self.vrf, self.i_ip3)

    def runTest(self):
        self.fail_flag = False

        print "\nSending L3 packet L3 port {0} -> non existent next hop L3 VI [vlan=10] port {1}".format(
            swports[1], swports[4])
        pkt = simple_tcp_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[1],
            ip_dst=self.nh_ip[2],
            ip_src=self.ip_src[1],
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.nh_mac[2],
            eth_src=self.rmac_address,
            ip_dst=self.nh_ip[2],
            ip_src=self.ip_src[1],
            ip_id=101,
            ip_ttl=63)
        send_packet(self, swports[1], str(pkt))
        print "Verify packet should be received by CPU port... "
        verify_packet_prefix(self, str(pkt), swports[64], 12)
        print "Verify packet not recieved on other port..."
        verify_no_other_packets(self)

        print "\nSending L3 packet L3 VI access port {0} -> non existent next hop L3 lag port {1}".format(
            swports[4], swports[3])
        pkt = simple_tcp_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[2],
            ip_dst=self.nh_ip[3],
            ip_src=self.ip_src[2],
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.nh_mac[3],
            eth_src=self.rmac_address,
            ip_dst=self.nh_ip[3],
            ip_src=self.ip_src[2],
            ip_id=101,
            ip_ttl=63)
        send_packet(self, swports[4], str(pkt))
        print "Verify packet should be received by CPU port... "
        verify_packet_prefix(self, str(pkt), swports[64], 12)
        print "Verify packet not recieved on other port..."
        verify_no_other_packets(self)

        print "\nSending L3 packet L3 VI trunk port {0} -> non existent next hop L3 lag port {1}".format(
            swports[5], swports[3])
        pkt = simple_tcp_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[2],
            ip_dst=self.nh_ip[3],
            ip_src=self.ip_src[2],
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.nh_mac[3],
            eth_src=self.rmac_address,
            ip_dst=self.nh_ip[3],
            ip_src=self.ip_src[2],
            ip_id=101,
            ip_ttl=63)
        send_packet(self, swports[5], str(pkt))
        print "Verify packet should be received by CPU port... "
        verify_packet_prefix(self, str(pkt), swports[64], 12)
        print "Verify packet not recieved on other port..."
        verify_no_other_packets(self)

        print "\nSending L3 packet L3 VI access lag port {0} -> non existent next hop L3 lag port {1}".format(
            swports[6], swports[3])
        pkt = simple_tcp_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[2],
            ip_dst=self.nh_ip[3],
            ip_src=self.ip_src[2],
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.nh_mac[3],
            eth_src=self.rmac_address,
            ip_dst=self.nh_ip[3],
            ip_src=self.ip_src[2],
            ip_id=101,
            ip_ttl=63)
        send_packet(self, swports[6], str(pkt))
        print "Verify packet should be received by CPU port... "
        verify_packet_prefix(self, str(pkt), swports[64], 12)
        print "Verify packet not recieved on other port..."
        verify_no_other_packets(self)

        print "\nSending L3 packet L3 VI trunk lag port {0} -> non existent next hop L3 lag port {1}".format(
            swports[7], swports[3])
        pkt = simple_tcp_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[2],
            ip_dst=self.nh_ip[3],
            ip_src=self.ip_src[2],
            dl_vlan_enable=True,
            vlan_vid=10,
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.nh_mac[3],
            eth_src=self.rmac_address,
            ip_dst=self.nh_ip[3],
            ip_src=self.ip_src[2],
            ip_id=101,
            ip_ttl=63)
        send_packet(self, swports[7], str(pkt))
        print "Verify packet should be received by CPU port... "
        verify_packet_prefix(self, str(pkt), swports[64], 12)
        print "Verify packet not recieved on other port..."
        verify_no_other_packets(self)

        print "\nSending L3 packet L3 lag port {0} -> non existent next hop L3 port {1}".format(
            swports[3], swports[1])
        pkt = simple_tcp_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[3],
            ip_dst=self.nh_ip[1],
            ip_src=self.ip_src[3],
            ip_id=101,
            ip_ttl=64)
        exp_pkt = simple_tcp_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[3],
            ip_dst=self.nh_ip[1],
            ip_src=self.ip_src[3],
            ip_id=101,
            ip_ttl=64,
            pktlen=116)
        send_packet(self, swports[3], str(pkt))
        print "Verify packet should be received by CPU port... "
        verify_packet_prefix(self, str(pkt), swports[64], 12)
        print "Verify packet not recieved on other port..."
        verify_no_other_packets(self)

    def tearDown(self):
        self.client.switch_api_mac_table_entries_delete_all(device)

        self.vlan10.vlan_cleanup()

        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip1)
        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip2)
        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip3)

        self.client.switch_api_interface_delete(device, self.if1)
        self.client.switch_api_interface_delete(device, self.if2)
        self.client.switch_api_interface_delete(device, self.if3)

        self.if11.interface_delete()
        self.if12.interface_delete()
        self.if13.interface_delete()
        self.if14.interface_delete()

        self.client.switch_api_rif_delete(0, self.rif1)
        self.client.switch_api_rif_delete(0, self.rif2)
        self.client.switch_api_rif_delete(0, self.rif3)

        self.lag.member_delete([swports[3]])
        self.lag.lag_delete()
        self.lag6.member_delete([swports[6]])
        self.lag6.lag_delete()
        self.lag7.member_delete([swports[7]])
        self.lag7.lag_delete()

        self.client.switch_api_router_mac_delete(device, self.rmac,
                                                 self.rmac_address)
        self.client.switch_api_router_mac_group_delete(device, self.rmac)
        self.client.switch_api_vrf_delete(device, self.vrf)

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('negative')
class L3IPv6DuplicateAddTest(api_base_tests.ThriftInterfaceDataPlane):
    ## JIRA TSW-67 Duplicate or invalid IPv6 address create is not returning error ##
    def setUp(self):
        print "\nConfiguring different l3 interface type for duplicate IPv6 add test\n"
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.ip = [
            '', '2001:1:1:1::1', '2001:2:2:2::2', '2001:3:3:3::3',
            '2001:4:4:4::4'
        ]
        self.prefix_length = ['', 80, 80, 80, 80]

        self.vrf = self.client.switch_api_vrf_create(device, swports[1])
        self.rmac = self.client.switch_api_router_mac_group_create(device)
        self.client.switch_api_router_mac_add(device, self.rmac,
                                              '00:77:66:55:44:33')
        self.vlan10 = Vlan(client=self.client, device=device, vlan_id=10)

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif1 = self.client.switch_api_rif_create(0, rif_info)
        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif2 = self.client.switch_api_rif_create(0, rif_info2)
        self.rif3 = self.client.switch_api_rif_create(0, rif_info)
        self.rif4 = self.client.switch_api_rif_create(0, rif_info)

        ## L3 interface
        iu1 = interface_union(port_lag_handle=swports[1])
        i_info1 = switcht_interface_info_t(
            device=0,
            type=SWITCH_INTERFACE_TYPE_PORT,
            u=iu1,
            mac='00:77:66:55:44:33',
            label=0,
            rif_handle=self.rif1)
        self.if1 = self.client.switch_api_interface_create(device, i_info1)

        self.i_ip1 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.ip[1], prefix_length=self.prefix_length[1])
        self.client.switch_api_l3_interface_address_add(device, self.rif1,
                                                        self.vrf, self.i_ip1)

        ## L2 access vlan 10 interface
        self.if0 = L2Interface(
            client=self.client, device=device, intf_type=2, port=swports[2])
        self.vlan10.member_add([self.if0])
        self.mac1 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac='00:22:22:22:22:22',
            mac_type=2,
            handle=self.if0.if_hdl)
        ## L3 VI interface
        self.i_ip2 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.ip[2], prefix_length=self.prefix_length[2])
        self.client.switch_api_l3_interface_address_add(device, self.rif2,
                                                        self.vrf, self.i_ip2)

        ## L3 sub-interface
        pv3 = switcht_port_vlan_t(port_lag_handle=swports[3], vlan_id=10)
        iu3 = interface_union(port_vlan=pv3)
        i_info3 = switcht_interface_info_t(
            device=0,
            type=6,
            u=iu3,
            mac='00:77:66:55:44:33',
            label=0,
            rif_handle=self.rif3)
        self.if3 = self.client.switch_api_interface_create(device, i_info3)
        self.i_ip3 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.ip[3], prefix_length=self.prefix_length[3])
        self.client.switch_api_l3_interface_address_add(device, self.rif3,
                                                        self.vrf, self.i_ip3)

        ## L3 lag interface
        self.lag = Lag(client=self.client, device=device)
        self.lag.member_add([swports[4]])
        iu4 = interface_union(port_lag_handle=self.lag.lag_hdl)
        i_info4 = switcht_interface_info_t(
            device=0,
            type=SWITCH_INTERFACE_TYPE_PORT,
            u=iu4,
            mac='00:77:66:55:44:33',
            label=0,
            rif_handle=self.rif4)
        self.if4 = self.client.switch_api_interface_create(device, i_info4)
        self.i_ip4 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.ip[4], prefix_length=self.prefix_length[4])
        self.client.switch_api_l3_interface_address_add(device, self.rif4,
                                                        self.vrf, self.i_ip4)

    def runTest(self):
        self.fail_flag = False

        print "\nTry to add duplicate IPv6 {0}/{1} on L3 port {2}".format(
            self.ip[1], self.prefix_length[1], swports[1])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif1, self.vrf, self.i_ip1)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Duplicate IPv6 add {0}/{1} on L3 port {2} unexpectedly passed".format(
                self.ip[1], self.prefix_length[1], swports[1])

        print "\nTry to add duplicate IPv6 {0}/{1} on L3 VI port {2}".format(
            self.ip[2], self.prefix_length[2], swports[2])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif2, self.vrf, self.i_ip2)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Duplicate IPv6 add {0}/{1} on L3 VI port {2} unexpectedly passed".format(
                self.ip[2], self.prefix_length[2], swports[2])

        print "\nTry to add duplicate IPv6 {0}/{1} on L3 sub-interface port {2}".format(
            self.ip[3], self.prefix_length[3], swports[3])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif3, self.vrf, self.i_ip3)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Duplicate IPv6 add {0}/{1} on L3 sub-interface port {2} unexpectedly passed".format(
                self.ip[3], self.prefix_length[3], swports[3])

        print "\nTry to add duplicate IPv6 {0}/{1} on L3 lag port {2}".format(
            self.ip[4], self.prefix_length[4], swports[4])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif4, self.vrf, self.i_ip4)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Duplicate IPv6 add {0}/{1} on L3 port {2} unexpectedly passed".format(
                self.ip[4], self.prefix_length[4], swports[4])

        self.assertFalse(self.fail_flag,
                         "Duplicate IPv6 add unexpectedly passed")

    def tearDown(self):
        self.client.switch_api_mac_table_entries_delete_all(device)

        self.vlan10.vlan_cleanup()

        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip1)
        self.client.switch_api_l3_interface_address_delete(device, self.rif3,
                                                           self.vrf, self.i_ip3)
        self.client.switch_api_l3_interface_address_delete(device, self.rif4,
                                                           self.vrf, self.i_ip4)

        self.if0.interface_delete()

        self.client.switch_api_interface_delete(device, self.if1)
        self.client.switch_api_interface_delete(device, self.if2)
        self.client.switch_api_interface_delete(device, self.if3)
        self.client.switch_api_interface_delete(device, self.if4)

        self.client.switch_api_rif_delete(0, self.rif1)
        self.client.switch_api_rif_delete(0, self.rif2)
        self.client.switch_api_rif_delete(0, self.rif3)
        self.client.switch_api_rif_delete(0, self.rif4)

        self.lag.member_delete([swports[4]])
        self.lag.lag_delete()

        self.client.switch_api_router_mac_delete(device, self.rmac,
                                                 '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(device, self.rmac)
        self.client.switch_api_vrf_delete(device, self.vrf)

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('negative')
class L3IPv6InvalidAddTest(api_base_tests.ThriftInterfaceDataPlane):
    ## JIRA TSW-67 Duplicate or invalid IPv6 address create is not returning error ##
    def setUp(self):
        print "\nConfiguring different l3 interface type for invalid IPv6 add test\n"
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.ip = [
            '', '::::', '0::0', '!@#$%^&*abcefgh', 'fe00:1234::1234::1234'
        ]
        self.prefix_length = ['', 80, 80, 80, 80]

        self.vrf = self.client.switch_api_vrf_create(device, swports[1])
        self.rmac = self.client.switch_api_router_mac_group_create(device)
        self.client.switch_api_router_mac_add(device, self.rmac,
                                              '00:77:66:55:44:33')
        self.vlan10 = Vlan(client=self.client, device=device, vlan_id=10)

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif1 = self.client.switch_api_rif_create(0, rif_info)
        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif2 = self.client.switch_api_rif_create(0, rif_info2)
        self.rif3 = self.client.switch_api_rif_create(0, rif_info)
        self.rif4 = self.client.switch_api_rif_create(0, rif_info)

        ## L3 interface
        iu1 = interface_union(port_lag_handle=swports[1])
        i_info1 = switcht_interface_info_t(
            device=0,
            type=SWITCH_INTERFACE_TYPE_PORT,
            u=iu1,
            mac='00:77:66:55:44:33',
            label=0,
            rif_handle=self.rif1)
        self.if1 = self.client.switch_api_interface_create(device, i_info1)

        self.i_ip1 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.ip[1], prefix_length=self.prefix_length[1])
        self.client.switch_api_l3_interface_address_add(device, self.rif1,
                                                        self.vrf, self.i_ip1)

        ## L2 access vlan 10 interface
        self.if0 = L2Interface(
            client=self.client, device=device, intf_type=2, port=swports[2])
        self.vlan10.member_add([self.if0])
        self.mac1 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac='00:22:22:22:22:22',
            mac_type=2,
            handle=self.if0.if_hdl)
        ## L3 VI interface
        self.i_ip2 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.ip[2], prefix_length=self.prefix_length[2])
        self.client.switch_api_l3_interface_address_add(device, self.rif2,
                                                        self.vrf, self.i_ip2)

        ## L3 sub-interface
        pv3 = switcht_port_vlan_t(port_lag_handle=swports[3], vlan_id=10)
        iu3 = interface_union(port_vlan=pv3)
        i_info3 = switcht_interface_info_t(
            device=0,
            type=6,
            u=iu3,
            mac='00:77:66:55:44:33',
            label=0,
            rif_handle=self.rif3)
        self.if3 = self.client.switch_api_interface_create(device, i_info3)
        self.i_ip3 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.ip[3], prefix_length=self.prefix_length[3])
        self.client.switch_api_l3_interface_address_add(device, self.rif3,
                                                        self.vrf, self.i_ip3)

        ## L3 lag interface
        self.lag = Lag(client=self.client, device=device)
        self.lag.member_add([swports[4]])
        iu4 = interface_union(port_lag_handle=self.lag.lag_hdl)
        i_info4 = switcht_interface_info_t(
            device=0,
            type=SWITCH_INTERFACE_TYPE_PORT,
            u=iu4,
            mac='00:77:66:55:44:33',
            label=0,
            rif_handle=self.rif4)
        self.if4 = self.client.switch_api_interface_create(device, i_info4)
        self.i_ip4 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.ip[4], prefix_length=self.prefix_length[4])
        self.client.switch_api_l3_interface_address_add(device, self.rif4,
                                                        self.vrf, self.i_ip4)

    def runTest(self):
        self.fail_flag = False

        print "\nTry to add invalid IPv6 {0}/{1} on L3 port {2}".format(
            self.ip[1], self.prefix_length[1], swports[1])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif1, self.vrf, self.i_ip1)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Invalid IPv6 add {0}/{1} on L3 port {2} unexpectedly passed".format(
                self.ip[1], self.prefix_length[1], swports[1])

        print "\nTry to add invalid IPv6 {0}/{1} on L3 VI port {2}".format(
            self.ip[2], self.prefix_length[2], swports[2])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif2, self.vrf, self.i_ip2)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Invalid IPv6 add {0}/{1} on L3 VI port {2} unexpectedly passed".format(
                self.ip[2], self.prefix_length[2], swports[2])

        print "\nTry to add invalid IPv6 {0}/{1} on L3 sub-interface port {2}".format(
            self.ip[3], self.prefix_length[3], swports[3])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif3, self.vrf, self.i_ip3)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Invalid IPv6 add {0}/{1} on L3 sub-interface port {2} unexpectedly passed".format(
                self.ip[3], self.prefix_length[3], swports[3])

        print "\nTry to add invalid IPv6 {0}/{1} on L3 lag port {2}".format(
            self.ip[4], self.prefix_length[4], swports[4])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif4, self.vrf, self.i_ip4)
        print "Status: {0}".format(status)
        if (status == 0):
            self.fail_flag = True
            print "Invalid IPv6 add {0}/{1} on L3 port {2} unexpectedly passed".format(
                self.ip[4], self.prefix_length[4], swports[4])

        self.assertFalse(self.fail_flag, "Invalid IPv6 add unexpectedly passed")

    def tearDown(self):
        self.client.switch_api_mac_table_entries_delete_all(device)

        self.vlan10.vlan_cleanup()

        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip1)
        self.client.switch_api_l3_interface_address_delete(device, self.rif2,
                                                           self.vrf, self.i_ip2)
        self.client.switch_api_l3_interface_address_delete(device, self.rif3,
                                                           self.vrf, self.i_ip3)
        self.client.switch_api_l3_interface_address_delete(device, self.rif4,
                                                           self.vrf, self.i_ip4)

        self.if0.interface_delete()

        self.client.switch_api_interface_delete(device, self.if1)
        self.client.switch_api_interface_delete(device, self.if3)
        self.client.switch_api_interface_delete(device, self.if4)

        self.client.switch_api_rif_delete(0, self.rif1)
        self.client.switch_api_rif_delete(0, self.rif2)
        self.client.switch_api_rif_delete(0, self.rif3)
        self.client.switch_api_rif_delete(0, self.rif4)

        self.lag.member_delete([swports[4]])
        self.lag.lag_delete()

        self.client.switch_api_router_mac_delete(device, self.rmac,
                                                 '00:77:66:55:44:33')
        self.client.switch_api_router_mac_group_delete(device, self.rmac)
        self.client.switch_api_vrf_delete(device, self.vrf)

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('negative')
class L3IPv6DeleteReaddSameIPTest(api_base_tests.ThriftInterfaceDataPlane):
    def setUp(self):
        print "\nConfiguring different l3 interface type for Ipv6 delete readd same ip test\n"
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.ip = ['', '2001:1:1:1::1', '2001:2:2:2::2', '2001:3:3:3::3']
        self.ip_src = [
            '', '2001:1:1:1::100', '2001:2:2:2::100', '2001:3:3:3::100'
        ]
        self.nh_ip = [
            '', '2001:11:11:11::11', '2001:22:22:22::22', '2001:33:33:33::33'
        ]
        self.nh_mac = [
            '', '00:11:11:11:11:11', '00:22:22:22:22:22', '00:33:33:33:33:33'
        ]
        self.prefix_length = ['', 64, 64, 64]
        self.rmac_address = '00:77:66:55:44:33'

        self.vrf = self.client.switch_api_vrf_create(device, swports[1])
        self.rmac = self.client.switch_api_router_mac_group_create(device)
        self.client.switch_api_router_mac_add(device, self.rmac,
                                              self.rmac_address)
        self.vlan10 = Vlan(client=self.client, device=device, vlan_id=10)

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif1 = self.client.switch_api_rif_create(0, rif_info)
        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif2 = self.client.switch_api_rif_create(0, rif_info2)
        self.rif3 = self.client.switch_api_rif_create(0, rif_info)

        ## L3 interface
        iu1 = interface_union(port_lag_handle=swports[1])
        i_info1 = switcht_interface_info_t(
            device=0,
            type=SWITCH_INTERFACE_TYPE_PORT,
            u=iu1,
            mac=self.rmac_address,
            label=0,
            rif_handle=self.rif1)
        self.if1 = self.client.switch_api_interface_create(device, i_info1)
        self.i_ip1 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.ip[1], prefix_length=self.prefix_length[1])
        self.client.switch_api_l3_interface_address_add(device, self.rif1,
                                                        self.vrf, self.i_ip1)

        ## L2 access vlan 10 interface
        self.if0 = L2Interface(
            client=self.client, device=device, intf_type=2, port=swports[2])
        self.vlan10.member_add([self.if0])
        self.mac1 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac='00:22:22:22:22:22',
            mac_type=2,
            handle=self.if0.if_hdl)
        ## L3 VI interface
        self.i_ip2 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.ip[2], prefix_length=self.prefix_length[2])
        self.client.switch_api_l3_interface_address_add(device, self.rif2,
                                                        self.vrf, self.i_ip2)

        ## L3 lag interface
        self.lag = Lag(client=self.client, device=device)
        self.lag.member_add([swports[3]])
        iu3 = interface_union(port_lag_handle=self.lag.lag_hdl)
        i_info3 = switcht_interface_info_t(
            device=0,
            type=SWITCH_INTERFACE_TYPE_PORT,
            u=iu3,
            mac=self.rmac_address,
            label=0,
            rif_handle=self.rif3)
        self.if3 = self.client.switch_api_interface_create(device, i_info3)
        self.i_ip3 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.ip[3], prefix_length=self.prefix_length[3])
        self.client.switch_api_l3_interface_address_add(device, self.rif3,
                                                        self.vrf, self.i_ip3)

    def runTest(self):
        self.fail_flag = False

        print "\nTry to delete / readd same IPv6 {0}/{1} on L3 port {2}".format(
            self.ip[1], self.prefix_length[1], swports[1])
        status = self.client.switch_api_l3_interface_address_delete(
            device, self.rif1, self.vrf, self.i_ip1)
        print "Delete ip status: {0}".format(status)
        if (status != 0):
            self.fail_flag = True
            print "Delete ip {0}/{1} on L3 port {2} failed".format(
                self.ip[1], self.prefix_length[1], swports[1])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif1, self.vrf, self.i_ip1)
        print "Readd ip status: {0}".format(status)
        if (status != 0):
            self.fail_flag = True
            print "Readd same ip {0}/{1} on L3 port {2} failed".format(
                self.ip[1], self.prefix_length[1], swports[1])

        print "\nTry to delete / readd same IPv6 {0}/{1} on L3 VI port {2}".format(
            self.ip[2], self.prefix_length[2], swports[2])
        status = self.client.switch_api_l3_interface_address_delete(
            device, self.rif2, self.vrf, self.i_ip2)
        print "Delete ip tatus: {0}".format(status)
        if (status != 0):
            self.fail_flag = True
            print "Delete ip {0}/{1} on L3 VI port {2} failed".format(
                self.ip[2], self.prefix_length[2], swports[2])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif2, self.vrf, self.i_ip2)
        print "Readd ip status: {0}".format(status)
        if (status != 0):
            self.fail_flag = True
            print "Readd same ip {0}/{1} on L3 VI port {2} failed".format(
                self.ip[2], self.prefix_length[2], swports[2])

        print "\nTry to delete / readd IPv6 {0}/{1} on L3 lag port {2}".format(
            self.ip[3], self.prefix_length[3], swports[3])
        status = self.client.switch_api_l3_interface_address_delete(
            device, self.rif3, self.vrf, self.i_ip3)
        print "Delete ip tatus: {0}".format(status)
        if (status != 0):
            self.fail_flag = True
            print "Delete ip {0}/{1} on L3 lag port {2} failed".format(
                self.ip[3], self.prefix_length[3], swports[3])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif3, self.vrf, self.i_ip3)
        print "Readd ip status: {0}".format(status)
        if (status != 0):
            self.fail_flag = True
            print "Readd same ip {0}/{1} on L3 lag port {2} failed".format(
                self.ip[3], self.prefix_length[3], swports[3])

        self.assertFalse(self.fail_flag, "Delete / readd same IPv6 failed")

        # Add static routes
        self.nh_ip1 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.nh_ip[1], prefix_length=128)
        self.nhop1, self.neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, self.rif1, self.nh_ip1, self.nh_mac[1])
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip1,
                                            self.nhop1)

        self.nh_ip2 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.nh_ip[2], prefix_length=128)
        self.nhop2, self.neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, self.rif2, self.nh_ip2, self.nh_mac[2])
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip2,
                                            self.nhop2)

        self.nh_ip3 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.nh_ip[3], prefix_length=128)
        self.nhop3, self.neighbor3 = switch_api_l3_nhop_neighbor_create(self, device, self.rif3, self.nh_ip3, self.nh_mac[3])
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip3,
                                            self.nhop3)

        print "\nSending L3 packet L3 port {0} -> L3 VI [vlan=10] port {1}".format(
            swports[1], swports[2])
        pkt = simple_tcpv6_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[1],
            ipv6_dst=self.nh_ip[2],
            ipv6_src=self.ip_src[1],
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst=self.nh_mac[2],
            eth_src=self.rmac_address,
            ipv6_dst=self.nh_ip[2],
            ipv6_src=self.ip_src[1],
            ipv6_hlim=63)
        send_packet(self, swports[1], str(pkt))
        verify_packets(self, exp_pkt, [swports[2]])

        print "Sending L3 packet L3 VI port {0} -> L3 lag port {1}".format(
            swports[2], swports[3])
        pkt = simple_tcpv6_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[2],
            ipv6_dst=self.nh_ip[3],
            ipv6_src=self.ip_src[2],
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst=self.nh_mac[3],
            eth_src=self.rmac_address,
            ipv6_dst=self.nh_ip[3],
            ipv6_src=self.ip_src[2],
            ipv6_hlim=63)
        send_packet(self, swports[2], str(pkt))
        #verify_packets(self, exp_pkt, [swports[3]])

        print "Sending L3 packet L3 lag port {0} -> L3 port {1}".format(
            swports[3], swports[1])
        pkt = simple_tcpv6_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[3],
            ipv6_dst=self.nh_ip[1],
            ipv6_src=self.ip_src[3],
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst=self.nh_mac[1],
            eth_src=self.rmac_address,
            ipv6_dst=self.nh_ip[1],
            ipv6_src=self.ip_src[3],
            ipv6_hlim=63)
        send_packet(self, swports[3], str(pkt))
        verify_packets(self, exp_pkt, [swports[1]])

    def tearDown(self):
        self.client.switch_api_mac_table_entries_delete_all(device)

        self.vlan10.vlan_cleanup()

        self.client.switch_api_neighbor_delete(device, self.neighbor1)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip1,
                                               self.nhop1)
        self.client.switch_api_nhop_delete(device, self.nhop1)

        self.client.switch_api_neighbor_delete(device, self.neighbor2)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip2,
                                               self.nhop2)
        self.client.switch_api_nhop_delete(device, self.nhop2)

        self.client.switch_api_neighbor_delete(device, self.neighbor3)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip3,
                                               self.nhop3)
        self.client.switch_api_nhop_delete(device, self.nhop3)

        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip1)
        self.client.switch_api_l3_interface_address_delete(device, self.rif2,
                                                           self.vrf, self.i_ip2)
        self.client.switch_api_l3_interface_address_delete(device, self.rif3,
                                                           self.vrf, self.i_ip3)

        self.if0.interface_delete()

        self.client.switch_api_interface_delete(device, self.if1)
        self.client.switch_api_interface_delete(device, self.if3)

        self.client.switch_api_rif_delete(0, self.rif1)
        self.client.switch_api_rif_delete(0, self.rif2)
        self.client.switch_api_rif_delete(0, self.rif3)

        self.lag.member_delete([swports[3]])
        self.lag.lag_delete()

        self.client.switch_api_router_mac_delete(device, self.rmac,
                                                 self.rmac_address)
        self.client.switch_api_router_mac_group_delete(device, self.rmac)
        self.client.switch_api_vrf_delete(device, self.vrf)

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('negative')
class L3IPv6AddDeleteUpdateTest(api_base_tests.ThriftInterfaceDataPlane):
    def setUp(self):
        print "\nConfiguring different l3 interface type for ipv6 add / delete / update different ip test\n"
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.old_ip = [
            '', '2001:121:1:1::1', '2001:121:2:2::2', '2001:121:3:3::3'
        ]
        self.ip = ['', '2001:1:1:1::1', '2001:2:2:2::2', '2001:3:3:3::3']
        self.ipv6_src = [
            '', '2001:1:1:1::100', '2001:2:2:2::100', '2001:3:3:3::100'
        ]
        self.nh_ip = [
            '', '2001:11:11:11::11', '2001:22:22:22::22', '2001:33:33:33::33'
        ]
        self.nh_mac = [
            '', '00:11:11:11:11:11', '00:22:22:22:22:22', '00:33:33:33:33:33'
        ]
        self.prefix_length = ['', 64, 64, 64]
        self.rmac_address = '00:77:66:55:44:33'

        self.vrf = self.client.switch_api_vrf_create(device, swports[1])
        self.rmac = self.client.switch_api_router_mac_group_create(device)
        self.client.switch_api_router_mac_add(device, self.rmac,
                                              self.rmac_address)
        self.vlan10 = Vlan(client=self.client, device=device, vlan_id=10)

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif1 = self.client.switch_api_rif_create(0, rif_info)
        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif2 = self.client.switch_api_rif_create(0, rif_info2)
        self.rif3 = self.client.switch_api_rif_create(0, rif_info)

        ## L3 interface
        iu1 = interface_union(port_lag_handle=swports[1])
        i_info1 = switcht_interface_info_t(
            device=0,
            type=SWITCH_INTERFACE_TYPE_PORT,
            u=iu1,
            mac=self.rmac_address,
            label=0,
            rif_handle=self.rif1)
        self.if1 = self.client.switch_api_interface_create(device, i_info1)
        self.i_ip1 = switcht_ip_addr_t(
            addr_type=1,
            ipaddr=self.old_ip[1],
            prefix_length=self.prefix_length[1])
        self.client.switch_api_l3_interface_address_add(device, self.rif1,
                                                        self.vrf, self.i_ip1)

        ## L2 access vlan 10 interface
        self.if0 = L2Interface(
            client=self.client, device=device, intf_type=2, port=swports[2])
        self.vlan10.member_add([self.if0])
        self.mac1 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac='00:22:22:22:22:22',
            mac_type=2,
            handle=self.if0.if_hdl)
        ## L3 VI interface
        self.i_ip2 = switcht_ip_addr_t(
            addr_type=1,
            ipaddr=self.old_ip[2],
            prefix_length=self.prefix_length[2])
        self.client.switch_api_l3_interface_address_add(device, self.rif2,
                                                        self.vrf, self.i_ip2)

        ## L3 lag interface
        self.lag = Lag(client=self.client, device=device)
        self.lag.member_add([swports[3]])
        iu3 = interface_union(port_lag_handle=self.lag.lag_hdl)
        i_info3 = switcht_interface_info_t(
            device=0,
            type=SWITCH_INTERFACE_TYPE_PORT,
            u=iu3,
            mac=self.rmac_address,
            label=0,
            rif_handle=self.rif3)
        self.if3 = self.client.switch_api_interface_create(device, i_info3)
        self.i_ip3 = switcht_ip_addr_t(
            addr_type=1,
            ipaddr=self.old_ip[3],
            prefix_length=self.prefix_length[3])
        self.client.switch_api_l3_interface_address_add(device, self.rif3,
                                                        self.vrf, self.i_ip3)

    def runTest(self):
        self.fail_flag = False

        print "\nTry to delete {0}/{1} update different IPv6 {2}/{1} on L3 port {3}".format(
            self.old_ip[1], self.prefix_length[1], self.ip[1], swports[1])
        status = self.client.switch_api_l3_interface_address_delete(
            device, self.rif1, self.vrf, self.i_ip1)
        print "Delete ip status: {0}".format(status)
        if (status != 0):
            self.fail_flag = True
            print "Delete ip {0}/{1} on L3 port {2} failed".format(
                self.old_ip[1], self.prefix_length[1], swports[1])
        self.i_ip1 = switcht_ip_addr_t(
            addr_type=1,
            ipaddr=self.old_ip[1],
            prefix_length=self.prefix_length[1])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif1, self.vrf, self.i_ip1)
        print "Add ip status: {0}".format(status)
        if (status != 0):
            self.fail_flag = True
            print "Update different ip {0}/{1} on L3 port {2} failed".format(
                self.ip[1], self.prefix_length[1], swports[1])

        print "\nTry to delete {0}/{1} update different IPv6 {2}/{1} on L3 VI port {3}".format(
            self.old_ip[2], self.prefix_length[2], self.ip[2], swports[2])
        status = self.client.switch_api_l3_interface_address_delete(
            device, self.rif2, self.vrf, self.i_ip2)
        print "Delete ip tatus: {0}".format(status)
        if (status != 0):
            self.fail_flag = True
            print "Delete ip {0}/{1} on L3 VI port {2} failed".format(
                self.old_ip[2], self.prefix_length[2], swports[2])
        self.i_ip2 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.ip[2], prefix_length=self.prefix_length[2])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif2, self.vrf, self.i_ip2)
        print "Add ip status: {0}".format(status)
        if (status != 0):
            self.fail_flag = True
            print "Update different ip {0}/{1} on L3 VI port {2} failed".format(
                self.ip[2], self.prefix_length[2], swports[2])

        print "\nTry to delete {0}/{1} update different IPv6 {2}/{1} on L3 lag port {3}".format(
            self.old_ip[3], self.prefix_length[3], self.ip[3], swports[3])
        status = self.client.switch_api_l3_interface_address_delete(
            device, self.rif3, self.vrf, self.i_ip3)
        print "Delete ip tatus: {0}".format(status)
        if (status != 0):
            self.fail_flag = True
            print "Delete ip {0}/{1} on L3 lag port {2} failed".format(
                self.old_ip[3], self.prefix_length[3], swports[3])
        self.i_ip3 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.ip[3], prefix_length=self.prefix_length[3])
        status = self.client.switch_api_l3_interface_address_add(
            device, self.rif3, self.vrf, self.i_ip3)
        print "Add ip status: {0}".format(status)
        if (status != 0):
            self.fail_flag = True
            print "Update different ip {0}/{1} on L3 lag port {2} failed".format(
                self.ip[3], self.prefix_length[3], swports[3])

        self.assertFalse(self.fail_flag,
                         "Delete / update different IPv6 failed")

        # Add static routes
        self.nh_ip1 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.nh_ip[1], prefix_length=128)
        self.nhop1, self.neighbor1 = switch_api_l3_nhop_neighbor_create(self, device, self.rif1, self.nh_ip1, self.nh_mac[1])
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip1,
                                            self.nhop1)

        self.nh_ip2 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.nh_ip[2], prefix_length=128)
        self.nhop2, self.neighbor2 = switch_api_l3_nhop_neighbor_create(self, device, self.rif2, self.nh_ip2, self.nh_mac[2])
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip2,
                                            self.nhop2)

        self.nh_ip3 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.nh_ip[3], prefix_length=128)
        self.nhop3, self.neighbor3 = switch_api_l3_nhop_neighbor_create(self, device, self.rif3, self.nh_ip3, self.nh_mac[3])
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip3,
                                            self.nhop3)

        print "\nSending L3 packet L3 port {0} -> L3 VI [vlan=10] port {1}".format(
            swports[1], swports[2])
        pkt = simple_tcpv6_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[1],
            ipv6_dst=self.nh_ip[2],
            ipv6_src=self.ipv6_src[1],
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst=self.nh_mac[2],
            eth_src=self.rmac_address,
            ipv6_dst=self.nh_ip[2],
            ipv6_src=self.ipv6_src[1],
            ipv6_hlim=63)
        send_packet(self, swports[1], str(pkt))
        verify_packets(self, exp_pkt, [swports[2]])

        print "Sending L3 packet L3 VI port {0} -> L3 lag port {1}".format(
            swports[2], swports[3])
        pkt = simple_tcpv6_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[2],
            ipv6_dst=self.nh_ip[3],
            ipv6_src=self.ipv6_src[2],
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst=self.nh_mac[3],
            eth_src=self.rmac_address,
            ipv6_dst=self.nh_ip[3],
            ipv6_src=self.ipv6_src[2],
            ipv6_hlim=63)
        send_packet(self, swports[2], str(pkt))
        verify_packets(self, exp_pkt, [swports[3]])

        print "Sending L3 packet L3 lag port {0} -> L3 port {1}".format(
            swports[3], swports[1])
        pkt = simple_tcpv6_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[3],
            ipv6_dst=self.nh_ip[1],
            ipv6_src=self.ipv6_src[3],
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst=self.nh_mac[1],
            eth_src=self.rmac_address,
            ipv6_dst=self.nh_ip[1],
            ipv6_src=self.ipv6_src[3],
            ipv6_hlim=63)
        send_packet(self, swports[3], str(pkt))
        verify_packets(self, exp_pkt, [swports[1]])

    def tearDown(self):
        self.client.switch_api_mac_table_entries_delete_all(device)

        self.vlan10.vlan_cleanup()

        self.client.switch_api_neighbor_delete(device, self.neighbor1)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip1,
                                               self.nhop1)
        self.client.switch_api_nhop_delete(device, self.nhop1)

        self.client.switch_api_neighbor_delete(device, self.neighbor2)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip2,
                                               self.nhop2)
        self.client.switch_api_nhop_delete(device, self.nhop2)

        self.client.switch_api_neighbor_delete(device, self.neighbor3)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip3,
                                               self.nhop3)
        self.client.switch_api_nhop_delete(device, self.nhop3)

        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip1)
        self.client.switch_api_l3_interface_address_delete(device, self.rif2,
                                                           self.vrf, self.i_ip2)
        self.client.switch_api_l3_interface_address_delete(device, self.rif3,
                                                           self.vrf, self.i_ip3)

        self.if0.interface_delete()

        self.client.switch_api_interface_delete(device, self.if1)
        self.client.switch_api_interface_delete(device, self.if3)

        self.client.switch_api_rif_delete(0, self.rif1)
        self.client.switch_api_rif_delete(0, self.rif2)
        self.client.switch_api_rif_delete(0, self.rif3)

        self.lag.member_delete([swports[3]])
        self.lag.lag_delete()

        self.client.switch_api_router_mac_delete(device, self.rmac,
                                                 self.rmac_address)
        self.client.switch_api_router_mac_group_delete(device, self.rmac)
        self.client.switch_api_vrf_delete(device, self.vrf)

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('negative')
class L3IPv6MultipleVITest(api_base_tests.ThriftInterfaceDataPlane):
    def setUp(self):
        print "\nConfiguring multiple l3 VI interfaces for ipv6 add / delete / update different ip test\n"
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.ip = ['', '2001:1:1:1::1', '2001:2:2:2::2', '2001:3:3:3::3']
        self.src_ip_vlan10 = ['', '2001:1:1:1::101', '2001:1:1:1::102']
        self.src_ip_vlan20 = ['', '2001:2:2:2::101', '2001:2:2:2::102']
        self.src_ip_vlan30 = ['', '2001:3:3:3::101', '2001:3:3:3::102']
        self.nh_ip_vlan10 = ['', '2001:11:11:11::101', '2001:11:11:11::102']
        self.nh_ip_vlan20 = ['', '2001:22:22:22::101', '2001:22:22:22::102']
        self.nh_ip_vlan30 = ['', '2001:33:33:33::101', '2001:33:33:33::102']
        self.nh_ip_vlan10_new = [
            '', '2001:111:11:11::101', '2001:111:11:11::102'
        ]
        self.nh_ip_vlan20_new = [
            '', '2001:122:22:22::101', '2001:122:22:22::102'
        ]
        self.nh_ip_vlan30_new = [
            '', '2001:133:33:33::101', '2001:133:33:33::102'
        ]
        self.nh_mac_vlan10 = ['', '00:10:11:11:11:11', '00:10:22:22:22:22']
        self.nh_mac_vlan20 = ['', '00:20:11:11:11:11', '00:20:22:22:22:22']
        self.nh_mac_vlan30 = ['', '00:30:11:11:11:11', '00:30:22:22:22:22']
        self.prefix_length = ['', 64, 64, 64]
        self.rmac_address = '00:77:66:55:44:33'

        self.vrf = self.client.switch_api_vrf_create(device, swports[1])
        self.rmac = self.client.switch_api_router_mac_group_create(device)
        self.client.switch_api_router_mac_add(device, self.rmac,
                                              self.rmac_address)
        self.vlan10 = Vlan(client=self.client, device=device, vlan_id=10)
        self.vlan20 = Vlan(client=self.client, device=device, vlan_id=20)
        self.vlan30 = Vlan(client=self.client, device=device, vlan_id=30)

        ## L2 access and trunk interface for vlan 10
        self.if11 = L2Interface(
            client=self.client, device=device, intf_type=2, port=swports[1])
        self.mac11 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac=self.nh_mac_vlan10[1],
            mac_type=2,
            handle=self.if11.if_hdl)
        self.if12 = L2Interface(
            client=self.client, device=device, intf_type=3, port=swports[2])
        self.mac12 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac=self.nh_mac_vlan10[2],
            mac_type=2,
            handle=self.if12.if_hdl)
        self.vlan10.member_add([self.if11, self.if12])

        rif_info1 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif1 = self.client.switch_api_rif_create(0, rif_info1)
        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=20,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif2 = self.client.switch_api_rif_create(0, rif_info2)
        rif_info3 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=30,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif3 = self.client.switch_api_rif_create(0, rif_info3)
        rif_info4 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif4 = self.client.switch_api_rif_create(0, rif_info4)

        ## L3 VI interface (vlan10)
        self.i_ip1 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.ip[1], prefix_length=self.prefix_length[1])
        self.client.switch_api_l3_interface_address_add(device, self.rif1,
                                                        self.vrf, self.i_ip1)

        ## L2 access and trunk interface for vlan 20
        self.if21 = L2Interface(
            client=self.client, device=device, intf_type=2, port=swports[3])
        self.mac21 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan20,
            mac=self.nh_mac_vlan20[1],
            mac_type=2,
            handle=self.if21.if_hdl)
        self.if22 = L2Interface(
            client=self.client, device=device, intf_type=3, port=swports[4])
        self.mac22 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan20,
            mac=self.nh_mac_vlan20[2],
            mac_type=2,
            handle=self.if22.if_hdl)
        self.vlan20.member_add([self.if21, self.if22])

        ## L3 VI interface (vlan20)
        self.i_ip2 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.ip[2], prefix_length=self.prefix_length[2])
        self.client.switch_api_l3_interface_address_add(device, self.rif2,
                                                        self.vrf, self.i_ip2)

        ## L2 access and trunk interface for vlan 30
        self.if31 = L2Interface(
            client=self.client, device=device, intf_type=2, port=swports[5])
        self.mac31 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan30,
            mac=self.nh_mac_vlan30[1],
            mac_type=2,
            handle=self.if31.if_hdl)
        self.if32 = L2Interface(
            client=self.client, device=device, intf_type=3, port=swports[6])
        self.mac32 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan30,
            mac=self.nh_mac_vlan30[2],
            mac_type=2,
            handle=self.if32.if_hdl)
        self.vlan30.member_add([self.if31, self.if32])

        ## L3 VI interface (vlan30)
        self.i_ip3 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.ip[3], prefix_length=self.prefix_length[3])
        self.client.switch_api_l3_interface_address_add(device, self.rif3,
                                                        self.vrf, self.i_ip3)

        ## L3 port
        iu4 = interface_union(port_lag_handle=swports[7])
        i_info4 = switcht_interface_info_t(
            device=0,
            type=SWITCH_INTERFACE_TYPE_PORT,
            u=iu4,
            mac=self.rmac_address,
            label=0,
            rif_handle=self.rif4)
        self.if4 = self.client.switch_api_interface_create(device, i_info4)
        self.i_ip4 = switcht_ip_addr_t(
            addr_type=1,
            ipaddr='2001:4:4:4::4',
            prefix_length=self.prefix_length[3])
        self.client.switch_api_l3_interface_address_add(device, self.rif4,
                                                        self.vrf, self.i_ip4)

    def runTest(self):
        self.fail_flag = False

        # Add static routes L3 interface
        self.nh_ip4 = switcht_ip_addr_t(
            addr_type=1, ipaddr='2001:4:4:4::10', prefix_length=128)
        self.nhop4, self.neighbor4 = switch_api_l3_nhop_neighbor_create(self, device, self.rif4, self.nh_ip4, '00:44:44:44:44:44')
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip4,
                                            self.nhop4)

        # Add static routes vlan10
        self.nh_ip11 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.nh_ip_vlan10[1], prefix_length=128)
        self.nhop11, self.neighbor11 = switch_api_l3_nhop_neighbor_create(self, device, self.rif1, self.nh_ip11, self.nh_mac_vlan10[1])
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip11,
                                            self.nhop11)

        self.nh_ip12 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.nh_ip_vlan10[2], prefix_length=128)
        self.nhop12, self.neighbor12 = switch_api_l3_nhop_neighbor_create(self, device, self.rif1, self.nh_ip12, self.nh_mac_vlan10[2])
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip12,
                                            self.nhop12)

        # Add static routes vlan20
        self.nh_ip21 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.nh_ip_vlan20[1], prefix_length=128)
        self.nhop21, self.neighbor21 = switch_api_l3_nhop_neighbor_create(self, device, self.rif2, self.nh_ip21, self.nh_mac_vlan20[1])
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip21,
                                            self.nhop21)

        self.nh_ip22 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.nh_ip_vlan20[2], prefix_length=128)
        self.nhop22, self.neighbor22 = switch_api_l3_nhop_neighbor_create(self, device, self.rif2, self.nh_ip22, self.nh_mac_vlan20[2])
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip22,
                                            self.nhop22)

        # Add static routes vlan10
        self.nh_ip31 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.nh_ip_vlan30[1], prefix_length=128)
        self.nhop31, self.neighbor31 = switch_api_l3_nhop_neighbor_create(self, device, self.rif3, self.nh_ip31, self.nh_mac_vlan30[1])
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip31,
                                            self.nhop31)

        self.nh_ip128 = switcht_ip_addr_t(
            addr_type=1, ipaddr=self.nh_ip_vlan30[2], prefix_length=128)
        self.nhop128, self.neighbor128 = switch_api_l3_nhop_neighbor_create(self, device, self.rif3, self.nh_ip128, self.nh_mac_vlan30[2])
        self.client.switch_api_l3_route_add(device, self.vrf, self.nh_ip128,
                                            self.nhop128)

        print "Sending L3 port {0} -> L3 VI 10 trunk port {1}".format(
            swports[7], swports[2])
        pkt = simple_tcpv6_packet(
            eth_dst=self.rmac_address,
            eth_src='00:44:44:44:44:44',
            ipv6_dst=self.nh_ip_vlan10[2],
            ipv6_src='2001:4:4:4::10',
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst=self.nh_mac_vlan10[2],
            eth_src=self.rmac_address,
            ipv6_dst=self.nh_ip_vlan10[2],
            ipv6_src='2001:4:4:4::10',
            dl_vlan_enable=True,
            vlan_vid=10,
            pktlen=104,
            ipv6_hlim=63)
        send_packet(self, swports[7], str(pkt))
        verify_packets(self, exp_pkt, [swports[2]])

        print "Sending L3 VI 10 trunk portt {0} -> L3 port {1}".format(
            swports[2], swports[7])
        pkt = simple_tcpv6_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac_vlan10[2],
            ipv6_dst='2001:4:4:4::10',
            ipv6_src=self.nh_ip_vlan10[2],
            dl_vlan_enable=True,
            vlan_vid=10,
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst='00:44:44:44:44:44',
            eth_src=self.rmac_address,
            ipv6_dst='2001:4:4:4::10',
            ipv6_src=self.nh_ip_vlan10[2],
            pktlen=96,
            ipv6_hlim=63)
        send_packet(self, swports[2], str(pkt))
        verify_packets(self, exp_pkt, [swports[7]])

        print "Sending L3 port {0} -> L3 VI 10 access port {1}".format(
            swports[7], swports[1])
        pkt = simple_tcpv6_packet(
            eth_dst=self.rmac_address,
            eth_src='00:44:44:44:44:44',
            ipv6_dst=self.nh_ip_vlan10[1],
            ipv6_src='2001:4:4:4::10',
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst=self.nh_mac_vlan10[1],
            eth_src=self.rmac_address,
            ipv6_dst=self.nh_ip_vlan10[1],
            ipv6_src='2001:4:4:4::10',
            ipv6_hlim=63)
        send_packet(self, swports[7], str(pkt))
        verify_packets(self, exp_pkt, [swports[1]])

        print "Sending L3 VI 10 access portt {0} -> L3 port {1}".format(
            swports[1], swports[7])
        pkt = simple_tcpv6_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac_vlan10[1],
            ipv6_dst='2001:4:4:4::10',
            ipv6_src=self.nh_ip_vlan10[1],
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst='00:44:44:44:44:44',
            eth_src=self.rmac_address,
            ipv6_dst='2001:4:4:4::10',
            ipv6_src=self.nh_ip_vlan10[1],
            ipv6_hlim=63)
        send_packet(self, swports[1], str(pkt))
        verify_packets(self, exp_pkt, [swports[7]])

        print "Sending L3 VI 10 access port {0} -> L3 VI 20 trunk port {1}".format(
            swports[1], swports[4])
        pkt = simple_tcpv6_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac_vlan10[1],
            ipv6_dst=self.nh_ip_vlan20[2],
            ipv6_src=self.src_ip_vlan10[1],
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst=self.nh_mac_vlan20[2],
            eth_src=self.rmac_address,
            ipv6_dst=self.nh_ip_vlan20[2],
            ipv6_src=self.src_ip_vlan10[1],
            dl_vlan_enable=True,
            vlan_vid=20,
            pktlen=104,
            ipv6_hlim=63)
        send_packet(self, swports[1], str(pkt))
        verify_packets(self, exp_pkt, [swports[4]])

        print "Sending L3 VI 10 trunk port {0} -> L3 VI 30 access port {1}".format(
            swports[2], swports[5])
        pkt = simple_tcpv6_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac_vlan10[2],
            ipv6_dst=self.nh_ip_vlan30[1],
            ipv6_src=self.src_ip_vlan10[2],
            dl_vlan_enable=True,
            vlan_vid=10,
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst=self.nh_mac_vlan30[1],
            eth_src=self.rmac_address,
            ipv6_dst=self.nh_ip_vlan30[1],
            ipv6_src=self.src_ip_vlan10[2],
            pktlen=96,
            ipv6_hlim=63)
        send_packet(self, swports[2], str(pkt))
        verify_packets(self, exp_pkt, [swports[5]])

        print "Sending L3 VI 20 access port {0} -> L3 VI 30 trunk port {1}".format(
            swports[3], swports[6])
        pkt = simple_tcpv6_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac_vlan20[1],
            ipv6_dst=self.nh_ip_vlan30[2],
            ipv6_src=self.src_ip_vlan20[1],
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst=self.nh_mac_vlan30[2],
            eth_src=self.rmac_address,
            ipv6_dst=self.nh_ip_vlan30[2],
            ipv6_src=self.src_ip_vlan20[1],
            dl_vlan_enable=True,
            vlan_vid=30,
            pktlen=104,
            ipv6_hlim=63)
        send_packet(self, swports[3], str(pkt))
        verify_packets(self, exp_pkt, [swports[6]])

        print "Sending L3 VI 20 trunk port {0} -> L3 VI 10 trunk port {1}".format(
            swports[4], swports[2])
        pkt = simple_tcpv6_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac_vlan20[2],
            ipv6_dst=self.nh_ip_vlan10[2],
            ipv6_src=self.src_ip_vlan20[2],
            dl_vlan_enable=True,
            vlan_vid=20,
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst=self.nh_mac_vlan10[2],
            eth_src=self.rmac_address,
            ipv6_dst=self.nh_ip_vlan10[2],
            ipv6_src=self.src_ip_vlan20[2],
            dl_vlan_enable=True,
            vlan_vid=10,
            ipv6_hlim=63)
        send_packet(self, swports[4], str(pkt))
        verify_packets(self, exp_pkt, [swports[2]])

        print "Sending L3 VI 30 access port {0} -> L3 VI 10 access port {1}".format(
            swports[5], swports[1])
        pkt = simple_tcpv6_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac_vlan30[1],
            ipv6_dst=self.nh_ip_vlan10[1],
            ipv6_src=self.src_ip_vlan30[1],
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst=self.nh_mac_vlan10[1],
            eth_src=self.rmac_address,
            ipv6_dst=self.nh_ip_vlan10[1],
            ipv6_src=self.src_ip_vlan30[1],
            ipv6_hlim=63)
        send_packet(self, swports[5], str(pkt))
        verify_packets(self, exp_pkt, [swports[1]])

        print "Sending L3 VI 30 trunk port {0} -> L3 VI 20 access port {1}".format(
            swports[6], swports[3])
        pkt = simple_tcpv6_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac_vlan30[2],
            ipv6_dst=self.nh_ip_vlan20[1],
            ipv6_src=self.src_ip_vlan30[2],
            dl_vlan_enable=True,
            vlan_vid=30,
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst=self.nh_mac_vlan20[1],
            eth_src=self.rmac_address,
            ipv6_dst=self.nh_ip_vlan20[1],
            ipv6_src=self.src_ip_vlan30[2],
            pktlen=96,
            ipv6_hlim=63)
        send_packet(self, swports[6], str(pkt))
        verify_packets(self, exp_pkt, [swports[3]])

    def tearDown(self):
        self.client.switch_api_mac_table_entries_delete_all(device)

        self.vlan10.vlan_cleanup()
        self.vlan20.vlan_cleanup()
        self.vlan30.vlan_cleanup()

        self.client.switch_api_neighbor_delete(device, self.neighbor11)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip11,
                                               self.nhop11)
        self.client.switch_api_nhop_delete(device, self.nhop11)
        self.client.switch_api_neighbor_delete(device, self.neighbor12)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip12,
                                               self.nhop12)
        self.client.switch_api_nhop_delete(device, self.nhop12)
        self.client.switch_api_neighbor_delete(device, self.neighbor21)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip21,
                                               self.nhop21)
        self.client.switch_api_nhop_delete(device, self.nhop21)
        self.client.switch_api_neighbor_delete(device, self.neighbor22)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip22,
                                               self.nhop22)
        self.client.switch_api_nhop_delete(device, self.nhop22)
        self.client.switch_api_neighbor_delete(device, self.neighbor31)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip31,
                                               self.nhop31)
        self.client.switch_api_nhop_delete(device, self.nhop31)
        self.client.switch_api_neighbor_delete(device, self.neighbor128)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip128,
                                               self.nhop128)
        self.client.switch_api_nhop_delete(device, self.nhop128)
        self.client.switch_api_neighbor_delete(device, self.neighbor4)
        self.client.switch_api_l3_route_delete(device, self.vrf, self.nh_ip4,
                                               self.nhop4)
        self.client.switch_api_nhop_delete(device, self.nhop4)

        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip1)
        self.client.switch_api_l3_interface_address_delete(device, self.rif2,
                                                           self.vrf, self.i_ip2)
        self.client.switch_api_l3_interface_address_delete(device, self.rif3,
                                                           self.vrf, self.i_ip3)
        self.client.switch_api_l3_interface_address_delete(device, self.rif4,
                                                           self.vrf, self.i_ip4)

        self.if11.interface_delete()
        self.if12.interface_delete()
        self.if21.interface_delete()
        self.if22.interface_delete()
        self.if31.interface_delete()
        self.if32.interface_delete()
        self.client.switch_api_interface_delete(device, self.if4)

        self.client.switch_api_rif_delete(0, self.rif1)
        self.client.switch_api_rif_delete(0, self.rif2)
        self.client.switch_api_rif_delete(0, self.rif3)
        self.client.switch_api_rif_delete(0, self.rif4)

        self.client.switch_api_router_mac_delete(device, self.rmac,
                                                 self.rmac_address)
        self.client.switch_api_router_mac_group_delete(device, self.rmac)

        self.client.switch_api_vrf_delete(device, self.vrf)

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


###############################################################################
@group('negative')
class L3NonExistentIPv6TrafficTest(api_base_tests.ThriftInterfaceDataPlane):
    def setUp(self):
        print "\nConfiguring different l3 interface type for non existent ipv6 traffic test\n"
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        self.ip = ['', '2001:1:1:1::1', '2001:2:2:2::2', '2001:3:3:3::3']
        self.ipv6_src = [
            '', '2001:1:1:1::100', '2001:2:2:2::100', '2001:3:3:3::100'
        ]
        self.nh_ip = [
            '', '2001:11:11:11::11', '2001:22:22:22::22', '2001:33:33:33::33'
        ]
        self.nh_mac = [
            '', '00:11:11:11:11:11', '00:22:22:22:22:22', '00:33:33:33:33:33'
        ]
        self.prefix_length = ['', 64, 64, 64]
        self.rmac_address = '00:77:66:55:44:33'

        self.vrf = self.client.switch_api_vrf_create(device, swports[1])
        self.rmac = self.client.switch_api_router_mac_group_create(device)
        self.client.switch_api_router_mac_add(device, self.rmac,
                                              self.rmac_address)
        self.vlan10 = Vlan(client=self.client, device=device, vlan_id=10)

        ## L3 interface
        iu1 = interface_union(port_lag_handle=swports[1])
        i_info1 = switcht_interface_info_t(
            device=0,
            type=SWITCH_INTERFACE_TYPE_PORT,
            u=iu1,
            mac=self.rmac_address,
            label=0,
            rif_handle=self.rif1)
        self.if1 = self.client.switch_api_interface_create(device, i_info1)
        self.i_ip1 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.ip[1], prefix_length=self.prefix_length[1])
        self.client.switch_api_l3_interface_address_add(device, self.rif1,
                                                        self.vrf, self.i_ip1)

        rif_info = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_INTF,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif1 = self.client.switch_api_rif_create(0, rif_info)
        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=self.vrf,
            rmac_handle=self.rmac,
            v4_unicast_enabled=True,
            v6_unicast_enabled=True)
        self.rif2 = self.client.switch_api_rif_create(0, rif_info2)
        self.rif3 = self.client.switch_api_rif_create(0, rif_info)

        ## L3 VI interface
        self.i_ip2 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.ip[2], prefix_length=self.prefix_length[2])
        self.client.switch_api_l3_interface_address_add(device, self.rif2,
                                                        self.vrf, self.i_ip2)

        ## L2 access interface in vlan 10 for flood test
        self.if11 = L2Interface(
            client=self.client, device=device, intf_type=2, port=swports[4])
        self.mac11 = MacEntry(
            client=self.client,
            device=device,
            vlan=self.vlan10,
            mac=self.nh_mac[2],
            mac_type=2,
            handle=self.if11.if_hdl)

        ## L2 trunk interface in vlan 10 for flood test
        self.if12 = L2Interface(
            client=self.client, device=device, intf_type=3, port=swports[5])

        ## L2 lag access interface in vlan 10 for flood test
        self.lag6 = Lag(client=self.client, device=device)
        self.lag6.member_add([swports[6]])
        self.if13 = L2Interface(
            client=self.client,
            device=device,
            intf_type=2,
            port=self.lag6.lag_hdl)

        ## L2 lag trunk interface in vlan 10 for flood test
        self.lag7 = Lag(client=self.client, device=device)
        self.lag7.member_add([swports[7]])
        self.if14 = L2Interface(
            client=self.client,
            device=device,
            intf_type=3,
            port=self.lag7.lag_hdl)

        self.vlan10.member_add([self.if11, self.if12, self.if13, self.if14])

        ## L3 lag interface
        self.lag = Lag(client=self.client, device=device)
        self.lag.member_add([swports[3]])
        iu3 = interface_union(port_lag_handle=self.lag.lag_hdl)
        i_info3 = switcht_interface_info_t(
            device=0,
            type=SWITCH_INTERFACE_TYPE_PORT,
            u=iu3,
            mac=self.rmac_address,
            label=0,
            rif_handle=self.rif3)
        self.if3 = self.client.switch_api_interface_create(device, i_info3)
        self.i_ip3 = switcht_ip_addr_t(
            addr_type=0, ipaddr=self.ip[3], prefix_length=self.prefix_length[3])
        self.client.switch_api_l3_interface_address_add(device, self.rif3,
                                                        self.vrf, self.i_ip3)

    def runTest(self):
        self.fail_flag = False

        print "\nSending L3 packet L3 port {0} -> non existent next hop L3 VI [vlan=10] port {1}".format(
            swports[1], swports[4])
        pkt = simple_tcpv6_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[1],
            ipv6_dst=self.nh_ip[2],
            ipv6_src=self.ipv6_src[1],
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst=self.nh_mac[2],
            eth_src=self.rmac_address,
            ipv6_dst=self.nh_ip[2],
            ipv6_src=self.ipv6_src[1],
            ipv6_hlim=63)
        send_packet(self, swports[1], str(pkt))
        print "Verify packet should be received by CPU port... "
        verify_packet_prefix(self, str(pkt), swports[64], 12)
        print "Verify packet not recieved on other port..."
        verify_no_other_packets(self)

        print "\nSending L3 packet L3 VI access port {0} -> non existent next hop L3 lag port {1}".format(
            swports[4], swports[3])
        pkt = simple_tcpv6_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[2],
            ipv6_dst=self.nh_ip[3],
            ipv6_src=self.ipv6_src[2],
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst=self.nh_mac[3],
            eth_src=self.rmac_address,
            ipv6_dst=self.nh_ip[3],
            ipv6_src=self.ipv6_src[2],
            ipv6_hlim=63)
        send_packet(self, swports[4], str(pkt))
        print "Verify packet should be received by CPU port... "
        verify_packet_prefix(self, str(pkt), swports[64], 12)
        print "Verify packet not recieved on other port..."
        verify_no_other_packets(self)

        print "\nSending L3 packet L3 VI trunk port {0} -> non existent next hop L3 lag port {1}".format(
            swports[5], swports[3])
        pkt = simple_tcpv6_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[2],
            ipv6_dst=self.nh_ip[3],
            ipv6_src=self.ipv6_src[2],
            dl_vlan_enable=True,
            vlan_vid=10,
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst=self.nh_mac[3],
            eth_src=self.rmac_address,
            ipv6_dst=self.nh_ip[3],
            ipv6_src=self.ipv6_src[2],
            ipv6_hlim=63)
        send_packet(self, swports[5], str(pkt))
        print "Verify packet should be received by CPU port... "
        verify_packet_prefix(self, str(pkt), swports[64], 12)
        print "Verify packet not recieved on other port..."
        verify_no_other_packets(self)

        print "\nSending L3 packet L3 VI access lag port {0} -> non existent next hop L3 lag port {1}".format(
            swports[6], swports[3])
        pkt = simple_tcpv6_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[2],
            ipv6_dst=self.nh_ip[3],
            ipv6_src=self.ipv6_src[2],
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst=self.nh_mac[3],
            eth_src=self.rmac_address,
            ipv6_dst=self.nh_ip[3],
            ipv6_src=self.ipv6_src[2],
            ipv6_hlim=63)
        send_packet(self, swports[6], str(pkt))
        print "Verify packet should be received by CPU port... "
        verify_packet_prefix(self, str(pkt), swports[64], 12)
        print "Verify packet not recieved on other port..."
        verify_no_other_packets(self)

        print "\nSending L3 packet L3 VI trunk lag port {0} -> non existent next hop L3 lag port {1}".format(
            swports[7], swports[3])
        pkt = simple_tcpv6_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[2],
            ipv6_dst=self.nh_ip[3],
            ipv6_src=self.ipv6_src[2],
            dl_vlan_enable=True,
            vlan_vid=10,
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst=self.nh_mac[3],
            eth_src=self.rmac_address,
            ipv6_dst=self.nh_ip[3],
            ipv6_src=self.ipv6_src[2],
            ipv6_hlim=63)
        send_packet(self, swports[7], str(pkt))
        print "Verify packet should be received by CPU port... "
        verify_packet_prefix(self, str(pkt), swports[64], 12)
        print "Verify packet not recieved on other port..."
        verify_no_other_packets(self)

        print "\nSending L3 packet L3 lag port {0} -> non existent next hop L3 port {1}".format(
            swports[3], swports[1])
        pkt = simple_tcpv6_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[3],
            ipv6_dst=self.nh_ip[1],
            ipv6_src=self.ipv6_src[3],
            ipv6_hlim=64)
        exp_pkt = simple_tcpv6_packet(
            eth_dst=self.rmac_address,
            eth_src=self.nh_mac[3],
            ipv6_dst=self.nh_ip[1],
            ipv6_src=self.ipv6_src[3],
            ipv6_hlim=64,
            pktlen=116)
        send_packet(self, swports[3], str(pkt))
        print "Verify packet should be received by CPU port... "
        verify_packet_prefix(self, str(pkt), swports[64], 12)
        print "Verify packet not recieved on other port..."
        verify_no_other_packets(self)

    def tearDown(self):
        self.client.switch_api_mac_table_entries_delete_all(device)

        self.vlan10.vlan_cleanup()

        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip1)
        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip2)
        self.client.switch_api_l3_interface_address_delete(device, self.rif1,
                                                           self.vrf, self.i_ip3)

        self.client.switch_api_interface_delete(device, self.if1)
        self.client.switch_api_interface_delete(device, self.if3)

        self.client.switch_api_rif_delete(0, self.rif1)
        self.client.switch_api_rif_delete(0, self.rif2)
        self.client.switch_api_rif_delete(0, self.rif3)

        self.if11.interface_delete()
        self.if12.interface_delete()
        self.if13.interface_delete()
        self.if14.interface_delete()

        self.lag.member_delete([swports[3]])
        self.lag.lag_delete()
        self.lag6.member_delete([swports[6]])
        self.lag6.lag_delete()
        self.lag7.member_delete([swports[7]])
        self.lag7.lag_delete()

        self.client.switch_api_router_mac_delete(device, self.rmac,
                                                 self.rmac_address)
        self.client.switch_api_router_mac_group_delete(device, self.rmac)
        self.client.switch_api_vrf_delete(device, self.vrf)

        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)
