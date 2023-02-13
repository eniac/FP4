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

#ifndef _switch_config_h
#define _switch_config_h

#include "switch_base_types.h"
#include "switch_handle.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum switch_switching_mode_ {
  SWITCH_SWITCHING_MODE_CUT_THROUGH = 0x1,
  SWITCH_SWITCHING_MODE_STORE_AND_FORWARD = 0x2
} switch_switching_mode_t;

typedef struct switch_config_s {
  bool use_pcie;

  bool add_ports;

  bool enable_ports;

  bool program_smac;

  switch_port_speed_t default_port_speed;

  switch_char_t cpu_interface[SWITCH_HOSTIF_NAME_SIZE];

  switch_uint16_t max_devices;

  switch_log_level_t default_log_level;

  switch_table_t table_info[SWITCH_TABLE_MAX];

} switch_config_t;

switch_status_t switch_config_init(switch_config_t *switch_config);

switch_status_t switch_config_free();

switch_status_t switch_api_config_dump(const switch_device_t device,
                                       const void *cli_ctx);

switch_status_t switch_api_configuration_get(switch_config_t *config);

switch_status_t switch_api_config_default_vlan_id_get(switch_vlan_t *vlan);

switch_status_t switch_api_config_default_vrf_get(switch_vrf_t *vrf);

switch_status_t switch_api_config_mac_get(switch_mac_addr_t *mac);

switch_status_t switch_api_config_deflect_on_drop_set(bool dod);

switch_status_t switch_api_config_deflect_on_drop_get(bool *dod);

switch_status_t switch_api_flowlet_switching_set(switch_device_t device,
                                                 uint32_t inactivity_timeout);

switch_status_t switch_api_switch_id_set(switch_device_t device,
                                         uint32_t switch_id);

switch_status_t switch_api_batch_begin();

switch_status_t switch_api_batch_end(bool hw_synchronous);

/**
 Set flag to enable/disable SMAC program in hardare by learn notification.
 Usage: Init time API.
 @param device device to use
 @param flag enable/disable flag
*/
switch_status_t switch_api_config_smac_program_set(switch_device_t device,
                                                   bool flag);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _switch_config_h */
