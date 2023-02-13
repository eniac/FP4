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
import os, sys, copy, pdb
import switchsai_thrift

from switchsai_thrift.ttypes import *
from switchsai_thrift.sai_headers import *
from dtel_utils import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../base/sai-ocp-tests'))
import sai_base_test
from switch_utils import *


def get_SAI_ip_address_family_code(ip_address):
    try:
        # Test if IPv4
        socket.inet_pton(socket.AF_INET, ip_address)
        return SAI_IP_ADDR_FAMILY_IPV4
    except socket.error:
        try:
            # Test IPv6
            socket.inet_pton(socket.AF_INET6, ip_address)
            return SAI_IP_ADDR_FAMILY_IPV6
        except socket.error:
            print(ip_address, ' not valid.')
            sys.exit()

def check_ip_address_validity(params):
    for ip_address in params.ipaddr_inf \
            + params.ipaddr_nbr \
            + params.report_src \
            + params.report_dst:
        get_SAI_ip_address_family_code(ip_address)

class SwitchSAIConfig_Params():
    def __init__(self):
        self.swports = range(64)
        self.switch_id = 0x11111111;
        self.mac_self = '00:77:66:55:44:33'
        self.nports = 0
        self.ipaddr_inf = ['10.1.0.1',  '10.2.0.1']
        self.ipaddr_nbr = ['10.11.0.1', '10.12.0.1']
        self.mac_nbr = ['00:11:22:33:44:55', '00:11:22:33:44:56']
        self.report_ports = [1]
        self.report_src = '4.4.4.1'
        self.report_dst = ['4.4.4.3']
        self.report_udp_port = 32766
        self.report_truncate_size = 256
        self.configure_routes = False
        self.routes = [('192.168.10.1', 1)]

class SAIManager(object):
    def __init__(self, test, params):
        self.client = test.client
        self.params = copy.deepcopy(params)
        if self.params.configure_routes:
            switch_init(self.client)
            sai_thrift_remove_default_bridge_ports(
                self.client, [port_list[i] for i in params.swports])
        self.switch = SAISwitch(self)
        self.ports = [self.switch.all_ports[self.switch.hw_lanes.index(port)]
                      for port in params.swports]
        self.router_interfaces = []
        self.next_hops = []
        self.routes = []
        self.neighbors = []
        self.queue_reports = []
        self.report_session = None
        self.int_sessions = []
        self.watchlists = []
        self.events = []
        if self.params.configure_routes:
            self.configure_routes()

    def create_dtel_int_session(
            self,
            max_hop_count=8,
            collect_switch_id=True,
            collect_switch_ports=True,
            collect_ig_timestamp=True,
            collect_eg_timestamp=True,
            collect_queue_info=True):
        return DTelINTSession(
            self,
            max_hop_count,
            collect_switch_id,
            collect_switch_ports,
            collect_ig_timestamp,
            collect_eg_timestamp,
            collect_queue_info)

    def create_dtel_watchlist(self, watchlist_type):
        return DTelWatchlist(self, watchlist_type)

    def create_dtel_watchlist_entry(
            self,
            watchlist,
            priority=None,
            ether_type=None,
            ether_type_mask=None,
            ip_src=None,
            ip_src_mask=None,
            ip_dst=None,
            ip_dst_mask=None,
            ip_proto=None,
            ip_proto_mask=None,
            dscp=None,
            dscp_mask=None,
            l4_src_port=None,
            l4_src_port_mask=None,
            l4_dst_port=None,
            l4_dst_port_mask=None,
            tunnel_vni=None,
            tunnel_vni_mask=None,
            inner_ether_type=None,
            inner_ether_type_mask=None,
            inner_src_ip=None,
            inner_src_ip_mask=None,
            inner_dst_ip=None,
            inner_dst_ip_mask=None,
            inner_ip_proto=None,
            inner_ip_proto_mask=None,
            inner_l4_src_port=None,
            inner_l4_src_port_mask=None,
            inner_l4_dst_port=None,
            inner_l4_dst_port_mask=None,
            range_list=None,
            dtel_int_enable=None,
            dtel_int_session=None,
            dtel_postcard_enable=None,
            dtel_sample_percent=None,
            dtel_report_all=None,
            dtel_drop_report_enable=None):
        return DTelWatchlistEntry(
            watchlist,
            priority,
            ether_type,
            ether_type_mask,
            ip_src,
            ip_src_mask,
            ip_dst,
            ip_dst_mask,
            ip_proto,
            ip_proto_mask,
            dscp,
            dscp_mask,
            l4_src_port,
            l4_src_port_mask,
            l4_dst_port,
            l4_dst_port_mask,
            tunnel_vni,
            tunnel_vni_mask,
            inner_ether_type,
            inner_ether_type_mask,
            inner_src_ip,
            inner_src_ip_mask,
            inner_dst_ip,
            inner_dst_ip_mask,
            inner_ip_proto,
            inner_ip_proto_mask,
            inner_l4_src_port,
            inner_l4_src_port_mask,
            inner_l4_dst_port,
            inner_l4_dst_port_mask,
            range_list,
            dtel_int_enable,
            dtel_int_session,
            dtel_postcard_enable,
            dtel_sample_percent,
            dtel_report_all,
            dtel_drop_report_enable)

    def create_dtel_queue_report(self, port, queue, depth, latency, quota, drop):
        return DTelQueueReport(self, port, queue, depth, latency, quota, drop)

    def create_dtel_report_session(self):
        params = self.params
        return DTelReportSession(self,
                                 params.report_src,
                                 params.report_dst,
                                 params.report_udp_port,
                                 params.report_truncate_size)

    def create_dtel_event(self, event_type, dscp):
        params = self.params
        return DTelEvent(self, event_type, dscp)

    def configure_routes(self):
        params = self.params
        router_ifs = []
        for i in params.swports:
            rif = SAIRouterInterface(self, self.ports[i])
            router_ifs.append(rif)
        neighbors = []
        for i in range(len(params.mac_nbr)):
            neighbor = SAINeighbor(
                self, router_ifs[i], params.ipaddr_nbr[i], params.mac_nbr[i])
            neighbors.append(neighbor)
        next_hops = []
        for i in range(len(params.ipaddr_nbr)):
            nhop = SAINextHop(self, params.ipaddr_nbr[i], router_ifs[i])
            next_hops.append(nhop)
        if params.report_ports:
            for i, port in enumerate(params.report_ports):
                route = (params.report_dst[i], port)
                if route not in params.routes:
                    params.routes.append(route)
        if params.routes:
            for i, route in enumerate(params.routes):
                if len(route) <= 2:
                    params.routes[i] = (route[0], route[1], '255.255.255.255')
            routes = [SAIRoute(self, route[0], route[2], next_hops[route[1]])
                      for route in params.routes]

    def cleanup(self):
        self.switch.dtel_int_sink_port_list = []
        for watchlist in list(self.watchlists):
            watchlist.delete()
        for queue_report in list(self.queue_reports):
            queue_report.delete()
        for int_session in list(self.int_sessions):
            int_session.delete()
        for event in list(self.events):
            event.delete()
        if self.report_session:
            self.report_session.delete()

        if self.params.configure_routes:
            for route in list(self.routes):
                route.delete()
            for neighbor in list(self.neighbors):
                neighbor.delete()
            for next_hop in list(self.next_hops):
                next_hop.delete()
            for router_interface in list(self.router_interfaces):
                router_interface.delete()
            sai_thrift_create_default_bridge_ports(
                self.client, [port_list[i] for i in self.params.swports])


