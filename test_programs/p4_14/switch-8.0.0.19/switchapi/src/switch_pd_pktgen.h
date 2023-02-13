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

#ifndef _switch_pd_pktgen_h_
#define _switch_pd_pktgen_h_

#include "p4_pktgen.h"

#define SWITCH_PKTGEN_BUFFER_SIZE (16 * 1024)
#define SWITCH_PKTGEN_MIN_PKT_SIZE 64  // Must generate at least 64-byte packet
#define MAX_PKTGEN_APPS 8
#define PKTGEN_PORT 68  // could be 17 ??

#define SWITCH_PD_PKTGEN_RECIRC_PORT(_p) \
  SWITCH_PD_MAKE_DEV_PORT(_p, PKTGEN_PORT)

#define SWITCH_PD_MAKE_DEV_PORT(_pipe, _port) ((_pipe << 7) | _port)

#ifdef SWITCH_PD
switch_uint16_t switch_pd_pktgen_app_buffer_offset(
    p4_pd_dev_target_t p4_pd_device, switch_app_id_t app_id);
#endif /* SWITCH_PD */

switch_status_t switch_pd_pktgen_init(switch_device_t device);

switch_status_t switch_pd_pktgen_app_disable(switch_device_t device,
                                             switch_app_id_t app_id);

switch_status_t switch_pd_pktgen_app_enable(switch_device_t device,
                                            switch_app_id_t app_id);

switch_status_t switch_pd_pktgen_clear_port_down(switch_device_t device,
                                                 switch_port_t port);

#endif /* _SWITCH_PD_PKTGEN_H_ */
