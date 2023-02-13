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

import switchsai_thrift

import time
import sys
import logging

import unittest
import random

import ptf.dataplane as dataplane

from ptf.testutils import *
from ptf.thriftutils import *

import os
from switchsai_thrift.ttypes import *
from switchsai_thrift.sai_headers import *

from erspan3 import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../base'))
from common.utils import *
from common.sai_utils import *
sys.path.append(os.path.join(this_dir, '../../base/sai-tests'))
import sai_base_test

swports = [x for x in range(0, 16, 1)]

cpu_port = 64
switch_inited = 0
port_list = []
table_attr_list = []


def switch_init(client):
    global switch_inited
    if switch_inited:
        return

    switch_attr_list = client.sai_thrift_get_switch_attribute()
    attr_list = switch_attr_list.attr_list
    for attribute in attr_list:
        if attribute.id == SAI_SWITCH_ATTR_PORT_NUMBER:
            print "max ports: " + str(attribute.value.u32)
        elif attribute.id == SAI_SWITCH_ATTR_PORT_LIST:
            for x in attribute.value.objlist.object_id_list:
                port_list.append(x)
        else:
            print "unknown switch attribute"

    attr_value = sai_thrift_attribute_value_t(mac='00:77:66:55:44:33')
    attr = sai_thrift_attribute_t(
        id=SAI_SWITCH_ATTR_SRC_MAC_ADDRESS, value=attr_value)
    client.sai_thrift_set_switch_attribute(attr)
    switch_inited = 1


################################################################################
@group('egress_acl')
@group('ent')
class IPEgressAclTest(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):

        print "Sending packet port 1 -> port 2 \
	    (192.168.0.1 -> 10.10.10.1 [id = 101])"

        switch_init(self.client)
        port1 = port_list[1]
        port2 = port_list[2]
        v4_enabled = 1
        v6_enabled = 1
        mac = ''

        vr_id = sai_thrift_create_virtual_router(self.client, v4_enabled,
                                                 v6_enabled)
        self.assertFalse(vr_id == 0)
        rif_id1 = sai_thrift_create_router_interface(
            self.client, SAI_ROUTER_INTERFACE_TYPE_PORT, vr_id, port1, 0,
            v4_enabled, v6_enabled, mac)
        rif_id2 = sai_thrift_create_router_interface(
            self.client, SAI_ROUTER_INTERFACE_TYPE_PORT, vr_id, port2, 0,
            v4_enabled, v6_enabled, mac)

        addr_family = SAI_IP_ADDR_FAMILY_IPV4
        ip_addr1 = '10.10.10.1'
        ip_mask1 = '255.255.255.255'
        dmac1 = '00:11:22:33:44:55'
        nhop1 = sai_thrift_create_nhop(self.client, addr_family, ip_addr1,
                                       rif_id1)
        sai_thrift_create_neighbor(self.client, addr_family, rif_id1, ip_addr1,
                                   dmac1)

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
        if True:

            # setup ACL to block based on Source IP
            action_list = [SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION]
            packet_action = SAI_PACKET_ACTION_DROP
            in_ports = [port1, port2]
            ip_src = "192.168.0.1"
            ip_src_mask = "255.255.255.0"

            acl_table_id = sai_thrift_create_acl_table(
                client=self.client, acl_stage=SAI_ACL_STAGE_EGRESS, ip_src=True)

            for port in in_ports:
                sai_thrift_set_port_attribute(
                    self.client, port, SAI_PORT_ATTR_EGRESS_ACL, acl_table_id)

            acl_entry_id = sai_thrift_create_acl_entry(
                client=self.client,
                acl_table_id=acl_table_id,
                action_list=action_list,
                packet_action=packet_action,
                ip_src=ip_src,
                ip_src_mask=ip_src_mask)

            # send the same packet
            failed = 0
            send_packet(self, 2, str(pkt))

            # ensure packet is dropped
            # check for absence of packet here!
            try:
                verify_packets(self, exp_pkt, [1])
                print 'FAILED - did not expect packet'
                failed = 1
            except:
                print 'Success'

            finally:
                if failed == 1:
                    fail_flag = True
                    self.assertFalse(fail_flag, 'Failed did not expect pkt')

            for port in in_ports:
                sai_thrift_set_port_attribute(
                    self.client, port, SAI_PORT_ATTR_EGRESS_ACL, SAI_NULL_OBJECT_ID)

            # delete ACL
            self.client.sai_thrift_remove_acl_entry(acl_entry_id)
            self.client.sai_thrift_remove_acl_table(acl_table_id)

            # cleanup
            sai_thrift_remove_neighbor(self.client, addr_family, rif_id1,
                                       ip_addr1, dmac1)
            self.client.sai_thrift_remove_next_hop(nhop1)

            self.client.sai_thrift_remove_router_interface(rif_id1)
            self.client.sai_thrift_remove_router_interface(rif_id2)

            self.client.sai_thrift_remove_virtual_router(vr_id)
