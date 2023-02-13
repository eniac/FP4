#! /bin/bash

set -e
set -euo pipefail

output_path=sample_out/
verbose=0
target=hw
rebuild=1
rules_path=""

usage() { echo "Usage: $0 [-vv] [-t target[hw/sim]] [-n] [-o output_dir] [-r rules] input_file" 1>&2; exit 1; }
while getopts ":o:t:r:nv" opt
do
  case "${opt}" in
    o ) output_path=${OPTARG};;
    t ) target=${OPTARG};;
    r ) rules_path=${OPTARG};;
    v ) verbose=$(($verbose + 1));;
    n ) rebuild=0;;
    \? ) usage;;
  esac
done
shift $((OPTIND-1))

if [ $# -lt 1 ]; then
  usage
fi

echo "Rule file path: ${rules_path}, inputfile: $1"

# Parse input file
input_file=$1
infile_prefix=$(basename -- "$input_file")
infile_prefix="${infile_prefix%.*}"

# Parse output director.  Ensures that output_path ends with a slash
[[ "${output_path}" != */ ]] && output_path="${output_path}/"
mkdir -p ${output_path}

output_base="${output_path}${infile_prefix}"

set -x

if [ $rebuild -eq 1 ]; then
  echo "====== Rebuild frontends ======"
  rm -rf build && mkdir build && cd build

  # Parse verbosity
  if [ $verbose -ge 2 ]; then
    echo "=== cmake with VERBOSE ==="
    cmake .. -DVERBOSE=true
  elif [ $verbose -eq 1 ]; then
    echo "=== cmake with INFO ==="
    cmake .. -DINFO=true
  else
    cmake  .. 
  fi
  make -j4
  cd ..
fi

{ echo "infile_prefix: ${infile_prefix}"; } 2> /dev/null
{ echo "Output base: ${output_base}"; } 2> /dev/null

# Avoid staleness
if [ -f "${input_file}.c" ]; then
    rm ${input_file}.c
fi
if [ -f "${input_file}.tmp" ]; then
    rm ${input_file}.tmp
fi

if [ -z "$rules_path" ]
then
  echo "rules_path NULL" 
  python2 instrument.py --input_file ${input_file} --output_base ${output_base} --target ${target}
else 
  echo "rules_path not NULL: ${rules_path}" 
  python2 instrument.py --input_file ${input_file} --output_base ${output_base} --target ${target} --rules_path ${rules_path}
fi
