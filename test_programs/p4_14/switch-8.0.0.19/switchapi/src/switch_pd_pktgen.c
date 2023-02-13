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
#include "switch_pd_pktgen.h"

#ifdef SWITCH_PD
switch_uint16_t switch_pd_pktgen_app_buffer_offset(
    p4_pd_dev_target_t p4_pd_device, switch_app_id_t app_id) {
  /*
   * The 16kb buffer is evenly divided into 8 apps with 1 buffer per app.
   * perform more complex buffer allocation in future if needed
   */
  SWITCH_ASSERT(app_id < MAX_PKTGEN_APPS);
  return (SWITCH_PKTGEN_BUFFER_SIZE * app_id / MAX_PKTGEN_APPS);
}
#endif /* SWITCH_PD */

switch_status_t switch_pd_pktgen_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

/*
 * enable pktgen functionality on all pipes
 * use same port for pktgen and recirc
 * recirc is enabled on port 68 by default
 */
#ifdef SWITCH_PD
#ifdef P4_PKTGEN_ENABLE

  switch_int32_t max_pipes = SWITCH_MAX_PIPES;
  switch_device_max_pipes_get(device, &max_pipes);

  switch_int32_t index = 0;
  for (index = 0; index < max_pipes; index++) {
    pd_status =
        p4_pd_pktgen_enable(switch_cfg_sess_hdl,
                            device,
                            SWITCH_PD_MAKE_DEV_PORT(index, PKTGEN_PORT));
  }

#endif /* P4_PKTGEN_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_pktgen_app_disable(switch_device_t device,
                                             switch_app_id_t app_id) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(app_id);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_PKTGEN_ENABLE

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  pd_status =
      p4_pd_pktgen_app_disable(switch_cfg_sess_hdl, p4_pd_device, app_id);

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_PKTGEN_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_pktgen_app_enable(switch_device_t device,
                                            switch_app_id_t app_id) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(app_id);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_PKTGEN_ENABLE

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  pd_status =
      p4_pd_pktgen_app_enable(switch_cfg_sess_hdl, p4_pd_device, app_id);

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_PKTGEN_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_pktgen_clear_port_down(switch_device_t device,
                                                 switch_port_t port) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(port);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_PKTGEN_ENABLE

  pd_status = p4_pd_pktgen_clear_port_down(switch_cfg_sess_hdl, device, port);

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_PKTGEN_ENABLE */
#endif /* SWITCH_PD */

  return status;
}
