#!/usr/bin/env python

import argparse
import os
import sys


parser = argparse.ArgumentParser()

parser.add_argument('--input_file', type=str, required=True)
parser.add_argument('--output_base', type=str, required=True)
parser.add_argument('--target', type=str, required=True)
parser.add_argument('--rules_path', type=str, required=False)

args = parser.parse_args()

print("=====instrument.py args=====")
print("input_file: "+args.input_file)
print("output_base: "+args.output_base)
print("target: "+args.target)
print(args.rules_path)
print("==========")

# TODO: integrate ILP planner
program2plan_json = {
    "basic_routing.p4": "/home/leoyu/FP4/instrumentation/ilp/basic_routing_plan.json",
    "dv_router.p4": "/home/leoyu/FP4/instrumentation/ilp/dv_router_plan.json",
    "Firewall.p4": "/home/leoyu/FP4/instrumentation/ilp/Firewall_plan.json",
    "LoadBalance.p4": "/home/leoyu/FP4/instrumentation/ilp/LoadBalance_plan.json",
    "mirror_clone.p4": "/home/leoyu/FP4/instrumentation/ilp/mirror_clone_plan.json",
    "netchain.p4": "/home/leoyu/FP4/instrumentation/ilp/netchain_plan.json",
    "RateLimiter.p4": "/home/leoyu/FP4/instrumentation/RateLimiter_plan.json"
}
plan_json = None
for program in program2plan_json.keys():
    if program in args.input_file:
        plan_json = program2plan_json[program]
        print("Plan path: {}".format(program2plan_json[program]))
if not plan_json:
    print("Empty plan.json")
    exit

# Bypass #include <.*> for preprocessor
skipped_includes_lines = []
# os.system("cp {0} {0}.c".format(args.input_file))
with open(args.input_file,'rb') as f, open(args.input_file+'.c','wb') as g:
    for line in f:
        if "#include <" in line or "#include<" in line or '#include "intrinsic_metadata.p4"' in line:
            skipped_includes_lines.append(line)
        else:
            g.write(line)

print("--- skipped_includes_lines ---")
print(skipped_includes_lines)
print("------")

os.system("g++ -fmax-errors=5 -E -P {0}.c >> {0}.tmp".format(args.input_file))

# Bring skipped_includes_lines back
with open(args.input_file+'.tmp', 'r+') as f:
   content = f.read()
   f.seek(0)
   for line in skipped_includes_lines:
       f.write(line)
   f.write(content)

command = "./frontend -v -i {0}.tmp -o {1} -t {2} -p {3}".format(
    args.input_file, args.output_base, args.target, plan_json)

if args.rules_path is not None:
    command += " -r " + args.rules_path



print(command)
# ./frontend -v -i /home/leoyu/FP4/test_programs/p4_14/basic_routing/basic_routing.p4.tmp -o sample_out/basic_routing -t hw -r /home/leoyu/FP4/test_programs/p4_14/basic_routing/hardware_rules.txt
os.system(command)


