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

#ifndef __SWITCH_VLAN_INT_H__
#define __SWITCH_VLAN_INT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** maximum vlans */
#define SWITCH_MAX_VLANS 4096

/** vlan aging interval */
#define SWITCH_API_VLAN_DEFAULT_AGE_INTERVAL 10000

/** port vlan hashtable seed */
#define SWITCH_PV_HASH_SEED 0x1234abcd

/** port vlan hash key size */
#define SWITCH_PV_HASH_KEY_SIZE sizeof(switch_pv_key_t)

/** vlan handle wrappers */
#define switch_vlan_handle_create(_device) \
  switch_handle_create(                    \
      _device, SWITCH_HANDLE_TYPE_VLAN, sizeof(switch_vlan_info_t))

#define switch_vlan_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_VLAN, _handle)

#define switch_vlan_get(_device, _handle, _info) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_VLAN, _handle, (void **)_info)

/** port vlan hashtable key */
typedef struct switch_pv_key_s {
  /** outer vlan */
  switch_vlan_t outer_vlan;

  /** inner vlan */
  switch_vlan_t inner_vlan;

  /** logical ifindex` */
  switch_port_lag_index_t port_lag_index;
} switch_pv_key_t;

/** port vlan hashtable node */
typedef struct switch_pv_entry_s {
  /**
   * port vlan hashtable key.
   * this should be always on top
   */
  switch_pv_key_t pv_key;

  /** hashtable node */
  switch_hashnode_t node;

  /** interface handle */
  switch_handle_t intf_handle;

  /** bridge domain handle */
  switch_handle_t bd_handle;

  /** bridge domain member handle */
  switch_handle_t member_handle;

} switch_pv_entry_t;

/** vlan device context */
typedef struct switch_vlan_context_s {
  /** vlan id to handle array */
  switch_handle_t vlan_handle_list[SWITCH_MAX_VLANS];

  /** port vlan hashtable */
  switch_hashtable_t pv_hashtable;

} switch_vlan_context_t;

/** vlan info identified by vlan handle */
typedef struct switch_vlan_info_s {
  /** vlan id */
  switch_vlan_t vlan_id;

  /** l3 interface handle */
  switch_handle_t l3_intf_handle;

  /** bridge domain handle */
  switch_handle_t bd_handle;

  /** hostif handle */
  switch_handle_t hostif_handle;

} switch_vlan_info_t;

/** minimum vlan id */
#define SWITCH_VLAN_ID_MIN 1

/** maximum vlan id */
#define SWITCH_VLAN_ID_MAX 4095

#define SWITCH_VLAN_ID_VALID(_vlan_id) \
  ((_vlan_id >= SWITCH_VLAN_ID_MIN) && (_vlan_id <= SWITCH_VLAN_ID_MAX))

switch_status_t switch_vlan_init(switch_device_t device);

switch_status_t switch_vlan_free(switch_device_t device);

switch_status_t switch_vlan_default_entries_add(switch_device_t device);

switch_status_t switch_vlan_default_entries_delete(switch_device_t device);

switch_status_t switch_pv_member_add(switch_device_t device,
                                     switch_handle_t bd_handle,
                                     switch_handle_t intf_handle,
                                     switch_vlan_t outer_vlan,
                                     switch_vlan_t inner_vlan,
                                     switch_uint64_t flags,
                                     switch_handle_t *member_handle);

switch_status_t switch_pv_member_delete(switch_device_t device,
                                        switch_handle_t bd_handle,
                                        switch_handle_t intf_handle,
                                        switch_uint64_t flags);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_VLAN_INT_H__ */
