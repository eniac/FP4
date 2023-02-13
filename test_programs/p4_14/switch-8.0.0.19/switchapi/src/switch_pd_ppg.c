/*
Copyright 2013-present Barefoot Networks, Inc.
*/

#include "switch_internal.h"
#include "switch_pd.h"

#if !defined(BMV2TOFINO) && !defined(BMV2)

switch_status_t switch_pd_port_drop_limit_set(switch_device_t device,
                                              switch_handle_t port_handle,
                                              uint32_t num_bytes) {
  switch_status_t status = 0;

  UNUSED(device);
  UNUSED(port_handle);
  UNUSED(num_bytes);
  UNUSED(status);
#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE
  status = p4_pd_tm_set_ingress_port_drop_limit(
      device, handle_to_id(port_handle), num_bytes);
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */
  return status;
}

switch_status_t switch_pd_port_drop_hysteresis_set(switch_device_t device,
                                                   switch_handle_t port_handle,
                                                   uint32_t num_bytes) {
  switch_status_t status = 0;
#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE
  status = p4_pd_tm_set_ingress_port_hysteresis(
      device, handle_to_id(port_handle), num_bytes);
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */
  return status;
}

switch_status_t switch_pd_port_pfc_cos_mapping(switch_device_t device,
                                               switch_dev_port_t dev_port,
                                               uint8_t *cos_to_icos) {
  switch_status_t status = 0;
#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE
  status = p4_pd_tm_set_port_pfc_cos_mapping(device, dev_port, cos_to_icos);
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */
  return status;
}

switch_status_t switch_pd_port_flowcontrol_mode_set(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_flowcontrol_type_t flow_control) {
  switch_status_t status = 0;
#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE
  status = p4_pd_tm_set_port_flowcontrol_mode(device, dev_port, flow_control);
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */
  return status;
}

switch_status_t switch_pd_ppg_create(switch_device_t device,
                                     switch_dev_port_t dev_port,
                                     switch_tm_ppg_hdl_t *ppg_handle) {
  switch_status_t status = 0;
#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE
  status = p4_pd_tm_allocate_ppg(device, dev_port, ppg_handle);
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */
  return status;
}

switch_status_t switch_pd_ppg_delete(switch_device_t device,
                                     switch_tm_ppg_hdl_t ppg_handle) {
  switch_status_t status = 0;
#ifdef SWITCH_PD
  SWITCH_FAST_RECONFIG(device)
#ifdef P4_QOS_CLASSIFICATION_ENABLE
  status = p4_pd_tm_free_ppg(device, ppg_handle);
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */
  return status;
}

