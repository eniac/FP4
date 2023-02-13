/*
Copyright 2013-present Barefoot Networks, Inc.
*/

#ifndef _switch_buffer_h_
#define _switch_buffer_h_

#include "switch_base_types.h"
#include "switch_handle.h"
#include "switch_meter.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup Buffer Buffer API
 *  API functions to create buffers
 *  @{
 */  // begin of Buffer API

// Buffer
/** Buffer threshold mode */
typedef enum switch_buffer_threshold_mode_ {
  SWITCH_BUFFER_THRESHOLD_MODE_STATIC = 1,
  SWITCH_BUFFER_THRESHOLD_MODE_DYNAMIC = 2
} switch_buffer_threshold_mode_t;

/** Buffer information */
typedef struct switch_api_buffer_pool_ {
  switch_direction_t direction; /** <Ingress or Egress buffer pool> */
  switch_uint32_t pool_size;    /** buffer pool size */
  switch_buffer_threshold_mode_t
      threshold_mode;          /** buffer threshold mode - static/dynamic */
  switch_uint32_t xoff_size;   /** Ingress buffer XOFF size */
  switch_uint32_t shared_size; /** Buffer shared pool size */
} switch_api_buffer_pool_t;

/** Buffer profile struct */
typedef struct switch_api_buffer_profile_ {
  switch_buffer_threshold_mode_t threshold_mode; /**< buffer threshold mode */
  switch_handle_t pool_handle;                   /**< buffer pool handle */
  uint32_t buffer_size;                          /**< buffer size */
  uint32_t threshold;                            /**< threshold limit */
  uint32_t xoff_threshold;                       /**< xoff threshold */
  uint32_t xon_threshold;                        /**< xon threashold */
} switch_api_buffer_profile_t;

/**
 Create a buffer pool
 @param device device
 @param api_buffer_pool API buffer pool
 @param threshold_mode buffer threshold mode
*/
switch_status_t switch_api_buffer_pool_create(
    switch_device_t device,
    switch_api_buffer_pool_t api_buffer_pool,
    switch_handle_t *pool_handle);

/**
 Delete a buffer pool
 @param device device
 @param pool_handle pool handle
*/
switch_status_t switch_api_buffer_pool_delete(switch_device_t device,
                                              switch_handle_t pool_handle);

/**
 Get the buffer profile threshold mode
 @param device device
 @param pool_handle pool handle
*/
switch_status_t switch_api_buffer_pool_threshold_mode_get(
    switch_device_t device,
    switch_handle_t pool_handle,
    switch_buffer_threshold_mode_t *threshold_mode);

/**
 Set switch buffer pool size
 @param device device
 @param buffer_pool_handle buffer pool handle
 @param pool_size size of the pool
*/
switch_status_t switch_api_buffer_pool_size_set(
    switch_device_t device,
    switch_handle_t buffer_pool_handle,
    switch_uint32_t size);
/**
 Get switch buffer pool size
 @param device device
 @param buffer_pool_handle buffer pool handle
*/
switch_status_t switch_api_buffer_pool_size_get(
    switch_device_t device,
    switch_handle_t buffer_pool_handle,
    switch_uint32_t *size);
/**
 Get switch buffer pool type
 @param device device
 @param buffer_pool_handle buffer pool handle
*/
switch_status_t switch_api_buffer_pool_type_get(
    switch_device_t device,
    switch_handle_t buffer_pool_handle,
    switch_direction_t *dir);
/**
 Create switch buffer profile
 @param device device
 @param buffer_info buffer profile info
*/
switch_status_t switch_api_buffer_profile_create(
    switch_device_t device,
    switch_api_buffer_profile_t *buffer_info,
    switch_handle_t *buffer_profile_handle);

/**
 Delete switch buffer profile
 @param device device
 @param buffer_profile_handle buffer profile handle
*/
switch_status_t switch_api_buffer_profile_delete(
    switch_device_t device, switch_handle_t buffer_profile_handle);

/**
 Set buffer profile for a priority group
 @param device device
 @param pg_handle priority group handle
 @param buffer_profile_handle buffer profile handle
*/
switch_status_t switch_api_priority_group_buffer_profile_set(
    switch_device_t device,
    switch_handle_t pg_handle,
    switch_handle_t buffer_profile_handle);

/**
 Get buffer profile for a priority group
 @param device device
 @param pg_handle priority group handle
*/
switch_status_t switch_api_priority_group_buffer_profile_get(
    switch_device_t device,
    switch_handle_t pg_handle,
    switch_handle_t *buffer_profile_handle);

/**
 Get port handle for a priority group
 @param device device
 @param pg_handle priority group handle
*/
switch_status_t switch_api_priority_group_port_get(
    switch_device_t device,
    switch_handle_t pg_handle,
    switch_handle_t *port_handle);

/**
 Get PPG index for a priority group
 @param device device
 @param pg_handle priority group handle
*/
switch_status_t switch_api_priority_group_index_get(switch_device_t device,
                                                    switch_handle_t pg_handle,
                                                    switch_uint32_t *index);

/**
 Set skid buffer size
 @param device device
 @param buffer_size buffer size
*/
switch_status_t switch_api_buffer_skid_limit_set(switch_device_t device,
                                                 uint32_t buffer_size);

/**
 Set skid buffer size
 @param device device
 @param num_bytes number of bytes
*/
switch_status_t switch_api_buffer_skid_hysteresis_set(switch_device_t device,
                                                      uint32_t num_bytes);

/**
 set buffer pool pfc limit for a icos
 @param device device
 @param pool_handle pool handle
 @param icos ingress cos
 @param num_bytes number of bytes
*/
switch_status_t switch_api_buffer_pool_pfc_limit(switch_device_t device,
                                                 switch_handle_t pool_handle,
                                                 uint8_t icos,
                                                 uint32_t num_bytes);

/**
 Set buffer profile for a queue
 @param device device
 @param queue_handle queue handle
 @param buffer_profile_handle buffer profile handle
*/
switch_status_t switch_api_queue_buffer_profile_set(
    switch_device_t device,
    switch_handle_t queue_handle,
    switch_handle_t buffer_profile_handle);

/**
 Get buffer profile for a queue
 @param device device
 @param queue_handle queue handle
*/
switch_status_t switch_api_queue_buffer_profile_get(
    switch_device_t device,
    switch_handle_t queue_handle,
    switch_handle_t *buffer_profile_handle);

/**
 enable color based drop on a pool
 @param device device
 @param pool_handle pool handle
 @param enable enable/disable
*/
switch_status_t switch_api_buffer_pool_color_drop_enable(
    switch_device_t device, switch_handle_t pool_handle, bool enable);

/**
 buffer pool color limit set
 @param device device
 @param pool_handle pool handle
 @param color packet color
 @param num_bytes number of bytes
*/
switch_status_t switch_api_buffer_pool_color_limit_set(
    switch_device_t device,
    switch_handle_t pool_handle,
    switch_color_t color,
    uint32_t num_bytes);

/**
 pool color hystersis set
 @param device device
 @param color packet color
 @param num_bytes number of bytes
*/
switch_status_t switch_api_buffer_pool_color_hysteresis_set(
    switch_device_t device, switch_color_t color, uint32_t num_bytes);

/**
 switch max ingress pool get
 @param device device
*/
switch_status_t switch_api_max_ingress_pool_get(switch_device_t device,
                                                switch_uint8_t *pool_size);
/**
 switch max egress pool get
 @param device device
*/
switch_status_t switch_api_max_egress_pool_get(switch_device_t device,
                                               switch_uint8_t *pool_size);

/**
 switch total buffer size get
 @param device device
*/
switch_status_t switch_api_total_buffer_size_get(switch_device_t device,
                                                 switch_uint64_t *size);

/**
 Buffer profile parameters set
 @param device device
 @param buffer_profile_handle buffer profile handle
 @param profile_info buffer profile parameters
*/
switch_status_t switch_api_buffer_profile_info_set(
    switch_device_t device,
    switch_handle_t buffer_profile_handle,
    switch_api_buffer_profile_t *profile_info);

/**
 Buffer profile parameters get
 @param device device
 @param buffer_profile_handle buffer profile handle
*/
switch_status_t switch_api_buffer_profile_info_get(
    switch_device_t device,
    switch_handle_t buffer_profile_handle,
    switch_api_buffer_profile_t *profile_info);

/**
  Buffer pool xoff size set
  @param device device
  @param pool_handle buffer pool handle
  @param xoff_size pool xoff size.
*/
switch_status_t switch_api_buffer_pool_xoff_size_set(
    switch_device_t device,
    switch_handle_t pool_handle,
    switch_uint32_t xoff_size);

/**
  Buffer pool xoff size get
  @param pool_handle buffer pool handle
  @param device device
*/
switch_status_t switch_api_buffer_pool_xoff_size_get(
    switch_device_t device,
    switch_handle_t pool_handle,
    switch_uint32_t *xoff_size);

switch_status_t switch_api_buffer_pool_usage_get(
    switch_device_t device,
    switch_handle_t pool_handle,
    switch_uint32_t *curr_occupancy_bytes,
    switch_uint32_t *watermark_bytes);

/** @} */  // end of Buffer API
#ifdef __cplusplus
}
#endif

#endif
