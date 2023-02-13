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

#ifndef __SWITCH_DEVICE_INT_H__
#define __SWITCH_DEVICE_INT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_IFINDEX_SIZE 65536

#define SWITCH_MAX_PORTS 512

#define SWITCH_MAX_RECIRC_PORTS 4

#define SWITCH_MAX_PORT_MTU 15000

typedef switch_status_t (*switch_device_stats_poll_interval_set_fn)(
    switch_device_t device, switch_uint32_t interval);

/** device context information */
typedef struct switch_device_context_s {
  /** p4 table sizes */
  switch_table_t table_info[SWITCH_TABLE_MAX];

  /**
   * handle info array indexed based on handle type
   * which provides the handle allocator
   */
  switch_array_t handle_info_array;

  /**
   * handle array is to allocate and store the
   * contents of every handle's associaated struct.
   */
  switch_array_t handle_array[SWITCH_HANDLE_TYPE_MAX];

  /** device context for every module */
  void *context[SWITCH_API_TYPE_MAX];

  /** flag to indicate if module is initialized */
  bool api_inited[SWITCH_API_TYPE_MAX];

  /** ifindex allocator */
  switch_id_allocator_t *ifindex_allocator;

  /** application device info */
  switch_api_device_info_t device_info;

  /** cpu port number */
  switch_port_t cpu_port;

  /** cpu port handle */
  switch_handle_t cpu_port_handle;

  /** default vrf handle */
  switch_handle_t vrf_handle;

  /** lag hash handle */
  switch_handle_t lag_hash_handle;

  /** ecmp hash handle */
  switch_handle_t ecmp_hash_handle;

  /** maximum pipes */
  switch_uint32_t max_pipes;

  /** refresh interval */
  switch_uint32_t refresh_interval;

  /** stats poll interval */
  switch_device_stats_poll_interval_set_fn stats_poll_interval_fn;

  /** front port list */
  switch_port_t fp_list[SWITCH_MAX_PORTS];

  /** dev port list */
  switch_dev_port_t dp_list[SWITCH_MAX_PORTS];

  /** cpu ethernet dev port */
  switch_dev_port_t eth_cpu_dev_port;

  /** cpu pcie dev port */
  switch_dev_port_t pcie_cpu_dev_port;

  /** recirc port list */
  switch_port_t recirc_port_list[SWITCH_MAX_RECIRC_PORTS];

  /** recirc port dev port list */
  switch_dev_port_t recirc_dev_port_list[SWITCH_MAX_RECIRC_PORTS];

  /** recirc port handles */
  switch_handle_t recirc_port_handles[SWITCH_MAX_RECIRC_PORTS];

  /** max recirc ports */
  switch_uint32_t max_recirc_ports;

  /** device level lock*/
  bf_sys_rmutex_t mtx;

  /** boolean for warm init */
  bool warm_init;

  /** stats timer per device */
  bf_sys_timer_t stats_timer;

  /** device id */
  switch_device_t device_id;

  /** system packet_type ACL handles */
  switch_handle_t acl_pkt_type_handle[SWITCH_PACKET_TYPE_MAX];

  /** system packet_type ACE handles */
  switch_handle_t ace_pkt_type_handle[SWITCH_PACKET_TYPE_MAX];

  /** meter handle for L2 dst miss */
  switch_handle_t meter_pkt_type_handle[SWITCH_PACKET_TYPE_MAX];

  /** system action for L2 dst miss */
  switch_acl_action_t l2_miss_action[SWITCH_PACKET_TYPE_MAX];

  /** cut-through mode */
  bool cut_through_mode;

  /** tunnel dmac index */
  switch_id_t tunnel_dmac_index;

} switch_device_context_t;

#define SWITCH_DEVICE_DEFAULT_MAC(_device, _mac)    \
  _mac.mac_addr[0] = 0x00;                          \
  _mac.mac_addr[1] = 0xBA;                          \
  _mac.mac_addr[2] = 0x7E;                          \
  _mac.mac_addr[3] = 0xF0;                          \
  _mac.mac_addr[4] = switch_config_switch_id_get(); \
  _mac.mac_addr[5] = device;

switch_status_t switch_device_init(switch_device_t device,
                                   switch_size_t *table_sizes);
switch_status_t switch_device_deinit(switch_device_t device);

switch_status_t switch_device_free(switch_device_t device);

switch_status_t switch_device_api_init(switch_device_t device);

switch_status_t switch_device_api_free(switch_device_t device);

switch_status_t switch_device_table_get(switch_device_t device,
                                        switch_table_t **table_info);

switch_status_t switch_device_api_context_get(switch_device_t device,
                                              switch_api_type_t api_type,
                                              void **context);

switch_status_t switch_device_api_context_set(switch_device_t device,
                                              switch_api_type_t api_type,
                                              void *context);

switch_status_t switch_device_context_get(
    switch_device_t device, switch_device_context_t **context_get);

switch_status_t switch_device_ifindex_allocate(switch_device_t device,
                                               switch_ifindex_t *ifindex);

switch_status_t switch_device_ifindex_deallocate(switch_device_t device,
                                                 switch_ifindex_t ifindex);

switch_status_t switch_api_device_vrf_max_get(switch_device_t device,
                                              switch_uint16_t *max_vrf);

switch_status_t switch_device_cpu_pcie_dev_port_set(switch_device_t device,
                                                    switch_dev_port_t dev_port);

switch_status_t switch_device_cpu_eth_dev_port_set(switch_device_t device,
                                                   switch_dev_port_t dev_port);

switch_status_t switch_device_cpu_eth_dev_port_get(switch_device_t device,
                                                   switch_dev_port_t *dev_port);

switch_status_t switch_device_max_pipes_get(switch_device_t device,
                                            switch_int32_t *max_pipes);

switch_status_t switch_device_dev_port_get(switch_device_t device,
                                           switch_port_t port,
                                           switch_dev_port_t *dev_port);
switch_status_t switch_device_front_port_get(switch_device_t device,
                                             switch_dev_port_t dev_port,
                                             switch_port_t *fp_port);

bool switch_device_recirc_port(switch_device_t device, switch_port_t port);

switch_status_t switch_device_recirc_dev_port_get(switch_device_t device,
                                                  switch_port_t port,
                                                  switch_dev_port_t *dev_port);

switch_status_t switch_api_device_api_dump(const switch_device_t device,
                                           const switch_api_type_t api_type,
                                           const void *cli_ctx);

switch_status_t switch_device_active_ports_increment(switch_device_t device);

switch_status_t switch_device_active_ports_decrement(switch_device_t device);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_DEVICE_INT_H__ */
