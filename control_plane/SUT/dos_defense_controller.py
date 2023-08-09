import pystate # CRC-based state tracking
import dpkt
import socket
import struct
from collections import defaultdict
import threading
import json
from helper import decodeIPv4, encodeNum, encodeIPv4, encodePacketAndRuleList
import time
import binascii
import sys
from dataplaneSocket import DataplaneSocket
import random

def run_dynamic(static_controller, *args, **kwargs):
    # dp_iface = DataplaneSocket(static_controller.TOFINO_INTERFACE)
    dp_iface = DataplaneSocket(static_controller.interace_name)
    dynamic_controller = Controller(dp_iface)

    installed_rules = []

    ruleList = []
    ruleList.append("pd ipv4_lpm add_entry ai_handle_blacklist_pfuzz_ipv4_lpm ipv4_dstAddr 50.0.0.1 ipv4_dstAddr_prefix_length 8")
    static_controller.generate_output_rules(ruleList)
    static_controller.add_entries()
    installed_rules.extend(ruleList)
    while True:
        print("============")
        sys.stdout.flush()
        switchData = dp_iface.receive_packet()
        if switchData is not None:
            ruleList = dynamic_controller.parsePacket(switchData)
            if not ruleList:
                print("Empty ruleList")
            else:
                print("--- Install ruleList len: {} ---".format(len(ruleList)))
                print(ruleList)
                sys.stdout.flush()
                ruleList = [rule for rule in ruleList if rule not in installed_rules]
                print("--- Sanitized ruleList len: {} ---".format(len(ruleList)))
                sys.stdout.flush()
                static_controller.generate_output_rules(ruleList)
                static_controller.add_entries()
                installed_rules.extend(ruleList)

    # Debug read registers


class Controller(pystate.TrackState):
    @pystate.track_init
    def __init__(self, _dp_socket, visited_bytes=8):
        self.dp_socket = _dp_socket
        self.routingTable = defaultdict(lambda: {"cost": INFINITY, "nhAddr":''})
        self.existingEntries = []
        self.dvLock = threading.Lock()
        self.visited_bytes = visited_bytes
        self.visited_bytes = 12  # TODO: change hardcoding

        self.dst_ips = set()
        self.SDTsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    @pystate.track_stack_calls
    def parsePacket(self, packet):
        print("--- parsePacket ---")
        sys.stdout.flush()
        ruleList = []
        fp4_header = packet[:self.visited_bytes]
        index = self.visited_bytes + 14  # Ethernet
        index = self.visited_bytes + 16  # ipv4 and dst_ip
        dst_ip = Controller.prettify_ip(packet[index:index+4])
        print("--- dst_ip: {} ---".format(dst_ip))
        sys.stdout.flush()

        if dst_ip not in self.dst_ips:
            self.dst_ips.add(dst_ip)
            ruleList.append("pd ipv4_lpm add_entry ai_handle_blacklist_pfuzz_ipv4_lpm ipv4_dstAddr " + dst_ip + " ipv4_dstAddr_prefix_length 8")

        return ruleList

    @staticmethod
    def prettify_mac(mac_string):
        return ':'.join('%02x' % b for b in mac_string)

    @staticmethod
    def prettify_ip(ip_bytes):
        ip_bytes = struct.unpack("!BBBB", ip_bytes)
        return '.'.join([str(b) for b in ip_bytes])
