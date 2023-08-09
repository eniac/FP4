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

    # TODO: data plane not mirroring any packets, probably the mirror bug, add 1 rule to bootstrap; however, still not receiving any rules...
    ruleList = []
    ruleList.append("pd tiLearnMAC add_entry aiNoOp_pfuzz_tiLearnMAC ethernet_srcAddr 0x000000000000")
    ruleList.append("pd tiForward add_entry aiForward_pfuzz_tiForward ethernet_dstAddr 0x000000000000 action_egress_port 0")
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

class Controller(pystate.TrackState):
    @pystate.track_init
    def __init__(self, _dp_socket, visited_bytes=8):
        self.dp_socket = _dp_socket
        self.routingTable = defaultdict(lambda: {"cost": INFINITY, "nhAddr":''})
        self.existingEntries = []
        self.dvLock = threading.Lock()
        self.myIPs = []
        self.visited_bytes = visited_bytes
        self.visited_bytes = 10  # TODO: change hardcoding

        self.src_macs = set()
        self.SDTsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    @pystate.track_stack_calls
    def parsePacket(self, packet):
        print("--- parsePacket ---")
        sys.stdout.flush()
        ruleList = []
        fp4_header = packet[:self.visited_bytes]
        index = self.visited_bytes + 6
        # TODO: read the source port from temp_port in fp4_header
        src_mac = struct.unpack("!BBBBBB", packet[index:index+6])
        src_mac = Controller.prettify_mac(src_mac)
        print("--- src_mac: {} ---".format(src_mac))
        sys.stdout.flush()
        if src_mac not in self.src_macs:
            self.src_macs.add(src_mac)
            # Install rules for tiLearnMAC
            rule = ("pd tiLearnMAC add_entry aiNoOp_pfuzz_tiLearnMAC ethernet_srcAddr 0x" + str(src_mac).replace(":",""))
            ruleList.append(rule)

            # Instal rules for tiForward
            port_list = [i for i in range(0, 4, 60)]
            random_port = random.choice(port_list)
            rule = ("pd tiForward add_entry aiForward_pfuzz_tiForward ethernet_dstAddr 0x" + str(src_mac).replace(":","") + " action_egress_port " + str(random_port))
            ruleList.append(rule)
            # TODO: differentiate update/write
        return ruleList

    @staticmethod
    def prettify_mac(mac_string):
        return ':'.join('%02x' % b for b in mac_string)
