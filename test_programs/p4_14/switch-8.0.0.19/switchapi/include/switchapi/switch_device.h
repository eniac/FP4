/*
Copyright 2013-present Barefoot Networks, Inc.
*/

#ifndef __SWITCH_DEVICE_H__
#define __SWITCH_DEVICE_H__

#include "switch_base_types.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum switch_device_attrbute_s {
  SWITCH_DEVICE_ATTR_DEFAULT_VRF = (1 << 0),
  SWITCH_DEVICE_ATTR_DEFAULT_VRF_HANDLE = (1 << 1),
  SWITCH_DEVICE_ATTR_DEFAULT_VLAN = (1 << 2),
  SWITCH_DEVICE_ATTR_DEFAULT_VLAN_HANDLE = (1 << 3),
  SWITCH_DEVICE_ATTR_DEFAULT_MAC = (1 << 4),
  SWITCH_DEVICE_ATTR_DEFAULT_MAC_HANDLE = (1 << 5),
  SWITCH_DEVICE_ATTR_MAX_LAG_GROUPS = (1 << 6),
  SWITCH_DEVICE_ATTR_MAX_LAG_MEMBERS = (1 << 7),
  SWITCH_DEVICE_ATTR_LAG_HASH_ALOGORITHM = (1 << 8),
  SWITCH_DEVICE_ATTR_LAG_HASH_FIELDS = (1 << 9),
  SWITCH_DEVICE_ATTR_MAX_ECMP_GROUPS = (1 << 10),
  SWITCH_DEVICE_ATTR_MAX_ECMP_MEMBERS = (1 << 11),
  SWITCH_DEVICE_ATTR_ECMP_HASH_ALOGORITHM = (1 << 12),
  SWITCH_DEVICE_ATTR_ECMP_HASH_FIELDS = (1 << 13),
  SWITCH_DEVICE_ATTR_DEFAULT_LOG_LEVEL = (1 << 14),
  SWITCH_DEVICE_ATTR_INSTALL_DMAC = (1 << 15),
  SWITCH_DEVICE_ATTR_DEFAULT_AGING_TIME = (1 << 16),
  SWITCH_DEVICE_ATTR_PORT_LIST = (1 << 17),
  SWITCH_DEVICE_ATTR_ETH_CPU_PORT = (1 << 18),
  SWITCH_DEVICE_ATTR_PCIE_CPU_PORT = (1 << 19),
  SWITCH_DEVICE_ATTR_MAX_PORTS = (1 << 20),
  SWITCH_DEVICE_ATTR_COUNTER_REFRESH_INTERVAL = (1 << 21),
  SWITCH_DEVICE_ATTR_MAX_VRFS = (1 << 22),
  SWITCH_DEVICE_ATTR_TUNNEL_DMAC = (1 << 23),
  SWITCH_DEVICE_ATTR_ACTIVE_PORTS = (1 << 24),
  SWITCH_DEVICE_ATTR_MAX_PORT_MTU = (1 << 25),
  SWITCH_DEVICE_ATTR_MAC_LEARNING = (1 << 26),

} switch_device_attribute_t;

typedef struct switch_api_device_info_s {
  /** default vrf id */
  switch_vrf_t default_vrf;

  /** vrf handle */
  switch_handle_t vrf_handle;

  /** default vlan id */
  switch_vlan_t default_vlan;

  /** vlan handle */
  switch_handle_t vlan_handle;

  /** switch mac address */
  switch_mac_addr_t mac;

  /** router mac handle */
  switch_handle_t rmac_handle;

  /** maximum lag groups supported */
  switch_uint16_t max_lag_groups;

  /** maximum lag members supported */
  switch_uint16_t max_lag_members;

  /** maximum ecmp groups supported */
  switch_uint16_t max_ecmp_groups;

  /** maximum ecmp members supported */
  switch_uint16_t max_ecmp_members;

  /** lag hashing flags */
  switch_uint32_t lag_hash_flags;

  /** ecmp hashing flags */
  switch_uint32_t ecmp_hash_flags;

  /** default logging level */
  switch_log_level_t default_log_level;

  /** install learnt dmacs */
  bool install_dmac;

  /** maximum vrf supported */
  switch_uint16_t max_vrf;

  /** maximum ports */
  switch_uint32_t max_ports;

  /** list of front port handles */
  switch_handle_list_t port_list;

  /** ethernet cpu port */
  switch_port_t eth_cpu_port;

  /** pcie cpu port */
  switch_port_t pcie_cpu_port;

  /** counter refresh interval */
  switch_uint32_t refresh_interval;

  /** mac aging interval */
  switch_int32_t aging_interval;

  /** tunnel destination mac address */
  switch_mac_addr_t tunnel_dmac;

  /** number of active ports */
  switch_uint16_t num_active_ports;

  /** maximum port mtu */
  switch_uint32_t max_port_mtu;

  /** mac learning flag */
  bool mac_learning;

} switch_api_device_info_t;

