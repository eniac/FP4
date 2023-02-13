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
#include "switchapi/switch_qos.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_QOS

switch_status_t switch_qos_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_pd_qos_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "qos default entry add failed on device %d: "
        "qos table default add failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_qos_map_egress_default_entries_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "qos default entries add failed on device %d: "
        "egress qos map table default entries add failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("qos default entries add success on device %d\n", device);
  return status;
}

switch_status_t switch_qos_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

  return status;
}

switch_size_t switch_qos_table_size_get(switch_device_t device,
                                        switch_size_t *qos_table_size) {
  switch_size_t table_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_table_id_t table_id = 0;

  SWITCH_ASSERT(qos_table_size != NULL);

  *qos_table_size = 0;

  for (table_id = SWITCH_TABLE_INGRESS_QOS_MAP_DSCP;
       table_id <= SWITCH_TABLE_EGRESS_QOS_MAP;
       table_id++) {
    status = switch_api_table_size_get(device, table_id, &table_size);
    if (status != SWITCH_STATUS_SUCCESS) {
      *qos_table_size = 0;
      SWITCH_LOG_ERROR(
          "qos table size get failed on device %d: %s"
          "for table %s",
          device,
          switch_error_to_string(status),
          switch_table_id_to_string(table_id));
      return status;
    }

    *qos_table_size += table_size;
  }

  return status;
}

