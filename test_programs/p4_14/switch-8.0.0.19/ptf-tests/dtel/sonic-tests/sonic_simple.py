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
Simple SONiC test
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

from switch_ptf_config import *

srcport = 0
dstports = [1, 2]

################################################################################
@group('simple')
class SimpleTest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):

        for dstport in dstports:
            print 'simple packet test %d to %d' %(srcport, dstport)
            pkt_in = simple_tcp_packet(
                eth_dst=mac_self,
                eth_src=mac_nbr[srcport],
                ip_dst=ipaddr_nbr[dstport],
                ip_src=ipaddr_nbr[srcport],
                ip_id=105,
                ip_ttl=64,
                tcp_flags=None,
                pktlen=128)

            exp_pkt_out = simple_tcp_packet(
                eth_dst=mac_nbr[dstport],
                eth_src=mac_self,
                ip_dst=ipaddr_nbr[dstport],
                ip_src=ipaddr_nbr[srcport],
                ip_id=105,
                tcp_flags=None,
                ip_ttl=63,
                pktlen=128)

            # send a test packet
            send_packet(self, swports[srcport], str(pkt_in))
            # verify packet out
            verify_packet(self, exp_pkt_out, swports[dstport])
            #receive_print_packet(self, swports[1], exp_pkt_out, True, False)
