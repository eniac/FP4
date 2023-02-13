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

#include <saiqosmap.h>
#include "saiinternal.h"
#include <switchapi/switch_qos.h>

static sai_api_t api_id = SAI_API_QOS_MAP;

static sai_status_t sai_qos_map_type_to_switch_qos_map_type(
    sai_qos_map_type_t qos_map_type,
    switch_direction_t *direction,
    switch_qos_map_ingress_t *ingress_qos_map_type,
    switch_qos_map_egress_t *egress_qos_map_type) {
  sai_status_t status = SAI_STATUS_SUCCESS;

  *ingress_qos_map_type = SWITCH_QOS_MAP_INGRESS_NONE;
  *egress_qos_map_type = SWITCH_QOS_MAP_EGRESS_NONE;

  switch (qos_map_type) {
    case SAI_QOS_MAP_TYPE_DOT1P_TO_TC:
      *ingress_qos_map_type = SWITCH_QOS_MAP_INGRESS_PCP_TO_TC;
      *direction = SWITCH_API_DIRECTION_INGRESS;
      break;
    case SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR:
      *ingress_qos_map_type = SWITCH_QOS_MAP_INGRESS_PCP_TO_COLOR;
      *direction = SWITCH_API_DIRECTION_INGRESS;
      break;
    case SAI_QOS_MAP_TYPE_DSCP_TO_TC:
      *ingress_qos_map_type = SWITCH_QOS_MAP_INGRESS_DSCP_TO_TC;
      *direction = SWITCH_API_DIRECTION_INGRESS;
      break;
    case SAI_QOS_MAP_TYPE_DSCP_TO_COLOR:
      *ingress_qos_map_type = SWITCH_QOS_MAP_INGRESS_DSCP_TO_COLOR;
      *direction = SWITCH_API_DIRECTION_INGRESS;
      break;
    case SAI_QOS_MAP_TYPE_TC_TO_QUEUE:
      *ingress_qos_map_type = SWITCH_QOS_MAP_INGRESS_TC_TO_QUEUE;
      *direction = SWITCH_API_DIRECTION_INGRESS;
      break;
    case SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DSCP:
      *egress_qos_map_type = SWITCH_QOS_MAP_EGRESS_TC_AND_COLOR_TO_DSCP;
      *direction = SWITCH_API_DIRECTION_EGRESS;
      break;
    case SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DOT1P:
      *egress_qos_map_type = SWITCH_QOS_MAP_EGRESS_TC_AND_COLOR_TO_PCP;
      *direction = SWITCH_API_DIRECTION_EGRESS;
      break;
    case SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_PRIORITY_GROUP:
      *ingress_qos_map_type = SWITCH_QOS_MAP_INGRESS_PFC_PRIORITY_TO_PPG;
      *direction = SWITCH_API_DIRECTION_INGRESS;
      break;

    case SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_QUEUE:
      *direction = SWITCH_API_DIRECTION_EGRESS;
      *egress_qos_map_type = SWITCH_QOS_MAP_EGRESS_PFC_PRIORITY_TO_QUEUE;
      break;

    case SAI_QOS_MAP_TYPE_TC_TO_PRIORITY_GROUP:
      *ingress_qos_map_type = SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS;
      *direction = SWITCH_API_DIRECTION_INGRESS;
      break;
    default:
      status = SAI_STATUS_NOT_SUPPORTED;
      break;
  }

  return status;
}