switch_status_t switch_api_device_add(switch_device_t device);

switch_status_t switch_api_device_remove(switch_device_t device);

switch_status_t switch_api_device_attribute_set(
    switch_device_t device,
    switch_uint64_t flags,
    switch_api_device_info_t *api_device_info);

switch_status_t switch_api_device_attribute_get(
    switch_device_t device,
    switch_uint64_t flags,
    switch_api_device_info_t *api_device_info);

switch_status_t switch_api_device_default_rmac_handle_get(
    switch_device_t device, switch_handle_t *rmac_handle);

switch_status_t switch_api_device_default_vrf_get(switch_device_t device,
                                                  switch_vrf_t *vrf_id,
                                                  switch_handle_t *vrf_handle);

switch_status_t switch_api_device_default_vlan_get(
    switch_device_t device,
    switch_vlan_t *vlan_id,
    switch_handle_t *vlan_handle);

switch_status_t switch_api_device_cpu_port_get(switch_device_t device,
                                               switch_port_t *port);

switch_status_t switch_api_device_cpu_eth_port_get(switch_device_t device,
                                                   switch_port_t *cpu_port);

switch_status_t switch_api_device_cpu_pcie_port_get(switch_device_t device,
                                                    switch_port_t *cpu_port);

switch_status_t switch_api_device_cpu_port_handle_get(
    switch_device_t device, switch_handle_t *port_handle);

switch_status_t switch_api_device_dump(const switch_device_t device,
                                       const void *cli_ctx);

switch_status_t switch_api_device_counter_refresh_interval_set(
    switch_device_t device, switch_uint32_t refresh_interval);

switch_status_t switch_api_device_counter_refresh_interval_get(
    switch_device_t device, switch_uint32_t *refresh_interval);

switch_status_t switch_api_device_recirc_port_get(switch_device_t device,
                                                  switch_pipe_t pipe_id,
                                                  switch_handle_t *port_handle);

switch_status_t switch_api_device_max_recirc_ports_get(
    switch_device_t device, switch_uint16_t *num_ports);

switch_status_t switch_api_device_dmac_miss_packet_action_set(
    switch_device_t device,
    switch_packet_type_t pkt_type,
    switch_acl_action_t action_type);

switch_status_t switch_api_device_dmac_miss_packet_action_get(
    switch_device_t device,
    switch_packet_type_t pkt_type,
    switch_acl_action_t *action_type);

switch_status_t switch_api_device_mac_aging_interval_set(
    const switch_device_t device, const switch_int32_t aging_time);

switch_status_t switch_api_device_mac_aging_interval_get(
    const switch_device_t device, switch_int32_t *aging_time);

switch_status_t switch_api_device_cut_through_mode_get(switch_device_t device,
                                                       bool *enable);

switch_status_t switch_api_device_cut_through_mode_set(switch_device_t device,
                                                       bool enable);

switch_status_t switch_api_device_tunnel_dmac_get(switch_device_t device,
                                                  switch_mac_addr_t *mac_addr);

switch_status_t switch_api_device_tunnel_dmac_set(switch_device_t device,
                                                  switch_mac_addr_t *mac_addr);

switch_status_t switch_api_device_active_ports_get(
    switch_device_t device, switch_uint16_t *num_active_ports);

switch_status_t switch_api_device_mac_learning_set(switch_device_t device,
                                                   bool enable);

switch_status_t switch_api_device_mac_learning_get(switch_device_t device,
                                                   bool *enable);

/** @} */
#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_DEVICE_H__ */
