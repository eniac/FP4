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

#ifndef _switch_wred_h_
#define _switch_wred_h_

#include "switch_base_types.h"
#include "switch_handle.h"
#include "switch_meter.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum switch_wred_counter_ {
  SWITCH_WRED_STATS_GREEN_DROPPED,
  SWITCH_WRED_STATS_YELLOW_DROPPED,
  SWITCH_WRED_STATS_RED_DROPPED,
  SWITCH_WRED_STATS_GREEN_ECN_MARKED,
  SWITCH_WRED_STATS_YELLOW_ECN_MARKED,
  SWITCH_WRED_STATS_RED_ECN_MARKED,
  SWITCH_WRED_STATS_MAX
} switch_wred_counter_t;

typedef struct switch_api_wred_info_ {
  bool enable;
  bool ecn_mark;
  switch_size_t min_threshold;
  switch_size_t max_threshold;
  double max_probability;
  double time_constant;
} switch_api_wred_info_t;

typedef struct switch_api_wred_profile_info_ {
  switch_uint32_t min_threshold[SWITCH_COLOR_MAX];
  switch_uint32_t max_threshold[SWITCH_COLOR_MAX];
  bool enable[SWITCH_COLOR_MAX];
  switch_uint32_t probability[SWITCH_COLOR_MAX];
  bool ecn_mark[SWITCH_COLOR_MAX];
} switch_api_wred_profile_info_t;

/**
 @param device - device
*/
switch_status_t switch_api_wred_create(switch_device_t device,
                                       switch_api_wred_info_t *api_wred_info,
                                       switch_handle_t *wred_handle);

switch_status_t switch_api_wred_update(switch_device_t device,
                                       switch_handle_t wred_handle,
                                       switch_api_wred_info_t *api_wred_info);

switch_status_t switch_api_wred_delete(switch_device_t device,
                                       switch_handle_t wred_handle);

switch_status_t switch_api_wred_get(switch_device_t device,
                                    switch_handle_t wred_handle,
                                    switch_api_wred_info_t *api_wred_info);

switch_status_t switch_api_wred_attach(switch_device_t device,
                                       switch_handle_t queue_handle,
                                       switch_meter_counter_t packet_color,
                                       switch_handle_t wred_handle);

switch_status_t switch_api_wred_detach(switch_device_t device,
                                       switch_handle_t queue_handle,
                                       switch_meter_counter_t packet_color);

switch_status_t switch_api_wred_stats_get(switch_device_t device,
                                          switch_handle_t queue_handle,
                                          switch_uint8_t count,
                                          switch_wred_counter_t *counter_ids,
                                          switch_counter_t *counters);

switch_status_t switch_api_wred_stats_clear(switch_device_t device,
                                            switch_handle_t queue_handle,
                                            switch_uint8_t count,
                                            switch_wred_counter_t *counter_ids);

/**
 Wred profile create with green/yellow/red color profiles.
 @param device device
 @param profile_info wred_profile_info
 */
switch_status_t switch_api_wred_profile_create(
    switch_device_t device,
    switch_api_wred_profile_info_t *profile_info,
    switch_handle_t *profile_handle);

/**
 Wred profile delete with green/yellow/red color profiles.
 @param device device
 @param profile_handle wred_profile handle
 */
switch_status_t switch_api_wred_profile_delete(switch_device_t device,
                                               switch_handle_t profile_handle);

/**
 Wred profile info get
 @param device device
 @param profile_handle wred_profile_handle
 */
switch_status_t switch_api_wred_profile_get(
    switch_device_t device,
    switch_handle_t profile_handle,
    switch_api_wred_profile_info_t *profile_info);

/**
 Wred profile info set
 @param device device
 @param profile_handle wred_profile_handle
 */
switch_status_t switch_api_wred_profile_set(
    switch_device_t device,
    switch_handle_t profile_handle,
    switch_color_t color,
    switch_api_wred_profile_info_t *profile_info);

/**
 WRED profile attach to queue
 @param device device
 @param profile_handle WRED profile handle
 @param queue_handle Queue handle
 */
switch_status_t switch_api_queue_wred_profile_set(
    switch_device_t device,
    switch_handle_t profile_handle,
    switch_handle_t queue_handle);

/**
 WRED profile get from queue
 @param device device
 @param queue_handle Queue handle
 */
switch_status_t switch_api_queue_wred_profile_get(
    switch_device_t device,
    switch_handle_t queue_handle,
    switch_handle_t *profile_handle);

#ifdef __cplusplus
}
#endif

#endif /* _switch_wred_h_ */
