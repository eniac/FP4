#! /bin/bash

rm -rf sample_out
rm -rf out
mkdir sample_out
mkdir out

set -ex

#!/bin/bash

programs=(
    "firewall"
    "load_balance"
    "rate_limiter"
    "basic_routing"
    "dv_router"
    "mirror_clone"
    "netchain"
)

for prog in "${programs[@]}"; do
  if [ "$prog" == "firewall" ]; then
    ./instrument.sh -vv -t hw -r $HOME/FP4/test_programs/p4_14/$prog/hardware_rules.txt -i $HOME/FP4/instrumentation/mvbl/plan/${prog}_ingress.json -e $HOME/FP4/instrumentation/mvbl/plan/${prog}_egress.json $HOME/FP4/test_programs/p4_14/$prog/$prog.p4
  else
    ./instrument.sh -n -t hw -r $HOME/FP4/test_programs/p4_14/$prog/hardware_rules.txt -i $HOME/FP4/instrumentation/mvbl/plan/${prog}_ingress.json -e $HOME/FP4/instrumentation/mvbl/plan/${prog}_egress.json $HOME/FP4/test_programs/p4_14/$prog/$prog.p4
  fi
done

mkdir $HOME/FP4/hardware_run/ut_hw/ || echo "Folder exists"
mkdir $HOME/FP4/hardware_run/dt_hw/ || echo "Folder exists"
cp sample_out/*ut_hw.p4 $HOME/FP4/hardware_run/ut_hw/
cp sample_out/*dt_hw.p4 $HOME/FP4/hardware_run/dt_hw/
cp sample_out/*_ut_hw_rules.txt $HOME/FP4/control_plane/SUT/
cp sample_out/*_ut_hw_rules.txt $HOME/FP4/control_plane/SDT/
cp out/*.json $HOME/FP4/control_plane/SDT/
