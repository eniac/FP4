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
#ifndef _switch_qos_h_
#define _switch_qos_h_

#include "switch_base_types.h"
#include "switch_handle.h"
#include "switch_meter.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup QOS QOS API
 *  API functions to create buffers and qos maps
 *  @{
 */  // begin of QOS API

// QOS
/** QOS information */

/** qos map ingress type */
typedef enum switch_qos_map_ingress_ {
  SWITCH_QOS_MAP_INGRESS_NONE = 0,
  SWITCH_QOS_MAP_INGRESS_DSCP_TO_TC = 1,
  SWITCH_QOS_MAP_INGRESS_PCP_TO_TC = 2,
  SWITCH_QOS_MAP_INGRESS_DSCP_TO_COLOR = 3,
  SWITCH_QOS_MAP_INGRESS_PCP_TO_COLOR = 4,
  SWITCH_QOS_MAP_INGRESS_DSCP_TO_TC_AND_COLOR = 5,
  SWITCH_QOS_MAP_INGRESS_PCP_TO_TC_AND_COLOR = 6,
  SWITCH_QOS_MAP_INGRESS_DSCP_TO_QID_AND_TC_AND_COLOR = 14,
  SWITCH_QOS_MAP_INGRESS_PCP_TO_QID_AND_TC_AND_COLOR = 15,
  SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS = 7,
  SWITCH_QOS_MAP_INGRESS_TC_TO_QUEUE = 8,
  SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS_AND_QUEUE = 9,
  SWITCH_QOS_MAP_INGRESS_TOS_TO_TC = 10,
  SWITCH_QOS_MAP_INGRESS_TOS_TO_COLOR = 11,
  SWITCH_QOS_MAP_INGRESS_TOS_TO_TC_AND_COLOR = 12,
  SWITCH_QOS_MAP_INGRESS_TOS_TO_QID_AND_TC_AND_COLOR = 13,
  SWITCH_QOS_MAP_INGRESS_PFC_PRIORITY_TO_PPG = 16
} switch_qos_map_ingress_t;

/** qos map egress type */
typedef enum switch_qos_map_egress_ {
  SWITCH_QOS_MAP_EGRESS_NONE = 0,
  SWITCH_QOS_MAP_EGRESS_TC_TO_DSCP = 1,
  SWITCH_QOS_MAP_EGRESS_TC_TO_PCP = 2,
  SWITCH_QOS_MAP_EGRESS_COLOR_TO_DSCP = 3,
  SWITCH_QOS_MAP_EGRESS_COLOR_TO_PCP = 4,
  SWITCH_QOS_MAP_EGRESS_TC_AND_COLOR_TO_DSCP = 5,
  SWITCH_QOS_MAP_EGRESS_TC_AND_COLOR_TO_PCP = 6,
  SWITCH_QOS_MAP_EGRESS_PFC_PRIORITY_TO_QUEUE = 7,
  SWITCH_QOS_MAP_EGRESS_TC_TO_TOS = 8,
  SWITCH_QOS_MAP_EGRESS_COLOR_TO_TOS = 9,
  SWITCH_QOS_MAP_EGRESS_TC_AND_COLOR_TO_TOS = 10
} switch_qos_map_egress_t;

/** switch qos map struct */
typedef struct switch_qos_map_ {
  uint8_t dscp;         /**< dscp */
  uint8_t tos;          /**< tos */
  uint8_t pcp;          /**< pcp */
  uint16_t tc;          /**< traffic class */
  switch_color_t color; /**< packet color */
  uint8_t icos;         /**< ingress cos */
  uint8_t qid;          /**< queue id */
  uint8_t pfc_priority;
  uint8_t ppg;
} switch_qos_map_t;

/**
 Create ingress qos map
 @param device device
 @param map_type qos map type
 @param num_entries number of qos map entries
 @param qos_map QOS map
*/
switch_status_t switch_api_qos_map_ingress_create(
    switch_device_t device,
    switch_qos_map_ingress_t map_type,
    switch_uint8_t num_entries,
    switch_qos_map_t *qos_map,
    switch_handle_t *qos_map_handle);

/**
 Delete ingress qos map
 @param device device
 @param qos_map_handle Qos map handle
*/
switch_status_t switch_api_qos_map_ingress_delete(
    switch_device_t device, switch_handle_t qos_map_handle);

/**
 Create egress qos map
 @param device device
 @param map_type qos map type
 @param num_entries number of qos map entries
 @param qos_map QOS map
*/
switch_status_t switch_api_qos_map_egress_create(
    switch_device_t device,
    switch_qos_map_egress_t map_type,
    switch_uint8_t num_entries,
    switch_qos_map_t *qos_map,
    switch_handle_t *qos_map_handle);

/**
 Delete ingress qos map
 @param device device
 @param qos_map_handle Qos map handle
*/
switch_status_t switch_api_qos_map_egress_delete(
    switch_device_t device, switch_handle_t qos_map_handle);

/**
 Update qos map
 @param device device
 @param num_entries number of qos map entries
 @param qos_map_handle Qos map handle
 @param qos_map QOS map
*/
switch_status_t switch_api_qos_map_update(switch_device_t device,
                                          switch_handle_t qos_map_handle,
                                          switch_uint8_t num_entries,
                                          switch_qos_map_t *qos_map);

switch_status_t switch_api_qos_map_type_get(
    switch_device_t device,
    switch_handle_t qos_map_handle,
    switch_direction_t *dir,
    switch_qos_map_ingress_t *ig_map_type,
    switch_qos_map_egress_t *eg_map_type);

switch_status_t switch_api_qos_map_list_get(switch_device_t device,
                                            switch_handle_t qos_map_handle,
                                            switch_qos_map_t **qos_map_list,
                                            switch_uint32_t *num_entries);

switch_status_t switch_api_qos_map_set(switch_device_t device,
                                       switch_handle_t qos_map_handle,
                                       switch_qos_map_t *qos_map);
/** @} */  // end of QOS API

#ifdef __cplusplus
}
#endif

#endif
