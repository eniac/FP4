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
"""
Base classes for test cases

Tests will usually inherit from one of these classes to have the controller
and/or dataplane automatically set up.
"""

import importlib
import os
import logging
import unittest

import oftest
from oftest import config
import oftest.dataplane as dataplane
import oftest.base_tests as base_tests

################################################################
#
# Thrift interface base tests
#
################################################################

from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol
from thrift.protocol import TMultiplexedProtocol


class OFTestInterface(base_tests.SimpleDataPlane):
    def __init__(self, p4_name):
        base_tests.SimpleDataPlane.__init__(self)
        self.p4_name = p4_name
        self.p4_client_module = importlib.import_module(
            ".".join(["p4_pd_rpc", p4_name]))
        self.mc_client_module = importlib.import_module(
            ".".join(["mc_pd_rpc", "mc"]))
        self.conn_mgr_client_module = importlib.import_module(
            ".".join(["conn_mgr_pd_rpc", "conn_mgr"]))

    def setUp(self):
        base_tests.SimpleDataPlane.setUp(self)
        if config["log_dir"] != None:
            filename = os.path.join(config["log_dir"], str(self)) + ".pcap"
            self.dataplane.start_pcap(filename)

        # Set up thrift client and contact server
        self.transport = TSocket.TSocket('localhost', 9090)
        self.transport = TTransport.TBufferedTransport(self.transport)
        bprotocol = TBinaryProtocol.TBinaryProtocol(self.transport)

        self.mc_protocol = TMultiplexedProtocol.TMultiplexedProtocol(bprotocol,
                                                                     "mc")
        self.conn_mgr_protocol = TMultiplexedProtocol.TMultiplexedProtocol(
            bprotocol, "conn_mgr")
        self.p4_protocol = TMultiplexedProtocol.TMultiplexedProtocol(
            bprotocol, self.p4_name)

        self.client = self.p4_client_module.Client(self.p4_protocol)
        self.mc = self.mc_client_module.Client(self.mc_protocol)
        self.conn_mgr = self.conn_mgr_client_module.Client(
            self.conn_mgr_protocol)
        self.transport.open()

    def tearDown(self):
        if config["log_dir"] != None:
            self.dataplane.stop_pcap()
        base_tests.SimpleDataPlane.tearDown(self)
        self.transport.close()
