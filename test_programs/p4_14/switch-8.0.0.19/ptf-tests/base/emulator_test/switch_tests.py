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
Thrift PD interface basic tests
"""

import time
import sys
import logging

import unittest
import random

import pd_base_tests

from ptf.testutils import *
from ptf.thriftutils import *

import os

from switch.p4_pd_rpc.ttypes import *

from res_pd_rpc.ttypes import *
from mc_pd_rpc.ttypes import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '..'))
from common.utils import *
from common.pd_utils import *

#global defaults
inner_rmac_group = 1
outer_rmac_group = 2
smac_index = 1
rewrite_index = 1
vrf = 1
rmac = '00:33:33:33:33:33'

g_num_pipes = 1
g_start_mcidx = 2000
g_flood_mcidx = 5000
g_chan_per_port = 1

g_fabric_mcast_device_id = 127
g_fabric_mgid = 3333
g_unicast_fabric_tunnel_rewrite_index = 66
g_multicast_fabric_tunnel_rewrite_index = 77
vlan1 = 10
vlan1_uuc_mc_index = 12345

#Enable features based on p4src/p4feature.h
tunnel_enabled = 1
mc_tunnel_enabled = 0
ipv6_enabled = 1
acl_enabled = 1
multicast_enabled = 1
if test_param_get('target') == "bmv2":
    stats_enabled = 1
    int_enabled = 1
else:
    stats_enabled = 0
    int_enabled = 0
learn_timeout = 6


@group("emulator_test")
class EmulatorTest(pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        if test_param_get('target') == "bmv2":
            pd_base_tests.ThriftInterfaceDataPlane.__init__(self, [""], ["dc"])
        else:
            pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                            ["dc"])

    def setUp(self):
        print
        print 'Configuring the devices'

        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.sess_hdl = self.conn_mgr.client_init()
        self.dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))
        self.device = 0

        client_init(self.client, self.sess_hdl, self.dev_tgt)

        populate_default_entries(self.client, self.sess_hdl, self.dev_tgt,
                                 ipv6_enabled, acl_enabled, tunnel_enabled,
                                 mc_tunnel_enabled, multicast_enabled, int_enabled)
        self.ret_init = populate_init_entries(
            self.client, self.sess_hdl, self.dev_tgt, rewrite_index, rmac,
            inner_rmac_group, outer_rmac_group, ipv6_enabled, tunnel_enabled)

        j = 0
        portno = []
        for pipe in range(0, g_num_pipes):
            for port in range(0, 72):
                j += 1
                portno.append(j)

        # create ports
        self.ret_list = program_emulation_ports(self.client, self.sess_hdl,
                                                self.dev_tgt, j)

        # untagged vlan
        self.vlan = 5
        # tagged vlans
        self.trunk_vlans = [2, 10, 36, 70, 85, 101, 999, 1000, 2001, 3002, 4095]
        v4_enabled = 0
        v6_enabled = 0
        mgid = g_flood_mcidx
        rid = 0x200
        xid = portno[1]

        self.vlan_hdl = program_bd(self.client, self.sess_hdl, self.dev_tgt,
                                   self.vlan, mgid)

        self.trunk_vlan_hdl = {}
        self.trunk_vlan_ehdl = {}
        for i in self.trunk_vlans:
            self.trunk_vlan_hdl[i] = program_bd(self.client, self.sess_hdl,
                                                self.dev_tgt, i, mgid)
            self.trunk_vlan_ehdl[i] = \
                program_egress_bd_properties(self.client, self.sess_hdl,self.dev_tgt, i, rewrite_index)

        self.mc_sess_hdl = self.mc.mc_create_session()
        init_pre(self.mc, self.mc_sess_hdl, g_num_pipes, g_start_mcidx,
                 g_chan_per_port, g_flood_mcidx)
        #init_mac_table(self.client, self.sess_hdl)
        #init_qos_table(self.client, self.sess_hdl)

        self.hdl = {}
        self.vhdl = {}
        self.xlate_hdl = {}
        self.mbr_hdl = {}
        self.vmbr_hdl = {}
        self.dhdl = {}
        self.shdl = {}
        self.mc_dmac_hdl = {}
        for pipe in range(0, g_num_pipes):
            for port in range(0, 72):
                asic_port = pipe_port_to_asic_port(pipe, port)
                self.hdl[asic_port], self.mbr_hdl[asic_port] = \
                            program_vlan_mapping(self.client, self.sess_hdl,self.dev_tgt, vrf, self.vlan, asic_port,v4_enabled, v6_enabled,0, 0, ctag=None, stag=None, rid=0)

                mac_addr = '00:00:00:01:' + hex(pipe)[2:].zfill(2)
                mac_addr = mac_addr + ':' + hex(port)[2:].zfill(2)
                self.dhdl[asic_port], self.shdl[asic_port] = \
                            program_mac(self.client, self.sess_hdl,self.dev_tgt, self.vlan,mac_addr, asic_port+1)

                mac_addr = '01:00:5e:00:' + hex(pipe)[2:].zfill(2)
                mac_addr = mac_addr + ':' + hex(port)[2:].zfill(2)
                self.mc_dmac_hdl[asic_port] = \
                            program_multicast_mac(self.client, self.sess_hdl,self.dev_tgt, self.vlan,mac_addr, asic_port,g_start_mcidx)
                for i in self.trunk_vlans:
                    self.vhdl[asic_port], self.vmbr_hdl[asic_port] = \
                        program_vlan_mapping(self.client, self.sess_hdl,self.dev_tgt, vrf, i, asic_port,v4_enabled, v6_enabled, 0, 0,ctag=None, stag=i, rid=0)
                    self.xlate_hdl[asic_port] = \
                        program_egress_vlan_xlate(self.client, self.sess_hdl,self.dev_tgt, asic_port,i, ctag=i, stag=None)
                    mac_addr = '0000' + hex(i)[2:].zfill(4) + hex(pipe)[
                        2:].zfill(2)
                    mac_addr = ':'.join(
                        s.encode('hex') for s in mac_addr.decode('hex'))
                    mac_addr = mac_addr + ':' + hex(port)[2:].zfill(2)
                    self.dhdl[asic_port], self.shdl[asic_port] = \
                            program_mac(self.client, self.sess_hdl,self.dev_tgt, i,mac_addr, asic_port+1)
                    mac_addr = '01005e' + hex(i)[2:].zfill(3) + \
                                hex(pipe)[2:].zfill(1) + hex(port)[2:].zfill(2)
                    mac_addr = ':'.join(
                        s.encode('hex') for s in mac_addr.decode('hex'))
                    self.mc_dmac_hdl[asic_port] = \
                            program_multicast_mac(self.client, self.sess_hdl,self.dev_tgt, i,mac_addr, asic_port,g_start_mcidx)

        for port in range(0, 60):
            for j in range(2, 900):
                # Add L3 routes
                addr = '0x0aa' + hex(port)[2:].zfill(2) + hex(j)[2:].zfill(3)
                program_ipv4_route(self.client, self.sess_hdl, self.dev_tgt,
                                   vrf, int(addr, 16), 32, port + 1)

            for i in range(1, 892):
                # Program mac entries
                mac_addr = '0000aa' + hex(i)[2:].zfill(4) + \
                            hex(port)[2:].zfill(2)
                mac_addr = ':'.join(
                    s.encode('hex') for s in mac_addr.decode('hex'))
                mac_programmed = program_mac(self.client, self.sess_hdl,
                                             self.dev_tgt, i, mac_addr,
                                             port + 1)

        self.mc.mc_complete_operations(self.mc_sess_hdl)
        self.conn_mgr.complete_operations(self.sess_hdl)

    def runTest(self):
        pass
        """
        print
        print 'Running test'
        for pipe in range(0,g_num_pipes):
            for port in range(0,8,4):

                exp_asic_port = pipe_port_to_asic_port(pipe, port)

                if exp_asic_port == 0:
                    continue

                # unicast packet
                print "Untagged unicast packet test"
                mac_addr = '00:00:00:01:' + hex(pipe)[2:].zfill(2)
                mac_addr = mac_addr + ':' + hex(port)[2:].zfill(2)
                pkt = simple_tcp_packet(eth_dst=mac_addr,eth_src='00:00:00:00:01:01',ip_dst='10.10.3.3',ip_src='10.10.1.1',ip_id=105,ip_ttl=4)
                exp_pkt = simple_tcp_packet(eth_dst=mac_addr,eth_src='00:00:00:00:01:01',ip_dst='10.10.3.3',ip_src='10.10.1.1',ip_id=105,ip_ttl=4)
                send_packet(self, 0, str(pkt))
                verify_packets(self, exp_pkt, [exp_asic_port])

                # multicast packet
                print "Untagged multicast packet test"
                mac_addr = '01:00:5e:00:' + hex(pipe)[2:].zfill(2)
                mac_addr = mac_addr + ':' + hex(port)[2:].zfill(2)
                pkt = simple_tcp_packet(eth_dst=mac_addr,eth_src='00:00:00:00:01:01',ip_dst='10.10.3.3',ip_src='10.10.1.1',ip_id=105,ip_ttl=4)
                exp_pkt = simple_tcp_packet(eth_dst=mac_addr,eth_src='00:00:00:00:01:01',ip_dst='10.10.3.3',ip_src='10.10.1.1',ip_id=105,ip_ttl=4)
                send_packet(self, 0, str(pkt))
                verify_packets(self, exp_pkt, [exp_asic_port])

                # tagged packets
                for i in self.trunk_vlans:
                    print "Tagged unicast packet vlan %d" % i
                    mac_addr = '0000' + hex(i)[2:].zfill(4) + hex(pipe)[2:].zfill(2)
                    mac_addr = ':'.join(s.encode('hex') for s in
                               mac_addr.decode('hex'))
                    mac_addr = mac_addr + ':' + hex(port)[2:].zfill(2)
                    src_mac_addr = '0000' + hex(i)[2:].zfill(4) + \
                                   hex(pipe)[2:].zfill(2) + '00'
                    src_mac_addr = ':'.join(s.encode('hex') for s in
                               src_mac_addr.decode('hex'))

                    pkt = simple_tcp_packet(eth_dst=mac_addr,eth_src=src_mac_addr,ip_dst='10.10.3.3',dl_vlan_enable=True,vlan_vid=i,ip_id=102,ip_ttl=64)
                    exp_pkt = simple_tcp_packet(eth_dst=mac_addr,eth_src=src_mac_addr,ip_dst='10.10.3.3',dl_vlan_enable=True,vlan_vid=i,ip_id=102,ip_ttl=64)
                    send_packet(self, 0, str(pkt))
                    verify_packets(self, exp_pkt, [exp_asic_port])

                    print "Tagged multicast packet vlan %d" % i
                    mac_addr = '01005e' + hex(i)[2:].zfill(3) + \
                                hex(pipe)[2:].zfill(1) + hex(port)[2:].zfill(2)
                    mac_addr = ':'.join(s.encode('hex') for s in
                               mac_addr.decode('hex'))
                    pkt = simple_tcp_packet(eth_dst=mac_addr,eth_src='00:00:00:00:01:01',ip_dst='10.10.3.3',ip_src='10.10.1.1',dl_vlan_enable=True,vlan_vid=i,ip_id=105,ip_ttl=4)
                    exp_pkt = simple_tcp_packet(eth_dst=mac_addr,eth_src='00:00:00:00:01:01',ip_dst='10.10.3.3',ip_src='10.10.1.1',dl_vlan_enable=True,vlan_vid=i,ip_id=105,ip_ttl=4)
                    send_packet(self, 0, str(pkt))
                    verify_packets(self, exp_pkt, [exp_asic_port])
        """

    def tearDown(self):

        pass
        """

        delete_default_entries(self.client, self.sess_hdl, self.device)
        for pipe in range(0,g_num_pipes):
            for port in range(0,72):
                asic_port = pipe_port_to_asic_port(pipe, port)
                delete_mac(self.client, self.sess_hdl, self.device,self.dhdl[asic_port], self.shdl[asic_port])
                delete_dmac(self.client, self.sess_hdl, self.device,self.mc_dmac_hdl[asic_port])

                delete_vlan_mapping(self.client, self.sess_hdl, self.device,self.hdl[asic_port], self.mbr_hdl[asic_port])

        delete_bd(self.client, self.sess_hdl, self.device, self.vlan_hdl)
        for i in self.trunk_vlans:
            delete_bd(self.client, self.sess_hdl, self.device,self.trunk_vlan_hdl[i])

        # delete ports
        delete_ports(self.client, self.sess_hdl, self.device, 2, self.ret_list)

        # delete  init and default entries
        delete_init_entries(self.client, self.sess_hdl, self.device,self.ret_init, tunnel_enabled)

        self.mc.mc_destroy_session(self.mc_sess_hdl)
        self.conn_mgr.complete_operations(self.sess_hdl)
        self.conn_mgr.client_cleanup(self.sess_hdl)
        """
