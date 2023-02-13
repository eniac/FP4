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

#ifndef __SWITCH_INTERFACE_INT_H__
#define __SWITCH_INTERFACE_INT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_INTERFACE_MAX 16384

/** interface handle wrappers */
#define switch_interface_handle_create(_device) \
  switch_handle_create(                         \
      _device, SWITCH_HANDLE_TYPE_INTERFACE, sizeof(switch_interface_info_t))

#define switch_interface_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_INTERFACE, _handle)

#define switch_interface_get(_device, _handle, _info) \
  switch_handle_get(                                  \
      _device, SWITCH_HANDLE_TYPE_INTERFACE, _handle, (void **)_info)

#define SWITCH_IFINDEX_SET(_device, _handle, _ifindex, _status)      \
  do {                                                               \
    if (SWITCH_INTERFACE_HANDLE(_handle)) {                          \
      switch_interface_info_t *_intf_info = NULL;                    \
      _status = switch_interface_get(_device, _handle, &_intf_info); \
      SWITCH_ASSERT(_status == SWITCH_STATUS_SUCCESS);               \
      _intf_info->ifindex = _ifindex;                                \
    } else {                                                         \
      SWITCH_ASSERT(0);                                              \
    }                                                                \
  } while (0);

#define SWITCH_IFINDEX_GET(_device, _handle, _ifindex, _status)      \
  do {                                                               \
    _ifindex = 0;                                                    \
    if (SWITCH_INTERFACE_HANDLE(_handle)) {                          \
      switch_interface_info_t *_intf_info = NULL;                    \
      _status = switch_interface_get(_device, _handle, &_intf_info); \
      SWITCH_ASSERT(_status == SWITCH_STATUS_SUCCESS);               \
      _ifindex = _intf_info->ifindex;                                \
    } else {                                                         \
      SWITCH_ASSERT(0);                                              \
    }                                                                \
  } while (0);

#define SWITCH_INTERFACE_ID_GET(_device, _handle, _id, _lag, _status)    \
  do {                                                                   \
    _id = 0;                                                             \
    switch_handle_t _tmp_handle = _handle;                               \
    if (SWITCH_INTERFACE_HANDLE(_tmp_handle)) {                          \
      switch_interface_info_t *_intf_info = NULL;                        \
      _status = switch_interface_get(_device, _tmp_handle, &_intf_info); \
      SWITCH_ASSERT(_status == SWITCH_STATUS_SUCCESS);                   \
      if (_status == SWITCH_STATUS_SUCCESS) {                            \
        _tmp_handle = SWITCH_INTF_ATTR_HANDLE(_intf_info);               \
      }                                                                  \
      _lag = FALSE;                                                      \
    } else {                                                             \
      _status = SWITCH_STATUS_INVALID_HANDLE;                            \
      SWITCH_ASSERT(_status == SWITCH_STATUS_SUCCESS);                   \
    }                                                                    \
                                                                         \
    if (SWITCH_PORT_HANDLE(_tmp_handle)) {                               \
      switch_port_info_t *_port_info = NULL;                             \
      _status = switch_port_get(_device, _tmp_handle, &_port_info);      \
      SWITCH_ASSERT(_status == SWITCH_STATUS_SUCCESS);                   \
      if (_status == SWITCH_STATUS_SUCCESS) {                            \
        _id = _port_info->port;                                          \
      }                                                                  \
    } else if (SWITCH_LAG_HANDLE(_tmp_handle)) {                         \
      switch_lag_info_t *_lag_info = NULL;                               \
      _status = switch_lag_get(_device, _tmp_handle, &_lag_info);        \
      SWITCH_ASSERT(_status == SWITCH_STATUS_SUCCESS);                   \
      if (_status == SWITCH_STATUS_SUCCESS) {                            \
        _lag = TRUE;                                                     \
        _id = handle_to_id(_tmp_handle);                                 \
      }                                                                  \
    } else {                                                             \
      _status = SWITCH_STATUS_INVALID_HANDLE;                            \
      SWITCH_ASSERT(_status == SWITCH_STATUS_SUCCESS);                   \
    }                                                                    \
  } while (0);

static inline char *switch_interface_type_to_string(
    switch_interface_type_t intf_type) {
  switch (intf_type) {
    case SWITCH_INTERFACE_TYPE_NONE:
      return "none";
    case SWITCH_INTERFACE_TYPE_ACCESS:
      return "access";
    case SWITCH_INTERFACE_TYPE_TRUNK:
      return "trunk";
    case SWITCH_INTERFACE_TYPE_PORT_VLAN:
      return "port vlan";
    case SWITCH_INTERFACE_TYPE_TUNNEL:
      return "tunnel";
    default:
      return "none";
  }
}

