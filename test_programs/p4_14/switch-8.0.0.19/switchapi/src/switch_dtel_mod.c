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

#include "switchapi/switch_dtel.h"
#include "switch_internal.h"
#include "switch_pd_dtel.h"

#include <pthread.h>
#include <unistd.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_mirror_on_drop_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

#ifdef P4_DTEL_DROP_REPORT_ENABLE
  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel MoD init failed for device %d: %s, cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  dtel_ctx->_mod.watchlist.size = DTEL_DROP_WATCHLIST_TABLE_SIZE * 2;
  dtel_ctx->_mod.watchlist.compare_func = switch_twl_key_compare;
  dtel_ctx->_mod.watchlist.key_func = switch_twl_key_init;
  dtel_ctx->_mod.watchlist.hash_seed = 0x98761234;
  status = SWITCH_HASHTABLE_INIT(&dtel_ctx->_mod.watchlist);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Cannot init MoD Watchlist for device %d: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  dtel_ctx->_mod.off_hdl = SWITCH_PD_INVALID_HANDLE;
  for (int i = 0; i < MIRROR_ON_DROP_ENCAP_ENTRIES_NUM; i++) {
    dtel_ctx->_mod.me_hdl[i] = SWITCH_PD_INVALID_HANDLE;
  }
  dtel_ctx->_mod.dod_init = false;

#endif
  return status;
}

switch_status_t switch_mirror_on_drop_default_entries_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);

