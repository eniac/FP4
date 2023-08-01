#!/bin/bash
set -ex

files=("ut_hw/firewall_ut_hw.p4" 
       "ut_hw/load_balance_ut_hw.p4" 
       "ut_hw/rate_limiter_ut_hw.p4" 
       "ut_hw/basic_routing_ut_hw.p4" 
       "ut_hw/dv_router_ut_hw.p4" 
       "ut_hw/mirror_clone_ut_hw.p4" 
       "ut_hw/netchain_ut_hw.p4")

for file in "${files[@]}"; do
    if sudo -E ./compile_p4_14.sh "$file" 2>&1 | tee /dev/stderr | grep -q "error:"; then
        echo "Compilation failed: $file"
        exit 1
    fi
done
