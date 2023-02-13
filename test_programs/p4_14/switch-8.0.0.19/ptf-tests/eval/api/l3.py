################################################################################
# BAREFOOT NETWORKS CONFIDENTIAL & PROPRIETARY
#
# Copyright (c) 2017-present Barefoot Networks, Inc.
#
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

import json
import os
import sys
import unittest
import ptf.dataplane as dataplane
import pd_base_tests

from port_mapping import *
from ptf.testutils import *
from ptf.thriftutils import *

import switchapi_thrift
from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../base'))
from common.utils import *
sys.path.append(os.path.join(this_dir, '../../base/api-tests'))
import api_base_tests

frontPanelPorts = [
    "1/0", "1/1", "1/2", "1/3", "2/0", "2/1", "3/0", "4/0", "5/0"
]

################################################################################
# This script uses switchAPI thrift interface to demonstrate L3 routing over
# ECMP. The ECMP path contains both regular ports and LAGs. It uses 9 ports.
# The ports are specified in list "frontPanelPorts". Ports 0 thru 7 are egress
# ports and port 8 is the ingress port. Ports 0 thru 3 are member ports of a
# LAG. Likewise ports 4 and 5 are members of another LAG. See figure below for
# how the ports are configured.
#
#                     +--- frontPanelPorts[0], 10G
#                     |
#                     +--- frontPanelPorts[1], 10G
#          +-- LAG1 --|
#          |          +--- frontPanelPorts[2], 10G
#          |          |
#          |          +--- frontPanelPorts[3], 10G
#    ECMP--+
#          |          +--- frontPanelPorts[4], 10G
#          +-- LAG2 --|
#          |          +--- frontPanelPorts[5], 10G
#          |
#          +-------------- frontPanelPorts[6], 100G
#          |
#          +-------------- frontPanelPorts[7], 100G
#
# Traffic ingress port : frontPanelPorts[8], 10G
#
# The system creates a VRF 2
#   ip vrf 2
#
# The device's MAC address is configured as 00:77:66:55:44:33
#
# First, the script configures 5 L3 interfaces.
#   ip address add 10.1.1.1/24 dev lag1
#   ip address add 10.1.2.1/24 dev lag2
#   ip address add 10.1.3.1/24 dev port6
#   ip address add 10.1.4.1/24 dev port7
#   ip address add 10.1.5.1/24 dev port8
#
# Next, 5 neighbor entries are added, one on each interface.
#   ip neigh add 10.1.1.2 lladdr 00:11:11:11:11:11 dev lag1
#   ip neigh add 10.1.2.2 lladdr 00:11:11:11:11:22 dev lag2
#   ip neigh add 10.1.3.2 lladdr 00:11:11:11:11:33 dev port6
#   ip neigh add 10.1.4.2 lladdr 00:11:11:11:11:44 dev port7
#   ip neigh add 10.1.5.2 lladdr 00:11:11:11:11:55 dev port8
#
# Finally, 3 routes are added with ECMP nexthop containing 4 neighbors.
#   ip route add 10.100.0.0/16
#      nexthop via 10.1.1.2
#      nexthop via 10.1.2.2
#      nexthop via 10.1.3.2
#      nexthop via 10.1.4.2
#   ip route add 10.101.0.0/16
#      nexthop via 10.1.1.2
#      nexthop via 10.1.2.2
#      nexthop via 10.1.3.2
#      nexthop via 10.1.4.2
#   ip route add 10.102.0.0/16
#      nexthop via 10.1.1.2
#      nexthop via 10.1.2.2
#      nexthop via 10.1.3.2
#      nexthop via 10.1.4.2
#
# To verify the configuration, the following traffic is sent on the input port.
#     Input port               : frontPanelPort[8]
#     MAC source               : 00:11:11:11:11:55
#     MAC destination addresss : 00:77:66:55:44:33
#     IP source address        : 192.168.0.0/16 subnet
#     IP destination address   : 10.100.0.0/16 subnet
#
# Packets are expected on frontPanelPorts 0 thru 7 with the following weights:
#     frontPanelPort[0] : 1/16
#     frontPanelPort[1] : 1/16
#     frontPanelPort[2] : 1/16
#     frontPanelPort[3] : 1/16
#     frontPanelPort[4] : 1/8
#     frontPanelPort[5] : 1/8
#     frontPanelPort[6] : 1/4
#     frontPanelPort[7] : 1/4
################################################################################


