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

#ifndef __SWITCH_CONFIG_INT_H__
#define __SWITCH_CONFIG_INT_H__

#define SWITCH_DEFAULT_VRF 1

#define SWITCH_DEFAULT_VLAN 1

#define SWITCH_MAX_VRF 1024

#define SWITCH_MAX_DEVICE 256

#define SWITCH_DEVICE_INTERNAL 0xBF

#define SWITCH_MAX_LAG_MEMBERS 16

#define SWITCH_MAX_ECMP_MEMBERS 16

#define SWITCH_MAX_PIPES 4

#define SWITCH_COUNTER_REFRESH_INTERVAL_DEFAULT 2

#define SWITCH_CPU_PORT_ETH_DEFAULT 501

#define SWITCH_CPU_PORT_PCIE_DEFAULT 502

#define SWITCH_CPU_DEV_PORT_ETH_DEFAULT 64

#define SWITCH_CPU_DEV_PORT_PCIE_DEFAULT 208

typedef struct switch_config_params_s {
  switch_uint32_t inactivity_timeout;
  switch_uint32_t switch_id;
} switch_config_params_t;

typedef struct switch_config_info_s {
  bool config_inited;

  switch_config_t api_switch_config;

  bool device_inited[SWITCH_MAX_DEVICE];

  switch_device_context_t *device_ctx[SWITCH_MAX_DEVICE];

  switch_config_params_t config_params;

  switch_pktdriver_context_t pktdriver_ctx;

  switch_logging_context_t log_ctx;

  /* global session handle */
  switch_pd_sess_hdl_t sess_hdl;

  /* global multicast session handle */
  switch_pd_sess_hdl_t mc_sess_hdl;

} switch_config_info_t;

extern switch_config_info_t config_info;

#define SWITCH_CONFIG_DEVICE_INITED(_device) config_info.device_inited[_device]

#define switch_api_config_vrf_max_get() 0

#define switch_config_packet_driver_context_get() &config_info.pktdriver_ctx

#define switch_config_logging_context_get() &config_info.log_ctx

#define SWITCH_CONFIG_INITALIZED() config_info.config_inited

#define SWITCH_CONFIG_PORT_ADD() config_info.api_switch_config.add_ports

#define SWITCH_CONFIG_PORT_ENABLE() config_info.api_switch_config.enable_ports

#define SWITCH_CONFIG_PORT_SPEED_DEFAULT \
  config_info.api_switch_config.default_port_speed

#define SWITCH_CONFIG_PCIE() config_info.api_switch_config.use_pcie

#define switch_config_switch_id_get() config_info.config_params.switch_id

#define SWITCH_CONFIG_CPU_ETH_INTF() config_info.api_switch_config.cpu_interface

#define SWITCH_CONFIG_CPU_ETH_INTF_LEN() \
  strlen(config_info.api_switch_config.cpu_interface)

#define SWITCH_CONFIG_SMAC_PROGRAM() config_info.api_switch_config.program_smac

#define switch_cfg_sess_hdl config_info.sess_hdl

#define switch_cfg_mc_sess_hdl config_info.mc_sess_hdl

switch_status_t switch_config_table_sizes_get(switch_device_t device,
                                              switch_size_t *table_sizes);

switch_status_t switch_config_device_context_set(
    switch_device_t device, switch_device_context_t *device_ctx);

switch_status_t switch_config_device_context_get(
    switch_device_t device, switch_device_context_t **device_ctx);

#endif /* __SWITCH_CONFIG_INT_H__ */
