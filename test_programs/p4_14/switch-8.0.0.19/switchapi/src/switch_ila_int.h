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

#ifndef _switch_ila_int_h_
#define _switch_ila_int_h_

#include "switchapi/switch_base_types.h"
#include "switchapi/switch_handle.h"
#include "switchapi/switch_ila.h"
#include "switch_pd_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_ILA_HASH_KEY_SIZE sizeof(switch_api_ila_info_t)

#define SWITCH_ILA_HASH_TABLE_SIZE 1024

#define SWITCH_ILA_HASH_SEED 0x12345678

#define SWITCH_ILA_SIR(info) info->sir.ip.v6addr

typedef struct switch_ila_info_ {
  switch_api_ila_info_t api_ila_info;
  switch_handle_t nhop_handle;
  switch_ip_addr_t ila_addr;
  bool ecmp;
  switch_hashnode_t node;
  switch_pd_hdl_t hw_entry;
} switch_ila_info_t;

switch_status_t switch_ila_init(switch_device_t device);

switch_status_t switch_ila_free(switch_device_t device);

#ifdef __cplusplus
}
#endif

#endif /* _switch_ila_int_h_ */
