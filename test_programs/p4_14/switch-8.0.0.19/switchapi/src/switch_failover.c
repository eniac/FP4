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

#include "switch_failover_int.h"
#include "switch_pd_failover.h"
#include "switch_pd_pktgen.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_failover_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  return status;
}

switch_status_t switch_failover_free(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  return status;
}

switch_status_t switch_failover_default_entry_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  status = switch_pd_pktgen_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    return status;
  }
  // LAG failover
  status = switch_pd_lag_failover_pktgen_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    return status;
  }

  status = switch_pd_lag_failover_recirc_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    return status;
  }

  status = switch_pd_lag_failover_lookup_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    return status;
  }

  // ECMP failover
  status = switch_pd_ecmp_failover_pktgen_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    return status;
  }

  status = switch_pd_ecmp_failover_recirc_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    return status;
  }

  status = switch_pd_prepare_for_recirc_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    return status;
  }

  status = switch_pd_ecmp_failover_lookup_default_entry_add(device);

  return status;
}

switch_status_t switch_failover_default_entry_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  return status;
}

switch_status_t switch_api_fast_failover_enable_internal(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_pd_failover_pktgen_enable(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("fast failover enabled failed on device %d : %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_fast_failover_disable_internal(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_pd_failover_pktgen_disable(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("fast failover enabled failed on device %d : %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_fast_failover_enable(switch_device_t device) {
  SWITCH_MT_WRAP(switch_api_fast_failover_enable_internal(device))
}

switch_status_t switch_api_fast_failover_disable(switch_device_t device) {
  SWITCH_MT_WRAP(switch_api_fast_failover_disable_internal(device))
}
