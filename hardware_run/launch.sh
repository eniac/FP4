#! /bin/bash

export SDE=$HOME/bf-sde-9.2.0
set -e


if [ $# -lt 2 ];
then
    echo "Usage: $(basename ""$0"") <p4prog> <config.ini path>"
    exit 1
fi

PROG=switchd
# -Werror
sudo rm ${PROG} || echo "Empty"
gcc -I${SDE}/install/include/ -Wno-implicit-function-declaration -Wno-missing-field-initializers -g -O2 -std=c99 -L${SDE}/install/lib/ -o ${PROG} ${PROG}.c -ldriver -lbfsys -lbfutils -lbf_switchd_lib -lm -ldl -lpthread -lpython3.4m -lavago

export SDE_INSTALL=$SDE"/install"
export LD_LIBRARY_PATH=${SDE}/install/lib:$LD_LIBRARY_PATH
echo $LD_LIBRARY_PATH
sudo env LD_LIBRARY_PATH=$LD_LIBRARY_PATH ./${PROG} $SDE_INSTALL $SDE_INSTALL/share/p4/targets/tofino/$1.conf $2

