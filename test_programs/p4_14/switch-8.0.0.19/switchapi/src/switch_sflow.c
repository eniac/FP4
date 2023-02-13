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

#include "switchapi/switch_sflow.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_sflow_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_pd_sflow_tables_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("sflow default entry add failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_sflow_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

  return status;
}

switch_status_t switch_sflow_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_handle_type_allocator_init(device,
                                             SWITCH_HANDLE_TYPE_SFLOW,
                                             SWITCH_MAX_SFLOW_SESSIONS,
                                             false,
                                             false);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("sflow init failed for device %d:",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_allocator_init(device,
                                             SWITCH_HANDLE_TYPE_SFLOW_ACE,
                                             SWITCH_MAX_SFLOW_ACES,
                                             false,
                                             false);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("sflow init failed for device %d:",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_sflow_free(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_SFLOW);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("sflow init failed for device %d:",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_sflow_session_create_internal(
    const switch_device_t device,
    const switch_api_sflow_session_info_t *api_sflow_info,
    switch_handle_t *sflow_handle) {
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_sflow_info_t *sflow_info = NULL;
  switch_api_mirror_info_t api_mirror_info;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(api_sflow_info != NULL);
  if (!api_sflow_info) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("sflow session create failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (api_sflow_info->collector_type != SFLOW_COLLECTOR_TYPE_CPU) {
    status = SWITCH_STATUS_NOT_SUPPORTED;
    SWITCH_LOG_ERROR("sflow session create failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (api_sflow_info->sample_mode != SWITCH_SFLOW_SAMPLE_PKT) {
    // single packet per notificaiton - other modes are TBD
    status = SWITCH_STATUS_NOT_SUPPORTED;
    SWITCH_LOG_ERROR("sflow session create failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (api_sflow_info->sample_rate == 0) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("sflow session create failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  handle = switch_sflow_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("sflow session create failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_sflow_get(device, handle, &sflow_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("sflow session create failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(&sflow_info->api_info,
                api_sflow_info,
                sizeof(switch_api_sflow_session_info_t));

  status = SWITCH_LIST_INIT(&sflow_info->match_list);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("sflow session create failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  sflow_info->session_id = handle_to_id(handle);
  sflow_info->api_info = *api_sflow_info;
  SWITCH_LIST_INIT(&sflow_info->match_list);

  sflow_info->mirror_handle = SWITCH_API_INVALID_HANDLE;
  sflow_info->mirror_table_ent_hdl = SWITCH_API_INVALID_HANDLE;

  // Create a mirror session to send sampled pkts to CPU.
  // SWITCH_CPU_MIRROR_SESSION_ID mirror-session can be used, except
  // it does not truncate the packet. sFlow may not need entire packet.
  // CPU can perform tuncation as well, but this makes it a bit easier
  // for CPU
  if (api_sflow_info->collector_type == SFLOW_COLLECTOR_TYPE_CPU) {
    SWITCH_MEMSET(&api_mirror_info, 0, sizeof(api_mirror_info));
    api_mirror_info.mirror_type = SWITCH_MIRROR_TYPE_LOCAL;
    // mirror session id is allocated by the mirroring api
    api_mirror_info.session_type = SWITCH_MIRROR_SESSION_TYPE_SIMPLE;
    status = switch_api_device_cpu_port_handle_get(
        device, &api_mirror_info.egress_port_handle);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    api_mirror_info.direction = SWITCH_API_DIRECTION_BOTH;
    api_mirror_info.max_pkt_len = api_sflow_info->extract_len;

    status = switch_api_mirror_session_create(
        device, &api_mirror_info, &sflow_info->mirror_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("sflow session create failed on device %d: %s",
                       device,
                       switch_error_to_string(status));
      goto cleanup;
    }
  } else {
    SWITCH_ASSERT(0);
  }

  status = switch_pd_sflow_session_create(device, sflow_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("sflow session create failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    goto cleanup;
  }

  *sflow_handle = handle;

  return status;

cleanup:
  switch_api_sflow_session_delete(device, handle, false);
  return status;
}

switch_status_t switch_api_sflow_session_delete_internal(
    const switch_device_t device,
    const switch_handle_t sflow_handle,
    bool cleanup) {
  switch_sflow_info_t *sflow_info = NULL;
  switch_node_t *node = NULL;
  switch_sflow_match_entry_t *entry = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_SFLOW_HANDLE(sflow_handle));
  if (!SWITCH_SFLOW_HANDLE(sflow_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("sflow session delete failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_sflow_get(device, sflow_handle, &sflow_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("sflow session delete failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if ((!SWITCH_LIST_EMPTY(&sflow_info->match_list) && !cleanup)) {
    status = SWITCH_STATUS_RESOURCE_IN_USE;
    SWITCH_LOG_ERROR("sflow session delete failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (cleanup) {
    FOR_EACH_IN_LIST(sflow_info->match_list, node) {
      entry = (switch_sflow_match_entry_t *)node->data;
      status = switch_pd_sflow_ingress_table_delete(device, entry);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("sflow session delete failed on device %d: %s",
                         device,
                         switch_error_to_string(status));
      }

      /*
       * Should we do a safe delete here ?
       */
      SWITCH_LIST_DELETE(&sflow_info->match_list, node);

      status = switch_sflow_ace_handle_delete(device, entry->sflow_ace_hdl);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("sflow session delete failed on device %d: %s",
                         device,
                         switch_error_to_string(status));
      }
    }
    FOR_EACH_IN_LIST_END();
  }

  status = switch_pd_sflow_session_delete(device, sflow_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("sflow session delete failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (sflow_info->mirror_handle != SWITCH_API_INVALID_HANDLE) {
    status =
        switch_api_mirror_session_delete(device, sflow_info->mirror_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("sflow session delete failed on device %d: %s",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  }

  status = switch_sflow_handle_delete(device, sflow_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("sflow session delete failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_sflow_match_key_from_tlv(
    const switch_uint16_t kvp_count,
    const switch_sflow_match_key_value_pair_t *kvp,
    switch_sflow_match_key_t *match_key) {
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  bool key_found = FALSE;

  for (index = 0; index < kvp_count; index++) {
    switch (kvp[index].field) {
      case SWITCH_SFLOW_MATCH_PORT:
        match_key->port = kvp[index].value.port;
        key_found = true;
        break;
      case SWITCH_SFLOW_MATCH_VLAN:
        match_key->vlan = kvp[index].value.vlan;
        break;
      case SWITCH_SFLOW_MATCH_SIP:
        match_key->sip = kvp[index].value.sip;
        match_key->sip_mask = (uint32_t)kvp[index].mask.u.mask;
        key_found = true;
        break;
      case SWITCH_SFLOW_MATCH_DIP:
        match_key->dip = kvp[index].value.dip;
        match_key->dip_mask = (uint32_t)kvp[index].mask.u.mask;
        key_found = true;
        break;
      default:
        SWITCH_ASSERT(0);
        break;
    }
  }

  if (!key_found) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("sflow tlv to match key failed %s",
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_sflow_match_entry_get(
    switch_sflow_info_t *sflow_info,
    switch_handle_t entry_handle,
    switch_sflow_match_entry_t **match_entry,
    bool remove) {
  switch_node_t *node = NULL;
  switch_sflow_match_entry_t *tmp_match_entry = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(match_entry != NULL);
  SWITCH_ASSERT(sflow_info != NULL);

  *match_entry = NULL;

  FOR_EACH_IN_LIST(sflow_info->match_list, node) {
    tmp_match_entry = (switch_sflow_match_entry_t *)node->data;
    if (tmp_match_entry->sflow_ace_hdl == entry_handle) {
      if (remove) {
        status = SWITCH_LIST_DELETE(&sflow_info->match_list, node);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "sflow match entry get failed."
              "match entry node delete failed: %s",
              switch_error_to_string(status));
          return status;
        }
      }
      *match_entry = tmp_match_entry;
      break;
    }
  }
  FOR_EACH_IN_LIST_END();

  return status;
}

switch_status_t switch_api_sflow_session_attach_internal(
    const switch_device_t device,
    const switch_handle_t sflow_handle,
    const switch_direction_t direction,
    const switch_uint16_t priority,
    const switch_uint32_t sample_rate,
    const switch_uint16_t kvp_count,
    const switch_sflow_match_key_value_pair_t *kvp,
    switch_handle_t *entry_handle) {
  switch_sflow_info_t *sflow_info = NULL;
  switch_sflow_match_entry_t *match_entry = NULL;
  switch_sflow_match_key_t match_key = {0};
  switch_port_info_t *port_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_SFLOW_HANDLE(sflow_handle));
  CHECK_RET(!SWITCH_SFLOW_HANDLE(sflow_handle), SWITCH_STATUS_INVALID_HANDLE);

  status = switch_sflow_get(device, sflow_handle, &sflow_info);
  CHECK_RET(status != SWITCH_STATUS_SUCCESS, status);

  /*
   * key-value pairs are used to specify the match-criteria for enabling sflow
   * For ingress sflow, ternary match on ingress port, sip, dip are supported
   * TBD - check if the match_spec is already used - callers responsibilty for
   * now
   */
  CHECK_RET(!kvp || !entry_handle || kvp_count > SWITCH_SFLOW_MATCH_FIELD_MAX,
            SWITCH_STATUS_INVALID_PARAMETER);

  *entry_handle = SWITCH_API_INVALID_HANDLE;

  SWITCH_MEMSET(&match_key, 0, sizeof(switch_sflow_match_key_t));
  match_key.port = SWITCH_API_INVALID_HANDLE;

  status = switch_sflow_match_key_from_tlv(kvp_count, kvp, &match_key);
  CHECK_CLEAN(status != SWITCH_STATUS_SUCCESS, status);

  *entry_handle = switch_sflow_ace_handle_create(device);
  CHECK_RET(*entry_handle == SWITCH_API_INVALID_HANDLE,
            SWITCH_STATUS_NO_MEMORY);

  status = switch_sflow_ace_get(device, *entry_handle, &match_entry);
  CHECK_CLEAN(status != SWITCH_STATUS_SUCCESS, SWITCH_STATUS_NO_MEMORY);

  match_entry->sflow_ace_hdl = *entry_handle;
  status = switch_port_get(device, match_key.port, &port_info);
  CHECK_CLEAN(status != SWITCH_STATUS_SUCCESS, status);

  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    status = switch_pd_sflow_ingress_table_add(device,
                                               &match_key,
                                               port_info->port_lag_index,
                                               priority,
                                               sample_rate,
                                               sflow_info,
                                               match_entry);
    CHECK_CLEAN(status != SWITCH_STATUS_SUCCESS, status);

    status = SWITCH_LIST_INSERT(
        &sflow_info->match_list, &match_entry->node, match_entry);
    CHECK_CLEAN(status != SWITCH_STATUS_SUCCESS, status);

  } else if (direction == SWITCH_API_DIRECTION_EGRESS) {
    CHECK_CLEAN(direction == SWITCH_API_DIRECTION_EGRESS,
                SWITCH_STATUS_NOT_SUPPORTED);
  }

  return status;

clean:

  return status;
}

switch_status_t switch_api_sflow_session_detach_internal(
    const switch_device_t device,
    const switch_handle_t sflow_handle,
    const switch_handle_t entry_hdl) {
  switch_sflow_match_entry_t *match_entry = NULL;
  switch_sflow_info_t *sflow_info = NULL;
  switch_status_t status = SWITCH_STATUS_FAILURE;

  SWITCH_ASSERT(SWITCH_SFLOW_HANDLE(sflow_handle));
  if (!SWITCH_SFLOW_HANDLE(sflow_handle)) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR("sflow session detach failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_sflow_get(device, sflow_handle, &sflow_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("sflow session detach failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status =
      switch_sflow_match_entry_get(sflow_info, entry_hdl, &match_entry, TRUE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("sflow session detach failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_pd_sflow_ingress_table_delete(device, match_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("sflow session detach failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_sflow_ace_handle_delete(device, entry_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("sflow session detach failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

switch_status_t switch_api_sflow_session_create(
    const switch_device_t device,
    const switch_api_sflow_session_info_t *api_sflow_info,
    switch_handle_t *sflow_handle) {
  SWITCH_MT_WRAP(switch_api_sflow_session_create_internal(
      device, api_sflow_info, sflow_handle))
}

switch_status_t switch_api_sflow_session_detach(
    const switch_device_t device,
    const switch_handle_t sflow_handle,
    const switch_handle_t entry_handle) {
  SWITCH_MT_WRAP(switch_api_sflow_session_detach_internal(
      device, sflow_handle, entry_handle))
}

switch_status_t switch_api_sflow_session_port_detach(
    const switch_device_t device,
    const switch_handle_t port_handle,
    const switch_direction_t direction) {
  switch_handle_t sflow_handle;
  switch_handle_t session_handle;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_port_info_t *port_info = NULL;

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port handle get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    sflow_handle = port_info->ingress_sflow_handle;
    session_handle = port_info->ingress_sflow_entry_handle;
  } else {
    sflow_handle = port_info->egress_sflow_handle;
    session_handle = port_info->egress_sflow_entry_handle;
  }

  status =
      switch_api_sflow_session_detach(device, sflow_handle, session_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("sflow session attach failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_api_sflow_session_port_attach(
    const switch_device_t device,
    const switch_handle_t sflow_handle,
    const switch_handle_t port_handle,
    const switch_direction_t direction) {
  switch_sflow_match_key_value_pair_t kvp;
  switch_sflow_info_t *sflow_info = NULL;
  switch_handle_t session_handle;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_port_info_t *port_info = NULL;

  status = switch_sflow_get(device, sflow_handle, &sflow_info);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("sflow session get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_port_get(device, port_handle, &port_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port handle get failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  if (sflow_handle != SWITCH_API_INVALID_HANDLE) {
    kvp.field = SWITCH_SFLOW_MATCH_PORT;
    kvp.value.port = port_handle;

    status = switch_api_sflow_session_attach(device,
                                             sflow_handle,
                                             direction,
                                             0,
                                             sflow_info->api_info.sample_rate,
                                             1,
                                             &kvp,
                                             &session_handle);
    if (status == SWITCH_STATUS_SUCCESS) {
      if (direction == SWITCH_API_DIRECTION_INGRESS) {
        port_info->ingress_sflow_handle = sflow_handle;
        port_info->ingress_sflow_entry_handle = session_handle;
      } else {
        port_info->egress_sflow_handle = sflow_handle;
        port_info->egress_sflow_entry_handle = session_handle;
      }
    }
  }

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("sflow session attach failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_api_sflow_session_port_set_internal(
    const switch_device_t device,
    const switch_handle_t sflow_handle,
    const switch_handle_t port_handle,
    const switch_direction_t direction) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  if (sflow_handle != SWITCH_API_INVALID_HANDLE) {
    status = switch_api_sflow_session_port_attach(
        device, sflow_handle, port_handle, direction);
  } else {
    status =
        switch_api_sflow_session_port_detach(device, port_handle, direction);
  }

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("sflow session attach/detach failed on device %d: %s",
                     device,
                     switch_error_to_string(status));
  }
  return status;
}

switch_status_t switch_api_sflow_session_port_set(
    const switch_device_t device,
    const switch_handle_t sflow_handle,
    const switch_handle_t port_handle,
    const switch_direction_t direction) {
  SWITCH_MT_WRAP(switch_api_sflow_session_port_set_internal(
      device, sflow_handle, port_handle, direction))
}

switch_status_t switch_api_sflow_session_attach(
    const switch_device_t device,
    const switch_handle_t sflow_handle,
    const switch_direction_t direction,
    const switch_uint16_t priority,
    const switch_uint32_t sample_rate,
    const switch_uint16_t kvp_count,
    const switch_sflow_match_key_value_pair_t *kvp,
    switch_handle_t *entry_handle) {
  SWITCH_MT_WRAP(switch_api_sflow_session_attach_internal(device,
                                                          sflow_handle,
                                                          direction,
                                                          priority,
                                                          sample_rate,
                                                          kvp_count,
                                                          kvp,
                                                          entry_handle))
}

switch_status_t switch_api_sflow_session_delete(
    const switch_device_t device,
    const switch_handle_t sflow_handle,
    const bool cleanup) {
  SWITCH_MT_WRAP(
      switch_api_sflow_session_delete_internal(device, sflow_handle, cleanup))
}
