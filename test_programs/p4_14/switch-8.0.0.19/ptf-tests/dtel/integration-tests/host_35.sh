#!/bin/bash

sudo arp -i ens1f1 -s 10.32.1.1 aa:aa:aa:aa:aa:44
sudo arp -i ens1f0 -s 10.32.2.1 aa:aa:aa:aa:aa:44
sudo arp -i ens2f0 -s 10.31.3.1 aa:aa:aa:aa:aa:43
sudo arp -i ens2f1 -s 10.33.1.1 aa:aa:aa:aa:aa:45

sudo ip route add 10.33.5.0/24 via 10.32.1.1
sudo ip route add 10.33.6.0/24 via 10.32.2.1
sudo ip route add 10.31.3.0/24 via 10.32.2.1
sudo ip route add 10.32.7.0/24 via 10.32.2.1
