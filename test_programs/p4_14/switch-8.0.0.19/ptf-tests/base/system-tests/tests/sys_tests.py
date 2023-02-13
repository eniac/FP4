import os, sys, random, unittest
import pdb

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../'))
sys.path.append(os.path.join(this_dir, '../'))
sys.path.append(os.path.join(this_dir, '../../api-tests/'))

from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *

from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

from common.utils import *
from api_utils import *
from common.api_adapter import ApiAdapter

from systest_utils import *
from suite_libs import *

device = 0
cpu_port = 64
swports = []
for device, port, ifname in config["interfaces"]:
    swports.append(port)

if swports == []:
    swports = [x for x in range(65)]

log = get_systest_logger()
context = get_context_file_handler()

log.info("swports: %s" % swports)
global TOLERANCE  # Tolerance is used while filling table. We always fill upto 90% of table
TOLERANCE = 0.10


class AddDeleteVlans(ApiAdapter):
    def setUp(self):
        ApiAdapter.setUp(self)
        self.max_vlan, self.min_vlan = return_max_min_table_size_seen_based_on_table_sizes_api(
            device, self.client, 'bd flood')
        if self.max_vlan != None:
            #Set it to 10% of the table size
            # We divide by 3 because we use 3 entries in bd flood table \
            # for every vlan ( unicast, mcast, flood)
            self.max_vlan = (self.max_vlan - int(self.min_vlan * .10)) / 3
        else:
            self.max_vlan, self.min_vlan = 0, 0

    def runTest(self):
        log.info("Add/delete vlans in a loop")
        log.info("Getting the vlan table size from context.json")
        if self.max_vlan == 0:
            log, info(
                "Looks like switch profile compiled doesnot seems to have vlan table"
            )
        else:
            log.info(" Found possible min vlan table size to be : %s" %
                     self.max_vlan)
            vlan_list = []
            for vlan in range(2, self.max_vlan - 1):
                log.info("Creating vlan - %s on device - %s" % (vlan, device))
                vlan_list.append(self.add_vlan(device, vlan))

    def tearDown(self):
        self.cleanup()  # It would take care calling the api in the right way .
        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


class AddDeleteAccessInterfacesToVlan(ApiAdapter):
    def setUp(self):
        ApiAdapter.setUp(self)
        if len(swports) > 2:
            self.port_list = [
                self.select_port(device, port) for port in swports[:2]
            ]
            self.intf_list = [
                self.cfg_l2intf_on_port(device, port, 'access')
                for port in self.port_list
            ]
        else:
            self.intf_list = []

        # Reading the vlans possible from table api.
        self.max_vlan, self.min_vlan = return_max_min_table_size_seen_based_on_table_sizes_api(
            device, self.client, 'bd flood')
        if self.max_vlan != None:
            # We divide by 3 because we use 3 entries in bd flood table \
            # for every vlan ( unicast, mcast, flood)
            self.max_vlan = (self.max_vlan - int(self.min_vlan * .10)) / 3
        else:
            self.max_vlan, self.min_vlan = 0, 0

    def runTest(self):
        log.info("Add Delete Access Interfaces to Vlans")
        log.info("Or change access vlan on interfaces")
        if self.max_vlan == 0:
            log.info(
                "Looks like switch profile compiled doesnot seems to have vlan table"
            )
        else:
            vlan_list = get_n_random_intergers(2, self.max_vlan, 10)
            if len(vlan_list) == 0:
                log.info(
                    "Looks like we cannot configure atleast 10 vlans, abort this test"
                )
            log.info("Vlan list - %s ... " % vlan_list)
            for vlan in vlan_list:
                log.info("VLAN ID: %s, Create Vlan and Add Interfaces to Vlan"
                         % vlan)
                vlan_hdl = self.add_vlan(device, vlan)
                log.info("Adding vlan to interface list %s" % self.intf_list)
                for intf in self.intf_list:
                    log.info("Adding member - %s to vlan - %s" % (intf,
                                                                  vlan_hdl))
                    status = self.add_vlan_member(device, vlan_hdl, intf)
                    self.assertTrue(status)
                for intf in self.intf_list:
                    log.info("Removing members - %s from the vlan - %s" %
                             (intf, vlan_hdl))
                    status = self.no_vlan_member(device, vlan_hdl, intf)
                    self.assertTrue(status)

    def tearDown(self):
        self.cleanup()
        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


class L2TrunkToTrunkVlanTest(ApiAdapter):
    def setUp(self):
        ApiAdapter.setUp(self)
        if len(swports) > 2:
            self.port_list = [
                self.select_port(device, port) for port in swports[:2]
            ]
            self.intf_list = [
                self.cfg_l2intf_on_port(device, port, 'trunk')
                for port in self.port_list
            ]
        else:
            self.intf_list = []

        # Reading the vlans possible from table api.
        self.max_vlan, self.min_vlan = return_max_min_table_size_seen_based_on_table_sizes_api(
            device, self.client, 'bd flood')
        if self.max_vlan != None:
            # We divide by 3 because we use 3 entries in bd flood table \
            # for every vlan ( unicast, mcast, flood)
            self.max_vlan = (self.max_vlan - int(self.min_vlan * .10)) / 3
        else:
            self.max_vlan, self.min_vlan = 0, 0

    def runTest(self):
        log.info("Add Delete Access Interfaces to Vlans")
        log.info("Or change access vlan on interfaces")
        if self.max_vlan == 0:
            log.info(
                "Looks like switch profile compiled doesnot seems to have vlan table"
            )
        else:
            vlan_list = get_n_random_intergers(2, self.max_vlan, 10)
            if len(vlan_list) == 0:
                log.info(
                    "Looks like we cannot configure atleast 10 vlans, abort this test"
                )
            log.info("Vlan list - %s ... " % vlan_list)
            for vlan in vlan_list:
                log.info("VLAN ID: %s, Create Vlan and Add Interfaces to Vlan"
                         % vlan)
                vlan_hdl = self.add_vlan(device, vlan)
                log.info("Adding vlan to interface list %s" % self.intf_list)
                for intf in self.intf_list:
                    log.info("Adding member - %s to vlan - %s" % (intf,
                                                                  vlan_hdl))
                    status = self.add_vlan_member(device, vlan_hdl, intf)
                    self.assertTrue(status)
                for intf in self.intf_list:
                    log.info("Removing members - %s from the vlan - %s" %
                             (intf, vlan_hdl))
                    status = self.no_vlan_member(device, vlan_hdl, intf)
                    self.assertTrue(status)

    def tearDown(self):
        self.cleanup()
        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


@unittest.skip('Skiping TunnelTableFill test')
class L3MacTableFill(systestBaseTests):
    def setUp(self):
        print_testcase_header("L3MacTableFill", 'setUp', log)
        systestBaseTests.setUp(self)
        self.vrfToUse = random.randint(1, 32)
        self.max_mac = context.getMaxTableSizeBasedOnFeatureName('rmac')
        self.max_mac = 510 if self.max_mac >= 510 else self.max_mac
        self.mac_list = generate_random_mac_address(self.max_mac)
        #self.vrf = self.client.switch_api_vrf_create(device, self.vrfToUse)

    def runTest(self):
        print_testcase_header("L3MacTableFill", 'runTest', log)
        log.info("Starting L3mac fill test")
        if self.max_mac != 0:
            log.info("Maximum length of the mac table found to be : %s" %
                     self.max_mac)
            log.info("Creating VRF %s on device %s" % (self.vrfToUse, device))
            rmac = self.client.switch_api_router_mac_group_create(
                device, SWITCH_RMAC_TYPE_INNER)
            i = 0
            for mac in self.mac_list:
                log.info("Adding mac address %s " % mac)
                log.info("Adding mac- count : %s" % i)
                status = self.client.switch_api_router_mac_add(
                    device, rmac, mac)
                self.assertEqual(status, 0)
                i = i + 1
            log.info("Mac table is filled, now send a traffic and verify")

            log.info("Now do the traffic tests")
            num_intf = 2
            port = {}
            intf = {}
            rif = {}
            intf_ip = {}
            interface_info = {}
            rif_info = {}

            nhop_key = {}
            nhop = {}
            neighbor_entry = {}
            neighbor = {}
            vrf = self.client.switch_api_vrf_create(device, swports[1])

            for i in range(0, num_intf):
                port[i] = self.client.switch_api_port_id_to_handle_get(
                    device=device, port=swports[i])
                log.info("Got Port handle : %s" % port[i])
                log.info("Creating router interface handle")

                rif_info[i] = switcht_rif_info_t(
                    rif_type=SWITCH_RIF_TYPE_INTF,
                    vrf_handle=vrf,
                    rmac_handle=rmac,
                    v4_unicast_enabled=True)
                rif[i] = self.client.switch_api_rif_create(
                    device=device, rif_info=rif_info[i])
                log.info("Created router interface : %s" % rif[i])
                log.info("Creating interface hanlde to use it with rif")
                interface_info[i] = switcht_interface_info_t(
                    handle=port[i],
                    type=SWITCH_INTERFACE_TYPE_PORT,
                    rif_handle=rif[i])
                intf[i] = self.client.switch_api_interface_create(
                    device=device, interface_info=interface_info[i])
                log.info("Created interface : %s " % intf[i])
                ipaddr = "%d.%d.%d.%d" % ((i + 1), (i + 1), (i + 1), 1)
                log.info("Ip addres %s to use with intf %s" % (ipaddr,
                                                               intf[i]))
                log.info("Creating ip address handle for ip :%s" % ipaddr)
                intf_ip[i, 1] = switcht_ip_addr_t(
                    addr_type=SWITCH_API_IP_ADDR_V4,
                    ipaddr=ipaddr,
                    prefix_length=16)
                status = self.client.switch_api_l3_interface_address_add(
                    device, rif[i], vrf, intf_ip[i, 1])
                log.info("Status : %s" % status)
                self.assertEqual(status, 0)
                log.info("Added ip add %s to  vrf %s on intf %s" %
                         (ipaddr, vrf, intf[i]))
                log.info("Creating nhop entry for the intf : %s" % (intf[i]))
                nhop_key[i] = switcht_nhop_key_t(
                    intf_handle=rif[i], ip_addr_valid=0)
                nhop[i] = self.client.switch_api_nhop_create(
                    device=device, nhop_key=nhop_key[i])
                log.info("Created nhop : %s" % nhop[i])
                ipaddr = "%d.%d.%d.%d" % ((i + 1), 0, 0, 2)
                intf_ip[i, 2] = switcht_ip_addr_t(
                    addr_type=SWITCH_API_IP_ADDR_V4,
                    ipaddr=ipaddr,
                    prefix_length=16)
                mac_addr_r = "ab:cd:%d:ab:cd:%d" % ((i + 10), (i + 10))
                neighbor_entry[i] = switcht_neighbor_info_t(
                    nhop_handle=nhop[i],
                    interface_handle=rif[i],
                    mac_addr=mac_addr_r,
                    ip_addr=intf_ip[i, 2],
                    rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L3)
                neighbor[i] = self.client.switch_api_neighbor_entry_add(
                    device=device, neighbor=neighbor_entry[i])
                status = self.client.switch_api_l3_route_add(
                    device, vrf, intf_ip[i, 2], nhop[i])
                self.assertEqual(status, 0)

            for i in range(0, num_intf):
                log.info("Deleting L3 route")
                status = self.client.switch_api_l3_route_delete(
                    device, vrf, intf_ip[i, 2], nhop[i])
                log.debug("Status - %s" % status)
                self.assertEqual(status, 0)
                log.info("Deleting neighbor entry %s" % neighbor[i])
                status = self.client.switch_api_neighbor_entry_remove(
                    device, neighbor[i])
                log.debug("Status - %s" % status)
                self.assertEqual(status, 0)
                log.info("Deleting nhop %s" % nhop[i])
                status = self.client.switch_api_nhop_delete(device, nhop[i])
                log.debug("Status - %s" % status)
                self.assertEqual(status, 0)
                log.info("Deleting  l3 interface address %s" % intf_ip[i, 1])
                status = \
                    self.client.switch_api_l3_interface_address_delete(device,rif[i],vrf,intf_ip[i,1])
                log.debug("Status - %s" % status)
                self.assertEqual(status, 0)
                log.info("Deleting the interface %s" % intf[i])
                status = self.client.switch_api_interface_delete(
                    device, intf[i])
                log.debug("Status - %s" % status)
                self.assertEqual(status, 0)

            for mac in self.mac_list:
                log.info("Deleting mac address %s" % mac)
                status = self.client.switch_api_router_mac_delete(
                    device, rmac, mac)
                self.assertEqual(status, 0)

        else:
            log.info(
                "Looks like switch profile compiled doesnot seem to have mac table "
            )


