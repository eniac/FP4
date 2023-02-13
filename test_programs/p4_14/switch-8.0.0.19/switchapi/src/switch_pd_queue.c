/*
Copyright 2013-present Barefoot Networks, Inc.
*/

#include "switch_internal.h"
#include "switch_pd.h"
#include <math.h>

#if !defined(BMV2TOFINO) && !defined(BMV2)

switch_status_t switch_pd_queue_pool_usage_set(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_qid_t queue_id,
    switch_pd_pool_id_t pool_id,
    switch_api_buffer_profile_t *buffer_profile_info,
    bool enable) {
  switch_status_t status = 0;

#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE
  p4_pd_tm_queue_baf_t dyn_baf = PD_Q_BAF_DISABLE;
  switch_uint32_t buffer_cells = 0;
  switch_uint32_t default_hysteresis = 0, default_buffer, default_baf,
                  default_pool_id;
  switch_uint32_t threshold = 0;
  switch_uint32_t buffer_size = 0;

  if (enable) {
    if (buffer_profile_info->threshold_mode ==
        SWITCH_BUFFER_THRESHOLD_MODE_DYNAMIC) {
      /*
       * Hardware supports 8 different dynamic thresholds -
       * p4_pd_tm_queue_baf_t.
       * Distributing the thresholds based on the threshold factor(32).
       */
      threshold = (buffer_profile_info->threshold == 0)
                      ? SWITCH_BUFFER_MAX_THRESHOLD
                      : buffer_profile_info->threshold;
      if (threshold == SWITCH_BUFFER_MAX_THRESHOLD) {
        dyn_baf = PD_Q_BAF_80_PERCENT;
      } else {
        dyn_baf = (p4_pd_tm_queue_baf_t)(threshold / DYNAMIC_THRESHOLD_FACTOR);
      }
    }
    buffer_size = buffer_profile_info->buffer_size;
    /*
     * Driver expects the buffer size in cells.
     */
    switch_pd_buffer_bytes_to_cells(device, buffer_size, &buffer_cells);

    /*
     * Get the default hysteresis for buffer pool and set it.
     */
    p4_pd_tm_get_q_app_pool_usage(device,
                                  dev_port,
                                  queue_id,
                                  &default_pool_id,
                                  &default_buffer,
                                  &default_baf,
                                  &default_hysteresis);
    status = p4_pd_tm_set_q_app_pool_usage(device,
                                           dev_port,
                                           queue_id,
                                           pool_id,
                                           buffer_cells,
                                           dyn_baf,
                                           default_hysteresis);
  } else {
    status = p4_pd_tm_disable_q_app_pool_usage(device, dev_port, queue_id);
  }
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_queue_color_drop_enable(switch_device_t device,
                                                  switch_dev_port_t dev_port,
                                                  switch_qid_t queue_id,
                                                  bool enable) {
  switch_status_t status = 0;

#ifdef SWITCH_PD
  if (enable) {
    status = p4_pd_tm_enable_q_color_drop(device, dev_port, queue_id);
  } else {
    status = p4_pd_tm_disable_q_color_drop(device, dev_port, queue_id);
  }
#endif /* SWITCH_PD */
  return status;
}

switch_status_t switch_pd_queue_color_limit_set(switch_device_t device,
                                                switch_dev_port_t dev_port,
                                                switch_qid_t queue_id,
                                                switch_color_t color,
                                                uint32_t limit) {
  switch_status_t status = 0;

#ifdef SWITCH_PD
  status = p4_pd_tm_set_q_color_limit(device, dev_port, queue_id, color, limit);
#endif /* SWITCH_PD */
  return status;
}

switch_status_t switch_pd_queue_color_hysteresis_set(switch_device_t device,
                                                     switch_dev_port_t dev_port,
                                                     switch_qid_t queue_id,
                                                     switch_color_t color,
                                                     uint32_t limit) {
  switch_status_t status = 0;

#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE
  status =
      p4_pd_tm_set_q_color_hysteresis(device, dev_port, queue_id, color, limit);
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */
  return status;
}

switch_status_t switch_pd_queue_pfc_cos_mapping(switch_device_t device,
                                                switch_dev_port_t dev_port,
                                                switch_qid_t queue_id,
                                                uint8_t cos) {
  switch_status_t status = 0;

#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE
  status = p4_pd_tm_set_q_pfc_cos_mapping(device, dev_port, queue_id, cos);
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */
  return status;
}

switch_status_t switch_pd_queue_port_mapping(switch_device_t device,
                                             switch_dev_port_t dev_port,
                                             uint8_t queue_count,
                                             switch_qid_t *queue_mapping) {
  switch_status_t status = 0;

#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE
  status =
      p4_pd_tm_set_port_q_mapping(device, dev_port, queue_count, queue_mapping);
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */
  return status;
}

switch_status_t switch_pd_queue_scheduling_enable(switch_device_t device,
                                                  switch_dev_port_t dev_port,
                                                  switch_qid_t queue_id,
                                                  bool enable) {
  switch_status_t status = 0;

#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE
  if (enable) {
    status = p4_pd_tm_enable_q_sched(device, dev_port, queue_id);
  } else {
    status = p4_pd_tm_disable_q_sched(device, dev_port, queue_id);
  }
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */
  return status;
}

switch_status_t switch_pd_queue_scheduling_strict_priority_set(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_qid_t queue_id,
    uint32_t priority) {
  switch_status_t status = 0;

#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE
  status = p4_pd_tm_set_q_sched_priority(
      device,
      dev_port,
      queue_id,
      priority ? PD_TM_SCH_PRIO_HIGH : PD_TM_SCH_PRIO_0);
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */
  return status;
}

switch_status_t switch_pd_queue_scheduling_remaining_bw_priority_set(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_qid_t queue_id,
    uint32_t priority) {
  switch_status_t status = 0;

#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE
  status = p4_pd_tm_set_q_remaining_bw_sched_priority(
      device, dev_port, queue_id, priority);
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */
  return status;
}

/*
 * SAI's valid range for weight is 1-100 and hardware supports
 * 0-1023 value.
 */
#define SWITCH_PD_WEIGHT(weight) (weight * 100)
switch_status_t switch_pd_queue_scheduling_dwrr_weight_set(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_qid_t queue_id,
    uint16_t weight) {
  switch_status_t status = 0;

#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE
  status = p4_pd_tm_set_q_dwrr_weight(
      device, dev_port, queue_id, SWITCH_PD_WEIGHT(weight));
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */
  return status;
}

uint32_t switch_api_shaping_rate_kbps(uint64_t rate_bps) {
  uint32_t rate_kbps = ceil(rate_bps / 1000);
  return rate_kbps;
}

switch_status_t switch_pd_queue_guaranteed_rate_set(switch_device_t device,
                                                    switch_dev_port_t dev_port,
                                                    switch_qid_t queue_id,
                                                    bool pps,
                                                    uint32_t burst_size,
                                                    uint64_t rate) {
  switch_status_t status = 0;

#ifdef SWITCH_PD
#ifdef P4_QOS_CLASSIFICATION_ENABLE

  uint32_t min_rate = 0;

  if (pps) {
    min_rate = rate;
  } else {
    min_rate = switch_api_shaping_rate_kbps(rate);
  }
  status = p4_pd_tm_set_q_guaranteed_rate(
      device, dev_port, queue_id, pps, burst_size, min_rate);

  if (status != SWITCH_STATUS_SUCCESS) {
    return status;
  }

  if (min_rate) {
    status = p4_pd_tm_q_min_rate_shaper_enable(device, dev_port, queue_id);
  } else {
    status = p4_pd_tm_q_min_rate_shaper_disable(device, dev_port, queue_id);
  }
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#endif /* SWITCH_PD */
  return status;
}

switch_status_t switch_pd_port_shaping_set(switch_device_t device,
                                           switch_dev_port_t dev_port,
                                           bool pps,
                                           uint32_t burst_size,
                                           uint64_t rate) {
  switch_status_t status = 0;
  uint32_t shaping_rate = 0;

#ifdef SWITCH_PD
  if (pps) {
    shaping_rate = rate;
  } else {
    shaping_rate = switch_api_shaping_rate_kbps(rate);
  }
  status = p4_pd_tm_set_port_shaping_rate(
      device, dev_port, pps, burst_size, shaping_rate);

#endif /* SWITCH_PD */
  return status;
}

switch_status_t switch_pd_queue_shaping_set(switch_device_t device,
                                            switch_dev_port_t dev_port,
                                            switch_qid_t queue_id,
                                            bool pps,
                                            uint32_t burst_size,
                                            uint64_t rate) {
  switch_status_t status = 0;
  uint32_t shaping_rate;

#ifdef SWITCH_PD
  if (pps) {
    shaping_rate = rate;
  } else {
    shaping_rate = switch_api_shaping_rate_kbps(rate);
  }
  status = p4_pd_tm_set_q_shaping_rate(
      device, dev_port, queue_id, pps, burst_size, shaping_rate);

  if (status != SWITCH_STATUS_SUCCESS) {
    return status;
  }

  if (shaping_rate) {
    status = p4_pd_tm_q_max_rate_shaper_enable(device, dev_port, queue_id);
  } else {
    status = p4_pd_tm_q_max_rate_shaper_disable(device, dev_port, queue_id);
  }
#endif /* SWITCH_PD */
  return status;
}

switch_status_t switch_pd_dtel_tail_drop_deflection_queue_set(
    switch_device_t device,
    switch_pipe_t pipe_id,
    switch_dev_port_t dev_port,
    switch_qid_t queue_id) {
  switch_status_t status = 0;

#ifdef SWITCH_PD
  status =
      p4_pd_tm_set_negative_mirror_dest(device, pipe_id, dev_port, queue_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mod queue set failed on device %d "
        "TM set deflect on drop on pipe %d devport %d "
        "and queue %d failed:(%s)\n",
        device,
        pipe_id,
        dev_port,
        queue_id,
        switch_error_to_string(status));

    return status;
  }
#endif /* SWITCH_PD */
  return status;
}