class L3EcmpLagTest(api_base_tests.ThriftInterfaceDataPlane):
    def frontpanel_to_swport(self, fpport):
        pgrp, chnl = fpport.split("/")
        swport = (int(pgrp) * 4 - 4) + int(chnl)
        return swport

    def swport_to_frontpanel(self, swport):
        pgrp = swport / 4 + 1
        chnl = swport % 4
        fpport = "%d/%d" % (pgrp, chnl)
        return fpport

    def getSwPorts(self):
        swapi_ports = []
        for i in frontPanelPorts:
            swapi_ports.append(self.frontpanel_to_swport(i))
        return swapi_ports

    def getFpPort(self, swport):
        return self.swport_to_frontpanel(swport)

    def deletePort(self, swport):
        handle = self.client.switch_api_port_id_to_handle_get(self.DEVICE,
                                                              swport)
        self.client.switch_api_port_delete(self.DEVICE, handle)

    def deleteAllPortsInPortGroup(self, fpport):
        pgrp, chnl = fpport.split("/")
        for i in range(0,4):
            swport = (int(pgrp) * 4 - 4) + i
            self.deletePort(swport)

    def addPort(self, swport, speed):
        portinfo = switcht_api_port_info_t(swport, speed)
        self.client.switch_api_port_add_with_attribute(self.DEVICE, portinfo)

    def setUp(self):
        print

        self.swports = self.getSwPorts()

        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        api = self.client
        self.DEVICE = 0
        self.VRF = 2
        self.RMAC = '00:77:66:55:44:33'

        # my interface addresses
        self.IFADDRS = [{
            'IP': '10.1.1.1',
            'LEN': 24
        }, {
            'IP': '10.1.2.1',
            'LEN': 24
        }, {
            'IP': '10.1.3.1',
            'LEN': 24
        }, {
            'IP': '10.1.4.1',
            'LEN': 24
        }, {
            'IP': '10.1.5.1',
            'LEN': 24
        }]

        # my neighbor's IP and MAC addresses
        self.NEIGHBORS = [{
            'IP': '10.1.1.2',
            'MAC': '00:11:11:11:11:11'
        }, {
            'IP': '10.1.2.2',
            'MAC': '00:11:11:11:11:22'
        }, {
            'IP': '10.1.3.2',
            'MAC': '00:11:11:11:11:33'
        }, {
            'IP': '10.1.4.2',
            'MAC': '00:11:11:11:11:44'
        }, {
            'IP': '10.1.5.2',
            'MAC': '00:11:11:11:11:55'
        }]

        # routes to program
        self.ROUTES = [{
            'IP': '10.100.0.0',
            'LEN': 16
        }, {
            'IP': '10.101.0.0',
            'LEN': 16
        }, {
            'IP': '10.102.0.0',
            'LEN': 16
        }]

        self.port_hdl = [None] * 9
        self.if_hdl = [None] * 5
        self.nhop_hdl = [None] * 5
        self.neigh_hdl = [None] * 5
        self.rif_hdl = [None] * 5

        # initialize API

        if test_param_get('setup') == True or (
                test_param_get('setup') != True and
                test_param_get('cleanup') != True):

            print 'Configuring the system'

            # create VRF
            self.vrf_hdl = api.switch_api_vrf_create(self.DEVICE, self.VRF)

            # add router MAC
            self.rmac_hdl = api.switch_api_router_mac_group_create(
                self.DEVICE, SWITCH_RMAC_TYPE_INNER)
            api.switch_api_router_mac_add(self.DEVICE, self.rmac_hdl, self.RMAC)

            # add 9 ports
            self.port_hdl[0] = api.switch_api_port_id_to_handle_get(
                self.DEVICE, self.swports[0])
            self.port_hdl[1] = api.switch_api_port_id_to_handle_get(
                self.DEVICE, self.swports[1])
            self.port_hdl[2] = api.switch_api_port_id_to_handle_get(
                self.DEVICE, self.swports[2])
            self.port_hdl[3] = api.switch_api_port_id_to_handle_get(
                self.DEVICE, self.swports[3])
            self.port_hdl[4] = api.switch_api_port_id_to_handle_get(
                self.DEVICE, self.swports[4])
            self.port_hdl[5] = api.switch_api_port_id_to_handle_get(
                self.DEVICE, self.swports[5])
            # ports 6 and 7 are 100G ports, delete the 10G ports in the port
            # group
            # created by default by switchapi and add the 100G port
            self.deleteAllPortsInPortGroup(frontPanelPorts[6])
            self.deleteAllPortsInPortGroup(frontPanelPorts[7])
            self.addPort(self.swports[6], SWITCH_PORT_SPEED_100G)
            self.addPort(self.swports[7], SWITCH_PORT_SPEED_100G)

            self.port_hdl[6] = api.switch_api_port_id_to_handle_get(
                self.DEVICE, self.swports[6])
            self.port_hdl[7] = api.switch_api_port_id_to_handle_get(
                self.DEVICE, self.swports[7])
            self.port_hdl[8] = api.switch_api_port_id_to_handle_get(
                self.DEVICE, self.swports[8])

            # wait for the ports to come up
            time.sleep(20)

            # create lags
            self.lag1_hdl = api.switch_api_lag_create(self.DEVICE)
            self.lag2_hdl = api.switch_api_lag_create(self.DEVICE)

            # add port_hdl[0] thru port_hdl[3] to lag1
            for idx in range(0, 4):
                api.switch_api_lag_member_add(
                    self.DEVICE,
                    lag_handle=self.lag1_hdl,
                    side=SWITCH_API_DIRECTION_BOTH,
                    port=self.port_hdl[idx])

            # add port_hdl[4] and port_hdl[5] to lag2
            for idx in range(4, 6):
                api.switch_api_lag_member_add(
                    self.DEVICE,
                    lag_handle=self.lag2_hdl,
                    side=SWITCH_API_DIRECTION_BOTH,
                    port=self.port_hdl[idx])

            rif_info = switcht_rif_info_t(
                rif_type=SWITCH_RIF_TYPE_INTF,
                vrf_handle=self.vrf_hdl,
                rmac_handle=self.rmac_hdl,
                v4_unicast_enabled=True,
                v6_unicast_enabled=True)
            self.rif_hdl[0] = self.client.switch_api_rif_create(0, rif_info)
            self.rif_hdl[1] = self.client.switch_api_rif_create(0, rif_info)
            self.rif_hdl[2] = self.client.switch_api_rif_create(0, rif_info)
            self.rif_hdl[3] = self.client.switch_api_rif_create(0, rif_info)
            self.rif_hdl[4] = self.client.switch_api_rif_create(0, rif_info)

            # create L3 interface on lag1
            intf_info = switcht_interface_info_t(
                handle=self.lag1_hdl,
                type=SWITCH_INTERFACE_TYPE_PORT,
                rif_handle=self.rif_hdl[0])
            self.if_hdl[0] = api.switch_api_interface_create(self.DEVICE,
                                                             intf_info)

            # create L3 interface on lag2
            intf_info = switcht_interface_info_t(
                handle=self.lag2_hdl,
                type=SWITCH_INTERFACE_TYPE_PORT,
                rif_handle=self.rif_hdl[1])
            self.if_hdl[1] = api.switch_api_interface_create(self.DEVICE,
                                                             intf_info)

            # create L3 interface on port_hdl[6]
            intf_info = switcht_interface_info_t(
                handle=self.port_hdl[6],
                type=SWITCH_INTERFACE_TYPE_PORT,
                rif_handle=self.rif_hdl[2])
            self.if_hdl[2] = api.switch_api_interface_create(self.DEVICE,
                                                             intf_info)

            # create L3 interface on port_hdl[7]
            intf_info = switcht_interface_info_t(
                handle=self.port_hdl[7],
                type=SWITCH_INTERFACE_TYPE_PORT,
                rif_handle=self.rif_hdl[3])
            self.if_hdl[3] = api.switch_api_interface_create(self.DEVICE,
                                                             intf_info)

            # create L3 interface on port_hdl[8], used as input interface
            intf_info = switcht_interface_info_t(
                handle=self.port_hdl[8],
                type=SWITCH_INTERFACE_TYPE_PORT,
                rif_handle=self.rif_hdl[4])
            self.if_hdl[4] = api.switch_api_interface_create(self.DEVICE,
                                                             intf_info)

            # configure addresses on the L3 interfaces
            for idx in range(0, len(self.if_hdl)):
                ipaddr = switcht_ip_addr_t(
                    addr_type=SWITCH_API_IP_ADDR_V4,
                    ipaddr=self.IFADDRS[idx]['IP'],
                    prefix_length=self.IFADDRS[idx]['LEN'])
                api.switch_api_l3_interface_address_add(
                    self.DEVICE, self.rif_hdl[idx], self.vrf_hdl, ipaddr)

            # create neighbors
            for idx in range(0, len(self.NEIGHBORS)):
                ipaddr = switcht_ip_addr_t(
                    addr_type=SWITCH_API_IP_ADDR_V4,
                    ipaddr=self.NEIGHBORS[idx]['IP'],
                    prefix_length=32)
                self.nhop_hdl[idx], self.neigh_hdl[idx] =
                switch_api_l3_nhop_neighbor_create(
                        self,
                        device,
                        self.rif_hdl[idx],
                        ipaddr,
                        self.NEIGHBORS[idx]['MAC'])

            # create ecmp group
            self.ecmp_hdl = api.switch_api_ecmp_create(self.DEVICE)

            # add nexthops to ecmp group
            nhop_list = [self.nhop_hdl[n] for n in range(0, 4)]
            api.switch_api_ecmp_member_add(self.DEVICE, self.ecmp_hdl,
                                           len(nhop_list), nhop_list)

            # add routes with destination as ecmp handle created above
            for idx in range(0, len(self.ROUTES)):
                ipaddr = switcht_ip_addr_t(
                    addr_type=SWITCH_API_IP_ADDR_V4,
                    ipaddr=self.ROUTES[idx]['IP'],
                    prefix_length=self.ROUTES[idx]['LEN'])
                api.switch_api_l3_route_add(self.DEVICE, self.vrf_hdl, ipaddr,
                                            self.ecmp_hdl)

    def runTest(self):
        if test_param_get('setup') != True and \
            test_param_get('cleanup') != True:

            print 'Verifying configuration by sending packets'
            count = [0] * 8
            dst_ip = int(socket.inet_aton('10.100.1.1').encode('hex'), 16)
            src_ip = int(socket.inet_aton('192.168.8.1').encode('hex'), 16)
            max_itrs = 500
            for i in range(0, max_itrs):
                # ingress packet
                dst_ip_addr = \
                    socket.inet_ntoa(format(dst_ip, 'x').zfill(8).decode('hex'))
                src_ip_addr = \
                    socket.inet_ntoa(format(src_ip, 'x').zfill(8).decode('hex'))
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:11:11:11:11:55',
                    ip_dst=dst_ip_addr,
                    ip_src=src_ip_addr,
                    ip_ttl=64)

                # egress packets
                exp_pkt1 = simple_tcp_packet(
                    eth_dst='00:11:11:11:11:11',
                    eth_src='00:77:66:55:44:33',
                    ip_dst=dst_ip_addr,
                    ip_src=src_ip_addr,
                    ip_ttl=63)
                exp_pkt2 = simple_tcp_packet(
                    eth_dst='00:11:11:11:11:22',
                    eth_src='00:77:66:55:44:33',
                    ip_dst=dst_ip_addr,
                    ip_src=src_ip_addr,
                    ip_ttl=63)
                exp_pkt3 = simple_tcp_packet(
                    eth_dst='00:11:11:11:11:33',
                    eth_src='00:77:66:55:44:33',
                    ip_dst=dst_ip_addr,
                    ip_src=src_ip_addr,
                    ip_ttl=63)
                exp_pkt4 = simple_tcp_packet(
                    eth_dst='00:11:11:11:11:44',
                    eth_src='00:77:66:55:44:33',
                    ip_dst=dst_ip_addr,
                    ip_src=src_ip_addr,
                    ip_ttl=63)

                send_packet(self, self.swports[8], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self, [exp_pkt1, exp_pkt2, exp_pkt3, exp_pkt4], [
                        self.swports[0], self.swports[1], self.swports[2],
                        self.swports[3], self.swports[4], self.swports[5],
                        self.swports[6], self.swports[7]
                    ])
                count[rcv_idx] += 1
                dst_ip += 1
                src_ip += 1

            print 'Packets per port: ', count