class LagTest(ApiAdapter):
    """
        Run this test with ports.json file as input to model and test
    """

    def setUp(self):
        #print_testcase_header("LagTest", 'setUp', log)
        ApiAdapter.setUp(self)
        self.vlan = self.add_vlan(device, 10)
        # Remove first one and CPU port.
        #self.max_ports = len(swports)
        self.max_ports = 7
        self.port_list = create_port_handles(device, self.client,
                                             range(0, self.max_ports), log)
        self.iface = self.cfg_l2intf_on_port(device, self.port_list[0])

    def runTest(self):
        """ Lag Test , We just create and delete as many lag as possible """
        #print_testcase_header("LagTest", 'runTest', log)
        self.lag_list = []
        self.lag_max, self.lag_min = return_max_min_table_size_seen_based_on_table_sizes_api(
            device, self.client, 'lag')
        self.lag_max = self.lag_max / 64  #64 because by default we create 64 entries for each lag entry
        self.lag_max = self.lag_max - int(self.lag_max * TOLERANCE)
        log.info("Lag max - %s, lag min - %s" % (self.lag_max, self.lag_min))
        if self.lag_max != None:
            for i in range(0, self.lag_max):
                lag = self.add_lag(device)
                self.lag_list.append(lag)
                log.info("Created lag - %s - iteration - %s ..." % (lag, i))

            for lag in self.lag_list:
                status = self.remove_lag(device, lag)
                log.debug("Status :  %s" % status)
                self.assertTrue(status)
                log.info("Deleted lag %s ..." % lag)

            self.lag_list = []
            for i in range(0, self.lag_max - 1):
                lag = self.add_lag(device)
                self.lag_list.append(lag)
                log.info("Created lag - %s ..." % lag)

            self.lag_to_test = self.add_lag(device)
            for port in self.port_list[1:self.max_ports]:
                status = self.add_lag_member(device, self.lag_to_test, port)
                log.debug("Status - %s" % status)
                self.assertTrue(status)
                log.info("Added port %s to lag %s ..." % (port,
                                                          self.lag_to_test))

            self.lag_intf = self.add_logical_l2lag(device, self.lag_to_test)
            for intf in [self.iface, self.lag_intf]:
                status = self.add_vlan_member(device, self.vlan, intf)
                log.debug("Status - %s" % status)
                self.assertTrue(status)
                log.info("Added iface - %s to vlan - %s ..." % (intf,
                                                                self.vlan))

            status = self.add_mac_table_entry(
                device, self.vlan, '00:22:22:22:22:22', 2, self.iface)
            log.debug("Status - %s" % status)
            self.assertTrue(status)
            log.info("Added mac - %s to intf - %s ... " % ('00:22:22:22:22:22',
                                                           self.iface))
            try:
                count = [0 for i in self.port_list[1:self.max_ports]]
                dst_ip = int(socket.inet_aton('10.10.10.1').encode('hex'), 16)
                src_ip = int(socket.inet_aton('192.168.8.1').encode('hex'), 16)
                max_itrs = 500
                for i in range(0, max_itrs):
                    dst_ip_addr = socket.inet_ntoa(
                        format(dst_ip, 'x').zfill(8).decode('hex'))
                    src_ip_addr = socket.inet_ntoa(
                        format(src_ip, 'x').zfill(8).decode('hex'))
                    pkt = simple_tcp_packet(
                        eth_dst='00:11:11:11:11:11',
                        eth_src='00:22:22:22:22:22',
                        ip_dst=dst_ip_addr,
                        ip_src=src_ip_addr,
                        ip_id=109,
                        ip_ttl=64)

                    exp_pkt = simple_tcp_packet(
                        eth_dst='00:11:11:11:11:11',
                        eth_src='00:22:22:22:22:22',
                        ip_dst=dst_ip_addr,
                        ip_src=src_ip_addr,
                        ip_id=109,
                        ip_ttl=64)

                    send_packet(self, swports[0], str(pkt))
                    rcv_idx = verify_any_packet_any_port(
                        self,
                        [exp_pkt for i in self.port_list[1:self.max_ports]],
                        swports[1:self.max_ports])
                    log.info("rcv_idx : %s ..." % rcv_idx)
                    count[rcv_idx] += 1
                    dst_ip += 1
                    src_ip += 1

                log.info('L2LagTest: %s ...' % count)
                for i in range(0, self.max_ports - 1):
                    self.assertTrue(
                        (count[i] >=
                         ((max_itrs / len(swports[1:self.max_ports])) * 0.6)),
                        "Not all paths are equally balanced")
            finally:
                pass
        else:
            log.info("Cannnot run lag test as lag is not enabled")

    def tearDown(self):
        self.cleanup()
        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


