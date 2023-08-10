#!/bin/bash
set -ex

files=(
    "firewall_p4testgen.p4" 
    "load_balance_p4testgen.p4" 
    "rate_limiter_p4testgen.p4" 
    "basic_routing_p4testgen.p4" 
    "dv_router_p4testgen.p4" 
    "mirror_clone_p4testgen.p4" 
    "netchain_p4testgen.p4"
    "mac_learning_p4testgen.p4"
    "dos_defense_p4testgen.p4"
    "cheetah_groupby_p4testgen.p4" 
    )

for file in "${files[@]}"; do
    if sudo -E ./compile_p4_14.sh "$file" 2>&1 | tee /dev/stderr | grep -q "error:"; then
        echo "Compilation failed: $file"
        exit 1
    fi
done
