#!/bin/bash
# increase file descriptor limit: https://underyx.me/2015/05/18/raising-the-maximum-number-of-file-descriptors
# echo 55000 | sudo tee /sys/fs/cgroup/pids/user.slice/user-1000.slice/pids.max

# disable offloading
#sudo ethtool -K ens1f1 rx off
#sudo ethtool -K ens1f1 tx off
#sudo ethtool -K ens1f1 tso off
#sudo ethtool -K ens1f1 gso off
#sudo ethtool -K ens1f1 gro off
for port in `seq 10000 11000`;
do
	sudo nc -d -l $port -I 2097152 > /dev/null &
done
