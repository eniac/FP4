#! /bin/bash

set -ex

sudo -E ./compile_p4_14.sh ut_hw/firewall_ut_hw.p4
sudo -E ./compile_p4_14.sh ut_hw/load_balance_ut_hw.p4
sudo -E ./compile_p4_14.sh ut_hw/rate_limiter_ut_hw.p4
sudo -E ./compile_p4_14.sh ut_hw/basic_routing_ut_hw.p4
sudo -E ./compile_p4_14.sh ut_hw/dv_router_ut_hw.p4
sudo -E ./compile_p4_14.sh ut_hw/mirror_clone_ut_hw.p4
sudo -E ./compile_p4_14.sh ut_hw/netchain_ut_hw.p4
