import argparse
import subprocess
import sys
import json
import numpy as np
import importlib
import time
import os

from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol
from thrift.protocol import TMultiplexedProtocol

from os.path import expanduser

home = expanduser("~")
sys.path.append(home + "/bf-sde-9.2.0/install/lib/python2.7/site-packages/tofino")
sys.path.append(home + "/bf-sde-9.2.0/install/lib/python2.7/site-packages")
sys.path.append(home + "/bf-sde-9.2.0/install/lib/python2.7/site-packages/tofinopd")

from res_pd_rpc.ttypes import DevTarget_t
import pal_rpc.pal as pal_i
from pal_rpc.ttypes import *
import mirror_pd_rpc.mirror as mirror_i
from mirror_pd_rpc.ttypes import *
import conn_mgr_pd_rpc.conn_mgr as  conn_mgr_client_module
from ptf.thriftutils import hex_to_i16


TOFINO_INTERFACE = 'enp6s0'
CPU_PORT = 192
CPU_INGRESS_MIRROR_ID = 98
MC0_INGRESS_MIRROR_ID = 100


def parse_aruments():
    parser = argparse.ArgumentParser(description='Parse input arguments')
    parser.add_argument('-p', '--program', default='load_balance_dt_hw', help='Name of the program', required=True)
    parser.add_argument('-sw', '--simulation', dest='simulation', action='store_true', help='Run in simulator', default=False)
    parser.add_argument('-d', '--dynamic', dest='dynamic', help='Class name of dynamic controller')
    parser.add_argument('-a', '--optional', dest='optional', nargs='+', help='Optional arguments for dynamic controller')

    return parser.parse_args()

import importlib


def main():
    arguments = parse_aruments()
    print("Waiting for switch to be up")
    time.sleep(12)
    controller = StaticController(arguments.program)
    controller.bring_ports_up()
    controller.add_entries()
    sys.stdout.flush()
    if arguments.dynamic is not None:
        # eval('import run_dynamic from %s' % arguments.dynamic)
        module = importlib.import_module(arguments.dynamic)
        run_dynamic = getattr(module, 'run_dynamic')
        print("=== run_dynamic ===")
        run_dynamic(controller, arguments.optional)

    print("add_entries")

    # while True:
    #     time.sleep(1)
        # check_ip_counter = controller.readRegister('re_check_ip', 0)
        # for port_indx in range(0, 68):
        #     port_counter = controller.readRegister('re_port2count', port_indx)
        #     if port_counter != 0:
        #         print(port_indx, port_counter)
        # print("check_ip_counter: {}".format(check_ip_counter))
        # for i in range(0, 68):
        #     counter_ingress = controller.readRegister('ri_port2count', i)
        #     counter_egress = controller.readRegister('re_port2count', i)
        #     if counter_ingress != 0:
        #         print("port_indx {0} counter_ingress {1}".format(i, counter_ingress))
        #     if counter_egress != 0:
        #         print("port_indx {0} counter_egress {1}".format(i, counter_egress))
        # for i in range(0, 68):
        #     e0 = controller.readRegister('re_e02count', i)
        #     i0 = controller.readRegister('re_i02count', i)
        #     i1 = controller.readRegister('re_i12count', i)
        #     if e0 !=0:
        #         print("e0 {0}: {1}".format(i, e0))
        #     if i0 !=0:
        #         print("i0 {0}: {1}".format(i, i0))
        #     if i1 !=0:
        #         print("i1 {0}: {1}".format(i, i1))

def mirror_session_new(mir_type, mir_dir, sid, egr_port=0, egr_port_v=False,
                        egr_port_queue=0, packet_color=0, mcast_grp_a=0,
                        mcast_grp_a_v=False, mcast_grp_b=0, mcast_grp_b_v=False,
                        max_pkt_len=0, level1_mcast_hash=0, level2_mcast_hash=0,
                        cos=7, c2c=0, extract_len=0, timeout=0, int_hdr=[]):
    return MirrorSessionInfo_t(mir_type, mir_dir, sid,
                               egr_port, egr_port_v, egr_port_queue,
                               packet_color,
                               mcast_grp_a, mcast_grp_a_v,
                               mcast_grp_b, mcast_grp_b_v,
                               max_pkt_len,
                               level1_mcast_hash, level2_mcast_hash,
                               cos, c2c, extract_len, timeout,
                               int_hdr, len(int_hdr))