switch_status_t switch_qos_init(switch_device_t device) {
  switch_qos_context_t *qos_ctx = NULL;
  switch_size_t qos_table_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  qos_ctx = SWITCH_MALLOC(device, sizeof(switch_qos_context_t), 0x1);
  if (!qos_ctx) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "qos init failed on device %d: "
        "qos context memory allocation failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(qos_ctx, 0x0, sizeof(switch_qos_context_t));

  status = switch_device_api_context_set(
      device, SWITCH_API_TYPE_QOS, (void *)qos_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "qos init failed on device %d: "
        "qos context set failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  /*
   * Compute the qos handle array size
   */
  status = switch_qos_table_size_get(device, &qos_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "qos init failed on device %d: "
        "qos table size get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  /*
   * Allocating handle for SWITCH_HANDLE_TYPE_QOS_MAP
   */
  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_QOS_MAP, qos_table_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "qos init failed on device %d: "
        "qos handle init failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_id_allocator_new(
      device, 256, FALSE, &qos_ctx->ingress_qos_map_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "qos init failed on device %d: "
        "qos ingress map failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_api_id_allocator_new(device, 256, FALSE, &qos_ctx->tc_qos_map_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "qos init failed on device %d: "
        "qos tc qos map failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_api_id_allocator_new(
      device, 256, FALSE, &qos_ctx->egress_qos_map_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "qos init failed on device %d: "
        "qos egress map failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  memset(qos_ctx->tc_qos_map,
         0,
         SWITCH_MAX_TRAFFIC_CLASSES * sizeof(switch_qos_map_t));
  memset(qos_ctx->pd_tc_qos_map,
         0,
         SWITCH_MAX_TRAFFIC_CLASSES * sizeof(switch_pd_hdl_t));
  SWITCH_LOG_DEBUG("qos init successful on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_qos_free(switch_device_t device) {
  switch_qos_context_t *qos_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_QOS, (void **)&qos_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "qos free failed on device %d: "
        "qos context get failed(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  /*
   * Freeing handle for SWITCH_HANDLE_TYPE_QOS_MAP
   */
  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_QOS_MAP);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "qos free failed on device %d: "
        "qos handle free failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_api_id_allocator_destroy(device, qos_ctx->ingress_qos_map_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "qos free failed on device %d: "
        "qos ingress qos map free failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_api_id_allocator_destroy(device, qos_ctx->tc_qos_map_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "qos free failed on device %d: "
        "qos tc qos map free failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_api_id_allocator_destroy(device, qos_ctx->egress_qos_map_id);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "qos free failed on device %d: "
        "qos egress qos map free failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  SWITCH_FREE(device, qos_ctx);
  status = switch_device_api_context_set(device, SWITCH_API_TYPE_QOS, NULL);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_DEBUG("qos free success on device %d\n", device);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_qos_map_create(
    switch_device_t device,
    switch_direction_t direction,
    switch_qos_map_ingress_t ingress_qos_map_type,
    switch_qos_map_egress_t egress_qos_map_type,
    switch_uint8_t num_entries,
    switch_qos_map_t *qos_map,
    switch_handle_t *qos_map_handle,
    bool update) {
  switch_qos_context_t *qos_ctx = NULL;
  switch_qos_map_list_t *qos_map_list_info = NULL;
  switch_qos_map_info_t *qos_map_info = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_qos_group_t qos_group = 0;
  switch_uint32_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  bool hardware_update = true;

  if (!qos_map || num_entries == 0) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "qos map create faield on device %d: "
        "parameters invalid(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_QOS, (void **)&qos_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "qos map create faield on device %d: "
        "qos context get failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (!update) {
    handle = switch_qos_map_handle_create(device);
  } else {
    handle = *qos_map_handle;
  }
  SWITCH_ASSERT(handle != SWITCH_API_INVALID_HANDLE);

  status = switch_qos_map_get(device, handle, &qos_map_list_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "qos map create faield on device %d: "
        "qos map get failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LIST_INIT(&qos_map_list_info->qos_map_list);

  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    switch (ingress_qos_map_type) {
      case SWITCH_QOS_MAP_INGRESS_DSCP_TO_TC:
      case SWITCH_QOS_MAP_INGRESS_TOS_TO_TC:
      case SWITCH_QOS_MAP_INGRESS_PCP_TO_TC:
      case SWITCH_QOS_MAP_INGRESS_DSCP_TO_COLOR:
      case SWITCH_QOS_MAP_INGRESS_TOS_TO_COLOR:
      case SWITCH_QOS_MAP_INGRESS_PCP_TO_COLOR:
      case SWITCH_QOS_MAP_INGRESS_DSCP_TO_TC_AND_COLOR:
      case SWITCH_QOS_MAP_INGRESS_TOS_TO_TC_AND_COLOR:
      case SWITCH_QOS_MAP_INGRESS_PCP_TO_TC_AND_COLOR:
      case SWITCH_QOS_MAP_INGRESS_DSCP_TO_QID_AND_TC_AND_COLOR:
      case SWITCH_QOS_MAP_INGRESS_TOS_TO_QID_AND_TC_AND_COLOR:
      case SWITCH_QOS_MAP_INGRESS_PCP_TO_QID_AND_TC_AND_COLOR:
      case SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS:
      case SWITCH_QOS_MAP_INGRESS_TC_TO_QUEUE:
      case SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS_AND_QUEUE:
        if (update) {
          /*
           * In case of update, use the same qos_group as previously allocated
           * index.
           * All the targets will have the same index.
           */
          qos_group = qos_map_list_info->qos_group;
        } else {
          status = switch_api_id_allocator_allocate(
              device, qos_ctx->ingress_qos_map_id, &qos_group);
          if (status != SWITCH_STATUS_SUCCESS) {
            SWITCH_LOG_ERROR(
                "qos map create failed on device %d: "
                "ingress qos map id allocation failed(%s)",
                device,
                switch_error_to_string(status));
            goto cleanup;
          }
        }
        break;
      case SWITCH_QOS_MAP_INGRESS_PFC_PRIORITY_TO_PPG:
        hardware_update = false;
        break;

      default:
        status = SWITCH_STATUS_INVALID_PARAMETER;
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "qos map create failed on device %d: "
              "ingress qos map type invalid(%s)",
              device,
              switch_error_to_string(status));
          goto cleanup;
        }
    }
  } else {
    switch (egress_qos_map_type) {
      case SWITCH_QOS_MAP_EGRESS_TC_TO_DSCP:
      case SWITCH_QOS_MAP_EGRESS_TC_TO_TOS:
      case SWITCH_QOS_MAP_EGRESS_TC_TO_PCP:
      case SWITCH_QOS_MAP_EGRESS_COLOR_TO_DSCP:
      case SWITCH_QOS_MAP_EGRESS_COLOR_TO_TOS:
      case SWITCH_QOS_MAP_EGRESS_COLOR_TO_PCP:
      case SWITCH_QOS_MAP_EGRESS_TC_AND_COLOR_TO_DSCP:
      case SWITCH_QOS_MAP_EGRESS_TC_AND_COLOR_TO_TOS:
      case SWITCH_QOS_MAP_EGRESS_TC_AND_COLOR_TO_PCP:
        if (update) {
          qos_group = qos_map_list_info->qos_group;
        } else {
          status = switch_api_id_allocator_allocate(
              device, qos_ctx->egress_qos_map_id, &qos_group);
          if (status != SWITCH_STATUS_SUCCESS) {
            SWITCH_LOG_ERROR(
                "qos map create failed on device %d: "
                "egress qos map id allocation failed(%s)",
                device,
                switch_error_to_string(status));
            goto cleanup;
          }
        }
        break;
      case SWITCH_QOS_MAP_EGRESS_PFC_PRIORITY_TO_QUEUE:
        hardware_update = false;
        break;
      default:
        status = SWITCH_STATUS_INVALID_PARAMETER;
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "qos map create failed on device %d: "
              "egress qos map type invalid(%s)",
              device,
              switch_error_to_string(status));
          goto cleanup;
        }
    }
  }

  for (index = 0; index < num_entries; index++) {
    qos_map_info = SWITCH_MALLOC(device, sizeof(switch_qos_map_info_t), 1);
    if (!qos_map_info) {
      status = SWITCH_STATUS_NO_MEMORY;
      SWITCH_LOG_ERROR(
          "qos map create failed on device %d: "
          "qos map memory allocation failed(%s)",
          device,
          switch_error_to_string(status));
      goto cleanup;
    }

    if (direction == SWITCH_API_DIRECTION_INGRESS && hardware_update) {
      if ((ingress_qos_map_type == SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS) ||
          (ingress_qos_map_type == SWITCH_QOS_MAP_INGRESS_TC_TO_QUEUE) ||
          (ingress_qos_map_type ==
           SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS_AND_QUEUE)) {
        if ((ingress_qos_map_type == SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS &&
             (qos_ctx->tc_icos_hdl[qos_map[index].tc] !=
              SWITCH_API_INVALID_HANDLE)) ||
            (ingress_qos_map_type == SWITCH_QOS_MAP_INGRESS_TC_TO_QUEUE &&
             (qos_ctx->tc_queue_hdl[qos_map[index].tc] !=
              SWITCH_API_INVALID_HANDLE))) {
          SWITCH_LOG_ERROR(
              "qos map create failed on device %d"
              "only one global tc to icos/queue map table is allowed",
              device);
          return SWITCH_STATUS_INSUFFICIENT_RESOURCES;
        }
        /*
         * Update the global TC to icos/queue map table and program the
         * hardware.
         */
        printf("Index %d, tc %d, qid %d, icos %d \n",
               index,
               qos_map[index].tc,
               qos_map[index].qid,
               qos_map[index].icos);

        if (ingress_qos_map_type == SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS) {
          qos_map[index].qid = qos_ctx->tc_qos_map[qos_map[index].tc].qid;
          qos_ctx->tc_icos_hdl[qos_map[index].tc] = handle;
        } else if (ingress_qos_map_type == SWITCH_QOS_MAP_INGRESS_TC_TO_QUEUE) {
          qos_map[index].icos = qos_ctx->tc_qos_map[qos_map[index].tc].icos;
          qos_ctx->tc_queue_hdl[qos_map[index].tc] = handle;
        }

        if (qos_ctx->pd_tc_qos_map[qos_map[index].tc] == 0) {
          status = switch_pd_qos_map_ingress_entry_add(
              device,
              SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS_AND_QUEUE,
              qos_group,
              &qos_map[index],
              &qos_map_info->pd_hdl);
          if (status != SWITCH_STATUS_SUCCESS) {
            SWITCH_LOG_ERROR(
                "qosmap create failed on device %d"
                "ingress tc qosmap update failed for tc %d",
                qos_map[index].tc);
            return status;
          }
          qos_ctx->tc_qos_map[qos_map[index].tc].tc = qos_map[index].tc;
          qos_ctx->pd_tc_qos_map[qos_map[index].tc] = qos_map_info->pd_hdl;
        } else {
          status = switch_pd_qos_map_ingress_entry_update(
              device,
              SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS_AND_QUEUE,
              qos_group,
              &qos_map[index],
              qos_ctx->pd_tc_qos_map[qos_map[index].tc]);
          if (status != SWITCH_STATUS_SUCCESS) {
            SWITCH_LOG_ERROR(
                "qosmap create failed on device %d"
                "ingress tc qosmap update failed for tc %d",
                qos_ctx->pd_tc_qos_map[qos_map[index].tc]);
            return status;
          }
        }
        qos_ctx->tc_qos_map[qos_map[index].tc].icos = qos_map[index].icos;
        qos_ctx->tc_qos_map[qos_map[index].tc].qid = qos_map[index].qid;
      } else {
        status = switch_pd_qos_map_ingress_entry_add(device,
                                                     ingress_qos_map_type,
                                                     qos_group,
                                                     &qos_map[index],
                                                     &qos_map_info->pd_hdl);
      }
    } else if (direction == SWITCH_API_DIRECTION_EGRESS && hardware_update) {
      status = switch_pd_qos_map_egress_entry_add(device,
                                                  egress_qos_map_type,
                                                  qos_group,
                                                  &qos_map[index],
                                                  &qos_map_info->pd_hdl);
    }

    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "qos map create failed on device %d: "
          "qos map table add failed(%s)",
          device,
          switch_error_to_string(status));
      goto cleanup;
    }

    SWITCH_MEMCPY(
        &qos_map_info->qos_map, &qos_map[index], sizeof(switch_qos_map_t));

    status = SWITCH_LIST_INSERT(
        &qos_map_list_info->qos_map_list, &qos_map_info->node, qos_map_info);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  }

  qos_map_list_info->qos_group = qos_group;
  qos_map_list_info->direction = direction;
  qos_map_list_info->ingress_qos_map_type = ingress_qos_map_type;
  qos_map_list_info->egress_qos_map_type = egress_qos_map_type;
  qos_map_list_info->num_entries = num_entries;

  if (!update) {
    *qos_map_handle = handle;
  }

  SWITCH_LOG_DEBUG(
      "qos map created on device %d qos map handle %lx", device, handle);

  return status;