sai_status_t switch_qos_map_type_to_sai_qos_map_type(
    switch_direction_t dir,
    switch_qos_map_ingress_t map_ingress,
    switch_qos_map_egress_t map_egress,
    sai_qos_map_type_t *sai_qos_map) {
  switch_status_t status = SAI_STATUS_SUCCESS;

  if (dir == SWITCH_API_DIRECTION_INGRESS) {
    switch (map_ingress) {
      case SWITCH_QOS_MAP_INGRESS_PCP_TO_TC:
        *sai_qos_map = SAI_QOS_MAP_TYPE_DOT1P_TO_TC;
        break;
      case SWITCH_QOS_MAP_INGRESS_PCP_TO_COLOR:
        *sai_qos_map = SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR;
        break;
      case SWITCH_QOS_MAP_INGRESS_DSCP_TO_TC:
        *sai_qos_map = SAI_QOS_MAP_TYPE_DSCP_TO_TC;
        break;
      case SWITCH_QOS_MAP_INGRESS_DSCP_TO_COLOR:
        *sai_qos_map = SAI_QOS_MAP_TYPE_DSCP_TO_COLOR;
        break;
      case SWITCH_QOS_MAP_INGRESS_TC_TO_QUEUE:
        *sai_qos_map = SAI_QOS_MAP_TYPE_TC_TO_QUEUE;
        break;
      case SWITCH_QOS_MAP_INGRESS_PFC_PRIORITY_TO_PPG:
        *sai_qos_map = SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_PRIORITY_GROUP;
        break;
      case SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS:
        *sai_qos_map = SAI_QOS_MAP_TYPE_TC_TO_PRIORITY_GROUP;
        break;
      default:
        status = SAI_STATUS_NOT_SUPPORTED;
        break;
    }
  } else {
    switch (map_egress) {
      case SWITCH_QOS_MAP_EGRESS_TC_AND_COLOR_TO_DSCP:
        *sai_qos_map = SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DSCP;
        break;
      case SWITCH_QOS_MAP_EGRESS_TC_AND_COLOR_TO_PCP:
        *sai_qos_map = SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DOT1P;
        break;
      default:
        status = SAI_STATUS_NOT_SUPPORTED;
    }
  }
  return status;
}

static switch_color_t sai_color_to_switch_color(sai_packet_color_t color) {
  switch (color) {
    case SAI_PACKET_COLOR_GREEN:
      return SWITCH_COLOR_GREEN;
    case SAI_PACKET_COLOR_YELLOW:
      return SWITCH_COLOR_YELLOW;
    case SAI_PACKET_COLOR_RED:
      return SWITCH_COLOR_RED;
    default:
      return SWITCH_COLOR_GREEN;
  }
}

sai_packet_color_t switch_color_to_sai_color(switch_color_t switch_color) {
  switch (switch_color) {
    case SWITCH_COLOR_GREEN:
      return SAI_PACKET_COLOR_GREEN;
    case SWITCH_COLOR_YELLOW:
      return SAI_PACKET_COLOR_YELLOW;
    case SWITCH_COLOR_RED:
      return SAI_PACKET_COLOR_RED;
    default:
      return SAI_PACKET_COLOR_GREEN;
  }
}

static void sai_qos_map_to_switch_qos_map(sai_qos_map_type_t qos_map_type,
                                          sai_qos_map_t *qos_map,
                                          switch_qos_map_t *switch_qos_map) {
  switch (qos_map_type) {
    case SAI_QOS_MAP_TYPE_DOT1P_TO_TC:
      switch_qos_map->pcp = qos_map->key.dot1p;
      switch_qos_map->tc = qos_map->value.tc;
      break;
    case SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR:
      switch_qos_map->pcp = qos_map->key.dot1p;
      switch_qos_map->color = sai_color_to_switch_color(qos_map->value.color);
      break;
    case SAI_QOS_MAP_TYPE_DSCP_TO_TC:
      switch_qos_map->dscp = qos_map->key.dscp;
      switch_qos_map->tc = qos_map->value.tc;
      break;
    case SAI_QOS_MAP_TYPE_DSCP_TO_COLOR:
      switch_qos_map->dscp = qos_map->key.dscp;
      switch_qos_map->color = sai_color_to_switch_color(qos_map->value.color);
      break;
    case SAI_QOS_MAP_TYPE_TC_TO_QUEUE:
      switch_qos_map->tc = qos_map->key.tc;
      switch_qos_map->qid = qos_map->value.queue_index;
      break;
    case SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DSCP:
      switch_qos_map->tc = qos_map->key.tc;
      switch_qos_map->color = sai_color_to_switch_color(qos_map->key.color);
      switch_qos_map->dscp = qos_map->value.dscp;
      break;
    case SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DOT1P:
      switch_qos_map->tc = qos_map->key.tc;
      switch_qos_map->color = sai_color_to_switch_color(qos_map->key.color);
      switch_qos_map->pcp = qos_map->value.dot1p;
      break;
    case SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_PRIORITY_GROUP:
      switch_qos_map->pfc_priority = qos_map->key.prio;
      switch_qos_map->ppg = qos_map->value.pg;
      break;
    case SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_QUEUE:
      switch_qos_map->pfc_priority = qos_map->key.prio;
      switch_qos_map->qid = qos_map->value.queue_index;
      break;
    case SAI_QOS_MAP_TYPE_TC_TO_PRIORITY_GROUP:
      switch_qos_map->tc = qos_map->key.tc;
      switch_qos_map->icos = qos_map->value.pg;
      break;
    default:
      break;
  }
}

