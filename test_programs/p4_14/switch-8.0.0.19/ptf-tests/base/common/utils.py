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

from ptf.testutils import *
import ptf.mask
from scapy.all import Packet
from scapy.fields import *
from scapy.all import Ether
from scapy.all import bind_layers

import scapy.layers.l2
import scapy.layers.inet
from port_mapping import *


###############################################################################
# Helper functions                                                            #
###############################################################################
def verify_any_packet_on_ports_list(test, pkts=[], ports=[], device_number=0, timeout=2):
    """
    Ports is list of port lists
    Check that _any_ packet is received atleast once in every sublist in
    ports belonging to the given device (default device_number is 0).

    Also verifies that the packet is ot received on any other ports for this
    device, and that no other packets are received on the device
    (unless --relax is in effect).
    """
    pkt_cnt = 0
    for port_list in ports:
        for port in port_list:
            (rcv_device, rcv_port, rcv_pkt, pkt_time) = test.dataplane.poll(
                port_number=port, timeout=timeout, filters=get_filters())
            if rcv_device != device_number:
                continue
            for pkt in pkts:
                logging.debug("Checking for pkt on device %d, port %d",
                              device_number, port)
                if str(rcv_pkt) == str(pkt):
                    pkt_cnt += 1

    verify_no_other_packets(test)
    test.assertTrue(pkt_cnt == len(ports),
                    "Did not receive pkt on one of ports %r for device %d" %
                    (ports, device_number))


def verify_any_packet_on_port(test, pkts=[], port_id=0, device_number=0):
    """
    Check that a packet received on the specified port is one of _any_ packets
    Useful to check packets arriving in non-deterministic order on a port
    Does not check any other ports (relaxed check)
    """
    device, port = port_to_tuple(port_id)
    logging.debug("Checking for pkt on device %d, port %d", device, port)
    (rcv_device, rcv_port, rcv_pkt, pkt_time) = dp_poll(
        test, device_number=device, port_number=port, timeout=2)
    test.assertTrue(rcv_pkt != None,
                    "Did not receive expected pkt on device %d, port %r" %
                    (device, port))
    for pkt in pkts:
        if str(pkt) == str(rcv_pkt):
            return True
    return False


def verify_multiple_packets_on_ports(test, plist=[], device=0, timeout=3):
    for port, pkts in plist:
        for n in range(0, len(pkts)):
            (rcv_device, rcv_port, rcv_pkt, pkt_time) = test.dataplane.poll(
                port_number=port, timeout=timeout, filters=get_filters())
            if rcv_port is None:
                test.assertTrue(False,
                                'Failed to receive packet(s) on %d' % port)
            for pkt in pkts[:]:
                if str(rcv_pkt) == str(pkt):
                    pkts.remove(pkt)
                    logging.debug("received expected packet on port %d", rcv_port)
                else:
                    logging.debug("packet mismatch on port %d", rcv_port)
        test.assertTrue(
            len(pkts) == 0, "Not all packets for port %d were received" % port)
    verify_no_other_packets(test)


def get_swtype(client):
    board_type = client.pltfm_pm.pltfm_pm_board_type_get()
    swtype = ""
    if re.search("0x0234|0x1234|0x4234|0x5234", hex(board_type)):
        swtype = "mavericks"
    elif re.search("0x2234|0x3234", hex(board_type)):
        swtype = "montara"
    return swtype

def devport_to_swport(client, devport):
    if (test_param_get('arch') == 'Tofino') and \
       (test_param_get('target') == 'hw'):
        swport = devport
        try:
            # if built with platform then get the swtype
            swtype = get_swtype(client)
            fpport = getFrontPanelPort(swtype, devport)
            pgrp, chnl = fpport.split("/")
            swport = (int(pgrp) * 4 - 4) + int(chnl)
        except:
            # not built for platform return the devport
            return devport
        return swport
    else:
        return devport

def swport_to_devport(client, swport):
    if (test_param_get('arch') == 'Tofino') and \
       (test_param_get('target') == 'hw'):
        fpport = []
        pgrp = swport/4 + 1
        chnl = swport%4
        fpport.append("%d/%d" % (pgrp, chnl))
        try:
            # if built with platform then get the swtype
            swtype = get_swtype(client)
            devports = getDevPorts(swtype, fpport)
        except:
            # not built for platform return the swport
            return swport
        return devports[0]
    else:
        return swport

