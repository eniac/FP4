#! /bin/bash

set -ex

sudo -E ./compile_p4_14.sh firewall_ut_hw.p4
sudo -E ./compile_p4_14.sh load_balance_ut_hw.p4
sudo -E ./compile_p4_14.sh rate_limiter_ut_hw.p4
sudo -E ./compile_p4_14.sh basic_routing_ut_hw.p4
sudo -E ./compile_p4_14.sh dv_router_ut_hw.p4
sudo -E ./compile_p4_14.sh mirror_clone_ut_hw.p4
sudo -E ./compile_p4_14.sh netchain_ut_hw.p4
