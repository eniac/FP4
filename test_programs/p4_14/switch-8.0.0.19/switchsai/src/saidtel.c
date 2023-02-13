/*******************************************************************************
 * BAREFOOT NETWORKS CONFIDENTIAL & PROPRIETARY
 *
 * Copyright (c) 2015-2016 Barefoot Networks, Inc.

 * All Rights Reserved.
 *
 * NOTICE: All information contained herein is, and remains the property of
 * Barefoot Networks, Inc. and its suppliers, if any. The intellectual and
 * technical concepts contained herein are proprietary to Barefoot Networks,
 * Inc.
 * and its suppliers and may be covered by U.S. and Foreign Patents, patents in
 * process, and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material is
 * strictly forbidden unless prior written permission is obtained from
 * Barefoot Networks, Inc.
 *
 * No warranty, explicit or implicit is provided, unless granted under a
 * written agreement with Barefoot Networks, Inc.
 *
 * $Id: $
 *
 ******************************************************************************/

#include <saidtel.h>
#include "saiinternal.h"
#include <switchapi/switch.h>
#include <switchapi/switch_dtel.h>
#include <switchapi/switch_queue.h>
#include <switchapi/switch_device.h>
#include <switchapi/switch_mirror.h>

static sai_api_t api_id = SAI_API_DTEL;

//------------------------------------------------------------------------------
// DTel
//------------------------------------------------------------------------------

typedef struct sai_dtel_info_ {
  switch_handle_t oid;
  bool int_endpoint_enable;
  bool int_transit_enable;
  bool postcard_enable;
  bool drop_report_enable;
  bool queue_report_enable;
  sai_uint32_t switch_id;
  sai_uint16_t clear_cycle;
  sai_uint8_t quant_shift;
  sai_acl_field_data_t int_dscp;
  sai_uint32_t sink_port_count;
  switch_handle_t sink_ports[PORTMAP_TABLE_SIZE];
} sai_dtel_info_t;

static sai_dtel_info_t dtel_info;

sai_status_t sai_dtel_init() {
  sai_status_t status = SAI_STATUS_SUCCESS;
  SAI_MEMSET(&dtel_info, 0, sizeof(sai_dtel_info_t));
  for (switch_port_t port = 0; port < PORTMAP_TABLE_SIZE; port++) {
    dtel_info.sink_ports[port] = SWITCH_API_INVALID_HANDLE;
  }
  dtel_info.oid = sai_id_to_oid(SWITCH_HANDLE_TYPE_DTEL, 0);
  return status;
}

sai_status_t sai_set_dtel_sink_ports(uint32_t count,
                                     const sai_object_id_t *list) {
  SAI_LOG_ENTER();
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  switch_handle_t ports[PORTMAP_TABLE_SIZE];
  SAI_MEMSET(ports, 0, sizeof(switch_handle_t) * PORTMAP_TABLE_SIZE);
  switch_port_t port = 0;

  for (uint32_t i = 0; i < count; i++) {
    switch_status = switch_api_port_handle_to_id_get(device, list[i], &port);
    status = sai_switch_status_to_sai_status(switch_status);
    if (status != SAI_STATUS_SUCCESS) {
      SAI_LOG_ERROR("DTel sink ports set failed(%s)\n",
                    sai_status_to_string(status));
      return status;
    }
    ports[port] = list[i];
  }
  for (port = 0; port < PORTMAP_TABLE_SIZE; port++) {
    if (dtel_info.sink_ports[port] == SWITCH_API_INVALID_HANDLE &&
        ports[port] != SWITCH_API_INVALID_HANDLE) {
      switch_status = switch_api_dtel_int_sink_ports_add(device, port);
      SAI_LOG_INFO("DTel add sink port 0x%lx\n", port);
    }
    if (dtel_info.sink_ports[port] != SWITCH_API_INVALID_HANDLE &&
        ports[port] == SWITCH_API_INVALID_HANDLE) {
      switch_status = switch_api_dtel_int_sink_ports_delete(device, port);
      SAI_LOG_INFO("DTel delete sink port 0x%lx\n", port);
    }
    status = sai_switch_status_to_sai_status(switch_status);
    if (status != SAI_STATUS_SUCCESS) {
      SAI_LOG_ERROR("DTel sink port set failed(%s)\n",
                    sai_status_to_string(status));
      return status;
    }
    dtel_info.sink_ports[port] = ports[port];
  }
  dtel_info.sink_port_count = count;

  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

void sai_get_dtel_sink_ports(sai_object_list_t *objlist) {
  objlist->count = dtel_info.sink_port_count;
  sai_uint32_t count = 0;
  for (switch_port_t port = 0; port < PORTMAP_TABLE_SIZE; port++) {
    if (dtel_info.sink_ports[port] != SWITCH_API_INVALID_HANDLE) {
      objlist->list[count] = dtel_info.sink_ports[port];
      count++;
    }
  }
}

sai_status_t sai_set_dtel_attr(const sai_attribute_t *attr) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  switch (attr->id) {
    case SAI_DTEL_ATTR_INT_ENDPOINT_ENABLE:
      if (attr->value.booldata == true) {
        switch_status = switch_api_dtel_int_endpoint_enable(device);
        SAI_LOG_INFO("DTel enable INT endpoint\n");
      } else {
        switch_status = switch_api_dtel_int_endpoint_disable(device);
        SAI_LOG_INFO("DTel disable INT endpoint\n");
      }
      dtel_info.int_endpoint_enable = attr->value.booldata;
      break;
    case SAI_DTEL_ATTR_INT_TRANSIT_ENABLE:
      if (attr->value.booldata == true) {
        switch_status = switch_api_dtel_int_transit_enable(device);
        SAI_LOG_INFO("DTel enable INT transit\n");
      } else {
        switch_status = switch_api_dtel_int_transit_disable(device);
        SAI_LOG_INFO("DTel disable INT transit\n");
      }
      dtel_info.int_transit_enable = attr->value.booldata;
      break;
    case SAI_DTEL_ATTR_POSTCARD_ENABLE:
      if (attr->value.booldata == true) {
        switch_status = switch_api_dtel_postcard_enable(device);
        SAI_LOG_INFO("DTel enable postcard\n");
      } else {
        switch_status = switch_api_dtel_postcard_disable(device);
        SAI_LOG_INFO("DTel disable postcard\n");
      }
      dtel_info.postcard_enable = attr->value.booldata;
      break;
    case SAI_DTEL_ATTR_DROP_REPORT_ENABLE:
      if (attr->value.booldata == true) {
        switch_status = switch_api_dtel_drop_report_enable(device);
        SAI_LOG_INFO("DTel enable drop report\n");
      } else {
        switch_status = switch_api_dtel_drop_report_disable(device);
        SAI_LOG_INFO("DTel disable drop report\n");
      }
      dtel_info.drop_report_enable = attr->value.booldata;
      break;
    case SAI_DTEL_ATTR_QUEUE_REPORT_ENABLE:
      break;
    case SAI_DTEL_ATTR_SWITCH_ID:
      switch_status = switch_api_dtel_switch_id_set(device, attr->value.u32);
      dtel_info.switch_id = attr->value.u32;
      SAI_LOG_INFO("DTel set switch ID: 0x%x\n", dtel_info.switch_id);
      break;
    case SAI_DTEL_ATTR_FLOW_STATE_CLEAR_CYCLE:
      switch_status =
          switch_api_dtel_flow_state_clear_cycle(device, attr->value.u16);
      dtel_info.clear_cycle = attr->value.u16;
      SAI_LOG_INFO("DTel set flow state clear cycle: %d\n",
                   dtel_info.clear_cycle);
      break;
    case SAI_DTEL_ATTR_LATENCY_SENSITIVITY:
      switch_status =
          switch_api_dtel_latency_quantization_shift(device, attr->value.u8);
      dtel_info.quant_shift = attr->value.u8;
      SAI_LOG_INFO("DTel set latency sensitivity: %d\n", dtel_info.quant_shift);
      break;
    case SAI_DTEL_ATTR_SINK_PORT_LIST:
      switch_status = sai_set_dtel_sink_ports(attr->value.objlist.count,
                                              attr->value.objlist.list);
      break;
    case SAI_DTEL_ATTR_INT_L4_DSCP:
      switch_status = switch_api_dtel_int_dscp_value_set(
          device, attr->value.aclfield.data.u8, attr->value.aclfield.mask.u8);
      dtel_info.int_dscp.data.u8 = attr->value.aclfield.data.u8;
      dtel_info.int_dscp.mask.u8 = attr->value.aclfield.mask.u8;
      SAI_LOG_INFO("DTel set L4 DSCP value: 0x%x, mask: 0x%x\n",
                   dtel_info.int_dscp.data.u8,
                   dtel_info.int_dscp.mask.u8);
      break;
    default:
      SAI_LOG_ERROR("Unsupported dtel attribute: %d", attr->id);
      switch_status = SAI_STATUS_NOT_SUPPORTED;
  }
  status = sai_switch_status_to_sai_status(switch_status);

  return status;
}

