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
bloom filter test in int 1 hop sink
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
port_monitor = 2
port_src = 3
port_dst = 3
eth_dst = mac_s33_r
eth_src = mac_h41_3
ip_dst = ip_h42_1
ip_src = ip_h41_3

################################################################################
@group('test_bfilter')
# 1 hop should be tested only with statefull as we cannot disable that
# if p4 is compiled with stateful suppression
class INTL45_EgressBFilter(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):

        print "Test INT L45/StFull Bloom Filter"
        prepare_int_l45_bindings()

        payload = 'int l45'
        # make input frame to inject to sink
        init_sport=101
        init_dport=202
        pkt = simple_udp_packet(
            eth_dst=eth_dst,
            eth_src=eth_src,
            ip_dst=ip_dst,
            ip_src=ip_src,
            ip_id=108,
            ip_ttl=64,
            udp_sport=init_sport,
            udp_dport=init_dport,
            with_udp_chksum=False,
            udp_payload=payload)

        try:
	  max_iter=int(raw_input("# packets: "))
	  if max_iter > 0:
            # false negative is practically zero (should send but not send)
            # because we keep flow_hash inside entries (16bit) and have 64k
            # entries
            sport=init_sport
            dport=init_dport
            for i in range(max_iter):
                sport=(sport+1) & 0xffff
                dport=(dport+1) & 0xffff
                if sport==init_sport:
                    dport=(dport+1) & 0xffff
                pkt[UDP].sport=sport
                pkt[UDP].dport=dport

                send_packet(self, port_src, str(pkt))
                if (i+1) % 1000==0:
                    print i+1

            raw_input("press enter")
            # send again and count the # false positive (no change but report)
            sport=init_sport
            dport=init_dport
            for i in range(max_iter):
                sport=(sport+1) & 0xffff
                dport=(dport+1) & 0xffff
                if sport==init_sport:
                    dport=(dport+1) & 0xffff
                pkt[UDP].sport=sport
                pkt[UDP].dport=dport

                send_packet(self, port_src, str(pkt))
                if (i+1) % 1000==0:
                    print i+1

        finally:
            print "Test Cleanup"
