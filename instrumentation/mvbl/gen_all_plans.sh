#! /bin/bash

set -ex

SDE_PREFIX=/home/leoyu/bf-sde-9.2.0/pkgsrc/p4-build/tofino

program_list=(
   "basic_routing"
   # "firewall"
   # "load_balance"
   # "rate_limiter"
   # "basic_routing_old"
   # "dv_router"
   # "mirror_clone"
   # "netchain"
)

context_dot_cache_dir="context_dot_cache"

rm -rf $context_dot_cache_dir
mkdir $context_dot_cache_dir

for program in "${program_list[@]}"
do
    mkdir $context_dot_cache_dir/$program
    cp $SDE_PREFIX/$program/graphs/ingress.dot $context_dot_cache_dir/$program
    cp $SDE_PREFIX/$program/graphs/egress.dot $context_dot_cache_dir/$program
    cp $SDE_PREFIX/$program/context.json $context_dot_cache_dir/$program
    dot -Tsvg $context_dot_cache_dir/$program/ingress.dot > $context_dot_cache_dir/$program/ingress.svg
    dot -Tsvg $context_dot_cache_dir/$program/egress.dot > $context_dot_cache_dir/$program/egress.svg
    python3 gen_mvbl_plan.py -d $SDE_PREFIX/$program/graphs/ingress.dot -c $SDE_PREFIX/$program/context.json
done

# python3 gen_mvbl_plan.py -d test_examples/switch_80019/ingress.dot -c test_examples/switch_80019/context.json
# python3 gen_mvbl_plan.py -d test_examples/basic_routing/ingress.dot -c test_examples/basic_routing/context.json