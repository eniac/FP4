/*
Copyright 2013-present Barefoot Networks, Inc.
*/

#include <math.h>
#include "switch_internal.h"
#include <math.h>

#if !defined(BMV2TOFINO) && !defined(BMV2)

switch_status_t switch_pd_ingress_pool_init(
    switch_device_t device, switch_buffer_pd_pool_use_t *pd_pool) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(status);
#ifdef SWITCH_PD

  switch_uint16_t i = 0;

  for (i = 0; i < SWITCH_BUFFER_POOL_INGRESS_MAX; i++) {
    switch (i) {
      case 0:
        pd_pool[i].pool_id = PD_INGRESS_POOL_0;
        break;
      case 1:
        pd_pool[i].pool_id = PD_INGRESS_POOL_1;
        break;
      case 2:
        pd_pool[i].pool_id = PD_INGRESS_POOL_2;
        break;
      case 3:
        pd_pool[i].pool_id = PD_INGRESS_POOL_3;
        break;
      default:
        return SWITCH_STATUS_FAILURE;
    }
  }
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_egress_pool_init(
    switch_device_t device, switch_buffer_pd_pool_use_t *pd_pool) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);
#ifdef SWITCH_PD

  switch_uint16_t i = 0;

  for (i = 0; i < SWITCH_BUFFER_POOL_EGRESS_MAX; i++) {
    switch (i) {
      case 0:
        pd_pool[i].pool_id = PD_EGRESS_POOL_0;
        break;
      case 1:
        pd_pool[i].pool_id = PD_EGRESS_POOL_1;
        break;
      case 2:
        pd_pool[i].pool_id = PD_EGRESS_POOL_2;
        break;
      case 3:
        pd_pool[i].pool_id = PD_EGRESS_POOL_3;
        break;
      default:
        return SWITCH_STATUS_FAILURE;
    }
  }

#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_buffer_bytes_to_cells(
    switch_device_t device,
    switch_uint32_t bytes_threshold,
    switch_uint32_t *cell_threshold) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_uint32_t cell_size = 0;

  UNUSED(device);
  UNUSED(bytes_threshold);
  UNUSED(status);
  UNUSED(pd_status);
  UNUSED(cell_size);

#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE
  pd_status = p4_pd_tm_get_cell_size_in_bytes(device, &cell_size);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    status = switch_pd_status_to_status(pd_status);
    SWITCH_PD_LOG_ERROR("Failed to get cell size in bytes for device %d: %s",
                        device,
                        switch_error_to_string(status));
    return status;
  }
  *cell_threshold = ceil(bytes_threshold / cell_size);
#endif
#endif
  return status;
}

switch_status_t switch_pd_buffer_pool_set(switch_device_t device,
                                          switch_pd_pool_id_t pool_id,
                                          switch_uint32_t pool_size) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_uint32_t max_threshold = 0;

  UNUSED(pool_id);
  UNUSED(pool_size);
  UNUSED(status);
  UNUSED(pd_status);
  UNUSED(max_threshold);

#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE
  status = switch_pd_buffer_bytes_to_cells(device, pool_size, &max_threshold);
  if (status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR("Failed to get cell size for bytes for device %d: %s",
                        device,
                        switch_error_to_string(status));
    return status;
  }
  pd_status = p4_pd_tm_set_app_pool_size(device, pool_id, max_threshold);
  status = switch_pd_status_to_status(pd_status);
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_buffer_pool_color_drop_enable(
    switch_device_t device, switch_pd_pool_id_t pool_id, bool enable) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pool_id);
  UNUSED(enable);
  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE

  if (enable) {
    pd_status = p4_pd_tm_enable_app_pool_color_drop(device, pool_id);
  } else {
    pd_status = p4_pd_tm_disable_app_pool_color_drop(device, pool_id);
  }