class LagTableFill(ApiAdapter):
    """
        Run this test with ports.json file as input to model and test
    """

    def setUp(self):
        """ Setup for the LagTableFill test """
        ApiAdapter.setUp(self)

    def runTest(self):
        """ Actual test run """
        max_lags, _ = return_max_min_table_size_seen_based_on_table_sizes_api(
            device, self.client, 'lag')
        # We divide by 64 because for each lag we set aside 64 entries
        max_lags = max_lags / 64
        # Set tolereance
        max_lags = max_lags - int(max_lags * TOLERANCE)
        lag = {}

        for i in range(0, max_lags):
            lag[i] = self.add_lag(device)

        for i in range(0, max_lags):
            status = self.remove_lag(device, lag[i])
            self.assertTrue(status)

        mac_type = 2
        # lag group tests
        intf_count = 16

        for i in range(0, intf_count):
            vid = 1000 + i
            log.info("Create vlan - %s" % vid)
            vlan = self.add_vlan(device, vid)
            log.info("Created vlan - %s" % vlan)
            self.lag = self.add_lag(device)

            lag_port_list = []
            port_list = []

            for j in range(1, intf_count):
                lag_port_list.append(
                    self.client.switch_api_port_id_to_handle_get(
                        device, swports[j]))
                port_list.append(swports[j])

            log.info(lag_port_list)
            list_len = len(lag_port_list)

            # create lag interface and add to vlan
            log.info("create lag interface")
            self.lag_intf = self.add_logical_l2lag(device, self.lag)
            log.info("Add lag interface - %s to vlan - %s" % (self.lag_intf,
                                                              vlan))
            status = self.add_vlan_member(device, vlan, self.lag_intf)
            self.assertTrue(status)
            log.info("delete lag interface")
            status = self.no_vlan_member(device, vlan, self.lag_intf)
            self.assertTrue(True)
            status = self.remove_l2intf_on_port(device, self.lag_intf)
            self.assertTrue(True)
            log.info("Remove lag %s" % self.lag)
            status = self.remove_lag(device, self.lag)
            self.assertTrue(True)
            log.info("Remove vlan - %s" % vlan)
            status = self.no_vlan(device, vlan)
            self.assertTrue(True)

            self.lag = self.add_lag(device)
            log.info("Add members %s to lag %s" % (lag_port_list, self.lag))
            for member in lag_port_list:
                status = self.add_lag_member(device, self.lag, member)
                self.assertTrue(status)
            log.info("Create Vlan")
            vid = 2000 + i
            vlan = self.add_vlan(device, vid)
            log.info("create lag interface")
            self.lag_intf = self.add_logical_l2lag(device, self.lag)
            log.info("Add lag interface - %s to vlan - %s" % (self.lag_intf,
                                                              vlan))
            status = self.add_vlan_member(device, vlan, self.lag_intf)
            self.assertTrue(status)

            # create access interface on port 0
            snd_port = swports[0]
            # create interface for the swport
            log.info("Create Access Interface")
            port1 = self.client.switch_api_port_id_to_handle_get(
                device, swports[0])

            access_intf = self.cfg_l2intf_on_port(device, port1)
            log.info("Add Access Intf to Vlan")
            status = self.add_vlan_member(device, vlan, access_intf)
            self.assertTrue(status)

            # remove a range of members from lag group
            # add members back to lag group
            for k in range(1, list_len):
                log.info("removing members %s" % lag_port_list[k:list_len])
                for member in lag_port_list[k:list_len]:
                    status = self.remove_lag_member(device, self.lag, member)
                    self.assertTrue(status)

                log.info("adding members %s" % lag_port_list[k:list_len])
                for member in lag_port_list[k:list_len]:
                    status = self.add_lag_member(device, self.lag, member)
                    self.assertTrue(status)

                mac_entry = {}
                # program mac on the swport and the lag_port
                mac_addr1 = "002222" + hex(vid).lstrip('0x').zfill(6)
                mac_addr1 = ":".join(
                    s.encode('hex') for s in mac_addr1.decode('hex'))
                log.info(
                    "Program mac entry %s on access interface" % mac_addr1)

                mac_entry[mac_addr1] = self.add_mac_table_entry(
                    device, vlan, mac_addr1, mac_type, access_intf)
                mac_addr2 = "003333" + hex(vid).lstrip('0x').zfill(6)
                mac_addr2 = ':'.join(
                    s.encode('hex') for s in mac_addr2.decode('hex'))

                log.info("Program mac entry %s on lag interface" % mac_addr2)
                mac_entry[mac_addr2] = self.add_mac_table_entry(
                    device, vlan, mac_addr2, mac_type, self.lag_intf)

                # send traffic from the access_intf to the lag group
                dst_ip_addr = "10.10.10.1"
                pkt = simple_tcp_packet(
                    eth_dst=mac_addr2,
                    eth_src=mac_addr1,
                    ip_dst=dst_ip_addr,
                    ip_src='192.168.1.1',
                    ip_id=109,
                    ip_ttl=64)
                exp_pkt = simple_tcp_packet(
                    eth_dst=mac_addr2,
                    eth_src=mac_addr1,
                    ip_dst=dst_ip_addr,
                    ip_src='192.168.1.1',
                    ip_id=109,
                    ip_ttl=64)
                try:
                    log.info("Send packet from access port 0 to lag interface")
                    send_packet(self, swports[0], str(pkt))
                    verify_any_packet_any_port(self, exp_pkt, ports=port_list)
                finally:
                    log.info("Access to Lag traffic test done")

                pkt = simple_tcp_packet(
                    eth_dst=mac_addr1,
                    eth_src=mac_addr2,
                    ip_dst=dst_ip_addr,
                    ip_src='192.168.1.1',
                    ip_id=109,
                    ip_ttl=64)
                exp_pkt = simple_tcp_packet(
                    eth_dst=mac_addr1,
                    eth_src=mac_addr2,
                    ip_dst=dst_ip_addr,
                    ip_src='192.168.1.1',
                    ip_id=109,
                    ip_ttl=64)
                try:
                    log.info("Send packet from lag member to access interface")
                    log.info("Lag member %d to access interface 0" % k)

                    send_packet(self, swports[k], str(pkt))
                    verify_packets(self, exp_pkt, [swports[0]])
                finally:
                    log.info("Lag member to Access traffic test done")

                status = self.remove_mac_table_entry(device, vlan, mac_addr1,
                                                     mac_type, access_intf)
                self.assertTrue(status)
                status = self.remove_mac_table_entry(device, vlan, mac_addr2,
                                                     mac_type, self.lag_intf)
                self.assertTrue(status)

            # remove the access_intf from the vlan
            log.info("remove access interface from vlan")
            status = self.no_vlan_member(device, vlan, access_intf)
            self.assertTrue(status)
            log.info("access interface delete")
            status = self.remove_l2intf_on_port(device, access_intf)
            self.assertTrue(status)
            log.info("lag member delete")
            for member in lag_port_list:
                status = self.remove_lag_member(device, self.lag, member)
                self.assertTrue(status)
            log.info("remove lag interface from vlan")
            status = self.no_vlan_member(device, vlan, self.lag_intf)
            self.assertTrue(status)
            status = self.remove_l2intf_on_port(device, self.lag_intf)
            self.assertTrue(status)
            status = self.remove_lag(device, self.lag)
            self.assertTrue(status)
            # delete vlan
            log.info("vlan delete")
            status = self.no_vlan(device, vlan)
            self.assertTrue(status)

    def tearDown(self):
        self.cleanup()
        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


class L3HostFill(ApiAdapter):
    def setUp(self):
        ApiAdapter.setUp(self)
        self.rmac = self.add_rmac(device, '00:77:66:55:44:33')
        self.vrf = self.add_vrf(device, 2)
        self.port_list = create_port_handles(device, self.client,
                                             range(0, 16), log)
        self.rif_list, self.interface_list, self.ipaddr_list, self.ip_list = [], [], [], []

        for i in range(0, len(self.port_list)):
            ip = "%s.%s.%s.%s" % ((i + 1), (i + 1), (i + 1), (i + 2))
            self.ip_list.append(ip)
            rif = self.create_l3_rif(device, self.vrf, self.rmac,
                                     self.port_list[i], ip)
            self.rif_list.append(rif)

    def runTest(self):
        log.info("Now add the static route and do traffic check")
        # TODO: Write a proc to get valid ip addresses
        ip_host_list = ["%s.%s.%s.%s" % (i, i, i, 10) for i in range(50, 100)]
        self.ipaddr_handle_list = []
        self.nhop_list = []
        self.neighbor_list = []
        for ip in ip_host_list:
            nhop = self.add_l3_nhop(device, self.rif_list[1], ip,
                                    '00:11:22:33:44:55')
            self.nhop_list.append(nhop)
            status = self.add_static_route(
                device, self.vrf, ip, nhop, prefix_length=32)
            self.assertTrue(status)

        # send the test packet(s)
        for ip in ip_host_list:

            log.info("Now sending from ip %s to address ip - %s" %
                     (self.ip_list[0], ip))
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst=ip,
                ip_src=self.ip_list[0],
                ip_id=105,
                pktlen=9100,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ip_dst=ip,
                ip_src=self.ip_list[0],
                ip_id=105,
                pktlen=9100,
                ip_ttl=63)
            try:
                send_packet(self, swports[0], str(pkt))
                verify_packets(self, exp_pkt, [swports[1]])
            finally:
                pass

    def tearDown(self):
        self.cleanup()
        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


class L3IPv6HostFill(ApiAdapter):
    def setUp(self):
        ApiAdapter.setUp(self)
        self.rmac = self.add_rmac(device, '00:77:66:55:44:33')
        self.vrf = self.add_vrf(device, 2)
        self.port_list = create_port_handles(device, self.client,
                                             range(0, 16), log)
        self.rif_list, self.interface_list, self.ipaddr_list, self.ip_list = [], [], [], []

        for i in range(0, len(self.port_list)):
            ip = "%s::%s" % ((i + 2000), (2))
            self.ip_list.append(ip)
            rif = self.create_l3_rif(device, self.vrf, self.rmac,
                                     self.port_list[i], ip)
            self.rif_list.append(rif)

    def runTest(self):
        log.info("Now add the static route and do traffic check")
        # TODO: Write a proc to get valid ip addresses
        ip_host_list = ["%s::%s" % (i, 1) for i in range(4000, 4051)]
        self.ipaddr_handle_list = []
        self.nhop_list = []
        self.neighbor_list = []
        for ip in ip_host_list:
            nhop = self.add_l3_nhop(
                device, self.rif_list[1], ip, '00:11:22:33:44:55', v4=False)
            self.nhop_list.append(nhop)
            status = self.add_static_route(
                device, self.vrf, ip, nhop, v4=False, prefix_length=128)
            self.assertTrue(status)

        # send the test packet(s)
        for ip in ip_host_list[:10]:

            log.info("Now sending from ip %s to address ip - %s" %
                     (self.ip_list[0], ip))
            pkt = simple_tcpv6_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ipv6_dst=ip,
                ipv6_src=self.ip_list[0],
                ipv6_hlim=64)
            exp_pkt = simple_tcpv6_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ipv6_dst=ip,
                ipv6_src=self.ip_list[0],
                ipv6_hlim=63)
            try:
                send_packet(self, swports[0], str(pkt))
                verify_packets(self, exp_pkt, [swports[1]])
            finally:
                pass

    def tearDown(self):
        self.cleanup()
        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


class L3LpmTableFill(ApiAdapter):
    def setUp(self):
        ApiAdapter.setUp(self)
        self.rmac = self.add_rmac(device, '00:77:66:55:44:33')
        self.vrf = self.add_vrf(device, 2)
        self.port_list = create_port_handles(device, self.client,
                                             range(0, 16), log)
        self.rif_list, self.interface_list, self.ipaddr_list, self.ip_list = [], [], [], []

        for i in range(0, len(self.port_list)):
            ip = "%s.%s.%s.%s" % ((i + 1), (i + 1), (i + 1), (i + 2))
            self.ip_list.append(ip)
            rif = self.create_l3_rif(device, self.vrf, self.rmac,
                                     self.port_list[i], ip)
            self.rif_list.append(rif)

    def runTest(self):
        log.info("Now add the static route and do traffic check")
        # TODO: Write a proc to get valid ip addresses
        ip_host_list = ["%s.%s.%s.%s" % (i, i, i, 10) for i in range(50, 100)]
        self.ipaddr_handle_list = []
        self.nhop_list = []
        self.neighbor_list = []
        for ip in ip_host_list:
            nhop = self.add_l3_nhop(device, self.rif_list[1], ip,
                                    '00:11:22:33:44:55')
            self.nhop_list.append(nhop)
            status = self.add_static_route(
                device, self.vrf, ip, nhop, prefix_length=16)
            self.assertTrue(status)

        # send the test packet(s)
        for ip in ip_host_list:

            log.info("Now sending from ip %s to address ip - %s" %
                     (self.ip_list[0], ip))
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst=ip,
                ip_src=self.ip_list[0],
                ip_id=105,
                pktlen=9100,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ip_dst=ip,
                ip_src=self.ip_list[0],
                ip_id=105,
                pktlen=9100,
                ip_ttl=63)
            try:
                send_packet(self, swports[0], str(pkt))
                verify_packets(self, exp_pkt, [swports[1]])
            finally:
                pass

    def tearDown(self):
        self.cleanup()
        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