class DTelQueueReport(object):
    def __init__(self, sai_mgr, port, queue, depth, latency, quota, drop):
        self.sai_mgr = sai_mgr
        self.client = sai_mgr.client
        self.port = sai_mgr.switch.all_ports[port]
        self._depth_threshold = depth
        self._latency_threshold = latency
        self._quota = quota
        self._drop = drop
        self.queue = queue
        self.queue_id = None

        queue_list = []
        port_attr_list = self.client.sai_thrift_get_port_attribute(self.port)
        attr_list = port_attr_list.attr_list
        for attribute in attr_list:
            if attribute.id == SAI_PORT_ATTR_QOS_QUEUE_LIST:
                for queue_id in attribute.value.objlist.object_id_list:
                    queue_list.append(queue_id)


        #attr = sai_thift_attribute_t(id=SAI_PORT_ATTR_QOS_QUEUE_LIST)
        #queue_list = self.client.sai_thrift_get_port_attribute([attr], self.port)
        self.queue_id = queue_list[queue]

        attr1 = sai_thrift_attribute_t(
            id=SAI_DTEL_QUEUE_REPORT_ATTR_QUEUE_ID,
            value=sai_thrift_attribute_value_t(oid=self.queue_id))
        attr2 = sai_thrift_attribute_t(
            id=SAI_DTEL_QUEUE_REPORT_ATTR_DEPTH_THRESHOLD,
            value=sai_thrift_attribute_value_t(u32=depth))
        attr3 = sai_thrift_attribute_t(
            id=SAI_DTEL_QUEUE_REPORT_ATTR_LATENCY_THRESHOLD,
            value=sai_thrift_attribute_value_t(u32=latency))
        attr4 = sai_thrift_attribute_t(
            id=SAI_DTEL_QUEUE_REPORT_ATTR_BREACH_QUOTA,
            value=sai_thrift_attribute_value_t(u32=quota))
        attr5 = sai_thrift_attribute_t(
            id=SAI_DTEL_QUEUE_REPORT_ATTR_TAIL_DROP,
            value=sai_thrift_attribute_value_t(booldata=drop))
        attr_list = [attr1, attr2, attr3, attr4, attr5]
        self.id = self.client.sai_thrift_create_dtel_queue_report(attr_list)
        self.sai_mgr.queue_reports.append(self)

    @property
    def depth_threshold(self):
        return self._depth_threshold

    @depth_threshold.setter
    def depth_threshold(self, value):
        attr = sai_thrift_attribute_t(
            id=SAI_DTEL_QUEUE_REPORT_ATTR_DEPTH_THRESHOLD,
            value=sai_thrift_attribute_value_t(u32=value))
        self.client.sai_thrift_set_dtel_queue_report_attribute(self.id, attr)
        self._depth_threshold = value

    @property
    def latency_threshold(self):
        return self._latency_threshold

    @latency_threshold.setter
    def latency_threshold(self, value):
        attr = sai_thrift_attribute_t(
            id=SAI_DTEL_QUEUE_REPORT_ATTR_LATENCY_THRESHOLD,
            value=sai_thrift_attribute_value_t(u32=value))
        self.client.sai_thrift_set_dtel_queue_report_attribute(self.id, attr)
        self._latency_threshold = value

    def delete(self):
        self.client.sai_thrift_remove_dtel_queue_report(self.id)
        self.sai_mgr.queue_reports.remove(self)

class DTelINTSession(object):
    def __init__(self,
                 sai_mgr,
                 max_hop_count,
                 collect_switch_id=True,
                 collect_switch_ports=True,
                 collect_ig_timestamp=True,
                 collect_eg_timestamp=True,
                 collect_queue_info=True):
        self.sai_mgr = sai_mgr
        self.client = sai_mgr.client
        self.max_hop_count = max_hop_count
        self.collect_switch_id = collect_switch_id
        self.collect_switch_ports = collect_switch_ports
        self.collect_ig_timestamp = collect_ig_timestamp
        self.collect_eg_timestamp = collect_eg_timestamp
        self.collect_queue_info = collect_queue_info

        attr1 = sai_thrift_attribute_t(
            id=SAI_DTEL_INT_SESSION_ATTR_MAX_HOP_COUNT,
            value=sai_thrift_attribute_value_t(u8=max_hop_count))
        attr2 = sai_thrift_attribute_t(
            id=SAI_DTEL_INT_SESSION_ATTR_COLLECT_SWITCH_ID,
            value=sai_thrift_attribute_value_t(booldata=collect_switch_id))
        attr3 = sai_thrift_attribute_t(
            id=SAI_DTEL_INT_SESSION_ATTR_COLLECT_SWITCH_PORTS,
            value=sai_thrift_attribute_value_t(booldata=collect_switch_ports))
        attr4 = sai_thrift_attribute_t(
            id=SAI_DTEL_INT_SESSION_ATTR_COLLECT_INGRESS_TIMESTAMP,
            value=sai_thrift_attribute_value_t(booldata=collect_ig_timestamp))
        attr5 = sai_thrift_attribute_t(
            id=SAI_DTEL_INT_SESSION_ATTR_COLLECT_EGRESS_TIMESTAMP,
            value=sai_thrift_attribute_value_t(booldata=collect_eg_timestamp))
        attr6 = sai_thrift_attribute_t(
            id=SAI_DTEL_INT_SESSION_ATTR_COLLECT_QUEUE_INFO,
            value=sai_thrift_attribute_value_t(booldata=collect_queue_info))

        attr_list = [attr1, attr2, attr3, attr4, attr5, attr6]
        self.id = self.client.sai_thrift_create_dtel_int_session(attr_list)
        sai_mgr.int_sessions.append(self)

        print 'SAI CONFIG: create INT sesstion', self.id

    def delete(self):
        self.client.sai_thrift_remove_dtel_int_session(self.id)
        self.sai_mgr.int_sessions.remove(self)

