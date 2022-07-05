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

command = "./frontend -v -i {0}.tmp -o {1} -t {2}".format(args.input_file, args.output_base, args.target)

if args.rules_path is not None:
    command += " -r " + args.rules_path

os.system(command)