class L3IPv6LpmTableFill(ApiAdapter):
    def setUp(self):
        ApiAdapter.setUp(self)
        self.rmac = self.add_rmac(device, '00:77:66:55:44:33')
        self.vrf = self.add_vrf(device, 2)
        self.port_list = create_port_handles(device, self.client,
                                             range(0, 16), log)
        self.rif_list, self.interface_list, self.ipaddr_list, self.ip_list = [], [], [], []

        for i in range(0, len(self.port_list)):
            ip = "%s::%s" % ((i + 2000), (2))
            self.ip_list.append(ip)
            rif = self.create_l3_rif(device, self.vrf, self.rmac,
                                     self.port_list[i], ip)
            self.rif_list.append(rif)

    def runTest(self):
        log.info("Now add the static route and do traffic check")
        # TODO: Write a proc to get valid ip addresses
        ip_host_list = ["%s::%s" % (i, 1) for i in range(4000, 4051)]
        self.ipaddr_handle_list = []
        self.nhop_list = []
        self.neighbor_list = []
        for ip in ip_host_list:
            nhop = self.add_l3_nhop(
                device,
                self.rif_list[1],
                ip,
                '00:11:22:33:44:55',
                v4=False,
                prefix_length=64)
            self.nhop_list.append(nhop)
            status = self.add_static_route(
                device, self.vrf, ip, nhop, v4=False, prefix_length=64)
            self.assertTrue(status)

        # send the test packet(s)
        for ip in ip_host_list[:10]:

            log.info("Now sending from ip %s to address ip - %s" %
                     (self.ip_list[0], ip))
            pkt = simple_tcpv6_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ipv6_dst=ip,
                ipv6_src=self.ip_list[0],
                ipv6_hlim=64)
            exp_pkt = simple_tcpv6_packet(
                eth_dst='00:11:22:33:44:55',
                eth_src='00:77:66:55:44:33',
                ipv6_dst=ip,
                ipv6_src=self.ip_list[0],
                ipv6_hlim=63)
            try:
                send_packet(self, swports[0], str(pkt))
                verify_packets(self, exp_pkt, [swports[1]])
            finally:
                pass

    def tearDown(self):
        self.cleanup()
        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


class EcmpTableFill(ApiAdapter):
    def setUp(self):
        ApiAdapter.setUp(self)
        self.sw_ports = swports
        # Remove cpu port from the sw_port list
        try:
            self.sw_ports = swports.pop(cpu_port)
        except IndexError:
            log.info('Looks like cpu port - %s' % cpu_port)
        # TODO: Remove this
        self.sw_ports = self.sw_ports[:16]

        self.rmac = self.add_rmac(device, '00:77:66:55:44:33')
        self.vrf = self.add_vrf(device, 2)
        self.port_list = create_port_handles(device, self.client,
                                             range(0, len(self.sw_ports)), log)
        self.rif_list, self.interface_list, self.ipaddr_list, self.ip_list = [], [], [], []

        for i in range(0, len(self.port_list)):
            ip = "%s.%s.%s.%s" % ((i + 1), (i + 1), (i + 1), (i + 2))
            self.ip_list.append(ip)
            rif = self.create_l3_rif(device, self.vrf, self.rmac,
                                     self.port_list[i], ip)
            self.rif_list.append(rif)

    def runTest(self):

        log.info("Now setup ECMP and fill the table")
        self.ip = "10.10.10.10"
        # We can do this because we know we limit the intf to 16
        self.mac_list = [
            "00:11:22:33:44:%s" % format((hex(i).split('x')[1]), '0>2')
            for i in range(1, len(self.rif_list))
        ]
        self.nhop_list = []
        self.neighbor_list = []
        for mac, iface in zip(self.mac_list, self.rif_list[1:]):
            nhop = self.add_nhop(device, iface, self.ip)
            self.nhop_list.append(nhop)
            neighbor = self.add_neighbor_l3intf(device, nhop, mac)
            self.neighbor_list.append(neighbor)

        self.ecmp = self.add_ecmp(device)
        status = self.add_ecmp_member(device, self.ecmp,
                                      len(self.nhop_list), self.nhop_list)
        log.info("Sending traffic and validate")
        status = self.add_static_route(
            device, self.vrf, self.ip, self.ecmp, prefix_length=32)
        self.assertTrue(status)
        try:
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.10',
                ip_src=self.ip_list[0],
                ip_id=106,
                ip_ttl=64)

            exp = [
                simple_tcp_packet(
                    eth_dst=mac,
                    eth_src='00:77:66:55:44:33',
                    ip_dst='10.10.10.10',
                    ip_src=self.ip_list[0],
                    ip_id=106,
                    ip_ttl=63) for mac in self.mac_list
            ]
            send_packet(self, self.sw_ports[0], str(pkt))
            verify_any_packet_any_port(self, exp,
                                       self.sw_ports[1:len(self.mac_list)])
        finally:
            pass

        status = self.no_ecmp_member(device, self.ecmp, 2, self.nhop_list[-2:])
        self.assertTrue(status)

        log.info("Sending traffic and validate")
        try:
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.10',
                ip_src=self.ip_list[0],
                ip_id=106,
                ip_ttl=64)

            exp = [
                simple_tcp_packet(
                    eth_dst=mac,
                    eth_src='00:77:66:55:44:33',
                    ip_dst='10.10.10.10',
                    ip_src=self.ip_list[0],
                    ip_id=106,
                    ip_ttl=63) for mac in self.mac_list[:-2]
            ]
            send_packet(self, self.sw_ports[0], str(pkt))
            verify_any_packet_any_port(
                self, exp, self.sw_ports[1:len(self.mac_list[:-2])])
        finally:
            pass

        status = self.add_ecmp_member(device, self.ecmp, 2,
                                      self.nhop_list[-2:])
        self.assertTrue(status)

        log.info("Sending traffic and validate")
        try:
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst='10.10.10.10',
                ip_src=self.ip_list[0],
                ip_id=106,
                ip_ttl=64)

            exp = [
                simple_tcp_packet(
                    eth_dst=mac,
                    eth_src='00:77:66:55:44:33',
                    ip_dst='10.10.10.10',
                    ip_src=self.ip_list[0],
                    ip_id=106,
                    ip_ttl=63) for mac in self.mac_list
            ]
            send_packet(self, self.sw_ports[0], str(pkt))
            verify_any_packet_any_port(self, exp,
                                       self.sw_ports[1:len(self.mac_list)])
        finally:
            pass

    def tearDown(self):
        self.cleanup()
        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


class MirrorTableFill(ApiAdapter):
    def setUp(self):
        ApiAdapter.setUp(self)
        self.sw_ports = swports
        # Remove cpu port from the sw_port list
        try:
            self.sw_ports = swports.pop(cpu_port)
        except IndexError:
            log.info('Looks like cpu port - %s' % cpu_port)
        # TODO: Remove this
        self.sw_ports = self.sw_ports[:16]

        self.rmac = self.add_rmac(device, '00:77:66:55:44:33')
        self.vrf = self.add_vrf(device, 2)
        self.port_list = create_port_handles(device, self.client,
                                             range(0, len(self.sw_ports)), log)
        self.rif_list, self.interface_list, self.ipaddr_list, self.ip_list = [], [], [], []

        for i in range(0, len(self.port_list)):
            ip = "%s.%s.%s.%s" % ((i + 1), (i + 1), (i + 1), (i + 2))
            self.ip_list.append(ip)
            rif = self.create_l3_rif(device, self.vrf, self.rmac,
                                     self.port_list[i], ip)
            self.rif_list.append(rif)

    def runTest(self):
        log.info("Doing mirror test now ")
        self.mirror_list = []
        # Create mirrors for all ports except for last one
        # We will mirror traffic to that port.
        session_id = 1
        max_sessions_per_port, _ = return_max_min_table_size_seen_based_on_table_sizes_api(
            device, self.client, 'mirror')
        max_sessions_per_port = max_sessions_per_port - int(
            max_sessions_per_port * TOLERANCE)
        log.info("Max mirror session that are possible - %s ..." %
                 max_sessions_per_port)
        for port in self.port_list[:-1]:
            for j in range(0, 50):
                if session_id <= max_sessions_per_port:
                    if session_id != 250:
                        mirror = self.add_mirror(
                            device,
                            session_id=session_id,
                            direction=1,
                            egress_port_handle=port,
                            mirror_type=SWITCH_MIRROR_TYPE_LOCAL,
                            cos=0,
                            max_pkt_len=0,
                            ttl=0,
                            nhop_handle=0)
                        self.mirror_list.append(mirror)
                        log.info(
                            "Created mirror session - %s on port %s with session id: %s"
                            % (mirror, port, session_id))
                    else:
                        log.info(
                            "Mirror Session id - %s is reserved for mirroring to CPU"
                            % session_id)
                else:
                    log.info("Seems like we are out of mirror sessions")
                    log.info("Current port - %s" % port)
                session_id = session_id + 1

        #Create a new mirror session to send traffic to it
        ip = '10.10.10.1'
        mac = '00:11:22:33:44:55'
        nhop = self.add_l3_nhop(device, self.rif_list[1], ip, mac)
        status = self.add_static_route(
            device, self.vrf, ip, nhop, prefix_length=32)
        self.assertTrue(True)

        # Create a mirror session
        mirror = self.add_mirror(
            device,
            session_id=1001,
            direction=2,
            egress_port_handle=self.port_list[3],
            mirror_type=SWITCH_MIRROR_TYPE_LOCAL,
            cos=0,
            max_pkt_len=0x600,
            ttl=0,
            nhop_handle=0)
        log.info("Created egress mirror - %s acl to mirror from %s to %s" %
                 (mirror, swports[0], swports[15]))

        acl = self.client.switch_api_acl_list_create(
            device, SWITCH_API_DIRECTION_EGRESS, SWITCH_ACL_TYPE_EGRESS_SYSTEM,
            SWITCH_HANDLE_TYPE_PORT)
        # create kvp to match egress port and deflect bit
        kvp = []
        kvp_val = switcht_acl_value_t(value_num=self.port_list[1])
        kvp_mask = switcht_acl_value_t(value_num=0xff)
        kvp.append(
            switcht_acl_key_value_pair_t(SWITCH_ACL_EGR_FIELD_DEST_PORT,
                                         kvp_val, kvp_mask))
        kvp_val = switcht_acl_value_t(value_num=0)
        kvp_mask = switcht_acl_value_t(value_num=0xff)
        kvp.append(
            switcht_acl_key_value_pair_t(SWITCH_ACL_EGR_FIELD_DEFLECT, kvp_val,
                                         kvp_mask))
        action = 1
        action_params = switcht_acl_action_params_t()
        opt_action_params = switcht_acl_opt_action_params_t(
            mirror_handle=mirror)
        ace = self.client.switch_api_acl_egr_rule_create(
            device, acl, 11, 2, kvp, action, action_params, opt_action_params)
        status = self.client.switch_api_acl_reference(device, acl,
                                                      self.port_list[1])
        log.debug("status %s" % status)
        self.assertEqual(status, 0)

        # send the test packet(s)
        try:
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst=ip,
                ip_src=self.ip_list[0],
                ip_id=105,
                ip_ttl=64)

            exp_pkt = simple_tcp_packet(
                eth_dst=mac,
                eth_src='00:77:66:55:44:33',
                ip_dst=ip,
                ip_src=self.ip_list[0],
                ip_id=105,
                ip_ttl=63)
            send_packet(self, swports[0], str(pkt))
            verify_packet(self, exp_pkt, swports[1])
            verify_packet(self, exp_pkt, swports[3])
            verify_no_other_packets(self)
        finally:
            pass

        # Cleanup ACL
        status = self.client.switch_api_acl_dereference(
            0, acl, self.port_list[1])
        log.debug('status - %s' % status)
        self.assertEqual(status, 0)
        log.info("Deleted mirror session - %s" % mirror)
        status = self.client.switch_api_acl_rule_delete(0, acl, ace)
        log.debug('status - %s' % status)
        self.assertEqual(status, 0)
        log.info("Deleted mirror session - %s" % mirror)
        status = self.client.switch_api_acl_list_delete(0, acl)
        log.debug('status - %s' % status)
        self.assertEqual(status, 0)
        log.info("Deleted mirror session - %s" % mirror)

    def tearDown(self):
        self.cleanup()
        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


