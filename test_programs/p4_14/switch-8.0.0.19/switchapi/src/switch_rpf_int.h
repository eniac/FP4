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

#ifndef __SWITCH_RPF_INT_H__
#define __SWITCH_RPF_INT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** rpf handle wrappers */
#define switch_rpf_group_handle_create(_device) \
  switch_handle_create(                         \
      _device, SWITCH_HANDLE_TYPE_RPF_GROUP, sizeof(switch_rpf_info_t))

#define switch_rpf_group_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_RPF_GROUP, _handle)

#define switch_rpf_group_get(_device, _handle, _info) \
  switch_handle_get(                                  \
      _device, SWITCH_HANDLE_TYPE_RPF_GROUP, _handle, (void **)_info)

/** rpf pd handles */
typedef enum switch_pd_rpf_entry_s {
  SWITCH_RPF_OUTER_PD_ENTRY = (1 << 0),
  SWITCH_RPF_INNER_PD_ENTRY = (1 << 1)
} switch_pd_rpf_entry_t;

/** rpf entry */
typedef struct switch_rpf_entry_s {
  /** list node */
  switch_node_t node;

  /** l3 interface handle */
  switch_handle_t rif_handle;

  /** outer rpf hardware handle */
  switch_pd_hdl_t outer_pd_hdl;

  /** inner rpf hardware handle */
  switch_pd_hdl_t inner_pd_hdl;

  /** hardware flags */
  switch_uint64_t hw_flags;

} switch_rpf_entry_t;

/** rpf info identified by rpf handle */
typedef struct switch_rpf_info_s {
  /** list of rpf interfaces */
  switch_list_t rpf_list;

  /** rpf type - inner/outer */
  switch_rpf_type_t rpf_type;

  /** rpf group id */
  switch_rpf_group_t rpf_group;

  /** pim mode - SM/BIDIR */
  switch_mcast_mode_t pim_mode;

} switch_rpf_info_t;

static inline char *switch_rpf_type_to_string(switch_rpf_type_t rpf_type) {
  switch (rpf_type) {
    case SWITCH_RPF_TYPE_INNER:
      return "inner";
    case SWITCH_RPF_TYPE_OUTER:
      return "outer";
    case SWITCH_RPF_TYPE_ALL:
      return "all";
    default:
      return "none";
  }
}

switch_status_t switch_rpf_init(switch_device_t device);

switch_status_t switch_rpf_free(switch_device_t device);

switch_status_t switch_rpf_default_entries_add(switch_device_t device);

switch_status_t switch_rpf_default_entries_delete(switch_device_t device);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_RPF_INT_H__ */
