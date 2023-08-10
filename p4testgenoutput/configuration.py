import argparse

parser = argparse.ArgumentParser(description='Parse input arguments')
parser.add_argument('-p', '--program', default='LoadBalance_dt_hw', help='Name of the program', required=True)
parser.add_argument('-sw', '--simulation', dest='simulation', action='store_true', help='Run in simulator', default=False)
parser.add_argument('-m', '--mode',  choices=['normal', 'no_seed', 'no_analysis', 'no_seed_analysis'], default='normal', help='Select the mode')
parser.add_argument('-r', '--rulesFile', dest='rulesFile', help='Name of rules file')
parser.add_argument('-c', '--cip', dest='cip', help='IP of port to receive packets from SDT control-port')


args = parser.parse_args()

global PROGRAM

PROGRAM = args.program

SIMULATION = args.simulation
CPU_INGRESS_MIRROR_ID = 98
CPU_EGRESS_MIRROR_ID = 99

if SIMULATION:
    CPU_PORT = 31
    TOFINO_INTERFACE = 's2-ethc'
else:
    CPU_PORT = 192
    # LC_TODO: should be adaptive based on basename /sys/module/bf_kpkt/drivers/pci\:bf/*/net/*
    TOFINO_INTERFACE = 'enp6s0'

FLOOD_GID_START = 1000
RULES_FILE = args.rulesFile
MODE = args.mode
C_IP = args.cip

print 'PROGRAM: ' + str(PROGRAM)
print 'SIMULATION: ' + str(SIMULATION)
print 'CPU_PORT: ' + str(CPU_PORT)
print 'TOFINO_INTERFACE: ' + str(TOFINO_INTERFACE)
print 'RULES file: ' + str(RULES_FILE)
print 'Mode: ' + str(MODE)
print 'C_IP: ' + str(C_IP)

# from covDet.headerSynthesizer import HeaderSynthesizer
# HeaderSynthesizer(PROGRAM.split("_dt_hw")[0] + "_headerFile.json")

# from header import Header
import sys
from os.path import expanduser
home = expanduser("~")
sys.path.append(home + "/bf-sde-9.2.0/install/lib/python2.7/site-packages/tofino")
sys.path.append(home + "/bf-sde-9.2.0/install/lib/python2.7/site-packages")
sys.path.append(home + "/bf-sde-9.2.0/install/lib/python2.7/site-packages/tofinopd")

exec('from %s.p4_pd_rpc.ttypes import *' % PROGRAM)
exec('from %s.p4_pd_rpc.%s import *' % (PROGRAM, PROGRAM))