class DTelWatchlist(object):
    def __init__(self, sai_mgr, watchlist_type):
        self.sai_mgr = sai_mgr
        self.client = sai_mgr.client
        self.type = watchlist_type
        self.entries = []
        if self.type == 'Flow':
            action_list = [SAI_ACL_ACTION_TYPE_ACL_DTEL_FLOW_OP,
                           SAI_ACL_ACTION_TYPE_DTEL_INT_SESSION,
                           SAI_ACL_ACTION_TYPE_DTEL_FLOW_SAMPLE_PERCENT,
                           SAI_ACL_ACTION_TYPE_DTEL_REPORT_ALL_PACKETS]
            action_count = 4
        elif self.type == 'Drop':
            action_list = [SAI_ACL_ACTION_TYPE_DTEL_DROP_REPORT_ENABLE]
            action_count = 1
        else:
            print "Invalid watchlist type"
            return
        s32list = sai_thrift_s32_list_t(s32list=action_list, count=action_count)
        attr = sai_thrift_attribute_t(
            id=SAI_ACL_TABLE_ATTR_ACL_ACTION_TYPE_LIST,
            value=sai_thrift_attribute_value_t(s32list=s32list))
        self.id = self.client.sai_thrift_create_acl_table([attr])
        self.sai_mgr.watchlists.append(self)

    def delete_entries(self):
        for entry in list(self.entries):
            entry.delete()

    def delete(self):
        self.delete_entries()
        self.client.sai_thrift_remove_acl_table(self.id)
        self.sai_mgr.watchlists.remove(self)


