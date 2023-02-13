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
INT-leaf switch configuration through thrift SAI interface
"""

import switchsai_thrift
import pdb
import time
import sys
import logging
import os
import unittest
import random

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *

from switchsai_thrift.ttypes import *
from switchsai_thrift.sai_headers import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../base/sai-tests'))
import sai_base_test

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '..'))
from dtel_sai_utils import *

swports = range(32)
sink_ports = [28]

params = SwitchSAIConfig_Params()
params.switch_id = 0x2
params.mac_self = '8C:EA:1B:A8:19:51'
params.ipaddr_erspan_src = '10.14.20.60'
#params.ipaddr_erspan_dst = ['192.168.10.5']
params.ipaddr_erspan_dst = ['192.168.110.103', '192.168.120.101']
params.erspan_span_id = 999
params.swports = swports
params.erspan_truncate_size = 256

@group('int_leaf_2')
class INT_LEAF_2(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        # create ptf SAI mananger
        sai_mgr = SAIManager(self, params)

        # configure erspan
        sai_mgr.configureErspan()

        # create SAI DTel INT session object
        int_session = sai_mgr.createINTSession(session_id=1,
                                               max_hop_count=8,
                                               inst_switch_id=True,
                                               inst_switch_ports=True,
                                               inst_ingress_timestamp=True,
                                               inst_egress_timestamp=True,
                                               inst_queue_info=True)

        # create INT watchlist table
        int_watchlist = sai_mgr.createDTelWatchlist('INT')
        # Add INT watchlist entry
        int_watchlist_entry = sai_mgr.createDTelWatchlistEntry(
            watchlist=int_watchlist,
            priority=10,
            ip_src='192.168.0.0',
            ip_src_mask='255.255.0.0',
            ip_dst='192.168.0.0',
            ip_dst_mask='255.255.0.0',
            dtel_int_enable=True,
            dtel_int_session=1,
            dtel_report_all=False)

        # add INT sink ports
        sai_mgr.switch.dtel_int_sink_port_list = \
            [sai_mgr.ports[i] for i in sink_ports]

        # Enable INT endpoint
        sai_mgr.switch.dtel_int_ep_enable = True

        sai_mgr.switch.dtel_flow_state_clear_cycle = 1

        # create MoD watchlist table
        mod_watchlist = sai_mgr.createDTelWatchlist('MOD')

        # Add MoD watchlist entry
        mod_watchlist_entry = sai_mgr.createDTelWatchlistEntry(
            watchlist=mod_watchlist,
            priority=10,
            ip_src='192.168.0.0',
            ip_src_mask='255.255.0.0',
            ip_dst='192.168.0.0',
            ip_dst_mask='255.255.0.0',
            dtel_mod_enable=True)

        # Enable Mirror on Drop
        sai_mgr.switch.dtel_mirror_on_drop_enable = True

        raw_input('press any key to cleanup ...')

        sai_mgr.switch.dtel_int_ep_enable = False
        sai_mgr.switch.dtel_mirror_on_drop_enable = False
        sai_mgr.switch.dtel_flow_state_clear_cycle = 0
        sai_mgr.cleanup()