switch_status_t switch_pd_queue_drop_count_get(switch_device_t device,
                                               switch_dev_port_t dev_port,
                                               switch_qid_t queue_id,
                                               uint64_t *num_packets) {
  switch_status_t status = 0;
#ifdef SWITCH_PD
  status = p4_pd_tm_q_drop_get(device, 0x0, dev_port, queue_id, num_packets);
#endif /* SWITCH_PD */
  return status;
}

#else

switch_status_t switch_pd_queue_pool_usage_set(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_qid_t queue_id,
    switch_pd_pool_id_t pool_id,
    switch_api_buffer_profile_t *buffer_profile_info,
    bool enable) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_queue_color_drop_enable(switch_device_t device,
                                                  switch_dev_port_t dev_port,
                                                  switch_qid_t queue_id,
                                                  bool enable) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_queue_color_limit_set(switch_device_t device,
                                                switch_dev_port_t dev_port,
                                                switch_qid_t queue_id,
                                                switch_color_t color,
                                                uint32_t limit) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_queue_color_hysteresis_set(switch_device_t device,
                                                     switch_dev_port_t dev_port,
                                                     switch_qid_t queue_id,
                                                     switch_color_t color,
                                                     uint32_t limit) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_queue_pfc_cos_mapping(switch_device_t device,
                                                switch_dev_port_t dev_port,
                                                switch_qid_t queue_id,
                                                uint8_t cos) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_queue_port_mapping(switch_device_t device,
                                             switch_dev_port_t dev_port,
                                             uint8_t queue_count,
                                             uint8_t *queue_mapping) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_queue_scheduling_enable(switch_device_t device,
                                                  switch_dev_port_t dev_port,
                                                  switch_qid_t queue_id,
                                                  bool enable) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_queue_scheduling_strict_priority_set(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_qid_t queue_id,
    uint32_t priority) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_queue_scheduling_remaining_bw_priority_set(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_qid_t queue_id,
    uint32_t priority) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_queue_scheduling_dwrr_weight_set(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_qid_t queue_id,
    uint16_t weight) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_queue_scheduling_guaranteed_shaping_set(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_qid_t queue_id,
    bool pps,
    uint32_t burst_size,
    uint32_t rate) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_port_shaping_set(switch_device_t device,
                                           switch_dev_port_t dev_port,
                                           bool pps,
                                           uint32_t burst_size,
                                           uint64_t rate) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_queue_scheduling_dwrr_shaping_set(
    switch_device_t device,
    switch_dev_port_t dev_port,
    switch_qid_t queue_id,
    bool pps,
    uint32_t burst_size,
    uint32_t rate) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_dtel_tail_drop_deflection_queue_set(
    switch_device_t device,
    switch_pipe_t pipe_id,
    switch_dev_port_t dev_port,
    switch_qid_t queue_id) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_queue_shaping_set(switch_device_t device,
                                            switch_dev_port_t dev_port,
                                            switch_qid_t queue_id,
                                            bool pps,
                                            uint32_t burst_size,
                                            uint64_t rate) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_queue_guaranteed_rate_set(switch_device_t device,
                                                    switch_dev_port_t dev_port,
                                                    switch_qid_t queue_id,
                                                    bool pps,
                                                    uint32_t burst_size,
                                                    uint64_t rate) {
  switch_status_t status = 0;
  return status;
}

switch_status_t switch_pd_queue_drop_count_get(switch_device_t device,
                                               switch_dev_port_t dev_port,
                                               switch_qid_t queue_id,
                                               uint64_t *num_packets) {
  switch_status_t status = 0;
  return status;
}
#endif /* BMV2TOFINO && BMV2 */