class DTelWatchlistEntry(object):
    def __init__(self,
                 watchlist,
                 priority=10,
                 ether_type=None,
                 ether_type_mask=None,
                 ip_src=None,
                 ip_src_mask=None,
                 ip_dst=None,
                 ip_dst_mask=None,
                 ip_proto=None,
                 ip_proto_mask=None,
                 dscp=None,
                 dscp_mask=None,
                 l4_src_port=None,
                 l4_src_port_mask=None,
                 l4_dst_port=None,
                 l4_dst_port_mask=None,
                 tunnel_vni=None,
                 tunnel_vni_mask=None,
                 inner_ether_type=None,
                 inner_ether_type_mask=None,
                 inner_src_ip=None,
                 inner_src_ip_mask=None,
                 inner_dst_ip=None,
                 inner_dst_ip_mask=None,
                 inner_ip_proto=None,
                 inner_ip_proto_mask=None,
                 inner_l4_src_port=None,
                 inner_l4_src_port_mask=None,
                 inner_l4_dst_port=None,
                 inner_l4_dst_port_mask=None,
                 range_list=None,
                 dtel_int_enable=None,
                 dtel_int_session=None,
                 dtel_postcard_enable=None,
                 dtel_sample_percent=None,
                 dtel_report_all=None,
                 dtel_drop_report_enable=None):

        self.watchlist = watchlist
        self.sai_mgr = self.watchlist.sai_mgr
        self.client = self.sai_mgr.client
        self.ether_type = ether_type
        self.ether_type_mask = ether_type_mask
        self.ip_src = ip_src
        self.ip_src_mask = ip_src_mask
        self.ip_dst = ip_dst
        self.ip_dst_mask = ip_dst_mask
        self.ip_proto = ip_proto
        self.ip_proto_mask = ip_proto_mask
        self.dscp = dscp
        self.dscp_mask = dscp_mask
        self.l4_src_port = l4_src_port
        self.l4_src_port_mask = l4_src_port_mask
        self.l4_dst_port = l4_dst_port
        self.l4_dst_port_mask = l4_dst_port_mask
        self.tunnel_vni = tunnel_vni
        self.tunnel_vni_mask = tunnel_vni_mask
        self.inner_ether_type = inner_ether_type
        self.inner_ether_type_mask = inner_ether_type_mask
        self.inner_src_ip = inner_src_ip
        self.inner_src_ip_mask = inner_src_ip_mask
        self.inner_dst_ip = inner_dst_ip
        self.inner_dst_ip_mask = inner_dst_ip_mask
        self.inner_ip_proto = inner_ip_proto
        self.inner_ip_proto_mask = inner_ip_proto_mask
        self.inner_l4_src_port = inner_l4_src_port
        self.inner_l4_src_port_mask = inner_l4_src_port_mask
        self.inner_l4_dst_port = inner_l4_dst_port
        self.inner_l4_dst_port_mask = inner_l4_dst_port_mask
        self.range_list = range_list
        self._priority = priority
        self._dtel_int_enable = dtel_int_enable
        self._dtel_int_session = dtel_int_session
        self._dtel_postcard_enable = dtel_postcard_enable
        self._dtel_drop_report_enable = dtel_drop_report_enable
        self._dtel_report_all = dtel_report_all

        acl_attr_list = []

        #ACL table id
        attr = sai_thrift_attribute_t(
            id = SAI_ACL_ENTRY_ATTR_TABLE_ID,
            value = sai_thrift_attribute_value_t(oid=self.watchlist.id))
        acl_attr_list.append(attr)

        #Priority
        attr = sai_thrift_attribute_t(
            id=SAI_ACL_ENTRY_ATTR_PRIORITY,
            value=sai_thrift_attribute_value_t(u32=hex_to_i32(priority)))
        acl_attr_list.append(attr)

        if ether_type != None:
            attr_value = sai_thrift_attribute_value_t(
                aclfield=sai_thrift_acl_field_data_t(
                    data=sai_thrift_acl_data_t(u16=hex_to_i16(ether_type)),
                    mask=sai_thrift_acl_mask_t(u16=hex_to_i16(ether_type_mask))))
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_ENTRY_ATTR_FIELD_ETHER_TYPE, value=attr_value)
            acl_attr_list.append(attr)

        if ip_src != None:
            attr_value = sai_thrift_attribute_value_t(
                aclfield=sai_thrift_acl_field_data_t(
                    data=sai_thrift_acl_data_t(ip4=ip_src),
                    mask=sai_thrift_acl_mask_t(ip4=ip_src_mask)))
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_ENTRY_ATTR_FIELD_SRC_IP, value=attr_value)
            acl_attr_list.append(attr)

        if ip_dst != None:
            attr_value = sai_thrift_attribute_value_t(
                aclfield=sai_thrift_acl_field_data_t(
                    data=sai_thrift_acl_data_t(ip4=ip_dst),
                    mask=sai_thrift_acl_mask_t(ip4=ip_dst_mask)))
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_ENTRY_ATTR_FIELD_DST_IP, value=attr_value)
            acl_attr_list.append(attr)

        if ip_proto != None:
            attr_value = sai_thrift_attribute_value_t(
                aclfield=sai_thrift_acl_field_data_t(
                    data=sai_thrift_acl_data_t(u8=ip_proto),
                    mask=sai_thrift_acl_mask_t(u8=ip_proto_mask)))
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_ENTRY_ATTR_FIELD_IP_PROTOCOL, value=attr_value)
            acl_attr_list.append(attr)

        if dscp != None:
            attr_value = sai_thrift_attribute_value_t(
                aclfield=sai_thrift_acl_field_data_t(
                    data=sai_thrift_acl_data_t(u8=dscp),
                    mask=sai_thrift_acl_mask_t(u8=dscp_mask)))
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_ENTRY_ATTR_FIELD_DSCP, value=attr_value)
            acl_attr_list.append(attr)

        if l4_src_port != None:
            attr_value = sai_thrift_attribute_value_t(
                aclfield=sai_thrift_acl_field_data_t(
                    data=sai_thrift_acl_data_t(u16=hex_to_i16(l4_src_port)),
                    mask=sai_thrift_acl_mask_t(u16=hex_to_i16(l4_src_port_mask))))
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_ENTRY_ATTR_FIELD_L4_SRC_PORT, value=attr_value)
            acl_attr_list.append(attr)

        if l4_dst_port != None:
            attr_value = sai_thrift_attribute_value_t(
                aclfield=sai_thrift_acl_field_data_t(
                    data=sai_thrift_acl_data_t(u16=hex_to_i16(l4_dst_port)),
                    mask=sai_thrift_acl_mask_t(u16=hex_to_i16(l4_dst_port_mask))))
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_ENTRY_ATTR_FIELD_L4_DST_PORT, value=attr_value)
            acl_attr_list.append(attr)

        if tunnel_vni != None:
            attr_value = sai_thrift_attribute_value_t(
                aclfield=sai_thrift_acl_field_data_t(
                    data=sai_thrift_acl_data_t(u32=hex_to_i32(tunnel_vni)),
                    mask=sai_thrift_acl_mask_t(u32=hex_to_i32(tunnel_vni_mask))))
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_ENTRY_ATTR_FIELD_TUNNEL_VNI, value=attr_value)
            acl_attr_list.append(attr)

        if inner_ether_type != None:
            attr_value = sai_thrift_attribute_value_t(
                aclfield=sai_thrift_acl_field_data_t(
                    data=sai_thrift_acl_data_t(u16=hex_to_i16(inner_ether_type)),
                    mask=sai_thrift_acl_mask_t(u16=hex_to_i16(inner_ether_type_mask))))
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_ENTRY_ATTR_FIELD_INNER_ETHER_TYPE,
                value=attr_value)
            acl_attr_list.append(attr)

        if inner_src_ip != None:
            attr_value = sai_thrift_attribute_value_t(
                aclfield=sai_thrift_acl_field_data_t(
                    data=sai_thrift_acl_data_t(ip4=inner_src_ip),
                    mask=sai_thrift_acl_mask_t(ip4=inner_src_ip_mask)))
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_ENTRY_ATTR_FIELD_INNER_SRC_IP, value=attr_value)
            acl_attr_list.append(attr)

        if inner_dst_ip != None:
            attr_value = sai_thrift_attribute_value_t(
                aclfield=sai_thrift_acl_field_data_t(
                    data=sai_thrift_acl_data_t(ip4=inner_dst_ip),
                    mask=sai_thrift_acl_mask_t(ip4=inner_dst_ip_mask)))
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_ENTRY_ATTR_FIELD_INNER_DST_IP, value=attr_value)
            acl_attr_list.append(attr)

        if inner_ip_proto != None:
            attr_value = sai_thrift_attribute_value_t(
                aclfield=sai_thrift_acl_field_data_t(
                    data=sai_thrift_acl_data_t(u8=inner_ip_proto),
                    mask=sai_thrift_acl_mask_t(u8=inner_ip_proto_mask)))
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_ENTRY_ATTR_FIELD_INNER_IP_PROTOCOL,
                value=attr_value)
            acl_attr_list.append(attr)

        if inner_l4_src_port != None:
            attr_value = sai_thrift_attribute_value_t(
                aclfield=sai_thrift_acl_field_data_t(
                    data=sai_thrift_acl_data_t(u16=hex_to_i16(inner_l4_src_port)),
                    mask=sai_thrift_acl_mask_t(u16=hex_to_i16(inner_l4_src_port_mask))))
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_ENTRY_ATTR_FIELD_INNER_L4_SRC_PORT,
                value=attr_value)
            acl_attr_list.append(attr)

        if inner_l4_dst_port != None:
            attr_value = sai_thrift_attribute_value_t(
                aclfield=sai_thrift_acl_field_data_t(
                    data=sai_thrift_acl_data_t(u16=hex_to_i16(inner_l4_dst_port)),
                    mask=sai_thrift_acl_mask_t(u16=hex_to_i16(inner_l4_dst_port_mask))))
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_ENTRY_ATTR_FIELD_INNER_L4_DST_PORT,
                value=attr_value)
            acl_attr_list.append(attr)

        if range_list != None:
            acl_range_list = sai_thrift_object_list_t(
                count=len(range_list), object_id_list=range_list)
            attr_value = sai_thrift_attribute_value_t(
                aclfield=sai_thrift_acl_field_data_t(
                    data = sai_thrift_acl_data_t(objlist=acl_range_list)))
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_ENTRY_ATTR_FIELD_ACL_RANGE_TYPE,
                value=attr_value)
            acl_attr_list.append(attr)

        if dtel_int_enable != None or dtel_postcard_enable != None :
            if dtel_int_enable == True:
                flow_op = SAI_ACL_DTEL_FLOW_OP_INT
            elif dtel_postcard_enable == True:
                flow_op = SAI_ACL_DTEL_FLOW_OP_POSTCARD
            else:
                flow_op = SAI_ACL_DTEL_FLOW_OP_NOP
            action = sai_thrift_acl_action_data_t(
                parameter=sai_thrift_acl_parameter_t(s32=flow_op))
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_ENTRY_ATTR_ACTION_ACL_DTEL_FLOW_OP,
                value=sai_thrift_attribute_value_t(aclaction=action))
            acl_attr_list.append(attr)

        if dtel_int_session != None:
            action = sai_thrift_acl_action_data_t(
                parameter=sai_thrift_acl_parameter_t(oid=dtel_int_session.id))
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_ENTRY_ATTR_ACTION_DTEL_INT_SESSION,
                value=sai_thrift_attribute_value_t(aclaction=action))
            acl_attr_list.append(attr)

        if dtel_sample_percent != None:
            action = sai_thrift_acl_action_data_t(
                parameter=sai_thrift_acl_parameter_t(u8=dtel_sample_percent))
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_ENTRY_ATTR_ACTION_DTEL_FLOW_SAMPLE_PERCENT,
                value=sai_thrift_attribute_value_t(aclaction=action))
            acl_attr_list.append(attr)

        if dtel_report_all != None:
            action = sai_thrift_acl_action_data_t(enable=dtel_report_all)
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_ENTRY_ATTR_ACTION_DTEL_REPORT_ALL_PACKETS,
                value=sai_thrift_attribute_value_t(aclaction=action))
            acl_attr_list.append(attr)

        if dtel_drop_report_enable != None:
            action = sai_thrift_acl_action_data_t(enable=dtel_drop_report_enable)
            attr = sai_thrift_attribute_t(
                id=SAI_ACL_ENTRY_ATTR_ACTION_DTEL_DROP_REPORT_ENABLE,
                value=sai_thrift_attribute_value_t(aclaction=action))
            acl_attr_list.append(attr)

        self.id = self.client.sai_thrift_create_acl_entry(acl_attr_list)
        if self.id != 0:
            print 'SAI CONFIG: create dtel watchlist entry', hex(self.id)
            self.watchlist.entries.append(self)
        else:
            raise ValueError("Entry not created")

    def delete(self):
        print 'SAI CONFIG: remove dtel watchlist entry', hex(self.id)
        self.client.sai_thrift_remove_acl_entry(self.id)
        self.watchlist.entries.remove(self)


    @property
    def dtel_report_all(self):
        return self._dtel_report_all

    @dtel_report_all.setter
    def dtel_report_all(self, value):
        if self._dtel_report_all == value:
            return
        action = sai_thrift_acl_action_data_t(enable=value)
        attr = sai_thrift_attribute_t(
            id=SAI_ACL_ENTRY_ATTR_ACTION_DTEL_REPORT_ALL_PACKETS,
            value=sai_thrift_attribute_value_t(aclaction=action))
        self.client.sai_thrift_set_acl_entry_attribute(self.id, attr)
        self._dtel_report_all = value