cleanup:
  return status;
}

switch_status_t switch_qos_map_delete(switch_device_t device,
                                      switch_direction_t direction,
                                      switch_handle_t qos_map_handle,
                                      bool update) {
  switch_qos_context_t *qos_ctx = NULL;
  switch_qos_map_list_t *qos_map_list_info = NULL;
  switch_qos_map_info_t *qos_map_info = NULL;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  bool qos_map_delete = true;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_QOS, (void **)&qos_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "qos map delete faield on device %d: "
        "qos context get failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_QOS_MAP_HANDLE(qos_map_handle));
  if (!SWITCH_QOS_MAP_HANDLE(qos_map_handle)) {
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("qos map delete faield on device: %s",
                       switch_error_to_string(status));
      goto cleanup;
    }
  }

  status = switch_qos_map_get(device, qos_map_handle, &qos_map_list_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("qos map delete faield on device: %s",
                     switch_error_to_string(status));
    return status;
  }

  FOR_EACH_IN_LIST(qos_map_list_info->qos_map_list, node) {
    qos_map_info = node->data;
    if (direction == SWITCH_API_DIRECTION_INGRESS) {
      if ((qos_map_list_info->ingress_qos_map_type ==
           SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS) ||
          (qos_map_list_info->ingress_qos_map_type ==
           SWITCH_QOS_MAP_INGRESS_TC_TO_QUEUE) ||
          (qos_map_list_info->ingress_qos_map_type ==
           SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS_AND_QUEUE)) {
        if ((qos_map_list_info->ingress_qos_map_type ==
             SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS) &&
            (qos_ctx->tc_queue_hdl[qos_map_info->qos_map.tc] !=
             SWITCH_API_INVALID_HANDLE)) {
          qos_map_delete = false;
          qos_ctx->tc_qos_map[qos_map_info->qos_map.tc].icos = 0;
          qos_ctx->tc_icos_hdl[qos_map_info->qos_map.tc] =
              SWITCH_API_INVALID_HANDLE;
        }
        if ((qos_map_list_info->ingress_qos_map_type ==
             SWITCH_QOS_MAP_INGRESS_TC_TO_QUEUE) &&
            (qos_ctx->tc_icos_hdl[qos_map_info->qos_map.tc] !=
             SWITCH_API_INVALID_HANDLE)) {
          qos_map_delete = false;
          qos_ctx->tc_qos_map[qos_map_info->qos_map.tc].qid = 0;
          qos_ctx->tc_queue_hdl[qos_map_info->qos_map.tc] =
              SWITCH_API_INVALID_HANDLE;
        }
        if (qos_map_delete) {
          status = switch_pd_qos_map_ingress_entry_delete(
              device,
              SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS_AND_QUEUE,
              qos_ctx->pd_tc_qos_map[qos_map_info->qos_map.tc]);
          if (status != SWITCH_STATUS_SUCCESS) {
            SWITCH_LOG_ERROR("qos map entry delete failed on device: %s",
                             switch_error_to_string(status));
            return status;
          }
          /* Global entry deleted. Reset the global state */
          qos_ctx->pd_tc_qos_map[qos_map_info->qos_map.tc] = 0;
          memset(&qos_ctx->tc_qos_map[qos_map_info->qos_map.tc],
                 0x0,
                 sizeof(switch_qos_map_t));
          qos_ctx->tc_queue_hdl[qos_map_info->qos_map.tc] =
              SWITCH_API_INVALID_HANDLE;
          qos_ctx->tc_icos_hdl[qos_map_info->qos_map.tc] =
              SWITCH_API_INVALID_HANDLE;
        } else {
          status = switch_pd_qos_map_ingress_entry_update(
              device,
              SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS_AND_QUEUE,
              qos_map_list_info->qos_group,
              &qos_ctx->tc_qos_map[qos_map_info->qos_map.tc],
              qos_ctx->pd_tc_qos_map[qos_map_info->qos_map.tc]);
          if (status != SWITCH_STATUS_SUCCESS) {
            SWITCH_LOG_ERROR("qos map entry update failed on device: %s",
                             switch_error_to_string(status));
            return status;
          }
        }
      } else {
        status = switch_pd_qos_map_ingress_entry_delete(
            device,
            qos_map_list_info->ingress_qos_map_type,
            qos_map_info->pd_hdl);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR("qos map ingress delete failed on device: %s",
                           switch_error_to_string(status));
          return status;
        }
      }
    } else {
      status = switch_pd_qos_map_egress_entry_delete(
          device, qos_map_list_info->egress_qos_map_type, qos_map_info->pd_hdl);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("qos map egress entry delete failed on device: %s",
                         switch_error_to_string(status));
        return status;
      }
    }

    status = SWITCH_LIST_DELETE(&qos_map_list_info->qos_map_list, node);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

    SWITCH_FREE(device, qos_map_info);
  }
  FOR_EACH_IN_LIST_END();

  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    switch (qos_map_list_info->ingress_qos_map_type) {
      case SWITCH_QOS_MAP_INGRESS_DSCP_TO_TC:
      case SWITCH_QOS_MAP_INGRESS_TOS_TO_TC:
      case SWITCH_QOS_MAP_INGRESS_PCP_TO_TC:
      case SWITCH_QOS_MAP_INGRESS_DSCP_TO_COLOR:
      case SWITCH_QOS_MAP_INGRESS_TOS_TO_COLOR:
      case SWITCH_QOS_MAP_INGRESS_PCP_TO_COLOR:
      case SWITCH_QOS_MAP_INGRESS_DSCP_TO_TC_AND_COLOR:
      case SWITCH_QOS_MAP_INGRESS_TOS_TO_TC_AND_COLOR:
      case SWITCH_QOS_MAP_INGRESS_PCP_TO_TC_AND_COLOR:
      case SWITCH_QOS_MAP_INGRESS_DSCP_TO_QID_AND_TC_AND_COLOR:
      case SWITCH_QOS_MAP_INGRESS_TOS_TO_QID_AND_TC_AND_COLOR:
      case SWITCH_QOS_MAP_INGRESS_PCP_TO_QID_AND_TC_AND_COLOR:
      case SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS:
      case SWITCH_QOS_MAP_INGRESS_TC_TO_QUEUE:
      case SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS_AND_QUEUE:
        if (!update) {
          /*
           * Don't release the qos_map_id for update case.
           * All the other bind points will still point to the same qos_map_id.
           */
          switch_api_id_allocator_release(device,
                                          qos_ctx->ingress_qos_map_id,
                                          qos_map_list_info->qos_group);
        }
        break;
      default:
        break;
    }
  } else {
    switch (qos_map_list_info->egress_qos_map_type) {
      case SWITCH_QOS_MAP_EGRESS_TC_TO_DSCP:
      case SWITCH_QOS_MAP_EGRESS_TC_TO_TOS:
      case SWITCH_QOS_MAP_EGRESS_TC_TO_PCP:
      case SWITCH_QOS_MAP_EGRESS_COLOR_TO_DSCP:
      case SWITCH_QOS_MAP_EGRESS_COLOR_TO_TOS:
      case SWITCH_QOS_MAP_EGRESS_COLOR_TO_PCP:
      case SWITCH_QOS_MAP_EGRESS_TC_AND_COLOR_TO_DSCP:
      case SWITCH_QOS_MAP_EGRESS_TC_AND_COLOR_TO_TOS:
      case SWITCH_QOS_MAP_EGRESS_TC_AND_COLOR_TO_PCP:
        if (!update) {
          switch_api_id_allocator_release(
              device, qos_ctx->egress_qos_map_id, qos_map_list_info->qos_group);
        }
        break;
      default:
        break;
    }
  }

  if (!update) {
    /*
     * Same handle will be used in case of update, so don't delete the handle.
     */
    status = switch_qos_map_handle_delete(device, qos_map_handle);
  }
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  return status;
cleanup:
  return status;
}

