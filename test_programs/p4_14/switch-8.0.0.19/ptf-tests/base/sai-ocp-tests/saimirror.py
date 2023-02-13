# Copyright 2013-present Barefoot Networks, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
Thrift SAI interface Mirror tests
"""

import socket

from switch import *
from ptf.mask import Mask
import sai_base_test
from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *
from switchsai_thrift.sai_headers import  *
from switch_utils import *
from erspan3 import *

@group('mirror-ocp')
@group('mirror')
class spanmonitor(sai_base_test.ThriftInterfaceDataPlane):
    '''
    This performs Local mirroring
    We set port2 traffic to be monitored(both ingress and egress) on port1
    We send a packet from port 2 to port 3
    We expect the same packet on port 1 which is a mirror packet
    '''
    def runTest(self):
        print
        switch_init(self.client)
        port1 = port_list[0]
        port2 = port_list[1]
        port3 = port_list[2]
        monitor_port=port1
        source_port=port2
        mac3='00:00:00:00:00:33'
        mac2='00:00:00:00:00:22'
        mirror_type=SAI_MIRROR_SESSION_TYPE_LOCAL
        mac_action = SAI_PACKET_ACTION_FORWARD
        vlan_id = 1
        vlan_remote_id = 2
        vlan_oid = sai_thrift_create_vlan(self.client, vlan_id)
        vlan_remote_oid = sai_thrift_create_vlan(self.client, vlan_remote_id)
        sai_thrift_create_fdb(self.client, vlan_remote_id, mac3, port3, mac_action)
        sai_thrift_create_fdb(self.client, vlan_remote_id, mac2, port2, mac_action)

        # Put ports under test in VLAN 2
        vlan_member1 = sai_thrift_create_vlan_member(self.client, vlan_remote_oid, port1, SAI_VLAN_TAGGING_MODE_TAGGED)
        vlan_member2 = sai_thrift_create_vlan_member(self.client, vlan_remote_oid, port2, SAI_VLAN_TAGGING_MODE_TAGGED)
        vlan_member3 = sai_thrift_create_vlan_member(self.client, vlan_remote_oid, port3, SAI_VLAN_TAGGING_MODE_TAGGED)

        # Remove ports from default VLAN
        sai_thrift_vlan_remove_ports(self.client, vlan_oid, [port1, port2, port3])

        # Set PVID
        attr_value = sai_thrift_attribute_value_t(u16=vlan_remote_id)
        attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_PORT_VLAN_ID, value=attr_value)
        self.client.sai_thrift_set_port_attribute(port1, attr)
        self.client.sai_thrift_set_port_attribute(port2, attr)
        self.client.sai_thrift_set_port_attribute(port3, attr)

        spanid=sai_thrift_create_mirror_session(self.client,mirror_type=mirror_type,port=monitor_port,vlan=vlan_oid,vlan_priority=0,vlan_tpid=0,vlan_header_valid=False,src_mac=None,dst_mac=None,src_ip=None,dst_ip=None,encap_type=0,iphdr_version=0,ttl=0,tos=0,gre_type=0)
        attrb_value = sai_thrift_attribute_value_t(objlist=sai_thrift_object_list_t(count=1,object_id_list=[spanid]))

        attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_INGRESS_MIRROR_SESSION, value=attrb_value)
        self.client.sai_thrift_set_port_attribute(port2, attr)

        pkt = simple_tcp_packet(eth_dst='00:00:00:00:00:33',
                                eth_src='00:22:22:22:22:22',
                                ip_dst='10.0.0.1',
                                dl_vlan_enable=True,
                                vlan_vid=2,
                                ip_id=101,
                                ip_ttl=64)

        exp_pkt = simple_tcp_packet(eth_dst='00:00:00:00:00:33',
                                eth_src='00:22:22:22:22:22',
                                ip_dst='10.0.0.1',
                                dl_vlan_enable=True,
                                vlan_vid=2,
                                ip_id=101,
                                ip_ttl=64)

        try:
            # in tuple: 0 is device number, 2 is port number
            # this tuple uniquely identifies a port
            # for ingress mirroring
            print "Checking INGRESS Local Mirroring"
            print "Sending packet port 2 -> port 3 (00:22:22:22:22:22 -> 00:00:00:00:00:33)"
            send_packet(self, 1, pkt)
            verify_packets(self, exp_pkt, ports=[0,2])
        finally:
            sai_thrift_delete_fdb(self.client, 2, mac3, port3)
            sai_thrift_delete_fdb(self.client, 2, mac2, port2)

            # Remove ports from mirror destination
            attrb_value = sai_thrift_attribute_value_t(objlist=sai_thrift_object_list_t(count=1,object_id_list=[SAI_NULL_OBJECT_ID]))
            attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_INGRESS_MIRROR_SESSION, value=attrb_value)
            self.client.sai_thrift_set_port_attribute(port2, attr)

            # Now you can remove destination
            self.client.sai_thrift_remove_mirror_session(spanid)

            # Remove ports from VLAN 2
            self.client.sai_thrift_remove_vlan_member(vlan_member1)
            self.client.sai_thrift_remove_vlan_member(vlan_member2)
            self.client.sai_thrift_remove_vlan_member(vlan_member3)
            self.client.sai_thrift_remove_vlan(vlan_remote_oid)

            # Add ports back to default VLAN
            vlan_member1a = sai_thrift_create_vlan_member(self.client, vlan_oid, port1, SAI_VLAN_TAGGING_MODE_UNTAGGED)
            vlan_member2a = sai_thrift_create_vlan_member(self.client, vlan_oid, port2, SAI_VLAN_TAGGING_MODE_UNTAGGED)
            vlan_member3a = sai_thrift_create_vlan_member(self.client, vlan_oid, port3, SAI_VLAN_TAGGING_MODE_UNTAGGED)

            attr_value = sai_thrift_attribute_value_t(u16=1)
            attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_PORT_VLAN_ID, value=attr_value)
            self.client.sai_thrift_set_port_attribute(port1, attr)
            self.client.sai_thrift_set_port_attribute(port2, attr)
            self.client.sai_thrift_set_port_attribute(port3, attr)


@group('mirror-ocp')
@group('mirror')
class spanaclmonitor(sai_base_test.ThriftInterfaceDataPlane):
    '''
    This performs Local mirroring
    We set port2 traffic to be monitored(both ingress and egress) on port1
    We send a packet from port 2 to port 3
    We expect the same packet on port 1 which is a mirror packet
    '''
    def runTest(self):
        print
        switch_init(self.client)
        port1 = port_list[0]
        port2 = port_list[1]
        port3 = port_list[2]
        monitor_port=port1
        source_port=port2
        mac3='00:00:00:00:00:33'
        mac2='00:00:00:00:00:22'
        mirror_type=SAI_MIRROR_SESSION_TYPE_LOCAL
        mac_action = SAI_PACKET_ACTION_FORWARD
        vlan_id = 1
        vlan_remote_id = 2
        vlan_oid = sai_thrift_create_vlan(self.client, vlan_id)
        vlan_remote_oid = sai_thrift_create_vlan(self.client, vlan_remote_id)
        sai_thrift_create_fdb(self.client, vlan_remote_id, mac3, port3, mac_action)
        sai_thrift_create_fdb(self.client, vlan_remote_id, mac2, port2, mac_action)

        # Put ports under test in VLAN 2
        vlan_member1 = sai_thrift_create_vlan_member(self.client, vlan_remote_oid, port1, SAI_VLAN_TAGGING_MODE_TAGGED)
        vlan_member2 = sai_thrift_create_vlan_member(self.client, vlan_remote_oid, port2, SAI_VLAN_TAGGING_MODE_TAGGED)
        vlan_member3 = sai_thrift_create_vlan_member(self.client, vlan_remote_oid, port3, SAI_VLAN_TAGGING_MODE_TAGGED)

        # Remove ports from default VLAN
        sai_thrift_vlan_remove_ports(self.client, vlan_oid, [port1, port2, port3])

        # Set PVID
        attr_value = sai_thrift_attribute_value_t(u16=vlan_remote_id)
        attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_PORT_VLAN_ID, value=attr_value)
        self.client.sai_thrift_set_port_attribute(port1, attr)
        self.client.sai_thrift_set_port_attribute(port2, attr)
        self.client.sai_thrift_set_port_attribute(port3, attr)

        spanid=sai_thrift_create_mirror_session(self.client,mirror_type=mirror_type,port=monitor_port,vlan=vlan_oid,vlan_priority=0,vlan_tpid=0,vlan_header_valid=False,src_mac=None,dst_mac=None,src_ip=None,dst_ip=None,encap_type=0,iphdr_version=0,ttl=0,tos=0,gre_type=0)
        print spanid
        attrb_value = sai_thrift_attribute_value_t(objlist=sai_thrift_object_list_t(count=1,object_id_list=[spanid]))

        print "Sending packet ptf_intf 2 -[acl]-> ptf_intf 1 (192.168.0.1 -[acl]-> 10.10.10.1 [id = 105])"
        # setup ACL to block based on Source IP
        addr_family = SAI_IP_ADDR_FAMILY_IPV4
        table_stage = SAI_ACL_STAGE_INGRESS
        table_bind_point_list = [SAI_ACL_BIND_POINT_TYPE_PORT]
        entry_priority = SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY
        action = None
        in_ports = [port1, port2]
        mac_src = None
        mac_dst = None
        mac_src_mask = None
        mac_dst_mask = None
        ip_src = "192.168.0.1"
        ip_src_mask = "255.255.255.0"
        ip_dst = "10.0.0.1"
        ip_dst_mask = "255.255.255.0"
        ip_proto = 6
        in_port = None
        out_port = None
        out_ports = None
        src_l4_port = 4000
        dst_l4_port = 5000
        ingress_mirror_id = spanid
        egress_mirror_id = None
        range_list = None

        acl_table_id = sai_thrift_create_acl_table(self.client,
            table_stage,
            table_bind_point_list,
            addr_family,
            mac_src,
            mac_dst,
            ip_src,
            ip_dst,
            ip_proto,
            in_ports,
            out_ports,
            in_port,
            out_port,
            src_l4_port,
            dst_l4_port,
            range_list)
        acl_entry_id = sai_thrift_create_acl_entry(self.client,
            acl_table_id,
            entry_priority,
            action, addr_family,
            mac_src, mac_src_mask,
            mac_dst, mac_dst_mask,
            ip_src, ip_src_mask,
            ip_dst, ip_dst_mask,
            ip_proto,
            in_ports, out_ports,
            in_port, out_port,
            src_l4_port, dst_l4_port,
            ingress_mirror_id,
            egress_mirror_id,
            range_list)

        # bind this ACL table to port2s object id
        attr_value = sai_thrift_attribute_value_t(oid=acl_table_id)
        attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_INGRESS_ACL, value=attr_value)
        self.client.sai_thrift_set_port_attribute(port2, attr)

        pkt = simple_tcp_packet(eth_dst='00:00:00:00:00:33',
                                eth_src='00:22:22:22:22:22',
                                ip_dst='10.0.0.1',
                                dl_vlan_enable=True,
                                vlan_vid=2,
                                ip_id=101,
                                ip_ttl=64,
                                tcp_sport=4000,
                                tcp_dport=5000)

        exp_pkt = simple_tcp_packet(eth_dst='00:00:00:00:00:33',
                                eth_src='00:22:22:22:22:22',
                                ip_dst='10.0.0.1',
                                dl_vlan_enable=True,
                                vlan_vid=2,
                                ip_id=101,
                                ip_ttl=64,
                                tcp_sport=4000,
                                tcp_dport=5000)

        pkt2 = simple_tcp_packet(eth_dst='00:00:00:00:00:22',
                                eth_src='00:33:33:33:33:33',
                                ip_dst='10.0.0.1',
                                dl_vlan_enable=True,
                                vlan_vid=2,
                                ip_id=101,
                                ip_ttl=64,
                                pktlen=104,
                                tcp_sport=4000,
                                tcp_dport=5000)

        exp_pkt2 = simple_tcp_packet(eth_dst='00:00:00:00:00:22',
                                eth_src='00:33:33:33:33:33',
                                ip_dst='10.0.0.1',
                                dl_vlan_enable=True,
                                vlan_vid=2,#use vlan_vid field if packets are expected to be monitored on client side otherwise not needed
                                ip_id=101,
                                ip_ttl=64,
                                pktlen=104,
                                tcp_sport=4000,
                                tcp_dport=5000)

        m=Mask(exp_pkt2)
        m.set_do_not_care_scapy(ptf.packet.IP,'id')
        m.set_do_not_care_scapy(ptf.packet.IP,'chksum')
        try:
            # in tuple: 0 is device number, 2 is port number
            # this tuple uniquely identifies a port
            # for ingress mirroring
            print "Checking INGRESS Local Mirroring"
            print "Sending packet port 2 -> port 3 (00:22:22:22:22:22 -> 00:00:00:00:00:33)"
            send_packet(self, 1, pkt)
            verify_packets(self, exp_pkt, ports=[0,2])
        finally:
            sai_thrift_delete_fdb(self.client, 2, mac3, port3)
            sai_thrift_delete_fdb(self.client, 2, mac2, port2)

            # unbind this ACL table from port2s object id
            attr_value = sai_thrift_attribute_value_t(oid=SAI_NULL_OBJECT_ID)
            attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_INGRESS_ACL, value=attr_value)
            self.client.sai_thrift_set_port_attribute(port2, attr)
            # cleanup ACL
            self.client.sai_thrift_remove_acl_entry(acl_entry_id)
            self.client.sai_thrift_remove_acl_table(acl_table_id)

            # Now you can remove destination
            self.client.sai_thrift_remove_mirror_session(spanid)

            # Remove ports from VLAN 2
            self.client.sai_thrift_remove_vlan_member(vlan_member1)
            self.client.sai_thrift_remove_vlan_member(vlan_member2)
            self.client.sai_thrift_remove_vlan_member(vlan_member3)
            self.client.sai_thrift_remove_vlan(vlan_remote_oid)

            # Add ports back to default VLAN
            vlan_member1a = sai_thrift_create_vlan_member(self.client, vlan_oid, port1, SAI_VLAN_TAGGING_MODE_UNTAGGED)
            vlan_member2a = sai_thrift_create_vlan_member(self.client, vlan_oid, port2, SAI_VLAN_TAGGING_MODE_UNTAGGED)
            vlan_member3a = sai_thrift_create_vlan_member(self.client, vlan_oid, port3, SAI_VLAN_TAGGING_MODE_UNTAGGED)

            attr_value = sai_thrift_attribute_value_t(u16=1)
            attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_PORT_VLAN_ID, value=attr_value)
            self.client.sai_thrift_set_port_attribute(port1, attr)
            self.client.sai_thrift_set_port_attribute(port2, attr)
            self.client.sai_thrift_set_port_attribute(port3, attr)


@group('mirror')
class erspanmonitor(sai_base_test.ThriftInterfaceDataPlane):
    '''
    This test performs erspan monitoring
    From port2(source port) we send traffic to port 3
    erspan mirror packets are expected on port 1(monitor port)
    '''
    def runTest(self):
        print
        switch_init(self.client)
        port1 = port_list[0]
        port2 = port_list[1]
        port3 = port_list[2]
        mac3='00:00:00:00:00:33'
        mac2='00:00:00:00:00:22'
        monitor_port=port1
        source_port=port2
        mirror_type=SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE
        vlan=0x1
        vlan_tpid=0x8100
        vlan_pri=0x6
        src_mac='00:00:00:00:11:22'
        dst_mac='00:00:00:00:11:33'
        encap_type=SAI_ERSPAN_ENCAPSULATION_TYPE_MIRROR_L3_GRE_TUNNEL
        ip_version=0x4
        tos=0
        ttl=64
        gre_type=0x88be
        src_ip='17.18.19.0'
        dst_ip='33.19.20.0'
        src_ip1='41.42.43.44'
        dst_ip1='45.46.47.48'
        src_mac1='00:00:00:00:22:33'
        dst_mac1='00:00:00:00:22:44'
        addr_family=0
        vlan_remote_id = 3
        vlan_oid = sai_thrift_create_vlan(self.client, vlan)
        vlan_remote_oid = sai_thrift_create_vlan(self.client, vlan_remote_id)
        mac_action = SAI_PACKET_ACTION_FORWARD

        sai_thrift_create_fdb(self.client, vlan_remote_id, mac3, port3, mac_action)
        sai_thrift_create_fdb(self.client, vlan_remote_id, mac2, port2, mac_action)

        # Put ports under test in VLAN 3
        vlan_member1 = sai_thrift_create_vlan_member(self.client, vlan_remote_oid, port3, SAI_VLAN_TAGGING_MODE_UNTAGGED)
        vlan_member2 = sai_thrift_create_vlan_member(self.client, vlan_remote_oid, port2, SAI_VLAN_TAGGING_MODE_UNTAGGED)

        # Remove ports from default VLAN
        sai_thrift_vlan_remove_ports(self.client, vlan_oid, [port2, port3])

        # Set PVID
        attr_value = sai_thrift_attribute_value_t(u16=vlan_remote_id)
        attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_PORT_VLAN_ID, value=attr_value)
        self.client.sai_thrift_set_port_attribute(port1, attr)
        self.client.sai_thrift_set_port_attribute(port2, attr)
        self.client.sai_thrift_set_port_attribute(port3, attr)

        erspanid=sai_thrift_create_mirror_session(
                          self.client,
                          mirror_type=mirror_type,
                          port=monitor_port,
                          vlan=vlan,
                          vlan_priority=vlan_pri,
                          vlan_tpid=vlan_tpid,
                          vlan_header_valid=False,
                          src_mac=src_mac,
                          dst_mac=dst_mac,
                          src_ip=src_ip,
                          dst_ip=dst_ip,
                          encap_type=encap_type,
                          iphdr_version=ip_version,
                          ttl=ttl,
                          tos=tos,
                          gre_type=gre_type)

        attrb_value = sai_thrift_attribute_value_t(objlist=sai_thrift_object_list_t(count=1,object_id_list=[erspanid]))

        attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_INGRESS_MIRROR_SESSION, value=attrb_value)
        self.client.sai_thrift_set_port_attribute(port2, attr)

        pkt = simple_tcp_packet(eth_dst='00:00:00:00:00:33',
                                eth_src='00:00:00:00:00:22',
                                ip_dst='10.0.0.1',
                                ip_id=101,
                                ip_ttl=64)

        eth_pkt = simple_eth_packet(pktlen=100, eth_dst='00:00:00:00:00:33', eth_src='00:00:00:00:00:22')

        exp_mirrored_pkt = ipv4_erspan_pkt(
            eth_src='00:00:00:00:11:22',
            eth_dst='00:00:00:00:11:33',
            ip_src='17.18.19.0',
            ip_dst='33.19.20.0',
            ip_id=0,
            ip_ttl=64,
            ip_flags=0x2,
            version=2,
            mirror_id=1,
            inner_frame=pkt)

        exp_eth_mirrored_pkt = ipv4_erspan_pkt(
            eth_src='00:00:00:00:11:22',
            eth_dst='00:00:00:00:11:33',
            ip_src='17.18.19.0',
            ip_dst='33.19.20.0',
            ip_id=0,
            ip_ttl=64,
            ip_flags=0x2,
            version=2,
            mirror_id=1,
            inner_frame=eth_pkt)

        try:
            # in tuple: 0 is device number, 2 is port number
            # this tuple uniquely identifies a port
            # for ingress mirroring
            print "Checking INGRESS ERSPAN Mirroring"
            print "Sending packet port 2 -> port 3 (00:22:22:22:22:22 -> 00:00:00:00:00:33)"
            send_packet(self, 1, pkt)
            verify_packet(self, pkt, 2)
            verify_erspan3_packet(self, exp_mirrored_pkt, 0)
            time.sleep(1)
            send_packet(self, 1, eth_pkt)
            verify_packet(self, eth_pkt, 2)
            verify_erspan3_packet(self, exp_eth_mirrored_pkt, 0)
            # for egress mirroring

            addr = sai_thrift_ip_t(ip4=src_ip1)
            src_ip_addr = sai_thrift_ip_address_t(addr_family=SAI_IP_ADDR_FAMILY_IPV4, addr=addr)
            attrb_value = sai_thrift_attribute_value_t(ipaddr=src_ip_addr)
            attr = sai_thrift_attribute_t(id=SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS, value=attrb_value)
            self.client.sai_thrift_set_mirror_attribute(erspanid, attr)
            addr = sai_thrift_ip_t(ip4=dst_ip1)
            dst_ip_addr = sai_thrift_ip_address_t(addr_family=SAI_IP_ADDR_FAMILY_IPV4, addr=addr)
            attrb_value = sai_thrift_attribute_value_t(ipaddr=dst_ip_addr)
            attr = sai_thrift_attribute_t(id=SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS, value=attrb_value)
            self.client.sai_thrift_set_mirror_attribute(erspanid, attr)
            attrb_value = sai_thrift_attribute_value_t(mac=src_mac1)
            attr = sai_thrift_attribute_t(id=SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS, value=attrb_value)
            self.client.sai_thrift_set_mirror_attribute(erspanid, attr)
            attrb_value = sai_thrift_attribute_value_t(mac=dst_mac1)
            attr = sai_thrift_attribute_t(id=SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS, value=attrb_value)
            self.client.sai_thrift_set_mirror_attribute(erspanid, attr)

            exp_mirrored_pkt = ipv4_erspan_pkt(
                eth_src='00:00:00:00:22:33',
                eth_dst='00:00:00:00:22:44',
                ip_src='41.42.43.44',
                ip_dst='45.46.47.48',
                ip_id=0,
                ip_ttl=64,
                ip_flags=0x2,
                version=2,
                mirror_id=1,
                inner_frame=pkt)
            time.sleep(1)
            send_packet(self, 1, pkt)
            verify_packet(self, pkt, 2)
            verify_erspan3_packet(self, exp_mirrored_pkt, 0)

        finally:
            sai_thrift_delete_fdb(self.client, 3, mac2, port2)
            sai_thrift_delete_fdb(self.client, 3, mac3, port3)

            # Remove ports from mirror destination
            attrb_value = sai_thrift_attribute_value_t(objlist=sai_thrift_object_list_t(count=1,object_id_list=[SAI_NULL_OBJECT_ID]))
            attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_INGRESS_MIRROR_SESSION, value=attrb_value)
            self.client.sai_thrift_set_port_attribute(port2, attr)

            # Now you can remove destination
            self.client.sai_thrift_remove_mirror_session(erspanid)

            # Remove ports from VLAN 3
            self.client.sai_thrift_remove_vlan_member(vlan_member1)
            self.client.sai_thrift_remove_vlan_member(vlan_member2)
            self.client.sai_thrift_remove_vlan(vlan_remote_oid)

            # Add ports back to default VLAN
            vlan_member2a = sai_thrift_create_vlan_member(self.client, vlan_oid, port2, SAI_VLAN_TAGGING_MODE_UNTAGGED)
            vlan_member3a = sai_thrift_create_vlan_member(self.client, vlan_oid, port3, SAI_VLAN_TAGGING_MODE_UNTAGGED)

            attr_value = sai_thrift_attribute_value_t(u16=1)
            attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_PORT_VLAN_ID, value=attr_value)
            self.client.sai_thrift_set_port_attribute(port1, attr)
            self.client.sai_thrift_set_port_attribute(port2, attr)
            self.client.sai_thrift_set_port_attribute(port3, attr)

@group('mirror')
class erspanvlanmonitor(sai_base_test.ThriftInterfaceDataPlane):
    '''
    This test performs erspan monitoring
    From port2(source port) we send traffic to port 3
    erspan mirror packets are expected on port 1(monitor port)
    '''
    def runTest(self):
        print
        switch_init(self.client)
        port1 = port_list[0]
        port2 = port_list[1]
        port3 = port_list[2]
        mac3='00:00:00:00:00:33'
        mac2='00:00:00:00:00:22'
        monitor_port=port1
        source_port=port2
        mirror_type=SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE
        vlan=0x1
        vlan_tpid=0x08100
        vlan_pri=0x6
        src_mac='00:00:00:00:11:22'
        dst_mac='00:00:00:00:11:33'
        encap_type=SAI_ERSPAN_ENCAPSULATION_TYPE_MIRROR_L3_GRE_TUNNEL
        ip_version=0x4
        tos=0x3c
        ttl=64
        gre_type=0x88be
        src_ip='17.18.19.0'
        dst_ip='33.19.20.0'
        src_ip1='41.42.43.44'
        dst_ip1='45.46.47.48'
        src_mac1='00:00:00:00:22:33'
        dst_mac1='00:00:00:00:22:44'
        addr_family=0
        vlan_remote_id = 3
        vlan_oid = sai_thrift_create_vlan(self.client, vlan)
        vlan_remote_oid = sai_thrift_create_vlan(self.client, vlan_remote_id)
        mac_action = SAI_PACKET_ACTION_FORWARD

        sai_thrift_create_fdb(self.client, vlan_remote_id, mac3, port3, mac_action)
        sai_thrift_create_fdb(self.client, vlan_remote_id, mac2, port2, mac_action)

        # Put ports under test in VLAN 3
        vlan_member1 = sai_thrift_create_vlan_member(self.client, vlan_remote_oid, port3, SAI_VLAN_TAGGING_MODE_TAGGED)
        vlan_member2 = sai_thrift_create_vlan_member(self.client, vlan_remote_oid, port2, SAI_VLAN_TAGGING_MODE_TAGGED)

        # Remove ports from default VLAN
        sai_thrift_vlan_remove_ports(self.client, vlan_oid, [port2, port3])

        # Set PVID
        attr_value = sai_thrift_attribute_value_t(u16=vlan_remote_id)
        attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_PORT_VLAN_ID, value=attr_value)
        self.client.sai_thrift_set_port_attribute(port1, attr)
        self.client.sai_thrift_set_port_attribute(port2, attr)
        self.client.sai_thrift_set_port_attribute(port3, attr)

        erspanid=sai_thrift_create_mirror_session(
                          self.client,
                          mirror_type=mirror_type,
                          port=monitor_port,
                          vlan=10,
                          vlan_priority=vlan_pri,
                          vlan_tpid=vlan_tpid,
                          vlan_header_valid=True,
                          src_mac=src_mac,
                          dst_mac=dst_mac,
                          src_ip=src_ip,
                          dst_ip=dst_ip,
                          encap_type=encap_type,
                          iphdr_version=ip_version,
                          ttl=ttl,
                          tos=tos,
                          gre_type=gre_type)

        attrb_value = sai_thrift_attribute_value_t(objlist=sai_thrift_object_list_t(count=1,object_id_list=[erspanid]))

        attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_INGRESS_MIRROR_SESSION, value=attrb_value)
        self.client.sai_thrift_set_port_attribute(port2, attr)

        pkt = simple_tcp_packet(eth_dst='00:00:00:00:00:33',
                                eth_src='00:00:00:00:00:22',
                                ip_dst='10.0.0.1',
                                ip_id=101,
                                dl_vlan_enable=True,
                                vlan_vid=3,
                                ip_ttl=64)

        exp_mirrored_pkt = ipv4_erspan_pkt(
            eth_src='00:00:00:00:11:22',
            eth_dst='00:00:00:00:11:33',
            ip_src='17.18.19.0',
            ip_dst='33.19.20.0',
            ip_id=0,
            ip_ttl=64,
            ip_flags=0x2,
            version=2,
            dl_vlan_enable=True,
            vlan_vid=10,
            mirror_id=1,
            ip_tos=0x3c,
            inner_frame=pkt)

        try:
            # in tuple: 0 is device number, 2 is port number
            # this tuple uniquely identifies a port
            # for ingress mirroring
            print "Checking INGRESS ERSPAN Mirroring"
            print "Sending packet port 2 -> port 3 (00:22:22:22:22:22 -> 00:00:00:00:00:33)"
            send_packet(self, 1, pkt)
            verify_packet(self, pkt, 2)
            verify_erspan3_packet(self, exp_mirrored_pkt, 0)
            # for egress mirroring

            addr = sai_thrift_ip_t(ip4=src_ip1)
            src_ip_addr = sai_thrift_ip_address_t(addr_family=SAI_IP_ADDR_FAMILY_IPV4, addr=addr)
            attrb_value = sai_thrift_attribute_value_t(ipaddr=src_ip_addr)
            attr = sai_thrift_attribute_t(id=SAI_MIRROR_SESSION_ATTR_SRC_IP_ADDRESS, value=attrb_value)
            self.client.sai_thrift_set_mirror_attribute(erspanid, attr)
            addr = sai_thrift_ip_t(ip4=dst_ip1)
            dst_ip_addr = sai_thrift_ip_address_t(addr_family=SAI_IP_ADDR_FAMILY_IPV4, addr=addr)
            attrb_value = sai_thrift_attribute_value_t(ipaddr=dst_ip_addr)
            attr = sai_thrift_attribute_t(id=SAI_MIRROR_SESSION_ATTR_DST_IP_ADDRESS, value=attrb_value)
            self.client.sai_thrift_set_mirror_attribute(erspanid, attr)
            attrb_value = sai_thrift_attribute_value_t(mac=src_mac1)
            attr = sai_thrift_attribute_t(id=SAI_MIRROR_SESSION_ATTR_SRC_MAC_ADDRESS, value=attrb_value)
            self.client.sai_thrift_set_mirror_attribute(erspanid, attr)
            attrb_value = sai_thrift_attribute_value_t(mac=dst_mac1)
            attr = sai_thrift_attribute_t(id=SAI_MIRROR_SESSION_ATTR_DST_MAC_ADDRESS, value=attrb_value)
            self.client.sai_thrift_set_mirror_attribute(erspanid, attr)

            exp_mirrored_pkt = ipv4_erspan_pkt(
                eth_src='00:00:00:00:22:33',
                eth_dst='00:00:00:00:22:44',
                ip_src='41.42.43.44',
                ip_dst='45.46.47.48',
                ip_id=0,
                ip_ttl=64,
                ip_flags=0x2,
                dl_vlan_enable=True,
                vlan_vid=10,
                version=2,
                mirror_id=1,
                ip_tos=0x3c,
                inner_frame=pkt)
            send_packet(self, 1, pkt)
            verify_packet(self, pkt, 2)
            verify_erspan3_packet(self, exp_mirrored_pkt, 0)

        finally:
            sai_thrift_delete_fdb(self.client, 3, mac2, port2)
            sai_thrift_delete_fdb(self.client, 3, mac3, port3)

            # Remove ports from mirror destination
            attrb_value = sai_thrift_attribute_value_t(objlist=sai_thrift_object_list_t(count=1,object_id_list=[SAI_NULL_OBJECT_ID]))
            attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_INGRESS_MIRROR_SESSION, value=attrb_value)
            self.client.sai_thrift_set_port_attribute(port2, attr)

            # Now you can remove destination
            self.client.sai_thrift_remove_mirror_session(erspanid)

            # Remove ports from VLAN 3
            self.client.sai_thrift_remove_vlan_member(vlan_member1)
            self.client.sai_thrift_remove_vlan_member(vlan_member2)
            self.client.sai_thrift_remove_vlan(vlan_remote_oid)

            # Add ports back to default VLAN
            vlan_member2a = sai_thrift_create_vlan_member(self.client, vlan_oid, port2, SAI_VLAN_TAGGING_MODE_UNTAGGED)
            vlan_member3a = sai_thrift_create_vlan_member(self.client, vlan_oid, port3, SAI_VLAN_TAGGING_MODE_UNTAGGED)

            attr_value = sai_thrift_attribute_value_t(u16=1)
            attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_PORT_VLAN_ID, value=attr_value)
            self.client.sai_thrift_set_port_attribute(port1, attr)
            self.client.sai_thrift_set_port_attribute(port2, attr)
            self.client.sai_thrift_set_port_attribute(port3, attr)

@group('mirror')
class IngressLocalMirrorTest(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print '----------------------------------------------------------------------------------------------'
        print "Sending packet ptf_intf 1 -> ptf_intf 2, ptf_intf 3 (local mirror)"
        print "Sending packet ptf_intf 2 -> ptf_intf 1, ptf_intf 3 (local mirror)"

        switch_init(self.client)
        vlan_id = 10
        port1 = port_list[1]
        port2 = port_list[2]
        port3 = port_list[3]
        mac1 = '00:11:11:11:11:11'
        mac2 = '00:22:22:22:22:22'
        mac_action = SAI_PACKET_ACTION_FORWARD
        addr_family=0

        vlan_oid = sai_thrift_create_vlan(self.client, vlan_id)
        vlan_member1 = sai_thrift_create_vlan_member(self.client, vlan_oid, port1, SAI_VLAN_TAGGING_MODE_UNTAGGED)
        vlan_member2 = sai_thrift_create_vlan_member(self.client, vlan_oid, port2, SAI_VLAN_TAGGING_MODE_TAGGED)

        attr_value = sai_thrift_attribute_value_t(u16=vlan_id)
        attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_PORT_VLAN_ID, value=attr_value)
        self.client.sai_thrift_set_port_attribute(port1, attr)

        sai_thrift_create_fdb(self.client, vlan_id, mac1, port1, mac_action)
        sai_thrift_create_fdb(self.client, vlan_id, mac2, port2, mac_action)

        # setup local mirror session
        mirror_type = SAI_MIRROR_SESSION_TYPE_LOCAL
        monitor_port = port3
        print "Create mirror session: mirror_type = SAI_MIRROR_TYPE_LOCAL, monitor_port = ptf_intf 3 "
        ingress_mirror_id = sai_thrift_create_mirror_session(self.client,
            mirror_type,
            monitor_port,
            None, None, None,
            None, None, None,
            None, None, None,
            None, None, None,
            None)
        print "ingress_mirror_id = %d" %ingress_mirror_id

        # setup ACL to block based on Source IP
        table_stage = SAI_ACL_STAGE_INGRESS
        table_bind_point_list = [SAI_ACL_BIND_POINT_TYPE_PORT]
        entry_priority = SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY
        action = SAI_PACKET_ACTION_DROP
        in_ports = [port1, port2]
        mac_src = None
        mac_dst = None
        mac_src_mask = None
        mac_dst_mask = None
        ip_src = "192.168.0.1"
        ip_src_mask = "255.255.255.0"
        ip_dst = None
        ip_dst_mask = None
        ip_proto = None
        in_port = None
        out_port = None
        out_ports = None
        src_l4_port = None
        dst_l4_port = None
        ingress_mirror_id = ingress_mirror_id
        egress_mirror_id = None
        range_list = None

        acl_table_id = sai_thrift_create_acl_table(self.client,
            table_stage,
            table_bind_point_list,
            addr_family,
            mac_src,
            mac_dst,
            ip_src,
            ip_dst,
            ip_proto,
            in_ports,
            out_ports,
            in_port,
            out_port,
            src_l4_port,
            dst_l4_port,
            range_list)
        acl_entry_id = sai_thrift_create_acl_entry(self.client,
            acl_table_id,
            entry_priority,
            action, addr_family,
            mac_src, mac_src_mask,
            mac_dst, mac_dst_mask,
            ip_src, ip_src_mask,
            ip_dst, ip_dst_mask,
            ip_proto,
            in_ports, out_ports,
            in_port, out_port,
            src_l4_port, dst_l4_port,
            ingress_mirror_id,
            egress_mirror_id,
            range_list)

        # bind this ACL table to port2s object id
        attr_value = sai_thrift_attribute_value_t(oid=acl_table_id)
        attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_INGRESS_ACL, value=attr_value)
        self.client.sai_thrift_set_port_attribute(port1, attr)
        self.client.sai_thrift_set_port_attribute(port2, attr)
        try:
            assert ingress_mirror_id > 0, 'ingress_mirror_id is <= 0'

            pkt = simple_tcp_packet(eth_dst=mac2,
                eth_src=mac1,
                ip_dst='10.0.0.1',
                ip_src='192.168.0.1',
                ip_id=102,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(eth_dst=mac2,
                eth_src=mac1,
                ip_dst='10.0.0.1',
                ip_src='192.168.0.1',
                dl_vlan_enable=True,
                vlan_vid=10,
                ip_id=102,
                ip_ttl=64,
                pktlen=104)

            print '#### Sending 00:22:22:22:22:22 | 00:11:11:11:11:11 | 10.0.0.1 | 192.168.0.1 | @ ptf_intf 1 ####'
            send_packet(self, 1, str(pkt))
            verify_each_packet_on_each_port(self, [exp_pkt, pkt], [2, 3])

            time.sleep(1)

            pkt = simple_tcp_packet(eth_dst=mac1,
                eth_src=mac2,
                ip_dst='10.0.0.1',
                ip_src='192.168.0.1',
                vlan_vid=10,
                dl_vlan_enable=True,
                ip_id=102,
                ip_ttl=64,
                pktlen=104)
            exp_pkt = simple_tcp_packet(eth_dst=mac1,
                eth_src=mac2,
                ip_dst='10.0.0.1',
                ip_src='192.168.0.1',
                ip_id=102,
                ip_ttl=64,
                pktlen=100)

            print '#### Sending 00:11:11:11:11:11 | 00:22:22:22:22:22 | 10.0.0.1 | 192.168.0.1 | @ ptf_intf 2 ####'
            send_packet(self, 2, str(pkt))
            verify_each_packet_on_each_port(self, [exp_pkt, pkt], [1, 3])

        finally:
            # unbind this ACL table from port2s object id
            attr_value = sai_thrift_attribute_value_t(oid=SAI_NULL_OBJECT_ID)
            attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_INGRESS_ACL, value=attr_value)
            self.client.sai_thrift_set_port_attribute(port1, attr)
            self.client.sai_thrift_set_port_attribute(port2, attr)
            # cleanup ACL
            self.client.sai_thrift_remove_acl_entry(acl_entry_id)
            self.client.sai_thrift_remove_acl_table(acl_table_id)
            # cleanup
            self.client.sai_thrift_remove_mirror_session(ingress_mirror_id)

            sai_thrift_delete_fdb(self.client, vlan_id, mac1, port1)
            sai_thrift_delete_fdb(self.client, vlan_id, mac2, port2)

            attr_value = sai_thrift_attribute_value_t(u16=1)
            attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_PORT_VLAN_ID, value=attr_value)
            self.client.sai_thrift_set_port_attribute(port1, attr)

            self.client.sai_thrift_remove_vlan_member(vlan_member1)
            self.client.sai_thrift_remove_vlan_member(vlan_member2)
            self.client.sai_thrift_remove_vlan(vlan_oid)

@group('mirror')
class IngressRSpanMirrorTest(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print '----------------------------------------------------------------------------------------------'
        print "Sending packet ptf_intf 1 -> ptf_intf 2, ptf_intf 3 (rspan mirror)"

        switch_init(self.client)
        vlan_id = 10
        vlan_remote_id = 110
        port1 = port_list[1]
        port2 = port_list[2]
        port3 = port_list[3]
        mac1 = '00:11:11:11:11:11'
        mac2 = '00:22:22:22:22:22'
        mac_action = SAI_PACKET_ACTION_FORWARD

        vlan_oid = sai_thrift_create_vlan(self.client, vlan_id)
        vlan_remote_oid = sai_thrift_create_vlan(self.client, vlan_remote_id)
        vlan_member1 = sai_thrift_create_vlan_member(self.client, vlan_oid, port1, SAI_VLAN_TAGGING_MODE_UNTAGGED)
        vlan_member2 = sai_thrift_create_vlan_member(self.client, vlan_oid, port2, SAI_VLAN_TAGGING_MODE_TAGGED)

        attr_value = sai_thrift_attribute_value_t(u16=vlan_id)
        attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_PORT_VLAN_ID, value=attr_value)
        self.client.sai_thrift_set_port_attribute(port1, attr)

        sai_thrift_create_fdb(self.client, vlan_id, mac1, port1, mac_action)
        sai_thrift_create_fdb(self.client, vlan_id, mac2, port2, mac_action)

        # setup remote mirror session
        mirror_type = SAI_MIRROR_SESSION_TYPE_REMOTE
        monitor_port = port3
        vlan = vlan_remote_id
        print "Create mirror session: mirror_type = SAI_MIRROR_SESSION_TYPE_REMOTE, monitor_port = ptf_intf 3 "
        ingress_mirror_id = sai_thrift_create_mirror_session(self.client,
            mirror_type,
            monitor_port,
            vlan, None, None,
            None, None, None,
            None, None, None,
            None, None, None,
            None)
        print "ingress_mirror_id ' %x" %ingress_mirror_id

        attr_value = sai_thrift_attribute_value_t(objlist=sai_thrift_object_list_t(count=1,object_id_list=[ingress_mirror_id]))
        attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_INGRESS_MIRROR_SESSION, value=attr_value)
        self.client.sai_thrift_set_port_attribute(port1, attr)

        try:
            assert ingress_mirror_id > 0, 'ingress_mirror_id is <= 0'

            pkt = simple_tcp_packet(eth_dst=mac2,
                eth_src=mac1,
                ip_dst='10.0.0.1',
                ip_src='192.168.0.1',
                ip_id=102,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(eth_dst=mac2,
                eth_src=mac1,
                ip_dst='10.0.0.1',
                ip_src='192.168.0.1',
                dl_vlan_enable=True,
                vlan_vid=10,
                ip_id=102,
                ip_ttl=64,
                pktlen=104)
            exp_mirrored_pkt = simple_tcp_packet(eth_dst=mac2,
                eth_src=mac1,
                ip_dst='10.0.0.1',
                ip_src='192.168.0.1',
                dl_vlan_enable=True,
                vlan_vid=110,
                ip_id=102,
                ip_ttl=64,
                pktlen=104)
            print '#### Sending 00:22:22:22:22:22 | 00:11:11:11:11:11 | 10.0.0.1 | 192.168.0.1 | @ ptf_intf 1 ####'
            send_packet(self, 1, str(pkt))
            verify_each_packet_on_each_port(self, [exp_pkt, exp_mirrored_pkt], [2, 3])

        finally:
            attr_value = sai_thrift_attribute_value_t(objlist=sai_thrift_object_list_t(count=0,object_id_list=[]))
            attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_INGRESS_MIRROR_SESSION, value=attr_value)
            self.client.sai_thrift_set_port_attribute(port1, attr)

            self.client.sai_thrift_remove_mirror_session(ingress_mirror_id)

            sai_thrift_delete_fdb(self.client, vlan_id, mac1, port1)
            sai_thrift_delete_fdb(self.client, vlan_id, mac2, port2)

            attr_value = sai_thrift_attribute_value_t(u16=1)
            attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_PORT_VLAN_ID, value=attr_value)
            self.client.sai_thrift_set_port_attribute(port1, attr)

            self.client.sai_thrift_remove_vlan_member(vlan_member1)
            self.client.sai_thrift_remove_vlan_member(vlan_member2)
            self.client.sai_thrift_remove_vlan(vlan_oid)
            self.client.sai_thrift_remove_vlan(vlan_remote_oid)

@group('mirror')
class IngressERSpanMirrorTest(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print '----------------------------------------------------------------------------------------------'
        print "Sending packet ptf_intf 1 -> ptf_intf 2, ptf_intf 3 (erspan mirror)"

        switch_init(self.client)
        vlan_id = 10
        port1 = port_list[1]
        port2 = port_list[2]
        port3 = port_list[3]
        mac1 = '00:11:11:11:11:11'
        mac2 = '00:22:22:22:22:22'
        mac_action = SAI_PACKET_ACTION_FORWARD

        vlan_oid = sai_thrift_create_vlan(self.client, vlan_id)
        vlan_member1 = sai_thrift_create_vlan_member(self.client, vlan_oid, port1, SAI_VLAN_TAGGING_MODE_UNTAGGED)
        vlan_member2 = sai_thrift_create_vlan_member(self.client, vlan_oid, port2, SAI_VLAN_TAGGING_MODE_TAGGED)

        attr_value = sai_thrift_attribute_value_t(u16=vlan_id)
        attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_PORT_VLAN_ID, value=attr_value)
        self.client.sai_thrift_set_port_attribute(port1, attr)

        sai_thrift_create_fdb(self.client, vlan_id, mac1, port1, mac_action)
        sai_thrift_create_fdb(self.client, vlan_id, mac2, port2, mac_action)

        # setup enhanced remote mirror session
        mirror_type = SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE
        monitor_port = port3
        vlan = vlan_id
        vlan_header_valid = True
        iphdr_version = 4
        tunnel_src_ip = "1.1.1.1"
        tunnel_dst_ip = "1.1.1.2"
        tunnel_src_mac = router_mac
        tunnel_dst_mac = "00:33:33:33:33:33"
        gre_protocol = 47
        ttl = 64

        print "Create mirror session: mirror_type = SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE, monitor_port = ptf_intf 3 "
        ingress_mirror_id = sai_thrift_create_mirror_session(self.client,
            mirror_type,
            monitor_port,
            vlan, None, None, vlan_header_valid,
            tunnel_src_mac, tunnel_dst_mac,
            tunnel_src_ip, tunnel_dst_ip,
            None, iphdr_version, ttl, None, gre_protocol)
        print "ingress_mirror_id = %d" %ingress_mirror_id

        attr_value = sai_thrift_attribute_value_t(objlist=sai_thrift_object_list_t(count=1,object_id_list=[ingress_mirror_id]))
        attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_INGRESS_MIRROR_SESSION, value=attr_value)
        self.client.sai_thrift_set_port_attribute(port1, attr)

        try:
            assert ingress_mirror_id > 0, 'ingress_mirror_id is <= 0'

            pkt = simple_tcp_packet(eth_dst=mac2,
                eth_src=mac1,
                ip_dst='10.0.0.1',
                ip_src='192.168.0.1',
                ip_id=102,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(eth_dst=mac2,
                eth_src=mac1,
                ip_dst='10.0.0.1',
                ip_src='192.168.0.1',
                dl_vlan_enable=True,
                vlan_vid=10,
                ip_id=102,
                ip_ttl=64,
                pktlen=104)
            exp_mirrored_pkt = ipv4_erspan_platform_pkt(eth_dst=tunnel_dst_mac,
                eth_src=tunnel_src_mac,
                ip_src=tunnel_src_ip,
                ip_dst=tunnel_dst_ip,
                dl_vlan_enable=True,
                vlan_vid=10,
                ip_id=0,
                ip_ttl=64,
                version=2,
                mirror_id=(ingress_mirror_id & 0x3FFFFFFF),
                inner_frame=pkt)

            exp_mask_mirrored_pkt = Mask(exp_mirrored_pkt)
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.IP, 'len')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.IP, 'flags')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.IP, 'chksum')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.GRE, 'proto')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.PlatformSpecific, 'platf_id')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.PlatformSpecific, 'info1')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.PlatformSpecific, 'info2')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.ERSPAN_III, 'span_id')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.ERSPAN_III, 'timestamp')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.ERSPAN_III, 'sgt_other')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.ERSPAN_III, 'direction')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.ERSPAN_III, 'version')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.ERSPAN_III, 'vlan')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.ERSPAN_III, 'priority')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.ERSPAN_III, 'truncated')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.ERSPAN_III, 'unknown2')

            print '#### Sending 00:22:22:22:22:22 | 00:11:11:11:11:11 | 10.0.0.1 | 192.168.0.1 | @ ptf_intf 1 ####'
            send_packet(self, 1, str(pkt))
            verify_packet(self, exp_pkt, 2)
            verify_packet(self, exp_mask_mirrored_pkt, 3)

        finally:
            attr_value = sai_thrift_attribute_value_t(objlist=sai_thrift_object_list_t(count=0,object_id_list=[]))
            attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_INGRESS_MIRROR_SESSION, value=attr_value)
            self.client.sai_thrift_set_port_attribute(port1, attr)

            self.client.sai_thrift_remove_mirror_session(ingress_mirror_id)

            sai_thrift_delete_fdb(self.client, vlan_id, mac1, port1)
            sai_thrift_delete_fdb(self.client, vlan_id, mac2, port2)

            attr_value = sai_thrift_attribute_value_t(u16=1)
            attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_PORT_VLAN_ID, value=attr_value)
            self.client.sai_thrift_set_port_attribute(port1, attr)

            self.client.sai_thrift_remove_vlan_member(vlan_member1)
            self.client.sai_thrift_remove_vlan_member(vlan_member2)
            self.client.sai_thrift_remove_vlan(vlan_oid)

@group('mirror')
class EgressLocalMirrorTest(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print '----------------------------------------------------------------------------------------------'
        print "Sending packet ptf_intf 1 -> ptf_intf 2, ptf_intf 3 (local mirror)"

        switch_init(self.client)
        vlan_id = 10
        port1 = port_list[1]
        port2 = port_list[2]
        port3 = port_list[3]
        mac1 = '00:11:11:11:11:11'
        mac2 = '00:22:22:22:22:22'
        mac_action = SAI_PACKET_ACTION_FORWARD

        vlan_oid = sai_thrift_create_vlan(self.client, vlan_id)
        vlan_member1 = sai_thrift_create_vlan_member(self.client, vlan_oid, port1, SAI_VLAN_TAGGING_MODE_UNTAGGED)
        vlan_member2 = sai_thrift_create_vlan_member(self.client, vlan_oid, port2, SAI_VLAN_TAGGING_MODE_TAGGED)

        attr_value = sai_thrift_attribute_value_t(u16=vlan_id)
        attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_PORT_VLAN_ID, value=attr_value)
        self.client.sai_thrift_set_port_attribute(port1, attr)

        sai_thrift_create_fdb(self.client, vlan_id, mac1, port1, mac_action)
        sai_thrift_create_fdb(self.client, vlan_id, mac2, port2, mac_action)

        # setup local mirror session
        mirror_type = SAI_MIRROR_SESSION_TYPE_LOCAL
        monitor_port = port3
        print "Create mirror session: mirror_type = SAI_MIRROR_TYPE_LOCAL, monitor_port = ptf_intf 3 "
        egress_mirror_id = sai_thrift_create_mirror_session(self.client,
            mirror_type,
            monitor_port,
            None, None, None,
            None, None, None,
            None, None, None,
            None, None, None,
            None)
        print "egress_mirror_id = %d" %egress_mirror_id

        attr_value = sai_thrift_attribute_value_t(objlist=sai_thrift_object_list_t(count=1,object_id_list=[egress_mirror_id]))
        attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_EGRESS_MIRROR_SESSION, value=attr_value)
        self.client.sai_thrift_set_port_attribute(port2, attr)

        try:
            assert egress_mirror_id > 0, 'egress_mirror_id is <= 0'

            pkt = simple_tcp_packet(eth_dst=mac2,
                eth_src=mac1,
                ip_dst='10.0.0.1',
                ip_src='192.168.0.1',
                ip_id=102,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(eth_dst=mac2,
                eth_src=mac1,
                ip_dst='10.0.0.1',
                ip_src='192.168.0.1',
                dl_vlan_enable=True,
                vlan_vid=10,
                ip_id=102,
                ip_ttl=64,
                pktlen=104)

            print '#### Sending 00:22:22:22:22:22 | 00:11:11:11:11:11 | 10.0.0.1 | 192.168.0.1 | @ ptf_intf 1 ####'
            send_packet(self, 1, str(pkt))
            verify_each_packet_on_each_port(self, [exp_pkt, exp_pkt], [2, 3])

        finally:
            attr_value = sai_thrift_attribute_value_t(objlist=sai_thrift_object_list_t(count=0,object_id_list=[]))
            attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_EGRESS_MIRROR_SESSION, value=attr_value)
            self.client.sai_thrift_set_port_attribute(port2, attr)

            self.client.sai_thrift_remove_mirror_session(egress_mirror_id)

            sai_thrift_delete_fdb(self.client, vlan_id, mac1, port1)
            sai_thrift_delete_fdb(self.client, vlan_id, mac2, port2)

            attr_value = sai_thrift_attribute_value_t(u16=1)
            attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_PORT_VLAN_ID, value=attr_value)
            self.client.sai_thrift_set_port_attribute(port1, attr)

            self.client.sai_thrift_remove_vlan_member(vlan_member1)
            self.client.sai_thrift_remove_vlan_member(vlan_member2)
            self.client.sai_thrift_remove_vlan(vlan_oid)

@group('mirror')
class EgressERSpanMirrorTest(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print
        print '----------------------------------------------------------------------------------------------'
        print "Sending packet ptf_intf 1 -> ptf_intf 2, ptf_intf 3 (erspan mirror)"

        switch_init(self.client)
        vlan_id = 10
        port1 = port_list[1]
        port2 = port_list[2]
        port3 = port_list[3]
        mac1 = '00:11:11:11:11:11'
        mac2 = '00:22:22:22:22:22'
        mac_action = SAI_PACKET_ACTION_FORWARD

        vlan_oid = sai_thrift_create_vlan(self.client, vlan_id)
        vlan_member1 = sai_thrift_create_vlan_member(self.client, vlan_oid, port1, SAI_VLAN_TAGGING_MODE_UNTAGGED)
        vlan_member2 = sai_thrift_create_vlan_member(self.client, vlan_oid, port2, SAI_VLAN_TAGGING_MODE_TAGGED)

        attr_value = sai_thrift_attribute_value_t(u16=vlan_id)
        attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_PORT_VLAN_ID, value=attr_value)
        self.client.sai_thrift_set_port_attribute(port1, attr)

        sai_thrift_create_fdb(self.client, vlan_id, mac1, port1, mac_action)
        sai_thrift_create_fdb(self.client, vlan_id, mac2, port2, mac_action)

        # setup enhanced remote mirror session
        mirror_type = SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE
        monitor_port = port3
        vlan = vlan_id
        vlan_header_valid = True
        iphdr_version = 4
        tunnel_src_ip = "1.1.1.1"
        tunnel_dst_ip = "1.1.1.2"

        tunnel_src_mac = router_mac
        tunnel_dst_mac = "00:33:33:33:33:33"
        gre_protocol = 47
        ttl = 64

        print "Create mirror session: mirror_type = SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE, monitor_port = ptf_intf 3 "
        egress_mirror_id = sai_thrift_create_mirror_session(self.client,
            mirror_type,
            monitor_port,
            vlan, None, None, vlan_header_valid,
            tunnel_src_mac, tunnel_dst_mac,
            tunnel_src_ip, tunnel_dst_ip,
            None, iphdr_version, ttl, None, gre_protocol)
        print "egress_mirror_id = %d" %egress_mirror_id

        attr_value = sai_thrift_attribute_value_t(objlist=sai_thrift_object_list_t(count=1,object_id_list=[egress_mirror_id]))
        attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_EGRESS_MIRROR_SESSION, value=attr_value)
        self.client.sai_thrift_set_port_attribute(port2, attr)

        try:
            assert egress_mirror_id > 0, 'egress_mirror_id is <= 0'

            pkt = simple_tcp_packet(eth_dst=mac2,
                eth_src=mac1,
                ip_dst='10.0.0.1',
                ip_src='192.168.0.1',
                ip_id=102,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(eth_dst=mac2,
                eth_src=mac1,
                ip_dst='10.0.0.1',
                ip_src='192.168.0.1',
                dl_vlan_enable=True,
                vlan_vid=10,
                ip_id=102,
                ip_ttl=64,
                pktlen=104)
            exp_mirrored_pkt = ipv4_erspan_platform_pkt(eth_dst=tunnel_dst_mac,
                eth_src=tunnel_src_mac,
                ip_src=tunnel_src_ip,
                ip_dst=tunnel_dst_ip,
                dl_vlan_enable=True,
                vlan_vid=10,
                ip_id=0,
                ip_ttl=64,
                version=2,
                mirror_id=(egress_mirror_id & 0x3FFFFFFF),
                inner_frame=exp_pkt)

            exp_mask_mirrored_pkt = Mask(exp_mirrored_pkt)
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.IP, 'len')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.IP, 'flags')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.IP, 'chksum')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.GRE, 'proto')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.PlatformSpecific, 'platf_id')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.PlatformSpecific, 'info1')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.PlatformSpecific, 'info2')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.ERSPAN_III, 'span_id')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.ERSPAN_III, 'timestamp')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.ERSPAN_III, 'sgt_other')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.ERSPAN_III, 'direction')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.ERSPAN_III, 'version')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.ERSPAN_III, 'vlan')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.ERSPAN_III, 'priority')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.ERSPAN_III, 'truncated')
            exp_mask_mirrored_pkt.set_do_not_care_scapy(ptf.packet.ERSPAN_III, 'unknown2')

            print '#### Sending 00:22:22:22:22:22 | 00:11:11:11:11:11 | 10.0.0.1 | 192.168.0.1 | @ ptf_intf 1 ####'
            send_packet(self, 1, str(pkt))
            verify_packet(self, exp_pkt, 2)
            verify_packet(self, exp_mask_mirrored_pkt, 3)

        finally:
            attr_value = sai_thrift_attribute_value_t(objlist=sai_thrift_object_list_t(count=0,object_id_list=[]))
            attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_EGRESS_MIRROR_SESSION, value=attr_value)
            self.client.sai_thrift_set_port_attribute(port2, attr)

            self.client.sai_thrift_remove_mirror_session(egress_mirror_id)

            sai_thrift_delete_fdb(self.client, vlan_id, mac1, port1)
            sai_thrift_delete_fdb(self.client, vlan_id, mac2, port2)

            attr_value = sai_thrift_attribute_value_t(u16=1)
            attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_PORT_VLAN_ID, value=attr_value)
            self.client.sai_thrift_set_port_attribute(port1, attr)

            self.client.sai_thrift_remove_vlan_member(vlan_member1)
            self.client.sai_thrift_remove_vlan_member(vlan_member2)
            self.client.sai_thrift_remove_vlan(vlan_oid)

@group('mirror')
class erspanaclmonitor(sai_base_test.ThriftInterfaceDataPlane):
    '''
    This test performs erspan monitoring
    From port2(source port) we send traffic to port 3
    erspan mirror packets are expected on port 1(monitor port)
    '''
    def runTest(self):
        print
        switch_init(self.client)
        port1 = port_list[0]
        port2 = port_list[1]
        port3 = port_list[2]
        mac3='00:00:00:00:00:33'
        mac2='00:00:00:00:00:22'
        monitor_port=port1
        source_port=port2
        mirror_type=SAI_MIRROR_SESSION_TYPE_ENHANCED_REMOTE
        vlan=0x1
        vlan_tpid=0x8100
        vlan_pri=0x6
        src_mac='00:00:00:00:11:22'
        dst_mac='00:00:00:00:11:33'
        encap_type=SAI_ERSPAN_ENCAPSULATION_TYPE_MIRROR_L3_GRE_TUNNEL
        ip_version=0x4
        tos=0
        ttl=64
        gre_type=0x88be
        src_ip='17.18.19.0'
        dst_ip='33.19.20.0'
        src_ip1='41.42.43.44'
        dst_ip1='45.46.47.48'
        src_mac1='00:00:00:00:22:33'
        dst_mac1='00:00:00:00:22:44'
        addr_family=0
        vlan_remote_id = 3
        vlan_oid = sai_thrift_create_vlan(self.client, vlan)
        vlan_remote_oid = sai_thrift_create_vlan(self.client, vlan_remote_id)
        mac_action = SAI_PACKET_ACTION_FORWARD

        sai_thrift_create_fdb(self.client, vlan_remote_id, mac3, port3, mac_action)
        sai_thrift_create_fdb(self.client, vlan_remote_id, mac2, port2, mac_action)

        # Put ports under test in VLAN 3
        vlan_member1 = sai_thrift_create_vlan_member(self.client, vlan_remote_oid, port3, SAI_VLAN_TAGGING_MODE_UNTAGGED)
        vlan_member2 = sai_thrift_create_vlan_member(self.client, vlan_remote_oid, port2, SAI_VLAN_TAGGING_MODE_UNTAGGED)

        # Remove ports from default VLAN
        sai_thrift_vlan_remove_ports(self.client, vlan_oid, [port2, port3])

        # Set PVID
        attr_value = sai_thrift_attribute_value_t(u16=vlan_remote_id)
        attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_PORT_VLAN_ID, value=attr_value)
        self.client.sai_thrift_set_port_attribute(port1, attr)
        self.client.sai_thrift_set_port_attribute(port2, attr)
        self.client.sai_thrift_set_port_attribute(port3, attr)

        erspanid=sai_thrift_create_mirror_session(
                          self.client,
                          mirror_type=mirror_type,
                          port=monitor_port,
                          vlan=vlan,
                          vlan_priority=vlan_pri,
                          vlan_tpid=vlan_tpid,
                          vlan_header_valid=False,
                          src_mac=src_mac,
                          dst_mac=dst_mac,
                          src_ip=src_ip,
                          dst_ip=dst_ip,
                          encap_type=encap_type,
                          iphdr_version=ip_version,
                          ttl=ttl,
                          tos=tos,
                          gre_type=gre_type)

        # setup ACL to block based on Source IP
        addr_family = SAI_IP_ADDR_FAMILY_IPV4
        table_stage = SAI_ACL_STAGE_INGRESS
        table_bind_point_list = [SAI_ACL_BIND_POINT_TYPE_PORT]
        entry_priority = SAI_SWITCH_ATTR_ACL_ENTRY_MINIMUM_PRIORITY
        action = None
        in_ports = [port1, port2]
        mac_src = None
        mac_dst = None
        mac_src_mask = None
        mac_dst_mask = None
        ip_src = "192.168.0.1"
        ip_src_mask = "255.255.255.0"
        ip_dst = "10.0.0.1"
        ip_dst_mask = "255.255.255.0"
        ip_proto = 6
        in_port = None
        out_port = None
        out_ports = None
        src_l4_port = None
        dst_l4_port = None
        ingress_mirror_id = erspanid
        egress_mirror_id = None
        range_list = None

        acl_table_id = sai_thrift_create_acl_table(self.client,
            table_stage,
            table_bind_point_list,
            addr_family,
            mac_src,
            mac_dst,
            ip_src,
            ip_dst,
            ip_proto,
            in_ports,
            out_ports,
            in_port,
            out_port,
            src_l4_port,
            dst_l4_port,
            range_list)
        acl_entry_id = sai_thrift_create_acl_entry(self.client,
            acl_table_id,
            entry_priority,
            action, addr_family,
            mac_src, mac_src_mask,
            mac_dst, mac_dst_mask,
            ip_src, ip_src_mask,
            ip_dst, ip_dst_mask,
            ip_proto,
            in_ports, out_ports,
            in_port, out_port,
            src_l4_port, dst_l4_port,
            ingress_mirror_id,
            egress_mirror_id,
            range_list)

        # bind this ACL table to port2s object id
        attr_value = sai_thrift_attribute_value_t(oid=acl_table_id)
        attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_INGRESS_ACL, value=attr_value)
        self.client.sai_thrift_set_port_attribute(port2, attr)

        pkt = simple_tcp_packet(eth_dst='00:00:00:00:00:33',
                                eth_src='00:00:00:00:00:22',
                                ip_dst='10.0.0.1',
                                ip_id=101,
                                ip_ttl=64,
                                tcp_sport=4000,
                                tcp_dport=5000)

        eth_pkt = simple_eth_packet(pktlen=100, eth_dst='00:00:00:00:00:33', eth_src='00:00:00:00:00:22')

        exp_mirrored_pkt = ipv4_erspan_pkt(
            eth_src='00:00:00:00:11:22',
            eth_dst='00:00:00:00:11:33',
            ip_src='17.18.19.0',
            ip_dst='33.19.20.0',
            ip_id=0,
            ip_ttl=64,
            ip_flags=0x2,
            version=2,
            mirror_id=1,
            inner_frame=pkt)

        exp_eth_mirrored_pkt = ipv4_erspan_pkt(
            eth_src='00:00:00:00:11:22',
            eth_dst='00:00:00:00:11:33',
            ip_src='17.18.19.0',
            ip_dst='33.19.20.0',
            ip_id=0,
            ip_ttl=64,
            ip_flags=0x2,
            version=2,
            mirror_id=1,
            inner_frame=eth_pkt)

        try:
            # in tuple: 0 is device number, 2 is port number
            # this tuple uniquely identifies a port
            # for ingress mirroring
            print "Checking INGRESS ERSPAN Mirroring"
            print "Sending packet port 2 -> port 3 (00:22:22:22:22:22 -> 00:00:00:00:00:33)"
            send_packet(self, 1, pkt)
            verify_packet(self, pkt, 2)
            verify_erspan3_packet(self, exp_mirrored_pkt, 0)


        finally:
            sai_thrift_delete_fdb(self.client, 3, mac2, port2)
            sai_thrift_delete_fdb(self.client, 3, mac3, port3)

            # unbind this ACL table from port2s object id
            attr_value = sai_thrift_attribute_value_t(oid=SAI_NULL_OBJECT_ID)
            attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_INGRESS_ACL, value=attr_value)
            self.client.sai_thrift_set_port_attribute(port2, attr)
            # cleanup ACL
            self.client.sai_thrift_remove_acl_entry(acl_entry_id)
            self.client.sai_thrift_remove_acl_table(acl_table_id)

            # Now you can remove destination
            self.client.sai_thrift_remove_mirror_session(erspanid)

            # Remove ports from VLAN 3
            self.client.sai_thrift_remove_vlan_member(vlan_member1)
            self.client.sai_thrift_remove_vlan_member(vlan_member2)
            self.client.sai_thrift_remove_vlan(vlan_remote_oid)

            # Add ports back to default VLAN
            vlan_member2a = sai_thrift_create_vlan_member(self.client, vlan_oid, port2, SAI_VLAN_TAGGING_MODE_UNTAGGED)
            vlan_member3a = sai_thrift_create_vlan_member(self.client, vlan_oid, port3, SAI_VLAN_TAGGING_MODE_UNTAGGED)

            attr_value = sai_thrift_attribute_value_t(u16=1)
            attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_PORT_VLAN_ID, value=attr_value)
            self.client.sai_thrift_set_port_attribute(port1, attr)
            self.client.sai_thrift_set_port_attribute(port2, attr)
            self.client.sai_thrift_set_port_attribute(port3, attr)

