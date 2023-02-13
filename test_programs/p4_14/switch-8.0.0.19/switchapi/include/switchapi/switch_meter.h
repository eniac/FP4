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

#ifndef _switch_meter_h_
#define _switch_meter_h_

#include <stdio.h>

#include "switch_base_types.h"
#include "switch_handle.h"
#include "switch_status.h"
#include "switch_acl.h"

#ifdef __cplusplus
extern "C" {
#endif                    /* __cplusplus */
/** @defgroup Meters API
 *  API functions listed to configure meters
 *  @{
 */  // begin of meters
                          // meters

/** Meter mode */
typedef enum switch_meter_mode_s {
  SWITCH_METER_MODE_NONE,                 /**< none */
  SWITCH_METER_MODE_TWO_RATE_THREE_COLOR, /**< two rate, three color */
  SWITCH_METER_MODE_STORM_CONTROL         /**< storm control */
} switch_meter_mode_t;

/** Meter color mode */
typedef enum switch_meter_color_source_s {
  SWITCH_METER_COLOR_SOURCE_NONE,  /**< none */
  SWITCH_METER_COLOR_SOURCE_BLIND, /**< color blind */
  SWITCH_METER_COLOR_SOURCE_AWARE  /**< color source */
} switch_meter_color_source_t;

/** Meter type */
typedef enum switch_meter_type_s {
  SWITCH_METER_TYPE_NONE = 0,
  SWITCH_METER_TYPE_PACKETS = 1,
  SWITCH_METER_TYPE_BYTES = 2,
} switch_meter_type_t;

/** shaper type */
typedef switch_meter_type_t switch_shaper_type_t;

typedef enum switch_meter_counter_s {
  SWITCH_METER_COUNTER_GREEN,
  SWITCH_METER_COUNTER_YELLOW,
  SWITCH_METER_COUNTER_RED,
  SWITCH_METER_COUNTER_MAX
} switch_meter_counter_t;

/** committed burst size */
typedef uint64_t switch_cbs_t;

/** peak burst size */
typedef uint64_t switch_pbs_t;

/** committed information rate */
typedef uint64_t switch_cir_t;

/** peak information rate */
typedef uint64_t switch_pir_t;

typedef enum switch_meter_attr_s {
  SWITCH_METER_ATTR_CBS = (1 << 0),
  SWITCH_METER_ATTR_PBS = (1 << 1),
  SWITCH_METER_ATTR_CIR = (1 << 2),
  SWITCH_METER_ATTR_PIR = (1 << 3),
} switch_meter_attr_t;

/** Meter attributes */
typedef struct switch_api_meter_ {
  switch_meter_mode_t meter_mode;           /**< meter mode */
  switch_meter_color_source_t color_source; /**< color source */
  switch_meter_type_t meter_type;           /**< meter type */
  switch_cbs_t cbs;                         /**< committed burst size */
  switch_pbs_t pbs;                         /**< peak burst size */
  switch_cir_t cir;                         /**< committed information rate */
  switch_pir_t pir;                         /**< peak information rate */
  switch_acl_action_t action[SWITCH_COLOR_MAX]; /**< packet action */
} switch_api_meter_t;

/**
 Create Meter
 @param device - device
 @param api_meter_info - contains meter attributes
*/
switch_status_t switch_api_meter_create(
    const switch_device_t device,
    const switch_api_meter_t *api_meter_info,
    switch_handle_t *meter_handle);

/**
 Update Meter
 @param device - device
 @param meter_handle - meter handle
 @param api_meter_info - contains meter attributes
*/
switch_status_t switch_api_meter_update(
    const switch_device_t device,
    const switch_handle_t meter_handle,
    const switch_uint64_t flags,
    const switch_api_meter_t *api_meter_info);

/**
 Delete Meter
 @param device- device
 @param meter_handle - meter handle
*/
switch_status_t switch_api_meter_delete(const switch_device_t device,
                                        const switch_handle_t meter_handle);

switch_status_t switch_api_meter_get(const switch_device_t device,
                                     const switch_handle_t meter_handle,
                                     switch_api_meter_t *meter_info);

/**
 Meter counters
 @param device device
 @param meter_handle meter handle
 @param count number of counters
 @param counter_ids meter counter ids
 @param counters counter values
 */
switch_status_t switch_api_meter_counters_get(
    const switch_device_t device,
    const switch_handle_t meter_handle,
    const switch_uint8_t count,
    const switch_meter_counter_t *counter_ids,
    switch_counter_t *counters);

switch_status_t switch_api_meter_handle_dump(const switch_device_t device,
                                             const switch_handle_t meter_handle,
                                             const void *cli_ctx);

/** @} */  // end of meter

#ifdef __cplusplus
}
#endif

#endif /* defined(__switch_api__switch_meter__) */
