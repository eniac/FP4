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

import ptf
from ptf.base_tests import BaseTest
from ptf import config
import ptf.dataplane as dataplane
from ptf.testutils import *

################################################################
#
# Thrift interface base tests
#
################################################################

import switchapi_thrift.switch_api_rpc as switch_api_rpc
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol
from thrift.protocol import TMultiplexedProtocol
from devport_mgr_pd_rpc.ttypes import *


class ThriftInterface(BaseTest):
    def setUp(self):
        BaseTest.setUp(self)

        # Set up thrift client and contact server
        self.thrift_server = 'localhost'
        if test_param_get('thrift_server') is not None:
            self.thrift_server = test_param_get('thrift_server')
        self.transport = TSocket.TSocket(self.thrift_server, 9091)
        self.transport = TTransport.TBufferedTransport(self.transport)
        self.protocol = TBinaryProtocol.TBinaryProtocol(self.transport)

        self.client = switch_api_rpc.Client(self.protocol)
        self.transport.open()

        self.pd_transport = TSocket.TSocket(self.thrift_server, 9090)
        self.pd_transport = TTransport.TBufferedTransport(self.pd_transport)
        self.pd_protocol = TBinaryProtocol.TBinaryProtocol(self.pd_transport)
        self.pd_protocol = TMultiplexedProtocol.TMultiplexedProtocol(
            self.pd_protocol, "devport_mgr")
        self.devport_mgr_client_module = importlib.import_module(
            ".".join(["devport_mgr_pd_rpc", "devport_mgr"]))
        self.devport_mgr = self.devport_mgr_client_module.Client(
            self.pd_protocol)
        self.pd_transport.open()
    def warm_init_begin(self, device):
    	self.transport.close()
	self.devport_mgr.devport_mgr_warm_init_begin(
            device, dev_init_mode.DEV_WARM_INIT_FAST_RECFG,
            dev_serdes_upgrade_mode.DEV_SERDES_UPD_NONE, False)
	self.transport.open()

    def tearDown(self):
        if config["log_dir"] != None:
            self.dataplane.stop_pcap()
        BaseTest.tearDown(self)
        self.transport.close()
        self.pd_transport.close()


class ThriftInterfaceDataPlane(ThriftInterface):
    """
    Root class that sets up the thrift interface and dataplane
    """

    def setUp(self):
        ThriftInterface.setUp(self)
        self.dataplane = ptf.dataplane_instance
        self.dataplane.flush()
        if config["log_dir"] != None:
            filename = os.path.join(config["log_dir"], str(self)) + ".pcap"
            self.dataplane.start_pcap(filename)

    def tearDown(self):
        if config["log_dir"] != None:
            self.dataplane.stop_pcap()
        ThriftInterface.tearDown(self)
