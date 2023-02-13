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
Thrift SAI interface INT transit tests
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
sys.path.append(os.path.join(this_dir, '../../base'))
from common.utils import *

sys.path.append(os.path.join(this_dir, '../../base/sai-ocp-tests'))
import sai_base_test
from switch_utils import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '..'))
from dtel_utils import *
from dtel_sai_utils import *

SID = 0x11111111
swports = range(3)
devports = range(3)

params = SwitchSAIConfig_Params()
params.swports = swports
params.switch_id = SID
params.mac_self = '00:77:66:55:44:33'
params.nports = 3
params.ipaddr_inf = ['2.2.0.1',  '1.1.0.1', '10.1.0.4']
params.ipaddr_nbr = ['2.2.0.200', '1.1.0.100', '10.1.0.1']
params.mac_nbr = ['00:11:22:33:44:54', '00:11:22:33:44:55', '00:11:22:33:44:56']
params.report_ports = [2]
params.report_src = '4.4.4.1'
params.report_dst = ['4.4.4.3']
params.report_udp_port = UDP_PORT_DTEL_REPORT
params.report_truncate_size = 256
params.configure_routes = True

@group('drop')
@group('ep_l45')
@group('int_transit')
@group('postcard')
class INGRESS_DROP_REPORT_Test(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45/StFull one-hop Sink device for all metadata"
        bind_mirror_on_drop_pkt()

        # create SAI manager
        sai_mgr = SAIManager(self, params)

        # create report session according to params
        sai_mgr.create_dtel_report_session()

        drop_watchlist = sai_mgr.create_dtel_watchlist('Drop')

        drop_watchlist_entry = sai_mgr.create_dtel_watchlist_entry(
            watchlist=drop_watchlist,
            priority=10,
            ip_src=params.ipaddr_nbr[0],
            ip_src_mask='255.255.255.0',
            ip_dst=params.ipaddr_nbr[1],
            ip_dst_mask='255.255.255.0',
            dtel_drop_report_enable=True)

        payload = '@!#?'
        pkt = simple_udp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_id=108,
            ip_ttl=64,
            with_udp_chksum=False,
            udp_payload=payload)

        exp_pkt = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            with_udp_chksum=False,
            ip_id=108,
            ip_ttl=63,
            udp_payload=payload)

        # mod report packet
        exp_mod_inner = mod_report(
            packet=pkt,
            switch_id=SID,
            ingress_port=swports[0],
            egress_port=INVALID_PORT_ID,
            queue_id=0,
            drop_reason=80)

        exp_mod_pkt = ipv4_dtel_pkt(
            eth_dst=params.mac_nbr[params.report_ports[0]],
            eth_src=params.mac_self,
            ip_src=params.report_src,
            ip_dst=params.report_dst[0],
            ip_id=0,
            ip_ttl=64,
            next_proto=DTEL_REPORT_NEXT_PROTO_MOD,
            dropped=1,
            congested_queue=0,
            path_tracking_flow=0,
            hw_id=get_pipeid(
                swport_to_devport(self, swports[params.report_ports[0]])),
            inner_frame=exp_mod_inner)

        try:
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_no_other_packets(self)
            print "pass 1st packet w/o ACL drop"

            action_list = [SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION]
            packet_action = SAI_PACKET_ACTION_DROP
            in_ports = [sai_mgr.ports[0], sai_mgr.ports[1]]
            ip_src = params.ipaddr_nbr[0]
            ip_src_mask = "255.255.255.0"

            'Create ACL table'
            attr_list = []
            attr_value = sai_thrift_attribute_value_t(s32=SAI_ACL_STAGE_INGRESS)
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_TABLE_ATTR_ACL_STAGE, value=attr_value)
            attr_list.append(attr)

            bp_point = [SAI_ACL_BIND_POINT_TYPE_PORT]
            attr_value = sai_thrift_attribute_value_t(
                s32list=sai_thrift_s32_list_t(s32list=bp_point, count=1))
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_TABLE_GROUP_ATTR_ACL_BIND_POINT_TYPE_LIST,
                value=attr_value)
            attr_list.append(attr)

            attr_value = sai_thrift_attribute_value_t(booldata=1)
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_TABLE_ATTR_FIELD_SRC_IP, value=attr_value)
            attr_list.append(attr)

            acl_table_id = self.client.sai_thrift_create_acl_table(attr_list)

            'Create ACL entry'
            attr_list = []
            attr_value = sai_thrift_attribute_value_t(oid=acl_table_id)
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_ENTRY_ATTR_TABLE_ID, value=attr_value)
            attr_list.append(attr)

            attr_value = sai_thrift_attribute_value_t(u32=10)
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_ENTRY_ATTR_PRIORITY, value=attr_value)
            attr_list.append(attr)

            attr_value = sai_thrift_attribute_value_t(
                aclfield=sai_thrift_acl_field_data_t(
                    data=sai_thrift_acl_data_t(ip4=ip_src),
                    mask=sai_thrift_acl_mask_t(ip4=ip_src_mask)))
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP, value=attr_value)
            attr_list.append(attr)

            attr_value = sai_thrift_attribute_value_t(
                aclfield=sai_thrift_acl_field_data_t(data=sai_thrift_acl_data_t(
                                                     s32=packet_action)))
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_ENTRY_ATTR_ACTION_PACKET_ACTION,
                value=attr_value)
            attr_list.append(attr)

            acl_entry_id = self.client.sai_thrift_create_acl_entry(attr_list)

            for port in in_ports:
                attr = sai_thrift_attribute_t(
                    id=SAI_PORT_ATTR_INGRESS_ACL,
                    value=sai_thrift_attribute_value_t(oid=acl_table_id))
                self.client.sai_thrift_set_port_attribute(port, attr)

            send_packet(self, swports[0], str(pkt))
            verify_no_other_packets(self, timeout=1)

            print "pass 2nd packet w/ ACL drop"

            sai_mgr.switch.dtel_drop_report_enable = True

            send_packet(self, swports[0], str(pkt))
            #receive_print_packet(
            #    self, swports[params.report_ports[0]], exp_mod_pkt, True)
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self, timeout=1)
            print "pass 3rd packet w/ drop report"

            event = sai_mgr.create_dtel_event(
                SAI_DTEL_EVENT_TYPE_DROP_REPORT, 5);
            exp_mod_pkt[IP].tos = 5<<2
            send_packet(self, swports[0], str(pkt))
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self, timeout=1)
            print "pass 4th packet w/ drop report DSCP 5"

            event.dscp_value = 9
            exp_mod_pkt[IP].tos = 9<<2
            send_packet(self, swports[0], str(pkt))
            verify_dtel_packet(
                self, exp_mod_pkt, swports[params.report_ports[0]])
            verify_no_other_packets(self)
            print "pass 5th packet w/ drop report DSCP 9"

        finally:
            ### Cleanup
            for port in in_ports:
                attr = sai_thrift_attribute_t(
                    id=SAI_PORT_ATTR_INGRESS_ACL,
                    value=sai_thrift_attribute_value_t(oid=SAI_NULL_OBJECT_ID))
                self.client.sai_thrift_set_port_attribute(port, attr)
            self.client.sai_thrift_remove_acl_entry(acl_entry_id)
            self.client.sai_thrift_remove_acl_table(acl_table_id)
            sai_mgr.switch.dtel_drop_report_enable = False
            sai_mgr.cleanup()
