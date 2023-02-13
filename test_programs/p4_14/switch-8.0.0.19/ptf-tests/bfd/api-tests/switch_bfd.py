"""
Thrift API interface BFD tests
"""

import switchapi_thrift

import time
import sys
import logging
import pdb

import unittest
import random

try:
  import pltfm_pm_rpc
  from pltfm_pm_rpc.ttypes import *
except ImportError:
  pass


import ptf.dataplane as dataplane

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *

import os
import ptf.mask

from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../../../ptf-utils'))
from bfd_utils import *

sys.path.append(os.path.join(this_dir, '../../base'))
from common.utils import *
from common.api_utils import *
sys.path.append(os.path.join(this_dir, '../../base/api-tests'))
import api_base_tests
import pd_base_tests

device=0
cpu_port=64
swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)

if swports == []:
    swports = [x for x in range(65)]

my_ip = ['10.1.1.100', '11.1.1.100', '12.1.1.100', '13.1.1.100', '14.1.1.100']
peer_ip = ['10.1.1.1', '11.1.1.1', '12.1.1.1', '13.1.1.1', '14.1.1.1']
peer_mac = [
    '00:11:22:33:44:10', '00:11:22:33:44:11', '00:11:22:33:44:12',
    '00:11:22:33:44:13', '00:11:22:33:44:14'
]
rmac_addr = '00:77:66:55:44:33'

my_disc = 0x11111111
your_disc = [0x22222222, 0x33333333, 0x44444444, 0x55555555, 0x66666666]

tx_int1 = 1000000  # 1sec
rx_int1 = 1500000  # 1.5sec
detect_mult = 3


