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

###############################################################################
# GLOBAL VARIABLES
SWITCH_STATUS_SUCCESS = 0
SWITCH_STATUS_FAILURE = 1
SWITCH_STATUS_NOT_SUPPORTED = 2


###############################################################################
class ApiAdapter(api_base_tests.ThriftInterfaceDataPlane):
    ''' Switchapi adapter to configure data plane'''

    def _push(self, keyword, *args, **kwargs):
        if not hasattr(self, 'stack'):
            self.stack = []
        self.stack.append((keyword, args, kwargs))

    def _pop(self, keyword, *args, **kwargs):
        match_this = (keyword, args, kwargs)
        for idx, pack in enumerate(self.stack):
            if pack == match_this:
                del self.stack[idx]
                break

    def _update(self, keyword, *args, **kwargs):
        try:
            if keyword == 'switch_api_l3_route_update':
                this_dev, this_vrf, this_addr, this_nhop = args
                for idx, pack in enumerate(self.stack):
                    i_keyw, i_args, i_kwargs = pack
                    if i_keyw == 'switch_api_l3_route_add':
                        i_dev, i_vrf, i_addr, i_nhop = i_args
                        if (i_dev == this_dev and i_vrf == this_vrf
                                and i_addr == this_addr):
                            self.stack[idx] = (i_keyw, args, kwargs)
                            return True

        except:
            print 'Failed to find and update cleanup stack register'
            return False

    def _clean(self, pack):
        keyword, args, kwargs = pack
        try:
            func = None
            if keyword == 'switch_api_vrf_create':
                func = self.client.switch_api_vrf_delete
            if keyword == 'switch_api_router_mac_group_create':
                func = self.client.switch_api_router_mac_group_delete
            if keyword == 'switch_api_router_mac_add':
                func = self.client.switch_api_router_mac_delete
            if keyword == 'switch_api_rif_create':
                func = self.client.switch_api_rif_delete
            if keyword == 'switch_api_port_bind_mode_set':
                func = self.client.switch_api_port_bind_mode_set
            if keyword == 'switch_api_lag_bind_mode_set':
                func = self.client.switch_api_lag_bind_mode_set
            if keyword == 'switch_api_interface_create':
                func = self.client.switch_api_interface_delete
            if keyword == 'switch_api_l3_interface_address_add':
                func = self.client.switch_api_l3_interface_address_delete
            if keyword == 'switch_api_nhop_create':
                func = self.client.switch_api_nhop_delete
            if keyword == 'switch_api_neighbor_create':
                func = self.client.switch_api_neighbor_delete
            if keyword == 'switch_api_l3_route_add':
                func = self.client.switch_api_l3_route_delete
            if keyword == 'switch_api_vlan_create':
                func = self.client.switch_api_vlan_delete
            if keyword == 'switch_api_lag_create':
                func = self.client.switch_api_lag_delete
            if keyword == 'switch_api_lag_member_add':
                func = self.client.switch_api_lag_member_delete
            if keyword == 'switch_api_vlan_member_add':
                func = self.client.switch_api_vlan_member_remove
            if keyword == 'switch_api_ecmp_create':
                func = self.client.switch_api_ecmp_delete
            if keyword == 'switch_api_ecmp_member_add':
                func = self.client.switch_api_ecmp_member_delete
            if keyword == 'switch_api_mac_table_entry_create':
                func = self.client.switch_api_mac_table_entry_delete
            if keyword == 'switch_api_stp_group_create':
                func = self.client.switch_api_stp_group_delete
            if keyword == 'switch_api_stp_group_member_add':
                func = self.client.switch_api_stp_group_member_remove
            if keyword == 'switch_api_logical_network_create':
                func = self.client.switch_api_logical_network_delete
            if keyword == 'switch_api_tunnel_mapper_create':
                func = self.client.switch_api_tunnel_mapper_delete
            if keyword == 'switch_api_tunnel_mapper_entry_create':
                func = self.client.switch_api_tunnel_mapper_entry_delete
            if keyword == 'switch_api_tunnel_create':
                func = self.client.switch_api_tunnel_delete
            if keyword == 'switch_api_tunnel_term_create':
                func = self.client.switch_api_tunnel_term_delete
            if keyword == 'switch_api_logical_network_member_add':
                func = self.client.switch_api_logical_network_member_remove
            if keyword == 'switch_api_mirror_session_create':
                func = self.client.switch_api_mirror_session_delete

            if func:
                func(*args, **kwargs)
            else:
                print 'Clean up method not found'
        except:
            msg = ('Calling %r with params\n  args  : %s\n  kwargs: %s'
                   '') % (func.__name__, args, kwargs)
            raise ValueError(msg)

    def cleanup(self):
        while self.stack:
            tmp = self.stack.pop()
            self._clean(tmp)

    def add_vrf(self, device, vrf_id, **kwargs):
        '''
        Parameters:
            device      int         device id
            vrf_id      int         vrf id
            keyword arguments:
                store   bool        True: add to stack for later clean up
        Return:
            vrf object
        '''
        vrf = self.client.switch_api_vrf_create(device, vrf_id)
        if kwargs.get('store', True):
            self._push('switch_api_vrf_create', device, vrf)
        return vrf

    def no_vrf(self, device, vrf):
        '''
        Parameters:
            device      int         device id
            vrf         obj         vrf object
        Return:
            bool    True if success
        '''
        status = self.client.switch_api_vrf_delete(device, vrf)
        if status == SWITCH_STATUS_SUCCESS:
            self._pop('switch_api_vrf_create', device, vrf)
        return status == SWITCH_STATUS_SUCCESS

    def add_ecmp(self, device, **kwargs):
        '''
        Parameters:
            device      int         device id
            keyword arguments:
                store   bool        True: add to stack for later clean up
        Return:
            ecmp object
        '''
        ecmp = self.client.switch_api_ecmp_create(device)
        if kwargs.get('store', True):
            self._push('switch_api_ecmp_create', device, ecmp)
        return ecmp

    def no_ecmp(self, device, ecmp):
        '''
        Parameters:
            device      int         device id
            ecmp        obj         ecmp object
        Return:
            bool    True if success
        '''
        status = self.client.switch_api_ecmp_delete(device, ecmp)
        if status == SWITCH_STATUS_SUCCESS:
            self._pop('switch_api_ecmp_create', device, ecmp)
        return status == SWITCH_STATUS_SUCCESS

    def add_ecmp_member(self, device, ecmp, num_nhops, memlist, **kwargs):
        '''
        Parameters:
            device      int         device id
            ecmp        obj         ecmp object id
            vrf_id      int         vrf id
            memlist     objlist     list of ecmp members
            keyword arguments:
                store   bool        True: add to stack for later clean up
        Return:
            bool    True if success
        '''
        status = self.client.switch_api_ecmp_member_add(
            device, ecmp, num_nhops, memlist)
        if status == SWITCH_STATUS_SUCCESS and kwargs.get('store', True):
            self._push('switch_api_ecmp_member_add', device, ecmp, num_nhops,
                       memlist)
        return status == SWITCH_STATUS_SUCCESS

    def no_ecmp_member(self, device, ecmp, num_nhops, memlist):
        '''
        Parameters:
            device      int         device id
            ecmp        obj         ecmp object
            nhop        obj         next hop object
        Return:
            bool    True if success
        '''
        status = self.client.switch_api_ecmp_member_delete(
            device, ecmp, num_nhops, memlist)
        if status == SWITCH_STATUS_SUCCESS:
            self._pop('switch_api_ecmp_member_add', device, ecmp, num_nhops,
                      memlist)
        return status == SWITCH_STATUS_SUCCESS

    def add_rmac_group(self, device, rmac_type='inner', **kwargs):
        '''
        Parameters:
            device      int         device id
            rmac_type   str         'inner' | 'outer' | 'all'
            keyword arguments:
                store   bool        True: add to stack for later clean up
        Return:
            rmac object
        '''
        rmac_type = {
            'inner': SWITCH_RMAC_TYPE_INNER,
            'outer': SWITCH_RMAC_TYPE_OUTER,
            'all': SWITCH_RMAC_TYPE_ALL
        }.get(rmac_type, SWITCH_RMAC_TYPE_INNER)

        rmac = self.client.switch_api_router_mac_group_create(
            device, rmac_type)
        if kwargs.get('store', True):
            self._push('switch_api_router_mac_group_create', device, rmac)
        return rmac

    def no_rmac(self, device, rmac):
        '''
        Parameters:
            device      int         device id
            rmac        obj         rmac object
        Return:
            bool    True if success
        '''
        status = self.client.switch_api_router_mac_group_delete(device, rmac)
        if status == SWITCH_STATUS_SUCCESS:
            self._pop('switch_api_router_mac_group_create', device, rmac)
        return status == SWITCH_STATUS_SUCCESS

    def add_stp(self, device, stp_mode, **kwargs):
        '''
        Parameters:
            device      int         device id
            stp_mode    int         STP mode
            keyword arguments:
                store   bool        True: add to stack for later clean up
        Return:
            stp object
        '''
        stp = self.client.switch_api_stp_group_create(device, stp_mode)
        if kwargs.get('store', True):
            self._push('switch_api_stp_group_create', device, stp)
        return stp

    def no_stp(self, device, stp):
        '''
        Parameters:
            device      int         device id
            stp         obj         stp object
        Return:
            bool    True if success
        '''
        status = self.client.switch_api_stp_group_delete(device, stp)
        if status == SWITCH_STATUS_SUCCESS:
            self._pop('switch_api_stp_group_create', device, stp)
        return status == SWITCH_STATUS_SUCCESS

    def add_stp_group_member(self, device, stp, vlan, **kwargs):
        '''
        Parameters:
            device      int         device id
            stp         obj         stp object
            vlan        obj         vlan object
            keyword arguments:
                store   bool        True: add to stack for later clean up
        Return:
            bool    True if success
        '''
        status = self.client.switch_api_stp_group_member_add(device, stp, vlan)
        if status == SWITCH_STATUS_SUCCESS and kwargs.get('store', True):
            self._push('switch_api_stp_group_member_add', device, stp, vlan)
        return status == SWITCH_STATUS_SUCCESS

    def no_stp_group_member(self, device, stp, vlan):
        '''
        Parameters:
            device      int         device id
            stp         obj         stp object
            vlan        obj         vlan object
        Return:
            bool    True if success
        '''
        status = self.client.switch_api_stp_group_member_remove(
            device, stp, vlan)
        if status == SWITCH_STATUS_SUCCESS:
            self._pop('switch_api_stp_group_member_add', device, stp, vlan)
        return status == SWITCH_STATUS_SUCCESS

    def get_stp_port_state(self, device, stp, intf, **kwargs):
        '''
        Parameters:
            device      int         device id
            stp         obj         stp object
            intf        obj         intf object
            keyword arguments:
                store   bool        True: add to stack for later clean up
        Return:
            stp_state    STP state
        '''
        stp_state = self.client.switch_api_stp_port_state_get(
            device, stp, intf)
        return stp_state

    def set_stp_port_state(self, device, stp, intf, stp_state, **kwargs):
        '''
        Parameters:
            device      int         device id
            stp         obj         stp object
            intf        obj         intf object
            stp_state   int         stp state
            keyword arguments:
                store   bool        True: add to stack for later clean up
        Return:
            bool    True if success
        '''
        status = self.client.switch_api_stp_port_state_set(
            device, stp, intf, stp_state)
        return status == SWITCH_STATUS_SUCCESS

    def add_router_mac(self, device, rmac, mac, **kwargs):
        '''
        Parameters:
            device      int         device id
            rmac        obj         rmac object
            mac         str         mac address
            keyword arguments:
                store   bool        True: add to stack for later clean up
        Return:
            bool    True if success
        '''
        status = self.client.switch_api_router_mac_add(device, rmac, mac)
        if kwargs.get('store', True):
            self._push('switch_api_router_mac_add', device, rmac, mac)
        return status

    def no_router_mac(self, device, rmac, mac, **kwargs):
        '''
        Parameters:
            device      int         device id
            rmac        obj         rmac object
            mac         str         mac address
            keyword arguments:
                store   bool        True: add to stack for later clean up
        Return:
            bool    True if success
        '''
        status = self.client.switch_api_router_mac_delete(device, rmac, mac)
        if status == SWITCH_STATUS_SUCCESS:
            self._pop('switch_api_router_mac_add', device, rmac, mac)
        return status == SWITCH_STATUS_SUCCESS

    def select_port(self, device, swport):
        '''
        Parameters:
            device      int         device id
            swport      int         switchport id
        Return:
            port object
        '''
        return self.client.switch_api_port_id_to_handle_get(device, swport)

    def cfg_l2intf_on_port(self, device, port, mode='access', **kwargs):
        '''
        Parameters:
            device      int         device id
            port        obj         port object id
            mode        str         'access' | 'trunk'
            keyword arguments:
                store   bool        True: add to stack for later clean up
        Return:
            l2intf object
        '''
        l2_type = SWITCH_INTERFACE_TYPE_ACCESS
        if mode == 'trunk':
            l2_type = SWITCH_INTERFACE_TYPE_TRUNK
        info = switcht_interface_info_t(handle=port, type=l2_type)
        new_intf = self.client.switch_api_interface_create(device, info)
        if kwargs.get('store', True):
            self._push('switch_api_interface_create', device, new_intf)
        return new_intf

    def remove_l2intf_on_port(self, device, intf):
        """ Remvoe the l2intf on port """
        status = self.client.switch_api_interface_delete(device, intf)
        if status == SWITCH_STATUS_SUCCESS:
            self._pop('switch_api_interface_create', device, intf)
        return status == SWITCH_STATUS_SUCCESS

    def create_tunnel_interface(self, device, tunnel, **kwargs):
        '''
        Parameters:
            device      int         device id
            tunnel      obj         tunnel interface
            keyword arguments:
                store   bool        True: add to stack for later clean up
        Return:
            tunnelintf object
        '''

        info = switcht_interface_info_t(
            handle=tunnel, type=SWITCH_INTERFACE_TYPE_TUNNEL)
        new_intf = self.client.switch_api_interface_create(device, info)
        if kwargs.get('store', True):
            self._push('switch_api_interface_create', device, new_intf)
        return new_intf

    def create_loopback_rif(self, device, vrf, rmac, **kwargs):
        '''
        Parameters:
            device      int         device id
            keyword arguments:
                store   bool        True: add to stack for later clean up
        Return:
            loopbackintf object
        '''
        params = {
            'rif_type': SWITCH_RIF_TYPE_LOOPBACK,
            'vrf_handle': vrf,
            'rmac_handle': rmac,
        }

        info = switcht_rif_info_t(**params)
        new_rif = self.client.switch_api_rif_create(device, info)
        if kwargs.get('store', True):
            self._push('switch_api_rif_create', device, new_rif)
        return new_rif

    def cfg_l3intf_on_port(self, device, port, interface, **kwargs):
        '''
        Parameters:
            device      int         device id
            port        obj         port object id
            interface   obj         rif object id
            keyword arguments:
                store   bool        True: add to stack for later clean up
        Return:
            l3intf object
        '''
        info = switcht_interface_info_t(
            handle=port, type=SWITCH_INTERFACE_TYPE_PORT, rif_handle=interface)
        new_intf = self.client.switch_api_interface_create(device, info)
        if kwargs.get('store', True):
            self._push('switch_api_interface_create', device, new_intf)
        return new_intf

    def cfg_subintf_on_port(self,
                            device,
                            port,
                            interface,
                            vlan_id=10,
                            **kwargs):
        '''
        Parameters:
            device      int         device id
            port        obj         port object
            interface   obj         rif object
            vlan_id     int         vlan id
            keyword arguments:
                store   bool        True: add to stack for later clean up
        Return:
            subintf object
        '''
        self.client.switch_api_port_bind_mode_set(
            device, port, SWITCH_PORT_BIND_MODE_PORT_VLAN)
        info = switcht_interface_info_t(
            handle=port,
            vlan=vlan_id,
            type=SWITCH_INTERFACE_TYPE_PORT_VLAN,
            rif_handle=interface)
        new_intf = self.client.switch_api_interface_create(device, info)
        if kwargs.get('store', True):
            self._push('switch_api_port_bind_mode_set', device, port,
                       SWITCH_PORT_BIND_MODE_PORT)
            self._push('switch_api_interface_create', device, new_intf)
        return new_intf

    def no_subintf_on_port(self, device, port, subintf):
        '''
        Parameters:
            device      int         device id
            port        obj         port object id
            subintf     obj         sub-interface object
        Return:
            bool    True if success
        '''
        status = self.client.switch_api_interface_delete(device, subintf)
        if status == SWITCH_STATUS_SUCCESS:
            self._pop('switch_api_interface_create', device, subintf)
            self._pop('switch_api_port_bind_mode_set', device, port,
                      SWITCH_PORT_BIND_MODE_PORT)
            self.client.switch_api_port_bind_mode_set(
                device, port, SWITCH_PORT_BIND_MODE_PORT)
        return status == SWITCH_STATUS_SUCCESS

    def cfg_subintf_on_lag(self, device, lag, interface, vlan_id=10, **kwargs):
        '''
        Parameters:
            device      int         device id
            lag        obj          lag object
            interface   obj         rif object
            vlan_id     int         vlan id
            keyword arguments:
                store   bool        True: add to stack for later clean up
        Return:
            subintf object
        '''
        self.client.switch_api_lag_bind_mode_set(
            device, lag, SWITCH_PORT_BIND_MODE_PORT_VLAN)
        info = switcht_interface_info_t(
            handle=lag,
            vlan=vlan_id,
            type=SWITCH_INTERFACE_TYPE_PORT_VLAN,
            rif_handle=interface)
        new_intf = self.client.switch_api_interface_create(device, info)
        if kwargs.get('store', True):
            self._push('switch_api_lag_bind_mode_set', device, lag,
                       SWITCH_PORT_BIND_MODE_PORT)
            self._push('switch_api_interface_create', device, new_intf)
        return new_intf

    def no_subintf_on_lag(self, device, lag, subintf):
        '''
        Parameters:
            device      int         device id
            lag         obj         lag object
            subintf     obj         sub-interface object
        Return:
            bool    True if success
        '''
        status = self.client.switch_api_interface_delete(device, subintf)
        if status == SWITCH_STATUS_SUCCESS:
            self._pop('switch_api_interface_create', device, subintf)
            self._pop('switch_api_lag_bind_mode_set', device, lag,
                      SWITCH_PORT_BIND_MODE_PORT)
            self.client.switch_api_lag_bind_mode_set(
                device, lag, SWITCH_PORT_BIND_MODE_PORT)
        return status == SWITCH_STATUS_SUCCESS

    def add_vlan(self, device, vlan_id, **kwargs):
        '''
        Parameters:
            device      int         device id
            vlan_id     int         vlan id
            keyword arguments:
                store   bool        True: add to stack for later clean up
        Return:
            vlan object
        '''
        vlan = self.client.switch_api_vlan_create(device, vlan_id)
        if kwargs.get('store', True):
            self._push('switch_api_vlan_create', device, vlan)
        return vlan

    def no_vlan(self, device, vlan):
        '''
        Parameters:
            device      int         device id
            vlan        obj         vlan object
        Return:
            bool    True if success
        '''
        status = self.client.switch_api_vlan_delete(device, vlan)
        if status == SWITCH_STATUS_SUCCESS:
            self._pop('switch_api_vlan_create', device, vlan)
        return status == SWITCH_STATUS_SUCCESS

    def add_vlan_member(self, device, vlan, member, **kwargs):
        '''
        Parameters:
            device      int         device id
            vlan        obj         vlan object
            member      obj         interface object
            keyword arguments:
                store   bool        True: add to stack for later clean up
        Return:
            bool    True if success
        '''
        status = self.client.switch_api_vlan_member_add(device, vlan, member)
        if status == SWITCH_STATUS_SUCCESS and kwargs.get('store', True):
            self._push('switch_api_vlan_member_add', device, vlan, member)
        return status == SWITCH_STATUS_SUCCESS

    def no_vlan_member(self, device, vlan, intf):
        '''
        Parameters:
            device      int         device id
            vlan        obj         vlan object
            intf        obj         interface object
        Return:
            bool    True if success
        '''
        status = self.client.switch_api_vlan_member_remove(device, vlan, intf)
        if status == SWITCH_STATUS_SUCCESS:
            self._pop('switch_api_vlan_member_add', device, vlan, intf)
        return status == SWITCH_STATUS_SUCCESS

    def set_native_vlan(self, device, intf, vlan, **kwargs):
        '''
        Parameters:
            native_id      int         native vlan id
            intf           obj         interface object
            vlan           obj         vlan object
            keyword arguments:
                store   bool        True: add to stack for later clean up
        Return:
            bool    True if success
        '''
        status = self.client.switch_api_interface_native_vlan_set(
            device, intf, vlan)
        if status == SWITCH_STATUS_SUCCESS and kwargs.get('store', True):
            self._push('switch_api_interface_native_vlan_set', device, intf,
                       vlan)
        return status == SWITCH_STATUS_SUCCESS

    def add_mac_table_entry(self, device, network, mac, mac_type, handle,
                            **kwargs):
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
        if status == SWITCH_STATUS_SUCCESS and kwargs.get('store', True):
            self._push('switch_api_mac_table_entry_create', device, mac_entry)
        return status == SWITCH_STATUS_SUCCESS

    def remove_mac_table_entry(self, device, network, mac, mac_type, handle,
                               **kwargs):
        '''
        Parameters:
            device      int         device id
            network     obj         network object
            mac         str         mac address
            handle      obj         interface/nhop/tunnel object
        Return:
            bool    True if success
        '''
        params = {
            'network_handle': network,
            'mac_addr': mac,
            'entry_type': mac_type,
            'handle': handle
        }
        if 'tunnel' in kwargs and kwargs['tunnel'] == True:
            ip = self.make_ip_addr(kwargs['tunnel_ip'], host=True, **kwargs)
            params.update({'tunnel_ip': ip})

        mac_entry = switcht_api_mac_entry_t(**params)
        status = self.client.switch_api_mac_table_entry_delete(
            device, mac_entry)
        if status == SWITCH_STATUS_SUCCESS:
            self._pop('switch_api_mac_table_entry_create', device, mac_entry)
        return status == SWITCH_STATUS_SUCCESS

    def add_lag(self, device, **kwargs):
        '''
        Parameters:
            device      int         device id
            keyword arguments:
                store   bool        True: add to stack for later clean up
        Return:
            lag object
        '''
        lag = self.client.switch_api_lag_create(device)
        if kwargs.get('store', True):
            self._push('switch_api_lag_create', device, lag)
        return lag

    def remove_lag(self, device, lag):
        """
        Remove lag
        """
        status = self.client.switch_api_lag_delete(device, lag)
        if status == SWITCH_STATUS_SUCCESS:
            self._pop('switch_api_lag_create', device, lag)
        return status == SWITCH_STATUS_SUCCESS

    def add_lag_member(self, device, lag, member, **kwargs):
        '''
        Parameters:
            device      int         device id
            lag         obj         lag object
            member      obj         port object id
            keyword arguments:
                store   bool        True: add to stack for later clean up
        Return:
            bool    True if success
        '''
        status = self.client.switch_api_lag_member_add(
            device,
            lag_handle=lag,
            side=SWITCH_API_DIRECTION_BOTH,
            port=member)
        if status == SWITCH_STATUS_SUCCESS and kwargs.get('store', True):
            self._push(
                'switch_api_lag_member_add',
                device,
                lag_handle=lag,
                side=SWITCH_API_DIRECTION_BOTH,
                port=member)
        return status == SWITCH_STATUS_SUCCESS

    def remove_lag_member(self, device, lag, member):
        """ Remove a member from lag """
        status = self.client.switch_api_lag_member_delete(
            device,
            lag_handle=lag,
            side=SWITCH_API_DIRECTION_BOTH,
            port=member)
        if status == SWITCH_STATUS_SUCCESS:
            self._pop(
                'switch_api_lag_member_add',
                device,
                lag_handle=lag,
                side=SWITCH_API_DIRECTION_BOTH,
                port=member)
        return status == SWITCH_STATUS_SUCCESS

    def add_logical_l2lag(self, device, lag, mode='access', **kwargs):
        '''
        Parameters:
            device      int         device id
            lag         obj         lag object
            mode        str         'access' | 'trunk'
            keyword arguments:
                store   bool        True: add to stack for later clean up
        Return:
            l2lag object
        '''
        l2_type = SWITCH_INTERFACE_TYPE_ACCESS
        if mode == 'trunk':
            l2_type = SWITCH_INTERFACE_TYPE_TRUNK
        info = switcht_interface_info_t(handle=lag, type=l2_type)
        intf_lag = self.client.switch_api_interface_create(device, info)
        if kwargs.get('store', True):
            self._push('switch_api_interface_create', device, intf_lag)
        return intf_lag

    def add_logical_l3intf(self, device, vrf, rmac, **kwargs):
        '''
        Parameters:
            device      int         device id
            vrf         obj         vrf object
            rmac        obj         rmac object
            keyword arguments:
                v4_unicast_enabled  bool   True|False , default is True
                v6_unicast_enabled  bool   True|False , default is True
                store               bool   True: add to stack for later clean up
        Return:
            l3intf object
        '''
        params = {
            'rif_type': SWITCH_RIF_TYPE_INTF,
            'vrf_handle': vrf,
            'rmac_handle': rmac,
            'v4_unicast_enabled': True,
            'v6_unicast_enabled': True
        }
        if 'v4_unicast_enabled' in kwargs:
            params.update({'v4_unicast_enabled': kwargs['v4_unicast_enabled']})
        if 'v6_unicast_enabled' in kwargs:
            params.update({'v6_unicast_enabled': kwargs['v6_unicast_enabled']})

        info = switcht_rif_info_t(**params)
        new_rif = self.client.switch_api_rif_create(device, info)
        if kwargs.get('store', True):
            self._push('switch_api_rif_create', device, new_rif)
        return new_rif

    def add_logical_l3intf_urpf(self, device, vrf, rmac, **kwargs):
        '''
        Parameters:
            device      int         device id
            vrf         obj         vrf object
            rmac        obj         rmac object
            keyword arguments:
                v4_urpf_mode        int    1 | 2
                v4_unicast_enabled  bool   True|False
                v6_urpf_mode        int    1 | 2
                v6_unicast_enabled  bool   True|False
                store               bool   True: add to stack for later clean up
        Return:
            l3intf_urpf object
        '''
        params = {
            'rif_type': SWITCH_RIF_TYPE_INTF,
            'vrf_handle': vrf,
            'rmac_handle': rmac
        }
        if 'v4_urpf_mode' in kwargs:
            params.update({'v4_urpf_mode': kwargs['v4_urpf_mode']})
        if 'v4_unicast_enabled' in kwargs:
            params.update({'v4_unicast_enabled': kwargs['v4_unicast_enabled']})

        if 'v6_urpf_mode' in kwargs:
            params.update({'v6_urpf_mode': kwargs['v6_urpf_mode']})
        if 'v6_unicast_enabled' in kwargs:
            params.update({'v6_unicast_enabled': kwargs['v6_unicast_enabled']})

        info = switcht_rif_info_t(**params)
        new_rif = self.client.switch_api_rif_create(device, info)
        if kwargs.get('store', True):
            self._push('switch_api_rif_create', device, new_rif)
        return new_rif

    def add_logical_l3vlan(self, device, vrf, rmac, vlan_id, **kwargs):
        '''
        Parameters:
            device      int         device id
            vrf         obj         vrf object
            rmac        obj         rmac object
            vlan_id     int         vlan id
            keyword arguments:
                v4_unicast_enabled  bool   default is True
                v6_unicast_enabled  bool   default is True
                store               bool   True: add to stack for later clean up
        Return:
            l3vlan object
        '''
        params = {
            'rif_type': SWITCH_RIF_TYPE_VLAN,
            'vlan': vlan_id,
            'vrf_handle': vrf,
            'rmac_handle': rmac,
            'v4_unicast_enabled': True,
            'v6_unicast_enabled': True
        }
        if 'v4_unicast_enabled' in kwargs:
            params.update({'v4_unicast_enabled': kwargs['v4_unicast_enabled']})
        if 'v6_unicast_enabled' in kwargs:
            params.update({'v6_unicast_enabled': kwargs['v6_unicast_enabled']})

        info = switcht_rif_info_t(**params)
        new_rif = self.client.switch_api_rif_create(device, info)
        if kwargs.get('store', True):
            self._push('switch_api_rif_create', device, new_rif)
        return new_rif

    def no_logical_l3vlan(self, device, interface):
        '''
        Parameters:
            device      int         device id
            interface   obj         interface object
        Return:
            bool    True if success
        '''
        status = self.client.switch_api_rif_delete(device, interface)
        if status == SWITCH_STATUS_SUCCESS:
            self._pop('switch_api_rif_create', device, interface)
        return status == SWITCH_STATUS_SUCCESS

    def cfg_ip_address(self, device, rif, vrf, ip, **kwargs):
        '''
        Parameters:
            device      int         device id
            rif         obj         rif object
            vrf         obj         vrf object
            address     obj         internet address object
            keyword arguments:
                store               bool   True: add to stack for later clean up
        Return:
            bool    True if success
        '''
        address = self.make_ip_addr(ip, **kwargs)
        status = self.client.switch_api_l3_interface_address_add(
            device, rif, vrf, address)
        if status == SWITCH_STATUS_SUCCESS and kwargs.get('store', True):
            self._push('switch_api_l3_interface_address_add', device, rif, vrf,
                       address)
        return status == SWITCH_STATUS_SUCCESS

    def no_ip_address(self, device, rif, vrf, address):
        '''
        Parameters:
            device      int         device id
            rif         obj         rif object
            vrf         obj         vrf object
            address     obj         internet address object
        Return:
            bool    True if success
        '''
        status = self.client.switch_api_l3_interface_address_delete(
            device, rif, vrf, address)
        if status == SWITCH_STATUS_SUCCESS:
            self._pop('switch_api_l3_interface_address_add', device, rif, vrf,
                      address)
        return status == SWITCH_STATUS_SUCCESS

    def make_ip_addr(self, ip, **kwargs):
        v4 = True
        if 'v4' in kwargs:
            v4 = kwargs['v4']

        if v4 == True:
            params = {
                'addr_type': SWITCH_API_IP_ADDR_V4,
                'ipaddr': ip,
                'prefix_length': 16
            }
        else:
            params = {
                'addr_type': SWITCH_API_IP_ADDR_V6,
                'ipaddr': ip,
                'prefix_length': 120
            }

        if 'prefix_length' in kwargs:
            params.update({'prefix_length': kwargs['prefix_length']})

        if 'host' in kwargs and kwargs['host'] == True:
            if 'v4' in kwargs and kwargs['v4'] == False:
                params.update({'addr_type': SWITCH_API_IP_ADDR_V6})
                params.update({'prefix_length': 128})
            else:
                params.update({'addr_type': SWITCH_API_IP_ADDR_V4})
                params.update({'prefix_length': 32})

        address = switcht_ip_addr_t(**params)
        return address

    def make_ipv4_ipaddr(self, ipv4, imask):
        '''
        Parameters:
            ipv4        str         ipv4 address
            imask       int         integer mask number
        Return:
            ipv4 object
        '''
        return switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V4, ipaddr=ipv4, prefix_length=imask)

    def make_ipv6_ipaddr(self, ipv6, imask):
        '''
        Parameters:
            ipv6        str         ipv6 address
            imask       int         integer mask number
        Return:
            ipv6 object
        '''
        return switcht_ip_addr_t(
            addr_type=SWITCH_API_IP_ADDR_V6, ipaddr=ipv6, prefix_length=imask)

    def add_nhop(self, device, rif, ip, **kwargs):
        '''
        Parameters:
            device      int         device id
            interface   obj         interface object
            keyword arguments:
                store               bool   True: add to stack for later clean up
        Return:
            nhop object
        '''

        nhtype = SWITCH_NHOP_TYPE_IP
        ip_addr = self.make_ip_addr(ip, host=True, **kwargs)
        nhop_info = switcht_api_nhop_info_t(
            nhop_type=nhtype, rif_handle=rif, ip_addr=ip_addr)
        nhop = self.client.switch_api_nhop_create(device, nhop_info)
        if kwargs.get('store', True):
            self._push('switch_api_nhop_create', device, nhop)
        return nhop

    def no_nhop(self, device, nhop, **kwargs):
        '''
        Parameters:
            device      int         device id
            nhop        obj         nhop object
        Return:
            bool    True if success
        '''
        status = self.client.switch_api_nhop_delete(device, nhop)
        if status == SWITCH_STATUS_SUCCESS:
            self._pop('switch_api_nhop_create', device, nhop)
        return status == SWITCH_STATUS_SUCCESS

    def add_neighbor_l3intf(self, device, nhop, mac, **kwargs):
        '''
        Parameters:
            device      int         device id
            interface   obj         interface object
            nhop        obj         nhop object
            mac         str         mac address
            keyword arguments:
                store               bool   True: add to stack for later clean up
        Return:
            l3intf neighbor object
        '''

        neighbor_info = switcht_api_neighbor_info_t(
            nhop_handle=nhop,
            rif_handle=0x0,
            mac_addr=mac,
            neighbor_type=SWITCH_NEIGHBOR_TYPE_NHOP,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L3)
        neighbor = self.client.switch_api_neighbor_create(
            device, neighbor_info)
        if kwargs.get('store', True):
            self._push('switch_api_neighbor_create', device, neighbor)
        return neighbor

    def add_neighbor_l3mpls(self, device, interface, nhop, mac, address,
                            **kwargs):
        '''
        Parameters:
            device      int         device id
            interface   obj         interface object
            nhop        obj         nhop object
            mac         str         mac address
            address     obj         address object
            keyword arguments:
                mpls_label          int     mpls label.  , default is 0
                header_count        int     header count , default is 2
                store               bool    True: add to stack for later clean up
        Return:
            l3mpls neighbor object
        '''
        params = {
            'nhop_handle': nhop,
            'interface_handle': interface,
            'rw_type': SWITCH_API_NEIGHBOR_RW_TYPE_L3,
            'neigh_type': SWITCH_API_NEIGHBOR_MPLS_PUSH_L3VPN
        }
        params.update({'mpls_label': kwargs.get('mpls_label', 0)})
        params.update({'header_count': kwargs.get('header_count', 2)})

        neighbor_entry = switcht_neighbor_info_t(**params)
        neighbor = self.client.switch_api_neighbor_entry_add(
            device, neighbor_entry)
        if kwargs.get('store', True):
            self._push('switch_api_neighbor_entry_add', device, neighbor)
        return neighbor

    def add_neighbor_l2tunnel(self, device, interface, nhop, mac, address,
                              **kwargs):
        '''
        Parameters:
            device      int         device id
            interface   obj         interface object
            nhop        obj         nhop object
            mac         str         mac address
            address     obj         address object
            keyword arguments:
                store               bool    True: add to stack for later clean up
        Return:
            l2tunnel neighbor object
        '''
        neighbor_entry = switcht_neighbor_info_t(
            nhop_handle=nhop,
            interface_handle=interface,
            mac_addr=mac,
            ip_addr=address,
            rw_type=SWITCH_API_NEIGHBOR_RW_TYPE_L2,
            neigh_type=SWITCH_API_NEIGHBOR_IPV4_TUNNEL)
        neighbor = self.client.switch_api_neighbor_entry_add(
            device, neighbor_entry)
        if kwargs.get('store', True):
            self._push('switch_api_neighbor_entry_add', device, neighbor)
        return neighbor

    def no_neighbor(self, device, neighbor):
        '''
        Parameters:
            device      int         device id
            neighbor    obj         neighbor object
        Return:
            bool    True if success
        '''
        status = self.client.switch_api_neighbor_delete(device, neighbor)
        if status == SWITCH_STATUS_SUCCESS:
            self._pop('switch_api_neighbor_create', device, neighbor)
        return status == SWITCH_STATUS_SUCCESS

    def add_static_route(self, device, vrf, address, nhop, **kwargs):
        '''
        Parameters:
            device      int         device id
            vrf         obj         vrf object
            address     obj         address object
            nhop        obj         nhop object
            keyword arguments:
                store               bool   True: add to stack for later clean up
        Return:
            bool    True if success
        '''
        address = self.make_ip_addr(address, **kwargs)
        status = self.client.switch_api_l3_route_add(device, vrf, address,
                                                     nhop)
        if status == SWITCH_STATUS_SUCCESS and kwargs.get('store', True):
            self._push('switch_api_l3_route_add', device, vrf, address, nhop)
        return status == SWITCH_STATUS_SUCCESS

    def update_static_route(self, device, vrf, address, nhop):
        '''
        Parameters:
            device      int         device id
            vrf         obj         vrf object
            address     obj         address object
            nhop        obj         nhop object
        Return:
            bool    True if success
        '''
        status = self.client.switch_api_l3_route_update(
            device, vrf, address, nhop)
        if status == SWITCH_STATUS_SUCCESS:
            self._update('switch_api_l3_route_update', device, vrf, address,
                         nhop)
        return status == SWITCH_STATUS_SUCCESS

    def is_static_route(self, device, vrf, address, nhop):
        '''
        Parameters:
            device      int         device id
            vrf         obj         vrf object
            address     obj         address object
            nhop        obj         nhop object
        Return:
            bool    True if success
        '''
        return self.client.switch_api_l3_route_lookup(device, vrf, address,
                                                      nhop)

    def no_static_route(self, device, vrf, address, nhop):
        '''
        Parameters:
            device      int         device id
            vrf         obj         vrf object
            address     obj         address object
            nhop        obj         nhop object
        Return:
            bool    True if success
        '''
        status = self.client.switch_api_l3_route_delete(
            device, vrf, address, nhop)
        if status == SWITCH_STATUS_SUCCESS:
            self._pop('switch_api_l3_route_add', device, vrf, address, nhop)
        return status == SWITCH_STATUS_SUCCESS

    def create_l3_rif(self, device, vrf, rmac, port, ip, **kwargs):
        '''
        Parameters:
            device      int         device id
            vrf         obj         vrf object
            rmac        obj         rmac object
            port        obj         port object
            ip          obj         address object

        Return:
            obj         Returns rif object
        '''
        new_rif = self.add_logical_l3intf(device, vrf, rmac, **kwargs)
        new_intf = self.cfg_l3intf_on_port(device, port, new_rif, **kwargs)
        self.cfg_ip_address(device, new_rif, vrf, ip, **kwargs)
        return new_rif

    def add_rmac(self, device, mac, rmac_type='inner', **kwargs):
        '''
        Parameters:
            device      int         device id
            type        string      inner/outer/all
            mac         string      mac address

        Return:
            obj         Returns rmac group object
        '''

        rmac = self.add_rmac_group(device, rmac_type, **kwargs)
        self.add_router_mac(device, rmac, mac, **kwargs)
        return rmac

    def add_l3_nhop(self, device, rif, ip, mac, **kwargs):
        '''
        Parameters:
            device      int         device id
            rif         obj         rif object
            ip          ip address  nhop ip
            mac         string      mac address

        Return:
            obj         Returns nhop object
        '''

        nhop = self.add_nhop(device, rif, ip, **kwargs)
        self.add_neighbor_l3intf(device, nhop, mac, **kwargs)
        return nhop

    def create_logical_network(self, device, **kwargs):
        '''
        Parameters:
            device      int         device id

        Return:
            obj         Returns ln object
        '''

        ln_info = switcht_logical_network_t()
        ln = self.client.switch_api_logical_network_create(device, ln_info)
        if kwargs.get('store', True):
            self._push('switch_api_logical_network_create', device, ln)
        return ln

    def add_logical_network_member(self, device, ln, intf, **kwargs):
        '''
        Parameters:
            device      int         device id
            ln          obj         logical network object
            intf        obj         interface object

        Return:
            bool    True if success
        '''

        status = self.client.switch_api_logical_network_member_add(
            device, ln, intf)
        if kwargs.get('store', True):
            self._push('switch_api_logical_network_member_add', device, ln,
                       intf)
        return status == SWITCH_STATUS_SUCCESS

    def create_tunnel_mapper(self, device, map_type, **kwargs):
        '''
        Parameters:
            device      int         device id
            map_type    enum        tunnel map type
            tunnel_type enum        tunnel type

        Return:
            obj         Returns tunnel mapper object
        '''

        tunnel_mapper = switcht_api_tunnel_mapper_t(tunnel_map_type=map_type)
        mapper = self.client.switch_api_tunnel_mapper_create(
            device, tunnel_mapper)
        if kwargs.get('store', True):
            self._push('switch_api_tunnel_mapper_create', device, mapper)
        return mapper

    def create_tunnel_mapper_entry(self, device, mapper, map_type, vni, handle,
                                   **kwargs):
        '''
        Parameters:
            device      int         device id
            map_type    enum        tunnel map type
            vni         int         tunnel vni

        Return:
            obj         Returns tunnel mapper object
        '''

        vlan_handle = 0
        ln_handle = 0
        vrf_handle = 0
        if map_type == SWITCH_TUNNEL_MAP_TYPE_VNI_TO_VLAN_HANDLE or map_type == SWITCH_TUNNEL_MAP_TYPE_VLAN_HANDLE_TO_VNI:
            vlan_handle = handle
        elif map_type == SWITCH_TUNNEL_MAP_TYPE_VNI_TO_VRF_HANDLE or map_type == SWITCH_TUNNEL_MAP_TYPE_VRF_HANDLE_TO_VNI:
            vrf_handle = handle
        else:
            ln_handle = handle

        tunnel_mapper_entry = switcht_api_tunnel_mapper_entry_t(
            tunnel_mapper_handle=mapper,
            tunnel_vni=vni,
            tunnel_map_type=map_type,
            vlan_handle=vlan_handle,
            ln_handle=ln_handle,
            vrf_handle=vrf_handle)
        mapper_entry = self.client.switch_api_tunnel_mapper_entry_create(
            device, tunnel_mapper_entry)
        if kwargs.get('store', True):
            self._push('switch_api_tunnel_mapper_entry_create', device,
                       mapper_entry)
        return mapper_entry

    def create_tunnel_table(self,
                            device,
                            src_ip,
                            tunnel_type,
                            imapper_h=0,
                            emapper_h=0,
                            urif=0,
                            orif=0,
                            tunnel_ip_type=SWITCH_TUNNEL_IP_ADDR_TYPE_IPV4,
                            **kwargs):
        '''
        Parameters:
            device      int         device id
            src_ip      ip address  tunnel encap ip
            tunnel_type enum        tunnel type
            entry_type  enum        p2p/p2mp
            imapper_h   obj         ingress mapper handle
            emapper_h   obj         egress mapper handle
            urif        obj         underlay rif
            orif        obj         overlay rif

        Return:
            obj         Returns tunnel object
        '''
        ip = self.make_ip_addr(src_ip, host=True, **kwargs)
        params = {
            'tunnel_type': tunnel_type,
            'entry_type': 0,
            'src_ip': ip,
            'decap_mapper_handle': imapper_h,
            'encap_mapper_handle': emapper_h,
            'underlay_rif_handle': urif,
            'overlay_rif_handle': orif,
            'ip_type': tunnel_ip_type
        }

        tunnel_info = switcht_api_tunnel_info_t(**params)
        tunnel = self.client.switch_api_tunnel_create(device, tunnel_info)
        if kwargs.get('store', True):
            self._push('switch_api_tunnel_create', device, tunnel)
        return tunnel

    def create_tunnel_term(self, device, vrf, tunnel, tunnel_type, src_ip,
                           dst_ip, entry_type, **kwargs):
        '''
        Parameters:
            device      int         device id
            vrf         obj         vrf handle
            tunnel_type enum        tunnel type
            src_ip      ip address  tunnel term src ip
            dst_ip      ip address  tunnel term dst ip
            entry_type  enum        p2p/p2mp

        Return:
            obj         Returns tunnel object
        '''

        sip = self.make_ip_addr(src_ip, host=True, **kwargs)
        dip = self.make_ip_addr(dst_ip, host=True, **kwargs)
        params = {
            'vrf_handle': vrf,
            'tunnel_handle': tunnel,
            'tunnel_type': tunnel_type,
            'term_entry_type': entry_type,
            'src_ip': sip,
            'dst_ip': dip
        }

        tunnel_info = switcht_api_tunnel_term_info_t(**params)
        tunnel_term = self.client.switch_api_tunnel_term_create(
            device, tunnel_info)
        if kwargs.get('store', True):
            self._push('switch_api_tunnel_term_create', device, tunnel_term)
        return tunnel_term

    def create_tunnel(self,
                      device,
                      underlay_vrf,
                      tunnel_type,
                      src_ip,
                      dst_ip,
                      entry_type,
                      urif,
                      orif,
                      ingress_map_type=None,
                      egress_map_type=None,
                      mapper_list=None,
                      tunnel_ip_type=SWITCH_TUNNEL_IP_ADDR_TYPE_IPV4,
                      **kwargs):

        imapper_h = 0
        emapper_h = 0
        tunnel_h = self.create_tunnel_table(
            device=device,
            tunnel_type=tunnel_type,
            src_ip=dst_ip,
            imapper_h=imapper_h,
            emapper_h=emapper_h,
            urif=urif,
            orif=orif,
            tunnel_ip_type=tunnel_ip_type,
            **kwargs)

        tunnel_term_h = self.create_tunnel_term(
            device=device,
            tunnel_type=tunnel_type,
            vrf=underlay_vrf,
            tunnel=tunnel_h,
            entry_type=entry_type,
            src_ip=src_ip,
            dst_ip=dst_ip,
            **kwargs)
        self.create_tunnel_interface(device=device, tunnel=tunnel_h)

        return tunnel_h

    def add_nhop_tunnel(self,
                        device,
                        nhop_tunnel_type,
                        handle,
                        tunnel,
                        tunnel_ip,
                        rw_type=SWITCH_NHOP_TUNNEL_REWRITE_TYPE_L2,
                        mac_addr=None,
                        **kwargs):
        '''
        Parameters:
            device      int         device id
            interface   obj         interface object
            keyword arguments:
                store               bool   True: add to stack for later clean up
        Return:
            nhop object
        '''

        nhtype = SWITCH_NHOP_TYPE_TUNNEL
        vrf_handle = 0
        network_handle = 0
        if nhop_tunnel_type == SWITCH_NHOP_TUNNEL_TYPE_VRF:
            vrf_handle = handle
        else:
            network_handle = handle

        ip_addr = self.make_ip_addr(tunnel_ip, host=True, **kwargs)
        nhop_info = switcht_api_nhop_info_t(
            nhop_type=nhtype,
            tunnel_handle=tunnel,
            nhop_tunnel_type=nhop_tunnel_type,
            vrf_handle=vrf_handle,
            network_handle=network_handle,
            ip_addr=ip_addr,
            mac_addr=mac_addr,
            rewrite_type=rw_type)
        nhop = self.client.switch_api_nhop_create(device, nhop_info)
        if kwargs.get('store', True):
            self._push('switch_api_nhop_create', device, nhop)
        return nhop

    def add_mirror(self, device, session_id, direction, egress_port_handle,
                   mirror_type, cos, max_pkt_len, ttl, nhop_handle, **kwargs):
        """ Add a mirror session """
        params = {
            'session_id': session_id,
            'direction': direction,
            'egress_port_handle': egress_port_handle,
            'mirror_type': mirror_type,
            'cos': cos,
            'max_pkt_len': max_pkt_len,
            'ttl': ttl,
            'nhop_handle': nhop_handle
        }
        for k, v, in kwargs.items():
            if k in [
                    'vlan_id', 'extract_len', 'timeout_usec', 'span_mode',
                    'src_ip', 'dst_ip', 'src_mac', 'dst_mac'
            ]:
                params.update({k: v})
        mirror_info = switcht_mirror_info_t(**params)
        mirror = self.client.switch_api_mirror_session_create(
            device, mirror_info)
        if kwargs.get('store', True):
            self._push('switch_api_mirror_session_create', device, mirror)
        return mirror

    def remove_mirror(slef, device, mirror):
        status = self.client.switch_api_mirror_session_delete(device, mirror)
        if status == SWITCH_STATUS_SUCCESS:
            self._pop('switch_api_mirror_session_create', mirror)
        return status == SWITCH_STATUS_SUCCESS

    def acl_list_create(self, device, direction, type, bp_type, **kwargs):
        """ Api adapter wrapper for switch_api_acl_list_create """
        params = {
            'device': device,
            'direction': direction,
            'type': type,
            'bp_type': bp_type
        }
        acl = self.client.switch_api_acl_list_create(**params)
        if kwargs.get('store', True):
            self._push('switch_api_acl_list_delete', device, acl)
        return acl


###############################################################################
