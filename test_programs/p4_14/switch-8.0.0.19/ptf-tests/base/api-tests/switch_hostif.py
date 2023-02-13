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
IP multicast tests
"""

import switchapi_thrift

import os
import time
import sys
import logging

import unittest
import random

import api_base_tests
import pd_base_tests
from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *
from ptf.mask import *

from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

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
@group('l3')
@group('l2')
@group('maxsizes')
@group('ent')
class HostIfRxTxTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        vlan = self.client.switch_api_vlan_create(device, 10)
        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_INNER)
        self.client.switch_api_router_mac_add(device, rmac, '00:11:22:33:44:55')
        self.cpu_port = get_cpu_port(self)

        port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])
        port2 = self.client.switch_api_port_id_to_handle_get(device, swports[2])
        port3 = self.client.switch_api_port_id_to_handle_get(device, swports[3])

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
        self.client.switch_api_l3_interface_address_add(device, rif1, vrf, i_ip1)

        i_info2 = switcht_interface_info_t(
            handle=port2, type=SWITCH_INTERFACE_TYPE_ACCESS)
        if2 = self.client.switch_api_interface_create(device, i_info2)

        i_info3 = switcht_interface_info_t(
            handle=port3, type=SWITCH_INTERFACE_TYPE_TRUNK)
        if3 = self.client.switch_api_interface_create(device, i_info3)

        self.client.switch_api_vlan_member_add(device, vlan, if2)
        self.client.switch_api_vlan_member_add(device, vlan, if3)

        rif_info2 = switcht_rif_info_t(
            rif_type=SWITCH_RIF_TYPE_VLAN,
            vlan=10,
            vrf_handle=vrf,
            rmac_handle=rmac,
            v4_unicast_enabled=1)
        rif2 = self.client.switch_api_rif_create(0, rif_info2)
        i_ip2 = switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4,
            ipaddr='200.10.10.2',
            prefix_length=16)
        self.client.switch_api_l3_interface_address_add(device, rif2, vrf, i_ip2)

        cpu_port_handle = self.client.switch_api_port_id_to_handle_get(
                          device, self.cpu_port)
        queue_handles = self.client.switch_api_queues_get(device, cpu_port_handle)


        hostif_group1 = switcht_hostif_group_t(queue_handles[0], policer_handle=0)
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

        arp_resp_rcode_info = switcht_hostif_rcode_info_t(
                reason_code=SWITCH_HOSTIF_REASON_CODE_ARP_RESPONSE,
                action=SWITCH_ACL_ACTION_REDIRECT_TO_CPU,
                hostif_group_id=hostif_group_id1)
        rcode_handle2 = self.client.switch_api_hostif_reason_code_create(
                device, flags, arp_resp_rcode_info)

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
        hostif_id1 = self.client.switch_api_hostif_create(device, hostif_flags, hostif1)

        hostif_name2 = "rif2"
        hostif2 = switcht_hostif_t(
            intf_name=hostif_name2,
            handle=rif2,
            mac='00:11:22:33:44:55',
            v4addr=i_ip2,
            operstatus=True,
            admin_state=True)
        hostif_flags = 0
        hostif_flags |= SWITCH_HOSTIF_ATTR_INTERFACE_NAME
        hostif_flags |= SWITCH_HOSTIF_ATTR_HANDLE
        hostif_flags |= SWITCH_HOSTIF_ATTR_MAC_ADDRESS
        hostif_flags |= SWITCH_HOSTIF_ATTR_IPV4_ADDRESS
        hostif_flags |= SWITCH_HOSTIF_ATTR_OPER_STATUS
        hostif_flags |= SWITCH_HOSTIF_ATTR_ADMIN_STATE
        hostif_id2 = self.client.switch_api_hostif_create(device, hostif_flags, hostif2)
        time.sleep(10)

        switch_api_mac_table_entry_create(
                self, device, vlan, '00:06:07;08:09:0b', 2, if2)
        try:
            pkt = simple_arp_packet(
                    arp_op=1,
                    pktlen=100,
                    ip_snd='192.168.0.1',
                    ip_tgt='192.168.0.2')
            exp_pkt = simple_arp_packet(
                    pktlen=42,
                    arp_op=2,
                    eth_src='00:11:22:33:44:55',
                    eth_dst='00:06:07:08:09:0a',
                    hw_snd='00:11:22:33:44:55',
                    hw_tgt='00:06:07:08:09:0a',
                    ip_snd='192.168.0.2',
                    ip_tgt='192.168.0.1')

            send_packet(self, swports[1], str(pkt))
            verify_packet(self, exp_pkt, swports[1])

            pkt = simple_icmp_packet(
                    eth_src='00:06:07:08:09:0a',
                    eth_dst='00:11:22:33:44:55',
                    ip_src='192.168.0.1',
                    ip_dst='192.168.0.2',
                    icmp_type=8,
                    icmp_data='000102030405')
            exp_pkt= simple_icmp_packet(
                    eth_src='00:11:22:33:44:55',
                    eth_dst='00:06:07:08:09:0a',
                    ip_src='192.168.0.2',
                    ip_dst='192.168.0.1',
                    icmp_type=0,
                    icmp_data='000102030405')

            send_packet(self, swports[1], str(pkt))
            m = Mask(exp_pkt)
            m.set_do_not_care_scapy(IP, 'id')
            m.set_do_not_care_scapy(IP, 'chksum')
            verify_packet(self, m, swports[1])

            pkt = simple_arp_packet(
                    arp_op=1,
                    pktlen=100,
                    hw_snd='00:06:07:08:09:0b',
                    eth_src='00:06:07:08:09:0b',
                    ip_snd='200.10.10.3',
                    ip_tgt='200.10.10.2')
            exp_pkt = simple_arp_packet(
                    pktlen=42,
                    arp_op=2,
                    eth_src='00:11:22:33:44:55',
                    eth_dst='00:06:07:08:09:0b',
                    hw_snd='00:11:22:33:44:55',
                    hw_tgt='00:06:07:08:09:0b',
                    ip_snd='200.10.10.2',
                    ip_tgt='200.10.10.3')

            send_packet(self, swports[2], str(pkt))
            verify_packet(self, exp_pkt, swports[2])

            pkt = simple_icmp_packet(
                    eth_src='00:06:07:08:09:0b',
                    eth_dst='00:11:22:33:44:55',
                    ip_src='200.10.10.3',
                    ip_dst='200.10.10.2',
                    icmp_type=8,
                    icmp_data='000102030405')
            exp_pkt = simple_icmp_packet(
                    eth_src='00:11:22:33:44:55',
                    eth_dst='00:06:07:08:09:0b',
                    ip_src='200.10.10.2',
                    ip_dst='200.10.10.3',
                    icmp_type=0,
                    icmp_data='000102030405')

            send_packet(self, swports[2], str(pkt))
            m = Mask(exp_pkt)
            m.set_do_not_care_scapy(IP, 'id')
            m.set_do_not_care_scapy(IP, 'chksum')
            verify_packet(self, m, swports[2])

        finally:
            switch_api_mac_table_entry_delete(self, device, vlan, '00:06:07:08:09:0b')
            self.client.switch_api_hostif_reason_code_delete(device, rcode_handle1)
            self.client.switch_api_hostif_reason_code_delete(device, rcode_handle2)

            self.client.switch_api_hostif_delete(device, hostif_id1)
            self.client.switch_api_hostif_delete(device, hostif_id2)

            self.client.switch_api_hostif_group_delete(device, hostif_group_id1)

            self.client.switch_api_l3_interface_address_delete(device, rif1, vrf, i_ip1)
            self.client.switch_api_l3_interface_address_delete(device, rif2, vrf, i_ip2)

            self.client.switch_api_vlan_member_remove(device, vlan, if2)
            self.client.switch_api_vlan_member_remove(device, vlan, if3)

            self.client.switch_api_interface_delete(device, if1)
            self.client.switch_api_interface_delete(device, if2)
            self.client.switch_api_interface_delete(device, if3)

            self.client.switch_api_rif_delete(0, rif1)
            self.client.switch_api_rif_delete(0, rif2)

            self.client.switch_api_router_mac_delete(device, rmac, '00:11:22:33:44:55')

            self.client.switch_api_vlan_delete(device, vlan)
            self.client.switch_api_vrf_delete(device, vrf)

###############################################################################

@group('ent')
@group('ptp')
class HostIfPtpTest( pd_base_tests.ThriftInterfaceDataPlane,
                     api_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
	print
	# this test is not valid when runing against a remote host.
	pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
	self.devport = []
	self.devport.append(swport_to_devport(self, swports[0]))
	self.devport.append(swport_to_devport(self, swports[1]))
	self.cpu_port = get_cpu_port(self)

	api_base_tests.ThriftInterfaceDataPlane.setUp(self)
	self.client.switch_api_init(device)

	if self.thrift_server != 'localhost':
	    return

	vrf = self.client.switch_api_vrf_create(device, 2)

	rmac = self.client.switch_api_router_mac_group_create(
	    device, SWITCH_RMAC_TYPE_INNER)
	self.client.switch_api_router_mac_add(device, rmac, '00:77:66:55:44:33')

	port1 = self.client.switch_api_port_id_to_handle_get(device, swports[1])

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
	self.client.switch_api_l3_interface_address_add(device, rif1, vrf,
							i_ip1)
	cpu_port_handle = self.client.switch_api_port_id_to_handle_get(
	    device, self.cpu_port)
	queue_handles = self.client.switch_api_queues_get(device, cpu_port_handle)
	hostif_group1 = switcht_hostif_group_t(queue_handles[0], policer_handle=0)
	hostif_group_id1 = self.client.switch_api_hostif_group_create(
	    device, hostif_group1)

	hostif_name = "test_host_if"
	hostif = switcht_hostif_t(
	    intf_name=hostif_name,
	    handle=port1,
	    operstatus=True)
	hostif_flags = 0
	hostif_flags |= SWITCH_HOSTIF_ATTR_INTERFACE_NAME
	hostif_flags |= SWITCH_HOSTIF_ATTR_HANDLE
	hostif_flags |= SWITCH_HOSTIF_ATTR_OPER_STATUS
	hostif_id = self.client.switch_api_hostif_create(device, hostif_flags, hostif)
	self.assertTrue(hostif_id != 0)
	hostif_table_entry_id = {}
	os.system("sudo ifconfig test_host_if up")
        ingress_ifindex = self.client.switch_api_interface_ifindex_get(
                device, if1)

	s = open_packet_socket(hostif_name)
	try:
	    print 'Installing hostif reason code for PTP'
	    flags = 0
	    flags |= SWITCH_HOSTIF_RCODE_ATTR_REASON_CODE
	    flags |= SWITCH_HOSTIF_RCODE_ATTR_PACKET_ACTION
	    flags |= SWITCH_HOSTIF_RCODE_ATTR_HOSTIF_GROUP
	    ptp_rcode_info = switcht_hostif_rcode_info_t(
		reason_code=SWITCH_HOSTIF_REASON_CODE_PTP,
		action=SWITCH_ACL_ACTION_REDIRECT_TO_CPU,
		hostif_group_id=hostif_group_id1)
	    rcode_handle = self.client.switch_api_hostif_reason_code_create(
		device, flags, ptp_rcode_info)

	    pkt = simple_eth_packet(eth_dst='01:1B:19:00:00:00', pktlen=100, eth_type=0x88f7)
	    exp_pkt = simple_cpu_packet(
		ingress_port=self.devport[1],
		ingress_ifindex=ingress_ifindex,
		reason_code=SWITCH_HOSTIF_REASON_CODE_PTP,
		ingress_bd=1,
		arrival_time=0x1,
		inner_pkt=pkt)
	    exp_ptpl2_pkt = cpu_packet_mask_ingress_bd_and_timestamp(exp_pkt)
	    print 'Sending PTP-L2 packet'
	    send_packet(self, swports[1], str(pkt))
	    verify_packet(self, exp_ptpl2_pkt, self.cpu_port)
	    # TODO : verify that packet is received by application
	    #self.assertTrue(socket_verify_packet(pkt, s))

	    pkt = simple_udp_packet(
		eth_dst='01:00:5E:00:01:81',
		ip_dst='224.0.1.81',
		udp_dport=319)
	    exp_pkt = simple_cpu_packet(
		ingress_port=self.devport[1],
		ingress_ifindex=ingress_ifindex,
		reason_code=SWITCH_HOSTIF_REASON_CODE_PTP,
		ingress_bd=1,
		arrival_time=0x1,
		inner_pkt=pkt)
	    exp_ptpl3_pkt = cpu_packet_mask_ingress_bd_and_timestamp(exp_pkt)
	    print 'Sending PTP-L3 packet'
	    send_packet(self, swports[1], str(pkt))
	    verify_packet(self, exp_ptpl3_pkt, self.cpu_port)
	    # TODO : verify that packet is received by application
	    #self.assertTrue(socket_verify_packet(pkt, s))

	    print 'Deleting hostif reason code'
	    self.client.switch_api_hostif_reason_code_delete(
		device, rcode_handle)

	    print 'Sending PTP-L2 packet'
	    pkt = simple_eth_packet(eth_dst='01:1B:19:00:00:00', pktlen=100, eth_type=0x88f7)
	    send_packet(self, swports[1], str(pkt))
	    verify_no_packet(self, exp_ptpl2_pkt, self.cpu_port, timeout=1)

	    print 'Sending PTP-L3 packet'
	    pkt = simple_udp_packet(
		eth_dst='01:00:5E:00:01:81',
		ip_dst='224.0.1.81',
		udp_dport=319)
	    send_packet(self, swports[1], str(pkt))
	    verify_no_packet(self, exp_ptpl3_pkt, self.cpu_port, timeout=1)

	finally:
	    s.close()
	    self.client.switch_api_hostif_delete(device, hostif_id)
	    self.client.switch_api_hostif_group_delete(device, hostif_group_id1)

	    self.client.switch_api_l3_interface_address_delete(device, rif1,
							       vrf, i_ip1)

	    self.client.switch_api_interface_delete(device, if1)

	    self.client.switch_api_rif_delete(0, rif1)

	    self.client.switch_api_router_mac_delete(device, rmac,
						     '00:77:66:55:44:33')
	    self.client.switch_api_vrf_delete(device, vrf)