/** interface ip address information */
typedef struct switch_interface_ip_addr_s {
  /** vrf handle */
  switch_handle_t vrf_handle;

  /** ip address */
  switch_ip_addr_t ip_address;

  /** flag to indicate if ip address is primary */
  bool primary;

  /** list node */
  switch_node_t node;

} switch_interface_ip_addr_t;

/** interface information */
typedef struct switch_interface_info_s {
  /** application interface info */
  switch_api_interface_info_t api_intf_info;

  /** port/lag index */
  switch_port_lag_index_t port_lag_index;

  /** interface ifindex */
  switch_ifindex_t ifindex;

  /** tunnel vni for l3 tunnels */
  switch_vni_t tunnel_vni;

  /** hostif handle */
  switch_handle_t hostif_handle;

  switch_if_label_t acl_label;

  switch_handle_t native_vlan_handle;

  switch_handle_t ln_handle;

  switch_handle_t bd_handle;

  /** tunnel src vtep hardware handle */
  switch_pd_hdl_t src_hw_entry;

  /** tunnel dst vtep hardware handle */
  switch_pd_hdl_t dst_hw_entry;

  /** tunnel src ip rewrite hardware handle */
  switch_pd_hdl_t src_rw_hw_entry;

  /** tunnel dst ip rewrite hardware handle */
  switch_pd_hdl_t dst_rw_hw_entry;

  /** tunnel direction */
  switch_direction_t direction;

  /** mac array */
  switch_array_t mac_array;

  switch_pd_hdl_t tunnel_hw_entry[3];

  switch_id_t tunnel_sip_index;

  switch_id_t tunnel_dip_index;

  switch_id_t tunnel_dmac_index;

  switch_pd_hdl_t tunnel_rewrite_pd_hdl;

} switch_interface_info_t;

#define TUNNEL_MGID_NHOP_HANDLE(ptinfo) ptinfo->mgid_info.nhop_handle

#define TUNNEL_MGID_ROUTE_HANDLE(ptinfo) ptinfo->mgid_info.route_handle

#define TUNNEL_MGID_TUNNEL_HANDLE(ptinfo) ptinfo->mgid_info.tunnel_encap_handle

#define TUNNEL_MIRROR_LIST(ptinfo) ptinfo->PJLarr_mirrors

#define TUNNEL_NUM_MIRRORS(ptinfo) ptinfo->num_mirrors

#define SET_TUNNEL_MGID_STATE(ptinfo, state_fn) \
  ptinfo->mgid_info.mgid_state = state_fn

#define SEND_TUNNEL_MGID_EVENT(ptinfo, dev, event, event_arg) \
  (ptinfo)->mgid_info.mgid_state(dev, (ptinfo), event, event_arg)

typedef struct switch_interface_context_s {
  /** ifindex array */
  switch_array_t ifindex_array;

} switch_interface_context_t;

#define SWITCH_INTF_TYPE(info) info->api_intf_info.type

#define SWITCH_INTF_ATTR_HANDLE(info) info->api_intf_info.handle

#define SWITCH_INTF_ATTR_VLAN_ID(info) info->api_intf_info.vlan

#define SWITCH_INTERFACE_TUNNEL(info) \
  (info->api_intf_info.type == SWITCH_INTERFACE_TYPE_TUNNEL)

#define SWITCH_INTF_NATIVE_VLAN_HANDLE(info) \
  info->api_intf_info.native_vlan_handle

#define SWITCH_INTERFACE_ACCESS(info) \
  (info->api_intf_info.type == SWITCH_INTERFACE_TYPE_ACCESS)

#define SWITCH_INTERFACE_TRUNK(info) \
  (info->api_intf_info.type == SWITCH_INTERFACE_TYPE_TRUNK)

switch_status_t switch_interface_init(switch_device_t device);

switch_status_t switch_interface_free(switch_device_t device);

switch_status_t switch_interface_default_entries_add(switch_device_t device);

switch_status_t switch_interface_default_entries_delete(switch_device_t device);

switch_status_t switch_interface_handle_get(switch_device_t device,
                                            switch_ifindex_t ifindex,
                                            switch_handle_t *intf_handle);

switch_status_t switch_interface_port_get(switch_device_t device,
                                          switch_handle_t handle,
                                          switch_handle_t *port_handle);

switch_status_t switch_interface_array_insert(switch_device_t device,
                                              switch_handle_t handle,
                                              switch_handle_t intf_handle);

switch_status_t switch_interface_array_delete(switch_device_t device,
                                              switch_handle_t handle,
                                              switch_handle_t intf_handle);

switch_status_t switch_interface_vlan_id_get(switch_device_t device,
                                             switch_handle_t bd_handle,
                                             switch_handle_t intf_handle,
                                             switch_vlan_t *outer_vlan,
                                             switch_vlan_t *inner_vlan);

switch_status_t switch_interface_array_get(switch_device_t device,
                                           switch_handle_t handle,
                                           switch_array_t **array);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_INTERFACE_INT_H__ */