#ifdef P4_DTEL_DROP_REPORT_ENABLE

  status = switch_pd_mirror_on_drop_tables_init(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Mirror on Drop table pd init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_api_dtel_drop_report_disable(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Mirror on Drop disable by default failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    return status;
  }

#endif  // P4_DTEL_DROP_REPORT_ENABLE

#if defined(P4_DTEL_QUEUE_REPORT_ENABLE) || defined(P4_DTEL_DROP_REPORT_ENABLE)
  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel MoD init failed for device %d: %s, cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  status = switch_pd_mirror_on_drop_encap_update(device,
                                                 dtel_ctx->switch_id,
                                                 dtel_ctx->dest_udp_port,
                                                 dtel_ctx->event_infos,
                                                 true,
                                                 dtel_ctx->_mod.me_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("Mirror on Drop encap init failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status = switch_mirror_on_drop_enable_dod(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("DoD enable failed for device %d: %s \n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

#endif  // P4_DTEL_QUEUE_REPORT_ENABLE || P4_DTEL_DROP_REPORT_ENABLE

  return status;
}

switch_status_t switch_mirror_on_drop_enable_dod(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

#if defined(P4_DTEL_QUEUE_REPORT_ENABLE) || defined(P4_DTEL_DROP_REPORT_ENABLE)

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel DoD enable failed for device %d: %s, cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  // set deflect on drop destination to recirc port on each pipe
  // when mirror on drop is enabled for the first time
  if (!dtel_ctx->_mod.dod_init) {
    switch_handle_t port_handle = SWITCH_API_INVALID_HANDLE;
    switch_port_info_t *port_info = NULL;
    for (switch_pipe_t pipe_id = 0; pipe_id < SWITCH_MAX_PIPES; pipe_id += 1) {
      status = switch_api_device_recirc_port_get(device, pipe_id, &port_handle);
      if (status == SWITCH_STATUS_SUCCESS &&
          port_handle != SWITCH_PD_INVALID_HANDLE) {  // pipe exists
        status = switch_port_get(device, port_handle, &port_info);
        if (status == SWITCH_STATUS_SUCCESS && port_info->num_queues > 0) {
          status = switch_api_dtel_tail_drop_deflection_queue_set(
              device, pipe_id, port_info->queue_handles[0]);
          if (status != SWITCH_STATUS_SUCCESS) {
            SWITCH_LOG_ERROR(
                "Cannot set DoD to recirc port on device %d, pipe %d: %s \n",
                device,
                pipe_id,
                switch_error_to_string(status));
            return status;
          }
        } else {
          SWITCH_LOG_ERROR(
              "Recirc port queue get error on device %d, pipe %d: %s \n",
              device,
              pipe_id,
              switch_error_to_string(status));
        }
      }
    }
    dtel_ctx->_mod.dod_init = true;
    status = SWITCH_STATUS_SUCCESS;
  }
#endif  // P4_DTEL_QUEUE_REPORT_ENABLE || P4_DTEL_DROP_REPORT_ENABLE

  return status;
}

switch_status_t switch_api_dtel_drop_report_enable_internal(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

#ifdef P4_DTEL_DROP_REPORT_ENABLE

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel MoD enable failed for device %d: %s, cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (dtel_ctx->_mod.off_hdl != SWITCH_PD_INVALID_HANDLE) {
    status =
        switch_pd_drop_watchlist_entry_delete(device, dtel_ctx->_mod.off_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("MoD enable failed for device %d: %s \n",
                       device,
                       switch_error_to_string(status));
      return status;
    }
    dtel_ctx->_mod.off_hdl = SWITCH_PD_INVALID_HANDLE;
  }

#endif
  return status;
}

switch_status_t switch_api_dtel_drop_report_disable_internal(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

#ifdef P4_DTEL_DROP_REPORT_ENABLE

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "DTel MoD disable failed for device %d: %s, cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (dtel_ctx->_mod.off_hdl == SWITCH_PD_INVALID_HANDLE) {
    switch_twl_match_spec_t twl_match;
    SWITCH_MEMSET(&twl_match, 0x0, sizeof(switch_twl_match_spec_t));
    twl_match.l4_port_src_start = 0;
    twl_match.l4_port_src_end = 0xFFFF;
    twl_match.l4_port_dst_start = 0;
    twl_match.l4_port_dst_end = 0xFFFF;
    twl_match.inner_l4_port_src_start = 0;
    twl_match.inner_l4_port_src_end = 0xFFFF;
    twl_match.inner_l4_port_dst_start = 0;
    twl_match.inner_l4_port_dst_end = 0xFFFF;
    status = switch_pd_drop_watchlist_entry_create(
        device, &twl_match, 0, false, NULL, &dtel_ctx->_mod.off_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("MoD disable failed for device %d: %s \n",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  }

#endif

  return status;
}

//------------------------------------------------------------------------------
// MoD watchlist internal add/update/delete/clear
//------------------------------------------------------------------------------

switch_status_t switch_dtel_drop_watchlist_entry_create(
    switch_device_t device,
    switch_twl_match_info_t *match_info,
    switch_uint16_t priority,
    bool watch,
    switch_twl_action_params_t *action_params) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(match_info);
  UNUSED(priority);
  UNUSED(watch);
  UNUSED(action_params);

#ifdef P4_DTEL_DROP_REPORT_ENABLE

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "MoD watchlist add failed for device %d: %s, cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  int count = SWITCH_HASHTABLE_COUNT(&dtel_ctx->_mod.watchlist);
  if (count > DTEL_DROP_WATCHLIST_TABLE_SIZE - 2) {
    status = SWITCH_STATUS_TABLE_FULL;
    SWITCH_LOG_ERROR("MoD watchlist full for device %d\n", device);
    return status;
  }

  // search if entry exists
  switch_twl_match_spec_t twl_match_spec;
  SWITCH_MEMSET(&twl_match_spec, 0x0, sizeof(switch_twl_match_spec_t));
  switch_twl_convert_match_spec(
      match_info->field_count, match_info->fields, &twl_match_spec);
  SWITCH_PD_LOG_DEBUG("DTel MoD add searching for: ");
  switch_twl_match_spec_print(&twl_match_spec);

  switch_twl_entry_t *twl_entry = NULL;
  status = SWITCH_HASHTABLE_SEARCH(&dtel_ctx->_mod.watchlist,
                                   (void *)(&twl_match_spec),
                                   (void **)&twl_entry);
  if (status == SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    SWITCH_LOG_ERROR(
        "MoD watchlist add failed for device %d: %s, item already exists\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  // create new entry
  twl_entry = SWITCH_MALLOC(device, sizeof(switch_twl_entry_t), 1);
  if (!twl_entry) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR("MoD watchlist memory allocation failed for device %d\n",
                     device);
    return status;
  }
  SWITCH_MEMSET(twl_entry, 0, sizeof(switch_twl_entry_t));
  switch_twl_convert_match_spec(
      match_info->field_count, match_info->fields, &twl_entry->match);
  SWITCH_PD_LOG_DEBUG("DTel MoD adding for: ");
  switch_twl_match_spec_print(&twl_match_spec);
  twl_entry->priority = priority;
  twl_entry->pd_hdl = 0;

  // add new entry to h/w
  status = switch_pd_drop_watchlist_entry_create(device,
                                                 &twl_entry->match,
                                                 priority,
                                                 watch,
                                                 action_params,
                                                 &twl_entry->pd_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("MoD watchlist add failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  // add new entry to hashtable
  status = SWITCH_HASHTABLE_INSERT(&dtel_ctx->_mod.watchlist,
                                   &twl_entry->node,
                                   (void *)(&twl_entry->match),
                                   (void *)(twl_entry));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("MoD watchlist hashtable insert failed for device %d\n",
                     device);
    return status;
  }

#endif  // P4_DTEL_DROP_REPORT_ENABLE

  return status;
}

switch_status_t switch_dtel_drop_watchlist_entry_update(
    switch_device_t device,
    switch_twl_match_info_t *match_info,
    switch_uint16_t priority,
    bool watch,
    switch_twl_action_params_t *action_params) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(match_info);
  UNUSED(priority);
  UNUSED(watch);
  UNUSED(action_params);

#ifdef P4_DTEL_DROP_REPORT_ENABLE

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "MoD watchlist update failed for device %d: %s, cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  // search if entry exists
  switch_twl_match_spec_t twl_match_spec;
  SWITCH_MEMSET(&twl_match_spec, 0x0, sizeof(switch_twl_match_spec_t));
  switch_twl_convert_match_spec(
      match_info->field_count, match_info->fields, &twl_match_spec);
  SWITCH_PD_LOG_DEBUG("DTel MoD update searching for: ");
  switch_twl_match_spec_print(&twl_match_spec);

  switch_twl_entry_t *twl_entry = NULL;
  status = SWITCH_HASHTABLE_SEARCH(&dtel_ctx->_mod.watchlist,
                                   (void *)(&twl_match_spec),
                                   (void **)&twl_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "MoD watchlist update failed for device %d: %s, item not found\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (twl_entry->priority != priority) {
    // different priority, have to delete and add agian
    status = switch_pd_drop_watchlist_entry_delete(device, twl_entry->pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "MoD watchlist update failed for device %d: %s, delete failure\n",
          device,
          switch_error_to_string(status));
      return status;
    }

    status = switch_pd_drop_watchlist_entry_create(device,
                                                   &twl_entry->match,
                                                   priority,
                                                   watch,
                                                   action_params,
                                                   &twl_entry->pd_hdl);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "MoD watchlist update failed for device %d: %s, add failure\n",
          device,
          switch_error_to_string(status));
      return status;
    }
  } else {  // same priority, update by pd handle
    status = switch_pd_drop_watchlist_entry_update(
        device, twl_entry->pd_hdl, watch, action_params);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "MoD watchlist update failed for device %d: %s, update failure\n",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

#endif  // P4_DTEL_DROP_REPORT_ENABLE

  return status;
}

switch_status_t switch_dtel_drop_watchlist_entry_delete(
    switch_device_t device, switch_twl_match_info_t *match_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  UNUSED(device);
  UNUSED(match_info);

#ifdef P4_DTEL_DROP_REPORT_ENABLE

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "MoD watchlist delete failed for device %d: %s, cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  switch_twl_match_spec_t twl_match_spec;
  SWITCH_MEMSET(&twl_match_spec, 0x0, sizeof(switch_twl_match_spec_t));
  switch_twl_convert_match_spec(
      match_info->field_count, match_info->fields, &twl_match_spec);
  SWITCH_PD_LOG_DEBUG("DTel MoD delete searching for: ");
  switch_twl_match_spec_print(&twl_match_spec);

  switch_twl_entry_t *twl_entry = NULL;
  status = SWITCH_HASHTABLE_SEARCH(&dtel_ctx->_mod.watchlist,
                                   (void *)(&twl_match_spec),
                                   (void **)&twl_entry);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "MoD watchlist delete failed for device %d: %s, item not found\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_drop_watchlist_entry_delete(device, twl_entry->pd_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("MoD watchlist delete failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  status =
      SWITCH_HASHTABLE_DELETE_NODE(&dtel_ctx->_mod.watchlist, &twl_entry->node);
  if (status == SWITCH_STATUS_ITEM_NOT_FOUND) {
    SWITCH_LOG_ERROR(
        "MoD watchlist hashtable delete failed for device %d: %s\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  SWITCH_FREE(device, twl_entry);

#endif  // P4_DTEL_DROP_REPORT_ENABLE

  return status;
}

#ifdef P4_DTEL_DROP_REPORT_ENABLE
static void drop_watchlist_entry_delete_foreach(void *arg, void *data) {
  switch_device_t *device = (switch_device_t *)arg;
  switch_twl_entry_t *twl_entry = (switch_twl_entry_t *)data;
  switch_status_t status =
      switch_pd_drop_watchlist_entry_delete(*device, twl_entry->pd_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("MoD watchlist delete failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      *device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "MoD watchlist delete failed for device %d: %s,"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
  }

  status =
      SWITCH_HASHTABLE_DELETE_NODE(&dtel_ctx->_mod.watchlist, &twl_entry->node);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("MoD hashtable delete failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
  }
  SWITCH_FREE(*device, twl_entry);
}
#endif  // P4_DTEL_DROP_REPORT_ENABLE

switch_status_t switch_dtel_drop_watchlist_clear(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

#ifdef P4_DTEL_DROP_REPORT_ENABLE

  switch_dtel_context_t *dtel_ctx = NULL;
  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_DTEL, (void **)&dtel_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "MoD watchlist clear failed for device %d: %s,"
        " cannot get context\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_HASHTABLE_FOREACH_ARG(
      &dtel_ctx->_mod.watchlist, &drop_watchlist_entry_delete_foreach, &device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("MoD watchlist clear failed for device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

#endif  // P4_DTEL_DROP_REPORT_ENABLE

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_dtel_drop_report_enable(switch_device_t device) {
  SWITCH_MT_WRAP(switch_api_dtel_drop_report_enable_internal(device))
}

switch_status_t switch_api_dtel_drop_report_disable(switch_device_t device) {
  SWITCH_MT_WRAP(switch_api_dtel_drop_report_disable_internal(device))
}