class StaticController:
    def __init__(self, program_name):
        self.program_name = program_name
        self.input_filename = program_name + '_rules.txt'
        self.output_filename = program_name + '_output_rules.txt'

        port_map_filename = home+"/FP4/control_plane/SUT/portMap.json"
        self.port_map = json.load(open(port_map_filename, "r"))

        self.transport = TTransport.TBufferedTransport(TSocket.TSocket('localhost', 9090))
        self.bprotocol = TBinaryProtocol.TBinaryProtocol(self.transport)
        self.transport.open()

        self.dp_client_module = importlib.import_module(program_name + ".p4_pd_rpc." +program_name)
        self.dp_ttypes = importlib.import_module(program_name + ".p4_pd_rpc.ttypes")
        self.reg_flags = eval("self.dp_ttypes." + program_name + "_register_flags_t(read_hw_sync=True)")
        self.p4_protocol = TMultiplexedProtocol.TMultiplexedProtocol(self.bprotocol, program_name)
        self.client = self.dp_client_module.Client(self.p4_protocol)

        self.conn_mgr_protocol = TMultiplexedProtocol.TMultiplexedProtocol(self.bprotocol, "conn_mgr")
        self.conn = conn_mgr_client_module.Client(self.conn_mgr_protocol)
        self.conn_hdl = self.conn.client_init()

        self.bw_dict = {"10G":pal_port_speed_t.BF_SPEED_10G,
                        "40G":pal_port_speed_t.BF_SPEED_40G,
                    "25G":pal_port_speed_t.BF_SPEED_25G,
                "100G":pal_port_speed_t.BF_SPEED_100G}
        self.fec_dict = {"NONE" :pal_fec_type_t.BF_FEC_TYP_NONE,
                         "FC" : pal_fec_type_t.BF_FEC_TYP_FIRECODE,
                 "RS" : pal_fec_type_t.BF_FEC_TYP_REED_SOLOMON}

        self.pal_protocol = TMultiplexedProtocol.TMultiplexedProtocol(self.bprotocol, "pal")
        self.pal = pal_i.Client(self.pal_protocol)

        self.mirror_protocol = TMultiplexedProtocol.TMultiplexedProtocol(self.bprotocol, "mirror")
        self.mirror = mirror_i.Client(self.mirror_protocol)
        self.dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))
        self.generate_output_rules()
        self.add_mirror_session_new()
        self.add_mirror_session_mc0()

        self.interace_name = TOFINO_INTERFACE

    def readRegister(self, registerName, index, pipeNum=None):
        val = eval("self.client.register_read_" + registerName + "(" \
            "self.conn_hdl, self.dev_tgt, " + str(index) +" , self.reg_flags)")
        self.conn.complete_operations(self.conn_hdl)

        if pipeNum is None:
            return max(val)
        else:
            return val[pipeNum]

    def bring_ports_up(self, rate="100G", fec="NONE"):
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

        # self.devPorts += devPorts
        return devPorts

    def add_mirror_session_new(self):
        info = mirror_session_new(MirrorType_e.PD_MIRROR_TYPE_NORM, Direction_e.PD_DIR_INGRESS, CPU_INGRESS_MIRROR_ID, CPU_PORT, True, max_pkt_len=128)
        self.mirror.mirror_session_create(self.conn_hdl, self.dev_tgt, info)
        self.conn.complete_operations(self.conn_hdl)

    def add_mirror_session_mc0(self):
        info = mirror_session_new(MirrorType_e.PD_MIRROR_TYPE_NORM, Direction_e.PD_DIR_INGRESS, MC0_INGRESS_MIRROR_ID, 0, True, max_pkt_len=128)
        self.mirror.mirror_session_create(self.conn_hdl, self.dev_tgt, info)
        self.conn.complete_operations(self.conn_hdl)

    def add_entries(self):
        print("=== add_entries via bfshell ===")
        command = "/home/leoyu/bf-sde-9.2.0/install/bin/bfshell -f " + self.output_filename
        print("Command:", command)
        process = subprocess.Popen(command.split(), stdout=subprocess.PIPE)
        output, error = process.communicate()
        print(output)
        print(error)
        return

    def generate_output_rules(self, ruleList = None):
        # port_rules = self.get_port_rules()
        if (ruleList is not None) and (len(ruleList) == 0):
            return

        output_file = open(self.output_filename, 'w')

        start_rule = "\npd-" + self.program_name.replace("_", "-") + "\n"
        # start_rule += "pd ti_count_ip add_entry ai_count_ip ipv4_dstAddr 50.0.0.1\n"
        # start_rule += "pd te_count_ip_start add_entry ae_count_ip_start ipv4_dstAddr 50.0.0.1\n"
        # start_rule += "pd te_count_ip add_entry ae_count_ip ipv4_dstAddr 50.0.0.1\n"
        # start_rule += "pd ti_set_zero add_entry ai_set_zero ig_intr_md_for_tm_ucast_egress_port 0\n"
        output_file.write(start_rule)

        if ruleList is None:
            input_rules_file = open(self.input_filename,'r')
            input_rules = input_rules_file.read()
            input_rules_file.close()
            output_file.write(input_rules)
        else:
            for rule in ruleList:
                output_file.write(rule + str("\n"))


        close_rules = "end\nexit\n"
        output_file.write(close_rules)

        output_file.close()

    def get_port_rules(self):
        rules = ['ucli', 'pm']
        interface_list = [str(x) + '/0' for x in range(9,25)]
        for interface in interface_list:
            rules.append('port-add ' + interface + " 100G NONE")
            rules.append('port-enb ' + interface)

        rules.append('exit')

        return '\n'.join(rules)




if __name__ == '__main__':
    main()