def get_cpu_port(client):
    cpu_port = 64
    if (test_param_get('arch') == 'Tofino') and \
       (test_param_get('target') == 'hw'):
        try:
            swtype = get_swtype(client)
            if swtype == "mavericks":
                cpu_port = 65 * 4 - 4
            elif swtype == "montara":
                cpu_port = 33 * 4 - 4
        except:
            return cpu_port

    return cpu_port

def get_pipeid(devport):
    if (test_param_get('arch') == 'Tofino') and \
       (test_param_get('target') == 'hw'):
        return (devport >> 7)
    else:
        return 0


###############################################################################
# CPU Header                                                                  #
###############################################################################
class FabricHeader(Packet):
    name = "Fabric Header"
    fields_desc = [
        BitField("packet_type", 0, 3),
        BitField("header_version", 0, 2),
        BitField("packet_version", 0, 2),
        BitField("pad1", 0, 1),
        BitField("fabric_color", 0, 3),
        BitField("fabric_qos", 0, 5),
        XByteField("dst_device", 0),
        XShortField("dst_port_or_group", 0),
    ]


class FabricCpuHeader(Packet):
    name = "Fabric Cpu Header"
    fields_desc = [
        BitField("egress_queue", 0, 5), BitField("tx_bypass", 0, 1),
        BitField("reserved1", 0, 2), XShortField("ingress_port", 0),
        XShortField("ingress_ifindex", 0), XShortField("ingress_bd", 0),
        XShortField("reason_code", 0)
    ]


class FabricCpuSflowHeader(Packet):
    name = "Fabric Cpu Sflow Header"
    fields_desc = [
        XShortField("sflow_sid", 0),
    ]


class FabricCpuTimestampHeader(Packet):
    name = "Fabric Cpu Timestamp Header"
    fields_desc = [
        IntField("arrival_time", 0),
    ]


class FabricCpuBfdEventHeader(Packet):
    name = "Fabric Cpu BFD Event Header"
    fields_desc = [
        XShortField("bfd_sid", 0),
        XShortField("bfd_event", 0),
    ]


class FabricPayloadHeader(Packet):
    name = "Fabric Payload Header"
    fields_desc = [XShortField("ether_type", 0)]


class FabricUnicastHeader(Packet):
    name = "Fabric Unicast Header"
    fields_desc = [
        BitField("routed", 0, 1), BitField("outerRouted", 0, 1),
        BitField("tunnelTerminate", 0, 1), BitField("ingressTunnelType", 0, 5),
        XShortField("nexthopIndex", 0)
    ]


class FabricMulticastHeader(Packet):
    name = "Fabric Multicast Header"
    fields_desc = [
        BitField("routed", 0, 1), BitField("outerRouted", 0, 1),
        BitField("tunnelTerminate", 0, 1), BitField("ingressTunnelType", 0, 5),
        XShortField("ingressIfindex", 0), XShortField("ingressBd", 0),
        XShortField("mcastGrpA", 0), XShortField("mcastGrpB", 0),
        XShortField("ingressRid", 0), XShortField("l1ExclusionId", 0)
    ]


class PktgenPortDownHeader(Packet):
    name = "Pktgen Port Down Header"
    fields_desc = [
        BitField("pad0", 0, 3), BitField("pipe_id", 0, 2),
        BitField("app_id", 0, 3), BitField("pad1", 0, 15),
        BitField("port_num", 0, 9), BitField("packet_id", 0, 16)
    ]


class PktgenRecircHeader(Packet):
    name = "Pktgen Recirc Header"
    fields_desc = [
        BitField("pad0", 0, 3), BitField("pipe_id", 0, 2),
        BitField("app_id", 0, 3), BitField("key", 0, 24),
        XShortField("packet_id", 0)
    ]


class PktgenExtHeader(Packet):
    name = "Pktgen Extention Header"
    fields_desc = [
        BitField("pad0", 0, 48),
        XShortField("ethType", 0),
        BitField("pad1", 0, 512)  # Make the packet at least 64 Bytes
    ]


def simple_pktgen_port_down_packet(app_id=0, pipe_id=0, port_num=0,
                                   packet_id=0):
    pktgen_port_down = PktgenPortDownHeader(
        pad0=0,
        app_id=app_id,
        pipe_id=pipe_id,
        pad1=0,
        port_num=port_num,
        packet_id=packet_id)

    pktgen_ext_header = PktgenExtHeader(pad0=0, ethType=0x9001, pad1=0)

    pkt = pktgen_port_down / pktgen_ext_header

    return pkt