switch_status_t switch_pd_port_ppg_icos_mapping(
    switch_device_t device,
    switch_tm_ppg_hdl_t tm_ppg_handle,
    uint8_t icos_bmp) {
  switch_status_t status = 0;

#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE

  status = p4_pd_tm_set_ppg_icos_mapping(device, tm_ppg_handle, icos_bmp);
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_ppg_lossless_enable(switch_device_t device,
                                              switch_tm_ppg_hdl_t tm_ppg_handle,
                                              bool enable) {
  switch_status_t status = 0;

#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE

  if (enable) {
    status = p4_pd_tm_enable_lossless_treatment(device, tm_ppg_handle);
  } else {
    status = p4_pd_tm_disable_lossless_treatment(device, tm_ppg_handle);
  }
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_ppg_pool_usage_set(
    switch_device_t device,
    switch_tm_ppg_hdl_t tm_ppg_handle,
    switch_pd_pool_id_t pool_id,
    switch_api_buffer_profile_t *buffer_profile_info,
    bool enable) {
  switch_status_t status = 0;
  switch_uint32_t xoff_threshold = 0;
  switch_uint32_t xon_threshold = 0;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(xoff_threshold);
  UNUSED(xon_threshold);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE
  p4_pd_tm_queue_baf_t dyn_baf = PD_Q_BAF_DISABLE;
  switch_uint32_t buffer_size = 0;
  switch_uint32_t buffer_cells = 0;
  if (enable) {
    SWITCH_ASSERT(buffer_profile_info);

    status = switch_pd_buffer_bytes_to_cells(
        device, buffer_profile_info->xoff_threshold, &xoff_threshold);
    if (status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "Failed to get cell size for xoff threshold for device %d: %s",
          device,
          switch_error_to_string(status));
      return status;
    }

    status = switch_pd_buffer_bytes_to_cells(
        device, buffer_profile_info->xon_threshold, &xon_threshold);
    if (status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "Failed to get cell size for xon threshold for device %d: %s",
          device,
          switch_error_to_string(status));
      return status;
    }
    if (buffer_profile_info->threshold_mode ==
        SWITCH_BUFFER_THRESHOLD_MODE_STATIC) {
      dyn_baf = PD_Q_BAF_DISABLE;
    } else {
      if ((buffer_profile_info->threshold == SWITCH_BUFFER_MAX_THRESHOLD) ||
          (buffer_profile_info->threshold == 0)) {
        dyn_baf = PD_Q_BAF_80_PERCENT;
      } else {
        dyn_baf = (p4_pd_tm_queue_baf_t)(buffer_profile_info->threshold /
                                         DYNAMIC_THRESHOLD_FACTOR);
      }
    }
    buffer_size = buffer_profile_info->buffer_size;
    switch_pd_buffer_bytes_to_cells(device, buffer_size, &buffer_cells);

    status = p4_pd_tm_set_ppg_app_pool_usage(
        device, tm_ppg_handle, pool_id, buffer_cells, dyn_baf, xon_threshold);
    if (status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR("Failed to set PPG pool usage for device %d: %s",
                          device,
                          switch_error_to_string(status));
      return status;
    }
  } else {
    status =
        p4_pd_tm_disable_ppg_app_pool_usage(device, pool_id, tm_ppg_handle);
  }
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_ppg_guaranteed_limit_set(
    switch_device_t device,
    switch_tm_ppg_hdl_t tm_ppg_handle,
    uint32_t num_bytes) {
  switch_status_t status = 0;

#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE
  status =
      p4_pd_tm_set_ppg_guaranteed_min_limit(device, tm_ppg_handle, num_bytes);
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_ppg_skid_limit_set(switch_device_t device,
                                             switch_tm_ppg_hdl_t tm_ppg_handle,
                                             uint32_t num_bytes) {
  switch_status_t status = 0;

#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE
  status = p4_pd_tm_set_ppg_skid_limit(device, tm_ppg_handle, num_bytes);
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_ppg_skid_hysteresis_set(
    switch_device_t device,
    switch_tm_ppg_hdl_t tm_ppg_handle,
    uint32_t num_bytes) {
  switch_status_t status = 0;

#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE
  status = p4_pd_tm_set_guaranteed_min_skid_hysteresis(
      device, tm_ppg_handle, num_bytes);
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_ppg_drop_count_get(switch_device_t device,
                                             switch_tm_ppg_hdl_t tm_ppg_handle,
                                             uint64_t *num_packets) {
  switch_status_t status = 0;

#ifdef SWITCH_PD
  status = p4_pd_tm_ppg_drop_get(device, 0x0, tm_ppg_handle, num_packets);
#endif /* SWITCH_PD */

  return status;
}

#else

switch_status_t switch_pd_port_drop_limit_set(switch_device_t device,
                                              switch_handle_t port_handle,
                                              uint32_t num_bytes) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_port_drop_hysteresis_set(switch_device_t device,
                                                   switch_handle_t port_handle,
                                                   uint32_t num_bytes) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_port_pfc_cos_mapping(switch_device_t device,
                                               switch_dev_port_t dev_port,
                                               uint8_t *cos_to_icos) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_port_flowcontrol_mode_set(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_flowcontrol_type_t flow_control) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_ppg_create(switch_device_t device,
                                     switch_dev_port_t dev_port,
                                     switch_tm_ppg_hdl_t *ppg_handle) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_ppg_delete(switch_device_t device,
                                     switch_tm_ppg_hdl_t ppg_handle) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_port_ppg_icos_mapping(
    switch_device_t device,
    switch_tm_ppg_hdl_t tm_ppg_handle,
    uint8_t icos_bmp) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_ppg_lossless_enable(switch_device_t device,
                                              switch_tm_ppg_hdl_t tm_ppg_handle,
                                              bool enable) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_ppg_pool_usage_set(
    switch_device_t device,
    switch_tm_ppg_hdl_t tm_ppg_handle,
    switch_pd_pool_id_t pool_id,
    switch_api_buffer_profile_t *buffer_profile_info,
    bool enable) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_ppg_guaranteed_limit_set(
    switch_device_t device,
    switch_tm_ppg_hdl_t tm_ppg_handle,
    uint32_t num_bytes) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_ppg_skid_limit_set(switch_device_t device,
                                             switch_tm_ppg_hdl_t tm_ppg_handle,
                                             uint32_t num_bytes) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_ppg_skid_hysteresis_set(
    switch_device_t device,
    switch_tm_ppg_hdl_t tm_ppg_handle,
    uint32_t num_bytes) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_ppg_drop_count_get(switch_device_t device,
                                             switch_tm_ppg_hdl_t tm_ppg_handle,
                                             uint64_t *num_packets) {
  switch_status_t status = 0;
  return status;
}
#endif /* BMV2 && BMV2TOFINO */
