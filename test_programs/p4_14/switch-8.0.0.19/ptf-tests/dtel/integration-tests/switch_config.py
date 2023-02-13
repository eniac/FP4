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
Switch configuration
"""

import logging
import os
import random
import switchapi_thrift
import sys
import time
import unittest
import threading
import struct
import socket

import ptf.dataplane as dataplane
from scapy.all import *
from ptf.testutils import *
from ptf.thriftutils import *
from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../base'))
from common.utils import *
sys.path.append(os.path.join(this_dir, '../../base/api-tests'))
import api_base_tests

from constants import *
sys.path.append(os.path.join(this_dir, '..'))
from dtel_utils import *

import aasw35
import aasw36
import aasw37
import aasw38
import aah42
import aah44

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

def config_watchlist(test, postcard_watch=False, mod_watch=False,
        int_watch=False, int_session_id=1, suppress=True, percent=100):
    if postcard_watch:
        postcard_ap = switcht_twl_postcard_params_t(
            report_all_packets=not suppress, flow_sample_percent=percent)
    if mod_watch:
        mod_ap = switcht_twl_mod_params_t(report_queue_tail_drops=True)
    if int_watch:
        int_ap = switcht_twl_int_params_t(session_id=int_session_id,
                                          report_all_packets=not suppress,
                                          flow_sample_percent=percent)

    twl_kvp = []
    kvp_val = switcht_twl_value_t(value_num=0x0800)
    kvp_mask = switcht_twl_value_t(value_num=0xffff)
    twl_kvp.append(switcht_twl_key_value_pair_t(
        SWITCH_TWL_FIELD_ETHER_TYPE, kvp_val, kvp_mask))

    kvp_val = switcht_twl_value_t(value_num=ipv4Addr_to_i32('10.131.0.0'))
    kvp_mask = switcht_twl_value_t(value_num=0xffe00000)
    twl_kvp.append(switcht_twl_key_value_pair_t(
        SWITCH_TWL_FIELD_IPV4_SRC, kvp_val, kvp_mask))

    kvp_val = switcht_twl_value_t(value_num=ipv4Addr_to_i32('10.131.0.0'))
    kvp_mask = switcht_twl_value_t(value_num=0xffe00000)
    twl_kvp.append(switcht_twl_key_value_pair_t(
        SWITCH_TWL_FIELD_IPV4_DST, kvp_val, kvp_mask))

    # add a rule to exclude (not monitor) traffic to switch interfaces
    twl_kvp2 = []
    kvp_val = switcht_twl_value_t(value_num=0x0800)
    kvp_mask = switcht_twl_value_t(value_num=0xffff)
    twl_kvp2.append(switcht_twl_key_value_pair_t(
        SWITCH_TWL_FIELD_ETHER_TYPE, kvp_val, kvp_mask))
    # assume *.*.*.1 is always switch interface
    kvp_val = switcht_twl_value_t(value_num=ipv4Addr_to_i32('0.0.0.1'))
    kvp_mask = switcht_twl_value_t(value_num=0x000000ff)
    twl_kvp2.append(switcht_twl_key_value_pair_t(
        SWITCH_TWL_FIELD_IPV4_DST, kvp_val, kvp_mask))
    if postcard_watch:
        test.client.switch_api_dtel_postcard_watchlist_entry_create(
            device, twl_kvp, priority=10, watch=True, action_params=postcard_ap)
    if mod_watch:
        test.client.switch_api_dtel_drop_watchlist_entry_create(
            device, twl_kvp, priority=10, watch=True, action_params=mod_ap)
    if int_watch:
        test.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp, priority=10, watch=True, action_params=int_ap)
        test.client.switch_api_dtel_int_watchlist_entry_create(
            device, twl_kvp2, priority=1, watch=False, action_params=int_ap)
        if mod_watch:
            test.client.switch_api_dtel_drop_watchlist_entry_create(
                device, twl_kvp2, priority=1, watch=False, action_params=mod_ap)

###############################################################################
@group('aasw35')
class ConfigAASW35(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Configure aasw35 (Mavericks 10.12.11.35)"
        sw = aasw35
	port_index = sw.port_index
        frontports = sw.frontports
        swports = sw.swports

        params = SwitchConfig_Params()
        params.switch_id = sw.switch_id
        params.mac_self = sw.mac_self
        params.nports = sw.nports
        params.ipaddr_inf = sw.ipaddr_inf
        params.ipaddr_nbr = sw.ipaddr_nbr
        params.mac_nbr = sw.mac_nbr
        params.routes = [(aah44.ipaddr_inf[0], sw.frontports.index("24/-")),
                         (aah44.ipaddr_inf[1], sw.frontports.index("24/-"))]
        params.report_ports = [sw.frontports.index("24/-")]
        params.ipaddr_report_src = ['10.12.11.35']
        params.ipaddr_report_dst = [aah44.ipaddr_inf[1]]
        params.mirror_ids = [555]
        params.device = device
        params.swports = swports
        params.port_speed = SWITCH_PORT_SPEED_25G
        params.nports_max = 132

        # enable MoD
        mod_enabled = True
        try:
            config = SwitchHwConfig(self, params)
            switch_type = int(raw_input(
                'Enter switch type: 0 -- postcard, 2 -- INT transit: '))

            if (mod_enabled):
                self.client.switch_api_dtel_drop_report_enable(device)

            if switch_type == TYPE_POSTCARD:
                # enable Postcard
                self.client.switch_api_dtel_postcard_enable(device)
                # add flow space to postcard watchlist
                config_watchlist(self, postcard_watch=True, mod_watch=mod_enabled)
                self.client.switch_api_dtel_latency_quantization_shift(
                    device, quantization_shift)
                self.client.switch_api_dtel_flow_state_clear_cycle(
                    device, reset_cycle)
            elif switch_type == TYPE_INT_TRANSIT:
                # create MOD watchlist
                config_watchlist(self, int_watch=False, mod_watch=mod_enabled,
                                 suppress=True)

                self.client.switch_api_dtel_int_transit_enable(device)
                self.client.switch_api_dtel_latency_quantization_shift(
                    device, quantization_shift)
            else:
                print 'invalid switch type'

            raw_input("Press any key to cleanup...")

        # cleanup
        finally:
            if mod_enabled:
                self.client.switch_api_dtel_drop_watchlist_clear(device)
                self.client.switch_api_dtel_drop_report_disable(device)
            if switch_type == TYPE_POSTCARD:
                self.client.switch_api_dtel_postcard_watchlist_clear(device)
                self.client.switch_api_dtel_postcard_disable(device)
            elif switch_type == TYPE_INT_TRANSIT:
                self.client.switch_api_dtel_int_transit_disable(
                    device=device)
            config.cleanup(self)

###############################################################################
@group('aasw36')
class ConfigAASW36(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Configure aasw36 (Mavericks 10.12.11.36)"
        sw = aasw36
	port_index = sw.port_index
        frontports = sw.frontports
        swports = sw.swports

        params = SwitchConfig_Params()
        params.switch_id = sw.switch_id
        params.mac_self = sw.mac_self
        params.nports = sw.nports
        params.ipaddr_inf = sw.ipaddr_inf
        params.ipaddr_nbr = sw.ipaddr_nbr
        params.mac_nbr = sw.mac_nbr
        params.routes = [(aah44.ipaddr_inf[0], sw.frontports.index("24/-")),
                         (aah44.ipaddr_inf[1], sw.frontports.index("24/-"))]
        params.report_ports = [sw.frontports.index("24/-")]
        params.ipaddr_report_src = ['10.12.11.36']
        params.ipaddr_report_dst = [aah44.ipaddr_inf[1]]
        params.mirror_ids = [555]
        params.device = device
        params.swports = swports
        params.port_speed = SWITCH_PORT_SPEED_25G
        params.nports_max = 132
        # enable MoD
        mod_enabled = True
        try:
            config = SwitchHwConfig(self, params)
            switch_type = int(raw_input(
                'Enter switch type: 0 -- postcard, 2 -- INT transit : '))

            if (mod_enabled):
                self.client.switch_api_dtel_drop_report_enable(device)

            if switch_type == TYPE_POSTCARD:
                # enable Postcard
                self.client.switch_api_dtel_postcard_enable(device)
                # add flow space to postcard watchlist
                config_watchlist(self, postcard_watch=True, mod_watch=mod_enabled)
                self.client.switch_api_dtel_latency_quantization_shift(
                    device, quantization_shift)
                self.client.switch_api_dtel_flow_state_clear_cycle(
                    device, reset_cycle)
            elif switch_type == TYPE_INT_TRANSIT:
                # create MOD watchlist
                config_watchlist(self, int_watch=False, mod_watch=mod_enabled,
                                 suppress=True)

                self.client.switch_api_dtel_int_transit_enable(device)
                self.client.switch_api_dtel_latency_quantization_shift(
                    device, quantization_shift)
            else:
                print 'invalid switch type'

            raw_input("Press any key to cleanup...")

        # cleanup
        finally:
            if mod_enabled:
                self.client.switch_api_dtel_drop_watchlist_clear(device)
                self.client.switch_api_dtel_drop_report_disable(device)
            if switch_type == TYPE_POSTCARD:
                self.client.switch_api_dtel_postcard_watchlist_clear(device)
                self.client.switch_api_dtel_postcard_disable(device)
            elif switch_type == TYPE_INT_TRANSIT:
                self.client.switch_api_dtel_int_transit_disable(
                    device=device)
            config.cleanup(self)

###############################################################################
@group('aasw37')
class ConfigAASW37(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Configure aasw37 (Mavericks 10.12.11.37)"
        sw = aasw37
	port_index = sw.port_index
        frontports = sw.frontports
        swports = sw.swports

        params = SwitchConfig_Params()
        params.switch_id = sw.switch_id
        params.mac_self = sw.mac_self
        params.nports = sw.nports
        params.ipaddr_inf = sw.ipaddr_inf
        params.ipaddr_nbr = sw.ipaddr_nbr
        params.mac_nbr = sw.mac_nbr
        params.routes = [(aah44.ipaddr_inf[0], sw.frontports.index("24/-")),
                         (aah44.ipaddr_inf[1], sw.frontports.index("24/-"))]
        params.report_ports = [sw.frontports.index("24/-")]
        params.ipaddr_report_src = ['10.12.11.37']
        params.ipaddr_report_dst = [aah44.ipaddr_inf[1]]
        params.mirror_ids = [555]
        params.device = device
        params.swports = swports
        params.port_speed = SWITCH_PORT_SPEED_25G
        params.nports_max = 132
        # enable MoD
        mod_enabled = True
        try:
            config = SwitchHwConfig(self, params)
            switch_type = int(raw_input(
                'Enter switch type: 0 -- postcard, 1 -- INT endpoint : '))

            if (mod_enabled):
                self.client.switch_api_dtel_drop_report_enable(device)

            if switch_type == TYPE_POSTCARD:
                # enable Postcard
                self.client.switch_api_dtel_postcard_enable(device)
                # add flow space to postcard watchlist
                config_watchlist(self, postcard_watch=True, mod_watch=mod_enabled)
                self.client.switch_api_dtel_latency_quantization_shift(
                    device, quantization_shift)
                self.client.switch_api_dtel_flow_state_clear_cycle(
                    device, reset_cycle)
            elif switch_type == TYPE_INT_EP:
                # create an INT session with all metadata
                self.client.switch_api_dtel_int_session_create(
                    device=device, session_id=1,
                    instruction=convert_int_instruction(0xDC00), max_hop=8)

                # create INT watchlist
                config_watchlist(self, int_watch=True, int_session_id=1,
                                 mod_watch=mod_enabled, suppress=True)

                # set INT sink ports
                # TODO: automate configuration of sink ports?
                for i in range(0, 8):
                    self.client.switch_api_dtel_int_sink_ports_add(
                        device=device, port=swports[i])

                self.client.switch_api_dtel_int_endpoint_enable(device)
                self.client.switch_api_dtel_latency_quantization_shift(
                    device, quantization_shift)
                self.client.switch_api_dtel_flow_state_clear_cycle(
                    device, reset_cycle)
            else:
                print 'invalid switch type'

            raw_input("Press any key to cleanup...")

        # cleanup
        finally:
            if mod_enabled:
                self.client.switch_api_dtel_drop_report_disable(device)
                self.client.switch_api_dtel_drop_watchlist_clear(device)
            if switch_type == TYPE_POSTCARD:
                self.client.switch_api_dtel_postcard_watchlist_clear(device)
                self.client.switch_api_dtel_postcard_disable(device)
            elif switch_type == TYPE_INT_EP:
                self.client.switch_api_dtel_int_watchlist_clear(device)
                self.client.switch_api_dtel_int_session_delete(
                    device=device, session_id=1)
                #self.client.switch_api_dtel_int_sink_ports_delete(
                #    device=device, port=swports[1])
                self.client.switch_api_dtel_int_endpoint_disable(
                    device=device)
            config.cleanup(self)

###############################################################################
@group('aasw38')
class ConfigAASW38(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Configure aasw38 (Mavericks 10.11.11.38)"
        sw = aasw38
	port_index = sw.port_index
        frontports = sw.frontports
        swports = sw.swports

        params = SwitchConfig_Params()
        params.switch_id = sw.switch_id
        params.mac_self = sw.mac_self
        params.nports = sw.nports
        params.ipaddr_inf = sw.ipaddr_inf
        params.ipaddr_nbr = sw.ipaddr_nbr
        params.mac_nbr = sw.mac_nbr
        params.routes = [(aah42.ipaddr_inf[0], sw.frontports.index("23/-"))]
        params.report_ports = [sw.frontports.index("1/3")]
        params.ipaddr_report_src = ['10.12.11.38']
        params.ipaddr_report_dst = [aah44.ipaddr_inf[1]]
        params.mirror_ids = [555]
        params.device = device
        params.swports = swports
        params.port_speed = SWITCH_PORT_SPEED_25G
        params.nports_max = 132
        # enable MoD
        mod_enabled = True
        try:
            config = SwitchHwConfig(self, params)
            switch_type = int(raw_input(
                'Enter switch type: 0 -- postcard, 1 -- INT endpoint : '))

            if (mod_enabled):
                self.client.switch_api_dtel_drop_report_enable(device)

            if switch_type == TYPE_POSTCARD:
                # enable Postcard
                self.client.switch_api_dtel_postcard_enable(device)
                # add flow space to postcard watchlist
                config_watchlist(self, postcard_watch=True, mod_watch=mod_enabled)
                self.client.switch_api_dtel_latency_quantization_shift(
                    device, quantization_shift)
                self.client.switch_api_dtel_flow_state_clear_cycle(
                    device, reset_cycle)
            elif switch_type == TYPE_INT_EP:
                # create an INT session with all metadata
                self.client.switch_api_dtel_int_session_create(
                    device=device, session_id=1,
                    instruction=convert_int_instruction(0xDC00), max_hop=8)

                # create INT watchlist
                config_watchlist(self, int_watch=True, int_session_id=1,
                                 mod_watch=mod_enabled, suppress=True)

                # set INT sink ports
                # TODO: automate configuration of sink ports?
                for i in range(0, 8):
                    self.client.switch_api_dtel_int_sink_ports_add(
                        device=device, port=swports[i])

                self.client.switch_api_dtel_int_endpoint_enable(device)
                self.client.switch_api_dtel_latency_quantization_shift(
                    device, quantization_shift)
                self.client.switch_api_dtel_flow_state_clear_cycle(
                    device, reset_cycle)
            else:
                print 'invalid switch type'

            raw_input("Press any key to cleanup...")

        # cleanup
        finally:
            if mod_enabled:
                self.client.switch_api_dtel_drop_report_disable(device)
                self.client.switch_api_dtel_drop_watchlist_clear(device)
            if switch_type == TYPE_POSTCARD:
                self.client.switch_api_dtel_postcard_watchlist_clear(device)
                self.client.switch_api_dtel_postcard_disable(device)
            elif switch_type == TYPE_INT_EP:
                self.client.switch_api_dtel_int_watchlist_clear(device)
                self.client.switch_api_dtel_int_session_delete(
                    device=device, session_id=1)
                #self.client.switch_api_dtel_int_sink_ports_delete(
                #    device=device, port=swports[1])
                self.client.switch_api_dtel_int_endpoint_disable(
                    device=device)
            config.cleanup(self)

###############################################################################

# for ptf integration tests, running aasw38 without NOS
@group('aasw38-q')
class ConfigAASW38q(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Configure aasw38 (Mavericks 10.11.11.38)"
        sw = aasw38
	port_index = sw.port_index
        frontports = sw.frontports
        swports = sw.swports

        params = SwitchConfig_Params()
        params.switch_id = sw.switch_id
        params.mac_self = sw.mac_self
        params.nports = sw.nports
        params.ipaddr_inf = sw.ipaddr_inf
        params.ipaddr_nbr = sw.ipaddr_nbr
        params.mac_nbr = sw.mac_nbr
        params.report_ports = [sw.frontports.index("1/0")]
        params.ipaddr_report_src = ['44.44.44.44']
        params.ipaddr_report_dst = ['2.2.2.2']
        params.mirror_ids = [555]
        params.device = device
        params.swports = swports
        params.port_speed = SWITCH_PORT_SPEED_25G
        params.nports_max = 132
        # enable MoD
        mod_enabled =  False
        switch_type = 0
        config = SwitchConfig(self, params)
        try:
            if (mod_enabled):
                self.client.switch_api_dtel_drop_report_enable(device)

            switch_type = int(raw_input(
                "Enter switch type: 0 -- postcard, 1 -- INT endpoint,"
                " 3 -- Queue endpoint: "))

            if switch_type == TYPE_POSTCARD:
                # enable Postcard
                self.client.switch_api_dtel_postcard_enable(device)
                # add flow space to postcard watchlist
                config_watchlist(self, postcard_watch=True, mod_watch=mod_enabled)
                self.client.switch_api_dtel_latency_quantization_shift(
                    device, quantization_shift)
                self.client.switch_api_dtel_flow_state_clear_cycle(
                    device, reset_cycle)
            elif switch_type == TYPE_INT_EP:
                # create an INT session with all metadata
                self.client.switch_api_dtel_int_session_create(
                    device=device, session_id=1,
                    instruction=convert_int_instruction(0xDC00), max_hop=8)

                # create INT watchlist
                config_watchlist(self, int_watch=True, int_session_id=1,
                                 mod_watch=mod_enabled, suppress=True)

                # set INT sink ports
                # TODO: automate configuration of sink ports?
                for i in range(0, 8):
                    self.client.switch_api_dtel_int_sink_ports_add(
                        device=device, port=swports[i])

                self.client.switch_api_dtel_int_endpoint_enable(device)
                self.client.switch_api_dtel_latency_quantization_shift(
                    device, quantization_shift)
                self.client.switch_api_dtel_flow_state_clear_cycle(
                    device, reset_cycle)
            elif switch_type == TYPE_QUEUEREPORT:
                config_watchlist(self, mod_watch=mod_enabled)
                threshold = int(raw_input('Queue threshold? '))
                quota = int(raw_input('Queue quota? '))
                dod = int(raw_input('Report tail drops (0:False, 1: True)? '))
                quantization = int(raw_input('Latency quantization? '))
                self.client.switch_api_dtel_queue_report_create(
                                        device, swports[1], 0, hex_to_i16(threshold),
                                        hex_to_i32(0xffffffff),
                                        hex_to_i16(quota), dod!=0)
                self.client.switch_api_dtel_latency_quantization_shift(
                    device, quantization)
            else:
                print 'invalid switch type'

            raw_input("Press any key to cleanup...")

        # cleanup
        finally:
            if mod_enabled:
                self.client.switch_api_dtel_drop_report_disable(device)
                self.client.switch_api_dtel_drop_watchlist_clear(device)
            if switch_type == TYPE_POSTCARD:
                self.client.switch_api_dtel_postcard_watchlist_clear(device)
                self.client.switch_api_dtel_postcard_disable(device)
            elif switch_type == TYPE_INT_EP:
                self.client.switch_api_dtel_int_watchlist_clear(device)
                self.client.switch_api_dtel_int_session_delete(
                    device=device, session_id=1)
                #self.client.switch_api_dtel_int_sink_ports_delete(
                #    device=device, port=swports[1])
                self.client.switch_api_dtel_int_endpoint_disable(
                    device=device)
            elif switch_type == TYPE_QUEUEREPORT:
                self.client.switch_api_dtel_queue_report_delete(
                                        device, swports[1], 0)
            config.cleanup(self)
###############################################################################

def hex_to_i64(h):
    x = int(h)
    if (x > 0x7FFFFFFFFFFFFFFF): x-= 0x10000000000000000
    return x

@group('probe-marker')
class ConfigAAProbe(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Configure probe marker"
        try:
            self.client.switch_api_telemetry_int_marker_set(
                device, 17, hex_to_i64(0xaaaaaaaabbbbbbbb))
            self.client.switch_api_telemetry_int_marker_set(
                device, 6, hex_to_i64(0xaaaaaaaabbbbbbbb))
            self.client.switch_api_telemetry_int_marker_set(
                device, 1, hex_to_i64(0xaaaaaaaabbbbbbbb))
            self.client.switch_api_telemetry_int_marker_port_add(
               device, 17, 5000, hex_to_i16(0x0000))
            self.client.switch_api_telemetry_int_marker_port_add(
               device, 6, 5000, hex_to_i16(0x0000))
            raw_input("Press any key to cleanup...")

        # cleanup
        finally:
            self.client.switch_api_telemetry_int_marker_port_clear(device, 17)
            self.client.switch_api_telemetry_int_marker_port_clear(device, 6)
            self.client.switch_api_telemetry_int_marker_delete(
                device, 1)
            self.client.switch_api_telemetry_int_marker_delete(
                device, 6)
            self.client.switch_api_telemetry_int_marker_delete(
                device, 17)