def simple_pktgen_recirc_packet(app_id=0, pipe_id=0, key=0, packet_id=0):
    pktgen_recirc = PktgenRecircHeader(
        pad0=0, app_id=app_id, pipe_id=pipe_id, key=key, packet_id=packet_id)

    pktgen_ext_header = PktgenExtHeader(pad0=0, ethType=0x9001)

    pkt = pktgen_recirc / pktgen_ext_header

    return pkt


def simple_cpu_packet(header_version=0,
                      packet_version=0,
                      fabric_color=0,
                      fabric_qos=0,
                      dst_device=0,
                      dst_port_or_group=0,
                      ingress_ifindex=1,
                      ingress_bd=0,
                      egress_queue=0,
                      tx_bypass=False,
                      ingress_port=1,
                      reason_code=0,
                      sflow_sid=0,
                      bfd_sid=0,
                      arrival_time=0,
                      inner_pkt=None):

    ether = Ether(str(inner_pkt))
    eth_type = ether.type
    ether.type = 0x9000

    fabric_header = FabricHeader(
        packet_type=0x5,
        header_version=header_version,
        packet_version=packet_version,
        pad1=0,
        fabric_color=fabric_color,
        fabric_qos=fabric_qos,
        dst_device=dst_device,
        dst_port_or_group=dst_port_or_group)

    fabric_cpu_header = FabricCpuHeader(
        egress_queue=egress_queue,
        tx_bypass=tx_bypass,
        reserved1=0,
        ingress_port=ingress_port,
        ingress_ifindex=ingress_ifindex,
        ingress_bd=ingress_bd,
        reason_code=reason_code)

    fabric_payload_header = FabricPayloadHeader(ether_type=eth_type)

    pkt = (str(ether)[:14]) / fabric_header / fabric_cpu_header

    if sflow_sid:
        pkt = pkt / FabricCpuSflowHeader(sflow_sid=sflow_sid)
    elif bfd_sid:
        pkt = pkt / FabricCpuBfdEventHeader(bfd_sid=bfd_sid)
    elif arrival_time:
        pkt = pkt / FabricCpuTimestampHeader(arrival_time=arrival_time)

    pkt = pkt / fabric_payload_header

    if inner_pkt:
        pkt = pkt / (str(inner_pkt)[14:])
    else:
        ip_pkt = simple_ip_only_packet()
        pkt = pkt / ip_pkt

    return pkt


def mpls_udp_inner_packet(pktlen=300, mpls_tags=[], inner_frame=None):
    """
    A mpls udp packet contains mpls labels and a payload that would be
    used inside of a mpls udp packet. The key differential between it and a
    simple mpls packet is that it does not have an ethernet header.

    Supports a few parameters:
    @param len Length of packet in bytes w/o CRC
    @param mpls_tags mpls tag stack
    @param inner_frame The inner frame

    """

    if MINSIZE > pktlen:
        pktlen = MINSIZE
    pkt = None
    mpls_tags = list(mpls_tags)
    while len(mpls_tags):
        tag = mpls_tags.pop(0)
        mpls = MPLS()
        if 'label' in tag:
            mpls.label = tag['label']
        if 'tc' in tag:
            mpls.cos = tag['tc']
        if 'ttl' in tag:
            mpls.ttl = tag['ttl']
        if 's' in tag:
            mpls.s = tag['s']
        if pkt == None:
            pkt = mpls
        else:
            pkt = pkt / mpls
    if inner_frame:
        pkt = pkt / inner_frame
    else:
        pkt = pkt / simple_tcp_packet(pktlen=pktlen - len(pkt))
    return pkt