class L2IPv4LagTableFill(ApiAdapter):

    # This need to be run with port.json file.
    # Default profile seems to only give 8 ports.
    def setUp(self):
        ApiAdapter.setUp(self)
        self.max_ports = 7
        self.port_list = create_port_handles(device, self.client,
                                             range(0, self.max_ports), log)
        self.iface = self.cfg_l2intf_on_port(device, self.port_list[0])

    def runTest(self):
        log.info("Starting L2lagTestTableFill test ...")
        self.lag = self.add_lag(device)
        log.info("Created lag handle - %s" % self.lag)
        for port in self.port_list[1:]:
            status = self.add_lag_member(device, self.lag, port)
            self.assertTrue(status)

        self.lag_iface = self.add_logical_l2lag(device, self.lag)
        self.vlan = self.add_vlan(device, 10)
        log.info("Created vlan %s ... " % self.vlan)

        for intf in [self.iface, self.lag_iface]:
            status = self.add_vlan_member(device, self.vlan, intf)
            self.assertTrue(status)

        status = self.add_mac_table_entry(
            device, self.vlan, '00:11:11:11:11:11', 2, self.lag_iface)
        self.assertTrue(status)
        status = self.add_mac_table_entry(device, self.vlan,
                                          '00:22:22:22:22:22', 2, self.iface)
        self.assertTrue(status)

        try:
            count = [0 for i in self.port_list[1:self.max_ports]]
            dst_ip = int(socket.inet_aton('10.10.10.1').encode('hex'), 16)
            src_ip = int(socket.inet_aton('192.168.8.1').encode('hex'), 16)
            max_itrs = 200
            for i in range(0, max_itrs):
                dst_ip_addr = socket.inet_ntoa(
                    format(dst_ip, 'x').zfill(8).decode('hex'))
                src_ip_addr = socket.inet_ntoa(
                    format(src_ip, 'x').zfill(8).decode('hex'))
                pkt = simple_tcp_packet(
                    eth_dst='00:11:11:11:11:11',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src=src_ip_addr,
                    ip_id=109,
                    ip_ttl=64)

                exp_pkt = simple_tcp_packet(
                    eth_dst='00:11:11:11:11:11',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src=src_ip_addr,
                    ip_id=109,
                    ip_ttl=64)

                send_packet(self, swports[0], str(pkt))
                rcv_idx = verify_any_packet_any_port(self, [
                    exp_pkt
                    for i in range(0, len(self.port_list[1:self.max_ports]))
                ], swports[1:])
                log.info("rcv_idx - %s" % rcv_idx)
                count[rcv_idx] += 1
                dst_ip += 1
                src_ip += 1

            log.info('L2LagTest: %s' % count)
            for i in range(0, len(self.port_list[1:])):
                self.assertTrue((count[i] >= (
                    (max_itrs / len(self.port_list[1:self.max_ports])) * 0.6)),
                                "Not all paths are equally balanced")
        finally:
            pass

        # Now remove a couple of interfaces from Lag and verify traffic
        for port in self.port_list[-2:]:
            status = self.remove_lag_member(device, self.lag, port)
            self.assertTrue(status)

        try:
            count = [0 for i in self.port_list[1:-2]]
            dst_ip = int(socket.inet_aton('10.10.10.1').encode('hex'), 16)
            src_ip = int(socket.inet_aton('192.168.8.1').encode('hex'), 16)
            max_itrs = 200
            for i in range(0, max_itrs):
                dst_ip_addr = socket.inet_ntoa(
                    format(dst_ip, 'x').zfill(8).decode('hex'))
                src_ip_addr = socket.inet_ntoa(
                    format(src_ip, 'x').zfill(8).decode('hex'))
                pkt = simple_tcp_packet(
                    eth_dst='00:11:11:11:11:11',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src=src_ip_addr,
                    ip_id=109,
                    ip_ttl=64)

                exp_pkt = simple_tcp_packet(
                    eth_dst='00:11:11:11:11:11',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src=src_ip_addr,
                    ip_id=109,
                    ip_ttl=64)

                send_packet(self, swports[0], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self,
                    [exp_pkt for i in range(0, len(self.port_list[1:-2]))],
                    swports[1:-2])
                log.info("rcv_idx - %s" % rcv_idx)
                count[rcv_idx] += 1
                dst_ip += 1
                src_ip += 1

            log.info('L2LagTest: %s' % count)
            for i in range(0, len(self.port_list[1:-2])):
                self.assertTrue(
                    (count[i] >=
                     ((max_itrs / len(self.port_list[1:-2])) * 0.6)),
                    "Not all paths are equally balanced")
        finally:
            pass

        # Now remove a couple of interfaces from Lag and verify traffic
        for port in self.port_list[-2:]:
            status = self.add_lag_member(device, self.lag, port)
            self.assertTrue(status)

        # Check the traffic distribution again
        try:
            count = [0 for i in self.port_list[1:self.max_ports]]
            dst_ip = int(socket.inet_aton('10.10.10.1').encode('hex'), 16)
            src_ip = int(socket.inet_aton('192.168.8.1').encode('hex'), 16)
            max_itrs = 200
            for i in range(0, max_itrs):
                dst_ip_addr = socket.inet_ntoa(
                    format(dst_ip, 'x').zfill(8).decode('hex'))
                src_ip_addr = socket.inet_ntoa(
                    format(src_ip, 'x').zfill(8).decode('hex'))
                pkt = simple_tcp_packet(
                    eth_dst='00:11:11:11:11:11',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src=src_ip_addr,
                    ip_id=109,
                    ip_ttl=64)

                exp_pkt = simple_tcp_packet(
                    eth_dst='00:11:11:11:11:11',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src=src_ip_addr,
                    ip_id=109,
                    ip_ttl=64)

                send_packet(self, swports[0], str(pkt))
                rcv_idx = verify_any_packet_any_port(self, [
                    exp_pkt
                    for i in range(0, len(self.port_list[1:self.max_ports]))
                ], swports[1:])
                log.info("rcv_idx - %s" % rcv_idx)
                count[rcv_idx] += 1
                dst_ip += 1
                src_ip += 1

            log.info('L2LagTest: %s' % count)
            for i in range(0, len(self.port_list[1:])):
                self.assertTrue((count[i] >= (
                    (max_itrs / len(self.port_list[1:self.max_ports])) * 0.6)),
                                "Not all paths are equally balanced")
        finally:
            pass

    def tearDown(self):
        self.cleanup()
        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


