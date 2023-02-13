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
#ifndef __SWITCH_QUEUE_H__
#define __SWITCH_QUEUE_H__

#include "switch_base_types.h"
#include "switch_handle.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup QUEUE QUEUE API
 *  API functions to allocate queues
 *  @{
 */  // begin of QUEUE API

// QUEUE
/** QUEUE information */

typedef enum switch_queue_attribute_s {
  SWITCH_QUEUE_ATTR_PORT_HANDLE = (1 << 0),
  SWITCH_QUEUE_ATTR_QUEUE_ID = (1 << 1)
} switch_queue_attribute_t;

typedef struct switch_api_queue_info_s {
  /** port handle */
  switch_handle_t port_handle;

  /** queue id */
  switch_qid_t queue_id;

} switch_api_queue_info_t;

/** max number of queues */
#define SWITCH_MAX_QUEUE 8

/** max number of port queues */
#define SWITCH_MAX_PORT_QUEUE 8

/** max number of CPU queues */
#define SWITCH_MAX_CPU_QUEUE 32

/** max number of traffic classes */
#define SWITCH_MAX_TRAFFIC_CLASSES 32

/**
 Get port queues
 @param device device
 @param port_handle port handle
 @param num_queues number of queues
 @param queue_handles list of queue handles
*/
switch_status_t switch_api_queues_get(switch_device_t device,
                                      switch_handle_t port_handle,
                                      switch_uint32_t *num_queues,
                                      switch_handle_t *queue_handles);

switch_status_t switch_api_queue_create(
    const switch_device_t device,
    const switch_uint64_t flags,
    const switch_api_queue_info_t *api_queue_info,
    switch_handle_t *queue_handle);

switch_status_t switch_api_queue_delete(switch_device_t device,
                                        switch_handle_t port_handle);

switch_status_t switch_api_queue_color_drop_enable(switch_device_t device,
                                                   switch_handle_t queue_handle,
                                                   bool enable);

switch_status_t switch_api_queue_color_limit_set(switch_device_t device,
                                                 switch_handle_t queue_handle,
                                                 switch_color_t color,
                                                 switch_uint32_t limit);

switch_status_t switch_api_queue_color_hysteresis_set(
    switch_device_t device,
    switch_handle_t queue_handle,
    switch_color_t color,
    switch_uint32_t limit);

switch_status_t switch_api_queue_pfc_cos_mapping(switch_device_t device,
                                                 switch_handle_t queue_handle,
                                                 switch_uint8_t cos);

switch_status_t switch_api_max_queues_get(switch_device_t device,
                                          switch_uint32_t *max_queues);
switch_status_t switch_api_max_cpu_queues_get(switch_device_t device,
                                              switch_uint32_t *max_queues);

switch_status_t switch_api_max_traffic_class_get(
    switch_device_t device, switch_uint32_t *traffic_classes);

switch_status_t switch_api_dtel_tail_drop_deflection_queue_set(
    switch_device_t device,
    switch_pipe_t pipe_id,
    switch_handle_t queue_handle);

switch_status_t switch_api_queue_handle_dump(const switch_device_t device,
                                             const switch_handle_t queue_handle,
                                             const void *cli_ctx);

/**
 set the guaranteed min rate of queue scheduler
 @param device device
 @param pps pps or bps flag
 @param queue queue handle
 @param burst burst
 @param rate guaranteed min rate in bps or pps.
*/
switch_status_t switch_api_queue_guaranteed_rate_set(
    switch_device_t device,
    switch_handle_t queue_handle,
    bool pps,
    uint32_t burst,
    uint32_t rate);

/**
 set the priority of queue scheduler
 @param device device
 @param queue queue handle
 @param priority priority
*/
switch_status_t switch_api_queue_strict_priority_set(
    switch_device_t device, switch_handle_t queue_handle, bool priority);

/**
 set the weight of queue scheduler
 @param device device
 @param queue_handle queue handle
 @param weight weight
*/
switch_status_t switch_api_queue_dwrr_weight_set(switch_device_t device,
                                                 switch_handle_t queue_handle,
                                                 uint16_t weight);

/**
 Set the queue shaper
 @param device device
 @pps pps/bps flag.
 @param queue_handle queue handle
 @param burst burst size in bytes
 @param rate rate in bps or in pps.
*/
switch_status_t switch_api_queue_shaping_set(switch_device_t device,
                                             switch_handle_t queue_handle,
                                             bool pps,
                                             switch_uint32_t burst,
                                             switch_uint64_t rate);
/**
 Get the queue index
 @param device device
 @param queue_handle queue handle.
*/
switch_status_t switch_api_queue_index_get(switch_device_t device,
                                           switch_handle_t queue_handle,
                                           switch_uint8_t *queue_index);

/**
 Get the queue port handle
 @param device device
 @param queue_handle queue handle.
*/
switch_status_t switch_api_queue_port_get(switch_device_t device,
                                          switch_handle_t queue_handle,
                                          switch_handle_t *port_handle);

switch_status_t switch_api_queue_drop_get(switch_device_t device,
                                          switch_handle_t queue_handle,
                                          uint64_t *num_packets);

/** @} */  // end of QUEUE API
#ifdef __cplusplus
}
#endif

#endif
