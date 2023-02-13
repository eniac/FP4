
################################################################################
# BAREFOOT NETWORKS CONFIDENTIAL & PROPRIETARY
#
# Copyright (c) 2015-2016 Barefoot Networks, Inc.

# All Rights Reserved.
#
# NOTICE: All information contained herein is, and remains the property of
# Barefoot Networks, Inc. and its suppliers, if any. The intellectual and
# technical concepts contained herein are proprietary to Barefoot Networks,
# Inc.
# and its suppliers and may be covered by U.S. and Foreign Patents, patents in
# process, and are protected by trade secret or copyright law.
# Dissemination of this information or reproduction of this material is
# strictly forbidden unless prior written permission is obtained from
# Barefoot Networks, Inc.
#
# No warranty, explicit or implicit is provided, unless granted under a
# written agreement with Barefoot Networks, Inc.
#
# $Id: $
#
###############################################################################
'''
Thrift API adapter module
'''

import os
import sys

from ptf.testutils import *
from ptf.thriftutils import *
from switchapi_thrift.ttypes import *
from switchapi_thrift.switch_api_headers import *

this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../base'))
sys.path.append(os.path.join(this_dir, '../../base/api-tests'))

import api_base_tests


def switch_api_mac_table_entry_create(self, device, network, mac, mac_type, handle, **kwargs):
    '''
    Parameters:
        device      int         device id
        network     obj         network object - vlan/ln
        mac         str         mac address
        handle      obj         interface/nhop/tunnel object
        keyword arguments:
            store   bool        True: add to stack for later clean up
    Return:
        bool    True if success
    '''
    params = {
        'network_handle': network,
        'mac_addr': mac,
        'entry_type': mac_type,
        'handle': handle,
    }

    if 'tunnel' in kwargs and kwargs['tunnel'] == True:
      ip = self.make_ip_addr(kwargs['tunnel_ip'], host=True, **kwargs)
      params.update({'tunnel_ip': ip})

    mac_entry = switcht_api_mac_entry_t(**params)
    status = self.client.switch_api_mac_table_entry_create(
        device, mac_entry)
    return status == 0

def switch_api_mac_table_entry_update(self, device, network, mac, mac_type, handle, **kwargs):
    '''
    Parameters:
        device      int         device id
        network     obj         network object - vlan/ln
        mac         str         mac address
        handle      obj         interface/nhop/tunnel object
        keyword arguments:
            store   bool        True: add to stack for later clean up
    Return:
        bool    True if success
    '''
    params = {
        'network_handle': network,
        'mac_addr': mac,
        'entry_type': mac_type,
        'handle': handle,
    }

    if 'tunnel' in kwargs and kwargs['tunnel'] == True:
      ip = self.make_ip_addr(kwargs['tunnel_ip'], host=True, **kwargs)
      params.update({'tunnel_ip': ip})

    mac_entry = switcht_api_mac_entry_t(**params)
    status = self.client.switch_api_mac_table_entry_update(
        device, mac_entry)
    return status == 0

def switch_api_mac_table_entry_delete(self, device, network, mac, **kwargs):
    '''
    Parameters:
        device      int         device id
        network     obj         network object - vlan/ln
        mac         str         mac address
        handle      obj         interface/nhop/tunnel object
        keyword arguments:
            store   bool        True: add to stack for later clean up
    Return:
        bool    True if success
    '''
    params = {
        'network_handle': network,
        'mac_addr': mac,
        'entry_type': 0,
        'handle': 0,
    }

    if 'tunnel' in kwargs and kwargs['tunnel'] == True:
      ip = self.make_ip_addr(kwargs['tunnel_ip'], host=True, **kwargs)
      params.update({'tunnel_ip': ip})

    mac_entry = switcht_api_mac_entry_t(**params)
    status = self.client.switch_api_mac_table_entry_delete(
        device, mac_entry)
    return status == 0

def switch_api_l3_nhop_neighbor_create(self, device, rif, ip_addr, mac, **kwargs):
    '''
    Parameters:
        device      int         device id
        rif         obj         router interface object
    Return:
        nhop and neighbor object
    '''

    nhtype = SWITCH_NHOP_TYPE_IP
    nhop_info = switcht_api_nhop_info_t(nhop_type=nhtype, rif_handle=rif, ip_addr=ip_addr)
    nhop = self.client.switch_api_nhop_create(device, nhop_info)
    neighbor_info = switcht_api_neighbor_info_t(
            nhop_handle=nhop,
            rif_handle=0x0,
            mac_addr=mac,
            neighbor_type=SWITCH_NEIGHBOR_TYPE_NHOP,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L3)
    neighbor = self.client.switch_api_neighbor_create(device, neighbor_info)
    return nhop, neighbor

def switch_api_nhop_create(self, device, rif, ip_addr):
    '''
    Parameters:
        device      int         device id
        rif         obj         router interface object
        ip          ip address  nhop ip
    Return:
        nhop object
    '''

    nhtype = SWITCH_NHOP_TYPE_IP
    nhop_info = switcht_api_nhop_info_t(nhop_type=nhtype, rif_handle=rif, ip_addr=ip_addr)
    nhop = self.client.switch_api_nhop_create(device, nhop_info)
    return nhop

def switch_api_neighbor_create(self, device, nhop, mac):
    '''
    Parameters:
        device      int         device id
        nhop        obj         nhop object
        mac         string      mac address
    Return:
        neighbor object
    '''
    neighbor_info = switcht_api_neighbor_info_t(
            nhop_handle=nhop,
            rif_handle=0x0,
            mac_addr=mac,
            neighbor_type=SWITCH_NEIGHBOR_TYPE_NHOP,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L3)
    neighbor = self.client.switch_api_neighbor_create(device, neighbor_info)
    return neighbor


