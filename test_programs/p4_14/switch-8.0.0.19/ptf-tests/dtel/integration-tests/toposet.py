import json
import sys
from collections import OrderedDict

def main():
    json_filename = 'topo1.json'

    with open(json_filename) as json_file:
        json_str = json_file.read()
    json_data = json.loads(json_str, object_pairs_hook=OrderedDict)


    for machine in json_data:
        if machine["type"] == "switch":
            # pm_config_str will contain the commands we need to run in BF shell
            pm_config_str = "pm port-del -/-\n"
            py_import_str = ""
            # Define switch MAC
            if "mac_self" not in machine:
                machine["mac_self"] = "aa:aa:aa:aa:aa:0" + str(machine["role"])

            # Define switch ID
            if "switch_id" not in machine:
                machine["switch_id"] = machine["role"]
            py_import_str += "switch_id = " + machine["switch_id"] + "\n"
            py_import_str += "mac_self = \"" + machine["mac_self"] + "\"\n"
            py_import_str += "nports = " + str(len(machine["ports"])) + "\n"

            imported_port_index = []
            imported_swports = []
            imported_frontports = []
            imported_ipaddr_inf = []
            imported_ipaddr_nbr = []
            imported_mac_nbr = []
            # Process ports
            for i, port in enumerate(machine["ports"]):
                port["name"] = str(port["name"])
                # Set IP
                if "ip_address" not in port:
                    # If the port is connected to a host
                    if "connected_to_host_role" in port:
                        port["ip_address"] = "10.1" \
                                             + str(machine["role"]) \
                                             + str(port["connected_to_host_role"]) \
                                             + "." \
                                             + str(port["connected_to_interface_role"]) \
                                             + ".1"

                    elif "connected_to_switch_role" in port:
                        if int(machine["role"]) > int(port["connected_to_switch_role"]):
                            port["ip_address"] = "10.2" \
                                                 + str(port["connected_to_switch_role"]) \
                                                 + str(machine["role"]) \
                                                 + "." \
                                                 + str(port["connected_to_interface_role"]) \
                                                 + ".2"
                        else:
                            port["ip_address"] = "10.2" \
                                                 + str(machine["role"]) \
                                                 + str(port["connected_to_switch_role"]) \
                                                 + "." \
                                                 + port["connected_to_interface_role"] \
                                                 + ".1"

                # Neighbor IP address
                if "neighbor_ip_address" not in port:
                    if port["ip_address"][-1] == "1":
                        port["neighbor_ip_address"] = port["ip_address"][:-1] + "2"
                    elif port["ip_address"][-1] == "2":
                        port["neighbor_ip_address"] = port["ip_address"][:-1] + "1"
                    else:
                        print "Cannot infer neighbor IP"
                        sys.exit()

                # Neighbor MAC address
                if "neighbor_mac_address" not in port:
                    if "connected_to_switch_role" in port:
                        port["neighbor_mac_address"] = "aa:aa:aa:aa:aa:0" + str(port["connected_to_switch_role"])
                    else:
                        neighbor_ip_fields = port["neighbor_ip_address"].split(".")
                        neighbor_mac_fields = [':{:0>2}'.format(int(x) % 100) for x in neighbor_ip_fields]
                        port["neighbor_mac_address"] = "00:00" + "".join(neighbor_mac_fields)
                if "sw_port_number" not in port:
                    port_number = port["name"].split("/")
                    if port_number[1] == "-":
                        port_number[1] = "0"
                    port["sw_port_number"] = (int(port_number[0]) - 1) * 4 + int(port_number[1])


                # Append calculated fields
                imported_port_index.append(i)
                imported_frontports.append(port["name"])
                imported_swports.append(port["sw_port_number"])
                imported_ipaddr_inf.append(port["ip_address"])
                imported_ipaddr_nbr.append(port["neighbor_ip_address"])
                imported_mac_nbr.append(port["neighbor_mac_address"])



                # Port manager configuration for the port
                pm_config_str += "pm port-add " + port["name"] + ' ' + port["speed"] + " NONE\n"
                if port["speed"] == "25G":
                    pm_config_str += "pm an-set " + port["name"] + " 2\n"

            pm_config_str += "pm port-enb -/-\npm show\n"
            py_import_str += "port_index = " + str(imported_port_index) + "\n"
            py_import_str += "frontports = " + str(imported_frontports).encode("utf-8") + "\n"
            py_import_str += "swports = " + str(imported_swports) + "\n"
            py_import_str += "ipaddr_inf = " + str(imported_ipaddr_inf) + "\n"
            py_import_str += "ipaddr_nbr = " + str(imported_ipaddr_nbr) + "\n"
            py_import_str += "mac_nbr = " + str(imported_mac_nbr) + "\n"
            py_import_str += "management_ip = \'" + str(machine["ip_address"]) + "\'\n"

            with open(machine["name"] + '_pm.cfg', 'w') as cfg_file:
                cfg_file.write(pm_config_str)
            with open(machine["name"] + '.py', 'w') as cfg_file:
                cfg_file.write(py_import_str)

        elif machine["type"] == "host":
            interfaces_string = "# interfaces(5) file used by ifup(8) and ifdown(8)\n"
            interfaces_string += "auto lo\n"
            interfaces_string += "iface lo inet loopback\n\n"
            interfaces_string += "auto " + str(machine["mgmt_iface"]) + "\n"
            interfaces_string += "iface " + str(machine["mgmt_iface"]) + " inet static\n"
            interfaces_string += "  address " + str(machine["ip_address"]) + "\n"
            interfaces_string += "  netmask " + str(machine["netmask"]) + "\n"
            interfaces_string += "  gateway " + str(machine["gateway"]) + "\n\n"
            interfaces_string += "dns-nameservers 10.10.10.10\n"
            interfaces_string += "dns-search swlab.com\n\n"
            py_import_str = ""
            imported_interface_index = []
            imported_interface_name = []
            imported_ipaddr_inf = []
            imported_ipaddr_nbr = []
            imported_mac_nbr = []
     

            for i,interface in enumerate(machine["interfaces"]):
                if "ip_address" not in interface:
                    interface["ip_address"] = "10.1" \
                                              + str(interface["connected_to_switch_role"]) \
                                              + str(machine["role"]) \
                                              + "." \
                                              + str(interface["role"]) \
                                              + ".2"
                interface["neighbor_ip_address"] = interface["ip_address"][:-1] + "1"
                ip_fields = interface["ip_address"].split(".")
                mac_fields = [':{:0>2}'.format(int(x) % 100) for x in ip_fields]
                interface["mac_address"] = "00:00" + "".join(mac_fields)
                interface["neighbor_mac_address"] = "aa:aa:aa:aa:aa:0" + str(interface["connected_to_switch_role"])
                
                imported_interface_index.append(i)
                imported_interface_name.append(str(interface["name"]))
                imported_ipaddr_inf.append(str(interface["ip_address"]))
                imported_ipaddr_nbr.append(str(interface["neighbor_ip_address"]))
                imported_mac_nbr.append(str(interface["neighbor_mac_address"]))

                interfaces_string += "auto " + interface["name"] + "\n"
                interfaces_string += "iface " + interface["name"] + " inet static\n"
                interfaces_string += "  address " + interface["ip_address"] + "\n"
                interfaces_string += "  netmask 255.255.255.0\n"
                interfaces_string += "  hwaddress ether " + interface["mac_address"] + "\n"
                interfaces_string += "  post-up ip neigh add " + interface["neighbor_ip_address"]
                interfaces_string += "  lladdr " + interface["neighbor_mac_address"]
                interfaces_string += "  dev " + interface["name"] +"\n\n"
            
            py_import_str += "interface_index = " + str(imported_interface_index) + "\n"
            py_import_str += "interface_name = " + str(imported_interface_name) + "\n"
            py_import_str += "ipaddr_inf = " + str(imported_ipaddr_inf) + "\n"
            py_import_str += "ipaddr_nbr = " + str(imported_ipaddr_nbr) + "\n"
            py_import_str += "mac_nbr = " + str(imported_mac_nbr) + "\n"
            py_import_str += "management_ip = \'" + str(machine["ip_address"]) + "\'\n"
            
            with open(machine["name"] + ".interfaces", 'w') as cfg_file:
                cfg_file.write(interfaces_string)
            with open(machine["name"] + ".py", 'w') as py_file:
                py_file.write(py_import_str)

if __name__ == "__main__":
    main()