switch_status_t switch_qos_map_update(switch_device_t device,
                                      switch_handle_t qos_map_handle,
                                      switch_qos_map_t *qos_map) {
  switch_qos_map_list_t *qos_map_list_info = NULL;
  switch_direction_t direction;
  switch_qos_map_ingress_t ingress_qos_map_type;
  switch_qos_map_egress_t egress_qos_map_type;
  switch_uint32_t num_entries;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_qos_map_get(device, qos_map_handle, &qos_map_list_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "qos map create faield on device %d: "
        "qos map get failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }

  direction = qos_map_list_info->direction;
  ingress_qos_map_type = qos_map_list_info->ingress_qos_map_type;
  egress_qos_map_type = qos_map_list_info->egress_qos_map_type;
  num_entries = qos_map_list_info->num_entries;

  /*
   * For qosmap update, delete the qosmap and create the qosmap.
   * Update uses the same handle and the same qos-group index.
   */
  status = switch_qos_map_delete(device, direction, qos_map_handle, true);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to update the qosmap on device %d for handle 0x%lx:"
        "qosmap delete failed: %s",
        device,
        qos_map_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_qos_map_create(device,
                                 direction,
                                 ingress_qos_map_type,
                                 egress_qos_map_type,
                                 num_entries,
                                 qos_map,
                                 &qos_map_handle,
                                 true);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to update qosmap on device %d for handle 0x%lx:"
        "qosmap create failed: %s",
        device,
        qos_map_handle,
        switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_api_qos_map_set_internal(switch_device_t device,
                                                switch_handle_t qos_map_handle,
                                                switch_qos_map_t *qos_map) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_qos_map_update(device, qos_map_handle, qos_map);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "Failed to update qosmap on device %d for handle 0x%lx"
        "qosmap update failed: %s",
        device,
        qos_map_handle,
        switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_api_qos_map_ingress_create_internal(
    switch_device_t device,
    switch_qos_map_ingress_t qos_map_type,
    switch_uint8_t num_entries,
    switch_qos_map_t *qos_map,
    switch_handle_t *qos_map_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_qos_map_create(device,
                                 SWITCH_API_DIRECTION_INGRESS,
                                 qos_map_type,
                                 SWITCH_QOS_MAP_EGRESS_NONE,
                                 num_entries,
                                 qos_map,
                                 qos_map_handle,
                                 false);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("qos map ingress create faield on device: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_qos_map_ingress_delete_internal(
    switch_device_t device, switch_handle_t qos_map_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_qos_map_delete(
      device, SWITCH_API_DIRECTION_INGRESS, qos_map_handle, false);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("qos map ingress delete faield on device: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_qos_map_egress_create_internal(
    switch_device_t device,
    switch_qos_map_egress_t qos_map_type,
    switch_uint8_t num_entries,
    switch_qos_map_t *qos_map,
    switch_handle_t *qos_map_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_qos_map_create(device,
                                 SWITCH_API_DIRECTION_EGRESS,
                                 SWITCH_QOS_MAP_INGRESS_NONE,
                                 qos_map_type,
                                 num_entries,
                                 qos_map,
                                 qos_map_handle,
                                 false);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("qos map egress create faield on device: %s",
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_qos_map_egress_delete_internal(
    switch_device_t device, switch_handle_t qos_map_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_qos_map_delete(
      device, SWITCH_API_DIRECTION_EGRESS, qos_map_handle, false);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("qos map egress delete faield on device: %s",
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_qos_map_type_get_internal(
    switch_device_t device,
    switch_handle_t qos_map_handle,
    switch_direction_t *direction,
    switch_qos_map_ingress_t *ig_map_type,
    switch_qos_map_egress_t *eg_map_type) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_qos_map_list_t *qos_map_list_info = NULL;

  status = switch_qos_map_get(device, qos_map_handle, &qos_map_list_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "qos map type get failed on device %d: "
        "qos map get failed(%s)",
        device,
        switch_error_to_string(status));
    return status;
  }
  if (qos_map_list_info->direction == SWITCH_API_DIRECTION_INGRESS) {
    *ig_map_type = qos_map_list_info->ingress_qos_map_type;
    *direction = SWITCH_API_DIRECTION_INGRESS;
  } else {
    *eg_map_type = qos_map_list_info->egress_qos_map_type;
    *direction = SWITCH_API_DIRECTION_EGRESS;
  }
  return status;
}

switch_status_t switch_api_qos_map_list_get_internal(
    switch_device_t device,
    switch_handle_t qos_map_handle,
    switch_qos_map_t **qos_map,
    switch_uint32_t *num_entries) {
  switch_qos_map_list_t *qos_map_list = NULL;
  switch_qos_map_info_t *qos_map_info = NULL;
  switch_qos_map_t *qos_map_entries;
  switch_node_t *node = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_uint32_t i = 0;

  SWITCH_ASSERT(SWITCH_QOS_MAP_HANDLE(qos_map_handle));
  status = switch_qos_map_get(device, qos_map_handle, &qos_map_list);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("qos map list get faield on device %d: %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  *qos_map = (switch_qos_map_t *)SWITCH_MALLOC(
      device, sizeof(switch_qos_map_t), qos_map_list->qos_map_list.num_entries);
  qos_map_entries = (*qos_map);
  FOR_EACH_IN_LIST(qos_map_list->qos_map_list, node) {
    qos_map_info = (switch_qos_map_info_t *)node->data;
    SWITCH_MEMCPY(&qos_map_entries[i++],
                  &qos_map_info->qos_map,
                  sizeof(switch_qos_map_t));
  }
  FOR_EACH_IN_LIST_END();
  *num_entries = qos_map_list->qos_map_list.num_entries;
  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_qos_map_egress_create(
    switch_device_t device,
    switch_qos_map_egress_t map_type,
    switch_uint8_t num_entries,
    switch_qos_map_t *qos_map,
    switch_handle_t *qos_map_handle) {
  SWITCH_MT_WRAP(switch_api_qos_map_egress_create_internal(
      device, map_type, num_entries, qos_map, qos_map_handle))
}

switch_status_t switch_api_qos_map_ingress_delete(
    switch_device_t device, switch_handle_t qos_map_handle) {
  SWITCH_MT_WRAP(
      switch_api_qos_map_ingress_delete_internal(device, qos_map_handle))
}

switch_status_t switch_api_qos_map_egress_delete(
    switch_device_t device, switch_handle_t qos_map_handle) {
  SWITCH_MT_WRAP(
      switch_api_qos_map_egress_delete_internal(device, qos_map_handle))
}

switch_status_t switch_api_qos_map_ingress_create(
    switch_device_t device,
    switch_qos_map_ingress_t map_type,
    switch_uint8_t num_entries,
    switch_qos_map_t *qos_map,
    switch_handle_t *qos_map_handle) {
  SWITCH_MT_WRAP(switch_api_qos_map_ingress_create_internal(
      device, map_type, num_entries, qos_map, qos_map_handle))
}

switch_status_t switch_api_qos_map_type_get(
    switch_device_t device,
    switch_handle_t qos_map_handle,
    switch_direction_t *dir,
    switch_qos_map_ingress_t *ig_map_type,
    switch_qos_map_egress_t *eg_map_type) {
  SWITCH_MT_WRAP(switch_api_qos_map_type_get_internal(
      device, qos_map_handle, dir, ig_map_type, eg_map_type))
}

switch_status_t switch_api_qos_map_list_get(switch_device_t device,
                                            switch_handle_t qos_map_handle,
                                            switch_qos_map_t **qos_map_list,
                                            switch_uint32_t *num_entries) {
  SWITCH_MT_WRAP(switch_api_qos_map_list_get_internal(
      device, qos_map_handle, qos_map_list, num_entries))
}

switch_status_t switch_api_qos_map_set(switch_device_t device,
                                       switch_handle_t qos_map_handle,
                                       switch_qos_map_t *qos_map) {
  SWITCH_MT_WRAP(
      switch_api_qos_map_set_internal(device, qos_map_handle, qos_map))
}
