#!/bin/bash

sudo arp -i enp4s0f0 -s 10.32.7.1 00:00:10:32:07:02
sudo arp -i enp4s0f1 -s 10.31.9.1 00:00:10:31:09:02
sudo arp -i enp5s0f0 -s 10.33.6.1 00:00:10:33:06:02
sudo arp -i enp5s0f1 -s 10.33.5.1 00:00:10:33:05:02

sudo ip route add 10.32.1.0/24 via 10.33.5.1
sudo ip route add 10.32.2.0/24 via 10.33.5.1
sudo ip route add 10.31.3.0/24 via 10.33.5.1
sudo ip route add 10.33.1.0/24 via 10.33.5.1
sudo ip route add 10.33.10.0/24 via 10.33.5.1
sudo ip route add 10.33.9.0/24 via 10.33.5.1
