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

#include "switchapi/switch_pktgen.h"

#include "switch_internal.h"
#include "switch_pd.h"
#include "switch_lag_int.h"
#include "switch_nhop_int.h"
#include "switch_pd_pktgen.h"
#include "switch_pd_failover.h"
#include "p4_pktgen.h"

#ifdef __cplusplus
extern "C" {
#endif /* cplusplus */

tommy_list lag_failover_info;
tommy_list ecmp_failover_info;

switch_status_t switch_pd_failover_pktgen_enable(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(pd_status);

  UNUSED(device);

#ifdef SWITCH_PD
#ifdef P4_FAST_FAILOVER_ENABLE

  pd_status = switch_pd_pktgen_app_enable(device, P4_PKTGEN_APP_LAG_FAILOVER);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }
  pd_status = switch_pd_pktgen_app_enable(device, P4_PKTGEN_APP_ECMP_FAILOVER);

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_FAST_FAILOVER_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "failover pktgen enable success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "failover pktgen enable failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_failover_pktgen_disable(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(pd_status);

  UNUSED(device);

#ifdef SWITCH_PD
#ifdef P4_FAST_FAILOVER_ENABLE

  pd_status = switch_pd_pktgen_app_disable(device, P4_PKTGEN_APP_LAG_FAILOVER);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }
  pd_status = switch_pd_pktgen_app_disable(device, P4_PKTGEN_APP_ECMP_FAILOVER);

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_FAST_FAILOVER_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "failover pktgen disable success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "failover pktgen disable failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_lag_action_profile_set_fallback_member(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(pd_status);

  UNUSED(device);

#ifdef SWITCH_PD
#ifdef P4_FAST_FAILOVER_ENABLE

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_set_lag_port_action_spec_t action_spec;
  p4_pd_mbr_hdl_t mbr_hdl;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&action_spec, 0, sizeof(p4_pd_dc_set_lag_port_action_spec_t));

  // Always recirculate to pipe 0
  action_spec.action_port = SWITCH_PD_PKTGEN_RECIRC_PORT(0);
  action_spec.action_fallback_check = 1;

  pd_status = p4_pd_dc_lag_action_profile_add_member_with_set_lag_port(
      switch_cfg_sess_hdl, p4_pd_device, &action_spec, &mbr_hdl);
  if (pd_status != SWITCH_STATUS_SUCCESS) {
    goto cleanup;
  }

  pd_status =
      p4_pd_dc_lag_action_profile_set_dynamic_action_selection_fallback_member(
          switch_cfg_sess_hdl, p4_pd_device, mbr_hdl);

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* FAST_FAILOVER_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_lag_failover_pktgen_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_FAST_FAILOVER_ENABLE

  p4_pd_dev_target_t p4_pd_device;
  struct p4_pd_pktgen_app_cfg lag_failover_app_config;
  switch_pktgen_ext_header_t *ext_header;
  uint8_t *pkt_buffer;
  uint32_t buffer_len = sizeof(switch_pktgen_ext_header_t);
  uint16_t pkt_offset = switch_pd_pktgen_app_buffer_offset(
      p4_pd_device, P4_PKTGEN_APP_LAG_FAILOVER);

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  /* Pktgen must generate at least a 64-byte packets including 6 bytes header
   * and 4 bytes CRC.
   */
  if (buffer_len < SWITCH_PKTGEN_MIN_PKT_SIZE - 4 - 6) {
    buffer_len = SWITCH_PKTGEN_MIN_PKT_SIZE;
  }

  // Configure lag failover app.
  lag_failover_app_config.trigger_type = PD_PKTGEN_TRIGGER_PORT_DOWN;
  lag_failover_app_config.batch_count = 0;
  lag_failover_app_config.packets_per_batch = MAX_PORT_INSTANCES;
  lag_failover_app_config.pattern_value = 0;
  lag_failover_app_config.pattern_mask = 0;
  lag_failover_app_config.timer_nanosec = 0;
  lag_failover_app_config.ibg = 0;
  lag_failover_app_config.ibg_jitter = 0;
  lag_failover_app_config.ipg = 10;
  lag_failover_app_config.ipg_jitter = 0;
  lag_failover_app_config.source_port = 0;
  lag_failover_app_config.increment_source_port = 0;
  lag_failover_app_config.pkt_buffer_offset = pkt_offset;
  lag_failover_app_config.length = buffer_len;

  pd_status = p4_pd_pktgen_cfg_app(switch_cfg_sess_hdl,
                                   p4_pd_device,
                                   P4_PKTGEN_APP_LAG_FAILOVER,
                                   lag_failover_app_config);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "pktgen app configuration failed "
        "for app %d on device %d : %s (pd: 0x%x)\n",
        P4_PKTGEN_APP_LAG_FAILOVER,
        device,
        switch_error_to_string(status),
        pd_status);
    goto cleanup;
  }
  pkt_buffer = SWITCH_MALLOC(device, buffer_len, 1);
  SWITCH_MEMSET(pkt_buffer, 0, buffer_len);

  ext_header = (switch_pktgen_ext_header_t *)pkt_buffer;
  ext_header->ether_type = htons(ETHERTYPE_BF_PKTGEN);

  pd_status = p4_pd_pktgen_write_pkt_buffer(
      switch_cfg_sess_hdl, p4_pd_device, pkt_offset, buffer_len, pkt_buffer);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  for (uint32_t port = 0; port < SWITCH_API_MAX_PORTS / SWITCH_MAX_PIPES;
       port++) {
    pd_status = p4_pd_pktgen_clear_port_down(switch_cfg_sess_hdl, device, port);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_DEBUG(
          "pktgen clear port down failed "
          "for port %d on device %d : %s (pd: 0x%x)\n",
          port,
          device,
          switch_error_to_string(status),
          pd_status);
      goto cleanup;
    }
  }
  p4_pd_complete_operations(switch_cfg_sess_hdl);

  // Enable LAG failover app on the pktgen_pipe
  pd_status = p4_pd_pktgen_app_enable(
      switch_cfg_sess_hdl, p4_pd_device, P4_PKTGEN_APP_LAG_FAILOVER);

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_FAST_FAILOVER_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "lag failover pktgen initialization success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "lag failover pktgen initialization failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_lag_failover_recirc_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_FAST_FAILOVER_ENABLE

  p4_pd_dc_recirc_failover_pkt_action_spec_t action_spec;
  p4_pd_dc_lag_failover_recirc_match_spec_t match_spec;
  p4_pd_entry_hdl_t entry_hdl;
  p4_pd_dev_target_t p4_pd_device;
  p4_pd_tbl_prop_value_t prop_val;
  p4_pd_tbl_prop_args_t prop_arg;
  int max_pipes = SWITCH_MAX_PIPES;
  switch_device_max_pipes_get(device, &max_pipes);

  // set up asymmetric table - per pipe programming
  prop_val.scope = PD_ENTRY_SCOPE_SINGLE_PIPELINE;
  prop_arg.value = 0;
  pd_status = p4_pd_dc_lag_failover_recirc_set_property(
      switch_cfg_sess_hdl, device, PD_TABLE_ENTRY_SCOPE, prop_val, prop_arg);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  SWITCH_MEMSET(
      &match_spec, 0, sizeof(p4_pd_dc_lag_failover_recirc_match_spec_t));
  SWITCH_MEMSET(
      &action_spec, 0, sizeof(p4_pd_dc_recirc_failover_pkt_action_spec_t));

  for (int pipe = 0; pipe < max_pipes; pipe++) {
    p4_pd_device.device_id = device;
    p4_pd_device.dev_pipe_id = pipe;
    match_spec.pktgen_port_down_pipe_id = (pipe + 1) % max_pipes;
    /* Drop the packet if this is the last pipeline */
    pd_status = p4_pd_dc_lag_failover_recirc_table_add_with_drop_failover_pkt(
        switch_cfg_sess_hdl, p4_pd_device, &match_spec, &entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      goto cleanup;
    }
    /* Recirc the port down packet to the next pipeline */
    action_spec.action_recirc_port =
        SWITCH_PD_PKTGEN_RECIRC_PORT((pipe + 1) % max_pipes);
    pd_status =
        p4_pd_dc_lag_failover_recirc_set_default_action_recirc_failover_pkt(
            switch_cfg_sess_hdl, p4_pd_device, &action_spec, &entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      goto cleanup;
    }
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_FAST_FAILOVER_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_lag_failover_lookup_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_FAST_FAILOVER_ENABLE

  p4_pd_entry_hdl_t entry_hdl;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  pd_status = p4_pd_dc_lag_failover_lookup_set_default_action_drop_failover_pkt(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_FAST_FAILOVER_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "lag failover lookup default entry add success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "lag failover lookup default entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_lag_failover_lookup_entry_add(
    switch_pd_target_t p4_pd_device,
    unsigned int port,
    unsigned int instance_id,
    int index,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_FAST_FAILOVER_ENABLE

  p4_pd_dc_lag_failover_lookup_match_spec_t match_spec;
  p4_pd_dc_set_lag_failover_index_action_spec_t action_spec;

  SWITCH_MEMSET(
      &match_spec, 0, sizeof(p4_pd_dc_lag_failover_lookup_match_spec_t));
  SWITCH_MEMSET(
      &action_spec, 0, sizeof(p4_pd_dc_set_lag_failover_index_action_spec_t));

  match_spec.pktgen_port_down_port_num = port;
  match_spec.pktgen_port_down_packet_id = instance_id;

  action_spec.action_index = index;
  pd_status =
      p4_pd_dc_lag_failover_lookup_table_add_with_set_lag_failover_index(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_FAST_FAILOVER_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "lag failover lookup entry add success "
        "on device %d\n",
        p4_pd_device.device_id);
  } else {
    SWITCH_PD_LOG_ERROR(
        "lag failover lookup entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        p4_pd_device.device_id,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_lag_failover_lookup_modify_entry(
    switch_pd_target_t p4_pd_device, int index, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_FAST_FAILOVER_ENABLE

  p4_pd_dc_set_lag_failover_index_action_spec_t action_spec;

  SWITCH_MEMSET(
      &action_spec, 0, sizeof(p4_pd_dc_set_lag_failover_index_action_spec_t));

  action_spec.action_index = index;
  pd_status =
      p4_pd_dc_lag_failover_lookup_table_modify_with_set_lag_failover_index(
          switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl, &action_spec);

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_FAST_FAILOVER_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "lag failover lookup entry update success "
        "on device %d 0x%lx\n",
        p4_pd_device.device_id,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "lag failover lookup entry update failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl %lx\n",
        p4_pd_device.device_id,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }

  return status;
}

#ifdef SWITCH_PD
#ifdef P4_FAST_FAILOVER_ENABLE
p4_pd_status_t switch_pd_lag_group_callback(p4_pd_sess_hdl_t sess_hdl,
                                            p4_pd_dev_target_t dev_target,
                                            void *cookie,
                                            p4_pd_grp_hdl_t grp_hdl,
                                            p4_pd_mbr_hdl_t mbr_hdl,
                                            int table_index,
                                            bool is_add) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

  switch_pd_failover_member_t *failover_member = NULL, *member = NULL;
  switch_handle_t lag_handle;
  switch_lag_info_t *lag_info = NULL;
  switch_lag_member_t *lag_member = NULL;
  tommy_node *node = NULL, *delete_node = NULL;
  switch_port_t port = 0;
  unsigned int count = 0;

  // Disable pktgen
  pd_status = switch_pd_failover_pktgen_disable(dev_target.device_id);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  // Find port index
  lag_handle = (switch_handle_t)cookie;
  if (!SWITCH_LAG_HANDLE(lag_handle)) {
    return SWITCH_STATUS_INVALID_HANDLE;
  }

  status = switch_lag_get(dev_target.device_id, lag_handle, &lag_info);
  if (!lag_info) {
    return SWITCH_STATUS_INVALID_HANDLE;
  }

  FOR_EACH_IN_LIST(lag_info->members, node) {
    lag_member = (switch_lag_member_t *)node->data;
    if (lag_member->mbr_hdl == mbr_hdl) {
      break;
    }
  }
  FOR_EACH_IN_LIST_END();

  if (!node) {
    return SWITCH_STATUS_PD_FAILURE;
  }

  switch_port_info_t *port_info = NULL;
  status = switch_port_get(
      dev_target.device_id, lag_member->port_handle, &port_info);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  port = port_info->dev_port;

  // Count the number of port instances
  node = tommy_list_head(&(lag_failover_info));
  while (node) {
    failover_member = (switch_pd_failover_member_t *)node->data;
    if (failover_member->u.port == port) {
      count++;
      if (failover_member->index == table_index) {
        delete_node = node;
      }
    }
    node = node->next;
  }
  // Update the failover lookup table
  if (is_add) {
    // New member added to the selection table
    failover_member =
        SWITCH_MALLOC(device, sizeof(switch_pd_failover_member_t), 1);
    failover_member->u.port = port;
    failover_member->instance_id = count;
    failover_member->index = table_index;
    tommy_list_insert_head(
        &lag_failover_info, &(failover_member->node), failover_member);
    pd_status = switch_pd_lag_failover_lookup_entry_add(
        dev_target, port, count, table_index, &(failover_member->entry_hdl));
  } else {
    // Member removed from the selection table
    if (!delete_node) {
      return SWITCH_STATUS_ITEM_NOT_FOUND;
    }
    failover_member = (switch_pd_failover_member_t *)delete_node->data;
    if (failover_member->instance_id < count - 1) {
      // Removing entry which does not have highest instance_id. Swap the
      // instance_id with the highest instance_id.
      node = tommy_list_head(&(lag_failover_info));
      while (node) {
        member = (switch_pd_failover_member_t *)node->data;
        if (member->u.port == port && member->instance_id == count - 1) {
          member->instance_id = failover_member->instance_id;
          break;
        }
        node = node->next;
      }
      // Update the ecmp_failover_lookup table
      pd_status = p4_pd_dc_lag_failover_lookup_table_delete(
          switch_cfg_sess_hdl, dev_target.device_id, member->entry_hdl);
      member->entry_hdl = failover_member->entry_hdl;
      member->instance_id = failover_member->instance_id;
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        goto cleanup;
      }
      pd_status = switch_pd_lag_failover_lookup_modify_entry(
          dev_target, member->index, failover_member->entry_hdl);
    } else {
      // Removing entry with highest instance_id, so just delete it.
      pd_status =
          p4_pd_dc_lag_failover_lookup_table_delete(switch_cfg_sess_hdl,
                                                    dev_target.device_id,
                                                    failover_member->entry_hdl);
    }
    failover_member =
        tommy_list_remove_existing(&(lag_failover_info), delete_node);
    SWITCH_FREE(dev_target.device_id, failover_member);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  // Enable pktgen
  pd_status = switch_pd_failover_pktgen_enable(dev_target.device_id);

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

  return status;
}

#endif /* P4_FAST_FAILOVER_ENABLE */
#endif /* SWITCH_PD */

switch_status_t switch_pd_lag_group_register_callback(switch_device_t device,
                                                      void *cookie) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(pd_status);

  UNUSED(device);
  UNUSED(cookie);

#ifdef SWITCH_PD
#ifdef P4_FAST_FAILOVER_ENABLE

  pd_status = p4_pd_dc_lag_action_profile_register_callback(
      switch_cfg_sess_hdl, device, switch_pd_lag_group_callback, cookie);
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_FAST_FAILOVER_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "lag group register callback success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "lag group register callback failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_ecmp_failover_pktgen_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_FAST_FAILOVER_ENABLE

  p4_pd_dev_target_t p4_pd_device;
  struct p4_pd_pktgen_app_cfg ecmp_failover_app_config;
  switch_pktgen_ext_header_t *ext_header;
  uint8_t *pkt_buffer;
  uint32_t buffer_len = sizeof(switch_pktgen_ext_header_t);
  int max_pipes = SWITCH_MAX_PIPES;
  switch_device_max_pipes_get(device, &max_pipes);
  uint16_t pkt_offset = switch_pd_pktgen_app_buffer_offset(
      p4_pd_device, P4_PKTGEN_APP_ECMP_FAILOVER);

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  /* Pktgen must generate at least a 64-byte packets including 6 bytes header
   * and 4 bytes CRC.
   */
  if (buffer_len < SWITCH_PKTGEN_MIN_PKT_SIZE - 4 - 6) {
    buffer_len = SWITCH_PKTGEN_MIN_PKT_SIZE;
  }

  // Configure lag failover app.
  ecmp_failover_app_config.trigger_type = PD_PKTGEN_TRIGGER_RECIRC_PATTERN;
  ecmp_failover_app_config.batch_count = 0;
  ecmp_failover_app_config.packets_per_batch = MAX_NHOP_INSTANCES;
  ecmp_failover_app_config.pattern_value = ECMP_FAILOVER_RECIRC_PATTERN_VALUE;
  ecmp_failover_app_config.pattern_mask = ECMP_FAILOVER_RECIRC_PATTERN_MASK;
  ecmp_failover_app_config.timer_nanosec = 0;
  ecmp_failover_app_config.ibg = 0;
  ecmp_failover_app_config.ibg_jitter = 0;
  ecmp_failover_app_config.ipg = 10;
  ecmp_failover_app_config.ipg_jitter = 0;
  ecmp_failover_app_config.source_port = 0;
  ecmp_failover_app_config.increment_source_port = 0;
  ecmp_failover_app_config.pkt_buffer_offset = pkt_offset;
  ecmp_failover_app_config.length = buffer_len;

  pd_status = p4_pd_pktgen_cfg_app(switch_cfg_sess_hdl,
                                   p4_pd_device,
                                   P4_PKTGEN_APP_ECMP_FAILOVER,
                                   ecmp_failover_app_config);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "pktgen app configuration failed "
        "for app %d on device %d : %s (pd: 0x%x)\n",
        P4_PKTGEN_APP_ECMP_FAILOVER,
        device,
        switch_error_to_string(status),
        pd_status);
    goto cleanup;
  }

  pkt_buffer = SWITCH_MALLOC(device, buffer_len, 1);
  SWITCH_MEMSET(pkt_buffer, 0, buffer_len);

  ext_header = (switch_pktgen_ext_header_t *)pkt_buffer;
  ext_header->ether_type = htons(ETHERTYPE_BF_PKTGEN);

  pd_status = p4_pd_pktgen_write_pkt_buffer(
      switch_cfg_sess_hdl, p4_pd_device, pkt_offset, buffer_len, pkt_buffer);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  for (int pipe = 0; pipe < max_pipes; pipe++) {
    pd_status = p4_pd_pktgen_enable_recirc_pattern_matching(
        switch_cfg_sess_hdl, device, SWITCH_PD_PKTGEN_RECIRC_PORT(pipe));
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      SWITCH_PD_LOG_DEBUG(
          "pktgen recirc pattern matching failed "
          "on pipe %d on device %d : %s (pd: 0x%x)\n",
          pipe,
          device,
          switch_error_to_string(status),
          pd_status);
      goto cleanup;
    }
  }

  p4_pd_complete_operations(switch_cfg_sess_hdl);

  // Enable ECMP failover app on the pktgen_pipe
  pd_status = p4_pd_pktgen_app_enable(
      switch_cfg_sess_hdl, p4_pd_device, P4_PKTGEN_APP_ECMP_FAILOVER);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_FAST_FAILOVER_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ecmp failover pktgen initialization success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ecmp failover pktgen initialization failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_prepare_for_recirc_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_FAST_FAILOVER_ENABLE

  p4_pd_entry_hdl_t entry_hdl;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  pd_status = p4_pd_dc_prepare_for_recirc_set_default_action_prepare_for_recirc(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_FAST_FAILOVER_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_ecmp_failover_recirc_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_FAST_FAILOVER_ENABLE

  p4_pd_dc_recirc_failover_pkt_action_spec_t action_spec;
  p4_pd_entry_hdl_t entry_hdl;
  p4_pd_dev_target_t p4_pd_device;
  p4_pd_tbl_prop_value_t prop_val;
  p4_pd_tbl_prop_args_t prop_arg;
  int max_pipes = SWITCH_MAX_PIPES;
  switch_device_max_pipes_get(device, &max_pipes);

  // set up asymmetric table - per pipe programming
  prop_val.scope = PD_ENTRY_SCOPE_SINGLE_PIPELINE;
  prop_arg.value = 0;
  pd_status = p4_pd_dc_ecmp_failover_recirc_set_property(
      switch_cfg_sess_hdl, device, PD_TABLE_ENTRY_SCOPE, prop_val, prop_arg);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  SWITCH_MEMSET(
      &action_spec, 0, sizeof(p4_pd_dc_recirc_failover_pkt_action_spec_t));
  p4_pd_device.device_id = device;

  /* Recirc the nhop down packet to the next pipeline */
  for (int pipe = 0; pipe < max_pipes - 1; pipe++) {
    p4_pd_device.dev_pipe_id = pipe;
    action_spec.action_recirc_port =
        SWITCH_PD_PKTGEN_RECIRC_PORT((pipe + 1) % max_pipes);
    pd_status =
        p4_pd_dc_ecmp_failover_recirc_set_default_action_recirc_failover_pkt(
            switch_cfg_sess_hdl, p4_pd_device, &action_spec, &entry_hdl);
    if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
      goto cleanup;
    }
  }

  /* Drop the packet if this is the last pipeline */
  p4_pd_device.dev_pipe_id = max_pipes - 1;
  pd_status =
      p4_pd_dc_ecmp_failover_recirc_set_default_action_drop_failover_pkt(
          switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_FAST_FAILOVER_ENABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_ecmp_failover_lookup_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_FAST_FAILOVER_ENABLE

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  pd_status =
      p4_pd_dc_ecmp_failover_lookup_set_default_action_drop_failover_pkt(
          switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_FAST_FAILOVER_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ecmp failover lookup default entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ecmp failover lookup default entry add failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl %lx\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }

  return status;
}

