import switchapi_thrift

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../'))
sys.path.append(os.path.join(this_dir, '../api-tests/'))

from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *


def create_switch_port_interface(device,
                                 client,
                                 sw_port_list,
                                 intf_type=SWITCH_INTERFACE_TYPE_ACCESS):
    """ create  switch ports """
    intf_list = []
    for sw_port in sw_port_list:
        port = client.switch_api_port_id_to_handle_get(device, sw_port)
        # Create interface info data strucutre
        interface_info = switcht_interface_info_t(handle=port, type=intf_type)
        intf_list.append(
            client.switch_api_interface_create(device, interface_info))
    return intf_list


def delete_switch_port_interface(device, client, intf_list):
    """ deletes switch port interfaces """
    for intf in intf_list:
        client.switch_api_interface_delete(device, intf)
    return None


def create_switch_port_handles(device, client, port_list):
    """ create and return the port handles list """
    return_list = []
    for port in port_list:
        return_list.append(
            client.switch_api_port_id_to_handle_get(device=device, port=port))
    return return_list


def create_router_interface(device, client, **kwargs):
    """ Creates router interface based on the args passed """
    rif_info = switcht_rif_info_t(**kwargs)
    return client.switch_api_rif_create(device, rif_info)


def create_and_add_router_mac(device, client, mac, logger):
    """ Create and add router mac """
    rmac = client.switch_api_router_mac_group_create(device,
                                                     SWITCH_RMAC_TYPE_INNER)
    logger.info("Created rmac %s on device %s" % (rmac, device))
    status = client.switch_api_router_mac_add(device, rmac, mac)
    logger.debug("status - %s" % status)
    assert (status == 0)
    logger.info("Router mac created with mac %s " % mac)
    return rmac



def create_port_handles(device, client, port_list, logger):
    """ Creates a list of port handles """
    logger.info("Creating port handles for port list - %s" % port_list)
    return [
        client.switch_api_port_id_to_handle_get(device, port)
        for port in port_list
    ]


def get_all_tables(device, client):
    """ Get all the tables entries we know of """
    return {
        table.table_name: table.table_size
        for table in client.switch_api_table_all_get(device)
        if table.table_name != 'NA'
    }


def print_switch_tables(device, client):
    """ Print tables info """
    print "{:>50} --> {:>7}".format(" Table Name ", " Table Size")
    print "%s" % ("-" * 62)
    for k, v in get_all_tables(device, client).items():
        print "{:>50} --> {:>7}".format(k, v)
    print "%s" % ("-" * 62)
    return None


def filter_based_on_feature(device,
                            client,
                            feature_name,
                            feature_specific_entry=None):
    """ Filter based on feature name """
    feature_list = [(k, v) for k, v in get_all_tables(device, client).items()
                    if feature_name in k]
    if feature_specific_entry != None:
        for k, v in feature_list:
            if feature_specific_entry in k:
                return (k, v)
    else:
        return feature_list


def return_max_min_table_size_seen_based_on_table_sizes_api(
        device, client, feature_name):
    """ Return (max, min) seen for a table  """
    feature_list = [
        v for k, v in get_all_tables(device, client).items()
        if feature_name in k
    ]
    if len(feature_list) != 0:
        return (max(feature_list), min(feature_list))
    return (0, 0)


def ip_addr_to_str(a, n):
    if n is not None:
        a = a & ~((1 << (32 - n)) - 1)
    result = "%d.%d.%d.%d" % (a >> 24, \
                              (a >> 16) & 0xff, \
                              (a >> 8) & 0xff, \
                              a & 0xff \
                              )
    if n is not None:
        result = result + ("/%d" % (n))
    return result


def rand_nw_addr():
    return random.randint(0, (1 << 32) - 1)


def is_v4_mcast_addr(ip):
    """ check and make sure if the addr is mcast or broadcast """
    ip = ip.split(".")
    if int(ip[0]) in range(224, 241) or ip[-1].endswith("255"):
        return True
    return False


def generate_random_ip_address(num_ip_addr):
    """ Generate random ip iaddress """
    return_list = []
    for i in range(num_ip_addr):
        ip = ip_addr_to_str(rand_nw_addr(), None)
        if not is_v4_mcast_addr(ip):
            return_list.append(ip)
        else:
            return_list.extend(generate_random_ip_address(1))
    return return_list

def print_testcase_header(testname, func_type, log):
    """ Just print testcase info, Find a better way to do this """
    print "%s" %('-'*80)
    print "%s --> %s" %(testname, func_type)
    print "%s" %('-'*80)
