#! /bin/bash

if [[ $EUID -eq 0 ]]; then
   echo "This script must NOT be run as root" 
   exit 1
fi

set -ex

start=$(date +%s)

program_list=(
   # "basic_routing"
   # "firewall"
   # "load_balance"
   # "rate_limiter"
   # "dv_router"
   # "mac_learning"
   # "mirror_clone"
   "cheetah_groupby"
   # "dos_defense"
   # "netchain"
   # "switch_80019"
)

for program in "${program_list[@]}"
do
    cp compile_p4_14.sh $HOME/FP4/test_programs/p4_14/$program
    cd $HOME/FP4/test_programs/p4_14/$program
    rm *.c || echo "*.c not found"
    sudo -E ./compile_p4_14.sh $program.p4
    rm compile_p4_14.sh || echo "compile_p4_14.sh not found"
    cd -
done

end=$(date +%s)
difference=$(( end - start ))
echo "Time taken is $difference seconds."
