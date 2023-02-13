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

#include "switchapi/switch_sflow.h"
#include "switch_pd_types.h"

#ifndef _switch_sflow_int_h_
#define _switch_sflow_int_h_

#define switch_sflow_handle_create(_device) \
  switch_handle_create(                     \
      _device, SWITCH_HANDLE_TYPE_SFLOW, sizeof(switch_sflow_info_t))

#define switch_sflow_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_SFLOW, _handle)

#define switch_sflow_get(_device, _handle, _info) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_SFLOW, _handle, (void **)_info)

#define switch_sflow_ace_handle_create(_device)      \
  switch_handle_create(_device,                      \
                       SWITCH_HANDLE_TYPE_SFLOW_ACE, \
                       sizeof(switch_sflow_match_entry_t))

#define switch_sflow_ace_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_SFLOW_ACE, _handle)

#define switch_sflow_ace_get(_device, _handle, _info) \
  switch_handle_get(                                  \
      _device, SWITCH_HANDLE_TYPE_SFLOW_ACE, _handle, (void **)_info)

typedef struct switch_sflow_match__key_ {
  switch_handle_t port;
  switch_uint16_t vlan;
  switch_uint32_t sip;
  switch_uint32_t sip_mask;
  switch_uint32_t dip;
  switch_uint32_t dip_mask;
} switch_sflow_match_key_t;

typedef struct switch_sflow_match_entry_ {
  switch_node_t node;
  switch_pd_hdl_t ingress_sflow_pd_hdl;
  switch_handle_t sflow_ace_hdl;
} switch_sflow_match_entry_t;

typedef struct switch_sflow_info_ {
  switch_api_sflow_session_info_t api_info;
  switch_uint8_t session_id;
  switch_handle_t mirror_handle;
  switch_pd_hdl_t mirror_table_ent_hdl;

  // use tommy list to store all the match key_value_pairs
  // using this sflow_session
  switch_list_t match_list;
} switch_sflow_info_t;

typedef struct switch_sflow_context_ {
  switch_array_t *sflow_array;
  switch_array_t *sflow_ace_array;
} switch_sflow_context_t;

switch_status_t switch_sflow_init(switch_device_t device);

switch_status_t switch_sflow_free(switch_device_t device);

switch_status_t switch_sflow_default_entries_add(switch_device_t device);

switch_status_t switch_sflow_default_entries_delete(switch_device_t device);

#endif /* _switch_sflow_int_h_ */