###############################################################################
@group('bfd')
@group('ipv4')
@group('maxsizes')
class bfdIPv4SessionOffloadTest(api_base_tests.ThriftInterfaceDataPlane,
                                pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def bfdTxPkt(self, i):
        return bfd_ipv4_packet(
            pktlen=66,
            eth_dst=peer_mac[i],
            eth_src=rmac_addr,
            ip_src=my_ip[i],
            ip_dst=peer_ip[i],
            ip_tos=48,
            ip_id=0x0001,
            udp_sport=0x1111 + i,
            detect_mult=detect_mult,
            my_discriminator=my_disc,
            your_discriminator=your_disc[i],
            min_tx_interval=tx_int1,
            min_rx_interval=rx_int1,
            echo_rx_interval=rx_int1)

    def bfdRxPkt(self, i):
        return bfd_ipv4_packet(
            pktlen=66,
            eth_dst=rmac_addr,
            eth_src=peer_mac[i],
            ip_src=peer_ip[i],
            ip_dst=my_ip[i],
            ip_tos=48,
            ip_id=0x0001,
            udp_sport=0x2222 + i,
            detect_mult=detect_mult,
            my_discriminator=your_disc[i],
            your_discriminator=my_disc,
            min_tx_interval=rx_int1,
            min_rx_interval=tx_int1,
            echo_rx_interval=tx_int1)

    def runTest(self):
        print
        print "BFD Offload Test"
        if test_param_get('target') == "bmv2":
            print "skipped (unsupported device)"

        self.cpu_port = get_cpu_port(self)
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)
        self.client.switch_api_init(device)

        i_if = []
        i_rif = []
        i_port = []
        i_ip1 = []
        i_ip3 = []  # peer ips
        nhop = []
        neighbor = []

        vrf = self.client.switch_api_vrf_create(device, 2)

        rmac = self.client.switch_api_router_mac_group_create(
            device, SWITCH_RMAC_TYPE_ALL)
        self.client.switch_api_router_mac_add(device, rmac, rmac_addr)

        for i in range(len(my_ip)):

            port = self.client.switch_api_port_id_to_handle_get(device,
                                                                swports[i + 1])
            i_port.append(port)

            rif_info = switcht_rif_info_t(
                rif_type=SWITCH_RIF_TYPE_INTF,
                vrf_handle=vrf,
                rmac_handle=rmac,
                v4_unicast_enabled=True)
            i_rif.append(self.client.switch_api_rif_create(0, rif_info))
            i_info1 = switcht_interface_info_t(
                handle=port,
                type=SWITCH_INTERFACE_TYPE_PORT,
                rif_handle=i_rif[i])
            i_if.append(
                self.client.switch_api_interface_create(device, i_info1))
            i_ip1.append(
                switcht_ip_addr_t(
                    addr_type=SWITCH_API_IP_ADDR_V4,
                    ipaddr=my_ip[i],
                    prefix_length=24))
            self.client.switch_api_l3_interface_address_add(device, i_rif[i],
                                                            vrf, i_ip1[i])
            # Add a static route
            i_ip3.append(
                switcht_ip_addr_t(
                    addr_type=SWITCH_API_IP_ADDR_V4,
                    ipaddr=peer_ip[i],
                    prefix_length=32))
            nhop_h, neighbor_h = switch_api_l3_nhop_neighbor_create(self, device, i_rif[i], i_ip3[i], peer_mac[i])
            nhop.append(nhop_h)
            neighbor.append(neighbor_h)
            self.client.switch_api_l3_route_add(device, vrf, i_ip3[i], nhop[i])

        bfd_hdl = []
        exp_bfd_tx_pkt = []
        exp_pkt_cpu = []
        bfd_rx_pkt = []
        bfd_info = []

        try:
            for i in range(len(my_ip)):
                exp_bfd_tx_pkt.append(self.bfdTxPkt(i))
                # ingress_port and ingress_ifindex should be set based on how pktgen
                # sets the ingress port and phase0 table programming
                recirc_ifindex = 0
		# TODO need a new reason code
                exp_pkt_cpu.append(
                    bfd_event_cpu_packet(
                        mac_da="00:00:00:00:00:00",
                        rcode=0x0006,
                        bfd_sid=i + 1,
                        ingress_port=0x80,
                        ingress_ifindex=recirc_ifindex,
                        bfd_event=0))
                bfd_rx_pkt.append(self.bfdRxPkt(i))
                bfd_info.append(
                    switcht_bfd_session_info_t(
                        my_disc=my_disc,
                        your_disc=your_disc[i],
                        detect_mult=detect_mult,
                        desired_tx_interval=tx_int1,  #usec
                        min_rx_interval=rx_int1,  #usec
                        tx_interval=tx_int1,
                        rx_interval=rx_int1,
                        remote_desired_tx_interval=rx_int1,  #usec
                        remote_min_rx_interval=tx_int1,  #usec
                        sip=switcht_ip_addr_t(
                            addr_type=SWITCH_API_IP_ADDR_V4,
                            ipaddr=my_ip[i],
                            prefix_length=32),
                        dip=switcht_ip_addr_t(
                            addr_type=SWITCH_API_IP_ADDR_V4,
                            ipaddr=peer_ip[i],
                            prefix_length=32),
                        sport=0x1111 + i,
                        dport=3784,
                        vrf_hdl=vrf,
                        rmac_hdl=rmac,
                        rmac=rmac_addr))

            time1 = (rx_int1 * detect_mult)
            tx_pkts = int(time1 / tx_int1)

            # Test transit BFD (m-hop) and non-bfd traffic
            non_bfd_pkt = simple_tcp_packet(
                eth_dst=rmac_addr,
                eth_src=peer_mac[0],
                ip_dst=peer_ip[1],
                ip_src=peer_ip[0],
                ip_id=105,
                ip_ttl=64)
            exp_non_bfd_pkt = simple_tcp_packet(
                eth_dst=peer_mac[1],
                eth_src=rmac_addr,
                ip_dst=peer_ip[1],
                ip_src=peer_ip[0],
                ip_id=105,
                ip_ttl=63)

            print "Test non-BFD pkt from port 1->2"
            send_packet(self, swports[1], str(non_bfd_pkt))
            verify_packets(self, exp_non_bfd_pkt, [swports[2]])

            # Test bfd transit pkt
            bfd_pkt_2_3 = bfd_ipv4_packet(
                pktlen=66,
                eth_dst=rmac_addr,
                eth_src=peer_mac[1],
                ip_src=peer_ip[1],
                ip_dst=peer_ip[2],
                ip_tos=48,
                ip_ttl=64,
                udp_dport=4784,
                my_discriminator=your_disc[1],
                your_discriminator=your_disc[2])

            exp_bfd_pkt_2_3 = bfd_ipv4_packet(
                pktlen=66,
                eth_dst=peer_mac[2],
                eth_src=rmac_addr,
                ip_src=peer_ip[1],
                ip_dst=peer_ip[2],
                ip_tos=48,
                ip_ttl=63,
                udp_dport=4784,
                my_discriminator=your_disc[1],
                your_discriminator=your_disc[2])
            print "Test m-hop BFD transit packet on port 2->3"
            send_packet(self, swports[2], str(bfd_pkt_2_3))
            verify_packets(self, exp_bfd_pkt_2_3, [swports[3]])

            # offload a session,