sai_status_t sai_create_dtel(_Out_ sai_object_id_t *dtel_id,
                             _In_ sai_object_id_t switch_id,
                             _In_ uint32_t attr_count,
                             _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;

  if (!attr_list && attr_count > 0) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("DTel object create failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  uint32_t index = 0;
  for (index = 0; index < attr_count; index++) {
    status = sai_set_dtel_attr(&attr_list[index]);
    if (status != SAI_STATUS_SUCCESS) {
      SAI_LOG_ERROR("DTel failed to set dtel attribute %d: %s",
                    attr_list[index].id,
                    sai_status_to_string(status));
      return status;
    }
  }

  *dtel_id = dtel_info.oid;
  SAI_LOG_INFO("DTel create dtel object 0x%lx", dtel_info.oid);

  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

sai_status_t sai_remove_dtel(_In_ sai_object_id_t dtel_id) {
  SAI_LOG_ENTER();

  SAI_ASSERT(sai_object_type_query(dtel_id) == SAI_OBJECT_TYPE_DTEL);

  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

sai_status_t sai_get_dtel_attribute(_In_ sai_object_id_t dtel_id,
                                    _In_ uint32_t attr_count,
                                    _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  SAI_ASSERT(sai_object_type_query(dtel_id) == SAI_OBJECT_TYPE_DTEL);

  sai_status_t status = SAI_STATUS_SUCCESS;

  uint32_t index = 0;
  sai_attribute_t *attr = NULL;
  for (index = 0; index < attr_count; index++) {
    attr = &attr_list[index];
    switch (attr->id) {
      case SAI_DTEL_ATTR_INT_ENDPOINT_ENABLE:
        attr->value.booldata = dtel_info.int_endpoint_enable;
        break;
      case SAI_DTEL_ATTR_INT_TRANSIT_ENABLE:
        attr->value.booldata = dtel_info.int_transit_enable;
        break;
      case SAI_DTEL_ATTR_POSTCARD_ENABLE:
        attr->value.booldata = dtel_info.postcard_enable;
        break;
      case SAI_DTEL_ATTR_DROP_REPORT_ENABLE:
        attr->value.booldata = dtel_info.drop_report_enable;
        break;
      case SAI_DTEL_ATTR_SWITCH_ID:
        attr->value.u32 = dtel_info.switch_id;
        break;
      case SAI_DTEL_ATTR_FLOW_STATE_CLEAR_CYCLE:
        attr->value.u16 = dtel_info.clear_cycle;
        break;
      case SAI_DTEL_ATTR_LATENCY_SENSITIVITY:
        attr->value.u8 = dtel_info.quant_shift;
        break;
      case SAI_DTEL_ATTR_SINK_PORT_LIST:
        sai_get_dtel_sink_ports(&(attr->value.objlist));
        break;
      case SAI_DTEL_ATTR_INT_L4_DSCP:
        attr->value.aclfield.data.u8 = dtel_info.int_dscp.data.u8;
        attr->value.aclfield.mask.u8 = dtel_info.int_dscp.mask.u8;
        break;
    }
  }

  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

sai_status_t sai_set_dtel_attribute(_In_ sai_object_id_t dtel_id,
                                    _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();

  SAI_ASSERT(sai_object_type_query(dtel_id) == SAI_OBJECT_TYPE_DTEL);

  sai_status_t status = SAI_STATUS_SUCCESS;
  status = sai_set_dtel_attr(attr);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel failed to set dtel attribute %d: %s",
                  attr->id,
                  sai_status_to_string(status));
    return status;
  }

  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

//------------------------------------------------------------------------------
// DTel Queue Report
//------------------------------------------------------------------------------

typedef struct sai_dtel_queue_report_info_ {
  switch_handle_t oid;
  switch_handle_t queue;
  sai_uint32_t depth;
  sai_uint32_t latency;
  sai_uint32_t quota;
  bool drop;
} sai_dtel_queue_report_info_t;

typedef struct sai_dtel_queue_reports_ {
  sai_dtel_queue_report_info_t info[DTEL_QUEUE_TABLE_SIZE];
  sai_uint32_t id_stack[DTEL_QUEUE_TABLE_SIZE];
  int top;
} sai_dtel_queue_reports_t;

static sai_dtel_queue_reports_t dtel_queue_reports;

sai_status_t sai_dtel_queue_report_init() {
  sai_status_t status = SAI_STATUS_SUCCESS;
  SAI_MEMSET(&dtel_queue_reports, 0, sizeof(sai_dtel_queue_reports_t));
  for (int i = 0; i < DTEL_QUEUE_TABLE_SIZE; i++) {
    dtel_queue_reports.info[i].oid =
        sai_id_to_oid(SWITCH_HANDLE_TYPE_DTEL_QUEUE_ALERT, i);
    dtel_queue_reports.info[i].queue = SWITCH_API_INVALID_HANDLE;
    dtel_queue_reports.id_stack[i] = i;
  }
  dtel_queue_reports.top = DTEL_QUEUE_TABLE_SIZE - 1;
  return status;
}

sai_status_t sai_create_dtel_queue_report(
    _Out_ sai_object_id_t *dtel_queue_report_id,
    _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  uint32_t index = 0;
  switch_handle_t queue = SWITCH_API_INVALID_HANDLE;
  sai_uint32_t depth = 0xFFFFFFFF;
  sai_uint32_t latency = 0xFFFFFFFF;
  sai_uint32_t quota = 1000;
  bool drop = false;

  if (dtel_queue_reports.top < 1) {
    status = SAI_STATUS_TABLE_FULL;
    SAI_LOG_ERROR("DTel queue report create failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("DTel queue report create failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  for (index = 0; index < attr_count; index++) {
    switch (attr_list[index].id) {
      case SAI_DTEL_QUEUE_REPORT_ATTR_QUEUE_ID:
        queue = attr_list[index].value.oid;
        break;
      case SAI_DTEL_QUEUE_REPORT_ATTR_DEPTH_THRESHOLD:
        depth = attr_list[index].value.u32;
        break;
      case SAI_DTEL_QUEUE_REPORT_ATTR_LATENCY_THRESHOLD:
        latency = attr_list[index].value.u32;
        break;
      case SAI_DTEL_QUEUE_REPORT_ATTR_BREACH_QUOTA:
        quota = attr_list[index].value.u32;
        break;
      case SAI_DTEL_QUEUE_REPORT_ATTR_TAIL_DROP:
        drop = attr_list[index].value.booldata;
        break;
    }
  }

  switch_uint8_t qid = 0;
  switch_handle_t port_handle = 0;
  switch_port_t port = 0;
  switch_status = switch_api_queue_index_get(device, queue, &qid);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel queue report create failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  switch_status = switch_api_queue_port_get(device, queue, &port_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel queue report create failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  switch_status = switch_api_port_handle_to_id_get(device, port_handle, &port);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel queue report create failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  switch_status = switch_api_dtel_queue_report_create(
      device, port, qid, depth, latency, quota, drop);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel queue report create failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  sai_uint32_t id = dtel_queue_reports.id_stack[dtel_queue_reports.top];
  dtel_queue_reports.info[id].queue = queue;
  dtel_queue_reports.info[id].depth = depth;
  dtel_queue_reports.info[id].latency = latency;
  dtel_queue_reports.info[id].quota = quota;
  dtel_queue_reports.info[id].drop = drop;
  dtel_queue_reports.top -= 1;

  *dtel_queue_report_id = dtel_queue_reports.info[id].oid;

  SAI_LOG_INFO("DTel create queue report...");
  SAI_LOG_INFO("DTel -- queue ID 0x%lx", queue);
  SAI_LOG_INFO("DTel -- depth threshold 0x%x", depth);
  SAI_LOG_INFO("DTel -- latency threshold 0x%x", latency);
  SAI_LOG_INFO("DTel -- breach quota %d", quota);
  SAI_LOG_INFO("DTel -- tail drop %s", drop ? "true" : "false");
  SAI_LOG_INFO("DTel queue report 0x%lx created\n", *dtel_queue_report_id);

  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

sai_status_t sai_remove_dtel_queue_report(_In_ sai_object_id_t
                                              dtel_queue_report_id) {
  SAI_LOG_ENTER();

  SAI_ASSERT(sai_object_type_query(dtel_queue_report_id) ==
             SAI_OBJECT_TYPE_DTEL_QUEUE_REPORT);

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_uint32_t id = sai_oid_to_id(dtel_queue_report_id);
  if (id >= DTEL_QUEUE_TABLE_SIZE) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("DTel queue report remove failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  if (dtel_queue_reports.info[id].queue == SWITCH_API_INVALID_HANDLE) {
    status = SAI_STATUS_ITEM_NOT_FOUND;
    SAI_LOG_ERROR("DTel queue report get attribute failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  switch_handle_t queue = dtel_queue_reports.info[id].queue;

  switch_uint8_t qid = 0;
  switch_handle_t port_handle = 0;
  switch_port_t port = 0;
  switch_status = switch_api_queue_index_get(device, queue, &qid);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel queue report remove failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }
  switch_status = switch_api_queue_port_get(device, queue, &port_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel queue report remove failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  switch_status = switch_api_port_handle_to_id_get(device, port_handle, &port);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel queue report remove failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  switch_status = switch_api_dtel_queue_report_delete(device, port, qid);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel queue report remove failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  SAI_LOG_INFO("DTel queue report created: handle 0x%lx\n",
               dtel_queue_reports.info[id].queue);

  dtel_queue_reports.info[id].queue = SWITCH_API_INVALID_HANDLE;
  dtel_queue_reports.top += 1;
  dtel_queue_reports.id_stack[dtel_queue_reports.top] = id;

  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

sai_status_t sai_get_dtel_queue_report_attribute(
    _In_ sai_object_id_t dtel_queue_report_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  SAI_ASSERT(sai_object_type_query(dtel_queue_report_id) ==
             SAI_OBJECT_TYPE_DTEL_QUEUE_REPORT);

  sai_status_t status = SAI_STATUS_SUCCESS;
  sai_uint32_t id = sai_oid_to_id(dtel_queue_report_id);
  if (id >= DTEL_QUEUE_TABLE_SIZE) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("DTel queue report get attribute failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  if (dtel_queue_reports.info[id].queue == SWITCH_API_INVALID_HANDLE) {
    status = SAI_STATUS_ITEM_NOT_FOUND;
    SAI_LOG_ERROR("DTel queue report get attribute failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  uint32_t index = 0;
  sai_attribute_t *attr = NULL;
  for (index = 0; index < attr_count; index++) {
    attr = &attr_list[index];
    switch (attr->id) {
      case SAI_DTEL_QUEUE_REPORT_ATTR_QUEUE_ID:
        attr->value.oid = dtel_queue_reports.info[id].queue;
        break;
      case SAI_DTEL_QUEUE_REPORT_ATTR_DEPTH_THRESHOLD:
        attr->value.u32 = dtel_queue_reports.info[id].depth;
        break;
      case SAI_DTEL_QUEUE_REPORT_ATTR_LATENCY_THRESHOLD:
        attr->value.u32 = dtel_queue_reports.info[id].latency;
        break;
      case SAI_DTEL_QUEUE_REPORT_ATTR_BREACH_QUOTA:
        attr->value.u32 = dtel_queue_reports.info[id].quota;
        break;
      case SAI_DTEL_QUEUE_REPORT_ATTR_TAIL_DROP:
        attr->value.booldata = dtel_queue_reports.info[id].drop;
        break;
    }
  }

  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

sai_status_t sai_set_dtel_queue_report_attribute(
    _In_ sai_object_id_t dtel_queue_report_id,
    _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();

  SAI_ASSERT(sai_object_type_query(dtel_queue_report_id) ==
             SAI_OBJECT_TYPE_DTEL_QUEUE_REPORT);

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_uint32_t id = sai_oid_to_id(dtel_queue_report_id);
  if (id >= DTEL_QUEUE_TABLE_SIZE) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("DTel queue report set attribute failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  if (dtel_queue_reports.info[id].queue == SWITCH_API_INVALID_HANDLE) {
    status = SAI_STATUS_ITEM_NOT_FOUND;
    SAI_LOG_ERROR("DTel queue report get attribute failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  switch_handle_t queue = dtel_queue_reports.info[id].queue;
  sai_uint32_t depth = dtel_queue_reports.info[id].depth;
  sai_uint32_t latency = dtel_queue_reports.info[id].latency;
  sai_uint32_t quota = dtel_queue_reports.info[id].quota;
  bool drop = dtel_queue_reports.info[id].drop;

  switch (attr->id) {
    case SAI_DTEL_QUEUE_REPORT_ATTR_QUEUE_ID:
      status = SAI_STATUS_INVALID_PARAMETER;
      SAI_LOG_ERROR("DTel queue report set attribute failed(%s)\n",
                    sai_status_to_string(status));
      return status;
    case SAI_DTEL_QUEUE_REPORT_ATTR_DEPTH_THRESHOLD:
      depth = attr->value.u32;
      if (depth == dtel_queue_reports.info[id].depth) {
        return SAI_STATUS_SUCCESS;
      }
      break;
    case SAI_DTEL_QUEUE_REPORT_ATTR_LATENCY_THRESHOLD:
      latency = attr->value.u32;
      if (latency == dtel_queue_reports.info[id].latency) {
        return SAI_STATUS_SUCCESS;
      }
      break;
    case SAI_DTEL_QUEUE_REPORT_ATTR_BREACH_QUOTA:
      quota = attr->value.u32;
      if (quota == dtel_queue_reports.info[id].quota) {
        return SAI_STATUS_SUCCESS;
      }
      break;
    case SAI_DTEL_QUEUE_REPORT_ATTR_TAIL_DROP:
      drop = attr->value.booldata;
      if (drop == dtel_queue_reports.info[id].drop) {
        return SAI_STATUS_SUCCESS;
      }
      break;
  }

  switch_uint8_t qid = 0;
  switch_handle_t port_handle = 0;
  switch_port_t port = 0;
  switch_status = switch_api_queue_index_get(device, queue, &qid);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel queue report set attribute failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }
  switch_status = switch_api_queue_port_get(device, queue, &port_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel queue report set attribute failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  switch_status = switch_api_port_handle_to_id_get(device, port_handle, &port);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel queue report set failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  switch_status = switch_api_dtel_queue_report_update(
      device, port, qid, depth, latency, quota, drop);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel queue report set attribute failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  dtel_queue_reports.info[id].depth = depth;
  dtel_queue_reports.info[id].latency = latency;
  dtel_queue_reports.info[id].quota = quota;
  dtel_queue_reports.info[id].drop = drop;

  SAI_LOG_INFO("DTel update queue report 0x%lx...", dtel_queue_report_id);
  SAI_LOG_INFO("DTel -- queue ID 0x%lx", queue);
  SAI_LOG_INFO("DTel -- depth threshold 0x%x", depth);
  SAI_LOG_INFO("DTel -- latency threshold 0x%x", latency);
  SAI_LOG_INFO("DTel -- breach quota %d", quota);
  SAI_LOG_INFO("DTel -- tail drop %s\n", drop ? "true" : "false");

  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

//------------------------------------------------------------------------------
// DTel INT Session
//------------------------------------------------------------------------------

#define DTEL_MAX_INT_SESSION_NUM 64

typedef struct sai_dtel_int_session_info_ {
  switch_handle_t oid;
  bool created;
  sai_uint8_t max_hop;
  bool collect_switch_id;
  bool collect_switch_ports;
  bool collect_ig_tstamp;
  bool collect_eg_tstamp;
  bool collect_queue_info;
  sai_uint32_t instruction;
} sai_dtel_int_session_info_t;

typedef struct sai_dtel_int_sessions_ {
  sai_dtel_int_session_info_t info[DTEL_MAX_INT_SESSION_NUM];
  sai_uint32_t id_stack[DTEL_MAX_INT_SESSION_NUM];
  int top;
} sai_dtel_int_sessions_t;

static sai_dtel_int_sessions_t dtel_int_sessions;

sai_status_t sai_dtel_int_session_init() {
  sai_status_t status = SAI_STATUS_SUCCESS;
  SAI_MEMSET(&dtel_int_sessions, 0, sizeof(sai_dtel_int_sessions_t));
  for (int i = 0; i < DTEL_MAX_INT_SESSION_NUM; i++) {
    dtel_int_sessions.info[i].oid =
        sai_id_to_oid(SWITCH_HANDLE_TYPE_DTEL_INT_SESSION, i);
    dtel_int_sessions.info[i].created = false;
    dtel_int_sessions.id_stack[i] = i;
  }
  dtel_int_sessions.top = DTEL_MAX_INT_SESSION_NUM - 1;
  return status;
}

sai_status_t sai_create_dtel_int_session(
    _Out_ sai_object_id_t *dtel_int_session_id,
    _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_uint32_t index = 0;

  sai_uint8_t max_hop = 0;
  bool collect_switch_id = false;
  bool collect_switch_ports = false;
  bool collect_ig_tstamp = false;
  bool collect_eg_tstamp = false;
  bool collect_queue_info = false;

  if (dtel_int_sessions.top < 1) {
    status = SAI_STATUS_TABLE_FULL;
    SAI_LOG_ERROR("DTel INT session create failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("DTel INT session create failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  for (index = 0; index < attr_count; index++) {
    switch (attr_list[index].id) {
      case SAI_DTEL_INT_SESSION_ATTR_MAX_HOP_COUNT:
        max_hop = attr_list[index].value.u8;
        break;
      case SAI_DTEL_INT_SESSION_ATTR_COLLECT_SWITCH_ID:
        collect_switch_id = attr_list[index].value.booldata;
        break;
      case SAI_DTEL_INT_SESSION_ATTR_COLLECT_SWITCH_PORTS:
        collect_switch_ports = attr_list[index].value.booldata;
        break;
      case SAI_DTEL_INT_SESSION_ATTR_COLLECT_INGRESS_TIMESTAMP:
        collect_ig_tstamp = attr_list[index].value.booldata;
        break;
      case SAI_DTEL_INT_SESSION_ATTR_COLLECT_EGRESS_TIMESTAMP:
        collect_eg_tstamp = attr_list[index].value.booldata;
        break;
      case SAI_DTEL_INT_SESSION_ATTR_COLLECT_QUEUE_INFO:
        collect_queue_info = attr_list[index].value.booldata;
        break;
    }
  }

  sai_uint32_t int_inst = 0;
  if (collect_switch_id) {
    int_inst |= 0x8000;
  }
  if (collect_switch_ports) {
    int_inst |= 0x4000;
  }
  if (collect_ig_tstamp) {
    int_inst |= 0x800;
  }
  if (collect_eg_tstamp) {
    int_inst |= 0x400;
  }
  if (collect_queue_info) {
    int_inst |= 0x1000;
  }

  if (int_inst == 0) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("DTel INT session create failed(%s), no instruction\n",
                  sai_status_to_string(status));
    return status;
  }

  sai_uint32_t sid = dtel_int_sessions.id_stack[dtel_int_sessions.top];

  switch_status =
      switch_api_dtel_int_session_create(device, sid, int_inst, max_hop);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel INT session create failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  dtel_int_sessions.info[sid].created = true;
  dtel_int_sessions.info[sid].max_hop = max_hop;
  dtel_int_sessions.info[sid].collect_switch_id = collect_switch_id;
  dtel_int_sessions.info[sid].collect_switch_ports = collect_switch_ports;
  dtel_int_sessions.info[sid].collect_ig_tstamp = collect_ig_tstamp;
  dtel_int_sessions.info[sid].collect_eg_tstamp = collect_eg_tstamp;
  dtel_int_sessions.info[sid].collect_queue_info = collect_queue_info;
  dtel_int_sessions.info[sid].instruction = int_inst;
  dtel_queue_reports.top -= 1;

  *dtel_int_session_id = dtel_int_sessions.info[sid].oid;

  SAI_LOG_INFO("DTel create INT session...");
  SAI_LOG_INFO("DTel -- max_hop %d", max_hop);
  SAI_LOG_INFO("DTel -- collect_switch_id    %s",
               collect_switch_id ? "true" : "false");
  SAI_LOG_INFO("DTel -- collect_switch_ports %s",
               collect_switch_ports ? "true" : "false");
  SAI_LOG_INFO("DTel -- collect_ig_tstamp    %s",
               collect_ig_tstamp ? "true" : "false");
  SAI_LOG_INFO("DTel -- collect_eg_tstamp    %s",
               collect_eg_tstamp ? "true" : "false");
  SAI_LOG_INFO("DTel -- collect_queue_info   %s",
               collect_queue_info ? "true" : "false");
  SAI_LOG_INFO("DTel INT session 0x%lx created\n", *dtel_int_session_id);

  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

sai_status_t sai_remove_dtel_int_session(_In_ sai_object_id_t
                                             dtel_int_session_id) {
  SAI_LOG_ENTER();

  SAI_ASSERT(sai_object_type_query(dtel_int_session_id) ==
             SAI_OBJECT_TYPE_DTEL_INT_SESSION);

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_uint16_t sid = sai_oid_to_id(dtel_int_session_id);

  if (sid >= DTEL_MAX_INT_SESSION_NUM) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("DTel INT session remove failed(%s), invalid ID\n",
                  sai_status_to_string(status));
    return status;
  }

  if (!dtel_int_sessions.info[sid].created) {
    status = SAI_STATUS_ITEM_NOT_FOUND;
    SAI_LOG_ERROR("DTel INT session remove failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  switch_status = switch_api_dtel_int_session_delete(device, sid);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel INT session remove failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  dtel_int_sessions.info[sid].created = false;
  dtel_int_sessions.top += 1;
  dtel_int_sessions.id_stack[dtel_int_sessions.top] = sid;

  SAI_LOG_INFO("DTel INT session 0x%lx removed\n", dtel_int_session_id);

  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

sai_status_t sai_get_dtel_int_session_attribute(
    _In_ sai_object_id_t dtel_int_session_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  SAI_ASSERT(sai_object_type_query(dtel_int_session_id) ==
             SAI_OBJECT_TYPE_DTEL_INT_SESSION);

  sai_status_t status = SAI_STATUS_SUCCESS;
  sai_uint16_t sid = sai_oid_to_id(dtel_int_session_id);

  if (sid >= DTEL_MAX_INT_SESSION_NUM) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("DTel INT session get attr failed(%s), invalid ID\n",
                  sai_status_to_string(status));
    return status;
  }

  if (!dtel_int_sessions.info[sid].created) {
    status = SAI_STATUS_ITEM_NOT_FOUND;
    SAI_LOG_ERROR("DTel INT session get attr failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  uint32_t index = 0;
  sai_attribute_t *attr = NULL;
  for (index = 0; index < attr_count; index++) {
    attr = &attr_list[index];
    switch (attr->id) {
      case SAI_DTEL_INT_SESSION_ATTR_MAX_HOP_COUNT:
        attr->value.u8 = dtel_int_sessions.info[sid].max_hop;
        break;
      case SAI_DTEL_INT_SESSION_ATTR_COLLECT_SWITCH_ID:
        attr->value.booldata = dtel_int_sessions.info[sid].collect_switch_id;
        break;
      case SAI_DTEL_INT_SESSION_ATTR_COLLECT_SWITCH_PORTS:
        attr->value.booldata = dtel_int_sessions.info[sid].collect_switch_ports;
        break;
      case SAI_DTEL_INT_SESSION_ATTR_COLLECT_INGRESS_TIMESTAMP:
        attr->value.booldata = dtel_int_sessions.info[sid].collect_ig_tstamp;
        break;
      case SAI_DTEL_INT_SESSION_ATTR_COLLECT_EGRESS_TIMESTAMP:
        attr->value.booldata = dtel_int_sessions.info[sid].collect_eg_tstamp;
        break;
      case SAI_DTEL_INT_SESSION_ATTR_COLLECT_QUEUE_INFO:
        attr->value.booldata = dtel_int_sessions.info[sid].collect_queue_info;
        break;
    }
  }

  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

sai_status_t sai_set_dtel_int_session_attribute(
    _In_ sai_object_id_t dtel_int_session_id,
    _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_uint16_t sid = sai_oid_to_id(dtel_int_session_id);

  SAI_ASSERT(sai_object_type_query(dtel_int_session_id) ==
             SAI_OBJECT_TYPE_DTEL_INT_SESSION);

  if (sid >= DTEL_MAX_INT_SESSION_NUM) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("DTel INT session get attr failed(%s), invalid ID\n",
                  sai_status_to_string(status));
    return status;
  }

  if (!dtel_int_sessions.info[sid].created) {
    status = SAI_STATUS_ITEM_NOT_FOUND;
    SAI_LOG_ERROR("DTel INT session get attr failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  sai_uint8_t max_hop = dtel_int_sessions.info[sid].max_hop;
  bool collect_switch_id = dtel_int_sessions.info[sid].collect_switch_id;
  bool collect_switch_ports = dtel_int_sessions.info[sid].collect_switch_ports;
  bool collect_ig_tstamp = dtel_int_sessions.info[sid].collect_ig_tstamp;
  bool collect_eg_tstamp = dtel_int_sessions.info[sid].collect_eg_tstamp;
  bool collect_queue_info = dtel_int_sessions.info[sid].collect_queue_info;

  switch (attr->id) {
    case SAI_DTEL_INT_SESSION_ATTR_MAX_HOP_COUNT:
      max_hop = attr->value.u8;
      if (max_hop == dtel_int_sessions.info[sid].max_hop) {
        return SAI_STATUS_SUCCESS;
      }
      break;
    case SAI_DTEL_INT_SESSION_ATTR_COLLECT_SWITCH_ID:
      collect_switch_id = attr->value.booldata;
      break;
    case SAI_DTEL_INT_SESSION_ATTR_COLLECT_SWITCH_PORTS:
      collect_switch_ports = attr->value.booldata;
      break;
    case SAI_DTEL_INT_SESSION_ATTR_COLLECT_INGRESS_TIMESTAMP:
      collect_ig_tstamp = attr->value.booldata;
      break;
    case SAI_DTEL_INT_SESSION_ATTR_COLLECT_EGRESS_TIMESTAMP:
      collect_eg_tstamp = attr->value.booldata;
      break;
    case SAI_DTEL_INT_SESSION_ATTR_COLLECT_QUEUE_INFO:
      collect_queue_info = attr->value.booldata;
      break;
  }

  sai_uint32_t int_inst = 0;
  if (collect_switch_id) {
    int_inst |= 0x8000;
  }
  if (collect_switch_ports) {
    int_inst |= 0x4000;
  }
  if (collect_ig_tstamp) {
    int_inst |= 0x800;
  }
  if (collect_eg_tstamp) {
    int_inst |= 0x400;
  }
  if (collect_queue_info) {
    int_inst |= 0x1000;
  }

  if (int_inst == dtel_int_sessions.info[sid].instruction) {
    return SAI_STATUS_SUCCESS;
  }

  if (int_inst == 0) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("DTel INT session create failed(%s), no instruction\n",
                  sai_status_to_string(status));
    return status;
  }

  switch_status =
      switch_api_dtel_int_session_update(device, sid, int_inst, max_hop);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel INT session create failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  dtel_int_sessions.info[sid].max_hop = max_hop;
  dtel_int_sessions.info[sid].collect_switch_id = collect_switch_id;
  dtel_int_sessions.info[sid].collect_switch_ports = collect_switch_ports;
  dtel_int_sessions.info[sid].collect_ig_tstamp = collect_ig_tstamp;
  dtel_int_sessions.info[sid].collect_eg_tstamp = collect_eg_tstamp;
  dtel_int_sessions.info[sid].collect_queue_info = collect_queue_info;
  dtel_int_sessions.info[sid].instruction = int_inst;

  SAI_LOG_INFO("DTel update INT session 0x%lx...", dtel_int_session_id);
  SAI_LOG_INFO("DTel -- max_hop %d", max_hop);
  SAI_LOG_INFO("DTel -- collect_switch_id    %s",
               collect_switch_id ? "true" : "false");
  SAI_LOG_INFO("DTel -- collect_switch_ports %s",
               collect_switch_ports ? "true" : "false");
  SAI_LOG_INFO("DTel -- collect_ig_tstamp    %s",
               collect_ig_tstamp ? "true" : "false");
  SAI_LOG_INFO("DTel -- collect_eg_tstamp    %s",
               collect_eg_tstamp ? "true" : "false");
  SAI_LOG_INFO("DTel -- collect_queue_info   %s\n",
               collect_queue_info ? "true" : "false");

  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

//------------------------------------------------------------------------------
// DTel Report Session
//------------------------------------------------------------------------------

typedef struct sai_dtel_report_session_info_ {
  switch_handle_t oid;
  sai_ip_address_t src_ip;
  sai_ip_address_list_t dst_ip_list;
  sai_object_id_t vrf_id;
  sai_uint16_t truncate_size;
  sai_uint16_t udp_port;
  switch_handle_t mirror_sessions[SWITCH_MAX_MIRROR_SESSIONS];
  switch_mac_addr_t src_mac;
} sai_dtel_report_session_info_t;

static sai_dtel_report_session_info_t dtel_report_session;

sai_status_t sai_dtel_report_session_init() {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  SAI_MEMSET(&dtel_report_session, 0, sizeof(sai_dtel_report_session_info_t));

  switch_api_device_info_t api_device_info;
  memset(&api_device_info, 0x0, sizeof(api_device_info));
  switch_uint64_t flags = 0;
  flags |= SWITCH_DEVICE_ATTR_DEFAULT_MAC;
  flags |= SWITCH_DEVICE_ATTR_DEFAULT_VRF_HANDLE;

  switch_status =
      switch_api_device_attribute_get(device, flags, &api_device_info);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel report session failed to get switch attribute: %s",
                  sai_status_to_string(status));
    return status;
  }

  memcpy(&dtel_report_session.src_mac, &api_device_info.mac, 6);
  dtel_report_session.vrf_id = api_device_info.vrf_handle;

  return status;
}

sai_status_t sai_create_dtel_report_session(
    _Out_ sai_object_id_t *dtel_report_session_id,
    _In_ sai_object_id_t switch_id,
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  uint32_t index = 0;

  if (dtel_report_session.oid != 0) {
    status = SAI_STATUS_TABLE_FULL;
    SAI_LOG_ERROR("DTel report session create failed (%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  const sai_attribute_t *attr = NULL;
  for (index = 0; index < attr_count; index++) {
    attr = &attr_list[index];
    switch (attr->id) {
      case SAI_DTEL_REPORT_SESSION_ATTR_SRC_IP:
        memcpy(&dtel_report_session.src_ip,
               &attr->value.ipaddr,
               sizeof(sai_ip_address_t));
        break;
      case SAI_DTEL_REPORT_SESSION_ATTR_DST_IP_LIST:
        dtel_report_session.dst_ip_list.list =
            SAI_MALLOC(sizeof(sai_ip_address_t) * attr->value.ipaddrlist.count);
        memcpy(dtel_report_session.dst_ip_list.list,
               attr->value.ipaddrlist.list,
               sizeof(sai_ip_address_t) * attr->value.ipaddrlist.count);
        dtel_report_session.dst_ip_list.count = attr->value.ipaddrlist.count;
        break;
      case SAI_DTEL_REPORT_SESSION_ATTR_VIRTUAL_ROUTER_ID:
        dtel_report_session.vrf_id = attr->value.oid;
        break;
      case SAI_DTEL_REPORT_SESSION_ATTR_TRUNCATE_SIZE:
        dtel_report_session.truncate_size = attr->value.u16;
        break;
      case SAI_DTEL_REPORT_SESSION_ATTR_UDP_DST_PORT:
        dtel_report_session.udp_port = attr->value.u16;
        break;
    }
  }

  switch_api_mirror_info_t api_mirror_info;
  switch_handle_t session_handle = SWITCH_API_INVALID_HANDLE;
  memset(&api_mirror_info, 0, sizeof(switch_api_mirror_info_t));

  // mirror type
  api_mirror_info.mirror_type = SWITCH_MIRROR_TYPE_DTEL_REPORT;
  api_mirror_info.span_mode = SWITCH_MIRROR_SPAN_MODE_TUNNEL_PARAMS;
  // vrf id
  api_mirror_info.vrf_handle = dtel_report_session.vrf_id;
  // source IP
  sai_ip_addr_to_switch_ip_addr(&dtel_report_session.src_ip,
                                &api_mirror_info.src_ip);
  // truncate size
  api_mirror_info.max_pkt_len = dtel_report_session.truncate_size;
  // source MAC

  switch_api_device_info_t api_device_info;
  memset(&api_device_info, 0x0, sizeof(api_device_info));
  switch_uint64_t flags = 0;
  flags |= SWITCH_DEVICE_ATTR_DEFAULT_MAC;
  flags |= SWITCH_DEVICE_ATTR_DEFAULT_VRF_HANDLE;

  switch_status =
      switch_api_device_attribute_get(device, flags, &api_device_info);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel report session failed to get switch attribute: %s",
                  sai_status_to_string(status));
    return status;
  }

  memcpy(&dtel_report_session.src_mac, &api_device_info.mac, 6);
  memcpy(&api_mirror_info.src_mac, &api_device_info.mac, 6);

  // destination UDP port
  switch_status = switch_api_dtel_report_udp_dstport_set(
      device, dtel_report_session.udp_port);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel report session set UDP port failed: %s",
                  sai_status_to_string(status));
    return status;
  }

  SAI_LOG_INFO("DTel create report session...");

  SAI_LOG_INFO("DTel -- report session src IP %08x",
               api_mirror_info.src_ip.ip.v4addr);

  for (index = 0; index < dtel_report_session.dst_ip_list.count; index++) {
    // destination IP
    sai_ip_addr_to_switch_ip_addr(&dtel_report_session.dst_ip_list.list[index],
                                  &api_mirror_info.dst_ip);
    SAI_LOG_INFO("DTel -- report session dst IP 0x%08x",
                 api_mirror_info.dst_ip.ip.v4addr);

    // create mirror session
    api_mirror_info.session_id = 0;
    switch_status = switch_api_mirror_session_create(
        device, &api_mirror_info, &session_handle);
    status = sai_switch_status_to_sai_status(switch_status);
    if (status != SAI_STATUS_SUCCESS) {
      SAI_LOG_ERROR("DTel mirror session create failed: %s",
                    sai_status_to_string(status));
      return status;
    }
    dtel_report_session.mirror_sessions[index] = session_handle;
    // add to dtel mirror sessions
    switch_status =
        switch_api_dtel_report_session_add(device, api_mirror_info.session_id);
    status = sai_switch_status_to_sai_status(switch_status);
    if (status != SAI_STATUS_SUCCESS) {
      SAI_LOG_ERROR("DTel dtel mirror session add failed: %s",
                    sai_status_to_string(status));
      return status;
    }
  }

  dtel_report_session.oid =
      sai_id_to_oid(SWITCH_HANDLE_TYPE_DTEL_REPORT_SESSION, 0);
  *dtel_report_session_id = dtel_report_session.oid;

  SAI_LOG_INFO("DTel -- report session udp_port %d",
               dtel_report_session.udp_port);
  SAI_LOG_INFO("DTel -- report session truncate_size %d",
               dtel_report_session.truncate_size);
  SAI_LOG_INFO("DTel report session 0x%lx created\n", dtel_report_session.oid);
  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

sai_status_t sai_remove_dtel_report_session(_In_ sai_object_id_t
                                                dtel_report_session_id) {
  SAI_LOG_ENTER();

  SAI_ASSERT(sai_object_type_query(dtel_report_session_id) ==
             SAI_OBJECT_TYPE_DTEL_REPORT_SESSION);

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  uint32_t index = 0;

  if (dtel_report_session_id != dtel_report_session.oid) {
    status = SAI_STATUS_ITEM_NOT_FOUND;
    SAI_LOG_ERROR("DTel report session remove failed (%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  switch_api_mirror_info_t mirror_info;
  switch_handle_t session_handle;
  for (index = 0; index < dtel_report_session.dst_ip_list.count; index++) {
    session_handle = dtel_report_session.mirror_sessions[index];
    switch_status = switch_api_mirror_session_info_get(
        device, session_handle, &mirror_info);
    status = sai_switch_status_to_sai_status(switch_status);
    if (status != SAI_STATUS_SUCCESS) {
      SAI_LOG_ERROR("DTel mirror session get failed: %s",
                    sai_status_to_string(status));
      return status;
    }

    switch_status =
        switch_api_dtel_report_session_delete(device, mirror_info.session_id);
    status = sai_switch_status_to_sai_status(switch_status);
    if (status != SAI_STATUS_SUCCESS) {
      SAI_LOG_ERROR("DTel dtel mirror session delete failed: %s",
                    sai_status_to_string(status));
      return status;
    }

    switch_status = switch_api_mirror_session_delete(device, session_handle);
    status = sai_switch_status_to_sai_status(switch_status);
    if (status != SAI_STATUS_SUCCESS) {
      SAI_LOG_ERROR("DTel mirror session delete failed: %s",
                    sai_status_to_string(status));
      return status;
    }
  }

  dtel_report_session.oid = 0;
  dtel_report_session.dst_ip_list.count = 0;
  SAI_FREE(dtel_report_session.dst_ip_list.list);

  SAI_LOG_INFO("DTel report session 0x%lx removed\n", dtel_report_session_id);

  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

sai_status_t sai_get_dtel_report_session_attribute(
    _In_ sai_object_id_t dtel_report_session_id,
    _In_ uint32_t attr_count,
    _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();
  SAI_ASSERT(sai_object_type_query(dtel_report_session_id) ==
             SAI_OBJECT_TYPE_DTEL_REPORT_SESSION);

  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

sai_status_t sai_set_dtel_report_session_attribute(
    _In_ sai_object_id_t dtel_report_session_id,
    _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();
  SAI_ASSERT(sai_object_type_query(dtel_report_session_id) ==
             SAI_OBJECT_TYPE_DTEL_REPORT_SESSION);

  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

//------------------------------------------------------------------------------
// DTel event
//------------------------------------------------------------------------------

static switch_dtel_event_type_t sai_dtel_event_to_switch(
    _In_ sai_dtel_event_type_t event_type) {
  switch (event_type) {
    case SAI_DTEL_EVENT_TYPE_FLOW_STATE:
      return SWITCH_DTEL_EVENT_TYPE_FLOW_STATE_CHANGE;
    case SAI_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS:
      return SWITCH_DTEL_EVENT_TYPE_FLOW_REPORT_ALL_PACKETS;
    case SAI_DTEL_EVENT_TYPE_FLOW_TCPFLAG:
      return SWITCH_DTEL_EVENT_TYPE_FLOW_TCPFLAG;
    case SAI_DTEL_EVENT_TYPE_QUEUE_REPORT_THRESHOLD_BREACH:
      return SWITCH_DTEL_EVENT_TYPE_Q_REPORT_THRESHOLD_BREACH;
    case SAI_DTEL_EVENT_TYPE_QUEUE_REPORT_TAIL_DROP:
      return SWITCH_DTEL_EVENT_TYPE_Q_REPORT_TAIL_DROP;
    case SAI_DTEL_EVENT_TYPE_DROP_REPORT:
      return SWITCH_DTEL_EVENT_TYPE_DROP_REPORT;
    default:
      return SWITCH_DTEL_EVENT_TYPE_MAX;
  }
}

typedef struct sai_dtel_events_info_ {
  switch_handle_t oid[SAI_DTEL_EVENT_TYPE_MAX];
  sai_uint8_t dscp[SAI_DTEL_EVENT_TYPE_MAX];
  switch_handle_t report_session;
} sai_dtel_events_info_t;

static sai_dtel_events_info_t dtel_events;

sai_status_t sai_dtel_event_init() {
  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_MEMSET(&dtel_events, 0, sizeof(sai_dtel_events_info_t));
  for (int type = 0; type < SAI_DTEL_EVENT_TYPE_MAX; type++) {
    dtel_events.oid[type] = sai_id_to_oid(SWITCH_HANDLE_TYPE_DTEL_EVENT, type);
  }

  return status;
}

sai_status_t sai_create_dtel_event(_Out_ sai_object_id_t *dtel_event_id,
                                   _In_ sai_object_id_t switch_id,
                                   _In_ uint32_t attr_count,
                                   _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  uint32_t index = 0;

  sai_dtel_event_type_t type = SAI_DTEL_EVENT_TYPE_MAX;
  sai_uint8_t dscp = 0;

  const sai_attribute_t *attr = NULL;
  for (index = 0; index < attr_count; index++) {
    attr = &attr_list[index];
    switch (attr->id) {
      case SAI_DTEL_EVENT_ATTR_TYPE:
        type = attr->value.s32;
        break;
      case SAI_DTEL_EVENT_ATTR_REPORT_SESSION:
        dtel_events.report_session = attr->value.oid;
        break;
      case SAI_DTEL_EVENT_ATTR_DSCP_VALUE:
        dscp = attr->value.u8;
        break;
    }
  }

  switch_status = switch_api_dtel_event_set_dscp(
      device, sai_dtel_event_to_switch(type), dscp);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel event set DSCP failed: %s",
                  sai_status_to_string(status));
    return status;
  }

  dtel_events.dscp[type] = dscp;
  *dtel_event_id = dtel_events.oid[type];

  SAI_LOG_INFO("DTel event created: ID 0x%lx, report_session 0x%lx, DSCP %d\n",
               dtel_events.oid[type],
               dtel_events.report_session,
               dscp);

  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

sai_status_t sai_remove_dtel_event(_In_ sai_object_id_t dtel_event_id) {
  SAI_LOG_ENTER();
  SAI_ASSERT(sai_object_type_query(dtel_event_id) ==
             SAI_OBJECT_TYPE_DTEL_EVENT);

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  sai_dtel_event_type_t type = sai_oid_to_id(dtel_event_id);
  if (type >= SAI_DTEL_EVENT_TYPE_MAX) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("DTel event remove failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  switch_status =
      switch_api_dtel_event_set_dscp(device, sai_dtel_event_to_switch(type), 0);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel event remove failed: %s", sai_status_to_string(status));
    return status;
  }

  SAI_LOG_INFO("DTel event 0x%lx removed\n", dtel_events.oid[type]);

  dtel_events.dscp[type] = 0;

  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

sai_status_t sai_get_dtel_event_attribute(_In_ sai_object_id_t dtel_event_id,
                                          _In_ uint32_t attr_count,
                                          _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();
  SAI_ASSERT(sai_object_type_query(dtel_event_id) ==
             SAI_OBJECT_TYPE_DTEL_EVENT);

  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

sai_status_t sai_set_dtel_event_attribute(_In_ sai_object_id_t dtel_event_id,
                                          _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();
  SAI_ASSERT(sai_object_type_query(dtel_event_id) ==
             SAI_OBJECT_TYPE_DTEL_EVENT);

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;

  sai_dtel_event_type_t type = sai_oid_to_id(dtel_event_id);
  if (type >= SAI_DTEL_EVENT_TYPE_MAX) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("DTel event remove failed(%s)\n",
                  sai_status_to_string(status));
    return status;
  }

  sai_uint8_t dscp = dtel_events.dscp[type];
  switch (attr->id) {
    case SAI_DTEL_EVENT_ATTR_DSCP_VALUE:
      dscp = attr->value.u8;
      if (dscp == dtel_events.dscp[type]) {
        return SAI_STATUS_SUCCESS;
      }
      break;
    default:
      status = SAI_STATUS_NOT_SUPPORTED;
      SAI_LOG_ERROR("DTel event set failed(%s)\n",
                    sai_status_to_string(status));
      return status;
  }

  switch_status = switch_api_dtel_event_set_dscp(
      device, sai_dtel_event_to_switch(type), dscp);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("DTel event remove failed: %s", sai_status_to_string(status));
    return status;
  }
  dtel_events.dscp[type] = dscp;

  SAI_LOG_INFO("DTel event updated: ID 0x%lx, report_session 0x%lx, DSCP %d\n",
               dtel_events.oid[type],
               dtel_events.report_session,
               dscp);

  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

/*
 *  DTel methods table retrieved with sai_api_query()
 */
sai_dtel_api_t dtel_api = {
    .create_dtel = sai_create_dtel,
    .remove_dtel = sai_remove_dtel,
    .get_dtel_attribute = sai_get_dtel_attribute,
    .set_dtel_attribute = sai_set_dtel_attribute,
    .create_dtel_queue_report = sai_create_dtel_queue_report,
    .remove_dtel_queue_report = sai_remove_dtel_queue_report,
    .get_dtel_queue_report_attribute = sai_get_dtel_queue_report_attribute,
    .set_dtel_queue_report_attribute = sai_set_dtel_queue_report_attribute,
    .create_dtel_int_session = sai_create_dtel_int_session,
    .remove_dtel_int_session = sai_remove_dtel_int_session,
    .get_dtel_int_session_attribute = sai_get_dtel_int_session_attribute,
    .set_dtel_int_session_attribute = sai_set_dtel_int_session_attribute,
    .create_dtel_report_session = sai_create_dtel_report_session,
    .remove_dtel_report_session = sai_remove_dtel_report_session,
    .get_dtel_report_session_attribute = sai_get_dtel_report_session_attribute,
    .set_dtel_report_session_attribute = sai_set_dtel_report_session_attribute,
    .create_dtel_event = sai_create_dtel_event,
    .remove_dtel_event = sai_remove_dtel_event,
    .get_dtel_event_attribute = sai_get_dtel_event_attribute,
    .set_dtel_event_attribute = sai_set_dtel_event_attribute};
sai_status_t sai_dtel_initialize(sai_api_service_t *sai_api_service) {
  SAI_LOG_DEBUG("Initializing DTel");
  sai_api_service->dtel_api = dtel_api;
  sai_dtel_init();
  sai_dtel_queue_report_init();
  sai_dtel_int_session_init();
  sai_dtel_report_session_init();
  sai_dtel_event_init();
  return SAI_STATUS_SUCCESS;
}
