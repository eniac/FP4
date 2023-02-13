###############################################################################
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
Postcard tests
"""

import logging
import os
import random
import switchapi_thrift
import sys
import time
import unittest
import threading

import ptf.dataplane as dataplane
from scapy.all import *
from erspan3 import *
from ptf.testutils import *
from ptf.thriftutils import *
from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../base/api-tests'))
import api_base_tests

from constants import *
sys.path.append(os.path.join(this_dir, '..'))
from dtel_utils import *

swports = [0, 1, 2, 3]

################################################################################
test_eth_dst = mac_s32_r
test_eth_src = mac_h41_0
test_ip_dst = ip_h41_3
test_ip_src = ip_h41_0

def tcp_test_packet():
    return simple_tcp_packet(eth_dst=test_eth_dst, eth_src=test_eth_src,
                             ip_dst=test_ip_dst, ip_src=test_ip_src, pktlen=256)

def udp_test_packet():
    return simple_udp_packet(eth_dst=test_eth_dst, eth_src=test_eth_src,
                             ip_dst=test_ip_dst, ip_src=test_ip_src,
                             with_udp_chksum=False, pktlen=256)

def vxlan_gpe_test_pkt(inner_frame):
    return simple_vxlan_gpe_packet(eth_dst=test_eth_dst, eth_src=test_eth_src,
                                   ip_dst=test_ip_dst, ip_src=test_ip_src,
                                   inner_frame=inner_frame)

################################################################################
@group('pktgen')
class PacketGen(api_base_tests.ThriftInterfaceDataPlane):
    def status_thread(self):
        prev_sent = 0
        time_sec = 0
        while (self.run == True):
            time.sleep(1)
            time_sec += 1
            print '\n==========================='
            print 'time : ', time_sec
            print 'packets sent :', self.pkt_sent, \
                  'rate: ', self.pkt_sent-prev_sent
            prev_sent = self.pkt_sent

    def runTest(self):
        print 'test postcard'
        self.run = True
        self.pkt_sent = 0
        self.pkt_rcvd = 0

        print 'Select packet type :'
        print '0 -- tcp'
        print '1 -- udp'
        print '2 -- vxlan_gep with inner tcp'
        print '3 -- vxlan_gep with inner udp'
        pkt_type = int(raw_input('Enter your choics : '))
        flow_num = int(raw_input('Enter number of flows : '))
        duration = int(raw_input('Enter experiment duration (sec) : '))

        if pkt_type == 0:
            test_pkt = tcp_test_packet()
        elif pkt_type == 1:
            test_pkt = udp_test_packet()
        elif pkt_type == 2:
            inner_pkt = simple_tcp_packet()
            test_pkt = vxlan_gpe_test_pkt(inner_pkt)
        else:
            inner_pkt = simple_udp_packet()
            test_pkt = vxlan_gpe_test_pkt(inner_pkt)

        t1 = threading.Thread(target=self.status_thread)
        t1.start()

        time_start = time.time()
        time_run = 0
        ip_id = 0
        while time_run < duration:
            ip_id += 1
            for j in range(flow_num):
                if pkt_type == 0:
                    test_pkt[TCP].sport = 10000 + j
                    test_pkt[TCP].dport = 10000 + j
                elif pkt_type == 1:
                    test_pkt[UDP].sport = 10000 + j
                    test_pkt[UDP].dport = 10000 + j
                elif pkt_type == 2:
                    inner_pkt = tcp_test_packet()
                    inner_pkt[TCP].sport = 10000 + j
                    inner_pkt[TCP].dport = 10000 + j
                    inner_pkt[IP].id = ip_id
                    test_pkt = vxlan_gpe_test_pkt(inner_pkt)
                else:
                    inner_pkt = udp_test_packet()
                    inner_pkt[UDP].sport = 10000 + j
                    inner_pkt[UDP].dport = 10000 + j
                    inner_pkt[IP].id = ip_id
                    test_pkt = vxlan_gpe_test_pkt(inner_pkt)
                test_pkt[IP].id = ip_id
                #if ip_id % 10 == 0:
                #    send_packet(self, swports[1], str(test_pkt))
                #else:
                send_packet(self, swports[0], str(test_pkt))
                self.pkt_sent += 1
                #time.sleep(0.001)
                time_run = int(time.time() - time_start)

        time.sleep(1)
        self.run = False
