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
Thrift SAI interface queue tests
"""
import socket
import sai_base_test
from ptf import config
from ptf.testutils import *
from ptf.thriftutils import *
from switchsai_thrift.sai_headers import  *
from switch_utils import *

class schedulerGroupAttribute(sai_base_test.ThriftInterfaceDataPlane):
    def checkSchedulerAttribute(self, scheduler_id, attr_list):
      for attr_id, attr_value in attr_list.iteritems():
        thrift_attr = self.client.sai_thrift_get_scheduler_profile(scheduler_id, sai_thrift_attribute_t(id=attr_id))
        if ((attr_id == SAI_SCHEDULER_ATTR_SCHEDULING_TYPE) or \
          (attr_id == SAI_SCHEDULER_ATTR_METER_TYPE) and \
          (attr_value[1] == thrift_attr.value.u32)):
          print "Scheduler type/meter type is correct %d"%(thrift_attr.value.u32)
        elif((attr_id == SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT) and \
            (attr_value[1] == thrift_attr.value.u8)):
          print "Scheduler weight is correct %d"%(thrift_attr.value.u8)
        elif((attr_id == SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE) or \
             (attr_id == SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE) or \
             (attr_id == SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE) or \
             (attr_id == SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE) and \
             (attr_value[1] == thrift_attr.value.u64)):
          print "Min and Max rate parameters are correct %lu"%(thrift_attr.value.u64)
        else:
          print "Attr %s is incorrect, expected %d and got %d"%(attr_value[0], attr_value[1], thrift_attr.value.u64)
          return 0

    def runTest(self):
        switch_init(self.client)
        port1 = port_list[0]
        thrift_sai_port_attr = sai_thrift_attribute_t(id = SAI_PORT_ATTR_QOS_SCHEDULER_GROUP_LIST)
        attr_list = self.client.sai_thrift_get_port_handles_attribute(port1, thrift_sai_port_attr);

        group_handle = []
        num_sch_groups = attr_list.value.objlist.count;
        sch_group_list = attr_list.value.objlist.object_id_list
        for sch_handle in sch_group_list:
          thrift_attr = sai_thrift_attribute_t(id = SAI_SCHEDULER_GROUP_ATTR_CHILD_LIST)
          thrift_list = self.client.sai_thrift_get_scheduler_group_attribute(sch_handle, thrift_attr)
          handle_list = thrift_list.value.objlist.object_id_list
          group_handle.append(sch_handle)
          #print "Queue handles from sch are 0x%lx"%(handle_list[0])

        for handle in group_handle:
          print "Scheduler group handle 0x%lx"%(handle)

        thrift_sai_port_attr = sai_thrift_attribute_t(id = SAI_PORT_ATTR_QOS_QUEUE_LIST)
        attr_list = self.client.sai_thrift_get_port_handles_attribute(port1, thrift_sai_port_attr);
        num_queues = attr_list.value.objlist.count;
        for queue_handle in attr_list.value.objlist.object_id_list:
          print "Queue handle from port are 0x%lx"%(queue_handle)

    
        weight = 100
        min_rate = 10000000
        min_burst = 1024
        max_rate = 500000000
        max_burst = 1024
        scheduler_attributes = {SAI_SCHEDULER_ATTR_SCHEDULING_TYPE:
                                ["Scheduler type", SAI_SCHEDULING_TYPE_DWRR],
                                SAI_SCHEDULER_ATTR_METER_TYPE:
                                ["Scheduler meter type", SAI_METER_TYPE_BYTES],
                                SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT:
                                ["Scheduler weight", weight],
                                SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE:
                                ["Scheduler min rate", min_rate],
                                SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE:
                                ["Scheduler min burst size", min_burst],
                                SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE:
                                ["Scheduler max rate", max_rate],
                                SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE:
                                ["Scheduler max burst", max_burst]}
        scheduler_id = \
          sai_thrift_create_scheduler_profile(self.client, 0, weight, max_rate, max_burst, min_rate, min_burst)
        schedulerGroupAttribute.checkSchedulerAttribute(self, scheduler_id, scheduler_attributes)

        scheduler_attributes[SAI_SCHEDULER_ATTR_SCHEDULING_TYPE][1] = SAI_SCHEDULING_TYPE_STRICT;

        prio_scheduler_id = \
          sai_thrift_create_scheduler_profile(self.client, 1, weight, max_rate, max_burst, min_rate, min_burst)
        schedulerGroupAttribute.checkSchedulerAttribute(self, prio_scheduler_id, scheduler_attributes)


        dwrr_attr_value = sai_thrift_attribute_value_t(oid = scheduler_id)
        dwrr_attr = sai_thrift_attribute_t(id = SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID, value = dwrr_attr_value)

        prio_attr_value = sai_thrift_attribute_value_t(oid = prio_scheduler_id)
        prio_attr = sai_thrift_attribute_t(id = SAI_SCHEDULER_GROUP_ATTR_SCHEDULER_PROFILE_ID, value = prio_attr_value)

        #Set all queue scheduler groups as DWRR 
        #for handle in group_handle:
        #  status = self.client.sai_thrift_set_scheduler_group(handle, dwrr_attr)

        self.client.sai_thrift_set_scheduler_group(group_handle[1],dwrr_attr)
        #Set first queue scheduler groups as Strict 
        self.client.sai_thrift_set_scheduler_group(group_handle[0],prio_attr)

        max_rate = 400000000
        attr_value = sai_thrift_attribute_value_t(u64 = max_rate)
        attr = sai_thrift_attribute_t(id = SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE, value = attr_value)
        self.client.sai_thrift_set_scheduler_profile(scheduler_id, attr)

class BufferAttributeTest(sai_base_test.ThriftInterfaceDataPlane):
  def checkBufferAttribute(self, pool_id, pool_attr):
    for attr_id, attr_value in pool_attr.iteritems():
      thrift_attr = sai_thrift_get_buffer_pool_attribute(self.client, pool_id, attr_id)

      if (attr_id == SAI_BUFFER_POOL_ATTR_TYPE) and \
         thrift_attr.value.u32 == attr_value[1]:
        print "Attr %s is correct"%(attr_value[0])
      elif(attr_id == SAI_BUFFER_POOL_ATTR_SIZE) and \
        thrift_attr.value.u32 == attr_value[1]:
        print "Attr %s is correct"%(attr_value[0])
      elif(attr_id == SAI_BUFFER_POOL_ATTR_THRESHOLD_MODE) and \
        thrift_attr.value.u32 == attr_value[1]:
        print "Attr %s is correct"%(attr_value[0])
      else:
        print "Attr %s is incorrect, got %d, expected %d"%(attr_value[0], thrift_attr.value.u32, attr_value[1])
        return 0;
    return 1

  def runTest(self):
    ingress_pool_size = 1024
    egress_pool_size = 10240

    ig_buffer_pool_id = sai_thrift_create_pool_profile(self.client, SAI_BUFFER_POOL_TYPE_INGRESS, ingress_pool_size, SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC)
    print "Ingress buffer handle 0x%lx"%(ig_buffer_pool_id)
    eg_buffer_pool_id = sai_thrift_create_pool_profile(self.client, SAI_BUFFER_POOL_TYPE_EGRESS, egress_pool_size, SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC)
    print "Egress buffer pool handle 0x%lx"%(eg_buffer_pool_id)

    buffer_pool_attr_list = {SAI_BUFFER_POOL_ATTR_SIZE:
                            ["Buffer pool size", ingress_pool_size],
                            SAI_BUFFER_POOL_ATTR_TYPE:
                            ["Buffer pool type", SAI_BUFFER_POOL_TYPE_INGRESS],
                            SAI_BUFFER_POOL_ATTR_THRESHOLD_MODE:
                            ["Buffer threshold mode", SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC]
                            }
    BufferAttributeTest.checkBufferAttribute(self, ig_buffer_pool_id, buffer_pool_attr_list)
    BufferAttributeTest.checkBufferAttribute(self, eg_buffer_pool_id, buffer_pool_attr_list)

    buffer_pool_attr_list[SAI_BUFFER_POOL_ATTR_SIZE][1] = egress_pool_size;
    buffer_pool_attr_list[SAI_BUFFER_POOL_ATTR_TYPE][1] = SAI_BUFFER_POOL_TYPE_EGRESS;

    switch_init(self.client)
    port1 = port_list[0]
    ig_buffer_profile = sai_thrift_create_buffer_profile(self.client, ig_buffer_pool_id, 1024, 50, 30, 10)
    print "Ingress buffer profile handle %lx"%(ig_buffer_profile)

    eg_buffer_profile = sai_thrift_create_buffer_profile(self.client, eg_buffer_pool_id, 1024, 50, 30, 10)
    print "Egress buffer profile handle %lx"%(eg_buffer_profile)

    thrift_sai_port_attr = sai_thrift_attribute_t(id = SAI_PORT_ATTR_QOS_QUEUE_LIST)
    attr_list = self.client.sai_thrift_get_port_handles_attribute(port1, thrift_sai_port_attr);
    num_queues = attr_list.value.objlist.count;
    for queue_handle in attr_list.value.objlist.object_id_list:
      print "Attaching buffer_profile  - Queue handle from port 0x%lx, Egress buffer prof 0x%lx"%(queue_handle, eg_buffer_profile)
      attr_value = sai_thrift_attribute_value_t(oid = eg_buffer_profile)
      attr = sai_thrift_attribute_t(id = SAI_QUEUE_ATTR_BUFFER_PROFILE_ID, value = attr_value)
      self.client.sai_thrift_set_queue_attribute(queue_handle, attr)

    try:
      ret = BufferAttributeTest.checkBufferAttribute(self, eg_buffer_pool_id,buffer_pool_attr_list) 
      if ret == 0 : 
        self.fai_flag = True;
        self.assertFalse(self.fail_flag, "Some buffer pool attributes are incorrect")

    finally:
      sai_thrift_remove_buffer_pool(self.client, ig_buffer_pool_id)
      sai_thrift_remove_buffer_pool(self.client, eg_buffer_pool_id)
      #for prof_id in buffer_profile_list:
      #  sai_thrift_remove_buffer_profile(self.client, prof_id)

class PortPPGAttributeTest(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        print "Running port attribute PPG test"
        switch_init(self.client)
        port1 = port_list[0]
        thrift_sai_port_attr = sai_thrift_attribute_t(id = SAI_PORT_ATTR_INGRESS_PRIORITY_GROUP_LIST)
        attr_list = self.client.sai_thrift_get_port_handles_attribute(port1, thrift_sai_port_attr);
        num_ppgs = attr_list.value.objlist.count;
        ppg_handle_list = attr_list.value.objlist.object_id_list
        print "Number of PPGS per port is %d"%(num_ppgs)
        for ppg_handle in ppg_handle_list:
          print "PPG Handles are 0x%lx"%(ppg_handle)

      
class PortPfcPpgQosmapAttribute(sai_base_test.ThriftInterfaceDataPlane):
  
    def runTest(self):
        switch_init(self.client)
        port1 = port_list[0]

        ingress_pfc_cos_list = [1, 2, 3, 4]
        ingress_pg_list = [1, 1, 1, 1]

        ingress_qos_map_id = sai_thrift_create_qos_map(
            self.client, SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_PRIORITY_GROUP, ingress_pfc_cos_list,
            ingress_pg_list)

        print "Qos map handle 0x%lx"%(ingress_qos_map_id)
        sai_thrift_set_port_attribute(self.client, port1,
                                      SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_PRIORITY_GROUP_MAP,
                                      ingress_qos_map_id)

class PortPfcQueueQosmapAttribute(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        switch_init(self.client)
        port1 = port_list[0]

        pfc_cos_list = [1, 2, 3, 4]
        queue_list = [0, 0, 0, 0]

        qos_map_id = sai_thrift_create_qos_map(
            self.client, SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_QUEUE, pfc_cos_list,
            queue_list)

        print "Qos map handle 0x%lx"%(qos_map_id)
        sai_thrift_set_port_attribute(self.client, port1,
                                      SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP,
                                      qos_map_id)

class PortSchedulerProfileAttribute(sai_base_test.ThriftInterfaceDataPlane):
    def runTest(self):
        switch_init(self.client)
        port1 = port_list[0]

        weight = 100
        min_rate = 10000000
        min_burst = 1024
        max_rate = 500000000
        max_burst = 1024
        scheduler_attributes = {SAI_SCHEDULER_ATTR_SCHEDULING_TYPE:
                                ["Scheduler type", SAI_SCHEDULING_TYPE_DWRR],
                                SAI_SCHEDULER_ATTR_METER_TYPE:
                                ["Scheduler meter type", SAI_METER_TYPE_BYTES],
                                SAI_SCHEDULER_ATTR_SCHEDULING_WEIGHT:
                                ["Scheduler weight", weight],
                                SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_RATE:
                                ["Scheduler min rate", min_rate],
                                SAI_SCHEDULER_ATTR_MIN_BANDWIDTH_BURST_RATE:
                                ["Scheduler min burst size", min_burst],
                                SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_RATE:
                                ["Scheduler max rate", max_rate],
                                SAI_SCHEDULER_ATTR_MAX_BANDWIDTH_BURST_RATE:
                                ["Scheduler max burst", max_burst]}
        scheduler_id = \
          sai_thrift_create_scheduler_profile(self.client, 0, weight, max_rate, max_burst, min_rate, min_burst)

        sai_thrift_set_port_attribute(self.client, port1,
                                      SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID,
                                      scheduler_id)

        thrift_sai_port_attr = sai_thrift_attribute_t(id = SAI_PORT_ATTR_QOS_SCHEDULER_PROFILE_ID)
        attr_list = self.client.sai_thrift_get_port_handles_attribute(port1, thrift_sai_port_attr);
        if scheduler_id != attr_list.value.oid:
          self.fail_flag = True;
          self.assertFalse(self.fail_flag, "Port scheduler profile id is incorrect")
        else:
          print "Port scheduler profile id is correct"
          
class QosMapAttributeTest(sai_base_test.ThriftInterfaceDataPlane):
    def checkDscpToTc(self, key_list, value_list, thrift_key, thrift_value, map_count):
        rev = 0
        for i in range(map_count):
          print key_list[i], thrift_key[i].dscp, value_list[i], thrift_value[i].tc
          if key_list[i] != thrift_key[i].dscp or value_list[i] != thrift_value[i].tc:
            rev = 1;
        if i == map_count:
            return 1;
        j = 0
        if rev == 1:
          for i in range(map_count -1, -1, -1):
            print key_list[j], thrift_key[i].dscp, value_list[j], thrift_value[i].tc
            if key_list[j] != thrift_key[i].dscp or value_list[j] != thrift_value[i].tc:
              return 0;
            j = j+1
          return 1;

    def checkAttribute(self, qos_map_id, attr_list, map_count):
        for attr_id, attr_values in attr_list.iteritems():
            thrift_attr = sai_thrift_get_qos_map_attribute(self.client, qos_map_id, attr_id, map_count)
            if(attr_id == SAI_QOS_MAP_ATTR_TYPE and \
               attr_values[1] == thrift_attr.value.u32):
                print "QoSmap type is correct"
            elif (attr_id == SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST):
                if QosMapAttributeTest.checkDscpToTc(self, attr_values[1][0], \
                                                     attr_values[1][1], \
                                                     thrift_attr.value.qosmap.key, \
                                                     thrift_attr.value.qosmap.data, \
                                                     map_count) == 1:
                  print "Attribute %s is correct"%(attr_values[0])
                else:
                  print "Attribute map list is incorrect"

    def runTest(self):
        ingress_dscp_list = [1, 2, 3, 4]
        ingress_tc_list = [11, 12, 13, 14]
        ingress_qos_map_id = sai_thrift_create_qos_map(
            self.client, SAI_QOS_MAP_TYPE_DSCP_TO_TC, ingress_dscp_list,
            ingress_tc_list)
        attr_list = {SAI_QOS_MAP_ATTR_TYPE:
                     ["QoS map attribute type", SAI_QOS_MAP_TYPE_DSCP_TO_TC],
                     SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST:
                     ["QoS map value list", [ingress_dscp_list, ingress_tc_list]]}
        QosMapAttributeTest.checkAttribute(self, ingress_qos_map_id, attr_list, 4)
        ingress_tc_list = [15,16,17,18]
        attr_list1 = {SAI_QOS_MAP_ATTR_TYPE:
                     ["QoS map attribute type", SAI_QOS_MAP_TYPE_DSCP_TO_TC],
                     SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST:
                     ["QoS map value list", [ingress_dscp_list, ingress_tc_list]]}
        sai_thrift_set_qos_map_attribute(self.client, ingress_qos_map_id, SAI_QOS_MAP_TYPE_DSCP_TO_TC, ingress_dscp_list, ingress_tc_list)
        QosMapAttributeTest.checkAttribute(self, ingress_qos_map_id, attr_list1, 4)
        #sai_thrift_remove_qos_map(self.client, ingress_qos_map_id)


class PortPfcTest(sai_base_test.ThriftInterfaceDataPlane):
  def runTest(self):
    switch_init(self.client)
    port1 = port_list[0]

    ingress_pool_size = 20480

    ig_buffer_pool_id = sai_thrift_create_pool_profile(self.client, SAI_BUFFER_POOL_TYPE_INGRESS, ingress_pool_size, SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC)
    print "Ingress buffer profile handle 0x%lx"%(ig_buffer_pool_id)

    ig_buffer_profile = sai_thrift_create_buffer_profile(self.client, ig_buffer_pool_id, ingress_pool_size, 50, 10000, 5000)
    print "Ingress buffer profile handle %lx"%(ig_buffer_profile)

    thrift_sai_port_attr = sai_thrift_attribute_t(id = SAI_PORT_ATTR_INGRESS_PRIORITY_GROUP_LIST)
    attr_list = self.client.sai_thrift_get_port_handles_attribute(port1, thrift_sai_port_attr);
    num_ppgs = attr_list.value.objlist.count;
    ppg_handle_list = attr_list.value.objlist.object_id_list
    print "Number of PPGS per port is %d"%(num_ppgs)
    for ppg_handle in ppg_handle_list:
      print "PPG Handles are 0x%lx"%(ppg_handle)

    thrift_attr_value = sai_thrift_attribute_value_t(oid = ig_buffer_profile)
    thrift_attr = sai_thrift_attribute_t(id = SAI_INGRESS_PRIORITY_GROUP_ATTR_BUFFER_PROFILE, value = thrift_attr_value)
    self.client.sai_thrift_set_priority_group_attribute(ppg_handle_list[0], thrift_attr)

    ingress_dscp_list = [3, 4, 8, 40]
    ingress_tc_list = [3, 4, 1, 0]
    ingress_qos_map_id = sai_thrift_create_qos_map(
        self.client, SAI_QOS_MAP_TYPE_DSCP_TO_TC, ingress_dscp_list,
        ingress_tc_list)
    attr_list = {SAI_QOS_MAP_ATTR_TYPE:
                 ["QoS map attribute type", SAI_QOS_MAP_TYPE_DSCP_TO_TC],
                 SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST:
                 ["QoS map value list", [ingress_dscp_list, ingress_tc_list]]}

    ingress_pfc_cos_list = [0, 1, 3, 4]
    egress_queue_list = [0, 1, 3, 4]

    ingress_qos_map_id = sai_thrift_create_qos_map(
        self.client, SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_QUEUE, ingress_pfc_cos_list,
        egress_queue_list)

    print "Qos map handle 0x%lx"%(ingress_qos_map_id)
    sai_thrift_set_port_attribute(self.client, port1,
                                  SAI_PORT_ATTR_QOS_PFC_PRIORITY_TO_QUEUE_MAP,
                                  ingress_qos_map_id)
    sai_thrift_set_port_attribute(self.client, port1,
                                  SAI_PORT_ATTR_PRIORITY_FLOW_CONTROL,
                                  0x18)

class TcQosMapAttributeTest(sai_base_test.ThriftInterfaceDataPlane):
    def checkTcToQueue(self, key_list, value_list, thrift_key, thrift_value, map_count):
        rev = 0
        for i in range(map_count):
          print key_list[i], thrift_key[i].tc, value_list[i], thrift_value[i].queue_index
          if key_list[i] != thrift_key[i].tc or value_list[i] != thrift_value[i].queue_index:
            rev = 1;
        print "Value of i %d after validation"%(i)
        if i == (map_count - 1):
            return 1;
        j = 0
        if rev == 1:
          for i in range(map_count -1, -1, -1):
            print key_list[j], thrift_key[i].tc, value_list[j], thrift_value[i].queue_index
            if key_list[j] != thrift_key[i].tc or value_list[j] != thrift_value[i].queue_index:
              return 0;
            j = j+1
          return 1;

    def checkTcToPg(self, key_list, value_list, thrift_key, thrift_value, map_count):
        rev = 0
        for i in range(map_count):
          print key_list[i], thrift_key[i].tc, value_list[i], thrift_value[i].pg
          if key_list[i] != thrift_key[i].tc or value_list[i] != thrift_value[i].pg:
            rev = 1;
        if i == (map_count - 1):
            return 1;
        j = 0
        if rev == 1:
          for i in range(map_count -1, -1, -1):
            print key_list[j], thrift_key[i].tc, value_list[j], thrift_value[i].pg
            if key_list[j] != thrift_key[i].tc or value_list[j] != thrift_value[i].pg:
              return 0;
            j = j+1
          return 1;

    def checkAttribute(self, qos_map_id, attr_list, map_type, map_count):
        for attr_id, attr_values in attr_list.iteritems():
            thrift_attr = sai_thrift_get_qos_map_attribute(self.client, qos_map_id, attr_id, map_count)
            if(attr_id == SAI_QOS_MAP_ATTR_TYPE and \
               attr_values[1] == thrift_attr.value.u32):
                print "QoSmap type is correct"
            elif (attr_id == SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST and map_type == SAI_QOS_MAP_TYPE_TC_TO_QUEUE):
                if TcQosMapAttributeTest.checkTcToQueue(self, attr_values[1][0], \
                                                     attr_values[1][1], \
                                                     thrift_attr.value.qosmap.key, \
                                                     thrift_attr.value.qosmap.data, \
                                                     map_count) == 1:
                  print "TC to queue Attribute %s is correct"%(attr_values[0])
                else:
                  print "TC to queue Attribute map list is incorrect"
            else:
                if TcQosMapAttributeTest.checkTcToPg(self, attr_values[1][0], \
                                                     attr_values[1][1], \
                                                     thrift_attr.value.qosmap.key, \
                                                     thrift_attr.value.qosmap.data, \
                                                     map_count) == 1:
                  print "TC to PG Attribute %s is correct"%(attr_values[0])
                else:
                  print "TC to PG Attribute map list is incorrect"

    def runTest(self):
        switch_init(self.client)
        port1 = port_list[0]

        ingress_tc_list = [1, 2, 3, 4]
        ingress_queue_list = [4, 3, 2, 1]
        ingress_icos_list = [0, 1, 0, 1]
        ingress_qos_map_id = sai_thrift_create_qos_map(
            self.client, SAI_QOS_MAP_TYPE_TC_TO_PRIORITY_GROUP, ingress_tc_list,
            ingress_icos_list)
        print "Ingress qosmap id 0x%x"%(ingress_qos_map_id)
        attr_list = {SAI_QOS_MAP_ATTR_TYPE:
                     ["QoS map attribute type", SAI_QOS_MAP_TYPE_TC_TO_PRIORITY_GROUP],
                     SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST:
                     ["QoS map value list", [ingress_tc_list, ingress_icos_list]]}

        TcQosMapAttributeTest.checkAttribute(self, ingress_qos_map_id, attr_list, SAI_QOS_MAP_TYPE_TC_TO_PRIORITY_GROUP, 4)
        sai_thrift_set_port_attribute(self.client, port1,
                                      SAI_PORT_ATTR_QOS_TC_TO_PRIORITY_GROUP_MAP,
                                      ingress_qos_map_id)

        ingress_qos_map_id = sai_thrift_create_qos_map(
            self.client, SAI_QOS_MAP_TYPE_TC_TO_QUEUE, ingress_tc_list,
            ingress_queue_list)
        attr_list = {SAI_QOS_MAP_ATTR_TYPE:
                     ["QoS map attribute type", SAI_QOS_MAP_TYPE_TC_TO_QUEUE],
                     SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST:
                     ["QoS map value list", [ingress_tc_list, ingress_queue_list]]}

        print "Ingress qosmap id 0x%x"%(ingress_qos_map_id)
        TcQosMapAttributeTest.checkAttribute(self, ingress_qos_map_id, attr_list, SAI_QOS_MAP_TYPE_TC_TO_QUEUE, 4)
