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
Thrift SAI Tunnel tests
"""

import socket

from switch import *
from ptf.mask import Mask
import sai_base_test
from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *
from switchsai_thrift.sai_headers import *
from switch_utils import *

import os
this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '..'))
from common.utils import *


def sai_thrift_create_loopback_rif(client, vr_oid, rmac, v4=1, v6=1):
    #vrf attribute
    rif_attr_list = []
    rif_attribute1_value = sai_thrift_attribute_value_t(oid=vr_oid)
    rif_attribute1 = sai_thrift_attribute_t(
        id=SAI_ROUTER_INTERFACE_ATTR_VIRTUAL_ROUTER_ID,
        value=rif_attribute1_value)
    #interface type
    rif_attr_list.append(rif_attribute1)
    rif_attribute2_value = sai_thrift_attribute_value_t(
        s32=SAI_ROUTER_INTERFACE_TYPE_LOOPBACK)
    rif_attribute2 = sai_thrift_attribute_t(
        id=SAI_ROUTER_INTERFACE_ATTR_TYPE, value=rif_attribute2_value)
    rif_attr_list.append(rif_attribute2)

    #v4_enabled
    rif_attribute4_value = sai_thrift_attribute_value_t(booldata=v4)
    rif_attribute4 = sai_thrift_attribute_t(
        id=SAI_ROUTER_INTERFACE_ATTR_ADMIN_V4_STATE,
        value=rif_attribute4_value)
    rif_attr_list.append(rif_attribute4)
    #v6_enabled
    rif_attribute5_value = sai_thrift_attribute_value_t(booldata=v6)
    rif_attribute5 = sai_thrift_attribute_t(
        id=SAI_ROUTER_INTERFACE_ATTR_ADMIN_V6_STATE,
        value=rif_attribute5_value)
    rif_attr_list.append(rif_attribute5)

    if rmac:
        rif_attribute6_value = sai_thrift_attribute_value_t(mac=rmac)
        rif_attribute6 = sai_thrift_attribute_t(
            id=SAI_ROUTER_INTERFACE_ATTR_SRC_MAC_ADDRESS,
            value=rif_attribute6_value)
        rif_attr_list.append(rif_attribute6)

    rif_id = client.sai_thrift_create_router_interface(rif_attr_list)
    return rif_id


def sai_thrift_create_nhop_tunnel(client, tunnel, ip_addr, mac='', addr_family=SAI_IP_ADDR_FAMILY_IPV4, vni=0):
    attr_list = []
    #ip addr
    if addr_family == SAI_IP_ADDR_FAMILY_IPV4:
        addr = sai_thrift_ip_t(ip4=ip_addr)
        ipaddr = sai_thrift_ip_address_t(
            addr_family=SAI_IP_ADDR_FAMILY_IPV4, addr=addr)
    else:
        addr = sai_thrift_ip_t(ip6=ip_addr)
        ipaddr = sai_thrift_ip_address_t(
            addr_family=SAI_IP_ADDR_FAMILY_IPV6, addr=addr)
    attr_value = sai_thrift_attribute_value_t(ipaddr=ipaddr)
    attr = sai_thrift_attribute_t(id=SAI_NEXT_HOP_ATTR_IP, value=attr_value)
    attr_list.append(attr)
    #tunnel type
    attr_value = sai_thrift_attribute_value_t(
        s32=SAI_NEXT_HOP_TYPE_TUNNEL_ENCAP)
    attr = sai_thrift_attribute_t(id=SAI_NEXT_HOP_ATTR_TYPE, value=attr_value)
    attr_list.append(attr)
    #tunnel id
    attr_value = sai_thrift_attribute_value_t(oid=tunnel)
    attr = sai_thrift_attribute_t(
        id=SAI_NEXT_HOP_ATTR_TUNNEL_ID, value=attr_value)
    attr_list.append(attr)
    #tunnel vni
    if vni:
        attr_value = sai_thrift_attribute_value_t(u32=vni)
        attr = sai_thrift_attribute_t(
            id=SAI_NEXT_HOP_ATTR_TUNNEL_VNI, value=attr_value)
        attr_list.append(attr)
    #tunnel mac
    if mac:
        attr_value = sai_thrift_attribute_value_t(mac=mac)
        attr = sai_thrift_attribute_t(
            id=SAI_NEXT_HOP_ATTR_TUNNEL_MAC, value=attr_value)
        attr_list.append(attr)

    nhop = client.sai_thrift_create_next_hop(thrift_attr_list=attr_list)
    return nhop


def sai_thrift_create_tunnel(client,
                             type,
                             tunnel_sip,
                             urif,
                             orif=0,
                             imap=0,
                             emap=0):
    tunnel_attr_list = []
    #tunnel type
    attr_value = sai_thrift_attribute_value_t(s32=type)
    attr = sai_thrift_attribute_t(id=SAI_TUNNEL_ATTR_TYPE, value=attr_value)
    tunnel_attr_list.append(attr)
    #underlay rif
    attr_value = sai_thrift_attribute_value_t(oid=urif)
    attr = sai_thrift_attribute_t(
        id=SAI_TUNNEL_ATTR_UNDERLAY_INTERFACE, value=attr_value)
    tunnel_attr_list.append(attr)
    #overlay rif
    if orif:
        attr_value = sai_thrift_attribute_value_t(oid=orif)
        attr = sai_thrift_attribute_t(
            id=SAI_TUNNEL_ATTR_OVERLAY_INTERFACE, value=attr_value)
        tunnel_attr_list.append(attr)
    #src ip addr
    addr = sai_thrift_ip_t(ip4=tunnel_sip)
    ip_addr = sai_thrift_ip_address_t(
        addr_family=SAI_IP_ADDR_FAMILY_IPV4, addr=addr)
    attr_value = sai_thrift_attribute_value_t(ipaddr=ip_addr)
    attr = sai_thrift_attribute_t(
        id=SAI_TUNNEL_ATTR_ENCAP_SRC_IP, value=attr_value)
    tunnel_attr_list.append(attr)
    #encap mapper
    attr_value = sai_thrift_attribute_value_t(
        objlist=sai_thrift_object_list_t(count=1, object_id_list=[imap]))
    attr = sai_thrift_attribute_t(
        id=SAI_TUNNEL_ATTR_ENCAP_MAPPERS, value=attr_value)
    tunnel_attr_list.append(attr)
    #decap mapper
    attr_value = sai_thrift_attribute_value_t(
        objlist=sai_thrift_object_list_t(count=1, object_id_list=[emap]))
    attr = sai_thrift_attribute_t(
        id=SAI_TUNNEL_ATTR_DECAP_MAPPERS, value=attr_value)
    tunnel_attr_list.append(attr)
    tunnel_id = client.sai_thrift_create_tunnel(tunnel_attr_list)
    return tunnel_id


def sai_thrift_create_tunnel_term(client, type, vr_id, src_ip, dst_ip,
                                  tunnel_id, tunnel_type):
    tunnel_term_list = []
    #entry typr
    attr_value = sai_thrift_attribute_value_t(s32=type)
    attr = sai_thrift_attribute_t(
        id=SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_TYPE, value=attr_value)
    tunnel_term_list.append(attr)
    #vrf id
    attr_value = sai_thrift_attribute_value_t(oid=vr_id)
    attr = sai_thrift_attribute_t(
        id=SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_VR_ID, value=attr_value)
    tunnel_term_list.append(attr)
    #src ip
    if type == SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2P:
        addr = sai_thrift_ip_t(ip4=src_ip)
        ip_addr = sai_thrift_ip_address_t(
            addr_family=SAI_IP_ADDR_FAMILY_IPV4, addr=addr)
        attr_value = sai_thrift_attribute_value_t(ipaddr=ip_addr)
        attr = sai_thrift_attribute_t(
            id=SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_SRC_IP, value=attr_value)
        tunnel_term_list.append(attr)
    #dst ip
    addr = sai_thrift_ip_t(ip4=dst_ip)
    ip_addr = sai_thrift_ip_address_t(
        addr_family=SAI_IP_ADDR_FAMILY_IPV4, addr=addr)
    attr_value = sai_thrift_attribute_value_t(ipaddr=ip_addr)
    attr = sai_thrift_attribute_t(
        id=SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_DST_IP, value=attr_value)
    tunnel_term_list.append(attr)
    #vrf id
    attr_value = sai_thrift_attribute_value_t(oid=tunnel_id)
    attr = sai_thrift_attribute_t(
        id=SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_ACTION_TUNNEL_ID, value=attr_value)
    tunnel_term_list.append(attr)
    #tunnel type
    attr_value = sai_thrift_attribute_value_t(s32=tunnel_type)
    attr = sai_thrift_attribute_t(
        id=SAI_TUNNEL_TERM_TABLE_ENTRY_ATTR_TUNNEL_TYPE, value=attr_value)
    tunnel_term_list.append(attr)
    tunnel_term_id = client.sai_thrift_create_tunnel_term(tunnel_term_list)
    return tunnel_term_id


def sai_thrift_create_tunnel_map(client, map_type):
    attr_list = []
    #tunnel map type
    attr_value = sai_thrift_attribute_value_t(s32=map_type)
    attr = sai_thrift_attribute_t(
        id=SAI_TUNNEL_MAP_ATTR_TYPE, value=attr_value)
    attr_list.append(attr)
    tunnel_map_id = client.sai_thrift_create_tunnel_map(attr_list)
    return tunnel_map_id


def sai_thrift_create_tunnel_map_entry(client, map_type, tunnel_map_id, ln,
                                       vlan, vrf, vni):
    attr_list = []
    #tunnel map type
    attr_value = sai_thrift_attribute_value_t(s32=map_type)
    attr = sai_thrift_attribute_t(
        id=SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP_TYPE, value=attr_value)
    attr_list.append(attr)
    # tunnel map
    attr_value = sai_thrift_attribute_value_t(oid=tunnel_map_id)
    attr = sai_thrift_attribute_t(
        id=SAI_TUNNEL_MAP_ENTRY_ATTR_TUNNEL_MAP, value=attr_value)
    attr_list.append(attr)
    if (map_type == SAI_TUNNEL_MAP_TYPE_VNI_TO_BRIDGE_IF):
        #ln handle
        attr_value = sai_thrift_attribute_value_t(oid=ln)
        attr = sai_thrift_attribute_t(
            id=SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_VALUE, value=attr_value)
        attr_list.append(attr)
    if (map_type == SAI_TUNNEL_MAP_TYPE_VNI_TO_VLAN_ID):
        #vlan handle
        attr_value = sai_thrift_attribute_value_t(u16=vlan)
        attr = sai_thrift_attribute_t(
            id=SAI_TUNNEL_MAP_ENTRY_ATTR_VLAN_ID_VALUE, value=attr_value)
        attr_list.append(attr)
    if (map_type == SAI_TUNNEL_MAP_TYPE_VNI_TO_VIRTUAL_ROUTER_ID):
        #vrf handle
        attr_value = sai_thrift_attribute_value_t(oid=vrf)
        attr = sai_thrift_attribute_t(
            id=SAI_TUNNEL_MAP_ENTRY_ATTR_VIRTUAL_ROUTER_ID_VALUE,
            value=attr_value)
        attr_list.append(attr)
    #vni handle
    attr_value = sai_thrift_attribute_value_t(u32=vni)
    attr = sai_thrift_attribute_t(
        id=SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_VALUE, value=attr_value)
    attr_list.append(attr)

    if (map_type == SAI_TUNNEL_MAP_TYPE_BRIDGE_IF_TO_VNI):
        #ln handle
        attr_value = sai_thrift_attribute_value_t(oid=ln)
        attr = sai_thrift_attribute_t(
            id=SAI_TUNNEL_MAP_ENTRY_ATTR_BRIDGE_ID_KEY, value=attr_value)
        attr_list.append(attr)
    if (map_type == SAI_TUNNEL_MAP_TYPE_VLAN_ID_TO_VNI):
        #vlan handle
        attr_value = sai_thrift_attribute_value_t(u16=vlan)
        attr = sai_thrift_attribute_t(
            id=SAI_TUNNEL_MAP_ENTRY_ATTR_VLAN_ID_KEY, value=attr_value)
        attr_list.append(attr)
    if (map_type == SAI_TUNNEL_MAP_TYPE_VIRTUAL_ROUTER_ID_TO_VNI):
        #vrf handle
        attr_value = sai_thrift_attribute_value_t(oid=vrf)
        attr = sai_thrift_attribute_t(
            id=SAI_TUNNEL_MAP_ENTRY_ATTR_VIRTUAL_ROUTER_ID_KEY,
            value=attr_value)
        attr_list.append(attr)
    #vni handle
    attr_value = sai_thrift_attribute_value_t(u32=vni)
    attr = sai_thrift_attribute_t(
        id=SAI_TUNNEL_MAP_ENTRY_ATTR_VNI_ID_KEY, value=attr_value)
    attr_list.append(attr)
    tunnel_map_entry = client.sai_thrift_create_tunnel_map_entry(attr_list)
    return tunnel_map_entry


def sai_thrift_create_tunnel_fdb(client,
                                 vlan_id,
                                 network,
                                 mac,
                                 port,
                                 type,
                                 dst_ip='0.0.0.0'):
    fdb_entry = sai_thrift_fdb_entry_t(
        mac_address=mac, vlan_id=vlan_id, bridge_type=type, bridge_id=network)
    #value 0 represents static entry, id=0, represents entry type
    fdb_attribute1_value = sai_thrift_attribute_value_t(
        s32=SAI_FDB_ENTRY_TYPE_STATIC)
    fdb_attribute1 = sai_thrift_attribute_t(
        id=SAI_FDB_ENTRY_ATTR_TYPE, value=fdb_attribute1_value)
    #value oid represents object id, id=1 represents port id
    fdb_attribute2_value = sai_thrift_attribute_value_t(oid=port)
    fdb_attribute2 = sai_thrift_attribute_t(
        id=SAI_FDB_ENTRY_ATTR_BRIDGE_PORT_ID, value=fdb_attribute2_value)
    #value oid represents object id, id=2 represents port mac action
    fdb_attribute3_value = sai_thrift_attribute_value_t(
        s32=SAI_PACKET_ACTION_FORWARD)
    fdb_attribute3 = sai_thrift_attribute_t(
        id=SAI_FDB_ENTRY_ATTR_PACKET_ACTION, value=fdb_attribute3_value)
    #value oid represents object id, id=2 represents endpoint ip
    addr = sai_thrift_ip_t(ip4=dst_ip)
    ip_addr = sai_thrift_ip_address_t(
        addr_family=SAI_IP_ADDR_FAMILY_IPV4, addr=addr)
    fdb_attribute4_value = sai_thrift_attribute_value_t(ipaddr=ip_addr)
    fdb_attribute4 = sai_thrift_attribute_t(
        id=SAI_FDB_ENTRY_ATTR_ENDPOINT_IP, value=fdb_attribute4_value)
    fdb_attr_list = [
        fdb_attribute1, fdb_attribute2, fdb_attribute3, fdb_attribute4
    ]
    client.sai_thrift_create_fdb_entry(
        thrift_fdb_entry=fdb_entry, thrift_attr_list=fdb_attr_list)


def sai_thrift_remove_tunnel_fdb(client,
                                 vlan_id,
                                 network,
                                 mac,
                                 port,
                                 type,
                                 dst_ip='0.0.0.0'):
    fdb_entry = sai_thrift_fdb_entry_t(
        mac_address=mac, vlan_id=vlan_id, bridge_type=type, bridge_id=network)
    client.sai_thrift_delete_fdb_entry(thrift_fdb_entry=fdb_entry)


@group('tunnel-ocp')
@group('tunnel')
class IPinIPEncapDecap(sai_base_test.ThriftInterfaceDataPlane):
    '''
    This performs ip-in-ip tunnel encap/decap
    '''

    def runTest(self):
        try:
            print
            switch_init(self.client)
            port1 = port_list[0]
            port2 = port_list[1]
            tunnel_type = SAI_TUNNEL_TYPE_IPINIP
            term_type = SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2P
            remote_vtep_ip = '2.2.2.2'
            my_vtep_ip = '2.2.2.3'

            v4 = 1
            v6 = 1
            ovrf = sai_thrift_create_virtual_router(self.client, v4, v6)
            uvrf = sai_thrift_create_virtual_router(self.client, v4, v6)
            sai_thrift_remove_default_bridge_ports(self.client, [port1, port2])

            #overlay rif
            rif1 = sai_thrift_create_router_interface(self.client, ovrf, 1,
                                                      port1, 0, v4, v6, 0)
            #underlay rif
            rif2 = sai_thrift_create_router_interface(self.client, uvrf, 1,
                                                      port2, 0, v4, v6, 0)

            #tunnel creation
            orif = sai_thrift_create_loopback_rif(self.client, ovrf,
                                                  router_mac)
            urif = sai_thrift_create_loopback_rif(self.client, uvrf,
                                                  router_mac)

            tunnel_id = sai_thrift_create_tunnel(self.client, tunnel_type,
                                                 my_vtep_ip, urif, orif)
            tunnel_term_id = sai_thrift_create_tunnel_term(
                self.client, term_type, uvrf, remote_vtep_ip, my_vtep_ip, tunnel_id,
                tunnel_type)

            # static route to tunnel
            tunnel_nhop = sai_thrift_create_nhop_tunnel(
                self.client, tunnel_id, remote_vtep_ip, '00:55:55:55:55:55',
                SAI_IP_ADDR_FAMILY_IPV4)
            sai_thrift_create_route(self.client, ovrf, SAI_IP_ADDR_FAMILY_IPV4,
                                    '10.10.10.0', '255.255.255.0', tunnel_nhop)

            # static route to underlay rif
            rif2_nhop = sai_thrift_create_nhop(
                self.client, SAI_IP_ADDR_FAMILY_IPV4, '2.2.2.2', rif2)
            sai_thrift_create_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4,
                                       rif2, '2.2.2.2', '00:44:44:44:44:44')

            # static route to overlay rif
            rif1_nhop = sai_thrift_create_nhop(
                self.client, SAI_IP_ADDR_FAMILY_IPV4, '11.11.11.1', rif1)
            sai_thrift_create_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4,
                                       rif1, '11.11.11.1', '00:22:22:22:22:22')

            print "Verifying 4in4 (ip in ip encap)"
            pkt = simple_tcp_packet(
                eth_dst=router_mac,
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='11.11.11.1',
                ip_id=105,
                ip_ttl=64)
            pkt2 = simple_tcp_packet(
                eth_dst='00:44:44:44:44:44',
                eth_src=router_mac,
                ip_dst='10.10.10.1',
                ip_src='11.11.11.1',
                ip_id=105,
                ip_ttl=63)
            ipip_pkt = simple_ipv4ip_packet(
                eth_src=router_mac,
                eth_dst='00:44:44:44:44:44',
                ip_id=0,
                ip_src='2.2.2.3',
                ip_dst='2.2.2.2',
                ip_ttl=64,
                ip_flags=0x2,
                inner_frame=pkt2['IP'])
            send_packet(self, 0, str(pkt))
            verify_packet(self, ipip_pkt, 1)

            print "Verifying 4in4 (ip in ip decap)"
            pkt1 = simple_tcp_packet(
                eth_src=router_mac,
                eth_dst='00:22:22:22:22:22',
                ip_dst='11.11.11.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            ipip_pkt = simple_ipv4ip_packet(
                eth_dst=router_mac,
                eth_src='00:44:44:44:44:44',
                ip_id=0,
                ip_src='2.2.2.2',
                ip_dst='2.2.2.3',
                ip_ttl=64,
                ip_flags=0x2,
                inner_frame=pkt1['IP'])
            pkt2 = simple_tcp_packet(
                eth_src=router_mac,
                eth_dst='00:22:22:22:22:22',
                ip_dst='11.11.11.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63)
            send_packet(self, 1, str(ipip_pkt))
            verify_packet(self, pkt2, 0)

        finally:
            #cleanup
            sai_thrift_remove_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4,
                                       rif1, '11.11.11.1', '00:22:22:22:22:22')
            sai_thrift_remove_nhop(self.client, [rif1_nhop])
            sai_thrift_remove_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4,
                                       rif2, '2.2.2.2', '00:44:44:44:44:44')
            sai_thrift_remove_nhop(self.client, [rif2_nhop])
            sai_thrift_remove_route(self.client, ovrf, SAI_IP_ADDR_FAMILY_IPV4,
                                    '10.10.10.0', '255.255.255.0', tunnel_nhop)
            sai_thrift_remove_nhop(self.client, [tunnel_nhop])
            self.client.sai_thrift_remove_tunnel_term(tunnel_term_id)
            self.client.sai_thrift_remove_tunnel(tunnel_id)
            self.client.sai_thrift_remove_router_interface(orif)
            self.client.sai_thrift_remove_router_interface(urif)
            self.client.sai_thrift_remove_router_interface(rif1)
            self.client.sai_thrift_remove_router_interface(rif2)
            sai_thrift_create_default_bridge_ports(self.client, [port1, port2])
            self.client.sai_thrift_remove_virtual_router(ovrf)
            self.client.sai_thrift_remove_virtual_router(uvrf)


@group('tunnel-ocp')
@group('tunnel')
class BridgeVxlanEncapDecap(sai_base_test.ThriftInterfaceDataPlane):
    '''
    This performs L2 Vxlan tunnel encap/decap
    '''

    def runTest(self):
        try:
            print
            switch_init(self.client)
            port1 = port_list[0]
            port2 = port_list[1]
            tunnel_type = SAI_TUNNEL_TYPE_VXLAN
            term_type = SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2P
            remote_vtep_ip = '2.2.2.2'
            my_vtep_ip = '2.2.2.3'
            decap_map_type = SAI_TUNNEL_MAP_TYPE_VNI_TO_BRIDGE_IF
            encap_map_type = SAI_TUNNEL_MAP_TYPE_BRIDGE_IF_TO_VNI
            vni = 10000

            v4 = 1
            v6 = 1
            ovrf = sai_thrift_create_virtual_router(self.client, v4, v6)
            uvrf = sai_thrift_create_virtual_router(self.client, v4, v6)
            sai_thrift_remove_default_bridge_ports(self.client, [port1, port2])

            #underlay rif
            rif2 = sai_thrift_create_router_interface(
                self.client, uvrf, 1, port2, 0, v4, v6, router_mac)

            #tunnel creation
            orif = sai_thrift_create_loopback_rif(self.client, ovrf,
                                                  router_mac)
            urif = sai_thrift_create_loopback_rif(self.client, uvrf,
                                                  router_mac)

            ln = sai_thrift_create_bridge(self.client, SAI_BRIDGE_TYPE_1D)

            decap_tun_map = sai_thrift_create_tunnel_map(self.client, decap_map_type)
            decap_tun_map_entry = sai_thrift_create_tunnel_map_entry(
                self.client, decap_map_type, decap_tun_map, ln, 0, 0, vni)
            encap_tun_map = sai_thrift_create_tunnel_map(self.client, encap_map_type)
            encap_tun_map_entry = sai_thrift_create_tunnel_map_entry(
                self.client, encap_map_type, encap_tun_map, ln, 0, 0, vni)

            tunnel_id = sai_thrift_create_tunnel(
                self.client,
                tunnel_type,
                my_vtep_ip,
                urif,
                orif,
                imap=decap_tun_map,
                emap=encap_tun_map)
            tunnel_term_id = sai_thrift_create_tunnel_term(
                self.client, term_type, uvrf, remote_vtep_ip, my_vtep_ip, tunnel_id,
                tunnel_type)

            ing_port_id = sai_thrift_create_bridge_port(
                self.client, SAI_BRIDGE_PORT_TYPE_PORT, port1, ln)
            ln_port_id = sai_thrift_create_bridge_port(
                self.client, SAI_BRIDGE_PORT_TYPE_TUNNEL, tunnel_id, ln)

            sai_thrift_create_tunnel_fdb(self.client, 0, ln,
                                         '00:22:22:22:22:22', ing_port_id,
                                         SAI_FDB_ENTRY_BRIDGE_TYPE_1D)
            sai_thrift_create_tunnel_fdb(
                self.client, 0, ln, '00:11:11:11:11:11', ln_port_id,
                SAI_FDB_ENTRY_BRIDGE_TYPE_1D, '2.2.2.2')

            # static route to underlay rif
            rif2_nhop = sai_thrift_create_nhop(
                self.client, SAI_IP_ADDR_FAMILY_IPV4, '2.2.2.2', rif2)
            sai_thrift_create_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4,
                                       rif2, '2.2.2.2', '00:44:44:44:44:44')

            print "Verifying 4in4 (Bridge ip in vxlan encap)"
            pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='11.11.11.1',
                ip_id=105,
                ip_ttl=64)
            udp_sport = entropy_hash(pkt)
            vxlan_pkt = simple_vxlan_packet(
                eth_src=router_mac,
                eth_dst='00:44:44:44:44:44',
                ip_id=0,
                ip_src='2.2.2.3',
                ip_dst='2.2.2.2',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=vni,
                inner_frame=pkt)
            send_packet(self, 0, str(pkt))
            verify_packet(self, vxlan_pkt, 1)

            print "Verifying 4in4 (Bridge ip in vxlan decap)"
            pkt1 = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                ip_dst='11.11.11.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            vxlan_pkt = simple_vxlan_packet(
                eth_dst=router_mac,
                eth_src='00:44:44:44:44:44',
                ip_id=0,
                ip_src='2.2.2.2',
                ip_dst='2.2.2.3',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=11638,
                vxlan_vni=vni,
                with_udp_chksum=False,
                inner_frame=pkt1)
            send_packet(self, 1, str(vxlan_pkt))
            verify_packet(self, pkt1, 0)

        finally:
            #cleanup
            sai_thrift_remove_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4,
                                       rif2, '2.2.2.2', '00:44:44:44:44:44')
            sai_thrift_remove_nhop(self.client, [rif2_nhop])
            sai_thrift_remove_tunnel_fdb(
                self.client, 0, ln, '00:11:11:11:11:11', ln_port_id,
                SAI_FDB_ENTRY_BRIDGE_TYPE_1D, '2.2.2.2')
            sai_thrift_remove_tunnel_fdb(self.client, 0, ln,
                                         '00:22:22:22:22:22', ing_port_id,
                                         SAI_FDB_ENTRY_BRIDGE_TYPE_1D)
            self.client.sai_thrift_remove_bridge_port(ln_port_id)
            self.client.sai_thrift_remove_bridge_port(ing_port_id)
            self.client.sai_thrift_remove_tunnel_term(tunnel_term_id)
            self.client.sai_thrift_remove_tunnel(tunnel_id)
            self.client.sai_thrift_remove_tunnel_map_entry(encap_tun_map_entry)
            self.client.sai_thrift_remove_tunnel_map_entry(decap_tun_map_entry)
            self.client.sai_thrift_remove_tunnel_map(encap_tun_map)
            self.client.sai_thrift_remove_tunnel_map(decap_tun_map)
            self.client.sai_thrift_remove_bridge(ln)
            self.client.sai_thrift_remove_router_interface(orif)
            self.client.sai_thrift_remove_router_interface(urif)
            self.client.sai_thrift_remove_router_interface(rif2)
            sai_thrift_create_default_bridge_ports(self.client, [port1, port2])
            self.client.sai_thrift_remove_virtual_router(ovrf)
            self.client.sai_thrift_remove_virtual_router(uvrf)


@group('tunnel-ocp')
@group('tunnel')
class VlanVxlanEncapDecap(sai_base_test.ThriftInterfaceDataPlane):
    '''
    This performs L2 Vxlan tunnel encap/decap
    '''

    def runTest(self):
        try:
            print
            switch_init(self.client)
            port1 = port_list[0]
            port2 = port_list[1]
            tunnel_type = SAI_TUNNEL_TYPE_VXLAN
            term_type = SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2P
            remote_vtep_ip = '2.2.2.2'
            my_vtep_ip = '2.2.2.3'
            decap_map_type = SAI_TUNNEL_MAP_TYPE_VNI_TO_VLAN_ID
            encap_map_type = SAI_TUNNEL_MAP_TYPE_VLAN_ID_TO_VNI
            vni = 10000

            vlan_id = 10
            vlan_oid = sai_thrift_create_vlan(self.client, vlan_id)
            vlan_member1 = sai_thrift_create_vlan_member(
                self.client, vlan_oid, port1, SAI_VLAN_TAGGING_MODE_UNTAGGED)
            sai_thrift_remove_default_bridge_ports(self.client, [port2])
            v4 = 1
            v6 = 1
            ovrf = sai_thrift_create_virtual_router(self.client, v4, v6)
            uvrf = sai_thrift_create_virtual_router(self.client, v4, v6)

            #underlay rif
            rif2 = sai_thrift_create_router_interface(
                self.client, uvrf, 1, port2, 0, v4, v6, router_mac)

            #tunnel creation
            orif = sai_thrift_create_loopback_rif(self.client, ovrf,
                                                  router_mac)
            urif = sai_thrift_create_loopback_rif(self.client, uvrf,
                                                  router_mac)

            decap_tun_map = sai_thrift_create_tunnel_map(self.client, decap_map_type)
            decap_tun_map_entry = sai_thrift_create_tunnel_map_entry(
                self.client, decap_map_type, decap_tun_map, 0, vlan_id, 0, vni)
            encap_tun_map = sai_thrift_create_tunnel_map(self.client, encap_map_type)
            encap_tun_map_entry = sai_thrift_create_tunnel_map_entry(
                self.client, encap_map_type, encap_tun_map, 0, vlan_id, 0, vni)

            tunnel_id = sai_thrift_create_tunnel(
                self.client,
                tunnel_type,
                my_vtep_ip,
                urif,
                orif,
                imap=decap_tun_map,
                emap=encap_tun_map)
            tunnel_term_id = sai_thrift_create_tunnel_term(
                self.client, term_type, uvrf, remote_vtep_ip, my_vtep_ip, tunnel_id,
                tunnel_type)

            # this basically fetches the tunnel interface reference
            br_port_id = sai_thrift_create_bridge_port(
                self.client, SAI_BRIDGE_PORT_TYPE_TUNNEL, tunnel_id, 0)

            attrs = self.client.sai_thrift_get_vlan_member_attribute(
                vlan_member1)
            for a in attrs.attr_list:
                if a.id == SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID:
                    bport_oid = a.value.oid
                    break

            sai_thrift_create_fdb(
                self.client,
                vlan_id,
                '00:22:22:22:22:22',
                port1,
                SAI_PACKET_ACTION_FORWARD,
                port_oid=bport_oid)
            sai_thrift_create_tunnel_fdb(
                self.client, vlan_id, 0, '00:11:11:11:11:11', br_port_id,
                SAI_FDB_ENTRY_BRIDGE_TYPE_1Q, '2.2.2.2')

            # static route to underlay rif
            rif2_nhop = sai_thrift_create_nhop(
                self.client, SAI_IP_ADDR_FAMILY_IPV4, '2.2.2.2', rif2)
            sai_thrift_create_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4,
                                       rif2, '2.2.2.2', '00:44:44:44:44:44')

            print "Verifying 4in4 (Vlan ip in vxlan encap)"
            pkt = simple_tcp_packet(
                eth_dst='00:11:11:11:11:11',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='11.11.11.1',
                ip_id=105,
                ip_ttl=64)
            udp_sport = entropy_hash(pkt)
            vxlan_pkt = simple_vxlan_packet(
                eth_src=router_mac,
                eth_dst='00:44:44:44:44:44',
                ip_id=0,
                ip_src='2.2.2.3',
                ip_dst='2.2.2.2',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=vni,
                inner_frame=pkt)
            send_packet(self, 0, str(pkt))
            verify_packet(self, vxlan_pkt, 1)

            print "Verifying 4in4 (Vlan ip in vxlan decap)"
            pkt1 = simple_tcp_packet(
                eth_dst='00:22:22:22:22:22',
                eth_src='00:11:11:11:11:11',
                ip_dst='11.11.11.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            vxlan_pkt = simple_vxlan_packet(
                eth_dst=router_mac,
                eth_src='00:44:44:44:44:44',
                ip_id=0,
                ip_src='2.2.2.2',
                ip_dst='2.2.2.3',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=11638,
                vxlan_vni=vni,
                with_udp_chksum=False,
                inner_frame=pkt1)
            send_packet(self, 1, str(vxlan_pkt))
            verify_packet(self, pkt1, 0)

        finally:
            #cleanup
            sai_thrift_remove_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4,
                                       rif2, '2.2.2.2', '00:44:44:44:44:44')
            sai_thrift_remove_nhop(self.client, [rif2_nhop])
            sai_thrift_remove_tunnel_fdb(
                self.client, vlan_id, 0, '00:11:11:11:11:11', br_port_id,
                SAI_FDB_ENTRY_BRIDGE_TYPE_1Q, '2.2.2.2')
            sai_thrift_delete_fdb(self.client, vlan_id, '00:22:22:22:22:22',
                                  port1)
            self.client.sai_thrift_remove_bridge_port(br_port_id)
            self.client.sai_thrift_remove_tunnel_term(tunnel_term_id)
            self.client.sai_thrift_remove_tunnel(tunnel_id)
            self.client.sai_thrift_remove_tunnel_map_entry(encap_tun_map_entry)
            self.client.sai_thrift_remove_tunnel_map_entry(decap_tun_map_entry)
            self.client.sai_thrift_remove_tunnel_map(encap_tun_map)
            self.client.sai_thrift_remove_tunnel_map(decap_tun_map)
            self.client.sai_thrift_remove_router_interface(orif)
            self.client.sai_thrift_remove_router_interface(urif)
            self.client.sai_thrift_remove_router_interface(rif2)
            sai_thrift_create_default_bridge_ports(self.client, [port2])
            self.client.sai_thrift_remove_virtual_router(ovrf)
            self.client.sai_thrift_remove_virtual_router(uvrf)
            self.client.sai_thrift_remove_vlan_member(vlan_member1)
            self.client.sai_thrift_remove_vlan(vlan_oid)


@group('tunnel-ocp')
@group('tunnel')
class L3VxlanEncapDecap(sai_base_test.ThriftInterfaceDataPlane):
    '''
    This performs ip-in-vxlan tunnel encap/decap
    '''

    def runTest(self):
        try:
            print
            switch_init(self.client)
            port1 = port_list[0]
            port2 = port_list[1]
            tunnel_type = SAI_TUNNEL_TYPE_VXLAN
            term_type = SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2P
            remote_vtep_ip = '2.2.2.2'
            my_vtep_ip = '2.2.2.3'
            decap_map_type = SAI_TUNNEL_MAP_TYPE_VNI_TO_VIRTUAL_ROUTER_ID
            encap_map_type = SAI_TUNNEL_MAP_TYPE_VIRTUAL_ROUTER_ID_TO_VNI
            vni = 10000

            v4 = 1
            v6 = 1
            ovrf = sai_thrift_create_virtual_router(self.client, v4, v6)
            uvrf = sai_thrift_create_virtual_router(self.client, v4, v6)
            sai_thrift_remove_default_bridge_ports(self.client, [port1, port2])

            #overlay rif
            rif1 = sai_thrift_create_router_interface(self.client, ovrf, 1,
                                                      port1, 0, v4, v6, 0)
            #underlay rif
            rif2 = sai_thrift_create_router_interface(self.client, uvrf, 1,
                                                      port2, 0, v4, v6, 0)

            #tunnel creation
            orif = sai_thrift_create_loopback_rif(self.client, ovrf,
                                                  router_mac)
            urif = sai_thrift_create_loopback_rif(self.client, uvrf,
                                                  router_mac)

            decap_tun_map = sai_thrift_create_tunnel_map(self.client, decap_map_type)
            decap_tun_map_entry = sai_thrift_create_tunnel_map_entry(
                self.client, decap_map_type, decap_tun_map, 0, 0, ovrf, vni)
            encap_tun_map = sai_thrift_create_tunnel_map(self.client, encap_map_type)
            encap_tun_map_entry = sai_thrift_create_tunnel_map_entry(
                self.client, encap_map_type, encap_tun_map, 0, 0, ovrf, vni)

            tunnel_id = sai_thrift_create_tunnel(
                self.client,
                tunnel_type,
                my_vtep_ip,
                urif,
                orif,
                imap=decap_tun_map,
                emap=encap_tun_map)
            tunnel_term_id = sai_thrift_create_tunnel_term(
                self.client, term_type, uvrf, remote_vtep_ip, my_vtep_ip, tunnel_id,
                tunnel_type)

            # static route to tunnel
            tunnel_nhop = sai_thrift_create_nhop_tunnel(
                self.client, tunnel_id, remote_vtep_ip, '00:55:55:55:55:55',
                SAI_IP_ADDR_FAMILY_IPV4)
            sai_thrift_create_route(self.client, ovrf, SAI_IP_ADDR_FAMILY_IPV4,
                                    '10.10.10.0', '255.255.255.0', tunnel_nhop)

            # static route to underlay rif
            rif2_nhop = sai_thrift_create_nhop(
                self.client, SAI_IP_ADDR_FAMILY_IPV4, '2.2.2.2', rif2)
            sai_thrift_create_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4,
                                       rif2, '2.2.2.2', '00:44:44:44:44:44')

            # static route to overlay rif
            rif1_nhop = sai_thrift_create_nhop(
                self.client, SAI_IP_ADDR_FAMILY_IPV4, '11.11.11.1', rif1)
            sai_thrift_create_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4,
                                       rif1, '11.11.11.1', '00:22:22:22:22:22')

            print "Verifying 4in4 (L3 ip in vxlan encap)"
            pkt = simple_tcp_packet(
                eth_dst=router_mac,
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='11.11.11.1',
                ip_id=105,
                ip_ttl=64)
            pkt2 = simple_tcp_packet(
                eth_dst='00:55:55:55:55:55',
                eth_src=router_mac,
                ip_dst='10.10.10.1',
                ip_src='11.11.11.1',
                ip_id=105,
                ip_ttl=63)
            udp_sport = entropy_hash(pkt)
            vxlan_pkt = simple_vxlan_packet(
                eth_src=router_mac,
                eth_dst='00:44:44:44:44:44',
                ip_id=0,
                ip_src='2.2.2.3',
                ip_dst='2.2.2.2',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=vni,
                inner_frame=pkt2)
            send_packet(self, 0, str(pkt))
            verify_packet(self, vxlan_pkt, 1)

            print "Verifying 4in4 (L3 ip in vxlan decap)"
            pkt1 = simple_tcp_packet(
                eth_dst=router_mac,
                eth_src='00:55:55:55:55:55',
                ip_dst='11.11.11.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            vxlan_pkt = simple_vxlan_packet(
                eth_dst=router_mac,
                eth_src='00:44:44:44:44:44',
                ip_id=0,
                ip_src='2.2.2.2',
                ip_dst='2.2.2.3',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=11638,
                vxlan_vni=vni,
                with_udp_chksum=False,
                inner_frame=pkt1)
            pkt2 = simple_tcp_packet(
                eth_src=router_mac,
                eth_dst='00:22:22:22:22:22',
                ip_dst='11.11.11.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63)
            send_packet(self, 1, str(vxlan_pkt))
            verify_packet(self, pkt2, 0)

        finally:
            #cleanup
            sai_thrift_remove_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4,
                                       rif1, '11.11.11.1', '00:22:22:22:22:22')
            sai_thrift_remove_nhop(self.client, [rif1_nhop])
            sai_thrift_remove_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4,
                                       rif2, '2.2.2.2', '00:44:44:44:44:44')
            sai_thrift_remove_nhop(self.client, [rif2_nhop])
            sai_thrift_remove_route(self.client, ovrf, SAI_IP_ADDR_FAMILY_IPV4,
                                    '10.10.10.0', '255.255.255.0', tunnel_nhop)
            sai_thrift_remove_nhop(self.client, [tunnel_nhop])
            self.client.sai_thrift_remove_tunnel_term(tunnel_term_id)
            self.client.sai_thrift_remove_tunnel(tunnel_id)
            self.client.sai_thrift_remove_tunnel_map_entry(encap_tun_map_entry)
            self.client.sai_thrift_remove_tunnel_map_entry(decap_tun_map_entry)
            self.client.sai_thrift_remove_tunnel_map(encap_tun_map)
            self.client.sai_thrift_remove_tunnel_map(decap_tun_map)
            self.client.sai_thrift_remove_router_interface(orif)
            self.client.sai_thrift_remove_router_interface(urif)
            self.client.sai_thrift_remove_router_interface(rif1)
            self.client.sai_thrift_remove_router_interface(rif2)
            sai_thrift_create_default_bridge_ports(self.client, [port1, port2])
            self.client.sai_thrift_remove_virtual_router(ovrf)
            self.client.sai_thrift_remove_virtual_router(uvrf)

@group('tunnel-ocp')
@group('tunnel')
class L3VxlanDmacGlobalEncapDecap(sai_base_test.ThriftInterfaceDataPlane):
    '''
    This performs ip-in-vxlan tunnel encap/decap
    '''

    def runTest(self):
        try:
            print
            switch_init(self.client)
            port1 = port_list[0]
            port2 = port_list[1]
            tunnel_type = SAI_TUNNEL_TYPE_VXLAN
            term_type = SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2P
            remote_vtep_ip = '2.2.2.2'
            my_vtep_ip = '2.2.2.3'
            i_map_type = SAI_TUNNEL_MAP_TYPE_VNI_TO_VIRTUAL_ROUTER_ID
            e_map_type = SAI_TUNNEL_MAP_TYPE_VIRTUAL_ROUTER_ID_TO_VNI
            vni = 10000

            attr_value = sai_thrift_attribute_value_t(mac=vxlan_router_mac)
            attr = sai_thrift_attribute_t(id=SAI_SWITCH_ATTR_VXLAN_DEFAULT_ROUTER_MAC, value=attr_value)
            self.client.sai_thrift_set_switch_attribute(attr)

            v4 = 1
            v6 = 1
            ovrf = sai_thrift_create_virtual_router(self.client, v4, v6)
            uvrf = sai_thrift_create_virtual_router(self.client, v4, v6)
            sai_thrift_remove_default_bridge_ports(self.client, [port1, port2])

            #overlay rif
            rif1 = sai_thrift_create_router_interface(self.client, ovrf, 1,
                                                      port1, 0, v4, v6, 0)
            #underlay rif
            rif2 = sai_thrift_create_router_interface(self.client, uvrf, 1,
                                                      port2, 0, v4, v6, 0)

            #tunnel creation
            orif = sai_thrift_create_loopback_rif(self.client, ovrf,
                                                  router_mac)
            urif = sai_thrift_create_loopback_rif(self.client, uvrf,
                                                  router_mac)

            i_tun_map = sai_thrift_create_tunnel_map(self.client, i_map_type)
            i_tun_map_entry = sai_thrift_create_tunnel_map_entry(
                self.client, i_map_type, i_tun_map, 0, 0, ovrf, vni)
            e_tun_map = sai_thrift_create_tunnel_map(self.client, e_map_type)
            e_tun_map_entry = sai_thrift_create_tunnel_map_entry(
                self.client, e_map_type, e_tun_map, 0, 0, ovrf, vni)

            tunnel_id = sai_thrift_create_tunnel(
                self.client,
                tunnel_type,
                my_vtep_ip,
                urif,
                orif,
                imap=i_tun_map,
                emap=e_tun_map)
            tunnel_term_id = sai_thrift_create_tunnel_term(
                self.client, term_type, uvrf, remote_vtep_ip, my_vtep_ip, tunnel_id,
                tunnel_type)

            # static route to tunnel
            tunnel_nhop = sai_thrift_create_nhop_tunnel(
                self.client, tunnel_id, remote_vtep_ip, '', SAI_IP_ADDR_FAMILY_IPV4, vni)
            sai_thrift_create_route(self.client, ovrf, SAI_IP_ADDR_FAMILY_IPV4,
                                    '10.10.10.0', '255.255.255.0', tunnel_nhop)

            # static route to underlay rif
            rif2_nhop = sai_thrift_create_nhop(
                self.client, SAI_IP_ADDR_FAMILY_IPV4, '2.2.2.2', rif2)
            sai_thrift_create_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4,
                                       rif2, '2.2.2.2', '00:44:44:44:44:44')

            # static route to overlay rif
            rif1_nhop = sai_thrift_create_nhop(
                self.client, SAI_IP_ADDR_FAMILY_IPV4, '11.11.11.1', rif1)
            sai_thrift_create_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4,
                                       rif1, '11.11.11.1', '00:22:22:22:22:22')

            print "Verifying 4in4 (L3 ip in vxlan encap)"
            pkt = simple_tcp_packet(
                eth_dst=router_mac,
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='11.11.11.1',
                ip_id=105,
                ip_ttl=64)
            pkt2 = simple_tcp_packet(
                eth_dst=vxlan_router_mac,
                eth_src=router_mac,
                ip_dst='10.10.10.1',
                ip_src='11.11.11.1',
                ip_id=105,
                ip_ttl=63)
            udp_sport = entropy_hash(pkt)
            vxlan_pkt = simple_vxlan_packet(
                eth_src=router_mac,
                eth_dst='00:44:44:44:44:44',
                ip_id=0,
                ip_src='2.2.2.3',
                ip_dst='2.2.2.2',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=vni,
                inner_frame=pkt2)
            send_packet(self, 0, str(pkt))
            verify_packet(self, vxlan_pkt, 1)

            print "Verifying 4in4 (L3 ip in vxlan decap)"
            pkt1 = simple_tcp_packet(
                eth_dst=router_mac,
                eth_src='00:55:55:55:55:55',
                ip_dst='11.11.11.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            vxlan_pkt = simple_vxlan_packet(
                eth_dst=router_mac,
                eth_src='00:44:44:44:44:44',
                ip_id=0,
                ip_src='2.2.2.2',
                ip_dst='2.2.2.3',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=11638,
                vxlan_vni=vni,
                with_udp_chksum=False,
                inner_frame=pkt1)
            pkt2 = simple_tcp_packet(
                eth_src=router_mac,
                eth_dst='00:22:22:22:22:22',
                ip_dst='11.11.11.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63)
            send_packet(self, 1, str(vxlan_pkt))
            verify_packet(self, pkt2, 0)

        finally:
            #cleanup
            sai_thrift_remove_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4,
                                       rif1, '11.11.11.1', '00:22:22:22:22:22')
            sai_thrift_remove_nhop(self.client, [rif1_nhop])
            sai_thrift_remove_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4,
                                       rif2, '2.2.2.2', '00:44:44:44:44:44')
            sai_thrift_remove_nhop(self.client, [rif2_nhop])
            sai_thrift_remove_route(self.client, ovrf, SAI_IP_ADDR_FAMILY_IPV4,
                                    '10.10.10.0', '255.255.255.0', tunnel_nhop)
            sai_thrift_remove_nhop(self.client, [tunnel_nhop])
            self.client.sai_thrift_remove_tunnel_term(tunnel_term_id)
            self.client.sai_thrift_remove_tunnel(tunnel_id)
            self.client.sai_thrift_remove_tunnel_map_entry(e_tun_map_entry)
            self.client.sai_thrift_remove_tunnel_map_entry(i_tun_map_entry)
            self.client.sai_thrift_remove_tunnel_map(e_tun_map)
            self.client.sai_thrift_remove_tunnel_map(i_tun_map)
            self.client.sai_thrift_remove_router_interface(orif)
            self.client.sai_thrift_remove_router_interface(urif)
            self.client.sai_thrift_remove_router_interface(rif1)
            self.client.sai_thrift_remove_router_interface(rif2)
            sai_thrift_create_default_bridge_ports(self.client, [port1, port2])
            self.client.sai_thrift_remove_virtual_router(ovrf)
            self.client.sai_thrift_remove_virtual_router(uvrf)

@group('tunnel-ocp')
@group('tunnel')
class TunnelVniEncapDecap(sai_base_test.ThriftInterfaceDataPlane):
    '''
    This performs ip-in-vxlan tunnel encap/decap with no mapper
    '''

    def runTest(self):
        try:
            print
            switch_init(self.client)
            port1 = port_list[0]
            port2 = port_list[1]
            tunnel_type = SAI_TUNNEL_TYPE_VXLAN
            term_type = SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2P
            remote_vtep_ip = '2.2.2.2'
            my_vtep_ip = '2.2.2.3'
            decap_map_type = SAI_TUNNEL_MAP_TYPE_VNI_TO_VIRTUAL_ROUTER_ID
            vni = 10000

            v4 = 1
            v6 = 1
            ovrf = sai_thrift_create_virtual_router(self.client, v4, v6)
            uvrf = sai_thrift_create_virtual_router(self.client, v4, v6)
            sai_thrift_remove_default_bridge_ports(self.client, [port1, port2])

            #overlay rif
            rif1 = sai_thrift_create_router_interface(self.client, ovrf, 1,
                                                      port1, 0, v4, v6, 0)
            #underlay rif
            rif2 = sai_thrift_create_router_interface(self.client, uvrf, 1,
                                                      port2, 0, v4, v6, 0)

            #tunnel creation
            orif = sai_thrift_create_loopback_rif(self.client, ovrf,
                                                  router_mac)
            urif = sai_thrift_create_loopback_rif(self.client, uvrf,
                                                  router_mac)

            decap_tun_map = sai_thrift_create_tunnel_map(self.client, decap_map_type)
            decap_tun_map_entry = sai_thrift_create_tunnel_map_entry(
                self.client, decap_map_type, decap_tun_map, 0, 0, ovrf, vni)

            tunnel_id = sai_thrift_create_tunnel(
                self.client, tunnel_type, my_vtep_ip, urif, orif, imap=decap_tun_map)
            tunnel_term_id = sai_thrift_create_tunnel_term(
                self.client, term_type, uvrf, remote_vtep_ip, my_vtep_ip, tunnel_id,
                tunnel_type)

            # static route to tunnel
            tunnel_nhop = sai_thrift_create_nhop_tunnel(
                self.client, tunnel_id, remote_vtep_ip, '00:55:55:55:55:55',
                SAI_IP_ADDR_FAMILY_IPV4, vni)
            sai_thrift_create_route(self.client, ovrf, SAI_IP_ADDR_FAMILY_IPV4,
                                    '10.10.10.0', '255.255.255.0', tunnel_nhop)

            # static route to underlay rif
            rif2_nhop = sai_thrift_create_nhop(
                self.client, SAI_IP_ADDR_FAMILY_IPV4, '2.2.2.2', rif2)
            sai_thrift_create_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4,
                                       rif2, '2.2.2.2', '00:44:44:44:44:44')

            # static route to overlay rif
            rif1_nhop = sai_thrift_create_nhop(
                self.client, SAI_IP_ADDR_FAMILY_IPV4, '11.11.11.1', rif1)
            sai_thrift_create_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4,
                                       rif1, '11.11.11.1', '00:22:22:22:22:22')

            print "Verifying 4in4 (ip in vxlan encap no mapper)"
            pkt = simple_tcp_packet(
                eth_dst=router_mac,
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.1',
                ip_src='11.11.11.1',
                ip_id=105,
                ip_ttl=64)
            pkt2 = simple_tcp_packet(
                eth_dst='00:55:55:55:55:55',
                eth_src=router_mac,
                ip_dst='10.10.10.1',
                ip_src='11.11.11.1',
                ip_id=105,
                ip_ttl=63)
            udp_sport = entropy_hash(pkt)
            vxlan_pkt = simple_vxlan_packet(
                eth_src=router_mac,
                eth_dst='00:44:44:44:44:44',
                ip_id=0,
                ip_src='2.2.2.3',
                ip_dst='2.2.2.2',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=vni,
                inner_frame=pkt2)
            send_packet(self, 0, str(pkt))
            verify_packet(self, vxlan_pkt, 1)

            print "Verifying 4in4 (ip in vxlan decap)"
            pkt1 = simple_tcp_packet(
                eth_dst=router_mac,
                eth_src='00:55:55:55:55:55',
                ip_dst='11.11.11.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=64)
            vxlan_pkt = simple_vxlan_packet(
                eth_dst=router_mac,
                eth_src='00:44:44:44:44:44',
                ip_id=0,
                ip_src='2.2.2.2',
                ip_dst='2.2.2.3',
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=11638,
                vxlan_vni=vni,
                with_udp_chksum=False,
                inner_frame=pkt1)
            pkt2 = simple_tcp_packet(
                eth_src=router_mac,
                eth_dst='00:22:22:22:22:22',
                ip_dst='11.11.11.1',
                ip_src='10.10.10.1',
                ip_id=108,
                ip_ttl=63)
            send_packet(self, 1, str(vxlan_pkt))
            verify_packet(self, pkt2, 0)

        finally:
            #cleanup
            sai_thrift_remove_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4,
                                       rif1, '11.11.11.1', '00:22:22:22:22:22')
            sai_thrift_remove_nhop(self.client, [rif1_nhop])
            sai_thrift_remove_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4,
                                       rif2, '2.2.2.2', '00:44:44:44:44:44')
            sai_thrift_remove_nhop(self.client, [rif2_nhop])
            sai_thrift_remove_route(self.client, ovrf, SAI_IP_ADDR_FAMILY_IPV4,
                                    '10.10.10.0', '255.255.255.0', tunnel_nhop)
            sai_thrift_remove_nhop(self.client, [tunnel_nhop])
            self.client.sai_thrift_remove_tunnel_term(tunnel_term_id)
            self.client.sai_thrift_remove_tunnel(tunnel_id)
            self.client.sai_thrift_remove_tunnel_map_entry(decap_tun_map_entry)
            self.client.sai_thrift_remove_tunnel_map(decap_tun_map)
            self.client.sai_thrift_remove_router_interface(orif)
            self.client.sai_thrift_remove_router_interface(urif)
            self.client.sai_thrift_remove_router_interface(rif1)
            self.client.sai_thrift_remove_router_interface(rif2)
            sai_thrift_create_default_bridge_ports(self.client, [port1, port2])
            self.client.sai_thrift_remove_virtual_router(ovrf)
            self.client.sai_thrift_remove_virtual_router(uvrf)

 

@group('tunnel-ocp')
@group('tunnel')
class L3VxlanP2MPTunnel(sai_base_test.ThriftInterfaceDataPlane):
    # Routing in-and-out of Vxlan tunnels
    def setUp(self):
        super(self.__class__, self).setUp()
        print
        switch_init(self.client)
        self.C1_port = port_list[1]
        self.C2_port = port_list[2]
        self.C3_port = port_list[3]
        self.C4_port = port_list[4]
        self.port5 = port_list[5]
        
        # One endpoint in each Customer Subnet
        self.C_ip = [0]*5
        self.C_ip[1] = '100.100.3.1'
        self.C_ip[2] = '100.100.4.1'
        self.C_ip[3] = '100.101.1.1'
        self.C_ip[4] = '100.101.1.1'
        
        self.C_mac = [0]*5
        self.C_mac[1] = '00:00:00:00:00:01'
        self.C_mac[2] = '00:00:00:00:00:02'
        self.C_mac[3] = '00:00:00:00:00:03'
        self.C_mac[4] = '00:00:00:00:00:04'

        # VMs and their hosts
        self.VM_ip = [0]*4
        self.VM_ip[1] = '100.100.1.1'
        self.VM_ip[2] = '100.100.2.1'
        self.VM_ip[3] = '100.101.2.1'

        self.host_ip = [0]*4
        self.host_ip[1] = '10.10.10.1'
        self.host_ip[2] = '10.10.10.2'
        self.host_ip[3] = '10.10.10.3'

        # Inner Destination MACs
        self.vxlan_default_router_mac='00:11:11:11:11:11'
        
        self.host_inner_dmac = [0]*4
        self.host_inner_dmac[1] = self.vxlan_default_router_mac
        self.host_inner_dmac[2] = '00:12:34:56:78:9a'
        self.host_inner_dmac[3] = self.vxlan_default_router_mac
        
        
        # Create Default VRF ( also used as underlay vrf )
        v4_enabled = 1
        self.uvrf = sai_thrift_create_virtual_router(self.client, v4_enabled, 0)
        sai_thrift_remove_default_bridge_ports(self.client, [self.C1_port, self.C2_port, self.C3_port, self.C4_port, self.port5])
        
        # Create Underlay loopback RIF ( required for tunnel object creation )
        self.urif_lb = sai_thrift_create_loopback_rif(self.client, self.uvrf, router_mac)
        
        #
        # Create Overlay VRFs
        #
        self.ovrf = [0]*4
        self.ovrf[1] = sai_thrift_create_virtual_router(self.client, v4_enabled, 0)
        self.ovrf[2] = sai_thrift_create_virtual_router(self.client, v4_enabled, 0)
        self.ovrf[3] = sai_thrift_create_virtual_router(self.client, v4_enabled, 0)
        
        self.ovrf_vni = [0]*4
        self.ovrf_vni[1] = 2000
        self.ovrf_vni[2] = 2001
        self.ovrf_vni[3] = 2005

        #
        # Setup underlay default route, the nexthop is 1.1.1.1
        #
        self.underlay_neighbor_mac = '00:55:55:55:55:55'
        self.underlay_nhop_ip = '1.1.1.1'
        self.underlay_route_addr = '0.0.0.0'
        self.underlay_route_mask = '0.0.0.0'
        self.underlay_rif = sai_thrift_create_router_interface(self.client, self.uvrf, 1, self.port5, 0, 1, 0 , 0)
        self.underlay_nhop = sai_thrift_create_nhop(self.client, SAI_IP_ADDR_FAMILY_IPV4, self.underlay_nhop_ip, self.underlay_rif)
        self.underlay_neighbor = sai_thrift_create_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4,
                                                       self.underlay_rif, self.underlay_nhop_ip, self.underlay_neighbor_mac)
        self.underlay_default_route = sai_thrift_create_route(self.client, self.uvrf, SAI_IP_ADDR_FAMILY_IPV4,
                                                         self.underlay_route_addr, self.underlay_route_mask, self.underlay_nhop)
        
        #
        # Setup overlay routes
        #
        
        # create port-based router interface for C1 
        self.rif1 = sai_thrift_create_router_interface(self.client, self.ovrf[1], 1, self.C1_port, 0, 1, 1, '')
        sai_thrift_create_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4, self.rif1, self.C_ip[1], self.C_mac[1])
        
        # create port-based router interface for C2 
        self.rif2 = sai_thrift_create_router_interface(self.client, self.ovrf[1], 1, self.C2_port, 0, 1, 1, '')
        sai_thrift_create_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4, self.rif2, self.C_ip[2], self.C_mac[2])
        
        # create port-based router interface for C3
        self.rif3 = sai_thrift_create_router_interface(self.client, self.ovrf[2], 1, self.C3_port, 0, 1, 1, '')
        sai_thrift_create_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4, self.rif3, self.C_ip[3], self.C_mac[3])
        
        # create port-based router interface for C4 
        self.rif4 = sai_thrift_create_router_interface(self.client, self.ovrf[3], 1, self.C4_port, 0, 1, 1, '')
        sai_thrift_create_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4, self.rif4, self.C_ip[4], self.C_mac[4])
        
        
        #
        # Create Tunnel
        #
        
        # Create Encap/decap mappers
        self.encap_tunnel_map = sai_thrift_create_tunnel_map(self.client, SAI_TUNNEL_MAP_TYPE_VIRTUAL_ROUTER_ID_TO_VNI)
        self.decap_tunnel_map = sai_thrift_create_tunnel_map(self.client, SAI_TUNNEL_MAP_TYPE_VNI_TO_VIRTUAL_ROUTER_ID)
        
        # Create Tunnel object
        self.my_lb_ip_addr = '10.10.10.10'
        self.my_lb_ip_mask = '255.255.255.255'
        self.tunnel_id = sai_thrift_create_tunnel(self.client, SAI_TUNNEL_TYPE_VXLAN, self.my_lb_ip_addr, self.urif_lb, 0, self.encap_tunnel_map, self.decap_tunnel_map)
        
        # Create Tunnel Map entries for C1, C2
        self.C1_encap_tunnel_map_entry = sai_thrift_create_tunnel_map_entry(self.client, SAI_TUNNEL_MAP_TYPE_VIRTUAL_ROUTER_ID_TO_VNI, self.encap_tunnel_map, 0, 0, self.ovrf[1], self.ovrf_vni[1])
        self.C1_decap_tunnel_map_entry = sai_thrift_create_tunnel_map_entry(self.client, SAI_TUNNEL_MAP_TYPE_VNI_TO_VIRTUAL_ROUTER_ID, self.decap_tunnel_map, 0, 0, self.ovrf[1], self.ovrf_vni[1])
        
        # Create Tunnel Map entries for C3
        self.C3_encap_tunnel_map_entry = sai_thrift_create_tunnel_map_entry(self.client, SAI_TUNNEL_MAP_TYPE_VIRTUAL_ROUTER_ID_TO_VNI, self.encap_tunnel_map, 0, 0, self.ovrf[2], self.ovrf_vni[2])
        self.C3_decap_tunnel_map_entry = sai_thrift_create_tunnel_map_entry(self.client, SAI_TUNNEL_MAP_TYPE_VNI_TO_VIRTUAL_ROUTER_ID, self.decap_tunnel_map, 0, 0, self.ovrf[2], self.ovrf_vni[2])
        
        # Create Tunnel Map entries for C4
        self.C4_encap_tunnel_map_entry = sai_thrift_create_tunnel_map_entry(self.client, SAI_TUNNEL_MAP_TYPE_VIRTUAL_ROUTER_ID_TO_VNI, self.encap_tunnel_map, 0, 0, self.ovrf[3], self.ovrf_vni[3])
        self.C4_decap_tunnel_map_entry = sai_thrift_create_tunnel_map_entry(self.client, SAI_TUNNEL_MAP_TYPE_VNI_TO_VIRTUAL_ROUTER_ID, self.decap_tunnel_map, 0, 0, self.ovrf[3], self.ovrf_vni[3])
        
        # Create tunnel decap for VM to customer server 
        self.tunnel_term_id = sai_thrift_create_tunnel_term(self.client, SAI_TUNNEL_TERM_TABLE_ENTRY_TYPE_P2MP, self.uvrf,
                                                       self.my_lb_ip_mask, self.my_lb_ip_addr, self.tunnel_id, SAI_TUNNEL_TYPE_VXLAN)
        
        
        
        # create tunnel nexthop for VM1, VM2 and VM3 */
        self.tunnel_nexthop_id1 = sai_thrift_create_nhop_tunnel(self.client, self.tunnel_id, self.host_ip[1], self.host_inner_dmac[1], SAI_IP_ADDR_FAMILY_IPV4)
#        self.tunnel_nexthop_id1 = sai_thrift_create_nhop_tunnel(self.client, tunnel_id, host_ip[1], SAI_IP_ADDR_FAMILY_IPV4, 0,    "")
        self.tunnel_nexthop_id2 = sai_thrift_create_nhop_tunnel(self.client, self.tunnel_id, self.host_ip[2], self.host_inner_dmac[2], SAI_IP_ADDR_FAMILY_IPV4, self.ovrf_vni[2])
#        self.tunnel_nexthop_id3 = sai_thrift_create_nhop_tunnel(self.client, tunnel_id, host_ip[3], SAI_IP_ADDR_FAMILY_IPV4, 0,    "")
        self.tunnel_nexthop_id3 = sai_thrift_create_nhop_tunnel(self.client, self.tunnel_id, self.host_ip[3], self.host_inner_dmac[3], SAI_IP_ADDR_FAMILY_IPV4)
        
        
        # Create routes for vrid 1 ingress */
        VM1_vnet1_route = sai_thrift_create_route(self.client, self.ovrf[1], SAI_IP_ADDR_FAMILY_IPV4, '100.100.1.1', '255.255.255.255', self.tunnel_nexthop_id1)
        VM2_vnet1_route = sai_thrift_create_route(self.client, self.ovrf[1], SAI_IP_ADDR_FAMILY_IPV4, '100.100.2.1', '255.255.255.255', self.tunnel_nexthop_id2)
        C1_vnet1_route  = sai_thrift_create_route(self.client, self.ovrf[1], SAI_IP_ADDR_FAMILY_IPV4, '100.100.3.0', '255.255.255.0', self.rif1)
        C2_vnet1_route  = sai_thrift_create_route(self.client, self.ovrf[1], SAI_IP_ADDR_FAMILY_IPV4, '100.100.4.0', '255.255.255.0', self.rif2)
        C3_vnet1_route  = sai_thrift_create_route(self.client, self.ovrf[1], SAI_IP_ADDR_FAMILY_IPV4, '100.102.1.0', '255.255.255.0', self.rif3)
        
        # Create routes for vrid 1 egress
        
        # create routes for vrid 2 ingress 
        VM1_vnet2_route  = sai_thrift_create_route(self.client, self.ovrf[2], SAI_IP_ADDR_FAMILY_IPV4, '100.100.1.1', '255.255.255.255', self.tunnel_nexthop_id1)
        VM2_vnet2_route  = sai_thrift_create_route(self.client, self.ovrf[2], SAI_IP_ADDR_FAMILY_IPV4, '100.100.2.1', '255.255.255.255', self.tunnel_nexthop_id2)
        C1_vnet2_route   = sai_thrift_create_route(self.client, self.ovrf[2], SAI_IP_ADDR_FAMILY_IPV4, '100.100.3.0', '255.255.255.0', self.rif1)
        C2_vnet2_route   = sai_thrift_create_route(self.client, self.ovrf[2], SAI_IP_ADDR_FAMILY_IPV4, '100.100.4.0', '255.255.255.0', self.rif2)
        C3_vnet2_route   = sai_thrift_create_route(self.client, self.ovrf[2], SAI_IP_ADDR_FAMILY_IPV4, '100.102.1.0', '255.255.255.0', self.rif3)
        
        # Create routes fro vrid 3 ingress 
        VM3_vnet3_route = sai_thrift_create_route(self.client, self.ovrf[3], SAI_IP_ADDR_FAMILY_IPV4, '100.101.2.1', '255.255.255.255', self.tunnel_nexthop_id3)
        C4_vnet3_route  = sai_thrift_create_route(self.client, self.ovrf[3], SAI_IP_ADDR_FAMILY_IPV4, '100.101.1.0', '255.255.255.0', self.rif4)
        
        
    def runTest(self):
        try:
            print "Tunnel Decap:"
            # VM1/2 -> C1/C2/C3
            for c_id in range(1,4):
                for vm_id in range(1,3):
                    if c_id==3:
                        pkt_vni = self.ovrf_vni[2]
                    else:
                        pkt_vni = self.ovrf_vni[1]
                    print "sending packet from VM%d to C%d" % (vm_id, c_id)
                    pkt1 = simple_tcp_packet(
                        eth_dst=router_mac,
                        eth_src=self.vxlan_default_router_mac,
                        ip_dst=self.C_ip[c_id],
                        ip_src=self.VM_ip[vm_id],
                        ip_id=108,
                        ip_ttl=64)
                    vxlan_pkt = simple_vxlan_packet(
                        eth_dst=router_mac,
                        eth_src=self.underlay_neighbor_mac,
                        ip_id=0,
                        ip_src=self.host_ip[vm_id],
                        ip_dst=self.my_lb_ip_addr,
                        ip_ttl=64,
                        ip_flags=0x2,
                        udp_sport=11638,
                        vxlan_vni=pkt_vni,
                        with_udp_chksum=False,
                        inner_frame=pkt1)
                    pkt2 = simple_tcp_packet(
                        eth_src=router_mac,
                        eth_dst=self.C_mac[c_id],
                        ip_dst=self.C_ip[c_id],
                        ip_src=self.VM_ip[vm_id],
                        ip_id=108,
                        ip_ttl=63)
                    send_packet(self, 5, str(vxlan_pkt))
                    verify_packet(self, pkt2, c_id)
                    
            # VM3 -> C4
            vm_id = 3
            c_id = 4
            pkt_vni = self.ovrf_vni[3]
            print "sending packet from VM%d to C%d" % (vm_id, c_id)
            pkt1 = simple_tcp_packet(
                eth_dst=router_mac,
                eth_src=self.vxlan_default_router_mac,
                ip_dst=self.C_ip[c_id],
                ip_src=self.VM_ip[vm_id],
                ip_id=108,
                ip_ttl=64)
            vxlan_pkt = simple_vxlan_packet(
                eth_dst=router_mac,
                eth_src=self.underlay_neighbor_mac,
                ip_id=0,
                ip_src=self.host_ip[vm_id],
                ip_dst=self.my_lb_ip_addr,
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=11638,
                vxlan_vni=pkt_vni,
                with_udp_chksum=False,
                inner_frame=pkt1)
            pkt2 = simple_tcp_packet(
                eth_src=router_mac,
                eth_dst=self.C_mac[c_id],
                ip_dst=self.C_ip[c_id],
                ip_src=self.VM_ip[vm_id],
                ip_id=108,
                ip_ttl=63)
            send_packet(self, 5, str(vxlan_pkt))
            verify_packet(self, pkt2, c_id)
        
            print "Tunnel Encap:"
            # C1/C2/C3 -> VM1/2
            for c_id in range(1,4):
                for vm_id in range(1,3):
                    if vm_id==2:
                        pkt_vni = self.ovrf_vni[2]
                    elif c_id==3:
                        pkt_vni = self.ovrf_vni[2]
                    else:
                        pkt_vni = self.ovrf_vni[1]
                    print "sending packet from C%d to VM%d" % (c_id, vm_id)
                    pkt = simple_tcp_packet(
                              eth_dst=router_mac,
                              eth_src=self.C_mac[c_id],
                              ip_dst=self.VM_ip[vm_id],
                              ip_src=self.C_ip[c_id],
                              ip_id=105,
                              ip_ttl=64)
                    pkt2 = simple_tcp_packet(
                        eth_dst=self.host_inner_dmac[vm_id],
                        eth_src=router_mac,
                        ip_dst=self.VM_ip[vm_id],
                        ip_src=self.C_ip[c_id],
                        ip_id=105,
                        ip_ttl=63)
                    udp_sport = entropy_hash(pkt)
                    vxlan_pkt = simple_vxlan_packet(
                        eth_src=router_mac,
                        eth_dst=self.underlay_neighbor_mac,
                        ip_id=0,
                        ip_src=self.my_lb_ip_addr,
                        ip_dst=self.host_ip[vm_id],
                        ip_ttl=64,
                        ip_flags=0x2,
                        udp_sport=udp_sport,
                        with_udp_chksum=False,
                        vxlan_vni=pkt_vni,
                        inner_frame=pkt2)
                    send_packet(self, c_id, str(pkt))
                    verify_packet(self, vxlan_pkt, 5)
                    
            # C1/C2/C3 -> VM1/2
            vm_id = 3
            c_id = 4
            pkt_vni = self.ovrf_vni[3]
            print "sending packet from C%d to VM%d" % (c_id, vm_id)
            pkt = simple_tcp_packet(
                      eth_dst=router_mac,
                      eth_src=self.C_mac[c_id],
                      ip_dst=self.VM_ip[vm_id],
                      ip_src=self.C_ip[c_id],
                      ip_id=105,
                      ip_ttl=64)
            pkt2 = simple_tcp_packet(
                eth_dst=self.host_inner_dmac[vm_id],
                eth_src=router_mac,
                ip_dst=self.VM_ip[vm_id],
                ip_src=self.C_ip[c_id],
                ip_id=105,
                ip_ttl=63)
            udp_sport = entropy_hash(pkt)
            vxlan_pkt = simple_vxlan_packet(
                eth_src=router_mac,
                eth_dst=self.underlay_neighbor_mac,
                ip_id=0,
                ip_src=self.my_lb_ip_addr,
                ip_dst=self.host_ip[vm_id],
                ip_ttl=64,
                ip_flags=0x2,
                udp_sport=udp_sport,
                with_udp_chksum=False,
                vxlan_vni=pkt_vni,
                inner_frame=pkt2)
            send_packet(self, c_id, str(pkt))
            verify_packet(self, vxlan_pkt, 5)
                    
        finally:
            print

    def tearDown(self):
        #cleanup
        sai_thrift_remove_route(self.client, self.ovrf[1], SAI_IP_ADDR_FAMILY_IPV4, '100.100.1.1', '255.255.255.255', self.tunnel_nexthop_id1)
        sai_thrift_remove_route(self.client, self.ovrf[1], SAI_IP_ADDR_FAMILY_IPV4, '100.100.2.1', '255.255.255.255', self.tunnel_nexthop_id2)
        sai_thrift_remove_route(self.client, self.ovrf[1], SAI_IP_ADDR_FAMILY_IPV4, '100.100.3.0', '255.255.255.0',   self.rif1)
        sai_thrift_remove_route(self.client, self.ovrf[1], SAI_IP_ADDR_FAMILY_IPV4, '100.100.4.0', '255.255.255.0',   self.rif2)
        sai_thrift_remove_route(self.client, self.ovrf[1], SAI_IP_ADDR_FAMILY_IPV4, '100.102.1.0', '255.255.255.0',   self.rif3)
        sai_thrift_remove_route(self.client, self.ovrf[2], SAI_IP_ADDR_FAMILY_IPV4, '100.100.1.1', '255.255.255.255', self.tunnel_nexthop_id1)
        sai_thrift_remove_route(self.client, self.ovrf[2], SAI_IP_ADDR_FAMILY_IPV4, '100.100.2.1', '255.255.255.255', self.tunnel_nexthop_id2)
        sai_thrift_remove_route(self.client, self.ovrf[2], SAI_IP_ADDR_FAMILY_IPV4, '100.100.3.0', '255.255.255.0',   self.rif1)
        sai_thrift_remove_route(self.client, self.ovrf[2], SAI_IP_ADDR_FAMILY_IPV4, '100.100.4.0', '255.255.255.0',   self.rif2)
        sai_thrift_remove_route(self.client, self.ovrf[2], SAI_IP_ADDR_FAMILY_IPV4, '100.102.1.0', '255.255.255.0',   self.rif3)
        sai_thrift_remove_route(self.client, self.ovrf[3], SAI_IP_ADDR_FAMILY_IPV4, '100.101.2.1', '255.255.255.255', self.tunnel_nexthop_id1)
        sai_thrift_remove_route(self.client, self.ovrf[3], SAI_IP_ADDR_FAMILY_IPV4, '100.101.1.0', '255.255.255.0',   self.rif4)
        
        sai_thrift_remove_nhop(self.client, [self.tunnel_nexthop_id1, self.tunnel_nexthop_id2, self.tunnel_nexthop_id3])
        
        self.client.sai_thrift_remove_tunnel_term(self.tunnel_term_id)
        self.client.sai_thrift_remove_tunnel_map_entry(self.C1_encap_tunnel_map_entry)
        self.client.sai_thrift_remove_tunnel_map_entry(self.C3_encap_tunnel_map_entry)
        self.client.sai_thrift_remove_tunnel_map_entry(self.C4_encap_tunnel_map_entry)
        self.client.sai_thrift_remove_tunnel_map_entry(self.C1_decap_tunnel_map_entry)
        self.client.sai_thrift_remove_tunnel_map_entry(self.C3_decap_tunnel_map_entry)
        self.client.sai_thrift_remove_tunnel_map_entry(self.C4_decap_tunnel_map_entry)
        self.client.sai_thrift_remove_tunnel_map(self.encap_tunnel_map)
        self.client.sai_thrift_remove_tunnel_map(self.decap_tunnel_map)
        self.client.sai_thrift_remove_tunnel(self.tunnel_id)
        
        sai_thrift_remove_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4, self.rif1, "100.100.3.1", '00:00:00:00:00:01')
        sai_thrift_remove_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4, self.rif2, "100.100.4.1", "00:00:00:00:00:02")
        sai_thrift_remove_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4, self.rif3, "100.102.1.1", "00:00:00:00:00:03")
        sai_thrift_remove_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4, self.rif4, "100.101.1.1", "00:00:00:00:00:04")
        self.client.sai_thrift_remove_router_interface(self.rif1)
        self.client.sai_thrift_remove_router_interface(self.rif2)
        self.client.sai_thrift_remove_router_interface(self.rif3)
        self.client.sai_thrift_remove_router_interface(self.rif4)
        
        sai_thrift_remove_route(self.client, self.uvrf, SAI_IP_ADDR_FAMILY_IPV4, '0.0.0.0', '0.0.0.0', self.rif4)
        sai_thrift_remove_neighbor(self.client, SAI_IP_ADDR_FAMILY_IPV4, self.underlay_rif, "1.1.1.1", '00:55:55:55:55:55')
        sai_thrift_remove_nhop(self.client, [self.underlay_nhop])
        self.client.sai_thrift_remove_router_interface(self.underlay_rif)
        self.client.sai_thrift_remove_router_interface(self.urif_lb)
        self.client.sai_thrift_remove_virtual_router(self.ovrf[1])
        self.client.sai_thrift_remove_virtual_router(self.ovrf[2])
        self.client.sai_thrift_remove_virtual_router(self.ovrf[3])
        self.client.sai_thrift_remove_virtual_router(self.uvrf)
        sai_thrift_create_default_bridge_ports(self.client, [self.C1_port, self.C2_port, self.C3_port, self.C4_port, self.port5])
        