class L3Ipv4LagTableFill(ApiAdapter):
    def setUp(self):
        ApiAdapter.setUp(self)
        self.rmac = self.add_rmac(device, '00:77:66:55:44:33')
        self.vrf = self.add_vrf(device, 2)
        self.sw_ports = swports
        try:
            self.sw_ports.pop(cpu_port)  # Remove cpu port from list
        except IndexError:
            log.info("Looks like cpu port %s is not there in swports list " %
                     cpu_port)
        # TODO: Make this readable from
        self.max_ports = len(
            self.sw_ports) - 1  # -1 because we need to remove CPU port
        self.max_ports = 7  # For now testing purpose making it 7
        self.port_list = create_port_handles(device, self.client,
                                             range(0, self.max_ports), log)

    def runTest(self):
        log.info("Started L3Ipv4LagTableFill ....")
        self.iface = self.create_l3_rif(device, self.vrf, self.rmac,
                                        self.port_list[0], '192.168.0.2')
        log.info("Created l3 interface - %s" % self.iface)

        self.lag = self.add_lag(device)
        log.info("Created lag - %s" % self.lag)

        for port in self.port_list[1:]:
            status = self.add_lag_member(device, self.lag, port)
            self.assertTrue(status)

        # Now create the handles for the lag
        self.lag_rif = self.add_logical_l3intf(device, self.vrf, self.rmac)
        self.lag_iface = self.cfg_l3intf_on_port(device, self.lag,
                                                 self.lag_rif)
        self.cfg_ip_address(device, self.lag_rif, self.vrf, "10.0.0.2")

        # Creating 200 hosts
        self.ip_dst_list = [
            "%s.%s.%s.%s" % (10, 10, 10, i) for i in range(1, 201)
        ]

        for ip in self.ip_dst_list:
            nhop = self.add_l3_nhop(device, self.lag_rif, ip,
                                    '00:11:22:33:44:55')
            status = self.add_static_route(
                device, self.vrf, ip, nhop, prefix_length=32)
            self.assertTrue(status)

        try:
            count = [0 for i in self.port_list[1:self.max_ports]]
            max_itrs = 200
            for i in range(0, max_itrs):
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=self.ip_dst_list[i],
                    ip_src='192.168.0.1',
                    ip_id=110,
                    ip_ttl=64)

                exp_pkt = simple_tcp_packet(
                    eth_dst='00:11:22:33:44:55',
                    eth_src='00:77:66:55:44:33',
                    ip_dst=self.ip_dst_list[i],
                    ip_src='192.168.0.1',
                    ip_id=110,
                    ip_ttl=63)
                send_packet(self, self.sw_ports[0], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self, [exp_pkt], self.sw_ports[1:self.max_ports])
                log.info("rcv_idx - %s" % rcv_idx)
                count[rcv_idx] += 1

            print 'L3LagTest:', count
            for i in range(0, self.max_ports - 1):
                self.assertTrue((count[i] >= (
                    (max_itrs / len(self.port_list[1:self.max_ports])) * 0.6)),
                                "Not all paths are equally balanced")

        finally:
            pass

        for port in self.port_list[-2:]:
            status = self.remove_lag_member(device, self.lag, port)
            self.assertTrue(status)

        try:
            count = [0 for i in self.port_list[1:self.max_ports - 2]]
            max_itrs = 200
            for i in range(0, max_itrs):
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=self.ip_dst_list[i],
                    ip_src='192.168.0.1',
                    ip_id=110,
                    ip_ttl=64)

                exp_pkt = simple_tcp_packet(
                    eth_dst='00:11:22:33:44:55',
                    eth_src='00:77:66:55:44:33',
                    ip_dst=self.ip_dst_list[i],
                    ip_src='192.168.0.1',
                    ip_id=110,
                    ip_ttl=63)
                send_packet(self, self.sw_ports[0], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self, [exp_pkt], self.sw_ports[1:self.max_ports - 2])
                log.info("rcv_idx - %s" % rcv_idx)
                count[rcv_idx] += 1

            print 'L3LagTest:', count
            for i in range(0, self.max_ports - 3):
                self.assertTrue(
                    (count[i] >=
                     ((max_itrs / len(self.port_list[1:self.max_ports - 2])) *
                      0.6)), "Not all paths are equally balanced")

        finally:
            pass

        for port in self.port_list[-2:]:
            status = self.add_lag_member(device, self.lag, port)
            self.assertTrue(status)

        try:
            count = [0 for i in self.port_list[1:self.max_ports]]
            max_itrs = 200
            for i in range(0, max_itrs):
                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=self.ip_dst_list[i],
                    ip_src='192.168.0.1',
                    ip_id=110,
                    ip_ttl=64)

                exp_pkt = simple_tcp_packet(
                    eth_dst='00:11:22:33:44:55',
                    eth_src='00:77:66:55:44:33',
                    ip_dst=self.ip_dst_list[i],
                    ip_src='192.168.0.1',
                    ip_id=110,
                    ip_ttl=63)
                send_packet(self, self.sw_ports[0], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self, [exp_pkt], self.sw_ports[1:self.max_ports])
                log.info("rcv_idx - %s" % rcv_idx)
                count[rcv_idx] += 1

            print 'L3LagTest:', count
            for i in range(0, self.max_ports - 1):
                self.assertTrue((count[i] >= (
                    (max_itrs / len(self.port_list[1:self.max_ports])) * 0.6)),
                                "Not all paths are equally balanced")

        finally:
            pass

    def tearDown(self):
        self.cleanup()
        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


class L3Ipv6LagTableFill(ApiAdapter):
    def setUp(self):
        ApiAdapter.setUp(self)
        self.rmac = self.add_rmac(device, '00:77:66:55:44:33')
        self.vrf = self.add_vrf(device, 2)
        self.sw_ports = swports
        try:
            self.sw_ports.pop(cpu_port)  # Remove cpu port from list
        except IndexError:
            log.info("Looks like cpu port %s is not there in swports list " %
                     cpu_port)
        # TODO: Make this readable from
        self.max_ports = len(
            self.sw_ports) - 1  # -1 because we need to remove CPU port
        self.max_ports = 7  # For now testing purpose making it 7
        self.port_list = create_port_handles(device, self.client,
                                             range(0, self.max_ports), log)

    def runTest(self):
        log.info("Started L3Ipv6LagTableFill ....")
        self.iface = self.create_l3_rif(
            device,
            self.vrf,
            self.rmac,
            self.port_list[0],
            '5001::10',
            v4=False)
        log.info("Created l3 interface - %s" % self.iface)

        self.lag = self.add_lag(device)
        log.info("Created lag - %s" % self.lag)

        for port in self.port_list[1:]:
            status = self.add_lag_member(device, self.lag, port)
            self.assertTrue(status)

        # Now create the handles for the lag
        self.lag_rif = self.add_logical_l3intf(device, self.vrf, self.rmac)
        self.lag_iface = self.cfg_l3intf_on_port(device, self.lag,
                                                 self.lag_rif)
        self.cfg_ip_address(
            device, self.lag_rif, self.vrf, "4001::10", v4=False)

        # Creating 200 hosts
        self.ip_dst_list = ["%s::%s" % (2000 + i, 1) for i in range(1, 201)]

        for ip in self.ip_dst_list:
            nhop = self.add_l3_nhop(
                device, self.lag_rif, ip, '00:11:22:33:44:55', v4=False)
            status = self.add_static_route(
                device, self.vrf, ip, nhop, prefix_length=128, v4=False)
            self.assertTrue(status)

        try:
            count = [0 for i in self.port_list[1:self.max_ports]]
            max_itrs = 200
            for i in range(0, max_itrs):
                pkt = simple_tcpv6_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ipv6_dst=self.ip_dst_list[i],
                    ipv6_src='5001::1',
                    ipv6_hlim=64)

                exp_pkt = simple_tcpv6_packet(
                    eth_dst='00:11:22:33:44:55',
                    eth_src='00:77:66:55:44:33',
                    ipv6_dst=self.ip_dst_list[i],
                    ipv6_src='5001::1',
                    ipv6_hlim=63)
                send_packet(self, self.sw_ports[0], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self, [exp_pkt], self.sw_ports[1:self.max_ports])
                log.info("rcv_idx - %s" % rcv_idx)
                count[rcv_idx] += 1

            print 'L3LagTest:', count
            for i in range(0, self.max_ports - 1):
                self.assertTrue((count[i] >= (
                    (max_itrs / len(self.port_list[1:self.max_ports])) * 0.6)),
                                "Not all paths are equally balanced")

        finally:
            pass

        for port in self.port_list[-2:]:
            status = self.remove_lag_member(device, self.lag, port)
            self.assertTrue(status)

        try:
            count = [0 for i in self.port_list[1:self.max_ports - 2]]
            max_itrs = 200
            for i in range(0, max_itrs):
                pkt = simple_tcpv6_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ipv6_dst=self.ip_dst_list[i],
                    ipv6_src='5001::1',
                    ipv6_hlim=64)

                exp_pkt = simple_tcpv6_packet(
                    eth_dst='00:11:22:33:44:55',
                    eth_src='00:77:66:55:44:33',
                    ipv6_dst=self.ip_dst_list[i],
                    ipv6_src='5001::1',
                    ipv6_hlim=63)
                send_packet(self, self.sw_ports[0], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self, [exp_pkt], self.sw_ports[1:self.max_ports - 2])
                log.info("rcv_idx - %s" % rcv_idx)
                count[rcv_idx] += 1

            print 'L3LagTest:', count
            for i in range(0, self.max_ports - 3):
                self.assertTrue(
                    (count[i] >=
                     ((max_itrs / len(self.port_list[1:self.max_ports - 2])) *
                      0.6)), "Not all paths are equally balanced")

        finally:
            pass

        for port in self.port_list[-2:]:
            status = self.add_lag_member(device, self.lag, port)
            self.assertTrue(status)

        try:
            count = [0 for i in self.port_list[1:self.max_ports]]
            max_itrs = 200
            for i in range(0, max_itrs):
                pkt = simple_tcpv6_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ipv6_dst=self.ip_dst_list[i],
                    ipv6_src='5001::1',
                    ipv6_hlim=64)

                exp_pkt = simple_tcpv6_packet(
                    eth_dst='00:11:22:33:44:55',
                    eth_src='00:77:66:55:44:33',
                    ipv6_dst=self.ip_dst_list[i],
                    ipv6_src='5001::1',
                    ipv6_hlim=63)
                send_packet(self, self.sw_ports[0], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self, [exp_pkt], self.sw_ports[1:self.max_ports])
                log.info("rcv_idx - %s" % rcv_idx)
                count[rcv_idx] += 1

            print 'L3LagTest:', count
            for i in range(0, self.max_ports - 1):
                self.assertTrue((count[i] >= (
                    (max_itrs / len(self.port_list[1:self.max_ports])) * 0.6)),
                                "Not all paths are equally balanced")

        finally:
            pass

    def tearDown(self):
        self.cleanup()
        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


