#!/bin/bash

# Copy this file along with the minigraph file, port configuration file
# and SDK on the switch and then run with sudo. You can re-run the script
# without any issues. Not setting one of the filenames will skip the according
# step.

MINIGRAPH_FILE=minigraph.xml
PORT_CONFIG_FILE=port_config.ini
SDK=install.tgz

# Run as root
if [ $EUID != 0 ]; then
    sudo "$0" "$@"
    exit $?
fi

HWSKU=`show platform summary | grep HwSKU: | cut -d ' ' -f 2`
echo "This is a" $HWSKU "machine"

# minigraph
if [ -f $MINIGRAPH_FILE ]; then
  echo "Setting minigraph.xml"
  cp $MINIGRAPH_FILE /etc/sonic/minigraph.xml
else
  echo "Skiping minigraph"
fi

# port_config.ini
if [ -f $PORT_CONFIG_FILE ]; then
  echo "Setting port_config.ini"
  if [ $HWSKU = "mavericks" ]; then
    cp $PORT_CONFIG_FILE /usr/share/sonic/device/x86_64-accton_wedge100bf_65x-r0/mavericks/port_config.ini
  elif [ $HWSKU = "montara" ]; then
    sed -i '/Ethernet132/,$d' $PORT_CONFIG_FILE
    cp $PORT_CONFIG_FILE /usr/share/sonic/device/x86_64-accton_wedge100bf_32x-r0/montara/port_config.ini
  else
    echo "Unknown HWSKU"
    exit 1
  fi
else
  "Skipping port_config.ini"
fi

# SDK
if [ -f $SDK ]; then
  echo 'Setting up SDK'
  docker cp $SDK syncd:/opt/bfn/install.tgz
  docker exec -ti syncd bash -c 'tar -zxvf /opt/bfn/install.tgz -C /opt/bfn/ > /dev/null'
fi

# BGP
# Check if already done
docker exec -ti bgp bash -c 'grep -q redistribute /usr/share/sonic/templates/bgpd.conf.j2'
if [ $? -eq 1 ]; then
  echo 'Editing BGP configuration'
  docker exec -ti bgp bash -c "sed -i 's/{% endblock bgp_peers_with_range %}/{% endblock bgp_peers_with_range %}\nredistribute connected/' /usr/share/sonic/templates/bgpd.conf.j2"
else
  echo 'BGP already configured'
fi

# redis
docker exec -ti database bash -c "grep -q 'protected-mode no' /etc/redis/redis.conf"
if [ $? -eq 1 ]; then
  echo 'Editing Redis configuration'
  docker exec -ti database bash -c "sed -i 's/bind 127.0.0.1/# bind 127.0.0.1/' /etc/redis/redis.conf"
  docker exec -ti database bash -c "sed -i 's/protected-mode yes/protected-mode no/' /etc/redis/redis.conf"
else
  echo 'Redis already configured'
fi

config load_minigraph -y
config save -y

echo "All operations complete"
echo "Please reboot the machine"
