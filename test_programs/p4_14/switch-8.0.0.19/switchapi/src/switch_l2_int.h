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

#ifndef _SWITCH_L2_INT_H__
#define _SWITCH_L2_INT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Default mac aging time in milliseconds */
#define SWITCH_MAC_TABLE_DEFAULT_AGING_TIME 10000

/** Default max aging time max in milliseconds */
#define SWITCH_MAC_TABLE_MAX_AGING_TIME 918000000 /** 255 Hours */

#define SWITCH_MAC_EVENT_REGISTRATION_MAX 32

/** mac handle wrappers */
#define switch_mac_handle_create(_device) \
  switch_handle_create(                   \
      _device, SWITCH_HANDLE_TYPE_MAC, sizeof(switch_mac_info_t))

#define switch_mac_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_MAC, _handle)

#define switch_mac_get(_device, _handle, _info) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_MAC, _handle, (void **)_info)
/**
 * Default mac buffer learn wait time in milliseconds.
 * when the mac buffer is not full, the hardware waits
 * till query interval before flushing them to software
 */
#define SWITCH_MAC_QUERY_INTERVAL 5000

/** Mac hashtable random seed value */
#define SWITCH_MAC_HASH_SEED 0x123456

/** Mac hashtable entry hash key size */
#define SWITCH_MAC_HASH_KEY_SIZE \
  sizeof(switch_handle_t) + sizeof(switch_mac_addr_t)

typedef enum switch_l2_mac_pd_entry_s {
  SWITCH_L2_PD_SMAC_ENTRY = (1 << 0),
  SWITCH_L2_PD_DMAC_ENTRY = (1 << 1)
} switch_l2_mac_pd_entry_t;

/** App registeration info */
typedef struct switch_mac_event_app_info_s {
  /** valid mac app entry */
  bool valid;

  /** App ID */
  switch_app_id_t app_id;

  /** Mac event flags - learn/age/move */
  switch_uint16_t mac_event_flags;

  /**
   * App data is set during callback registeration.
   * App data is sent during event notifications
   * to the registered application
   */
  void *app_data;

  /** Callback function for mac event notifications */
  switch_mac_notification_fn cb_fn;

} switch_mac_event_app_info_t;

/** L2 device context */
typedef struct switch_l2_context_s {
  /** Mac hashtable */
  switch_hashtable_t mac_hashtable;

  /**
   * Array indexed by dmac hw entry handle
   * When age notifications from hw provides
   * only the hit index, this array can be used
   * to retrive the associated mac entry
   */
  switch_array_t smac_pd_hdl_array;

  /**
   * client data to be registered with learn callback.
   * Registered client data will be received with
   * every learn notification.
   */
  void *learn_client_data;

  /**
   * client data to be registered with aging callback.
   * Registered client data will be received with
   * every aging notification.
   */
  void *aging_client_data;

  /** App list registered for mac learn/age notifications */
  switch_mac_event_app_info_t mac_event_list[SWITCH_MAC_EVENT_REGISTRATION_MAX];

} switch_l2_context_t;

/* mac hashtable key */
typedef struct switch_mac_entry_s {
  /** bridge domain handle */
  switch_handle_t bd_handle;

  /** mac address */
  switch_mac_addr_t mac;

} switch_mac_entry_t;

/** stores mac entry and associated hardware handles */
typedef struct switch_mac_info_s {
  /**
   * Mac entry programmed by the application and
   * acts as the key for mac hashtable. This should
   * be the first entry in this struct for hashing
   */
  switch_mac_entry_t mac_entry;

  /** Mac hashtable node */
  switch_hashnode_t node;

  /** Interface list node */
  switch_node_t intf_node;

  /** Vlan list node */
  switch_node_t vlan_node;

  /** mac action */
  switch_mac_action_t mac_action;

  /** mac entry type - static/dynamic */
  switch_mac_entry_type_t entry_type;

  /** mac handle */
  switch_handle_t mac_handle;

  /** handle - interface/nexthop/multicast */
  switch_handle_t handle;

  /** interface ifindex */
  switch_ifindex_t ifindex;

  /** aging time */
  switch_int32_t aging_interval;

  /** port/lag index */
  switch_port_lag_index_t port_lag_index;

  /** l2 nexthop for tunnel */
  switch_handle_t l2_nhop_handle;

  /** dmac table hw entry handle */
  switch_pd_hdl_t dmac_entry;

  /** smac table hw entry handle */
  switch_pd_hdl_t smac_entry;

  /** hardware flags */
  switch_uint64_t hw_flags;

} switch_mac_info_t;

/** struct received from hw during learn notifications */
typedef struct switch_pd_mac_info_s {
  /** Mac address */
  switch_mac_addr_t mac;

  /** Brdige domain */
  switch_bd_t bd;

  /** Logical interface index */
  switch_ifindex_t ifindex;

} switch_pd_mac_info_t;

static inline char *switch_mac_flush_type_to_string(
    switch_mac_flush_type_t flush_type) {
  switch (flush_type) {
    case SWITCH_MAC_FLUSH_TYPE_NETWORK:
      return "network";
    case SWITCH_MAC_FLUSH_TYPE_INTERFACE:
      return "interface";
    case SWITCH_MAC_FLUSH_TYPE_ALL:
      return "all";
    default:
      return "none";
  }
}

static inline char *switch_mac_entry_type_to_string(
    switch_mac_entry_type_t entry_type) {
  switch (entry_type) {
    case SWITCH_MAC_ENTRY_DYNAMIC:
      return "dynamic";
    case SWITCH_MAC_ENTRY_STATIC:
      return "static";
    default:
      return "none";
  }
}

static inline char *switch_mac_action_to_string(
    switch_mac_action_t mac_action) {
  switch (mac_action) {
    case SWITCH_MAC_ACTION_DROP:
      return "drop";
    case SWITCH_MAC_ACTION_FORWARD:
      return "forward";
    default:
      return "unknown";
  }
}

switch_status_t switch_l2_init(switch_device_t device);

switch_status_t switch_l2_free(switch_device_t device);

switch_status_t switch_l2_default_entries_add(switch_device_t device);

switch_status_t switch_l2_default_entries_delete(switch_device_t device);

switch_status_t switch_mac_table_entry_find(switch_device_t device,
                                            switch_mac_entry_t *mac_entry,
                                            switch_handle_t *mac_handle);

switch_status_t switch_mac_table_entry_delete_by_handle(
    switch_device_t device, switch_handle_t mac_handle);

switch_status_t switch_mac_learn_notify(switch_device_t device,
                                        switch_pd_mac_info_t *pd_mac_entries,
                                        switch_uint16_t num_entries);

switch_status_t switch_mac_aging_notify(switch_device_t device,
                                        switch_pd_hdl_t pd_hdl);

switch_status_t switch_api_mac_handle_get(switch_device_t device,
                                          switch_api_mac_entry_t *api_mac_entry,
                                          switch_handle_t *mac_handle);
switch_status_t switch_mac_entry_aging_hw_update(
    switch_device_t device,
    switch_handle_t mac_handle,
    switch_uint32_t aging_interval);

switch_status_t switch_api_l2_context_dump(const switch_device_t device,
                                           const void *cli_ctx);

switch_status_t switch_l2_hashtable_dump(const switch_device_t device,
                                         const switch_hashtable_type_t type,
                                         void *cli_ctx);
#ifdef __cplusplus
}
#endif

#endif /* _SWITCH_L2_INT_H_ */