class L3IPv4EcmpLagTableFill(ApiAdapter):
    def setUp(self):
        ApiAdapter.setUp(self)
        self.rmac = self.add_rmac(device, '00:77:66:55:44:33')
        self.vrf = self.add_vrf(device, 2)
        self.sw_ports = swports
        try:
            self.sw_ports.pop(cpu_port)  # Remove cpu port from list
        except IndexError:
            log.info("Looks like cpu port %s is not there in swports list " %
                     cpu_port)
        # TODO: Make this readable from
        self.max_ports = len(
            self.sw_ports) - 1  # -1 because we need to remove CPU port
        self.max_ports = 7  # For now testing purpose making it 7
        self.port_list = create_port_handles(device, self.client,
                                             range(0, self.max_ports), log)
        # Now we need to split into 3 lists
        # with atleast 2 list having more than 1 port
        self.src_port = self.port_list[0]
        self.src_rif = None
        self.last_port = self.port_list[self.max_ports - 1]
        self.last_rif = None
        self.lag1_port_list = self.port_list[1:self.max_ports / 2]
        self.lag2_port_list = self.port_list[self.max_ports / 2:
                                             self.max_ports - 1]

    def runTest(self):
        log.info("Started L3IPv4EcmpLagTableFill test ...")
        # Create Src rif
        self.src_intf = self.create_l3_rif(device, self.vrf, self.rmac,
                                           self.port_list[0], '192.168.0.2')
        log.info("Created l3 interface - %s" % self.src_intf)

        # Create lag related things
        self.lag1 = self.add_lag(device)
        log.info("Created lag - %s " % self.lag1)
        for port in self.lag1_port_list:
            status = self.add_lag_member(device, self.lag1, port)
            self.assertTrue(status)

        # Now create the handles for the lag
        self.lag1_rif = self.add_logical_l3intf(device, self.vrf, self.rmac)
        self.lag1_intf = self.cfg_l3intf_on_port(device, self.lag1,
                                                 self.lag1_rif)
        status = self.cfg_ip_address(
            device, self.lag1_rif, self.vrf, "10.0.2.2", prefix_length=32)
        self.assertTrue(status)

        # Create lag related things
        self.lag2 = self.add_lag(device)
        log.info("Created lag - %s " % self.lag2)
        for port in self.lag2_port_list:
            status = self.add_lag_member(device, self.lag2, port)
            self.assertTrue(status)

        # Now create the handles for the lag
        self.lag2_rif = self.add_logical_l3intf(device, self.vrf, self.rmac)
        self.lag2_intf = self.cfg_l3intf_on_port(device, self.lag2,
                                                 self.lag2_rif)
        status = self.cfg_ip_address(
            device, self.lag2_rif, self.vrf, "10.0.3.2", prefix_length=32)
        self.assertTrue(status)

        # Now create one for last intf
        self.dst_intf = self.create_l3_rif(device, self.vrf, self.rmac,
                                           self.last_port, '10.0.4.2')
        log.info("Created l3 interface - %s" % self.dst_intf)

        # 3 because we have lag1, lag3 and dst_intf
        self.host_ip = '10.100.0.0'
        self.mac_list = ['00:11:22:33:44:%s' % (55 + i) for i in range(0, 3)]
        self.intf_rif_list = [self.lag1_rif, self.lag2_rif, self.dst_intf]
        self.nhop_list = []

        for mac, iface in zip(self.mac_list, self.intf_rif_list):
            nhop = self.add_nhop(device, iface, self.host_ip)
            self.nhop_list.append(nhop)
            neighbor = self.add_neighbor_l3intf(device, nhop, mac)

        self.ecmp = self.add_ecmp(device)
        log.info("Created ecmp - %s ... " % self.ecmp)
        status = self.add_ecmp_member(device, self.ecmp,
                                      len(self.nhop_list), self.nhop_list)
        self.assertTrue(status)
        status = self.add_static_route(
            device, self.vrf, self.host_ip, self.ecmp, prefix_length=16)
        self.assertTrue(status)

        try:
            count = [0 for i in range(1, self.max_ports)]
            dst_ip = int(socket.inet_aton('10.100.10.1').encode('hex'), 16)
            src_ip = int(socket.inet_aton('192.168.8.1').encode('hex'), 16)
            max_itrs = 500
            for i in range(0, max_itrs):
                dst_ip_addr = socket.inet_ntoa(
                    format(dst_ip, 'x').zfill(8).decode('hex'))
                src_ip_addr = socket.inet_ntoa(
                    format(src_ip, 'x').zfill(8).decode('hex'))

                pkt = simple_tcp_packet(
                    eth_dst='00:77:66:55:44:33',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src=src_ip_addr,
                    ip_id=106,
                    ip_ttl=64)

                exp_pkt_list = [
                    simple_tcp_packet(
                        eth_dst=mac,
                        eth_src='00:77:66:55:44:33',
                        ip_dst=dst_ip_addr,
                        ip_src=src_ip_addr,
                        ip_id=106,
                        ip_ttl=63) for mac in self.mac_list
                ]

                send_packet(self, swports[0], str(pkt))
                rcv_idx = verify_any_packet_any_port(self, exp_pkt_list,
                                                     swports[1:self.max_ports])
                log.info("rcv_idx: %s" % rcv_idx)
                count[rcv_idx] += 1
                dst_ip += 1
                src_ip += 1

            log.info("Ecmp-count : %s" % count)
        finally:
            pass

    def tearDown(self):
        self.cleanup()
        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


class L2LagMiscTest(ApiAdapter):
    def setUp(self):
        ApiAdapter.setUp(self)
        self.vlan = self.add_vlan(device, 10)
        # Pop off cpu port from swport list
        try:
            swports.pop(cpu_port)
        except IndexError:
            log.info('Looks like cpu port - %s' % cpu_port)

        self.sw_ports = swports[:8]
        self.port_list = create_port_handles(device, self.client,
                                             range(0, len(self.sw_ports)), log)
        self.max_lag, _ = return_max_min_table_size_seen_based_on_table_sizes_api(
            device, self.client, 'lag')
        if self.max_lag != None:
            self.max_lag = self.max_lag - int(self.max_lag * TOLERANCE)
            self.max_lag = self.max_lag / 64
        else:
            self.max_lag = 0

    def runTest(self):
        self.max_lag = 20
        self.lag_list = [
            self.add_lag(device) for i in range(0, self.max_lag - 2)
        ]
        self.src_intf = self.cfg_l2intf_on_port(device, self.port_list[0])
        self.lag_to_test = self.add_lag(device)
        for port in self.port_list[1:]:
            status = self.add_lag_member(device, self.lag_to_test, port)
            self.assertTrue(status)
        self.lag_intf = self.add_logical_l2lag(device, self.lag_to_test)
        for intf in [self.src_intf, self.lag_intf]:
            status = self.add_vlan_member(device, self.vlan, intf)
            self.assertTrue(status)

        status = self.add_mac_table_entry(
            device, self.vlan, '00:11:11:11:11:11', 2, self.lag_intf)
        self.assertTrue(status)
        status = self.add_mac_table_entry(
            device, self.vlan, '00:22:22:22:22:22', 2, self.src_intf)
        self.assertTrue(status)

        try:
            count = [0 for i in self.port_list[1:]]
            dst_ip = int(socket.inet_aton('10.10.10.1').encode('hex'), 16)
            src_ip = int(socket.inet_aton('192.168.8.1').encode('hex'), 16)
            max_itrs = 200
            for i in range(0, max_itrs):
                dst_ip_addr = socket.inet_ntoa(
                    format(dst_ip, 'x').zfill(8).decode('hex'))
                src_ip_addr = socket.inet_ntoa(
                    format(src_ip, 'x').zfill(8).decode('hex'))
                pkt = simple_tcp_packet(
                    eth_dst='00:11:11:11:11:11',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src=src_ip_addr,
                    ip_id=109,
                    ip_ttl=64)

                exp_pkt = simple_tcp_packet(
                    eth_dst='00:11:11:11:11:11',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src=src_ip_addr,
                    ip_id=109,
                    ip_ttl=64)

                send_packet(self, self.sw_ports[0], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self, [exp_pkt
                           for i in self.port_list[1:]], self.sw_ports[1:])
                count[rcv_idx] += 1
                log.info("rcv_idx - %s ..." % rcv_idx)
                dst_ip += 1
                src_ip += 1

            log.info('L2LagTest: %s' % count)
            for i in range(0, len(self.port_list[1:])):
                self.assertTrue((count[i] >=
                                 ((max_itrs / len(self.port_list[1:])) * 0.6)),
                                "Not all paths are equally balanced")
        finally:
            pass

        # Now remove a bunch of interfaces and add them to vlan and send traffic and verify
        for port in self.port_list[-2:]:
            status = self.remove_lag_member(device, self.lag_to_test, port)
            self.assertTrue(status)

        if1 = self.cfg_l2intf_on_port(device, self.port_list[-2])
        if2 = self.cfg_l2intf_on_port(device, self.port_list[-1])
        for intf in [if1, if2]:
            status = self.add_vlan_member(device, self.vlan, intf)
            self.assertTrue(status)
        status = self.add_mac_table_entry(device, self.vlan,
                                          '00:33:33:33:33:33', 2, if1)
        self.assertTrue(status)
        status = self.add_mac_table_entry(device, self.vlan,
                                          '00:44:44:44:44:44', 2, if2)
        self.assertTrue(status)

        try:
            log.info("Sending traffic from %s --> %s" % ('00:33:33:33:33:33',
                                                         '00:11:11:11:11:11'))
            pkt = simple_tcp_packet(
                eth_src='00:33:33:33:33:33',
                eth_dst='00:11:11:11:11:11',
                ip_dst='10.0.0.1',
                ip_id=109,
                ip_ttl=64)
            exp = simple_tcp_packet(
                eth_src='00:33:33:33:33:33',
                eth_dst='00:11:11:11:11:11',
                ip_dst='10.0.0.1',
                ip_id=109,
                ip_ttl=64)

            send_packet(self, self.sw_ports[-2], str(pkt))
            verify_any_packet_any_port(
                self, [exp for i in self.port_list[1:-2]], self.sw_ports[1:-2])
        finally:
            pass

        # Remove lag
        status = self.remove_mac_table_entry(
            device, self.vlan, '00:11:11:11:11:11', 2, self.lag_intf)
        self.assertTrue(status)
        status = self.no_vlan_member(device, self.vlan, self.lag_intf)
        self.assertTrue(status)
        for port in self.port_list[1:-2]:
            status = self.remove_lag_member(device, self.lag_to_test, port)
            self.assertTrue(status)
        status = self.remove_l2intf_on_port(device, self.lag_intf)
        self.assertTrue(status)
        status = self.remove_lag(device, self.lag_to_test)
        self.assertTrue(status)

        try:
            log.info("Sending traffic from %s --> %s" % ('00:33:33:33:33:33',
                                                         '00:11:11:11:11:11'))
            pkt = simple_tcp_packet(
                eth_src='00:33:33:33:33:33',
                eth_dst='00:22:22:22:22:22',
                ip_dst='10.0.0.1',
                ip_id=109,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_src='00:33:33:33:33:33',
                eth_dst='00:22:22:22:22:22',
                ip_dst='10.0.0.1',
                ip_id=109,
                ip_ttl=64)

            send_packet(self, self.sw_ports[-2], str(pkt))
            verify_packets(self, exp_pkt, [self.sw_ports[0]])
        finally:
            pass

        # remove the recently created interfaces
        status = self.remove_mac_table_entry(device, self.vlan,
                                             '00:33:33:33:33:33', 2, if1)
        self.assertTrue(status)
        status = self.remove_mac_table_entry(device, self.vlan,
                                             '00:44:44:44:44:44', 2, if2)
        self.assertTrue(status)
        for intf in [if1, if2]:
            status = self.no_vlan_member(device, self.vlan, intf)
            self.assertTrue(status)
        for intf in [if1, if2]:
            status = self.remove_l2intf_on_port(device, intf)
            self.assertTrue(status)

        # Add back the lag and verify the traffic.
        self.lag_to_test = self.add_lag(device)
        for port in self.port_list[1:]:
            status = self.add_lag_member(device, self.lag_to_test, port)
            self.assertTrue(status)
        self.lag_intf = self.add_logical_l2lag(device, self.lag_to_test)
        status = self.add_vlan_member(device, self.vlan, self.lag_intf)
        self.assertTrue(status)

        status = self.add_mac_table_entry(
            device, self.vlan, '00:11:11:11:11:11', 2, self.lag_intf)
        self.assertTrue(status)

        try:
            count = [0 for i in self.port_list[1:]]
            dst_ip = int(socket.inet_aton('10.10.10.1').encode('hex'), 16)
            src_ip = int(socket.inet_aton('192.168.8.1').encode('hex'), 16)
            max_itrs = 200
            for i in range(0, max_itrs):
                dst_ip_addr = socket.inet_ntoa(
                    format(dst_ip, 'x').zfill(8).decode('hex'))
                src_ip_addr = socket.inet_ntoa(
                    format(src_ip, 'x').zfill(8).decode('hex'))
                pkt = simple_tcp_packet(
                    eth_dst='00:11:11:11:11:11',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src=src_ip_addr,
                    ip_id=109,
                    ip_ttl=64)

                exp_pkt = simple_tcp_packet(
                    eth_dst='00:11:11:11:11:11',
                    eth_src='00:22:22:22:22:22',
                    ip_dst=dst_ip_addr,
                    ip_src=src_ip_addr,
                    ip_id=109,
                    ip_ttl=64)

                send_packet(self, self.sw_ports[0], str(pkt))
                rcv_idx = verify_any_packet_any_port(
                    self, [exp_pkt
                           for i in self.port_list[1:]], self.sw_ports[1:])
                count[rcv_idx] += 1
                log.info("rcv_idx - %s ..." % rcv_idx)
                dst_ip += 1
                src_ip += 1

            log.info('L2LagTest: %s' % count)
            for i in range(0, len(self.port_list[1:])):
                self.assertTrue((count[i] >=
                                 ((max_itrs / len(self.port_list[1:])) * 0.6)),
                                "Not all paths are equally balanced")
        finally:
            pass

    def tearDown(self):
        self.cleanup()
        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)


