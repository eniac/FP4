################################################################################
# BAREFOOT NETWORKS CONFIDENTIAL & PROPRIETARY
#
# Copyright (c) 2017-present Barefoot Networks, Inc.
#
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

import os
import cmd
import sys
import argparse

parser = argparse.ArgumentParser()
parser.add_argument(
    '--install-dir', required=True, help='path to install directory')
args = parser.parse_args()
install_path = os.path.join(args.install_dir, 'lib/python2.7/site-packages')
sys.path.append(install_path)
sys.path.append(os.path.join(install_path, 'tofino'))
sys.path.append(os.path.join(install_path, 'tofinopd'))

import switch.p4_pd_rpc.dc as switch_pd_rpc
import conn_mgr_pd_rpc.conn_mgr as conn_mgr_rpc
from ptf.thriftutils import *
from switch.p4_pd_rpc.ttypes import *
from res_pd_rpc.ttypes import *
from mc_pd_rpc.ttypes import *
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol
from thrift.protocol import TMultiplexedProtocol


###############################################################################
# This is a simple Python shell to dump tables and action profiles from the
# hardware using the PD thrift interface. The class variable TABLES and
# ACTION_PROFILES enumerate the list of tables and action profiles that can be
# dumped.
#
# To use the shell, invoke it as follows:
#     python ./dump.py --install-dir <path to install directory>
#
# Use the "table" command to dump the entries of a table and the "profile"
# command to dump the action data of an action profile. Since the Dump class
# extends the python cmd module, command completion is supported for the table
# action profile names.
###############################################################################
class ThriftInterface():
    def __init__(self):
        self.transport = TSocket.TSocket('localhost', 9090)
        self.transport = TTransport.TBufferedTransport(self.transport)
        self.protocol = TBinaryProtocol.TBinaryProtocol(self.transport)
        self.conn_mgr_protocol = TMultiplexedProtocol.TMultiplexedProtocol(
            self.protocol, 'conn_mgr')
        self.switch_p4_protocol = TMultiplexedProtocol.TMultiplexedProtocol(
            self.protocol, 'dc')
        self.client = switch_pd_rpc.Client(self.switch_p4_protocol)
        self.conn_mgr = conn_mgr_rpc.Client(self.conn_mgr_protocol)
        self.transport.open()
        self.sess_hdl = self.conn_mgr.client_init()

    def dump_table(self, table):
        '''' Dump all entries of a table'''
        dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))
        table = 'self.client.' + table

        # fetch the total number of entries
        num_entries = eval(table + '_get_entry_count')\
                (self.sess_hdl, dev_tgt)
        print 'Number of entries : {}'.format(num_entries)
        if num_entries == 0:
            return

        # fetch the first entry
        hdl = eval(table + '_get_first_entry_handle')\
                (self.sess_hdl, dev_tgt)

        # fetch the remaining entries
        if num_entries > 1:
            hdls = eval(table + '_get_next_entry_handles')\
                (self.sess_hdl, dev_tgt, hdl, num_entries - 1)
            hdls.insert(0, hdl)
        else:
            hdls = [hdl]

        # dump the entries
        i = 1
        for hdl in hdls:
            entry = eval(table + '_get_entry')\
                (self.sess_hdl, dev_tgt.dev_id, hdl, True)
            print 'Entry', i
            print '    Match:'
            for key, val in entry.match_spec.__dict__.iteritems():
                print '        ',
                print key, ':', val
            if hasattr(entry, 'action_desc'):
                print '    Action:', entry.action_desc.name
                print '    Data:',
                for key, val in entry.action_desc.data.__dict__.items():
                    print '        ',
                    print key, ':', val
            elif hasattr(entry, 'members'):
                print '    Members:', entry.members
            i += 1

    def dump_profile(self, profile):
        '''' Dump all members of a action profile'''
        dev_tgt = DevTarget_t(0, hex_to_i16(0xFFFF))
        table = 'self.client.' + profile

        # fetch the first member
        hdl = eval(table + '_get_first_member')\
                (self.sess_hdl, dev_tgt)
        if hdl == -1:
            return

        # fetch the next 100 members
        hdls = eval(table + '_get_next_members')\
                (self.sess_hdl, dev_tgt, hdl, 100)
        hdls.insert(0, hdl)

        # dump the members
        for hdl in hdls:
            if hdl == -1:
                break
            print 'Member', hdl
            entry = eval(table + '_get_member')\
                    (self.sess_hdl, dev_tgt.dev_id, hdl, True)
            print '    Action:', entry.name
            print '    Data:',
            for key, val in entry.data.__dict__.items():
                print '        ',
                print key, ':', val

    def __del__(self):
        self.conn_mgr.client_cleanup(self.sess_hdl)
        self.transport.close()


class Dump(cmd.Cmd):
    TABLES = [
        'ingress_port_properties', 'port_vlan_to_bd_mapping', 'egress_port_mapping',
        'ipv4_fib_lpm', 'ecmp_group', 'lag_group', 'rewrite'
    ]

    ACTION_PROFILES = [
        'bd_action_profile', 'ecmp_action_profile', 'lag_action_profile'
    ]

    def preloop(self):
        self.conn = ThriftInterface()

    def postloop(self):
        print

    def do_table(self, table):
        "Dump contents of a table"
        if not table:
            print 'No table name specified'
            return

        if table not in self.TABLES:
            print 'Invalid table name:', table
            return

        self.conn.dump_table(table)

    def complete_table(self, text, line, begidx, endidx):
        if not text:
            completions = self.TABLES[:]
        else:
            completions = [t for t in self.TABLES if t.startswith(text)]
        return completions

    def do_profile(self, profile):
        "Dump contents of an action profile"
        if not profile:
            print 'No action profile name specified'
            return

        if profile not in self.ACTION_PROFILES:
            print 'Invalid action profile name:', profile
            return

        self.conn.dump_profile(profile)

    def complete_profile(self, text, line, begidx, endidx):
        if not text:
            completions = self.ACTION_PROFILES[:]
        else:
            completions = [
                t for t in self.ACTION_PROFILES if t.startswith(text)
            ]
        return completions

    def do_exit(self, line):
        "Exit"
        return True

    def do_quit(self, line):
        "Exit"
        return True


if __name__ == '__main__':
    dump = Dump()
    dump.prompt = '(dump) '
    dump.cmdloop()