def simple_unicast_fabric_packet(header_version=0,
                                 packet_version=0,
                                 fabric_color=0,
                                 fabric_qos=0,
                                 dst_device=0,
                                 dst_port_or_group=0,
                                 routed=0,
                                 outer_routed=0,
                                 tunnel_terminate=0,
                                 ingress_tunnel_type=0,
                                 nexthop_index=0,
                                 inner_pkt=None):

    ether = Ether(str(inner_pkt))
    eth_type = ether.type
    ether.type = 0x9000

    fabric_header = FabricHeader(
        packet_type=0x1,
        header_version=header_version,
        packet_version=packet_version,
        pad1=0,
        fabric_color=fabric_color,
        fabric_qos=fabric_qos,
        dst_device=dst_device,
        dst_port_or_group=dst_port_or_group)

    fabric_unicast_header = FabricUnicastHeader(
        routed=0,
        outerRouted=0,
        tunnelTerminate=0,
        ingressTunnelType=0,
        nexthopIndex=0)

    fabric_payload_header = FabricPayloadHeader(ether_type=eth_type)

    if inner_pkt:
        pkt = (
            str(ether)[:14]
        ) / fabric_header / fabric_unicast_header / fabric_payload_header / (
            str(inner_pkt)[14:])
    else:
        ip_pkt = simple_ip_only_packet()
        pkt = (
            str(ether)[:14]
        ) / fabric_header / fabric_unicast_header / fabric_payload_header / ip_pkt

    return pkt


def simple_multicast_fabric_packet(header_version=0,
                                   packet_version=0,
                                   fabric_color=0,
                                   fabric_qos=0,
                                   dst_device=0,
                                   dst_port_or_group=0,
                                   routed=0,
                                   outer_routed=0,
                                   tunnel_terminate=0,
                                   ingress_tunnel_type=0,
                                   ingress_ifindex=1,
                                   ingress_bd=0,
                                   mcast_grp_A=0,
                                   mcast_grp_B=0,
                                   ingress_rid=0,
                                   l1_exclusion_id=0,
                                   inner_pkt=None):

    ether = Ether(str(inner_pkt))
    eth_type = ether.type
    ether.type = 0x9000

    fabric_header = FabricHeader(
        packet_type=0x2,
        header_version=header_version,
        packet_version=packet_version,
        pad1=0,
        fabric_color=fabric_color,
        fabric_qos=fabric_qos,
        dst_device=dst_device,
        dst_port_or_group=dst_port_or_group)

    fabric_multicast_header = FabricMulticastHeader(
        routed=routed,
        outerRouted=outer_routed,
        tunnelTerminate=tunnel_terminate,
        ingressTunnelType=ingress_tunnel_type,
        ingressIfindex=ingress_ifindex,
        ingressBd=ingress_bd,
        mcastGrpA=mcast_grp_A,
        mcastGrpB=mcast_grp_B,
        ingressRid=ingress_rid,
        l1ExclusionId=l1_exclusion_id)

    fabric_payload_header = FabricPayloadHeader(ether_type=eth_type)

    if inner_pkt:
        pkt = (
            str(ether)[:14]
        ) / fabric_header / fabric_multicast_header / fabric_payload_header / (
            str(inner_pkt)[14:])
    else:
        ip_pkt = simple_ip_only_packet()
        pkt = (
            str(ether)[:14]
        ) / fabric_header / fabric_multicast_header / fabric_payload_header / ip_pkt

    return pkt


######
# Pktgen header
######
class pktgen_generic_header(Packet):
    name = "Pktgen Generic Header"
    fields_desc = [
        BitField("pad0", 0, 3),
        BitField("pipe_id", 0, 2),
        BitField("app_id", 0, 3),
        BitField("key_msb", 0, 8),
        BitField("batch_id", 0, 16),
        BitField("packet_id", 0, 16),
    ]


class pktgen_ext_header(Packet):
    name = "Pktgen Extention Header"
    fields_desc = [
        BitField("pad0", 0, 48),
        XShortField("ethType", 0),
    ]


def ifindex_from_pipe_port(pipe, port):
    return (pipe * 72) + port + 1


def bfd_event_cpu_packet(mac_da='00:00:01:00:00:00',
                         ingress_port=128,
                         ingress_ifindex=69,
                         rcode=0x217,
                         bfd_sid=1,
                         bfd_event=0):
    from bfd_utils import bfd_ipv4_packet
    pktgen_bfd_pkt = bfd_ipv4_packet(
        pktlen=66,
        eth_dst=mac_da,
        eth_src='00:00:00:00:00:00',
        dl_vlan_enable=False,
        vlan_vid=0,
        vlan_pcp=0,
        dl_vlan_cfi=0,
        ip_src='0.0.0.0',
        ip_dst='0.0.0.0',
        ip_tos=48,
        ip_ttl=255,
        ip_id=0x0001,
        udp_sport=0,
        udp_dport=0,
        with_udp_chksum=False,
        ip_ihl=None,
        ip_options=False,
        version=1,
        diag=0,
        sta=3,
        flags=0x00,
        detect_mult=0x00,
        bfdlen=24,
        my_discriminator=0,
        your_discriminator=0,
        min_tx_interval=00000,
        min_rx_interval=0000,
        echo_rx_interval=0000)
    pkt = simple_cpu_packet(
        header_version=0,
        packet_version=0,
        fabric_color=0,
        fabric_qos=0,
        dst_device=0,
        dst_port_or_group=0,
        ingress_ifindex=ingress_ifindex,
        ingress_bd=0,
        egress_queue=0,
        tx_bypass=False,
        ingress_port=ingress_port,
        reason_code=rcode,
        sflow_sid=0,
        bfd_sid=bfd_sid,
        inner_pkt=pktgen_bfd_pkt)

    fab_pyld_hdr = pkt.getlayer(FabricPayloadHeader)
    fab_pyld_hdr.ether_type = 0x9001
    return pkt


