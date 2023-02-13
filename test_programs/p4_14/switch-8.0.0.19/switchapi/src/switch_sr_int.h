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

#ifndef __SWITCH_SR_INT_H__
#define __SWITCH_SR_INT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_sr_endpoint_add(
    switch_device_t device, switch_interface_ip_addr_t *ip_addr_info);

switch_status_t switch_sr_endpoint_delete(
    switch_device_t device, switch_interface_ip_addr_t *ip_addr_info);

typedef struct switch_sr_endpoint_s {
  switch_node_t node;
  switch_interface_ip_addr_t ip_addr_info;
  switch_pd_hdl_t entry_hdl;
} switch_sr_endpoint_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SWITCH_SR_INT_H__ */