class SAISwitch(object):
    def __init__(self, sai_mgr):
        self.sai_mgr = sai_mgr
        self.client = sai_mgr.client
        self.params = sai_mgr.params
        self.all_ports = []
        self.hw_lanes = []
        self._src_mac_address = None
        self._dtel_switch_id = None
        self._dtel_int_endpoint_enable = False
        self._dtel_int_transit_enable = False
        self._dtel_postcard_enable = False
        self._dtel_drop_report_enable = False
        self._dtel_flow_state_clear_cycle = 0
        self._dtel_latency_sensitivity = MAX_QUANTIZATION
        self._dtel_int_sink_port_list = []
        self._dtel_int_l4_dscp = (get_int_l45_dscp_value(),
                                  get_int_l45_dscp_mask())
        self._dtel_oid = self.client.sai_thrift_create_dtel([])

        self.dtel_flow_state_clear_cycle = self._dtel_flow_state_clear_cycle
        self.dtel_latency_sensitivity = self._dtel_latency_sensitivity
        self.dtel_int_l4_dscp = (get_int_l45_dscp_value(),
                                 get_int_l45_dscp_mask())

        switch_attr_list = self.client.sai_thrift_get_switch_attribute()
        attr_list = switch_attr_list.attr_list
        for attribute in attr_list:
            if attribute.id == SAI_SWITCH_ATTR_PORT_LIST:
                for x in attribute.value.objlist.object_id_list:
                    self.all_ports.append(x)
                    port_attr_list = self.client.sai_thrift_get_port_attribute(x)
                    for attr in port_attr_list.attr_list:
                        if attr.id == SAI_PORT_ATTR_HW_LANE_LIST:
                            self.hw_lanes.append(attr.value.u32list.u32list[0])
            if attribute.id == SAI_SWITCH_ATTR_DEFAULT_VIRTUAL_ROUTER_ID:
                self.default_vrf = attribute.value.oid;
        if self.params is not None:
            self.src_mac_address = self.params.mac_self
            self.dtel_switch_id = self.params.switch_id

    @property
    def src_mac_address(self):
        return self._src_mac_address

    @src_mac_address.setter
    def src_mac_address(self, value):
        attr = sai_thrift_attribute_t(
            id=SAI_SWITCH_ATTR_SRC_MAC_ADDRESS,
            value=sai_thrift_attribute_value_t(mac=value))
        self.client.sai_thrift_set_switch_attribute(attr)
        print 'SAI CONFIG: set mac address', value
        self._src_mac_address = value

    @property
    def dtel_int_endpoint_enable(self):
        return self._dtel_int_endpoint_enable

    @dtel_int_endpoint_enable.setter
    def dtel_int_endpoint_enable(self, value):
        if self._dtel_int_endpoint_enable == value:
            return
        attr = sai_thrift_attribute_t(
            id=SAI_DTEL_ATTR_INT_ENDPOINT_ENABLE,
            value=sai_thrift_attribute_value_t(booldata=value))
        self.client.sai_thrift_set_dtel_attribute(self._dtel_oid, attr)
        if value:
            print 'SAI CONFIG: enable INT Endpoint'
        else:
            print 'SAI CONFIG: disable INT Endpoint'
        self._dtel_int_endpoint_enable = value

    @property
    def dtel_int_transit_enable(self):
        return self._dtel_int_transit_enable

    @dtel_int_transit_enable.setter
    def dtel_int_transit_enable(self, value):
        if self._dtel_int_transit_enable == value:
            return
        attr = sai_thrift_attribute_t(
            id=SAI_DTEL_ATTR_INT_TRANSIT_ENABLE,
            value=sai_thrift_attribute_value_t(booldata=value))
        self.client.sai_thrift_set_dtel_attribute(self._dtel_oid, attr)
        if value:
            print 'SAI CONFIG: enable INT Transit'
        else:
            print 'SAI CONFIG: disable INT Transit'
        self._dtel_int_transit_enable = value

    @property
    def dtel_postcard_enable(self):
        return self._dtel_postcard_enable

    @dtel_postcard_enable.setter
    def dtel_postcard_enable(self, value):
        if self._dtel_postcard_enable == value:
            return
        attr = sai_thrift_attribute_t(
            id=SAI_DTEL_ATTR_POSTCARD_ENABLE,
            value=sai_thrift_attribute_value_t(booldata=value))
        self.client.sai_thrift_set_dtel_attribute(self._dtel_oid, attr)
        if value:
            print 'SAI CONFIG: enable Postcard'
        else:
            print 'SAI CONFIG: disable Postcard'
        self._dtel_postcard_enable = value

    @property
    def dtel_drop_report_enable(self):
        return self._dtel_drop_report_enable

    @dtel_drop_report_enable.setter
    def dtel_drop_report_enable(self, value):
        if self._dtel_drop_report_enable == value:
            return
        attr = sai_thrift_attribute_t(
            id=SAI_DTEL_ATTR_DROP_REPORT_ENABLE,
            value=sai_thrift_attribute_value_t(booldata=value))
        self.client.sai_thrift_set_dtel_attribute(self._dtel_oid, attr)
        if value:
            print 'SAI CONFIG: enable Mirror on Drop'
        else:
            print 'SAI CONFIG: disable Mirror on Drop'
        self._dtel_drop_report_enable = value

    @property
    def dtel_switch_id(self):
        if self._dtel_switch_id is not None:
            switch_attr_list = self.client.sai_thrift_get_switch_attribute()
            attr_list = switch_attr_list.attr_list
            for attribute in attr_list:
                if attribute.id == SAI_DTEL_ATTR_SWITCH_ID:
                    self._dtel_switch_id = attribute.value.u32
                    print "DTel Switch ID: ", self._dtel_switch_id

    @dtel_switch_id.setter
    def dtel_switch_id(self, value):
        attr = sai_thrift_attribute_t(
            id=SAI_DTEL_ATTR_SWITCH_ID,
            value=sai_thrift_attribute_value_t(u32=value))
        self.client.sai_thrift_set_dtel_attribute(self._dtel_oid, attr)
        print 'SAI CONFIG: set dtel switch ID ', hex(value)
        self._dtel_switch_id = value

    @property
    def dtel_int_sink_port_list(self):
        return self._dtel_int_sink_port_list

    @dtel_int_sink_port_list.setter
    def dtel_int_sink_port_list(self, value):
        sai_port_list = sai_thrift_object_list_t(
            object_id_list=value, count=len(value))
        attr = sai_thrift_attribute_t(
            id=SAI_DTEL_ATTR_SINK_PORT_LIST,
            value=sai_thrift_attribute_value_t(objlist=sai_port_list))
        self.client.sai_thrift_set_dtel_attribute(self._dtel_oid, attr)
        print 'SAI CONFIG: set INT sink ports', value
        self._dtel_int_sink_port_list = value

    @property
    def dtel_flow_state_clear_cycle(self):
        return self._dtel_flow_state_clear_cycle

    @dtel_flow_state_clear_cycle.setter
    def dtel_flow_state_clear_cycle(self, value):
        attr = sai_thrift_attribute_t(
            id=SAI_DTEL_ATTR_FLOW_STATE_CLEAR_CYCLE,
            value=sai_thrift_attribute_value_t(u16=value))
        self.client.sai_thrift_set_dtel_attribute(self._dtel_oid, attr)
        self._dtel_flow_state_clear_cycle = value
        print 'SAI CONFIG: set dtel flow state clear cycle to', value

    @property
    def dtel_latency_sensitivity(self):
        return self._dtel_latency_sensitivity

    @dtel_latency_sensitivity.setter
    def dtel_latency_sensitivity(self, value):
        attr = sai_thrift_attribute_t(
            id=SAI_DTEL_ATTR_LATENCY_SENSITIVITY,
            value=sai_thrift_attribute_value_t(u8=value))
        self.client.sai_thrift_set_dtel_attribute(self._dtel_oid, attr)
        self._dtel_latency_sensitivity = value
        print 'SAI CONFIG: set dtel latency sentitivity to', value

    @property
    def dtel_int_l4_dscp(self):
        return self._dtel_int_l4_dscp

    @dtel_int_l4_dscp.setter
    def dtel_int_l4_dscp(self, (value, mask)):
        attr_value = sai_thrift_attribute_value_t(
            aclfield=sai_thrift_acl_field_data_t(
                data=sai_thrift_acl_data_t(u8=value),
                mask=sai_thrift_acl_mask_t(u8=mask)))
        attr = sai_thrift_attribute_t(
            id=SAI_DTEL_ATTR_INT_L4_DSCP, value=attr_value)
        self.client.sai_thrift_set_dtel_attribute(self._dtel_oid, attr)
        self._dtel_int_l4_dscp = (value, mask)
        print 'SAI CONFIG: set dtel INT L4 DSCP to value', value, 'mask', mask

