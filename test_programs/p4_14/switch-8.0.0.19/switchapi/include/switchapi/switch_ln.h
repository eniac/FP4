/*
Copyright 2013-present Barefoot Networks, Inc.
*/

#ifndef _switch_ln_h_
#define _switch_ln_h_

#include "switch_base_types.h"
#include "switch_handle.h"
#include "switch_status.h"
#include "switch_bd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_API_MAX_VLANS 4096

typedef enum switch_ln_attribute_s {
  SWITCH_LN_ATTR_LEARNING_ENABLED = (1 << 0),
  SWITCH_LN_ATTR_IGMP_SNOOPING_ENABLED = (1 << 1),
  SWITCH_LN_ATTR_MLD_SNOOPING_ENABLED = (1 << 2),
  SWITCH_LN_ATTR_AGING_INTERVAL = (1 << 3),
  SWITCH_LN_ATTR_STP_HANDLE = (1 << 4)
} switch_ln_attribute_t;

typedef struct switch_api_ln_info_s {
  bool learning_enabled;
  bool igmp_snooping_enabled;
  bool mld_snooping_enabled;
  switch_uint32_t aging_interval;
  switch_handle_t stp_handle;
} switch_api_ln_info_t;

/**
 Create a Logical network
 @param device -  device to be programmed
 @param ln_info - Logical network information
*/
switch_status_t switch_api_logical_network_create(const switch_device_t device,
                                                  switch_handle_t *ln_handle);

/**
 Update a Logical network
 @param device -  device to be programmed
 @param network_handle handle of logical network
 @param ln_info - Logical network information
*/
switch_status_t switch_api_logical_network_update(
    const switch_device_t device,
    const switch_handle_t ln_handle,
    const switch_uint64_t flags,
    const switch_api_ln_info_t *ln_info);

/**
 Delete a Logical network
 @param device -  device to be programmed
 @param network_handle handle of logical network
*/
switch_status_t switch_api_logical_network_delete(
    const switch_device_t device, const switch_handle_t ln_handle);

switch_status_t switch_api_logical_network_attribute_get(
    const switch_device_t device,
    const switch_handle_t ln_handle,
    const switch_uint64_t flags,
    switch_api_ln_info_t *api_ln_info);

switch_status_t switch_api_logical_network_attribute_set(
    const switch_device_t device,
    const switch_handle_t ln_handle,
    const switch_uint64_t flags,
    const switch_api_ln_info_t *api_ln_info);

switch_status_t switch_api_logical_network_learning_set(
    const switch_device_t device,
    const switch_handle_t ln_handle,
    const bool enable);

switch_status_t switch_api_logical_network_learning_get(
    const switch_device_t device,
    const switch_handle_t ln_handle,
    bool *enable);
/**
 Enable vlan statistics
 @param device device to be programmed
 @param vlan_handle Vlan handle that identifies vlan uniquely
 */
switch_status_t switch_api_logical_network_stats_enable(
    const switch_device_t device, const switch_handle_t ln_handle);

/**
 Enable vlan statistics
 @param device device to be programmed
 @param vlan_handle Vlan handle that identifies vlan uniquely
 */
switch_status_t switch_api_logical_network_stats_disable(
    const switch_device_t device, const switch_handle_t ln_handle);

/**
 Get vlan statistics
 @param vlan_handle Vlan handle that identifies vlan uniquely
 @param count number of counter ids
 @param counter_ids list of counter ids
 @param counters counter values to be returned
 */
switch_status_t switch_api_logical_network_stats_get(
    const switch_device_t device,
    const switch_handle_t ln_handle,
    const switch_uint8_t num_entries,
    const switch_bd_counter_id_t *counter_ids,
    switch_counter_t *counters);

/**
 Add member to logical network
 @param device device
 @param network_handle Logical network handle
 @param interface_handle Handle of access port ot Tunnel interface
*/
switch_status_t switch_api_logical_network_member_add(
    const switch_device_t device,
    const switch_handle_t ln_handle,
    const switch_handle_t intf_handle);

/**
 Delete member from logical network
 @param device device
 @param network_handle Logical network handle
 @param interface_handle Handle of access port ot Tunnel interface
*/
switch_status_t switch_api_logical_network_member_remove(
    const switch_device_t device,
    const switch_handle_t ln_handle,
    const switch_handle_t intf_handle);

/**
  Get bd value of an ln handle
  @param - device - device
  @param - ln_handle - ln handle that identifies logical network uniquely
  @param - bd value - Return bd value which is derived via the bd handle
*/
switch_status_t switch_api_logical_network_bd_get(switch_device_t device,
                                                  switch_handle_t ln_handle,
                                                  switch_uint32_t *bd);

switch_status_t switch_api_logical_network_stats_get(
    const switch_device_t device,
    const switch_handle_t ln_handle,
    const switch_uint8_t num_entries,
    const switch_bd_counter_id_t *counter_ids,
    switch_counter_t *counters);

switch_status_t switch_api_logical_network_stats_clear(
    const switch_device_t device, const switch_handle_t ln_handle);

/** @} */  // end of Logical Network

#ifdef __cplusplus
}
#endif

#endif
