#!/bin/bash

sudo arp -i enp4s0f0 -s 10.32.2.1 00:00:10:32:02:02
sudo arp -i enp4s0f1 -s 10.31.3.1 00:00:10:31:03:02
sudo arp -i enp5s0f0 -s 10.32.1.1 00:00:10:32:01:02
sudo arp -i enp5s0f1 -s 10.33.1.1 00:00:10:33:01:02

sudo ip route add 10.32.7.0/24 via 10.32.1.1
sudo ip route add 10.31.9.0/24 via 10.32.1.1
sudo ip route add 10.33.5.0/24 via 10.32.1.1
sudo ip route add 10.33.6.0/24 via 10.32.1.1
sudo ip route add 10.33.10.0/24 via 10.32.1.1
sudo ip route add 10.33.9.0/24 via 10.32.1.1
