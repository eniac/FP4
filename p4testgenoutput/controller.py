#import ptf
import signal
import importlib
#import sys
import time
import subprocess
import json
import sys
import socket
from datetime import datetime

from configuration import *

from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol
from thrift.protocol import TMultiplexedProtocol

from res_pd_rpc.ttypes import DevTarget_t

from covDet.stateMachine import StateMachine
from network.dataplaneSocket import DataplaneSocket
from network.packet import Packet, createEmptyPacket

from pal_rpc.ttypes import *
import pal_rpc.pal as pal_i

import conn_mgr_pd_rpc.conn_mgr as conn_mgr_client_module

from ptf.thriftutils import hex_to_i16

def main():
    print("====== Waiting 2s for switch to start ======")
    time.sleep(2)
    print("--- Assuming switch has started ---")
    controller = DTController(PROGRAM, None, C_IP)
    controller.bring_ports_up()

    def signal_handler(signal, frame):
        controller.cleanup()
        #controller.end()
        sys.exit(0)
    signal.signal(signal.SIGINT, signal_handler)

    controller.initialize_CLI_variables()    
    controller.main_loop()

class DTController:
    def __init__(self, program_name, rules_file = None, c_ip = None, port_map_filename="config/portMap.json"):
        print("====== DTController ======")
        self.transport = TTransport.TBufferedTransport(TSocket.TSocket('localhost', 9090))
        self.bprotocol = TBinaryProtocol.TBinaryProtocol(self.transport)
        self.transport.open()

        self.bw_dict = {"10G":pal_port_speed_t.BF_SPEED_10G, 
                        "40G":pal_port_speed_t.BF_SPEED_40G, 
                    "25G":pal_port_speed_t.BF_SPEED_25G,
                "100G":pal_port_speed_t.BF_SPEED_100G}
        self.fec_dict = {"NONE" :pal_fec_type_t.BF_FEC_TYP_NONE,
                         "FC" : pal_fec_type_t.BF_FEC_TYP_FIRECODE,
                 "RS" : pal_fec_type_t.BF_FEC_TYP_REED_SOLOMON}

        self.pal_protocol = TMultiplexedProtocol.TMultiplexedProtocol(self.bprotocol, "pal")
        self.pal = pal_i.Client(self.pal_protocol)

        print("program_name", program_name)
        self.dp_client_module = importlib.import_module(program_name + ".p4_pd_rpc." +program_name)
        self.dp_ttypes = importlib.import_module(program_name + ".p4_pd_rpc.ttypes")

        self.p4_protocol = TMultiplexedProtocol.TMultiplexedProtocol(self.bprotocol, program_name)
        self.client = self.dp_client_module.Client(self.p4_protocol)
        self.conn_mgr_protocol = TMultiplexedProtocol.TMultiplexedProtocol(self.bprotocol, "conn_mgr")
        self.conn = conn_mgr_client_module.Client(self.conn_mgr_protocol)
        self.conn_hdl = self.conn.client_init()


        self.dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))

        self.reg_flags = eval("self.dp_ttypes." + program_name + "_register_flags_t(read_hw_sync=True)")

        self.port_map = json.load(open(port_map_filename, "r"))

        self.program_name = program_name

        self.devPorts = []
        self.rules_file = RULES_FILE

        if c_ip is not None:
            print("--- c_ip {} ---".format(c_ip))
            self.SUTsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self.SUTsocket.bind((c_ip, 10000))
            self.SUTsocket.setblocking(False)
        else:
            self.SUTsocket = None

    @staticmethod
    def chunks(lst, n):
        """Yield successive n-sized chunks from lst."""
        for i in range(0, len(lst), n):
            yield lst[i:i + n]
 
    def readRegister(self, registerName, index, pipeNum=0):
        val = eval("self.client.register_read_" + registerName + "(" \
            "self.conn_hdl, self.dev_tgt, " + str(index) +" , self.reg_flags)")
        self.conn.complete_operations(self.conn_hdl)
        return val[pipeNum]

    def bring_ports_up(self, rate="100G", fec="NONE"):
        print("====== bring_ports_up ======")
        interface_list = [str(x) + '/0' for x in range(9,25)]
        devPorts = [self.port_map[p] for p in interface_list]

        print ("enabling ports: %s (dev ids: %s) at rate: %s"%(interface_list, devPorts, rate))
        for i in devPorts:
            print ("port: %s"%i)
            try:            
                self.pal.pal_port_add(0, i,
                                       self.bw_dict[rate],
                                       self.fec_dict[fec])
                self.pal.pal_port_enable(0, i)
            except:
                print ("\tport not enabled (already up?)")

        self.devPorts += devPorts
        print("All ports up")
        return devPorts


    def initialize_CLI_variables(self):
        print("====== initialize_CLI_variables ======")
        self.dp_iface = DataplaneSocket(TOFINO_INTERFACE)
        self.counter = 0
        self.baseName = self.program_name.split("_p4testgen")[0]
        print("baseName: {}".format(self.baseName))
        # self.packetParser = StateMachine(baseName + "_parserFile.json")
        # initialRules = self.coverageDetector.add_initial_seed_packets()
        self.discardPackets = self.readRegister('forward_count_register', 0)

    def cleanup(self):
        # self.mc.mc_destroy_session(self.mc_sess_hdl)
        self.conn.complete_operations(self.conn_hdl)
        self.conn.client_cleanup(self.conn_hdl)

        self.transport.close()

    @staticmethod
    def decodePacketAndRuleList(bytes):
        pair = bytes.split("@$", 1)
        assert len(pair) == 2
        packet = pair[1]
        ruleListBytes = pair[0].split("@@")[1:]
        ruleList = [str(rule) for rule in ruleListBytes]
        return packet, ruleList

    def main_loop(self):
        print("====== main_loop prologue ======")
        time.sleep(1)
        packetsForwarded = self.readRegister('forward_count_register', 0)

        trace_file = "inspected_coverage.json"

        p4testgetJson_file = open(trace_file, "r")
        p4testgetJson = json.load(p4testgetJson_file)

        packet_list = p4testgetJson[self.baseName]["packet_list"]
        print(p4testgetJson[self.baseName]["number_of_paths"])
        print(p4testgetJson[self.baseName]["number_of_actions"])

        header = PFuzzHeader(self.baseName)

        print("Sending {} packets...".format(len(packet_list)))
        for packet in packet_list:
            time.sleep(2)
            print("--- Original payload to send ---")
            print(packet)
            sys.stdout.flush()
            payload = bytearray(eval(packet))
            self.dp_iface.send_packet(payload, initial_bytes=header.get_header_size())

            print("--- self.dp_iface.receive_packet ---")
            switchData = self.dp_iface.receive_packet()
            # print(type(switchData))
            header.decode_rcv_bytearray(switchData)
            hex_string = " ".join("{:02x}".format(byte) for byte in switchData)
            print(hex_string)
            packetsForwarded = self.readRegister('forward_count_register', 0)

            print("Packet counter {0}".format(packetsForwarded))

        print("=== Digest ===")

        print("==============")