class CmpContextSwitchApi(systestBaseTests):
    def runTest(self):
        log.info("Print out the table names and sizes seen from switch api")
        print_switch_tables(device, self.client)

        self.feature_list = [
            'smac', 'rmac', 'vlan', 'lag', 'bd flood', 'acl', 'ipv4 lpm',
            'ipv6 lpm', 'mcast', 'ecmp', 'qos'
        ]
        self.switch_table_list = [
            return_max_min_table_size_seen_based_on_table_sizes_api(
                device, self.client, feature) for feature in self.feature_list
        ]
        self.context_json_list = [
            (context.getMaxTableSizeBasedOnFeatureName(feature),
             context.getMinTableSizeBasedOnFeatureName(feature))
            for feature in self.feature_list
        ]
        print " Printing the comparison table"
        print "%s" % ("-" * 63)
        print "|{:>30}|{:>15}|{:>15}|".format('Feature', 'Switch Api',
                                              'Context Json')
        print "%s" % ("-" * 63)
        for f, s, t in zip(self.feature_list, self.switch_table_list,
                           self.context_json_list):
            print "|{:>30}|{:>15}|{:>15}|".format(f, s, t)
        print "%s" % ("-" * 63)


class AclFillAllPorts(ApiAdapter):
    def setUp(self):
        ApiAdapter.setUp(self)
        self.rmac = self.add_rmac(device, '00:77:66:55:44:33')
        self.vrf = self.add_vrf(device, 2)
        self.sw_ports = swports
        try:
            self.sw_ports.pop(cpu_port)  # Remove cpu port from list
        except IndexError:
            log.info("Looks like cpu port %s is not there in swports list " %
                     cpu_port)
        # TODO: Make this readable from
        self.max_ports = len(
            self.sw_ports) - 1  # -1 because we need to remove CPU port
        # self.max_ports = 7  # For now testing purpose making it 7
        self.port_list = create_port_handles(device, self.client,
                                             range(0, self.max_ports), log)
        self.ip_list = []
        self.rif_list = []
        # Creating and attaching the ip address to the interfaces
        for i in range(0, len(self.port_list)):
            ip = "%s.%s.%s.%s" % ((i + 1), (i + 1), (i + 1), (i + 2))
            self.ip_list.append(ip)
            rif = self.create_l3_rif(device, self.vrf, self.rmac,
                                     self.port_list[i], ip)
            self.rif_list.append(rif)

    def runTest(self):
        log.info("Setup some static routes")
        max_acl = len(self.sw_ports[1:])
        mac_list = generate_random_mac_address(max_acl - 1)
        # Generate randon ip address
        ip_host_list = generate_random_ip_address(max_acl - 1)
        ip_host_list = [ip for ip in ip_host_list if ip not in self.ip_list]
        ip_host_int_list = [
            int(socket.inet_aton(ip).encode('hex'), 16) for ip in ip_host_list
        ]
        ip_mask_list = [int("ffffffff", 16) for ip in ip_host_list]
        self.ipaddr_handle_list = []
        self.nhop_list = []
        self.neighbor_list = []
        for rintf, ip, mac in zip(self.rif_list[1:], ip_host_list, mac_list):
            nhop = self.add_l3_nhop(device, rintf, ip, mac, prefix_length=32)
            status = self.add_static_route(
                device, self.vrf, ip, nhop, prefix_length=32)
            self.assertTrue(status)

        for port, ip, mac in zip(self.sw_ports[1:], ip_host_list, mac_list):
            log.info("Sending traffic to ip %s from %s" % (ip,
                                                           self.ip_list[0]))
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst=ip,
                ip_src=self.ip_list[0],
                ip_id=105,
                pktlen=9100,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst=mac,
                eth_src='00:77:66:55:44:33',
                ip_dst=ip,
                ip_src=self.ip_list[0],
                ip_id=105,
                pktlen=9100,
                ip_ttl=63)
            try:
                send_packet(self, self.sw_ports[0], str(pkt))
                verify_packets(self, exp_pkt, [port])
            finally:
                pass

        # Now configure an acl to deny
        self.acl_list = []
        self.ace_list = []
        self.acl_port_list = []
        for port, ip, mask in zip(self.sw_ports[1:], ip_host_int_list,
                                  ip_mask_list):
            acl = self.client.switch_api_acl_list_create(
                device,
                SWITCH_API_DIRECTION_INGRESS,
                0,  # Find out what does this mean.
                SWITCH_HANDLE_TYPE_PORT)
            # create kvp to match destination IP
            kvp = []
            kvp_val = switcht_acl_value_t(value_num=ip)
            kvp_mask = switcht_acl_value_t(value_num=mask)
            kvp.append(
                switcht_acl_key_value_pair_t(SWITCH_ACL_IP_FIELD_IPV4_DEST,
                                             kvp_val, kvp_mask))
            kvp_val = switcht_acl_value_t(value_num=int("2", 16))
            kvp_mask = switcht_acl_value_t(value_num=int("ff", 16))
            kvp.append(
                switcht_acl_key_value_pair_t(SWITCH_ACL_IP_FIELD_TCP_FLAGS,
                                             kvp_val, kvp_mask))

            action = 1
            action_params = switcht_acl_action_params_t(
                redirect=switcht_acl_action_redirect(handle=0))
            opt_action_params = switcht_acl_opt_action_params_t()
            ace = self.client.switch_api_acl_ip_rule_create(
                device, acl, 10, 2, kvp, action, action_params,
                opt_action_params)
            self.port = self.client.switch_api_port_id_to_handle_get(0, port)
            status = self.client.switch_api_acl_reference(0, acl, self.port)
            log.debug("Status - %s" % status)
            self.assertEqual(status, 0)
            log.info("Added acl for ip - %s " % ip)
            self.acl_list.append(acl)
            self.ace_list.append(ace)
            self.acl_port_list.append(self.port)

        # TODO: This needs fixing . Make acl to be egress driven
        for port, ip, mac in zip(self.sw_ports[1:], ip_host_list, mac_list):
            log.info("Sending traffic to ip %s from %s" % (ip,
                                                           self.ip_list[0]))
            pkt = simple_tcp_packet(
                eth_dst='00:77:66:55:44:33',
                eth_src='00:22:22:22:22:22',
                ip_dst=ip,
                ip_src=self.ip_list[0],
                ip_id=105,
                pktlen=9100,
                ip_ttl=64)
            exp_pkt = simple_tcp_packet(
                eth_dst=mac,
                eth_src='00:77:66:55:44:33',
                ip_dst=ip,
                ip_src=self.ip_list[0],
                ip_id=105,
                pktlen=9100,
                ip_ttl=63)
            try:
                send_packet(self, self.sw_ports[0], str(pkt))
                verify_packets(self, exp_pkt, [port])
            finally:
                pass

        for port, acl, ace in zip(self.acl_port_list, self.acl_list,
                                  self.ace_list):
            status = self.client.switch_api_acl_dereference(0, acl, port)
            log.debug("Status - %s" % status)
            self.assertEqual(status, 0)
            status = self.client.switch_api_acl_rule_delete(0, acl, ace)
            log.debug("Status - %s" % status)
            self.assertEqual(status, 0)
            status = self.client.switch_api_acl_list_delete(0, acl)
            log.debug("Status - %s" % status)
            self.assertEqual(status, 0)
            log.info("Removed acl - %s" % acl)

    def tearDown(self):
        self.cleanup()
        api_base_tests.ThriftInterfaceDataPlane.tearDown(self)
