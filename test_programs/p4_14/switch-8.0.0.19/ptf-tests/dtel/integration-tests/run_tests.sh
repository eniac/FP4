THIS_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
WORKSPACE=$( cd $THIS_DIR; cd ../../../../.. && pwd )

IP="10.201.124.31"
test="${1}"
case ${test} in
    leafA)      IP="10.201.124.32"
                ;;
    leafB)      IP="10.201.124.33"
                ;;
    leafA_loop) IP="10.201.124.32"
                ;;
    leafA_2)    IP="10.201.124.44"
                ;;
    leafB_2)    IP="10.201.124.45"
                ;;
    spine_2)    IP="10.201.124.43"
                ;;
esac

set +e
cd $WORKSPACE
./tools/run_p4_tests.sh -p switch --arch Tofino --target hw --no-veth \
    --thrift-server $IP  \
    -f submodules/switch/ptf-tests/dtel/integration-tests/eth_port.json \
    -t submodules/switch/ptf-tests/dtel/integration-tests/ \
    -s $test
