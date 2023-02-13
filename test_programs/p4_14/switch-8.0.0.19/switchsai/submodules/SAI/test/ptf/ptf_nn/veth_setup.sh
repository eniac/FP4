#!/bin/bash
for idx in 0 1; do
    intf0="veth$(($idx*2))"
    intf1="veth$(($idx*2+1))"
    if ! ip link show $intf0 &> /dev/null; then
        ip link add name $intf0 type veth peer name $intf1
        ip link set dev $intf0 up
        ip link set dev $intf1 up
    fi
    sysctl net.ipv6.conf.$intf0.disable_ipv6=1
    sysctl net.ipv6.conf.$intf1.disable_ipv6=1
done
