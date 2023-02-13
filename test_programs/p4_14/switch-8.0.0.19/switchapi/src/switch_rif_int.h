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

#ifndef __SWITCH_RIF_INT_H__
#define __SWITCH_RIF_INT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_RIF_MAX 16384

typedef struct switch_rif_info_s {
  /** application interface info */
  switch_api_rif_info_t api_rif_info;

  /** bridge domain handle */
  switch_handle_t bd_handle;

  /** host interface handle */
  switch_handle_t hostif_handle;

  /** list of ip addresses */
  switch_list_t ip_list;

} switch_rif_info_t;

static inline char *switch_rif_type_to_string(switch_rif_type_t rif_type) {
  switch (rif_type) {
    case SWITCH_RIF_TYPE_VLAN:
      return "vlan";
    case SWITCH_RIF_TYPE_LN:
      return "ln";
    case SWITCH_RIF_TYPE_INTF:
      return "intf";
    default:
      return "unknown";
  }
}

#define switch_rif_handle_create(_device) \
  switch_handle_create(                   \
      _device, SWITCH_HANDLE_TYPE_RIF, sizeof(switch_rif_info_t))

#define switch_rif_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_RIF, _handle)

#define switch_rif_get(_device, _handle, _info) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_RIF, _handle, (void **)_info)

#define SWITCH_RIF_TYPE(_rif_info) _rif_info->api_rif_info.rif_type

switch_status_t switch_rif_rewrite_smac_index_get(switch_device_t device,
                                                  switch_handle_t rif_handle,
                                                  switch_id_t *smac_index);

switch_status_t switch_rif_attr_handle_get(switch_device_t device,
                                           switch_handle_t rif_handle,
                                           switch_handle_t *handle);

switch_status_t switch_api_rif_handle_dump(const switch_device_t device,
                                           const switch_handle_t rif_handle,
                                           const void *cli_ctx);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_RIF_INT_H__ */
