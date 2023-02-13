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
Thrift API interface Mirror on Drop tests
"""

import logging
import os
import random
import switchapi_thrift
import sys
import time
import unittest

import ptf.dataplane as dataplane

from ptf.testutils import *
from ptf.thriftutils import *
from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../../base'))
from common.utils import *
sys.path.append(os.path.join(this_dir, '../../../base/api-tests'))
import api_base_tests

sys.path.append(os.path.join(this_dir, '../..'))
from dtel_utils import *

import pd_base_tests

device = 0
swports = range(4)
devports = range(4)
switch_id = 0x11111111

params = SwitchConfig_Params()
params.switch_id = switch_id
params.mac_self = '00:77:66:55:44:33'
params.nports = 3
params.ipaddr_inf = ['10.1.0.1',  '10.2.0.1',  '10.3.0.1']
params.ipaddr_nbr = ['10.1.0.11', '10.2.0.12', '10.3.0.13']
params.mac_nbr = ['00:11:22:33:44:55', '00:11:22:33:44:56', '00:11:22:33:44:57']
params.report_ports = [2]
params.ipaddr_report_src = ['4.4.4.1']
params.ipaddr_report_dst = ['4.4.4.3']
params.mirror_ids = [1015]
params.device = device
params.swports = swports

twl_kvp = []
kvp_val = switcht_twl_value_t(
    value_num=ipv4Addr_to_i32(params.ipaddr_nbr[0]))
kvp_mask = switcht_twl_value_t(value_num=0xffffff00)
twl_kvp.append(switcht_twl_key_value_pair_t(
    SWITCH_TWL_FIELD_IPV4_SRC, kvp_val, kvp_mask))
kvp_val = switcht_twl_value_t(
    value_num=ipv4Addr_to_i32(params.ipaddr_nbr[1]))
kvp_mask = switcht_twl_value_t(value_num=0xffffff00)
twl_kvp.append(switcht_twl_key_value_pair_t(
    SWITCH_TWL_FIELD_IPV4_DST, kvp_val, kvp_mask))

###############################################################################


@group('postcard_dod')
@group('ep_l45_dod')
@group('transit_l45_dod')
class MirrorOnDropDoDTest(api_base_tests.ThriftInterfaceDataPlane,
                          pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print
        print "Test drop report due to DoD"
        if test_param_get('target') == 'bmv2':
            return
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        bind_mirror_on_drop_pkt()
        config = SwitchConfig(self, params)

    # run test
        try:
            pkt_in = simple_tcp_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                ip_ttl=64,
                pktlen=256)

            exp_pkt_out = simple_tcp_packet(
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                ip_ttl=63,
                pktlen=256)

            input_port=swports[0]

            exp_mod_inner = mod_report(
                packet=pkt_in,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                drop_reason=71)  # drop traffic manager

            exp_mod_pkt = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_MOD,
                dropped=1,
                congested_queue=0,
                path_tracking_flow=0,
                hw_id=hw_id,
                inner_frame=exp_mod_inner)

            self.client.switch_api_dtel_drop_report_enable(device)

            ap = switcht_twl_mod_params_t(report_queue_tail_drops=True)
            self.client.switch_api_dtel_drop_watchlist_entry_create(
                device, twl_kvp, priority=1, watch=True, action_params=ap)

            # 10th pkt is dropped by the model
            # pay your attention that counter is enabled once MoD is enabled
            for n in range(0, 9):
                print "Sending pkt ", n
                send_packet(self, swports[0], str(pkt_in))
                verify_packet(self, exp_pkt_out, swports[1])
                verify_no_other_packets(self)

            # verify mirrored packet
            print "send pkt to be mirrored"
            send_packet(self, swports[0], str(pkt_in))
            # verify mirror on drop packet
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            #receive_print_packet(self, swports[2], exp_mod_pkt, True, False)
            verify_no_other_packets(self)
            print "Passed mirror on drop caused by TM decimation"

    # cleanup
        finally:
            self.client.switch_api_dtel_drop_report_disable(device)
            self.client.switch_api_dtel_drop_watchlist_clear(device)
            config.cleanup(self)


###############################################################################

@group('postcard')
@group('ep_l45')
@group('transit_l45')
class MirrorOnDropIngressAclTest(api_base_tests.ThriftInterfaceDataPlane,
                                 pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print
        print "Test drop report due to ingress ACL"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        bind_mirror_on_drop_pkt()
        config = SwitchConfig(self, params)

    # run test
        try:
            pkt_in = simple_tcp_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                ip_ttl=64,
                pktlen=256)

            exp_pkt_out = simple_tcp_packet(
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                ip_ttl=63,
                pktlen=256)

            exp_mod_inner = mod_report(
                packet=pkt_in,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=INVALID_PORT_ID,
                queue_id=0,
                drop_reason=80)  # drop acl deny

            exp_mod_pkt = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_MOD,
                dropped=1,
                congested_queue=0,
                path_tracking_flow=0,
                hw_id=hw_id,
                inner_frame=exp_mod_inner)

            self.client.switch_api_dtel_drop_report_enable(device)
            ap = switcht_twl_mod_params_t(report_queue_tail_drops=False)
            self.client.switch_api_dtel_drop_watchlist_entry_create(
                device, twl_kvp, priority=1, watch=True, action_params=ap)

            # create IP ACL on port 1 to deny packets
            acl = self.client.switch_api_acl_list_create(
                0, SWITCH_API_DIRECTION_INGRESS, 0, SWITCH_HANDLE_TYPE_PORT)
            acl_kvp = []
            acl_kvp_val = switcht_acl_value_t(
                value_num=ipv4Addr_to_i32(params.ipaddr_nbr[0]))
            acl_kvp_mask = switcht_acl_value_t(value_num=0xffffff00)
            acl_kvp.append(switcht_acl_key_value_pair_t(
                SWITCH_ACL_IP_FIELD_IPV4_SRC, acl_kvp_val, acl_kvp_mask))
            acl_kvp_val = switcht_acl_value_t(
                value_num=ipv4Addr_to_i32(params.ipaddr_nbr[1]))
            acl_kvp_mask = switcht_acl_value_t(value_num=0xffffff00)
            acl_kvp.append(switcht_acl_key_value_pair_t(
                SWITCH_ACL_IP_FIELD_IPV4_DEST, acl_kvp_val, acl_kvp_mask))
            action = 1
            action_params = switcht_acl_action_params_t(
                redirect=switcht_acl_action_redirect(handle=0))
            opt_action_params = switcht_acl_opt_action_params_t()
            ace = self.client.switch_api_acl_ip_rule_create(
                0, acl, 10, 2, acl_kvp, action, action_params,
                opt_action_params)
            port = self.client.switch_api_port_id_to_handle_get(0, swports[0])
            self.client.switch_api_acl_reference(0, acl, port)

            # send from port 3, should pass
            send_packet(self, swports[2], str(pkt_in))
            verify_packet(self, exp_pkt_out, swports[1])
            verify_no_other_packets(self)

            # send from port 1, should drop
            send_packet(self, swports[0], str(pkt_in))
            # verify mirror on drop packet
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            #receive_print_packet(self, swports[2], exp_mod_pkt, True, False)
            verify_no_other_packets(self)
            print "Passed mirror on drop caused by ingress ACL"

            old_type = pkt_in[Ether].type
            pkt_in[Ether].type = 0x1234
            send_packet(self, swports[2], str(pkt_in))
            verify_no_other_packets(self)
            pkt_in[Ether].type = old_type
            print "Passed mirror on drop watchlist for non-IP packets"

    # cleanup
        finally:
            self.client.switch_api_dtel_drop_report_disable(device)
            self.client.switch_api_dtel_drop_watchlist_clear(device)
            self.client.switch_api_acl_dereference(0, acl, port)
            self.client.switch_api_acl_rule_delete(0, acl, ace)
            self.client.switch_api_acl_list_delete(0, acl)
            config.cleanup(self)

###############################################################################

@group('postcard')
@group('ep_l45')
@group('transit_l45')
class MirrorOnDropEgressAclTest(api_base_tests.ThriftInterfaceDataPlane,
                                pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print
        print "Test drop report due to egress ACL"
        print "NOTE: Egress ACL is not supported in profiles. Skip this test."
        return
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        bind_mirror_on_drop_pkt()
        config = SwitchConfig(self, params)

    # run test
        try:
            pkt_in = simple_tcp_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                ip_ttl=64,
                pktlen=256)

            exp_pkt_out = simple_tcp_packet(
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                ip_ttl=63,
                pktlen=256)

            exp_mod_inner = mod_report(
                packet=exp_pkt_out,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                drop_reason=92)  # drop egress acl deny

            exp_mod_pkt = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_MOD,
                dropped=1,
                congested_queue=0,
                path_tracking_flow=0,
                hw_id=hw_id,
                inner_frame=exp_mod_inner)

            self.client.switch_api_dtel_drop_report_enable(device)
            ap = switcht_twl_mod_params_t(report_queue_tail_drops=False)
            self.client.switch_api_dtel_drop_watchlist_entry_create(
                device, twl_kvp, priority=1, watch=True, action_params=ap)

            # send packet, should pass
            send_packet(self, swports[0], str(pkt_in))
            verify_packet(self, exp_pkt_out, swports[1])
            verify_no_other_packets(self)

            # create IP ACL on port 1 to deny packets
            acl = self.client.switch_api_acl_list_create(
                device, SWITCH_API_DIRECTION_EGRESS, 0, SWITCH_HANDLE_TYPE_PORT)
            acl_kvp = []
            acl_kvp_val = switcht_acl_value_t(
                value_num=ipv4Addr_to_i32(params.ipaddr_nbr[0]))
            acl_kvp_mask = switcht_acl_value_t(value_num=0xffffff00)
            acl_kvp.append(switcht_acl_key_value_pair_t(
                SWITCH_ACL_IP_FIELD_IPV4_SRC, acl_kvp_val, acl_kvp_mask))
            acl_kvp_val = switcht_acl_value_t(
                value_num=ipv4Addr_to_i32(params.ipaddr_nbr[1]))
            acl_kvp_mask = switcht_acl_value_t(value_num=0xffffff00)
            acl_kvp.append(switcht_acl_key_value_pair_t(
                SWITCH_ACL_IP_FIELD_IPV4_DEST, acl_kvp_val, acl_kvp_mask))
            action = 1
            action_params = switcht_acl_action_params_t(
                redirect=switcht_acl_action_redirect(handle=0))
            opt_action_params = switcht_acl_opt_action_params_t()
            ace = self.client.switch_api_acl_ip_rule_create(
                device, acl, 10, 2, acl_kvp, action, action_params,
                opt_action_params)
            port = self.client.switch_api_port_id_to_handle_get(
                device, swports[1])
            self.client.switch_api_acl_reference(device, acl, port)

            send_packet(self, swports[0], str(pkt_in))
            # verify mirror on drop packet
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed mirror on drop caused by egress ACL"

    # cleanup
        finally:
            self.client.switch_api_dtel_drop_report_disable(device)
            self.client.switch_api_dtel_drop_watchlist_entry_delete(
                device=device, twl_kvp=twl_kvp)
            self.client.switch_api_acl_dereference(0, acl, port)
            self.client.switch_api_acl_rule_delete(0, acl, ace)
            self.client.switch_api_acl_list_delete(0, acl)
            config.cleanup(self)


###############################################################################


@group('postcard')
@group('ep_l45')
@group('transit_l45')
class MirrorOnDropNonDefaultRuleTest(api_base_tests.ThriftInterfaceDataPlane,
                                     pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print
        print "Test drop report due to user defined system ACL rule for same BD"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        bind_mirror_on_drop_pkt()
        params.vlans = {2:{0:0, 1:0}}
        config = SwitchConfig(self, params)

    # run test
        try:
            pkt_in = simple_tcp_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                ip_ttl=64,
                pktlen=256)

            exp_pkt_out = simple_tcp_packet(
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                ip_ttl=63,
                pktlen=256)

            exp_mod_inner = mod_report(
                packet=pkt_in,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=INVALID_PORT_ID,
                queue_id=0,
                drop_reason=58)  # drop same ifindex

            exp_mod_pkt = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_MOD,
                dropped=1,
                congested_queue=0,
                path_tracking_flow=0,
                hw_id=hw_id,
                inner_frame=exp_mod_inner)

            # drop watchlist configuration
            self.client.switch_api_dtel_drop_report_enable(device)
            ap = switcht_twl_mod_params_t(report_queue_tail_drops=False)
            self.client.switch_api_dtel_drop_watchlist_entry_create(
                device, twl_kvp, priority=1, watch=True, action_params=ap)

            # create system_acl rule to drop same bd packets
            acl = self.client.switch_api_acl_list_create(
                device, SWITCH_API_DIRECTION_INGRESS, SWITCH_ACL_TYPE_SYSTEM,
                SWITCH_HANDLE_TYPE_NONE)
            acl_kvp = []
            acl_kvp_val = switcht_acl_value_t(value_num=1)
            acl_kvp_mask = switcht_acl_value_t(value_num=1)
            acl_kvp.append(switcht_acl_key_value_pair_t(
                SWITCH_ACL_SYSTEM_FIELD_ROUTED, acl_kvp_val, acl_kvp_mask))
            acl_kvp_val = switcht_acl_value_t(value_num=0)
            acl_kvp_mask = switcht_acl_value_t(value_num=0xffff)
            acl_kvp.append(switcht_acl_key_value_pair_t(
                SWITCH_ACL_SYSTEM_FIELD_BD_CHECK, acl_kvp_val, acl_kvp_mask))
            action = SWITCH_ACL_ACTION_DROP
            action_params = switcht_acl_action_params_t(
                drop=switcht_acl_action_drop(reason_code=58))  # same ifindex
            opt_action_params = switcht_acl_opt_action_params_t()
            ace = self.client.switch_api_acl_system_rule_create(
                device, acl, 3000, 2, acl_kvp, action, action_params,
                opt_action_params)

            # send packet to destination on the same vlan, should drop
            send_packet(self, swports[0], str(pkt_in))
            # verify mirror on drop packet
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed mirror on drop caused by same BD check"

            # modify system_acl rule to permit same bd packets
            action = SWITCH_ACL_ACTION_PERMIT
            self.client.switch_api_acl_entry_action_set(
                device, ace, 3000, action, action_params, opt_action_params)

            # send packet to destination on the same vlan
            send_packet(self, swports[0], str(pkt_in))
            verify_packet(self, exp_pkt_out, swports[1])
            verify_no_other_packets(self)
            print "Passed sending packet when same BD is allowed"

            # modify system_acl rule to drop same bd packets
            action = SWITCH_ACL_ACTION_DROP
            self.client.switch_api_acl_entry_action_set(
                device, ace, 3000, action, action_params, opt_action_params)

            # send packet to destination on the same vlan, should drop
            send_packet(self, swports[0], str(pkt_in))
            # verify mirror on drop packet
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed mirror on drop caused by same BD check after modify"

            # modify system_acl rule so that it does not specify drop reason
            action_params = switcht_acl_action_params_t(
                drop=switcht_acl_action_drop(reason_code=0))
            exp_mod_pkt[MOD_HDR].drop_reason = 254  # drop others ingress
            self.client.switch_api_acl_entry_action_set(
                device, ace, 3000, action, action_params, opt_action_params)

            # send packet to destination on the same vlan, should drop
            send_packet(self, swports[0], str(pkt_in))
            # verify mirror on drop packet
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed mirror on drop with no drop reason"

    # cleanup
        finally:
            self.client.switch_api_dtel_drop_report_disable(device)
            self.client.switch_api_dtel_drop_watchlist_clear(device)
            self.client.switch_api_acl_rule_delete(device, acl, ace)
            self.client.switch_api_acl_list_delete(device, acl)
            params.vlans = None
            config.cleanup(self)


###############################################################################


@group('postcard')
@group('ep_l45')
@group('transit_l45')
class MirrorOnDropEgrNonDefaultRuleTest(api_base_tests.ThriftInterfaceDataPlane,
                                        pd_base_tests.ThriftInterfaceDataPlane):
    def __init__(self):
        pd_base_tests.ThriftInterfaceDataPlane.__init__(self, ["switch"],
                                                        ["dc"])

    def runTest(self):
        print
        print "Test drop report due to user defined system ACL rule for MTU"
        pd_base_tests.ThriftInterfaceDataPlane.setUp(self)
        hw_id = get_pipeid(swport_to_devport(self, swports[params.report_ports[0]]))
        api_base_tests.ThriftInterfaceDataPlane.setUp(self)

        bind_mirror_on_drop_pkt()
        config = SwitchConfig(self, params)

        mtu_set = False

    # run test
        try:
            pkt_in = simple_tcp_packet(
                eth_dst=params.mac_self,
                eth_src=params.mac_nbr[0],
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                ip_ttl=64,
                pktlen=256)

            exp_pkt_out = simple_tcp_packet(
                eth_dst=params.mac_nbr[1],
                eth_src=params.mac_self,
                ip_dst=params.ipaddr_nbr[1],
                ip_src=params.ipaddr_nbr[0],
                ip_id=105,
                ip_ttl=63,
                pktlen=256)

            exp_mod_inner = mod_report(
                packet=exp_pkt_out,
                switch_id=switch_id,
                ingress_port=swports[0],
                egress_port=swports[1],
                queue_id=0,
                drop_reason=70)  # drop mtu check fail

            exp_mod_pkt = ipv4_dtel_pkt(
                eth_dst=params.mac_nbr[params.report_ports[0]],
                eth_src=params.mac_self,
                ip_src=params.ipaddr_report_src[0],
                ip_dst=params.ipaddr_report_dst[0],
                ip_id=0,
                ip_ttl=64,
                next_proto=DTEL_REPORT_NEXT_PROTO_MOD,
                dropped=1,
                congested_queue=0,
                path_tracking_flow=0,
                hw_id=hw_id,
                inner_frame=exp_mod_inner)

            # drop watchlist configuration
            self.client.switch_api_dtel_drop_report_enable(device)
            ap = switcht_twl_mod_params_t(report_queue_tail_drops=False)
            self.client.switch_api_dtel_drop_watchlist_entry_create(
                device, twl_kvp, priority=1, watch=True, action_params=ap)

            # create system_acl rule to drop packets exceeding MTU
            acl = self.client.switch_api_acl_list_create(
                device, SWITCH_API_DIRECTION_EGRESS,
                SWITCH_ACL_TYPE_EGRESS_SYSTEM, SWITCH_HANDLE_TYPE_NONE)
            acl_kvp = []
            acl_kvp_val = switcht_acl_value_t(value_num=0)
            acl_kvp_mask = switcht_acl_value_t(value_num=1)
            acl_kvp.append(switcht_acl_key_value_pair_t(
                SWITCH_ACL_EGRESS_SYSTEM_FIELD_DEFLECT, acl_kvp_val, acl_kvp_mask))
            acl_kvp_val = switcht_acl_value_t(value_num=0)
            acl_kvp_mask = switcht_acl_value_t(value_num=0xffff)
            acl_kvp.append(switcht_acl_key_value_pair_t(
                SWITCH_ACL_EGRESS_SYSTEM_FIELD_L3_MTU_CHECK, acl_kvp_val, acl_kvp_mask))
            action = SWITCH_ACL_EGRESS_SYSTEM_ACTION_DROP
            action_params = switcht_acl_action_params_t(
                drop=switcht_acl_action_drop(reason_code=70))  # mtu check fail
            opt_action_params = switcht_acl_opt_action_params_t()
            ace = self.client.switch_api_acl_egress_system_rule_create(
                device, acl, 3000, 2, acl_kvp, action, action_params,
                opt_action_params)

            # set MTU to 200
            mtu_set = True
            mtu_200 = self.client.switch_api_l3_mtu_create(
                device, SWITCH_MTU_TYPE_IPV4, 200)
            self.client.switch_api_rif_mtu_set(
                device, config.rifs[1], mtu_200)

            # send oversize packet to destination, should drop
            send_packet(self, swports[0], str(pkt_in))
            # verify mirror on drop packet
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed mirror on drop due to user defined MTU rule"

            # modify system_acl rule to permit packets exceeding MTU
            action = SWITCH_ACL_EGRESS_SYSTEM_ACTION_PERMIT
            self.client.switch_api_acl_entry_egress_system_action_set(
                device, ace, 3000, action, action_params, opt_action_params)

            # send oversize packet to destination
            send_packet(self, swports[0], str(pkt_in))
            verify_packet(self, exp_pkt_out, swports[1])
            verify_no_other_packets(self)
            print "Passed sending oversize packet that is permitted"

            # modify system_acl rule to drop packets exceeding MTU
            action = SWITCH_ACL_EGRESS_SYSTEM_ACTION_DROP
            action_params = switcht_acl_action_params_t(
                drop=switcht_acl_action_drop(reason_code=70))  # mtu check fail
            self.client.switch_api_acl_entry_egress_system_action_set(
                device, ace, 3000, action, action_params, opt_action_params)

            # send oversize packet to destination, should drop
            send_packet(self, swports[0], str(pkt_in))
            # verify mirror on drop packet
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed mirror on drop due to user MTU rule after modify"

            # modify system_acl rule so that it does not specify drop reason
            action_params = switcht_acl_action_params_t(
                drop=switcht_acl_action_drop(reason_code=0))
            exp_mod_pkt[MOD_HDR].drop_reason = 255  # drop others egress
            self.client.switch_api_acl_entry_egress_system_action_set(
                device, ace, 3000, action, action_params, opt_action_params)

            # send packet to destination on the same vlan, should drop
            send_packet(self, swports[0], str(pkt_in))
            # verify mirror on drop packet
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "Passed mirror on drop with no drop reason"

    # cleanup
        finally:
            if mtu_set:
                self.client.switch_api_l3_mtu_delete(device, mtu_200)
            self.client.switch_api_dtel_drop_report_disable(device)
            self.client.switch_api_dtel_drop_watchlist_clear(device)
            self.client.switch_api_acl_rule_delete(device, acl, ace)
            self.client.switch_api_acl_list_delete(device, acl)
            config.cleanup(self)
