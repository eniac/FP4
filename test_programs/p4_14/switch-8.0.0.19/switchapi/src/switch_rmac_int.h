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

#ifndef __SWITCH_RMAC_INT_H__
#define __SWITCH_RMAC_INT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define switch_rmac_handle_create(_device) \
  switch_handle_create(                    \
      _device, SWITCH_HANDLE_TYPE_RMAC, sizeof(switch_rmac_info_t))

#define switch_rmac_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_RMAC, _handle)

#define switch_rmac_get(_device, _handle, _info) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_RMAC, _handle, (void **)_info)

/** rewrite mac address hash key size */
#define SWITCH_SMAC_HASH_KEY_SIZE sizeof(switch_mac_addr_t)

/** source mac rewrite hash seed */
#define SWITCH_SMAC_REWRITE_HASH_SEED 0x123456

/** tunnel source mac rewrite hash seed */
#define SWITCH_TUNNEL_SMAC_REWRITE_HASH_SEED 0x123456

typedef enum switch_smac_type_s {
  SWITCH_SMAC_TYPE_REWRITE = (1 << 0),
  SWITCH_SMAC_TYPE_TUNNEL_REWRITE = (1 << 1),
  SWITCH_SMAC_TYPE_REWRITE_ALL = 0x3
} switch_smac_type_t;

typedef enum switch_rmac_pd_entry_s {
  SWITCH_RMAC_PD_ENTRY_INNER = (1 << 0),
  SWITCH_RMAC_PD_ENTRY_OUTER = (1 << 1)
} switch_rmac_pd_entry_t;

/** rmac entry */
typedef struct switch_rmac_entry_s {
  /** mac address */
  switch_mac_addr_t mac;

  /** list node */
  switch_node_t node;

  /** outer rmac table hw handle */
  switch_pd_hdl_t outer_rmac_entry;

  /** inner rmac table hw handle */
  switch_pd_hdl_t inner_rmac_entry;

  /** rewrite smac index */
  switch_id_t smac_index;

  /** tunnel rewrite smac index */
  switch_id_t tunnel_smac_index;

  /** hardware flags */
  switch_uint64_t hw_flags;

} switch_rmac_entry_t;

/** rmac group info */
typedef struct switch_rmac_info_s {
  /** list of rmac entries */
  switch_list_t rmac_list;

  /** rmac type - inner/outer */
  switch_rmac_type_t rmac_type;
} switch_rmac_info_t;

typedef enum switch_smac_pd_entry_s {
  SWITCH_SMAC_PD_ENTRY_REWRITE = (1 << 0),
  SWITCH_SMAC_PD_ENTRY_TUNNEL_REWRITE = (1 << 1)
} switch_smac_pd_entry_t;

/** rewrite mac info */
typedef struct switch_smac_entry_s {
  /** mac address */
  switch_mac_addr_t mac;

  /** rewrite smac index */
  switch_id_t smac_index;

  /** smac index ref count */
  switch_uint16_t ref_count;

  /** hashtable node */
  switch_hashnode_t node;

  /** smac hardware handle */
  switch_pd_hdl_t hw_smac_entry;

  /** hardware flags */
  switch_uint64_t hw_flags;

} switch_smac_entry_t;

/** router mac device context */
typedef struct switch_rmac_context_s {
  /** smac index allocator */
  switch_id_allocator_t *smac_allocator;

  /** tunnel smac index allocator */
  switch_id_allocator_t *tunnel_smac_allocator;

  /** smac hashtable */
  switch_hashtable_t smac_hashtable;

  /** tunnel smac hashtable */
  switch_hashtable_t tunnel_smac_hashtable;
} switch_rmac_context_t;

static inline char *switch_smac_type_to_string(switch_smac_type_t smac_type) {
  switch (smac_type) {
    case SWITCH_SMAC_TYPE_REWRITE:
      return "rewrite";
    case SWITCH_SMAC_TYPE_TUNNEL_REWRITE:
      return "tunnel rewrite";
    case SWITCH_SMAC_TYPE_REWRITE_ALL:
      return "rewrite all";
    default:
      return "invalid";
  }
}

static inline char *switch_rmac_type_to_string(switch_rmac_type_t rmac_type) {
  switch (rmac_type) {
    case SWITCH_RMAC_TYPE_OUTER:
      return "outer";
    case SWITCH_RMAC_TYPE_INNER:
      return "inner";
    case SWITCH_RMAC_TYPE_ALL:
      return "all";
    default:
      return "invalid";
  }
}

switch_status_t switch_rmac_init(switch_device_t device);

switch_status_t switch_rmac_free(switch_device_t device);

switch_status_t switch_rmac_default_entries_add(switch_device_t device);

switch_status_t switch_rmac_default_entries_delete(switch_device_t device);

switch_status_t switch_smac_rewrite_index_by_rmac_handle_get(
    switch_device_t device,
    switch_handle_t rmac_handle,
    switch_smac_type_t smac_type,
    switch_id_t *smac_index);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_RMAC_INT_H__ */
