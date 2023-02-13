#!/bin/bash
# increase file descriptor limit: https://underyx.me/2015/05/18/raising-the-maximum-number-of-file-descriptors
# echo 55000 | sudo tee /sys/fs/cgroup/pids/user.slice/user-1000.slice/pids.max

# disable offloading
#sudo ethtool -K ens1f1 rx off
#sudo ethtool -K ens1f1 tx off
#sudo ethtool -K ens1f1 tso off
#sudo ethtool -K ens1f1 gso off
#sudo ethtool -K ens1f1 gro off
ip=10.33.5.2
len=60
pktnum=5

for port in `seq 10000 11000`;
do
	./delaytext.sh $len $pktnum | sudo nc $ip $port -O 2097152  &
	sleep 0.0$[ ( $RANDOM % 10 )  + 1 ]
done

for job in `jobs -p`
do
    wait $job
done