# check -
            # - pkts are periodically sent out to peer
            # - inject bfd rx packets (using a timers, not in response to
            #   bfd tx)
            # - check that session stays offloaded as long as bfd rx pkts are
            #   generated
            # - stop rx pkt
            # - event is sent to cpu to onload the session after detection
            #   timeout
            print "Test BFD session 1"
            bfd_hdl.append(
                self.client.switch_api_bfd_session_create(device, bfd_info[0]))

            time.sleep((time1 / 1000000) - 1)
            # packet is sent 1 per sec
            for t in range(1, tx_pkts):
                # print "check pkt ", t
                verify_packet(self, exp_bfd_tx_pkt[0], swports[1])
            # reset rx timeout by sending a packet
            print "Reset Rx Timer for bfd session 1"
            send_packet(self, swports[1], str(bfd_rx_pkt[0]))
            timeout = (time1 / 1000000) + 1
            # print "Timeout Rx Timer for bfd session 1"
            time.sleep(timeout * 2)
            for t in range(tx_pkts):
                # print "check pkt ", t
                verify_packet(self, exp_bfd_tx_pkt[0], swports[1])

            verify_packet(self, exp_pkt_cpu[0], self.cpu_port)
            verify_no_other_packets(self)
            # send another packet, this should not start the session again
            send_packet(self, swports[1], str(bfd_rx_pkt[0]))
            time.sleep(2)
            verify_no_other_packets(self)
            # delete the session
            # print "Delete BFD sessions 1"
            self.client.switch_api_bfd_session_delete(device, bfd_hdl[0])
            del bfd_hdl[0]

            ### do it again ###
            print "Test BFD session 1(again)"
            bfd_hdl.append(
                self.client.switch_api_bfd_session_create(device, bfd_info[0]))

            time.sleep((time1 / 1000000) - 1)
            # packet is sent 1 per sec
            for t in range(1, tx_pkts):
                # print "check pkt ", t
                verify_packet(self, exp_bfd_tx_pkt[0], swports[1])
            # reset rx timeout by sending a packet
            print "Reset Rx Timer for bfd session 1"
            send_packet(self, swports[1], str(bfd_rx_pkt[0]))
            timeout = (time1 / 1000000) + 1
            # print "Timeout Rx Timer for bfd session 1"
            time.sleep(timeout * 2)
            for t in range(tx_pkts):
                # print "check pkt ", t
                verify_packet(self, exp_bfd_tx_pkt[0], swports[1])

            verify_packet(self, exp_pkt_cpu[0], self.cpu_port)
            verify_no_other_packets(self)
            # send another packet, this should not start the session again
            send_packet(self, swports[1], str(bfd_rx_pkt[0]))
            time.sleep(2)
            verify_no_other_packets(self)
            # delete the session
            # print "Delete BFD sessions 1"
            self.client.switch_api_bfd_session_delete(device, bfd_hdl[0])
            del bfd_hdl[0]

            # test multiple (5) bfd sessions
            print "Test BFD sessions 1-5"
            for i in range(0, len(my_ip)):
                bfd_hdl.append(
                    self.client.switch_api_bfd_session_create(device, bfd_info[
                        i]))

            # Test transit traffic in presence of bfd offloads
            print "Test non-BFD pkt from port 1->2 (with offload sessions)"
            send_packet(self, swports[1], str(non_bfd_pkt))
            print "Test m-hop BFD transit packet on port 2->3 (with offload)"
            send_packet(self, swports[2], str(bfd_pkt_2_3))
            # just check a few packets on each port and cpu port
            # model speed can be un-predicatble with multiple sessions active
            # wait long enough for timeout
            timeout = (rx_int1 * detect_mult)
            # print "Timeout Rx Timer for all BFD sessions"
            time.sleep(timeout * 3 / 1000000)

            for i in range(0, len(my_ip)):
                for t in range(tx_pkts):
                    # print "check pkt ", t , " on session ", i+1
                    if i == 1:
                        verify_any_packet_on_port(
                            self, [exp_bfd_tx_pkt[i], exp_non_bfd_pkt],
                            swports[i + 1])
                    elif i == 2:
                        verify_any_packet_on_port(
                            self, [exp_bfd_tx_pkt[i], exp_bfd_pkt_2_3],
                            swports[i + 1])
                    else:
                        verify_packet(self, exp_bfd_tx_pkt[i], swports[i + 1])

            # check that all sessions timed-out
            for i in range(0, len(my_ip)):
                verify_any_packet_on_port(self, exp_pkt_cpu, self.cpu_port)

            # verify non bfd offload packets on ports 2,3
            verify_any_packet_on_port(
                self, [exp_bfd_tx_pkt[1], exp_non_bfd_pkt], swports[2])
            verify_any_packet_on_port(
                self, [exp_bfd_tx_pkt[2], exp_bfd_pkt_2_3], swports[3])

            verify_no_other_packets(self)

        finally:
            for h in bfd_hdl:
                self.client.switch_api_bfd_session_delete(device, h)

            for h in neighbor:
                self.client.switch_api_neighbor_delete(device, h)

            for i in range(len(i_ip3)):
                self.client.switch_api_l3_route_delete(device, vrf, i_ip3[i],
                                                       nhop[i])

            for h in nhop:
                self.client.switch_api_nhop_delete(device, h)

            for i in range(len(i_ip1)):
                self.client.switch_api_l3_interface_address_delete(
                    device, i_rif[i], vrf, i_ip1[i])

            for h in i_if:
                self.client.switch_api_interface_delete(device, h)

            for h in i_rif:
                self.client.switch_api_rif_delete(device, h)

            self.client.switch_api_router_mac_delete(device, rmac, rmac_addr)
            self.client.switch_api_router_mac_group_delete(device, rmac)
            self.client.switch_api_vrf_delete(device, vrf)
