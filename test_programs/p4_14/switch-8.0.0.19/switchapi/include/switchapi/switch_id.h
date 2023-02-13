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

#ifndef __SWITCH_ID_H__
#define __SWITCH_ID_H__

#include "switch_base_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** ID allocator */
typedef struct switch_id_allocator_t_ {
  switch_uint32_t n_words; /**< number fo 32 bit words in allocator */
  switch_uint32_t *data;   /**< bitmap of allocator */
  bool zero_based;         /**< allocate index from zero if set */
  bool expandable; /**< if set, expand bitmap when needed. zero_based must be
                      FALSE */
} switch_id_allocator_t;

/**
 Create a new allocator, which is expandable
 @param initial_size init size in words (32-bit) for allocator
 @param zero_based allocate index from 0 if set to true
*/
switch_status_t switch_api_id_allocator_new(switch_device_t device,
                                            switch_uint32_t initial_size,
                                            bool zero_based,
                                            switch_id_allocator_t **allocator);

/**
 Delete the allocator
 @param allocator allocator allocated with create
*/
switch_status_t switch_api_id_allocator_destroy(
    switch_device_t device, switch_id_allocator_t *allocator);

/**
 Allocate one id from the allocator
 If bitmap is full and expandable is false, return zero.
 @param allocator allocator created with create
*/
switch_status_t switch_api_id_allocator_allocate(
    switch_device_t device, switch_id_allocator_t *allocator, switch_id_t *id);

/**
 Allocate count consecutive ids from the allocator
 If bitmap is full and expandable is false, return zero.
 @param allocator allocator created with create
 @param count number of consecutive ids to allocate
*/
switch_status_t switch_api_id_allocator_allocate_contiguous(
    switch_device_t device,
    switch_id_allocator_t *allocator,
    switch_uint8_t count,
    switch_id_t *id);

/**
 Free up id in allocator
 @param allocator allocator created with create
 @param id id to free in allocator
*/
switch_status_t switch_api_id_allocator_release(
    switch_device_t device, switch_id_allocator_t *allocator, switch_id_t id);

/**
 Set a bit in allocator
 @param allocator - bitmap allocator reference
 @param id - bit to be set in allocator
*/
switch_status_t switch_api_id_allocator_set(switch_device_t device,
                                            switch_id_allocator_t *allocator,
                                            switch_id_t id);
/**
 Check if a bit is set in allocator
 @param allocator - bitmap allocator reference
 @param id - bit to be checked in allocator
*/
bool switch_api_id_allocator_is_set(switch_device_t device,
                                    switch_id_allocator_t *allocator,
                                    switch_id_t id);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_ID_H__ */
