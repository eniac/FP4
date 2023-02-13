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

#ifndef __SWITCH_QOS_INT_H__
#define __SWITCH_QOS_INT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_QOS_DEFAULT_TC 0

/** qos map info */
typedef struct switch_qos_map_info_s {
  /** list node */
  switch_node_t node;

  /** qos map */
  switch_qos_map_t qos_map;

  /** qos hardware handle */
  switch_pd_hdl_t pd_hdl;

} switch_qos_map_info_t;

/** qos map group identified by qos handle */
typedef struct switch_qos_map_list_s {
  /** list of qos map info */
  switch_list_t qos_map_list;

  /** qos map type */
  switch_qos_group_t qos_group;

  /** qos map direction */
  switch_direction_t direction;

  /** ingress qos map type */
  switch_qos_map_ingress_t ingress_qos_map_type;

  /** egress qos map type */
  switch_qos_map_egress_t egress_qos_map_type;

  /** total entries */
  switch_uint32_t num_entries;

} switch_qos_map_list_t;

/** qos map handle wrappers */
#define switch_qos_map_handle_create(_device) \
  switch_handle_create(                       \
      _device, SWITCH_HANDLE_TYPE_QOS_MAP, sizeof(switch_qos_map_list_t))

#define switch_qos_map_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_QOS_MAP, _handle)

#define switch_qos_map_get(_device, _handle, _info) \
  switch_handle_get(                                \
      _device, SWITCH_HANDLE_TYPE_QOS_MAP, _handle, (void **)_info)

/** qos device context */
typedef struct switch_qos_context_s {
  /** ingress qos map id allocator */
  switch_id_allocator_t *ingress_qos_map_id;

  /** tc qos map id allocator */
  switch_id_allocator_t *tc_qos_map_id;

  /** egress qos map id allocator */
  switch_id_allocator_t *egress_qos_map_id;

  /** default TC to icos/queue map */
  switch_qos_map_t tc_qos_map[SWITCH_MAX_TRAFFIC_CLASSES];

  /** tc_qos_map pd handles */
  switch_pd_hdl_t pd_tc_qos_map[SWITCH_MAX_TRAFFIC_CLASSES];

  /** tc_icos handle */
  switch_handle_t tc_icos_hdl[SWITCH_MAX_TRAFFIC_CLASSES];

  /** tc_queue handle */
  switch_handle_t tc_queue_hdl[SWITCH_MAX_TRAFFIC_CLASSES];

} switch_qos_context_t;

switch_status_t switch_qos_init(switch_device_t device);

switch_status_t switch_qos_free(switch_device_t device);

switch_status_t switch_qos_default_entries_add(switch_device_t device);

switch_status_t switch_qos_default_entries_delete(switch_device_t device);

#define SWITCH_GLOBAL_TC_ICOS_QUEUE_TABLE
#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_QOS_INT_H__ */
