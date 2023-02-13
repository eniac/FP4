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

#ifndef __SWITCH_STP_INT_H__
#define __SWITCH_STP_INT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_STP_INVALID_VLAN_HANDLE 0xFF

/** spanning tree handle wrappers */
#define switch_stp_handle_create(_device) \
  switch_handle_create(                   \
      _device, SWITCH_HANDLE_TYPE_STP, sizeof(switch_stp_info_t))

#define switch_stp_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_STP, _handle)

#define switch_stp_get(_device, _handle, _info) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_STP, _handle, (void **)_info)

static inline char *switch_stp_state_to_string(switch_stp_state_t stp_state) {
  switch (stp_state) {
    case SWITCH_PORT_STP_STATE_NONE:
      return "NONE";
    case SWITCH_PORT_STP_STATE_DISABLED:
      return "DISABLED";
    case SWITCH_PORT_STP_STATE_LEARNING:
      return "LEARNING";
    case SWITCH_PORT_STP_STATE_FORWARDING:
      return "FORWARDING";
    case SWITCH_PORT_STP_STATE_BLOCKING:
      return "BLOCKING";
    default:
      return "UNKNOWN";
  }
}

typedef enum switch_stp_intf_pd_entry_s {
  SWITCH_STP_PD_INTF_ENTRY = (1 << 0),
} switch_stp_intf_pd_entry_t;

/** spanning tree port entry */
typedef struct switch_stp_intf_entry_s {
  /** list node */
  switch_node_t node;

  /** interface handle */
  switch_handle_t intf_handle;

  /** spanning tree state */
  switch_stp_state_t stp_state;

  /** hardware handle */
  switch_pd_hdl_t hw_entry;

  /** hardware flags */
  switch_uint64_t hw_flags;

} switch_stp_intf_entry_t;

/** spanning tree vlan entry */
typedef struct switch_stp_network_entry_s {
  /** list node */
  switch_node_t node;

  /** domain handle - vlan/LN */
  switch_handle_t handle;

} switch_stp_network_entry_t;

/** spanning tree identified stp handle */
typedef struct switch_stp_info_s {
  /** list of networks in the stp group */
  switch_list_t network_list;

  /** list of interfaces in the stp group */
  switch_list_t intf_list;

} switch_stp_info_t;

switch_status_t switch_stp_init(switch_device_t device);

switch_status_t switch_stp_free(switch_device_t device);

switch_status_t switch_api_stp_handle_dump(const switch_device_t device,
                                           const switch_handle_t stp_handle,
                                           const void *cli_ctx);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_STP_INT_H__ */
