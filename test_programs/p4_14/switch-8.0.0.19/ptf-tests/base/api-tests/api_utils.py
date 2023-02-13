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
Thrift API interface basic tests
"""

import switchapi_thrift

import time
import sys
import logging

import unittest
import random

import ptf.dataplane as dataplane
import api_base_tests

from ptf.testutils import *
from ptf.thriftutils import *

import os

from switchapi_thrift.ttypes import *

this_dir = os.path.dirname(os.path.abspath(__file__))

intf_list = []
vlan_list = []


class Vlan:
    def __init__(self, client, device, vlan_id):
        self.client = client
        self.device = device
        self.vlan_id = vlan_id
        self.vlan_hdl = self.client.switch_api_vlan_create(self.device,
                                                           self.vlan_id)
        self.vlan_port_list = []
        assert (self.vlan_hdl != 0)
        vlan_list.append(self)

    def vlan_create(self):
        self.vlan_hdl = self.client.switch_api_vlan_create(self.device,
                                                           self.vlan_id)
        assert (self.vlan_hdl != 0)
        vlan_list.append(self)

    def vlan_delete(self):
        status = self.client.switch_api_vlan_delete(self.device, self.vlan_hdl)
        assert (status == 0)
        vlan_list.remove(self)

    def member_add(self, intf_list):
        for intf in intf_list:
            vlan_port = switcht_vlan_port_t(handle=intf.if_hdl, tagging_mode=0)
            status = self.client.switch_api_vlan_ports_add(
                self.device, self.vlan_hdl, vlan_port)
            assert (status == 0)
            self.vlan_port_list.append(vlan_port)

    def member_delete(self, intf_list):
        for intf in intf_list:
            vlan_port = switcht_vlan_port_t(handle=intf.if_hdl, tagging_mode=0)
            status = self.client.switch_api_vlan_ports_remove(
                self.device, self.vlan_hdl, vlan_port)
            assert (status == 0)
            self.vlan_port_list.remove(vlan_port)

    def learn_enable(self, learn):
        status = self.client.switch_api_vlan_learning_enabled_set(self.vlan_hdl,
                                                                  0)
        assert (status == 0)

    def vlan_hdl(self):
        return self.vlan_hdl

    def vlan_cleanup(self):
        for vlan_port in self.vlan_port_list:
            status = self.client.switch_api_vlan_ports_remove(
                self.device, self.vlan_hdl, vlan_port)
            assert (status == 0)
        while self.vlan_port_list:
            self.vlan_port_list.pop(0)
        assert (len(self.vlan_port_list) == 0)
        status = self.client.switch_api_vlan_delete(self.device, self.vlan_hdl)
        assert (status == 0)

    @staticmethod
    def cleanup():
        for vlan in vlan_list:
            vlan.vlan_cleanup()


class L2Interface:
    def __init__(self, client, device, intf_type, port):
        self.client = client
        self.device = device
        self.intf_type = intf_type
        self.port = port
        iu = interface_union(port_lag_handle=self.port)
        i_info = switcht_interface_info_t(
            self.device,
            type=self.intf_type,
            u=iu,
            mac='00:77:66:55:44:33',
            label=0)
        self.if_hdl = self.client.switch_api_interface_create(self.device,
                                                              i_info)
        assert (self.if_hdl != 0)

    def if_hdl(self):
        return self.if_hdl

    def interface_create(self):
        iu = interface_union(port_lag_handle=self.port)
        i_info = switcht_interface_info_t(
            self.device,
            type=self.intf_type,
            u=iu,
            mac='00:77:66:55:44:33',
            label=0)
        self.if_hdl = self.client.switch_api_interface_create(self.device,
                                                              i_info)
        assert (self.if_hdl != 0)

    def interface_delete(self):
        status = self.client.switch_api_interface_delete(self.device,
                                                         self.if_hdl)
        self.if_hdl = None
        assert (status == 0)


class MacEntry:
    def __init__(self, client, device, vlan, mac, mac_type, handle):
        self.client = client
        self.device = device
        self.vlan_hdl = vlan.vlan_hdl
        self.mac = mac
        self.mac_type = mac_type
        self.handle = handle
        status = self.client.switch_api_mac_table_entry_create(
            self.device, self.vlan_hdl, self.mac, self.mac_type, self.handle)
        assert (status == 0)

    def mac_add(self):
        status = self.client.switch_api_mac_table_entry_create(
            self.device, self.vlan_hdl, self.mac, self.mac_type, self.handle)
        assert (status == 0)

    def mac_delete(self):
        status = self.client.switch_api_mac_table_entry_delete(
            self.device, self.vlan_hdl, self.mac)
        assert (status == 0)

    def mac_update(self, handle):
        self.handle = handle
        status = self.client.switch_api_mac_table_entry_update(
            self.device, self.vlan_hdl, self.mac, self.mac_type, self.handle)


class Lag:
    def __init__(self, client, device):
        self.client = client
        self.device = device
        self.lag_hdl = None
        self.lag_hdl = self.client.switch_api_lag_create(self.device)
        assert (self.lag_hdl != 0)

    def lag_create(self):
        self.lag_hdl = self.client.switch_api_lag_create(self.device)
        assert (self.lag_hdl != 0)

    def lag_delete(self):
        status = self.client.switch_api_lag_delete(self.device, self.lag_hdl)
        assert (status == 0)

    def member_add(self, port_list):
        for port in port_list:
            status = self.client.switch_api_lag_member_add(
                self.device, self.lag_hdl, side=0, port=port)
            assert (status == 0)

    def member_delete(self, port_list):
        for port in port_list:
            status = self.client.switch_api_lag_member_delete(
                self.device, self.lag_hdl, side=0, port=port)
            assert (status == 0)

    def lag_hdl(self):
        return self.lag_hdl


class Stp:
    def __init__(self, client, device, mode):
        self.client = client
        self.device = device
        self.mode = mode
        self.stp_hdl = self.client.switch_api_stp_group_create(
            self.device, stp_mode=self.mode)
        assert (self.stp_hdl != 0)

    def group_create(self):
        self.stp_hdl = self.client.switch_api_stp_group_create(
            self.device, stp_mode=self.mode)
        assert (self.stp_hdl != 0)

    def group_delete(self):
        status = self.client.switch_api_stp_group_delete(self.device,
                                                         self.stp_hdl)
        assert (status == 0)

    def vlan_add(self, vlan_list):
        status = self.client.switch_api_stp_group_vlans_add(
            self.device, self.stp_hdl, len(vlan_list), vlan_list)
        assert (status == 0)

    def vlan_remove(self, vlan_list):
        status = self.client.switch_api_stp_group_vlans_remove(
            self.device, self.stp_hdl, len(vlan_list), vlan_list)
        assert (status == 0)

    def port_state_set(self, if_hdl, stp_state):
        status = self.client.switch_api_stp_port_state_set(
            self.device, self.stp_hdl, if_hdl, stp_state)
        assert (status == 0)


def macincrement(mac):
    mac = str(hex(int(mac.replace(':', ''), 16) + 1))[2:]
    mac = "00" + ":" + mac[0:2] + ":" + mac[2:4] + ":" + \
          mac[4:6] + ":" + mac[6:8] + ":" + mac[8:10]
    return mac