switch_status_t switch_pd_ecmp_failover_lookup_entry_add(
    switch_pd_target_t p4_pd_device,
    uint16_t nhop_index,
    unsigned int instance_id,
    int index,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_FAST_FAILOVER_ENABLE

  p4_pd_dc_ecmp_failover_lookup_match_spec_t match_spec;
  p4_pd_dc_set_ecmp_failover_index_action_spec_t action_spec;

  SWITCH_MEMSET(
      &match_spec, 0, sizeof(p4_pd_dc_ecmp_failover_lookup_match_spec_t));
  SWITCH_MEMSET(
      &action_spec, 0, sizeof(p4_pd_dc_set_ecmp_failover_index_action_spec_t));

  match_spec.pktgen_recirc_packet_id = instance_id;
  match_spec.pktgen_recirc_key = nhop_index;

  action_spec.action_index = index;
  pd_status =
      p4_pd_dc_ecmp_failover_lookup_table_add_with_set_ecmp_failover_index(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          &action_spec,
          entry_hdl);

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_FAST_FAILOVER_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ecmp failover lookup entry add success "
        "on device %d 0x%lx\n",
        p4_pd_device.device_id,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ecmp failover lookup entry add failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl %lx\n",
        p4_pd_device.device_id,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }

  return status;
}

