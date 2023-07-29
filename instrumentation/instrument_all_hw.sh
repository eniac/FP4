rm -rf sample_out
rm -rf out
mkdir sample_out
mkdir out

set -ex

./instrument.sh -vv -t hw -r $HOME/FP4/test_programs/p4_14/firewall/hardware_rules.txt $HOME/FP4/test_programs/p4_14/firewall/firewall.p4
./instrument.sh -n -t hw -r $HOME/FP4/test_programs/p4_14/load_balance/hardware_rules.txt $HOME/FP4/test_programs/p4_14/load_balance/load_balance.p4
./instrument.sh -n -t hw -r $HOME/FP4/test_programs/p4_14/rate_limiter/hardware_rules.txt $HOME/FP4/test_programs/p4_14/rate_limiter/rate_limiter.p4
./instrument.sh -n -t hw -r $HOME/FP4/test_programs/p4_14/basic_routing/hardware_rules.txt $HOME/FP4/test_programs/p4_14/basic_routing/basic_routing.p4
./instrument.sh -n -t hw -r $HOME/FP4/test_programs/p4_14/dv_router/hardware_rules.txt $HOME/FP4/test_programs/p4_14/dv_router/dv_router.p4
./instrument.sh -n -t hw -r $HOME/FP4/test_programs/p4_14/mirror_clone/hardware_rules.txt $HOME/FP4/test_programs/p4_14/mirror_clone/mirror_clone.p4
./instrument.sh -n -t hw -r $HOME/FP4/test_programs/p4_14/netchain/hardware_rules.txt $HOME/FP4/test_programs/p4_14/netchain/netchain.p4

cp sample_out/*.p4 $HOME/FP4/hardware_run/
cp sample_out/*_ut_hw_rules.txt $HOME/FP4/control_plane/SUT/
cp sample_out/*_ut_hw_rules.txt $HOME/FP4/control_plane/SDT/
cp out/*.json $HOME/FP4/control_plane/SDT/
