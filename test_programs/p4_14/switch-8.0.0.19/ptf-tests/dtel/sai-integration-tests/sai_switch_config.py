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

import switchsai_thrift
import pdb
import time
import sys
import logging
import os
import unittest
import random
import itertools

# Import machine specific parameters
import aasw35 as sw1
import aasw36 as sw2
import aasw37 as sw3
import aasw38 as sw4
import aah42 as h2
import aah44 as h4

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *

from switchsai_thrift.ttypes import *
from switchsai_thrift.sai_headers import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../base'))
from common.utils import *

sys.path.append(os.path.join(this_dir, '../../base/sai-ocp-tests'))
import sai_base_test
from switch_utils import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '..'))
from dtel_utils import *
from dtel_sai_utils import *

device = 0
quantization_shift = 15
if test_param_get('target') == "asic-model":
    reset_cycle = 6
    min_sleeptime = 30
else:
    reset_cycle = 1
    min_sleeptime = 1

TYPE_POSTCARD = 0
TYPE_INT_EP = 1
TYPE_INT_TRANSIT = 2

@group('configsw')
class ConfigSW(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Configure a switch"

        # User defined variables that might need setting
        hosts = [h2, h4]
        report_dst = [h4.ipaddr_inf[1]]
        dtel_sample_percent = 100
        dtel_report_all = False
        dtel_drop_report_enable = True
        dtel_latency_sensitivity = 15

        switch_number = int(raw_input(
                "Select switch number [1-4]: "))
        # Set switch under config.
        # Make sure that the report port can reach its destination
        if switch_number == 1:
            sw = sw1
            report_ports = [sw.swports[sw.frontports.index("24/-")]]
        elif switch_number == 2:
            sw = sw2
            report_ports = [sw.swports[sw.frontports.index("24/-")]]
        elif switch_number == 3:
            sw = sw3
            report_ports = [sw.swports[sw.frontports.index("21/-")]]
        elif switch_number == 4:
            sw = sw4
            report_ports = [sw.swports[sw.frontports.index("1/3")]]
        else:
            print "Wrong switch number"
            sys.exit()

        params = SwitchSAIConfig_Params()
        # User defined variables that probably do not need setting
        params.switch_id = sw.switch_id
        params.swports = sw.swports
        params.mac_self = sw.mac_self
        params.nports = sw.nports
        params.ipaddr_inf = sw.ipaddr_inf
        params.ipaddr_nbr = sw.ipaddr_nbr
        params.mac_nbr = sw.mac_nbr
        params.report_ports = report_ports
        params.report_src = sw.management_ip
        params.report_dst = report_dst
        params.report_udp_port = 32766
        params.report_truncate_size = 256
        params.configure_routes = False

        dtel_int_enable = None
        dtel_int_session = None
        dtel_postcard_enable = None
        
        switch_type = int(raw_input(
                "Select switch type: 0 -- postcard, 1 -- INT Endpoint, 2 -- INT transit: "))
        if switch_type not in [0, 1, 2]:
            print "Invalid type"
            sys.exit()

        try:
            sai_mgr = SAIManager(self, params)
            sai_mgr.create_dtel_report_session()

            if switch_type == TYPE_POSTCARD:
                # enable Postcard
                sai_mgr.switch.dtel_postcard_enable = True
                dtel_postcard_enable = True
            else:
                dtel_int_enable = True
                dtel_int_session = sai_mgr.create_dtel_int_session()
                if switch_type == TYPE_INT_EP:
                    # enable INT EP
                    sai_mgr.switch.dtel_int_endpoint_enable = True
                    sai_mgr.switch.dtel_int_sink_port_list = [sai_mgr.ports]
                else:
                    # enable INT TRANSIT
                    sai_mgr.switch.dtel_int_transit_enable = True
            
            

            addresses = [address for host in hosts for address in host.ipaddr_inf]
            address_combos = [combo for combo in itertools.combinations(addresses, 2)]

            flow_watchlist = sai_mgr.create_dtel_watchlist("Flow")
            watchlist_entries = [sai_mgr.create_dtel_watchlist_entry(
                                        watchlist=flow_watchlist,
                                        priority=10,
                                        ip_src=combo[0],
                                        ip_src_mask='255.255.255.0',
                                        ip_dst=combo[1],
                                        ip_dst_mask='255.255.255.0',
                                        dtel_int_enable=dtel_int_enable,
                                        dtel_int_session=dtel_int_session,
                                        dtel_postcard_enable=dtel_postcard_enable,
                                        dtel_sample_percent=dtel_sample_percent,
                                        dtel_report_all=dtel_report_all)
                                    for combo in address_combos]
            
            if dtel_drop_report_enable == True:
                sai_mgr.switch.dtel_drop_report_enable = True
                drop_watchlist = sai_mgr.create_dtel_watchlist("Drop")
                drop_watchlist_entries = [sai_mgr.create_dtel_watchlist_entry(
                                                watchlist=drop_watchlist,
                                                priority=10,
                                                ip_src=combo[0],
                                                ip_src_mask='255.255.255.0',
                                                ip_dst=combo[1],
                                                ip_dst_mask='255.255.255.0',
                                                dtel_sample_percent=dtel_sample_percent,
                                                dtel_report_all=dtel_report_all,
                                                dtel_drop_report_enable=True)
                                            for combo in address_combos]
            
            sai_mgr.switch.dtel_latency_sensitivity = dtel_latency_sensitivity
            
            if switch_type in [TYPE_POSTCARD, TYPE_INT_EP]:
                sai_mgr.switch.dtel_flow_state_clear_cycle = reset_cycle

            raw_input("Press any key to cleanup...")
        
        finally:
            if dtel_drop_report_enable == True:
                sai_mgr.switch.dtel_drop_report_enable = False
            if dtel_int_enable == True:
                if switch_type == TYPE_INT_EP:
                    sai_mgr.switch.dtel_int_endpoint_enable = False
                else:
                    sai_mgr.switch.dtel_int_transit_enable = False
            sai_mgr.cleanup()

