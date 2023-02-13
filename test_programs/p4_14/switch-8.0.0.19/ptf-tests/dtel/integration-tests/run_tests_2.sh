THIS_DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
WORKSPACE=$( cd $THIS_DIR; cd ../../../../.. && pwd )

IP="10.201.124.43"
test="${1}"
case ${test} in
    leafA_2)      IP="10.201.124.44"
                ;;
    leafB_2)      IP="10.201.124.45"
                ;;
    leafA_loop_2) IP="10.201.124.44"
                ;;
esac

cd $WORKSPACE
./tools/run_p4_tests.sh -p switch --arch Tofino --target hw --no-veth \
    --thrift-server $IP  \
    -f submodules/switch/ptf-tests/dtel/integration-tests/eth_port_2.json \
    -t submodules/switch/ptf-tests/dtel/integration-tests/ \
    -s $test
