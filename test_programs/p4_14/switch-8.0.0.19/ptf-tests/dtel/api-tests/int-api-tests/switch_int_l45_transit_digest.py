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
INT L45 transit tests with Digest
"""

import switchapi_thrift

import time
import sys
import logging
import ctypes

import unittest
import random

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *

import os
from math import ceil

from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *
import pdb
import crcmod

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../../base/api-tests'))
import api_base_tests

sys.path.append(os.path.join(this_dir, '../..'))
from dtel_utils import *


device = 0

swports = [0, 1, 2]

switch_id = 0x11111111
params = SwitchConfig_Params()
params.switch_id = switch_id
params.mac_self = '00:77:66:55:44:33'
params.nports = 2
params.ipaddr_inf = ['2.2.2.1',  '1.1.1.2']
params.ipaddr_nbr = ['2.2.2.2', '1.1.1.1']
params.mac_nbr = ['00:11:22:33:44:54', '00:11:22:33:44:55']
params.report_ports = None
params.device = device
params.swports = swports

@group('transit_l45')
class intl45_transitTest_hop2_with_digest(api_base_tests.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Test INT L45 Digest transit device - add and encode 2hop info"
        prepare_int_l45_bindings()
        config = SwitchConfig(self, params)

        # Enable INT transit processing
        self.client.switch_api_dtel_int_transit_enable(device)

        # create a packet coming from INT src - i.e.
        #   - It has INT meta header, but no INT data
        #   Each transit device will fill the data
        payload = 'int l45'
        pkt = simple_udp_packet(
            eth_dst=params.mac_self,
            eth_src=params.mac_nbr[0],
            ip_id=0,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=64,
            udp_sport=101,
            with_udp_chksum=False,
            udp_payload=payload)

        int_pkt = int_l45_src_packet(
            test=self,
            int_inst_mask=0xDC00,
            int_inst_cnt=5,
            pkt=pkt)

        exp_pkt = simple_udp_packet(
            eth_dst=params.mac_nbr[1],
            eth_src=params.mac_self,
            ip_id=0,
            ip_dst=params.ipaddr_nbr[1],
            ip_src=params.ipaddr_nbr[0],
            ip_ttl=63,
            udp_sport=101,
            with_udp_chksum=False,
            udp_payload=payload)

        exp_pkt = int_l45_src_packet(
            test=self,
            int_inst_mask=0xDC00,
            int_inst_cnt=5,
            pkt=exp_pkt)

        # add 1 hop info to the packet
        int_pkt = int_l45_packet_add_hop_info(Packet=int_pkt, val=5)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x0)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x0)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x0)
        int_pkt = int_l45_packet_add_hop_info(
            Packet=int_pkt, val=0x22222222, incr_cnt=1)
        # add Digest headers
        digest = 0xab12
        digest_pkt = int_l45_packet_add_update_digest(
            Packet=int_pkt, encoding=digest)

        exp_pkt = int_l45_packet_add_hop_info(Packet=exp_pkt, val=0x5)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x0)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x0)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x0)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x22222222, incr_cnt=1)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x7FFFFFFF)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x7FFFFFFF)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=0x7FFFFFFF)
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt,
            val=int_port_ids_pack(swports[0], swports[1]))
        exp_pkt = int_l45_packet_add_hop_info(
            Packet=exp_pkt, val=switch_id, incr_cnt=1)
        # add Digest headers
        exp_encoding = digest ^ (switch_id - 0)  # latency unknown
        exp_pkt = int_l45_packet_add_update_digest(
            Packet=exp_pkt, encoding=exp_encoding)  #exp_encoding will be ignore

        # set quantization shift
        self.client.switch_api_dtel_latency_quantization_shift(
            device=device, quant_shift=0)

        try:
            send_packet(self, swports[0], str(digest_pkt))
            (rcv_encoding, rcv_metadata, nrcv) = verify_int_packet(
                test=self, pkt=exp_pkt, port=swports[1],
                digest=True, ignore_hop_indices=[3,4,5])
            rcv_latency = rcv_metadata[2] - rcv_metadata[1]
            ports = nrcv.getlayer(INT_hop_info, 2) # ports

            # fields must be extended up to their length, then concatenate
            v = rcv_latency  # 32 bit latency
            v = (v << 9)  | ((ports.val >> 16) & 0x1ff) # 9 bits ingress port
            v = (v << 9)  | (ports.val & 0x1ff) # 9 bits egress ports
            # we ignore switch ID in encoding
            # v = (v << 32) | (switch_id) # 32 bits switchid
            v = (v << 16) | digest
            v_len = ceil((32 + 9 + 9 +16) / 4.0)
            v_hex = '%x' % v
            if (v_len % 2 == 1):
                v_len+=1
            while (len(v_hex) < v_len):
                v_hex = '0' + v_hex
            crc32 = crcmod.Crc(0x18005, initCrc=0, xorOut=0x0000)
            crc32.update(bytes(bytearray.fromhex(v_hex)));
            exp_encoding = crc32.crcValue;

            print("expected %x received %x latency %d\n" % (exp_encoding, rcv_encoding,
                                        rcv_latency));

            self.assertTrue(rcv_encoding == exp_encoding,
                            "Digest encoding doesn't match.")

            verify_no_other_packets(self)

        finally:
            ### Cleanup
            cleanup_int_l45_bindings()
            self.client.switch_api_dtel_int_transit_disable(device)
            config.cleanup(self)