sai_status_t switch_qos_map_to_sai_qos_map(switch_qos_map_t *switch_qos_map,
                                           sai_qos_map_type_t qos_map_type,
                                           sai_qos_map_t *sai_qos_map) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch (qos_map_type) {
    case SAI_QOS_MAP_TYPE_DOT1P_TO_TC:
      sai_qos_map->key.dot1p = switch_qos_map->pcp;
      sai_qos_map->value.tc = switch_qos_map->tc;
      break;
    case SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR:
      sai_qos_map->key.dot1p = switch_qos_map->pcp;
      sai_qos_map->value.color =
          switch_color_to_sai_color(switch_qos_map->color);
      break;
    case SAI_QOS_MAP_TYPE_DSCP_TO_TC:
      sai_qos_map->key.dscp = switch_qos_map->dscp;
      sai_qos_map->value.tc = switch_qos_map->tc;
      break;
    case SAI_QOS_MAP_TYPE_DSCP_TO_COLOR:
      sai_qos_map->key.dscp = switch_qos_map->dscp;
      sai_qos_map->value.color =
          switch_color_to_sai_color(switch_qos_map->color);
      break;
    case SAI_QOS_MAP_TYPE_TC_TO_QUEUE:
      sai_qos_map->key.tc = switch_qos_map->tc;
      sai_qos_map->value.queue_index = switch_qos_map->qid;
      break;
    case SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DSCP:
      sai_qos_map->key.tc = switch_qos_map->tc;
      sai_qos_map->key.color = switch_color_to_sai_color(switch_qos_map->color);
      sai_qos_map->value.dscp = switch_qos_map->dscp;
      break;
    case SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DOT1P:
      sai_qos_map->key.tc = switch_qos_map->tc;
      sai_qos_map->key.color = switch_color_to_sai_color(switch_qos_map->color);
      sai_qos_map->value.dot1p = switch_qos_map->pcp;
      break;
    case SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_PRIORITY_GROUP:
      sai_qos_map->key.prio = switch_qos_map->pfc_priority;
      sai_qos_map->value.pg = switch_qos_map->ppg;
      break;
    case SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_QUEUE:
      sai_qos_map->key.prio = switch_qos_map->pfc_priority;
      sai_qos_map->value.queue_index = switch_qos_map->qid;
      break;
    case SAI_QOS_MAP_TYPE_TC_TO_PRIORITY_GROUP:
      sai_qos_map->key.tc = switch_qos_map->tc;
      sai_qos_map->value.pg = switch_qos_map->icos;
      break;
    default:
      status = SAI_STATUS_NOT_SUPPORTED;
      break;
  }
  return status;
}

static sai_status_t sai_qos_map_attribute_parse(
    _In_ uint32_t attr_count,
    _In_ const sai_attribute_t *attr_list,
    _Out_ switch_direction_t *direction,
    _Out_ switch_qos_map_ingress_t *ingress_qos_map_type,
    _Out_ switch_qos_map_egress_t *egress_qos_map_type,
    _Out_ uint32_t *num_entries,
    _Out_ switch_qos_map_t **switch_qos_map_list) {
  const sai_attribute_t *attribute;
  uint32_t i = 0, j = 0;
  sai_status_t status = SAI_STATUS_SUCCESS;
  sai_qos_map_t *qos_map = NULL;
  switch_qos_map_t *switch_qos_map = NULL;
  sai_qos_map_type_t qos_map_type = 0;

  for (i = 0; i < attr_count; i++) {
    attribute = &attr_list[i];
    switch (attribute->id) {
      case SAI_QOS_MAP_ATTR_TYPE:
        status = sai_qos_map_type_to_switch_qos_map_type(attribute->value.u32,
                                                         direction,
                                                         ingress_qos_map_type,
                                                         egress_qos_map_type);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR("qos map attribute parse failed %s",
                        sai_status_to_string(status));
          return status;
        }
        qos_map_type = attribute->value.u32;
        break;
      case SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST:
        *num_entries = attribute->value.qosmap.count;
        *switch_qos_map_list =
            SAI_MALLOC(sizeof(switch_qos_map_t) * (*num_entries));
        if (!(*switch_qos_map_list)) {
          status = SAI_STATUS_NO_MEMORY;
          SAI_LOG_ERROR("memory allocation failed for qos map %s",
                        sai_status_to_string(status));
          return status;
        }

        memset(*switch_qos_map_list,
               0x0,
               sizeof(switch_qos_map_t) * (*num_entries));
        for (j = 0; j < (*num_entries); j++) {
          qos_map = &attribute->value.qosmap.list[j];
          switch_qos_map = &(*switch_qos_map_list)[j];
          sai_qos_map_to_switch_qos_map(qos_map_type, qos_map, switch_qos_map);
        }
        break;
      default:
        break;
    }
  }

  return status;
}

