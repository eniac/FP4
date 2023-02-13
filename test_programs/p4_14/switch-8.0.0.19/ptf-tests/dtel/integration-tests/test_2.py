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
port_monitor = 2
port_src = 0
port_dst = 3
eth_dst = mac_s44_r
eth_src = mac_h35_0
ip_dst = ip_h35_3
ip_src = ip_h35_0

################################################################################
@group('test_2')
class SimpleTest_2(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print 'test DTel on second setup'
        bind_postcard_pkt()
        bind_mirror_on_drop_pkt()
        prepare_int_l45_bindings()

        pkt_send = simple_udp_packet(
            eth_dst = eth_dst,
            eth_src = eth_src,
            ip_dst =  ip_dst,
            ip_src =  ip_src,
            ip_id = 105,
            ip_ttl = 64,
            pktlen = 256,
            with_udp_chksum = False)

        send_packet(self, port_src, str(pkt_send))
        receive_print_packet(self, port_dst, pkt_send, False, False)
        receive_print_packet(self, port_monitor, pkt_send, False, False)
        receive_print_packet(self, port_monitor, pkt_send, False, False)
        receive_print_packet(self, port_monitor, pkt_send, False, False)
