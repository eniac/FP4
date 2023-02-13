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

#ifndef __SWITCH_RPF_H__
#define __SWITCH_RPF_H__

#include "switch_base_types.h"
#include "switch_handle.h"
#include "switch_status.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum switch_rpf_type_s {
  SWITCH_RPF_TYPE_INNER = (1 << 0),
  SWITCH_RPF_TYPE_OUTER = (1 << 1),
  SWITCH_RPF_TYPE_ALL = 0x3
} switch_rpf_type_t;

switch_status_t switch_api_rpf_group_create(switch_device_t device,
                                            switch_rpf_type_t rpf_type,
                                            switch_mcast_mode_t pim_mode,
                                            switch_handle_t *rpf_group_handle);

switch_status_t switch_api_rpf_group_delete(switch_device_t device,
                                            switch_handle_t rpf_group_handle);

switch_status_t switch_api_rpf_member_add(switch_device_t device,
                                          switch_handle_t rpf_group_handle,
                                          switch_handle_t rif_handle);

switch_status_t switch_api_rpf_member_delete(switch_device_t device,
                                             switch_handle_t rpf_group_handle,
                                             switch_handle_t rif_handle);

switch_status_t switch_api_rpf_members_get(switch_device_t device,
                                           switch_handle_t rpf_group_handle,
                                           switch_size_t *num_entries,
                                           switch_handle_t **rif_handles);

switch_status_t switch_api_rpf_handle_dump(
    const switch_device_t device,
    const switch_handle_t rpf_group_handle,
    const void *cli_ctx);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_RPF_H__ */
