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

#include "switch_internal.h"
#include "switch_pd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static bool switch_api_debug_mode = 0;
static int _api_lib_inited = 0;

bool switch_api_debug_mode_get() { return switch_api_debug_mode; }
void switch_api_debug_mode_set(bool mode) { switch_api_debug_mode = mode; }

switch_status_t switch_api_init(switch_device_t device,
                                unsigned int num_ports,
                                char *cpu_port,
                                bool add_ports) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_config_t api_config;

  if (_api_lib_inited == 0) {
    _api_lib_inited = 1;
    switch_api_debug_mode_set(0);

    SWITCH_MEMSET(&api_config, 0x0, sizeof(api_config));
    api_config.max_devices = 1;
    api_config.add_ports = add_ports;
    api_config.enable_ports = TRUE;
    api_config.default_port_speed = SWITCH_PORT_SPEED_10G;
    api_config.program_smac = TRUE;

    if (!cpu_port) {
      api_config.use_pcie = TRUE;
    } else {
      api_config.use_pcie = FALSE;
      SWITCH_MEMCPY(api_config.cpu_interface, cpu_port, strlen(cpu_port));
    }

    status = switch_config_init(&api_config);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("switch api init failed on device %d",
                       device,
                       switch_error_to_string(status));
    }

    status = switch_api_device_add(device);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("switch api init failed on device %d",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  }

  return status;
}

switch_status_t switch_api_free(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_device_context_t *device_ctx = NULL;
  status = switch_device_context_get(device, &device_ctx);
  device_ctx->warm_init = true;
  if (_api_lib_inited == 1) {
    _api_lib_inited = 0;
    switch_api_device_remove(device);
    switch_device_deinit(device);
  }

  status = switch_config_free();
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("switch api free failed on device %d",
                     device,
                     switch_error_to_string(status));
  }

  return status;
}

#ifdef __cplusplus
}
#endif
