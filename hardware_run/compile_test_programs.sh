#! /bin/bash

if [[ $EUID -eq 0 ]]; then
   echo "This script must NOT be run as root" 
   exit 1
fi

cp compile_p4_14.sh $HOME/FP4/test_programs/p4_14/Firewall
cd $HOME/FP4/test_programs/p4_14/Firewall
rm *.c
sudo -E ./compile_p4_14.sh Firewall.p4
rm compile_p4_14.sh
cd -

cp compile_p4_14.sh $HOME/FP4/test_programs/p4_14/LoadBalance
cd $HOME/FP4/test_programs/p4_14/LoadBalance
rm *.c
sudo -E ./compile_p4_14.sh LoadBalance.p4
rm compile_p4_14.sh
cd -

cp compile_p4_14.sh $HOME/FP4/test_programs/p4_14/RateLimiter
cd $HOME/FP4/test_programs/p4_14/RateLimiter
rm *.c
sudo -E ./compile_p4_14.sh RateLimiter.p4
rm compile_p4_14.sh
cd -

cp compile_p4_14.sh $HOME/FP4/test_programs/p4_14/basic_routing
cd $HOME/FP4/test_programs/p4_14/basic_routing
rm *.c
sudo -E ./compile_p4_14.sh basic_routing.p4
rm compile_p4_14.sh
cd -

cp compile_p4_14.sh $HOME/FP4/test_programs/p4_14/dv_router
cd $HOME/FP4/test_programs/p4_14/dv_router
rm *.c
sudo -E ./compile_p4_14.sh dv_router.p4
rm compile_p4_14.sh
cd -

cp compile_p4_14.sh $HOME/FP4/test_programs/p4_14/mirror_clone
cd $HOME/FP4/test_programs/p4_14/mirror_clone
rm *.c
sudo -E ./compile_p4_14.sh mirror_clone.p4
rm compile_p4_14.sh
cd -

cp compile_p4_14.sh $HOME/FP4/test_programs/p4_14/netchain
cd $HOME/FP4/test_programs/p4_14/netchain
rm *.c
sudo -E ./compile_p4_14.sh netchain.p4
rm compile_p4_14.sh
cd -