class PFuzzHeader:
    def __init__(self, program_name):
        self.program_name = program_name
        self.name2bytes = {
            "basic_routing": 12,
        }
        self.name2encodinglist = {
            "basic_routing": [
                ("encoding_e0", 8),
                ("encoding_i0", 8),
                ("encoding_i1", 8)
            ],
        }

        # Some global statistics
        self.paths_seen = []
        self.actions_seen = []

        self.all_actions = []

        self.total_num_paths = 1
        self.total_num_actions = 0

        self.field2encoding2path = {}
        self.total_num_vars = 0

        ingress_plan_json = None
        egress_plan_json = None
        print("--- Read ingress plan ---")
        with open("/home/leoyu/FP4/instrumentation/mvbl/plan/"+self.program_name+"_ingress.json") as f:
            ingress_plan_json = json.load(f)
        self.ingress_num_vars = ingress_plan_json["num_vars"]
        self.total_num_vars += self.ingress_num_vars
        print("ingress_num_vars: {}".format(self.ingress_num_vars))
        for var in range(self.ingress_num_vars):
            num_paths = ingress_plan_json[str(var)]["num_paths"]
            print("var {0}, num_paths: {1}".format(var, num_paths))
            self.total_num_paths *= num_paths
            self.field2encoding2path["encoding_i"+str(var)] = ingress_plan_json[str(var)]["encoding_to_path"]
        # Get the total number of actions
        if self.ingress_num_vars != 0:
            for actions in ingress_plan_json["table2actions_dict"].values():
                self.total_num_actions += len(actions)
                self.all_actions.extend(actions)
            
        print("--- Read egress plan ---")
        with open("/home/leoyu/FP4/instrumentation/mvbl/plan/"+self.program_name+"_egress.json") as f:
            egress_plan_json = json.load(f)
        self.egress_num_vars = egress_plan_json["num_vars"]
        self.total_num_vars += self.egress_num_vars
        print("egress_num_vars: {}".format(self.egress_num_vars))
        for var in range(self.egress_num_vars):
            num_paths = egress_plan_json[str(var)]["num_paths"]
            print("var {0}, num_paths: {1}".format(var, num_paths))
            self.total_num_paths *= num_paths
            self.field2encoding2path["encoding_e"+str(var)] = egress_plan_json[str(var)]["encoding_to_path"]
        if self.egress_num_vars != 0:
            for actions in egress_plan_json["table2actions_dict"].values():
                self.total_num_actions += len(actions)
                self.all_actions.extend(actions)

        print("self.total_num_paths: {}".format(self.total_num_paths))
        print("self.total_num_actions: {}".format(self.total_num_actions))
        print("self.all_actions: {}".format(self.all_actions))
        print("self.field2encoding2path: {}".format(self.field2encoding2path))
        
    def get_header_size(self):
        return self.name2bytes[self.program_name]

    def decode_rcv_bytearray(self, data):
        print("--- decode_rcv_bytearray of size {}B ---".format(len(data)))
        index = 0
        index += 6  # Skip preamble
        pkt_type = int(data[index] >> 6)
        # print("pkt_type: {}".format(pkt_type))
        if pkt_type != 1:
            raise Exception("pkt_type != 1")
        index += 1  # Skip pkt_type and pad

        # Now go through each of the encoding
        path_seen = []
        for field2bits in self.name2encodinglist[self.program_name]:
            field_name = field2bits[0]
            bits = field2bits[1]
            if bits % 8 != 0:
                raise Exception("bits % 8 != 0")
            num_bytes = bits / 8
            byte_slice = (data[index:index+num_bytes])
            if not byte_slice:
                raise Exception("Empty byte slice")
            encoding_val = 0
            for byte in byte_slice:
                encoding_val = encoding_val * 256 + byte
            index += num_bytes
            print("field_name: {0}, value: {1}".format(field_name, encoding_val))
            print("sub-DAG path: {}".format(self.field2encoding2path[field_name][str(encoding_val)]))
            for node in self.field2encoding2path[field_name][str(encoding_val)]:
                path_seen.append(node)
                if "_pfuzz_" in node:
                    if node not in self.actions_seen:
                        self.actions_seen.append(node)

        print("path_seen: {}".format(path_seen))
        print("self.actions_seen: {}".format(self.actions_seen))
        if path_seen not in self.paths_seen:
            self.paths_seen.append(path_seen)
        else:
            print("path_seen already in self.paths_seen!")

        self.action_coverage = 1.0*len(self.actions_seen)/self.total_num_actions
        self.path_coverage = 1.0*len(self.paths_seen)/self.total_num_paths
        print("--- Path coverage: {0}/{1}={2} ---".format(len(self.paths_seen), self.total_num_paths, self.path_coverage))
        print("--- Action coverage: {0}/{1}={2}".format(len(self.actions_seen), self.total_num_actions, self.action_coverage))

        print("--- self.all_actions - self.seenActions ---")
        print(list(set(self.all_actions) - set(self.actions_seen)))
        sys.stdout.flush()  
    
    def get_digest(self):
        print("self.paths_seen: {}".format(self.paths_seen))
        print("self.actions_seen: {}".format(self.actions_seen))
        print("Path coverage: {0}/{1}={2}".format(len(self.paths_seen), self.total_num_paths, self.path_coverage))
        print("Action coverage: {0}/{1}={2}".format(len(self.actions_seen), self.total_num_actions, self.action_coverage))
        print("Unseen actions: {}".format(list(set(self.all_actions) - set(self.actions_seen))))

if __name__ == '__main__':
    main()
