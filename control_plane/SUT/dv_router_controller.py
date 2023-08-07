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

INFINITY = 16

def run_dynamic(static_controller, *args, **kwargs):
    # dp_iface = DataplaneSocket(static_controller.TOFINO_INTERFACE)
    dp_iface = DataplaneSocket(static_controller.interace_name)
    dynamic_controller = Controller(dp_iface)
    while True:
        switchData = dp_iface.receive_packet()
        if switchData is not None:
            ruleList, to_send = dynamic_controller.parsePacket(switchData)
            if not ruleList:
                print("ruleList empty")
            else:
                static_controller.generate_output_rules(ruleList)
                static_controller.add_entries()

            if to_send is not None:
                dynamic_controller.SDTsocket.sendto(to_send, (kwargs["SUT_IP"], kwargs["SUT_Port"]))

class ARP:
    def __init__(self):
        self.htype = 0
        self.ptype = 0
        self.hlen = 0
        self.plen = 0
        self.oper = 0
        self.senderHA = 0
        self.senderPA = 0
        self.targetHA = 0
        self.targetPA = 0

class DISTANCE_VEC:
    def __init__(self):
        self.preamble = 0
        self.src = 0
        self.length_dist = 0
        self.data = 0

class Controller(pystate.TrackState):
    @pystate.track_init
    def __init__(self, _dp_socket, visited_bytes=8):
        self.dp_socket = _dp_socket
        self.distanceVector = []
        self.routingTable = defaultdict(lambda: {"cost": INFINITY, "nhAddr":''})
        self.existingEntries = []
        self.dvLock = threading.Lock()
        self.myIPs = []
        self.visited_bytes = visited_bytes
        self.arp_entries = set()
        self.dist_entires = set()
        self.SDTsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        # --- Begin state tracking code ---
        # Shadow dataplane table rules so they're tracked by pystate
        self.dp_rules_shadow = []
        # --- End state tracking code ---

        config = json.loads('[{"ip": "10.0.1.100", "prefix_len": 24, "mac": "00:00:00:01:01:00", "port": 1}, ' \
                             '{"ip": "20.0.1.1",   "prefix_len": 24, "mac": "00:00:00:01:02:00", "port": 2}, ' \
                             '{"ip": "20.0.2.1",   "prefix_len": 24, "mac": "00:00:00:01:03:00", "port": 3}]')

        for p in config:
            self.myIPs.append(p["ip"])

        update_thread = threading.Thread(target=self.SendDistanceVector)
        update_thread.daemon = True
        update_thread.start()

    @pystate.track_stack_calls
    def SendDistanceVector(self):
        time.sleep(30)
        while True:
            # build payload
            for ip in self.myIPs:
                self.dvLock.acquire()
                payload = self.buildRoutingPayload(ip)
                self.dvLock.release()
                self.dp_socket.send_packet(payload, self.visited_bytes)

            time.sleep(10)

    @pystate.track_stack_calls
    def parsePacket(self, packet):
        ruleList = []
        fp4_header = packet[:self.visited_bytes]
        index = self.visited_bytes + 12
        ethernetType = struct.unpack("!H", packet[index:index+2])[0]

        index = index + 2
        to_send = None
        if ethernetType == 2054:
            arp = ARP()
            arp.htype, arp.ptype, arp.hlen, arp.plane, arp.oper = struct.unpack("!HHBBH", packet[index:index+8])
            index += 8
            arp.senderHA = Controller.prettify_mac(struct.unpack("!BBBBBB", packet[index:index + 6]))
            index += 6
            arp.senderPA = Controller.prettify_ip(packet[index:index+4])
            index += 4
            arp.targetHA = Controller.prettify_mac(struct.unpack("!BBBBBB", packet[index:index+6]))
            index+=6
            arp.targetPA = Controller.prettify_ip(packet[index:index+4])
            index +=4
            ingress_port = struct.unpack("!H", packet[index:index+2])[0]
            ingress_port = ingress_port & 511
            index+=2
            entryValue = arp.targetHA.split(".")[0]
            if entryValue not in self.arp_entries:
                self.arp_entries.add(entryValue)
                ruleList.extend(Controller.AddARPEntry(ingress_port, arp.senderHA, arp.targetHA, arp.targetPA))
                print("adding arp entry", ruleList[-1])

        elif ethernetType == 1363:
            dv = DISTANCE_VEC()
            dv.preamble, dv.src, dv.length = struct.unpack("!IIH", packet[index:index+10])
            index+=10
            dv.data = packet[index:index+144]
            dv.src = dv.src & 511
            dv.src = (dv.src % 16)*4
            ruleList.extend(self.ProcessRoutingUpdate(dv.src, dv.length, dv.data))

        # --- Begin state tracking code ---
        self.dp_rules_shadow += ruleList
        if self.is_new():
            # New state reached, send packet & CRC value
            to_send = encodePacketAndRuleList(packet, ruleList)
            self.SDTsocket.sendto(to_send, ("169.254.0.100", 10000))
        # --- End state tracking code ---

        return ruleList, to_send

    @staticmethod
    def getPacket(pktBytes):
        eth = dpkt.ethernet.Ethernet(pktBytes)
        print ("---- Notification ----")
        print ("\tTotal Length: %s"%len(pktBytes))
        print("Data length: " + str(len(eth.data)))

        try:
            print ("\tEth SRC / DST: %s --> %s"%(binascii.hexlify(eth.src), binascii.hexlify(eth.dst)))
            print ("\tEth payload (notification): %s"%binascii.hexlify(eth.data)[0:25])
        except:
            print ("\tRaw packet bytes: %s"%binascii.hexlify(pktBytes))

        return eth

    @staticmethod
    def prettify_ip(ip_bytes):
        ip_bytes = struct.unpack("!BBBB", ip_bytes)
        return '.'.join([str(b) for b in ip_bytes])

    @staticmethod
    def prettify_mac(mac_string):
        return ':'.join('%02x' % b for b in mac_string)

    @staticmethod
    def round_down(num, divisor):
        return num - (num%divisor)

    @pystate.track_stack_calls
    def ProcessRoutingUpdate(self,src, length, data):

        ruleList = []

        updated = False
        iterations = min(Controller.round_down(len(data),6)/6, length)
        for i in range(iterations):
            prefix, length, cost = struct.unpack_from('!IBB', data, i * 6)
            prefix = decodeIPv4(struct.pack('!I', prefix))

            currentCost = self.routingTable[(prefix, length)]["cost"]
            costThruSrc = cost + 1
            # small value of infinity for count-to-infinity
            if costThruSrc > INFINITY:
                costThruSrc = INFINITY

            if costThruSrc < currentCost:
                # send packets through src if cheaper
                self.routingTable[(prefix, length)]["cost"] = costThruSrc
                self.routingTable[(prefix, length)]["nhAddr"] = src
                # print("adding routing", prefix, '/', length, src)
                sys.stdout.flush()

                # add actual table entry
                ruleList.extend(self.AddRoutingEntry(prefix, length, src))
                updated = True
            elif src == self.routingTable[(prefix, length)]["nhAddr"]:
                # if src is already next hop, see if path cost has changed
                if costThruSrc != currentCost:
                    self.routingTable[(prefix, length)]["cost"] = costThruSrc
                    updated = True

        if updated:
            self.dvLock.acquire()
            del self.distanceVector[:]
            for (prefix, length), info in self.routingTable.items():
                self.distanceVector.append([prefix, length, info["cost"]])
            self.dvLock.release()

        return ruleList


    @staticmethod
    def AddARPEntry(local_port, local_mac, target_mac, target_ip):
        ruleList = []
        rule = 'pd tiHandleOutgoingEthernet add_entry aiForward cis553_metadata_nextHop ' + \
            str(target_ip) + ' cis553_metadata_nextHop_prefix_length 8 action_mac_sa 0x' \
            + local_mac.replace(":","") + ' action_mac_da 0x' + target_mac.replace(":","") + ' action_egress_port ' + str(local_port)
        ruleList.append(rule)
        return ruleList

    @pystate.track_stack_calls
    def findUpdatedCRC32(self):
        return 0


    @pystate.track_stack_calls
    def buildRoutingPayload(self, my_ip):
        """
        See documentation at beginning of document.

        This function uses struct.pack/unpack in order to make sure that the
        bytes are packed correctly.  Other function pack/unpack automatically
        with protobufs.  With this function, we're building the routing payload
        ourselves.
        """
        payload = bytearray(258)  # 6 (header) + 42 * 6 (prefix + length + cost)
        payload[0:4] = encodeIPv4(my_ip)
        payload[4:6] = encodeNum(len(self.distanceVector), 16)

        for i in range(min(len(self.distanceVector),3)):
            prefix, length, cost = self.distanceVector[i]
            prefix = struct.unpack('!I', encodeIPv4(prefix))[0]
            struct.pack_into('!IBB', payload, 6 + i * 6, prefix, length, cost)

        return payload

    @pystate.track_stack_calls
    def AddRoutingEntry(self, prefix, length, nextHop):
        print("Adding routing entry")
        ruleList = []
        rule = 'pd tiHandleIpv4 add_entry aiFindNextL3Hop ipv4_dstAddr ' + str(prefix) + ' ipv4_dstAddr_prefix_length ' \
            + str(length) + ' action_nextHop ' + str(nextHop)
        ruleList.append(rule)

        if (prefix, length) not in self.existingEntries:
            ruleList.append(rule)
            self.existingEntries.append((prefix, length))
        else:
            rule = 'pd tiHandleIpv4 mod_entry aiFindNextL3Hop by_match_spec ipv4_dstAddr ' + str(prefix) + ' ipv4_dstAddr_prefix_length ' \
            + str(length) + ' action_nextHop ' + str(nextHop)
            ruleList.append(rule)
        return ruleList
