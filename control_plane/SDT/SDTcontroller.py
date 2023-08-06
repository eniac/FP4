#import ptf
import signal
import importlib
#import sys
import time
import subprocess
import json
import socket
from datetime import datetime

from configuration import *

from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol
from thrift.protocol import TMultiplexedProtocol

from res_pd_rpc.ttypes import DevTarget_t

from covDet.coverageDetector import CoverageDetector
from covDet.stateMachine import StateMachine
from network.dataplaneSocket import DataplaneSocket
from network.packet import Packet, createEmptyPacket

from pal_rpc.ttypes import *
import pal_rpc.pal as pal_i

import conn_mgr_pd_rpc.conn_mgr as conn_mgr_client_module

from ptf.thriftutils import hex_to_i16

def main():
    print("====== Waiting for switch to start ======")
    time.sleep(15)
    print("--- Assuming switch has started ---")
    controller = DTController(PROGRAM, RULES_FILE, C_IP)
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

    def add_entries(self, ruleList):
        print("====== add_entries prologue ======")
        for current_list in DTController.chunks(ruleList, 50):
            print("--- current_list of len {} ---".format(len(current_list)))
            outFile = open(self.program_name + '_rules.txt', 'w')
            outFile.write("pd-" + self.program_name.replace("_", "-") + "\n")

            for item in current_list:
                print(item)
                outFile.write("%s\n" % item)

            outFile.write("end\nexit\n")
            command = home + "/bf-sde-9.2.0/install/bin/bfshell -f " + outFile.name
            outFile.close()
            process = subprocess.Popen(command.split(), stdout=subprocess.PIPE)
            output, error = process.communicate()
            # time.sleep(1)
        print("====== add_entries epilogue ======")
        return       
 
    def readRegister(self, registerName, index, pipeNum=0):
        val = eval("self.client.register_read_" + registerName + "(" \
            "self.conn_hdl, self.dev_tgt, " + str(index) +" , self.reg_flags)")
        self.conn.complete_operations(self.conn_hdl)
        return val[pipeNum]

    def bring_ports_up(self, rate="25G", fec="NONE"):
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
        baseName = self.program_name.split("_dt_hw")[0]
        print("baseName: {}".format(baseName))
        self.packetParser = StateMachine(baseName + "_parserFile.json")
        self.coverageDetector = CoverageDetector(baseName, simulation = SIMULATION, rulesFile=self.rules_file, mode=MODE)

        initialRules = self.coverageDetector.add_initial_seed_packets()
        self.discardPackets = self.readRegister('forward_count_register', 0)

        self.add_entries(initialRules)
        print("====== initialize_CLI_variables epilogue ======")

        # self.initialBits = 0

        # for field, numBytes in Header.fields["fp4_visited"]:
        #     if "field" in field:
        #         self.initialBits += numBytes

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
        time.sleep(2)
        packetsForwarded = self.readRegister('forward_count_register', 0)
        self.coverageDetector.set_packets_forwarded(packetsForwarded)
        print("Before entering loop time {}".format(datetime.now().time()))
        start_time = None

        while True:
            print("--- self.dp_iface.receive_packet ---")
            switchData = self.dp_iface.receive_packet()
            packetsForwarded = self.readRegister('forward_count_register', 0)

            if packetsForwarded < 0:
                packetsForwarded += (2**32)
            if start_time is None:    
                print("First packet forwarded on port 0, counter {0}, time: {1}".format(packetsForwarded, datetime.now().time()))
            else:
                print("Packet forwarded on port 0, counter {0}, duration {1}s".format(packetsForwarded, (datetime.now() - start_time).total_seconds()))

            self.coverageDetector.set_packets_forwarded(packetsForwarded)
            if switchData is not None:
                current_packet = self.packetParser.parsePacket(switchData)
                if (current_packet is None):
                    pass
                else:
                    if start_time is None:
                        start_time = datetime.now()
                    entries = self.coverageDetector.check_coverage_and_add_seed(current_packet, True)
                    if not entries:
                        pass
                    else:
                        self.add_entries(entries)

            if self.SUTsocket is not None:
                try:
                    data, addr = self.SUTsocket.recvfrom(1024)
                    packet, ruleList = DTController.decodePacketAndRuleList(data)
                    current_packet = self.packetParser.parsePacket(bytearray(packet))
                    if (current_packet is None):
                        pass
                    else:
                        self.coverageDetector.add_new_rules(ruleList)
                        entries = self.coverageDetector.check_coverage_and_add_seed(current_packet, True)
                        if not entries:
                            pass
                        else:
                            self.add_entries(entries)

                except socket.error:
                    pass

            if start_time is not None and ((datetime.now() - start_time).total_seconds() > 300):
                print(start_time)
                print(datetime.now())
                exit()

if __name__ == '__main__':
    main()