class DTelReportSession(object):
    def __init__(self,
                 sai_mgr,
                 src_ip,
                 dst_ip_list,
                 udp_port,
                 truncate_size):

        self.sai_mgr = sai_mgr
        self.client = sai_mgr.client
        self.addr_family = get_SAI_ip_address_family_code(src_ip)
        self.src_ip = src_ip
        self.dst_ip_list = dst_ip_list
        self.truncate_size = truncate_size
        self.udp_port = udp_port

        report_attr_list = []

        #source ip
        src_ip_addr = sai_thrift_ip_address_t(
            addr_family=self.addr_family, addr=sai_thrift_ip_t(ip4=self.src_ip))
        attr = sai_thrift_attribute_t(
            id=SAI_DTEL_REPORT_SESSION_ATTR_SRC_IP,
            value=sai_thrift_attribute_value_t(ipaddr=src_ip_addr))
        report_attr_list.append(attr)

        #dst ip list
        dst_ip_addrs = []
        for ip in dst_ip_list:
            dst_ip_addr = sai_thrift_ip_address_t(
                addr_family=self.addr_family, addr=sai_thrift_ip_t(ip4=ip))
            dst_ip_addrs.append(dst_ip_addr)
        dst_ip_addr_list = sai_thrift_ipaddr_list_t(
            ipaddr_list=dst_ip_addrs, count=len(dst_ip_list))
        attr = sai_thrift_attribute_t(
            id=SAI_DTEL_REPORT_SESSION_ATTR_DST_IP_LIST,
            value=sai_thrift_attribute_value_t(ipaddrlist=dst_ip_addr_list))
        report_attr_list.append(attr)

        #vrf id
        attr = sai_thrift_attribute_t(
            id=SAI_DTEL_REPORT_SESSION_ATTR_VIRTUAL_ROUTER_ID,
            value=sai_thrift_attribute_value_t(oid=sai_mgr.switch.default_vrf))
        report_attr_list.append(attr)

        #truncate size
        attr = sai_thrift_attribute_t(
            id=SAI_DTEL_REPORT_SESSION_ATTR_TRUNCATE_SIZE,
            value=sai_thrift_attribute_value_t(u16=self.truncate_size))
        report_attr_list.append(attr)

        # udp port
        attr = sai_thrift_attribute_t(
            id=SAI_DTEL_REPORT_SESSION_ATTR_UDP_DST_PORT,
            value=sai_thrift_attribute_value_t(u16=self.udp_port))
        report_attr_list.append(attr)

        self.id = self.client.sai_thrift_create_dtel_report_session(report_attr_list)
        self.sai_mgr.report_session = self

        print 'SAI CONFIG: create report session to', dst_ip_list

    def delete(self):
        self.client.sai_thrift_remove_dtel_report_session(self.id)
        self.sai_mgr.report_session = None

