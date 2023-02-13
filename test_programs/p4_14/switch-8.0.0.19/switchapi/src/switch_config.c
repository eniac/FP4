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

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_config_info_t config_info;

switch_status_t switch_config_init(switch_config_t *switch_config) {
  switch_config_params_t *config_params = NULL;
  switch_device_t device = SWITCH_DEVICE_INTERNAL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  if (config_info.config_inited) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    SWITCH_LOG_ERROR("config init failed : %s", switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&config_info, 0x0, sizeof(config_info));

  config_info.api_switch_config.max_devices = SWITCH_MAX_DEVICE;
  config_info.api_switch_config.add_ports = FALSE;
  config_info.api_switch_config.default_port_speed = SWITCH_PORT_SPEED_10G;
  config_info.api_switch_config.program_smac = TRUE;
  config_info.api_switch_config.default_log_level = SWITCH_LOG_LEVEL_ERROR;
  SWITCH_MEMCPY(config_info.api_switch_config.cpu_interface,
                SWITCH_CPU_ETH_INTF_DEFAULT,
                SWITCH_CPU_ETH_INTF_DEFAULT_LEN);

  if (switch_config) {
    SWITCH_ASSERT(switch_config->max_devices < SWITCH_MAX_DEVICE);
    if (switch_config->max_devices) {
      config_info.api_switch_config.max_devices = switch_config->max_devices;
    }

    if (!switch_config->use_pcie) {
      SWITCH_MEMCPY(config_info.api_switch_config.cpu_interface,
                    switch_config->cpu_interface,
                    strlen(switch_config->cpu_interface));
    }

    if (switch_config->add_ports) {
      config_info.api_switch_config.add_ports = switch_config->add_ports;
      config_info.api_switch_config.default_port_speed =
          switch_config->default_port_speed;
    }

    config_info.api_switch_config.enable_ports = switch_config->enable_ports;
    config_info.api_switch_config.use_pcie = switch_config->use_pcie;
    config_info.api_switch_config.program_smac = switch_config->program_smac;
  }

  SWITCH_ASSERT(config_info.api_switch_config.max_devices != 0);

  config_info.config_inited = TRUE;

  switch_log_init(config_info.api_switch_config.default_log_level);

  config_params = &config_info.config_params;
  // TODO: Move this to device init
  status = switch_pd_switch_config_params_update(0x0, config_params);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_CRITICAL("config init failed on device %d: %s",
                        device,
                        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_LOG_EXIT();

  return status;

cleanup:
  SWITCH_MEMSET(&config_info, 0x0, sizeof(config_info));
  return status;
}

switch_status_t switch_config_free(void) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  if (!config_info.config_inited) {
    return status;
  }

  config_info.config_inited = FALSE;

  SWITCH_MEMSET(&config_info, 0x0, sizeof(config_info));

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_config_device_context_set(
    switch_device_t device, switch_device_context_t *device_ctx) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (device_ctx && config_info.device_inited[device]) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    SWITCH_LOG_ERROR("config free failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (device_ctx) {
    config_info.device_ctx[device] = device_ctx;
    config_info.device_inited[device] = TRUE;
  } else {
    config_info.device_ctx[device] = NULL;
    config_info.device_inited[device] = FALSE;
  }

  return status;
}

switch_status_t switch_config_device_context_get(
    switch_device_t device, switch_device_context_t **device_ctx) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!config_info.device_inited[device]) {
    status = SWITCH_STATUS_UNINITIALIZED;
    SWITCH_LOG_ERROR("config free failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *device_ctx = config_info.device_ctx[device];

  return status;
}

switch_status_t switch_config_table_sizes_get(switch_device_t device,
                                              switch_size_t *table_sizes) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_table_default_sizes_get(table_sizes);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("config table sizes get failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_flowlet_switching_set_internal(
    switch_device_t device, switch_uint32_t inactivity_timeout) {
  switch_config_params_t *config_params = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  config_params = &config_info.config_params;

  config_params->inactivity_timeout = inactivity_timeout;

  status = switch_pd_switch_config_params_update(device, config_params);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("config flowlet switching set failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_switch_id_set_internal(switch_device_t device,
                                                  switch_uint32_t switch_id) {
  switch_config_params_t *config_params = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  config_params = &config_info.config_params;

  config_params->switch_id = switch_id;

  status = switch_pd_switch_config_params_update(device, config_params);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("config switch id set failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_config_smac_program_set_internal(
    switch_device_t device, bool flag) {
  if (config_info.api_switch_config.program_smac != flag) {
    config_info.api_switch_config.program_smac = flag;
  }
  return SWITCH_STATUS_SUCCESS;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_switch_id_set(switch_device_t device,
                                         uint32_t switch_id) {
  SWITCH_MT_WRAP(switch_api_switch_id_set_internal(device, switch_id))
}

switch_status_t switch_api_flowlet_switching_set(switch_device_t device,
                                                 uint32_t inactivity_timeout) {
  SWITCH_MT_WRAP(
      switch_api_flowlet_switching_set_internal(device, inactivity_timeout))
}

switch_status_t switch_api_batch_begin() { return switch_pd_batch_begin(); }

switch_status_t switch_api_batch_end(bool hw_synchronous) {
  return switch_pd_batch_end(hw_synchronous);
}

switch_status_t switch_api_config_smac_program_set(switch_device_t device,
                                                   bool flag) {
  SWITCH_MT_WRAP(switch_api_config_smac_program_set_internal(device, flag))
}
