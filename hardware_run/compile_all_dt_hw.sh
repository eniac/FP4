#!/bin/bash
set -ex

files=("dt_hw/firewall_dt_hw.p4" 
       "dt_hw/load_balance_dt_hw.p4" 
       "dt_hw/rate_limiter_dt_hw.p4" 
       "dt_hw/basic_routing_dt_hw.p4" 
       "dt_hw/dv_router_dt_hw.p4" 
       "dt_hw/mirror_clone_dt_hw.p4" 
       "dt_hw/netchain_dt_hw.p4")

for file in "${files[@]}"; do
    if sudo -E ./compile_p4_14.sh "$file" 2>&1 | tee /dev/stderr | grep -q "error:"; then
        echo "Compilation failed: $file"
        exit 1
    fi
done
