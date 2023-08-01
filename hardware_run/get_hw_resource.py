#!/usr/bin/python

import os
import sys
# import subprocess
# from subprocess import run
import argparse
import json

prog_names = [
        "firewall",
        "load_balance",
        "rate_limiter",
        "basic_routing",
        "dv_router",
        "mirror_clone",
        "netchain",
        "switch_80019"
        ]

prefix="/home/leoyu/bf-sde-9.2.0/pkgsrc/p4-build/tofino/"
suffix="/logs/"
ut_suffix="_ut_hw"
dt_suffix="_dt_hw"

def main():

    d = "."
    projects = [os.path.join(d, o) for o in os.listdir(d) 
                    if os.path.isdir(os.path.join(d,o))]
    metrics = ["Program", "Stages", "Tables", "sALUs", "SRAM (KB)", "TCAM (KB)", "Metadata (b)"]

    tbl_header = " & ".join(metrics) + " \\\\"
    print (tbl_header)
    if sys.argv[1] == "SUT":
        print("=== SUT before instrumentation ===")
        for index in range(len(prog_names)):
    	    printMetric(prog_names[index], metrics)
        print("=== SUT after instrumentation ===")
        for index in range(len(prog_names)):
    	    printMetric(prog_names[index]+ut_suffix, metrics)
    elif sys.argv[1] == "SDT":
        print("=== SDT ===")
        for index in range(len(prog_names)):
    	    printMetric(prog_names[index]+dt_suffix, metrics)
    else:
        print("Unknown arg")

def printMetric(prog_name, metrics):
    log_dir = prefix+prog_name+suffix

    resource_dict = getResourceUsage(log_dir)
    resource_dict["Program"] = prog_name
    stats = [str(resource_dict[f]) for f in metrics]
    line = " & ".join(stats) + " \\\\"
    print (line)
    return resource_dict

def getLoc(src_fn):
	# cmd= """ cat {0} | grep -v "^ *$" | grep -v "^ *//" | grep -v "^ */\*.*\*/" | wc -l""".format(src_fn)
	# res = run(cmd, shell=True, stdout=subprocess.PIPE)
	# return int(res.stdout)
	ret = 0
	with open(src_fn, "r") as read_file:
		lines = read_file.readlines()
		for line in lines:
			if "//" not in line:
				ret += 1
	return ret


def getResourceUsage(log_dir):
	sram_size = 16 # in KB
	tcam_size = 1.28 # in KB
	metrics = json.load(open(log_dir + "metrics.json", "r"))
	resources = json.load(open(log_dir + "resources.json", "r"))
	ret_dict = {}
	# logical tables
	ret_dict["Tables"] = int(metrics["mau"]["logical_tables"])	
	# SRAM (KB)
	ret_dict["SRAM (KB)"] = int(metrics["mau"]["srams"])*sram_size
	# TCAM (KB)
	ret_dict["TCAM (KB)"] = int(metrics["mau"]["tcams"])*tcam_size
	# stages
	ret_dict["Stages"] = len(resources["resources"]["pipes"][0]["mau"]["mau_stages"])
	# meter ALUs (sALUs)
	meter_ct = 0
	for stage in resources["resources"]["pipes"][0]["mau"]["mau_stages"]:
		meter_ct += len(stage["meter_alus"]["meters"])
	ret_dict["sALUs"] = meter_ct
	# metadata (bits)
	phv_util = 0
	for phv_type in metrics["phv"]["normal"]:
		phv_util += int(phv_type["bits_occupied"])
	for phv_type in metrics["phv"]["tagalong"]:
		phv_util += int(phv_type["bits_occupied"])
	ret_dict["Metadata (b)"] = phv_util
	return ret_dict


if __name__ == '__main__':
	main()

