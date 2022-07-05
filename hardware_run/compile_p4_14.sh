#! /bin/bash

if [ "$EUID" -ne 0 ]
    then echo "Please run as root"
    exit
fi

set -ex

export SDE="${HOME}/bf-sde-9.2.0"
export P4C="${HOME}/bf-sde-9.2.0/install/bin/bf-p4c"
export CURR=$(pwd)
export SDE_BUILD=$SDE"/build"
export SDE_INSTALL=$SDE"/install"
# Not applied to sudo
export PATH=$SDE_INSTALL/bin:$PATH

echo "Compile "$1
prog_name=$(basename $1 .p4)

DEFAULT_ARGS="--std p4_14 -g --verbose 3 --create-graphs --display-power-budget --arch tna --target tofino"
WITH_BFRT="--bf-rt-schema custom_build/$name.tofino/bfrt.json"
# (binary, json)
WITH_P4RT="--p4runtime-file custom_build/$name.tofino/p4info.proto.txt --p4runtime-format text"
P4_BUILD=$SDE_BUILD"/myprog"

check_stats() {
    cat<<EOF
===============     
Number of CPUs: $(nproc)
Distribution: $(uname -a)
Linux Arch: $(uname -m)
=============== 
EOF
}

check_env() {
    if [ -z $SDE ]; then
        echo "ERROR: SDE Environment variable is not set"
        exit 1
    else 
        echo "Using SDE ${SDE}"
    fi

    if [ ! -d $SDE ]; then
        echo "  ERROR: \$SDE ($SDE) is not a directory"
        exit 1
    fi

    return 0
}

build_p4_14() {

    echo "Building $1 in build_dir $P4_BUILD ... "
   
    # p4c-tofino is old and doesn't support p4-16, also only v1Model supported
    # bf-p4c is new and support p4-14, p4-16; support TNA, V1Model etc
    # Better use the p4-build infrastructure, bf-p4c only deals with DP
    # bf-p4c $DEFAULT_ARGS -o $P4_BUILD"/"$prog_name.tofino ${CURR}/$1 

    # --auto-init-metadata

    # bf-p4c by default, no need for --with-p4c
    cd $SDE/pkgsrc/p4-build
    ./configure \
    --prefix=$SDE_INSTALL --enable-thrift --with-tofino --with-p4c=p4c \
    P4_NAME=$prog_name P4_PATH=$CURR/$1 P4_VERSION=p4-14  P4_ARCHITECTURE=tna &&
    make clean && make && make install &&
    # Retain the custom modifications
    # make && make install &&

    echo "Install conf needed by the driver ... "
    CONF_IN=${SDE}"/pkgsrc/p4-examples/tofino/tofino_single_device.conf.in"
    # P4_16
    # CONF_IN=$SDE_PKGSRC/${p4_examples}/tofino/tofino_single_device_bfrt.conf.in
    if [ ! -f $CONF_IN ]; then
        cat <<EOF
ERROR: Template config.in file `$CONF_IN` missing.
EOF
        return 1
    fi

    CONF_OUT_DIR=${SDE_INSTALL}/share/p4/targets/tofino
    sed -e "s/TOFINO_SINGLE_DEVICE/${prog_name}/"  \
        $CONF_IN                                 \
        > ${CONF_OUT_DIR}/${prog_name}.conf

    return 0
}

check_stats
check_env
build_p4_14 "$@"

# Fix the import non-package bug
#sudo sed -i s/from\ .ttypes\ import/from\ ttypes\ import/ ${SDE}/install/lib/python2.7/site-packages/tofinopd/${prog_name}/p4_pd_rpc/${prog_name}.py

cd ${CURR}
exit 0
