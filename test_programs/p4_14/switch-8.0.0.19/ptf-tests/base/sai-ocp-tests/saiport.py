# Copyright 2013-present Barefoot Networks, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
Thrift SAI interface L2 tests
"""
import socket
import sai_base_test
from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *
from switchsai_thrift.sai_headers import  *
from switch_utils import *

port_list_tmp = []
class PortConfigure(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        self.test_params = testutils.test_params_get()
        port_num_list = self.test_params['port_list']
        speed = self.test_params['speed']
    	switch_attr_list = self.client.sai_thrift_get_switch_attribute()
        attr_list = switch_attr_list.attr_list
    	for attribute in attr_list:
            if attribute.id == SAI_SWITCH_ATTR_PORT_LIST:
                for x in attribute.value.objlist.object_id_list:
               	    port_list_tmp.append(x)


	
        attr_value = sai_thrift_attribute_value_t(u32=speed)
        attr = sai_thrift_attribute_t(id=SAI_PORT_ATTR_SPEED, value=attr_value)
        attr_value = sai_thrift_attribute_value_t(u32=SAI_PORT_FEC_MODE_NONE)
        attr_fec = sai_thrift_attribute_t(id=SAI_PORT_ATTR_FEC_MODE, value=attr_value)
        attr_value = sai_thrift_attribute_value_t(booldata=False)
        attr_an = sai_thrift_attribute_t(id=SAI_PORT_ATTR_AUTO_NEG_MODE, value=attr_value)

        for port_num in port_num_list:
            print port_list_tmp[port_num]
            self.client.sai_thrift_set_port_attribute(port_list_tmp[port_num], attr)
            self.client.sai_thrift_set_port_attribute(port_list_tmp[port_num], attr_fec)
            self.client.sai_thrift_set_port_attribute(port_list_tmp[port_num], attr_an)

