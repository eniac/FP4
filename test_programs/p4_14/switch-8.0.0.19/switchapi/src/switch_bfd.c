/*
 * Copyright 2016-present Barefoot Networks, Inc.
 */
#include "switchapi/switch_bfd.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"
#include "switch_pd_bfd.h"

switch_status_t switch_bfd_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_BFD, SWITCH_MAX_BFD_SESSIONS);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("bfd init failed for device %d:",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_bfd_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("bfd init failed for device %d:",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_bfd_free(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_BFD);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("bfd free failed for device %d: %s",
                     device,
                     switch_error_to_string(status));
  }

  return status;
}

switch_status_t switch_api_bfd_session_create_internal(
    switch_device_t device,
    switch_api_bfd_session_info_t *bfd_api_info,
    switch_handle_t *bfd_handle) {
  switch_bfd_info_t *bfd_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_int32_t tx_mult = 0;
  switch_int32_t rx_mult = 0;

  SWITCH_ASSERT(bfd_api_info != NULL);

  // offload a bfd session
  // allocate session_id
  // select pipe(pktgen) = session_id % 4
  // update mau tables
  // enable app
  // Parameter validation
  tx_mult = bfd_api_info->tx_interval / SWITCH_PKTGEN_BFD_TIMER_USEC;
  if (((tx_mult * SWITCH_PKTGEN_BFD_TIMER_USEC) !=
       (int32_t)bfd_api_info->tx_interval) ||
      tx_mult > 255) {
    // negotiated tx interval is not aligned to supported granularity
    // OR too big
    return SWITCH_STATUS_INVALID_PARAMETER;
  }

  // P4 pipeline uses timer <= tx_mult check, so decrement it by one
  if (--tx_mult == 0) {
    tx_mult = 1;  // cannot have zero value for tx_mult
  }

  rx_mult = bfd_api_info->rx_interval / SWITCH_PKTGEN_BFD_TIMER_USEC;
  if (((rx_mult * SWITCH_PKTGEN_BFD_TIMER_USEC) !=
       (int32_t)bfd_api_info->rx_interval) ||
      (rx_mult * bfd_api_info->detect_mult) > 255) {
    // negotiated tx interval is not aligned to supported granularity
    // OR too big
    return SWITCH_STATUS_INVALID_PARAMETER;
  }

  rx_mult = rx_mult * bfd_api_info->detect_mult;
  if (bfd_api_info->dport != UDP_PORT_BFD_1HOP &&
      bfd_api_info->dport != UDP_PORT_BFD_MHOP) {
    // not supported bfd offload
    return SWITCH_STATUS_INVALID_PARAMETER;
  }

  *bfd_handle = switch_bfd_handle_create(device);
  if (*bfd_handle == SWITCH_API_INVALID_HANDLE) {
    return SWITCH_STATUS_FAILURE;
  }

  SWITCH_ASSERT(SWITCH_VRF_HANDLE(bfd_api_info->vrf_hdl) &&
                SWITCH_RMAC_HANDLE(bfd_api_info->rmac_hdl));
  if (!SWITCH_VRF_HANDLE(bfd_api_info->vrf_hdl) ||
      !SWITCH_RMAC_HANDLE(bfd_api_info->rmac_hdl)) {
    return SWITCH_STATUS_INVALID_PARAMETER;
  }

  status = switch_bfd_get(device, *bfd_handle, &bfd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    return status;
  }

  bfd_info->session_id = handle_to_id(*bfd_handle);
  bfd_info->api_info = *bfd_api_info;
  bfd_info->tx_mult = tx_mult;
  bfd_info->rx_mult = rx_mult;

  // program bfd tables in the pipeline
  status = switch_pd_bfd_session_update(device, bfd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    switch_api_bfd_session_delete(device, *bfd_handle);
    *bfd_handle = SWITCH_API_INVALID_HANDLE;
  }

  return status;
}

switch_status_t switch_api_bfd_session_delete_internal(
    switch_device_t device, switch_handle_t bfd_handle) {
  switch_bfd_info_t *bfd_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_BFD_HANDLE(bfd_handle));

  status = switch_bfd_get(device, bfd_handle, &bfd_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    return SWITCH_STATUS_INVALID_HANDLE;
  }

  switch_pd_bfd_session_delete(device, bfd_info);

  status = switch_bfd_handle_delete(device, bfd_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  return status;
}

switch_status_t switch_api_bfd_session_delete(switch_device_t device,
                                              switch_handle_t bfd_session_hdl) {
  SWITCH_MT_WRAP(
      switch_api_bfd_session_delete_internal(device, bfd_session_hdl))
}

switch_status_t switch_api_bfd_session_create(
    switch_device_t device,
    switch_api_bfd_session_info_t *bfd_api_info,
    switch_handle_t *bfd_handle) {
  SWITCH_MT_WRAP(
      switch_api_bfd_session_create_internal(device, bfd_api_info, bfd_handle))
}