#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_buffer_pool_color_limit_set(
    switch_device_t device,
    switch_pd_pool_id_t pool_id,
    switch_color_t color,
    switch_uint32_t num_bytes) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(pool_id);
  UNUSED(color);
  UNUSED(num_bytes);
  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE

  status =
      p4_pd_tm_set_app_pool_color_drop_limit(device, pool_id, color, num_bytes);
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_buffer_pool_color_hysteresis_set(
    switch_device_t device, switch_color_t color, switch_uint32_t num_bytes) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(color);
  UNUSED(num_bytes);
  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE
  status =
      p4_pd_tm_set_app_pool_color_drop_hysteresis(device, color, num_bytes);
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_buffer_skid_limit_set(switch_device_t device,
                                                switch_uint32_t num_bytes) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(num_bytes);
  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE

  status = p4_pd_tm_set_skid_pool_size(device, num_bytes);

#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_buffer_skid_hysteresis_set(
    switch_device_t device, switch_uint32_t num_bytes) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(num_bytes);
  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE

  status = p4_pd_tm_set_skid_pool_hysteresis(device, num_bytes);

#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_buffer_pool_pfc_limit(switch_device_t device,
                                                switch_pd_pool_id_t pool_id,
                                                switch_uint8_t icos,
                                                switch_uint32_t num_bytes) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(num_bytes);
  UNUSED(pool_id);
  UNUSED(icos);
  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE

  status = p4_pd_tm_set_app_pool_pfc_limit(device, pool_id, icos, num_bytes);
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_total_buffer_size_get(switch_device_t device,
                                                switch_uint64_t *size) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE
  uint64_t buff_size = 0;
  status = p4_pd_tm_get_total_buffer_size(device, &buff_size);
  *size = buff_size;

#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */
  return status;
}

switch_status_t switch_pd_buffer_pool_usage_get(
    switch_device_t device,
    switch_pd_pool_id_t pool_id,
    switch_uint32_t *curr_occupancy_bytes,
    switch_uint32_t *watermark_bytes) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
#ifdef SWITCH_PD
  switch_uint32_t cell_size = 0;
  switch_uint32_t co_cell = 0;
  switch_uint32_t wm_cell = 0;

  p4_pd_tm_get_cell_size_in_bytes(device, &cell_size);

  status = p4_pd_tm_pool_usage_get(device, pool_id, &co_cell, &wm_cell);

  *curr_occupancy_bytes = co_cell *cell_size;
  *watermark_bytes = wm_cell *cell_size;

#endif /* SWITCH_PD */
  return status;
}

#else

switch_status_t switch_pd_ingress_pool_init(
    switch_device_t device, switch_buffer_pd_pool_use_t *pd_pool) {
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_pd_egress_pool_init(
    switch_device_t device, switch_buffer_pd_pool_use_t *pd_pool) {
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_pd_buffer_pool_set(switch_device_t device,
                                          switch_pd_pool_id_t pool_id,
                                          switch_uint32_t pool_size) {
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_pd_buffer_pool_color_drop_enable(
    switch_device_t device, switch_pd_pool_id_t pool_id, bool enable) {
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_pd_buffer_pool_color_limit_set(
    switch_device_t device,
    switch_pd_pool_id_t pool_id,
    switch_color_t color,
    switch_uint32_t num_bytes) {
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_pd_buffer_pool_color_hysteresis_set(
    switch_device_t device, switch_color_t color, switch_uint32_t num_bytes) {
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_pd_buffer_skid_limit_set(switch_device_t device,
                                                switch_uint32_t num_bytes) {
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_pd_buffer_skid_hysteresis_set(
    switch_device_t device, switch_uint32_t num_bytes) {
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_pd_buffer_pool_pfc_limit(switch_device_t device,
                                                switch_pd_pool_id_t pool_id,
                                                switch_uint8_t icos,
                                                switch_uint32_t num_bytes) {
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_pd_total_buffer_size_get(switch_device_t device,
                                                switch_uint64_t *size) {
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_pd_buffer_pool_usage_get(
    switch_device_t device,
    switch_pd_pool_id_t pool_id,
    switch_uint32_t *curr_occupancy_bytes,
    switch_uint32_t *watermark_bytes) {
  return SWITCH_STATUS_SUCCESS;
}
#endif /* BMV2 && BMV2TOFINO */