class DTelEvent(object):
    def __init__(self, sai_mgr, event_type, dscp):
        self.sai_mgr = sai_mgr
        self.client = sai_mgr.client
        self.type = event_type
        self._dscp_value = dscp

        attr1 = sai_thrift_attribute_t(
            id=SAI_DTEL_EVENT_ATTR_TYPE,
            value=sai_thrift_attribute_value_t(s32=event_type))
        attr2 = sai_thrift_attribute_t(
            id=SAI_DTEL_EVENT_ATTR_REPORT_SESSION,
            value=sai_thrift_attribute_value_t(oid=sai_mgr.report_session.id))
        attr3 = sai_thrift_attribute_t(
            id=SAI_DTEL_EVENT_ATTR_DSCP_VALUE,
            value=sai_thrift_attribute_value_t(u8=dscp))

        attr_list = [attr1, attr2, attr3]
        self.id = self.client.sai_thrift_create_dtel_event(attr_list)
        sai_mgr.events.append(self)

        print 'SAI CONFIG: create DTel event type', event_type, 'dscp', dscp

    def delete(self):
        self.client.sai_thrift_remove_dtel_event(self.id)
        self.sai_mgr.events.remove(self)

    @property
    def dscp_value(self):
        return self._dscp_value

    @dscp_value.setter
    def dscp_value(self, value):
        if self._dscp_value == value:
            return
        attr = sai_thrift_attribute_t(
            id=SAI_DTEL_EVENT_ATTR_DSCP_VALUE,
            value=sai_thrift_attribute_value_t(u8=value))
        self.client.sai_thrift_set_dtel_event_attribute(self.id, attr)

        print 'SAI CONFIG: create DTel event type', self.type, 'dscp', value
        self._dscp_value = value