###############################################################################
# CRC16 and Entropy hash calculation                                          #
###############################################################################
import crc16


def crc16_regular(buff, crc=0, poly=0xa001):
    l = len(buff)
    i = 0
    while i < l:
        ch = ord(buff[i])
        uc = 0
        while uc < 8:
            if (crc & 1) ^ (ch & 1):
                crc = (crc >> 1) ^ poly
            else:
                crc >>= 1
            ch >>= 1
            uc += 1
        i += 1
    return crc


def entropy_hash(pkt, layer='ipv4', ifindex=0):
    buff = ''
    if layer == 'ether':
        buff += str(format(ifindex, '02x')).zfill(4)
        buff += pkt[Ether].src.translate(None, ':')
        buff += pkt[Ether].dst.translate(None, ':')
        buff += str(hex(pkt[Ether].type)[2:]).zfill(4)
    elif layer == 'ipv4':
        buff += socket.inet_aton(pkt[IP].src).encode('hex')
        buff += socket.inet_aton(pkt[IP].dst).encode('hex')
        buff += str(hex(pkt[IP].proto)[2:]).zfill(2)
        if pkt[IP].proto == 6:
            buff += str(hex(pkt[TCP].sport)[2:]).zfill(4)
            buff += str(hex(pkt[TCP].dport)[2:]).zfill(4)
        elif pkt[IP].proto == 17:
            buff += str(hex(pkt[UDP].sport)[2:]).zfill(4)
            buff += str(hex(pkt[UDP].dport)[2:]).zfill(4)
    elif layer == 'ipv6':
        buff += socket.inet_pton(socket.AF_INET6, pkt[IPv6].src).encode('hex')
        buff += socket.inet_pton(socket.AF_INET6, pkt[IPv6].dst).encode('hex')
        buff += str(hex(pkt[IPv6].nh)[2:]).zfill(2)
        if pkt[IPv6].nh == 6:
            buff += str(hex(pkt[TCP].sport)[2:]).zfill(4)
            buff += str(hex(pkt[TCP].dport)[2:]).zfill(4)
        elif pkt[IPv6].nh == 17:
            buff += str(hex(pkt[UDP].sport)[2:]).zfill(4)
            buff += str(hex(pkt[UDP].dport)[2:]).zfill(4)
    else:
        buff = ''
    h = crc16_regular(buff.decode('hex'))
    return h


def open_packet_socket(hostif_name):
    s = socket.socket(socket.AF_PACKET, socket.SOCK_RAW,
                      socket.htons(ETH_P_ALL))
    s.bind((hostif_name, ETH_P_ALL))
    s.setblocking(0)
    return s


def socket_verify_packet(pkt, s, timeout=2):
    MAX_PKT_SIZE = 9100
    timeout = time.time() + timeout
    while time.time() < timeout:
        try:
            packet_from_tap_device = Ether(s.recv(MAX_PKT_SIZE))
            if (str(packet_from_tap_device) == str(pkt)):
                return True
        except:
            pass
    return False

def cpu_packet_mask_ingress_bd(pkt):
    pkt = ptf.mask.Mask(pkt)
    pkt.set_do_not_care_scapy(FabricCpuHeader, 'ingress_bd')
    return pkt

def cpu_packet_mask_ingress_bd_and_timestamp(pkt):
    pkt = ptf.mask.Mask(pkt)
    pkt.set_do_not_care_scapy(FabricCpuHeader, 'ingress_bd')
    pkt.set_do_not_care_scapy(FabricCpuTimestampHeader, 'arrival_time')
    return pkt
