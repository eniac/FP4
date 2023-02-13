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

#ifndef _switch_ila_h_
#define _switch_ila_h_

#include "switch_handle.h"
#include "switch_base_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct __attribute__((__packed__)) switch_ila_addr_ {
  uint64_t loc;
  uint8_t type : 3;
  uint8_t c : 1;
  uint64_t id : 60;
} switch_ila_addr_t;

/** ILA info */
typedef struct switch_api_ila_info_ {
  switch_handle_t vrf_handle;
  switch_ip_addr_t sir_addr;
} switch_api_ila_info_t;

/**
 Add a mapping to ILA table
 @param device - device
 @param api_ila_info ILA info that contains vrf handle, SIR, ila addr, and
 next_hop handle
*/
switch_status_t switch_api_ila_add(switch_device_t device,
                                   switch_api_ila_info_t *api_ila_info,
                                   switch_ip_addr_t ila_addr,
                                   switch_handle_t nhop_handle);

/**
 Update a mapping from ILA table
 @param device - device
 @param api_ila_info ILA info that contains vrf handle, SIR, ila addr, and
 next_hop handle
*/
switch_status_t switch_api_ila_update(switch_device_t device,
                                      switch_api_ila_info_t *api_ila_info,
                                      switch_ip_addr_t ila_addr,
                                      switch_handle_t nhop_handle);

/**
 Delete a mapping from ILA table
 @param device - device
 @param api_ila_info ILA info that contains vrf handle, SIR, ila addr, and
 next_hop handle
*/
switch_status_t switch_api_ila_delete(switch_device_t device,
                                      switch_api_ila_info_t *api_ila_info);

switch_status_t switch_api_ila_get(switch_device_t device,
                                   switch_api_ila_info_t *api_ila_info,
                                   switch_ip_addr_t *ila_addr,
                                   switch_handle_t *nhop_handle);

#ifdef __cplusplus
}
#endif

#endif /* _switch_ila_h_ */