/**
 * @brief Create Qos Map
 *
 * @param[out] qos_map_id Qos Map Id
 * @param[in] switch_id Switch id
 * @param[in] attr_count number of attributes
 * @param[in] attr_list array of attributes
 *
 * @return  SAI_STATUS_SUCCESS on success
 *          Failure status code on error
 */
sai_status_t sai_create_qos_map(_Out_ sai_object_id_t *qos_map_id,
                                _In_ sai_object_id_t switch_id,
                                _In_ uint32_t attr_count,
                                _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  uint32_t num_entries = 0;
  switch_qos_map_t *switch_qos_map_list = NULL;
  switch_direction_t direction = 0;
  switch_qos_map_ingress_t ingress_qos_map_type = 0;
  switch_qos_map_egress_t egress_qos_map_type = 0;
  switch_handle_t qos_map_handle = SWITCH_API_INVALID_HANDLE;

  *qos_map_id = SAI_NULL_OBJECT_ID;

  status = sai_qos_map_attribute_parse(attr_count,
                                       attr_list,
                                       &direction,
                                       &ingress_qos_map_type,
                                       &egress_qos_map_type,
                                       &num_entries,
                                       &switch_qos_map_list);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("qos map attribute parse failed %s",
                  sai_status_to_string(status));
    return status;
  }

  if (direction == SWITCH_API_DIRECTION_INGRESS) {
    status = switch_api_qos_map_ingress_create(device,
                                               ingress_qos_map_type,
                                               num_entries,
                                               switch_qos_map_list,
                                               &qos_map_handle);
  } else {
    status = switch_api_qos_map_egress_create(device,
                                              egress_qos_map_type,
                                              num_entries,
                                              switch_qos_map_list,
                                              &qos_map_handle);
  }

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to create qos map group: %s",
                  sai_status_to_string(status));
    if (switch_qos_map_list) {
      free(switch_qos_map_list);
    }
  }

  *qos_map_id = qos_map_handle;
  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * @brief Remove Qos Map
 *
 *  @param[in] qos_map_id Qos Map id to be removed.
 *
 *  @return  SAI_STATUS_SUCCESS on success
 *           Failure status code on error
 */
