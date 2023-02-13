/*
Copyright 2013-present Barefoot Networks, Inc.
*/

#ifndef _switch_scheduler_h_
#define _switch_scheduler_h_

#include "switch_base_types.h"
#include "switch_handle.h"
#include "switch_meter.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup Scheduler Scheduler API
 *  API functions to create Scheduler
 *  @{
 */  // begin of Scheduler API

// Scheduler
/** Scheduler information */

/** scheduler type */
typedef enum switch_scheduler_type_ {
  SWITCH_SCHEDULER_MODE_STRICT = 1,
  SWITCH_SCHEDULER_MODE_DWRR = 2,
  SWITCH_SCHEDULER_MODE_STRICT_AND_DWRR = 3
} switch_scheduler_type_t;

/** scheduler rate type */
typedef enum switch_scheduler_rate_type_ {
  SWITCH_SCHEDULER_PPS = 1,
  SWITCH_SCHEDULER_BPS = 2
} switch_scheduler_rate_type_t;

/** scheduler struct */
typedef struct switch_scheduler_api_info_ {
  switch_scheduler_type_t scheduler_type;   /**< scheduler type */
  switch_handle_t queue_handle;             /**< queue handle */
  switch_scheduler_rate_type_t shaper_type; /** < scheduler rate type */
  uint32_t priority;                        /**< priority */
  uint32_t rem_bw_priority;                 /**< remaining bandwidth priority */
  uint16_t weight;                          /**< weight */
  switch_uint64_t min_burst_size;           /**< minimum burst size */
  switch_uint64_t min_rate;                 /**< minimum rate in bps or pps */
  switch_uint64_t max_burst_size;           /**< maximum burst size */
  switch_uint64_t max_rate;                 /**<maximum rate in bps or pps > */
} switch_scheduler_api_info_t;

typedef enum switch_scheduler_group_type_ {
  SWITCH_SCHEDULER_GROUP_TYPE_PORT = 1,
  SWITCH_SCHEDULER_GROUP_TYPE_QUEUE = 2,
} switch_scheduler_group_type_t;

typedef struct switch_scheduler_group_api_info_ {
  switch_scheduler_group_type_t group_type;
  switch_handle_t port_handle;
  switch_handle_t scheduler_handle;
  switch_handle_t queue_handle;
} switch_scheduler_group_api_info_t;

/**
 Delete a scheduler
 @param device device
 @param scheduler_handle scheduler handle
*/
switch_status_t switch_api_scheduler_delete(switch_device_t device,
                                            switch_handle_t scheduler_handle);

/**
 enable scheduling in queue
 @param device device
 @param scheduler_handle scheduler handle
 @param enable enable
*/
switch_status_t switch_api_queue_scheduling_enable(
    switch_device_t device, switch_handle_t scheduler_handle, bool enable);

/**
 set the priority of queue scheduler
 @param device device
 @param scheduler_handle scheduler handle
 @param priority priority
*/
switch_status_t switch_api_queue_scheduling_remaining_bw_priority_set(
    switch_device_t device,
    switch_handle_t scheduler_handle,
    uint32_t priority);

/**
 configure queue scheduler
 @param device device
 @param scheduler_handle scheduler handle
 @param pps packet per second
 @param burst_size burst size
 @param rate rate
*/
switch_status_t switch_api_queue_scheduling_guaranteed_shaping_set(
    switch_device_t device,
    switch_handle_t scheduler_handle,
    bool pps,
    uint32_t burst_size,
    uint32_t rate);

/**
 configure queue remaining bandwidth scheduler
 @param device device
 @param scheduler_handle scheduler handle
 @param pps packet per second
 @param burst_size burst size
 @param rate rate
*/
switch_status_t switch_api_queue_scheduling_dwrr_shaping_set(
    switch_device_t device,
    switch_handle_t scheduler_handle,
    bool pps,
    uint32_t burst_size,
    uint32_t rate);

/**
 set port shaper
 @param device device
*/
switch_status_t switch_api_port_shaping_set(switch_device_t device,
                                            switch_handle_t port_handle,
                                            bool pps,
                                            uint32_t burst_size,
                                            switch_uint64_t rate);

/**
 Create a scheduler
 @param device device
 @param scheduler_api_info scheduler api info
*/
switch_status_t switch_api_scheduler_create(
    switch_device_t device,
    const switch_scheduler_api_info_t *api_info,
    switch_handle_t *scheduler_handle);

/**
 Create a scheduler group
 @param device device
 @param scheduler_group_api_info scheduler group api info
*/
switch_status_t switch_api_scheduler_group_create(
    switch_device_t device,
    switch_scheduler_group_api_info_t *api_info,
    switch_handle_t *scheduler_group_handle);

/**
 Delete a scheduler group
 @param device device
 @param scheduler_group_handle scheduler group handle
*/
switch_status_t switch_api_scheduler_group_delete(switch_device_t device,
                                                  switch_handle_t group_handle);

/**
 Retrieve a scheduler group child handle
 @param device device
 @param scheduler_group_handle scheduler group handle
*/
switch_status_t switch_api_scheduler_group_child_handle_get(
    switch_device_t device,
    switch_handle_t scheduler_group_handle,
    switch_handle_t *queue_handle);

/**
 Retrieve a scheduler group child count
 @param device device
 @param scheduler_group_handle scheduler group handle
*/
switch_status_t switch_api_scheduler_group_child_count_get(
    switch_device_t device,
    switch_handle_t scheduler_group_handle,
    switch_uint32_t *child_count);

/**
 Retrieve scheduler profile for a scheduler group
 @param device device
 @param scheduler_group_handle scheduler group handle
*/
switch_status_t switch_api_scheduler_group_profile_get(
    switch_device_t device,
    switch_handle_t scheduler_group_handle,
    switch_handle_t *profile_handle);

/**
 Assign a scheduler profile for a scheduler group
 @param device device
 @param scheduler_group_handle scheduler group handle
 @param profile_handle scheduler profile handle
*/
switch_status_t switch_api_scheduler_group_profile_set(
    switch_device_t device,
    switch_handle_t scheduler_group_handle,
    switch_handle_t profile_handle);

/**
 Modify the scheduler profile
 @param device device
 @param scheduler_handle scheduler group handle
 @param scheduler_api_info scheduler profile parameters
*/
switch_status_t switch_api_scheduler_config_set(
    switch_device_t device,
    switch_handle_t scheduler_handle,
    const switch_scheduler_api_info_t *api_info);

/**
 Get the scheduler config for a profile
 @param device device
 @param scheduler_handle scheduler group handle
*/
switch_status_t switch_api_scheduler_config_get(
    switch_device_t device,
    switch_handle_t scheduler_handle,
    switch_scheduler_api_info_t *api_info);

/**
 Get the scheduler group config for a profile
 @param device device
 @param scheduler_handle scheduler group handle
*/
switch_status_t switch_api_scheduler_group_config_get(
    switch_device_t device,
    switch_handle_t scheduler_group_handle,
    switch_scheduler_group_api_info_t *api_info);

/** @} */  // end of Scheduler API

#ifdef __cplusplus
}
#endif

#endif