switch_status_t switch_pd_ecmp_failover_lookup_modify_entry(
    switch_pd_target_t p4_pd_device, int index, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifdef P4_FAST_FAILOVER_ENABLE

  p4_pd_dc_set_ecmp_failover_index_action_spec_t action_spec;

  SWITCH_MEMSET(
      &action_spec, 0, sizeof(p4_pd_dc_set_ecmp_failover_index_action_spec_t));

  action_spec.action_index = index;
  pd_status =
      p4_pd_dc_ecmp_failover_lookup_table_modify_with_set_ecmp_failover_index(
          switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl, &action_spec);

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_FAST_FAILOVER_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ecmp failover lookup entry modify success "
        "on device %d 0x%lx\n",
        p4_pd_device.device_id,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ecmp failover lookup entry modify failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl %lx\n",
        p4_pd_device.device_id,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }

  return status;
}

#ifdef SWITCH_PD
#ifdef P4_FAST_FAILOVER_ENABLE
p4_pd_status_t switch_pd_ecmp_group_callback(p4_pd_sess_hdl_t sess_hdl,
                                             p4_pd_dev_target_t dev_target,
                                             void *cookie,
                                             p4_pd_grp_hdl_t grp_hdl,
                                             p4_pd_mbr_hdl_t mbr_hdl,
                                             int table_index,
                                             bool is_add) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

  switch_pd_failover_member_t *failover_member = NULL, *member = NULL;
  switch_handle_t ecmp_handle;
  switch_ecmp_info_t *ecmp_info = NULL;
  switch_mpath_info_t *mpath_info = NULL;
  switch_ecmp_member_t *ecmp_member = NULL;
  tommy_node *node = NULL, *delete_node = NULL;
  uint16_t nhop_index;
  unsigned int count = 0;

  // Disable pktgen
  pd_status = switch_pd_failover_pktgen_disable(dev_target.device_id);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  // Find nhop index
  ecmp_handle = (switch_handle_t)cookie;
  if (!SWITCH_ECMP_HANDLE(ecmp_handle)) {
    return SWITCH_STATUS_INVALID_HANDLE;
  }
  status = switch_ecmp_get(dev_target.device_id, ecmp_handle, &ecmp_info);
  if (!ecmp_info) {
    return SWITCH_STATUS_INVALID_NHOP;
  }

  mpath_info = &(SWITCH_ECMP_MPATH_INFO(ecmp_info));

  FOR_EACH_IN_LIST(mpath_info->members, node) {
    ecmp_member = (switch_ecmp_member_t *)node->data;
    if (ecmp_member->mbr_hdl == mbr_hdl) {
      break;
    }
  }
  FOR_EACH_IN_LIST_END();

  if (!node) {
    return SWITCH_STATUS_PD_FAILURE;
  }
  nhop_index = handle_to_id(ecmp_member->nhop_handle);

  // Count the number of port instances
  node = tommy_list_head(&(ecmp_failover_info));
  while (node) {
    failover_member = (switch_pd_failover_member_t *)node->data;
    if (failover_member->u.nhop_index == nhop_index) {
      count++;
      if (failover_member->index == table_index) {
        delete_node = node;
      }
    }
    node = node->next;
  }

  // Update the failover lookup table
  if (is_add) {
    // New member added to the selection table
    failover_member =
        SWITCH_MALLOC(device, sizeof(switch_pd_failover_member_t), 1);
    failover_member->u.nhop_index = nhop_index;
    failover_member->instance_id = count;
    failover_member->index = table_index;
    tommy_list_insert_head(
        &ecmp_failover_info, &(failover_member->node), failover_member);
    pd_status =
        switch_pd_ecmp_failover_lookup_entry_add(dev_target,
                                                 nhop_index,
                                                 count,
                                                 table_index,
                                                 &(failover_member->entry_hdl));
  } else {
    // Member removed from the selection table
    if (!delete_node) {
      return SWITCH_STATUS_ITEM_NOT_FOUND;
    }
    failover_member = (switch_pd_failover_member_t *)delete_node->data;
    if (failover_member->instance_id < count - 1) {
      // Removing entry which does not have highest instance_id. Swap the
      // failover_member with the member with the highest instance_id.
      node = tommy_list_head(&(ecmp_failover_info));
      while (node) {
        member = (switch_pd_failover_member_t *)node->data;
        if (member->u.nhop_index == nhop_index &&
            member->instance_id == count - 1) {
          break;
        }
        node = node->next;
      }
      // Update the ecmp_failover_lookup table
      status = p4_pd_dc_ecmp_failover_lookup_table_delete(
          switch_cfg_sess_hdl, dev_target.device_id, member->entry_hdl);
      member->entry_hdl = failover_member->entry_hdl;
      member->instance_id = failover_member->instance_id;
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        goto cleanup;
      }
      pd_status = switch_pd_ecmp_failover_lookup_modify_entry(
          dev_target, member->index, failover_member->entry_hdl);
    } else {
      // Removing entry with highest instance_id, so just delete it.
      pd_status = p4_pd_dc_ecmp_failover_lookup_table_delete(
          switch_cfg_sess_hdl,
          dev_target.device_id,
          failover_member->entry_hdl);
    }
    failover_member =
        tommy_list_remove_existing(&(ecmp_failover_info), delete_node);
    SWITCH_FREE(dev_target.device_id, failover_member);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    goto cleanup;
  }

  // Enable pktgen
  pd_status = switch_pd_failover_pktgen_enable(dev_target.device_id);

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

  return status;
}
#endif /* P4_FAST_FAILOVER_ENABLE */
#endif /* SWITCH_PD */

switch_status_t switch_pd_ecmp_group_register_callback(switch_device_t device,
                                                       void *cookie) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(pd_status);

  UNUSED(device);
  UNUSED(cookie);

#ifdef SWITCH_PD
#ifdef P4_FAST_FAILOVER_ENABLE

  pd_status = p4_pd_dc_ecmp_action_profile_register_callback(
      switch_cfg_sess_hdl, device, switch_pd_ecmp_group_callback, cookie);
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_FAST_FAILOVER_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ecmp group register callback success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ecmp group register callback failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

#ifdef __cplusplus
}
#endif