class SAIRouterInterface():
    def __init__(self, sai_mgr, port):
        self.sai_mgr = sai_mgr
        self.client = sai_mgr.client

        self.type = SAI_ROUTER_INTERFACE_TYPE_PORT
        self.port = port
        self._mtu = None

        rif_attr_list = []
        attr = sai_thrift_attribute_t(
            id=SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID,
            value=sai_thrift_attribute_value_t(oid=self.sai_mgr.switch.default_vrf))
        rif_attr_list.append(attr)

        attr = sai_thrift_attribute_t(
            id=SAI_ROUTER_INTERFACE_ATTR_TYPE,
            value=sai_thrift_attribute_value_t(s32=self.type))
        rif_attr_list.append(attr)

        attr = sai_thrift_attribute_t(
            id=SAI_ROUTER_INTERFACE_ATTR_PORT_ID,
            value=sai_thrift_attribute_value_t(oid=self.port))
        rif_attr_list.append(attr)

        attr = sai_thrift_attribute_t(
            id=SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE,
            value=sai_thrift_attribute_value_t(booldata=True))
        rif_attr_list.append(attr)

        self.id = self.client.sai_thrift_create_router_interface(rif_attr_list)
        self.sai_mgr.router_interfaces.append(self)

    def delete(self):
        self.client.sai_thrift_remove_router_interface(self.id)
        self.sai_mgr.router_interfaces.remove(self)

    @property
    def mtu(self):
        return self._mtu

    @mtu.setter
    def mtu(self, value):
        if (self._mtu == value) or (value is None):
            return
        attr = sai_thrift_attribute_t(
            id=SAI_ROUTER_INTERFACE_ATTR_MTU,
            value=sai_thrift_attribute_value_t(u32=value))
        self.client.sai_thrift_set_router_interface_attribute(self.id, attr)
        self._mtu = value

class SAINextHop():
    def __init__(self, sai_mgr, ip_addr, router_interface):
        self.sai_mgr = sai_mgr
        self.client = sai_mgr.client
        self.ip_addr = ip_addr
        self.addr_family = get_SAI_ip_address_family_code(self.ip_addr)
        self.router_interface = router_interface

        if self.addr_family == SAI_IP_ADDR_FAMILY_IPV4:
            ipaddr = sai_thrift_ip_address_t(
                addr_family=SAI_IP_ADDR_FAMILY_IPV4,
                addr=sai_thrift_ip_t(ip4=self.ip_addr))
        else:
            ipaddr = sai_thrift_ip_address_t(
                addr_family=SAI_IP_ADDR_FAMILY_IPV6,
                addr=sai_thrift_ip_t(ip6=self.ip_addr))

        attr1 = sai_thrift_attribute_t(
            id=SAI_NEXT_HOP_ATTR_IP,
            value=sai_thrift_attribute_value_t(ipaddr=ipaddr))

        attr2 = sai_thrift_attribute_t(
            id=SAI_NEXT_HOP_ATTR_ROUTER_INTERFACE_ID,
            value=sai_thrift_attribute_value_t(oid=self.router_interface.id))

        self.id = self.client.sai_thrift_create_next_hop([attr1, attr2])
        self.sai_mgr.next_hops.append(self)

    def delete(self):
        self.client.sai_thrift_remove_next_hop(self.id)
        self.sai_mgr.next_hops.remove(self)

class SAIRoute():
    def __init__(self, sai_mgr, ip_addr, ip_mask, next_hop):
        self.sai_mgr = sai_mgr
        self.client = sai_mgr.client
        self.next_hop = next_hop

        if get_SAI_ip_address_family_code(ip_addr) == SAI_IP_ADDR_FAMILY_IPV4:
            addr = sai_thrift_ip_t(ip4=ip_addr)
            mask = sai_thrift_ip_t(ip4=ip_mask)
            self.ip_prefix = sai_thrift_ip_prefix_t(
                addr_family=SAI_IP_ADDR_FAMILY_IPV4, addr=addr, mask=mask)
        else:
            addr = sai_thrift_ip_t(ip6=ip_addr)
            mask = sai_thrift_ip_t(ip6=ip_mask)
            self.ip_prefix = sai_thrift_ip_prefix_t(
                addr_family=SAI_IP_ADDR_FAMILY_IPV6, addr=addr, mask=mask)

        attr = sai_thrift_attribute_t(
            id=SAI_ROUTE_ENTRY_ATTR_NEXT_HOP_ID,
            value=sai_thrift_attribute_value_t(oid=self.next_hop.id))
        route = sai_thrift_route_entry_t(self.sai_mgr.switch.default_vrf, self.ip_prefix)

        self.client.sai_thrift_create_route(route, [attr])
        self.sai_mgr.routes.append(self)

    def delete(self):
        route = sai_thrift_route_entry_t(self.sai_mgr.switch.default_vrf, self.ip_prefix)
        self.client.sai_thrift_remove_route(route)
        self.sai_mgr.routes.remove(self)

class SAINeighbor():
    def __init__(self, sai_mgr, router_interface, neighbor_ip, dmac):
        self.sai_mgr = sai_mgr
        self.client = sai_mgr.client
        self.router_interface = router_interface
        self.neighbor_ip = neighbor_ip
        self.addr_family = get_SAI_ip_address_family_code(self.neighbor_ip)
        self.dmac = dmac

        if self.addr_family == SAI_IP_ADDR_FAMILY_IPV4:
            self.ipaddr = sai_thrift_ip_address_t(
                addr_family=SAI_IP_ADDR_FAMILY_IPV4,
                addr=sai_thrift_ip_t(ip4=self.neighbor_ip))
        else:
            self.ipaddr = sai_thrift_ip_address_t(
                addr_family=SAI_IP_ADDR_FAMILY_IPV6,
                addr=sai_thrift_ip_t(ip6=self.neighbor_ip))

        attr = sai_thrift_attribute_t(
            id=SAI_NEIGHBOR_ENTRY_ATTR_DST_MAC_ADDRESS,
            value=sai_thrift_attribute_value_t(mac=dmac))
        neighbor_entry = sai_thrift_neighbor_entry_t(
            rif_id=self.router_interface.id, ip_address=self.ipaddr)
        self.client.sai_thrift_create_neighbor_entry(neighbor_entry, [attr])
        self.sai_mgr.neighbors.append(self)

    def delete(self):
        neighbor_entry = sai_thrift_neighbor_entry_t(
            rif_id=self.router_interface.id, ip_address=self.ipaddr)
        self.client.sai_thrift_remove_neighbor_entry(neighbor_entry)
        self.sai_mgr.neighbors.remove(self)