sai_status_t sai_remove_qos_map(_In_ sai_object_id_t qos_map_id) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_direction_t dir;
  switch_qos_map_ingress_t map_ingress;
  switch_qos_map_egress_t map_egress;

  SAI_ASSERT(sai_object_type_query(qos_map_id) == SAI_OBJECT_TYPE_QOS_MAP);

  switch_status = switch_api_qos_map_type_get(
      device, qos_map_id, &dir, &map_ingress, &map_egress);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to get qosmap type for handle %lx: %s",
                  qos_map_id,
                  sai_status_to_string(status));
    return status;
  }

  if (dir == SWITCH_API_DIRECTION_INGRESS) {
    status = switch_api_qos_map_ingress_delete(device, qos_map_id);
    if (status != SWITCH_STATUS_SUCCESS &&
        status != SWITCH_STATUS_INVALID_HANDLE) {
      SAI_LOG_ERROR("failed to remove ingress qos map %s",
                    sai_status_to_string(status));
      return status;
    }
  } else {
    status = switch_api_qos_map_egress_delete(device, qos_map_id);
    if (status != SWITCH_STATUS_SUCCESS) {
      SAI_LOG_ERROR("failed to remove egress qos map %s",
                    sai_status_to_string(status));
      return status;
    }
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

static void sai_qos_update_qosmap_entries(sai_qos_map_type_t qos_map_type,
                                          sai_qos_map_t *sai_qos_map,
                                          sai_qos_map_t *new_qos_map,
                                          switch_uint32_t update_entries) {
  switch_uint32_t i;

  for (i = 0; i < update_entries; i++) {
    switch (qos_map_type) {
      case SAI_QOS_MAP_TYPE_DOT1P_TO_TC:
        if (sai_qos_map->key.dot1p == new_qos_map[i].key.dot1p) {
          sai_qos_map->value.tc = new_qos_map[i].value.tc;
        }
        break;
      case SAI_QOS_MAP_TYPE_DOT1P_TO_COLOR:
        if (sai_qos_map->key.dot1p == new_qos_map[i].key.dot1p) {
          sai_qos_map->value.color = new_qos_map[i].value.color;
        }
        break;
      case SAI_QOS_MAP_TYPE_DSCP_TO_TC:
        if (sai_qos_map->key.dscp == new_qos_map[i].key.dscp) {
          sai_qos_map->value.tc = new_qos_map[i].value.tc;
        }
        break;
      case SAI_QOS_MAP_TYPE_DSCP_TO_COLOR:
        if (sai_qos_map->key.dscp == new_qos_map[i].key.dscp) {
          sai_qos_map->value.color = new_qos_map[i].value.color;
        }
        break;
      case SAI_QOS_MAP_TYPE_TC_TO_QUEUE:
        if (sai_qos_map->key.tc == new_qos_map[i].key.tc) {
          sai_qos_map->value.queue_index = new_qos_map[i].value.queue_index;
        }
        break;
      case SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DSCP:
        if (sai_qos_map->key.tc == new_qos_map[i].key.tc &&
            sai_qos_map->key.color == new_qos_map[i].key.color) {
          sai_qos_map->value.dscp = new_qos_map[i].value.dscp;
        }
        break;
      case SAI_QOS_MAP_TYPE_TC_AND_COLOR_TO_DOT1P:
        if (sai_qos_map->key.tc == new_qos_map[i].key.tc &&
            sai_qos_map->key.color == new_qos_map[i].key.color) {
          sai_qos_map->value.dot1p = new_qos_map[i].value.dot1p;
        }
        break;
      case SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_PRIORITY_GROUP:
        if (sai_qos_map->key.prio == new_qos_map[i].key.prio) {
          sai_qos_map->value.pg = new_qos_map[i].value.pg;
        }
        break;
      case SAI_QOS_MAP_TYPE_PFC_PRIORITY_TO_QUEUE:
        if (sai_qos_map->key.prio == new_qos_map[i].key.prio) {
          sai_qos_map->value.queue_index = new_qos_map[i].value.queue_index;
        }
        break;
      default:
        break;
    }
  }
}

/**
 * @brief Set attributes for qos map
 *
 * @param[in] qos_map_id Qos Map Id
 * @param[in] attr attribute to set
 *
 * @return  SAI_STATUS_SUCCESS on success
 *          Failure status code on error
 */

sai_status_t sai_set_qos_map_attribute(_In_ sai_object_id_t qos_map_id,
                                       _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_direction_t dir;
  switch_qos_map_ingress_t map_ingress;
  switch_qos_map_t *switch_qos_map = NULL;
  switch_qos_map_egress_t map_egress;
  sai_qos_map_type_t sai_qos_type;
  switch_uint32_t sai_update_entries = 0;
  sai_qos_map_t *sai_qos_map;
  switch_uint32_t index = 0;
  switch_qos_map_t *qos_map_list = NULL;
  switch_uint32_t i = 0, num_entries = 0;

  SAI_ASSERT(sai_object_type_query(qos_map_id) == SAI_OBJECT_TYPE_QOS_MAP);

  switch_status = switch_api_qos_map_type_get(
      device, qos_map_id, &dir, &map_ingress, &map_egress);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to get qosmap type for handle %lx: %s",
                  qos_map_id,
                  sai_status_to_string(status));
    return status;
  }

  /*
   * Get all the old qosmap entries and update the new entries.
   * before passing to switchapi.
   */
  switch_status = switch_api_qos_map_list_get(
      device, qos_map_id, &qos_map_list, &num_entries);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to get qosmap list for handle %lx: %s",
                  qos_map_id,
                  sai_status_to_string(status));
    if (qos_map_list) {
      free(qos_map_list);
    }
    return status;
  }

  status = switch_qos_map_type_to_sai_qos_map_type(
      dir, map_ingress, map_egress, &sai_qos_type);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR(
        "Failed to map switch qosmap type to sai type for handle %lx: %s",
        qos_map_id,
        sai_status_to_string(status));
    return status;
  }

  sai_qos_map =
      (sai_qos_map_t *)SAI_MALLOC(sizeof(sai_qos_map_t) * num_entries);
  for (i = 0; i < num_entries; i++) {
    status = switch_qos_map_to_sai_qos_map(
        &qos_map_list[i], sai_qos_type, &sai_qos_map[i]);
  }
  if (qos_map_list) {
    free(qos_map_list);
  }

  sai_update_entries = attr->value.qosmap.count;

  for (i = 0; i < num_entries; i++) {
    sai_qos_update_qosmap_entries(sai_qos_type,
                                  &sai_qos_map[i],
                                  attr->value.qosmap.list,
                                  sai_update_entries);
  }

  switch_qos_map = SAI_MALLOC(sizeof(switch_qos_map_t) * num_entries);
  memset(switch_qos_map, 0, sizeof(switch_qos_map_t) * num_entries);

  for (index = 0; index < num_entries; index++) {
    sai_qos_map_to_switch_qos_map(
        sai_qos_type, &sai_qos_map[index], &switch_qos_map[index]);
  }

  switch_status = switch_api_qos_map_set(device, qos_map_id, switch_qos_map);
  status = sai_switch_status_to_sai_status(switch_status);
  if (switch_qos_map) {
    free(switch_qos_map);
  }

  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("Failed to update qosmap for handle 0x%lx: %s",
                  qos_map_id,
                  sai_status_to_string(status));
  }

  if (sai_qos_map) {
    free(sai_qos_map);
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * @brief  Get attrbutes of qos map
 *
 * @param[in] qos_map_id  map id
 * @param[in] attr_count  number of attributes
 * @param[inout] attr_list  array of attributes
 *
 * @return SAI_STATUS_SUCCESS on success
 *        Failure status code on error
 */

sai_status_t sai_get_qos_map_attribute(_In_ sai_object_id_t qos_map_id,
                                       _In_ uint32_t attr_count,
                                       _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_attribute_t *attr = attr_list;
  switch_direction_t dir;
  switch_qos_map_ingress_t map_ingress;
  switch_qos_map_egress_t map_egress;
  switch_uint32_t num_entries = 0;
  switch_qos_map_t *qos_map_list = NULL;
  sai_qos_map_type_t sai_qos_type;
  unsigned int attr_index = 0;
  int i = 0;
  int j = 0;

  SAI_ASSERT(sai_object_type_query(qos_map_id) == SAI_OBJECT_TYPE_QOS_MAP);

  switch_status = switch_api_qos_map_type_get(
      device, qos_map_id, &dir, &map_ingress, &map_egress);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to get qosmap type for handle %lx: %s",
                  qos_map_id,
                  sai_status_to_string(status));
    return status;
  }
  status = switch_qos_map_type_to_sai_qos_map_type(
      dir, map_ingress, map_egress, &sai_qos_type);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR(
        "Failed to map switch qosmap type to sai type for handle %lx: %s",
        qos_map_id,
        sai_status_to_string(status));
    return status;
  }

  for (attr_index = 0, attr = attr_list; attr_index < attr_count;
       attr_index++, attr++) {
    switch (attr->id) {
      case SAI_QOS_MAP_ATTR_TYPE:
        attr->value.u32 = sai_qos_type;
        break;

      case SAI_QOS_MAP_ATTR_MAP_TO_VALUE_LIST:
        switch_status = switch_api_qos_map_list_get(
            device, qos_map_id, &qos_map_list, &num_entries);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SAI_STATUS_SUCCESS) {
          SAI_LOG_ERROR("failed to get qosmap list for handle %lx: %s",
                        qos_map_id,
                        sai_status_to_string(status));
          if (qos_map_list) {
            free(qos_map_list);
          }
          return status;
        }
        attr->value.qosmap.count = num_entries;
        for (i = (num_entries - 1); i >= 0; i--) {
          status = switch_qos_map_to_sai_qos_map(
              &qos_map_list[i], sai_qos_type, &attr->value.qosmap.list[j++]);
        }
        free(qos_map_list);
        break;
      default:
        status = SAI_STATUS_NOT_SUPPORTED;
        break;
    }
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
*  Qos maps methods table retrieved with sai_api_query()
*/
sai_qos_map_api_t qos_api = {
    .create_qos_map = sai_create_qos_map,
    .remove_qos_map = sai_remove_qos_map,
    .set_qos_map_attribute = sai_set_qos_map_attribute,
    .get_qos_map_attribute = sai_get_qos_map_attribute};

sai_status_t sai_qos_map_initialize(sai_api_service_t *sai_api_service) {
  SAI_LOG_DEBUG("Initializing qos map");
  sai_api_service->qos_api = qos_api;
  return SAI_STATUS_SUCCESS;
}
