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

#include <saivlan.h>
#include "saiinternal.h"
#include <switchapi/switch.h>
#include <switchapi/switch_vlan.h>
#include <switchapi/switch_handle.h>
#include <switchapi/switch_hostif.h>

static sai_api_t api_id = SAI_API_VLAN;

static sai_status_t get_vlan_id(_Out_ switch_vlan_t *vlan_id,
                                _In_ uint32_t attr_count,
                                _In_ const sai_attribute_t *attr_list) {
  unsigned index = 0;
  for (index = 0; index < attr_count; index++) {
    if (attr_list[index].id == SAI_VLAN_ATTR_VLAN_ID) {
      *vlan_id = attr_list[index].value.u16;
      return SAI_STATUS_SUCCESS;
    }
  }
  return SAI_STATUS_INVALID_PARAMETER;
}
/**
 * Routine Description:
 *     Create a VLAN
 *
 * Arguments:
 *     [out] vlan_id VLAN ID
 *     [in] switch_id Switch id
 *     [in] attr_count Number of attributes
 *     [in] attr_list Array of attributes
 *
 * Return Values:
 *     SAI_STATUS_SUCCESS on success Failure status code on error
 */
sai_status_t sai_create_vlan_entry(_Out_ sai_object_id_t *vlan_object_id,
                                   _In_ sai_object_id_t switch_id,
                                   _In_ uint32_t attr_count,
                                   _In_ const sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_vlan_t vlan_id = 0;
  switch_handle_t vlan_handle = SWITCH_API_INVALID_HANDLE;
  *vlan_object_id = SAI_NULL_OBJECT_ID;

  status = get_vlan_id(&vlan_id, attr_count, attr_list);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("Could not find vlan id parameter!\n");
    return status;
  }
  status = switch_api_vlan_create(device, (switch_vlan_t)vlan_id, &vlan_handle);
  if (status != SWITCH_STATUS_SUCCESS &&
      status != SWITCH_STATUS_ITEM_ALREADY_EXISTS) {
    SAI_LOG_ERROR(
        "failed to create vlan %d: %s", vlan_id, sai_status_to_string(status));
  } else {
    /* enable IGMP and MLD snooping by default */
    switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
    switch_status =
        switch_api_vlan_igmp_snooping_set(device, vlan_handle, true);
    assert(switch_status == SWITCH_STATUS_SUCCESS);
    switch_status = switch_api_vlan_mld_snooping_set(device, vlan_handle, true);
    assert(switch_status == SWITCH_STATUS_SUCCESS);
    status = sai_switch_status_to_sai_status(switch_status);
  }
  *vlan_object_id = vlan_handle;
  SAI_LOG_EXIT();
  return (sai_status_t)status;
}

/*
* Routine Description:
*    Remove a VLAN
*
* Arguments:
*    [in] sai_object_id_t vlan id - a handle
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_remove_vlan_entry(_In_ sai_object_id_t vlan_id) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_status = switch_api_vlan_delete(device, vlan_id);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to remove vlan handle %lx: %s",
                  vlan_id,
                  sai_status_to_string(status));
    return status;
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
* Routine Description:
*    Set VLAN attribute Value
*
* Arguments:
*    [in] vlan_id - VLAN id
*    [in] attr - attribute
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_set_vlan_entry_attribute(_In_ sai_object_id_t vlan_id,
                                          _In_ const sai_attribute_t *attr) {
  SAI_LOG_ENTER();

  switch_handle_t vlan_handle = 0;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_handle_t acl_table_id;
  switch_api_vlan_info_t vlan_info;
  switch_uint64_t flags = 0;

  if (!attr) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute: %s", sai_status_to_string(status));
    return status;
  }

  switch_status = switch_api_vlan_id_to_handle_get(
      device, (switch_vlan_t)vlan_id, &vlan_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("set vlan entry attribute %d: %s",
                  vlan_id,
                  sai_status_to_string(status));
    return status;
  }

  switch (attr->id) {
    case SAI_VLAN_ATTR_STP_INSTANCE:
      switch_status =
          switch_api_vlan_stp_handle_set(device, vlan_handle, attr->value.oid);
      status = sai_switch_status_to_sai_status(switch_status);
      if (status != SAI_STATUS_SUCCESS) {
        SAI_LOG_ERROR("set vlan entry attribute %d: %s",
                      vlan_id,
                      sai_status_to_string(status));
        return status;
      }
      break;
    case SAI_VLAN_ATTR_INGRESS_ACL:
    case SAI_VLAN_ATTR_EGRESS_ACL:
      acl_table_id = (switch_handle_t)attr->value.oid;
      if (acl_table_id == SAI_NULL_OBJECT_ID) {
        if (attr->id == SAI_VLAN_ATTR_INGRESS_ACL) {
          switch_status = switch_api_vlan_ingress_acl_group_get(
              device, vlan_id, &acl_table_id);
          if (status != SAI_STATUS_SUCCESS) {
            SAI_LOG_ERROR("failed to get ingress acl_handle for vlan_id %d: %s",
                          (vlan_id & 0xFFFF),
                          sai_status_to_string(status));
            return status;
          }
        } else {
          switch_status = switch_api_vlan_egress_acl_group_get(
              device, vlan_id, &acl_table_id);
          if (status != SAI_STATUS_SUCCESS) {
            SAI_LOG_ERROR("failed to get egress acl_handle for vlan_id %d: %s",
                          (vlan_id & 0xFFFF),
                          sai_status_to_string(status));
            return status;
          }
        }
        switch_status =
            switch_api_acl_dereference(device, acl_table_id, vlan_id);
      } else {
        switch_status = switch_api_acl_reference(device, acl_table_id, vlan_id);
      }
      status = sai_switch_status_to_sai_status(switch_status);
      if (status != SAI_STATUS_SUCCESS) {
        SAI_LOG_ERROR("failed to bind vlan to acl for vlan %d: %s",
                      (vlan_id & 0xFFFF),
                      sai_status_to_string(status));
        return status;
      }
      break;
    case SAI_VLAN_ATTR_LEARN_DISABLE:
      flags |= SWITCH_VLAN_ATTR_LEARNING_ENABLED;
      vlan_info.learning_enabled = (attr->value.booldata ? FALSE : TRUE);
      switch_status =
          switch_api_vlan_attribute_set(device, vlan_id, flags, &vlan_info);
      status = sai_switch_status_to_sai_status(switch_status);
      if (status != SWITCH_STATUS_SUCCESS) {
        SAI_LOG_ERROR("Failed to get learn disable property for vlan %d: %s",
                      vlan_id,
                      sai_status_to_string(status));
        return status;
      }

    default:
      status = SAI_STATUS_NOT_SUPPORTED;
      break;
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
* Routine Description:
*    Get VLAN attribute Value
*
* Arguments:
*    [in] vlan_id - VLAN id
*    [in] attr_count - number of attributes
*    [inout] attr_list - array of attributes
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_get_vlan_entry_attribute(_In_ sai_object_id_t vlan_id,
                                          _In_ uint32_t attr_count,
                                          _Inout_ sai_attribute_t *attr_list) {
  SAI_LOG_ENTER();

  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  sai_status_t status = SAI_STATUS_SUCCESS;
  sai_attribute_t *attr = attr_list;
  switch_handle_t acl_handle;
  unsigned int i = 0;
  switch_handle_t stp_handle;
  switch_vlan_interface_t *vlan_intf_mbrs = NULL;
  switch_uint16_t vlan_mbr_count = 0;
  sai_object_list_t *objlist = NULL;
  switch_uint64_t flags = 0;
  switch_vlan_t switch_vlan = 0;
  switch_api_vlan_info_t vlan_info;

  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("null attribute list: %s", sai_status_to_string(status));
    return status;
  }

  for (i = 0, attr = attr_list; i < attr_count; i++, attr++) {
    switch (attr->id) {
      case SAI_VLAN_ATTR_INGRESS_ACL:
        switch_status =
            switch_api_vlan_ingress_acl_group_get(device, vlan_id, &acl_handle);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SWITCH_STATUS_SUCCESS) {
          return status;
        }
        attr->value.oid = (acl_handle == SWITCH_API_INVALID_HANDLE)
                              ? SAI_NULL_OBJECT_ID
                              : acl_handle;
        break;
      case SAI_VLAN_ATTR_EGRESS_ACL:
        switch_status =
            switch_api_vlan_egress_acl_group_get(device, vlan_id, &acl_handle);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SWITCH_STATUS_SUCCESS) {
          return status;
        }
        attr->value.oid = (acl_handle == SWITCH_API_INVALID_HANDLE)
                              ? SAI_NULL_OBJECT_ID
                              : acl_handle;
        break;
      case SAI_VLAN_ATTR_STP_INSTANCE:
        switch_status =
            switch_api_vlan_stp_handle_get(device, vlan_id, &stp_handle);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SWITCH_STATUS_SUCCESS) {
          return status;
        }
        attr->value.oid = stp_handle;
        break;

      case SAI_VLAN_ATTR_MEMBER_LIST:
        switch_status = switch_api_vlan_interfaces_get(
            device, vlan_id, &vlan_mbr_count, &vlan_intf_mbrs);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SWITCH_STATUS_SUCCESS) {
          SAI_LOG_ERROR("Failed to get list of vlan members for vlan %d: %s",
                        vlan_id,
                        sai_status_to_string(status));
          return status;
        }
        objlist = &attr->value.objlist;
        objlist->count = vlan_mbr_count;
        for (i = 0; i < vlan_mbr_count; i++) {
          objlist->list[i] = vlan_intf_mbrs[i].member_handle;
        }
        SAI_FREE(vlan_intf_mbrs);
        break;

      case SAI_VLAN_ATTR_LEARN_DISABLE:
        flags |= SWITCH_VLAN_ATTR_LEARNING_ENABLED;
        switch_status =
            switch_api_vlan_attribute_get(device, vlan_id, flags, &vlan_info);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SWITCH_STATUS_SUCCESS) {
          SAI_LOG_ERROR("Failed to get learn disable property for vlan %d: %s",
                        vlan_id,
                        sai_status_to_string(status));
          return status;
        }
        attr->value.booldata = (vlan_info.learning_enabled ? FALSE : TRUE);
        break;

      case SAI_VLAN_ATTR_VLAN_ID:
        switch_status =
            switch_api_vlan_handle_to_id_get(device, vlan_id, &switch_vlan);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SWITCH_STATUS_SUCCESS) {
          SAI_LOG_ERROR("Failed to get vlan-id for vlan %d: %s",
                        vlan_id,
                        sai_status_to_string(status));
          return status;
        }
        attr->value.u32 = switch_vlan;
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
* Routine Description:
*    Remove VLAN configuration (remove all VLANs).
*
* Arguments:
*    None
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_remove_all_vlans(void) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
    \brief Create VLAN Member
    \param[out] vlan_member_id VLAN member ID
    \param[in] switch_id Switch id
    \param[in] attr_count number of attributes
    \param[in] attr_list array of attributes
    \return Success: SAI_STATUS_SUCCESS
            Failure: failure status code on error
*/
sai_status_t sai_create_vlan_member(_Out_ sai_object_id_t *vlan_member_id,
                                    _In_ sai_object_id_t switch_id,
                                    _In_ uint32_t attr_count,
                                    _In_ const sai_attribute_t *attr_list) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  SAI_LOG_ENTER();
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_handle_t vlan_handle = 0;
  unsigned int index = 0;
  switch_handle_t intf_handle = 0;
  sai_vlan_tagging_mode_t tagging_mode = SAI_VLAN_TAGGING_MODE_UNTAGGED;
  switch_handle_t vlan_member_handle = SWITCH_API_INVALID_HANDLE;
  *vlan_member_id = SAI_NULL_OBJECT_ID;

  memset(vlan_member_id, 0, sizeof(sai_object_id_t));
  for (index = 0; index < attr_count; index++) {
    switch (attr_list[index].id) {
      case SAI_VLAN_MEMBER_ATTR_VLAN_ID:
        vlan_handle = attr_list[index].value.oid;
        break;
      case SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID:
        intf_handle = (switch_handle_t)attr_list[index].value.oid;
        break;
      case SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE:
        tagging_mode = attr_list[index].value.s32;
        break;
      default:
        return SAI_STATUS_NOT_SUPPORTED;
        break;
    }
  }

  if (tagging_mode == SAI_VLAN_TAGGING_MODE_UNTAGGED) {
    switch_status = switch_api_interface_native_vlan_set(
        device, intf_handle, vlan_handle, &vlan_member_handle);
    if (status != SAI_STATUS_SUCCESS) {
      SAI_LOG_ERROR("failed to add ports to vlan 0x%lx: %s",
                    vlan_handle,
                    sai_status_to_string(status));
      return status;
    }
  } else {
    switch_status = switch_api_vlan_member_add(
        device, vlan_handle, intf_handle, &vlan_member_handle);
    status = sai_switch_status_to_sai_status(switch_status);
    if (status != SAI_STATUS_SUCCESS) {
      SAI_LOG_ERROR("failed to add ports to vlan 0x%lx: %s",
                    vlan_handle,
                    sai_status_to_string(status));
      return status;
    }
  }
  *vlan_member_id = vlan_member_handle;
  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
    \brief Remove VLAN Member
    \param[in] vlan_member_id VLAN member ID
    \return Success: SAI_STATUS_SUCCESS
            Failure: failure status code on error
*/
sai_status_t sai_remove_vlan_member(_In_ sai_object_id_t vlan_member_id) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  SAI_LOG_ENTER();
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  // sai_vlan_tagging_mode_t tag_mode=SAI_VLAN_PORT_UNTAGGED;

  switch_status =
      switch_api_vlan_member_remove_by_member_handle(device, vlan_member_id);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to del ports from vlan %d: %s",
                  vlan_member_id,
                  sai_status_to_string(status));
    return status;
  }

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
    \brief Set VLAN Member Attribute
    \param[in] vlan_member_id VLAN member ID
    \param[in] attr attribute structure containing ID and value
    \return Success: SAI_STATUS_SUCCESS
            Failure: failure status code on error
*/
sai_status_t sai_set_vlan_member_attribute(_In_ sai_object_id_t vlan_member_id,
                                           _In_ const sai_attribute_t *attr) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  SAI_LOG_ENTER();

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

switch_status_t switch_api_vlan_member_intf_handle_get(
    switch_device_t device,
    switch_handle_t vlan_member_handle,
    switch_handle_t *intf_handle);

/*
    \brief Get VLAN Member Attribute
    \param[in] vlan_member_id VLAN member ID
    \param[in] attr_count number of attributes
    \param[in,out] attr_list list of attribute structures containing ID and
   value
    \return Success: SAI_STATUS_SUCCESS
            Failure: failure status code on error
*/
sai_status_t sai_get_vlan_member_attribute(_In_ sai_object_id_t vlan_member_id,
                                           _In_ const uint32_t attr_count,
                                           _Inout_ sai_attribute_t *attr_list) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  unsigned int i = 0;
  switch_vlan_t vlan_id = 0;
  SAI_LOG_ENTER();
  bool tag_mode = false;
  sai_attribute_t *attr = attr_list;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  switch_handle_t intf_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t vlan_handle = SWITCH_API_INVALID_HANDLE;
  if (!attr_list) {
    status = SAI_STATUS_INVALID_PARAMETER;
    SAI_LOG_ERROR("Null attribute list: %s", sai_status_to_string(status));
    return status;
  }
  for (i = 0, attr = attr_list; i < attr_count; i++, attr++) {
    switch (attr->id) {
      case SAI_VLAN_MEMBER_ATTR_VLAN_ID:
        switch_status = switch_api_vlan_member_vlan_id_get(
            device, vlan_member_id, &vlan_id);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SWITCH_STATUS_SUCCESS) {
          SAI_LOG_ERROR("Failed to get vlan-id from vlan member handle %d: %s",
                        vlan_member_id,
                        sai_status_to_string(status));
          return status;
        }
        switch_api_vlan_id_to_handle_get(device, vlan_id, &vlan_handle);
        attr->value.oid = vlan_handle;
        break;

      case SAI_VLAN_MEMBER_ATTR_VLAN_TAGGING_MODE:
        switch_status = switch_api_vlan_member_vlan_tagging_mode_get(
            device, vlan_member_id, &tag_mode);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SWITCH_STATUS_SUCCESS) {
          SAI_LOG_ERROR(
              "Failed to get vlan tagging mode vlan member handle %d: %s",
              vlan_member_id,
              sai_status_to_string(status));
          return status;
        }
        if (tag_mode) {
          attr->value.oid = SAI_VLAN_TAGGING_MODE_TAGGED;
        } else {
          attr->value.oid = SAI_VLAN_TAGGING_MODE_UNTAGGED;
        }
        break;
      case SAI_VLAN_MEMBER_ATTR_BRIDGE_PORT_ID:
        switch_status = switch_api_vlan_member_intf_handle_get(
            device, vlan_member_id, &intf_handle);
        status = sai_switch_status_to_sai_status(switch_status);
        if (status != SWITCH_STATUS_SUCCESS) {
          SAI_LOG_ERROR(
              "Failed to get vlan member bridge port id for vlan member handle "
              "%d: %s",
              vlan_member_id,
              sai_status_to_string(status));
          return status;
        }
        attr->value.oid = intf_handle;
        break;

      default:
        break;
    }
  }
  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/**
 * Routine Description:
 *   @brief Clear vlan statistics counters.
 *
 * Arguments:
 *    @param[in] vlan_id - vlan id
 *    @param[in] counter_ids - specifies the array of counter ids
 *    @param[in] number_of_counters - number of counters in the array
 *
 * Return Values:
 *    @return SAI_STATUS_SUCCESS on success
 *            Failure status code on error
 */
sai_status_t sai_clear_vlan_stats(_In_ sai_object_id_t vlan_id,
                                  _In_ uint32_t number_of_counters,
                                  _In_ const sai_vlan_stat_t *counter_ids) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  SAI_LOG_ENTER();

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

static sai_status_t switch_vlan_counters_to_sai_vlan_counters(
    _In_ uint32_t number_of_counters,
    _In_ const sai_vlan_stat_t *counter_ids,
    _In_ switch_counter_t *switch_counters,
    _Out_ uint64_t *counters) {
  uint32_t index = 0;
  for (index = 0; index < number_of_counters; index++) {
    switch (counter_ids[index]) {
      case SAI_VLAN_STAT_IN_OCTETS:
        counters[index] = switch_counters[SWITCH_BD_STATS_IN_UCAST].num_bytes +
                          switch_counters[SWITCH_BD_STATS_IN_MCAST].num_bytes +
                          switch_counters[SWITCH_BD_STATS_IN_BCAST].num_bytes;
        break;
      case SAI_VLAN_STAT_IN_UCAST_PKTS:
        counters[index] = switch_counters[SWITCH_BD_STATS_IN_UCAST].num_packets;
        break;
      case SAI_VLAN_STAT_IN_NON_UCAST_PKTS:
        counters[index] =
            switch_counters[SWITCH_BD_STATS_IN_MCAST].num_packets +
            switch_counters[SWITCH_BD_STATS_IN_BCAST].num_packets;
        break;
      case SAI_VLAN_STAT_IN_DISCARDS:
      case SAI_VLAN_STAT_IN_ERRORS:
      case SAI_VLAN_STAT_IN_UNKNOWN_PROTOS:
        counters[index] = 0;
        break;
      case SAI_VLAN_STAT_OUT_OCTETS:
        counters[index] = switch_counters[SWITCH_BD_STATS_OUT_UCAST].num_bytes +
                          switch_counters[SWITCH_BD_STATS_OUT_MCAST].num_bytes +
                          switch_counters[SWITCH_BD_STATS_OUT_BCAST].num_bytes;
        break;
      case SAI_VLAN_STAT_OUT_UCAST_PKTS:
        counters[index] =
            switch_counters[SWITCH_BD_STATS_OUT_UCAST].num_packets;
        break;
      case SAI_VLAN_STAT_OUT_NON_UCAST_PKTS:
        counters[index] =
            switch_counters[SWITCH_BD_STATS_OUT_MCAST].num_packets +
            switch_counters[SWITCH_BD_STATS_OUT_BCAST].num_packets;
        break;
      case SAI_VLAN_STAT_OUT_DISCARDS:
      case SAI_VLAN_STAT_OUT_ERRORS:
      case SAI_VLAN_STAT_OUT_QLEN:
        counters[index] = 0;
        break;
      case SAI_VLAN_STAT_IN_PACKETS:
      case SAI_VLAN_STAT_OUT_PACKETS:
        SAI_LOG_WARN("Unsupported attribute");
        break;
    }
  }
  return SAI_STATUS_SUCCESS;
}

/*
* Routine Description:
*   Get vlan statistics counters.
*
* Arguments:
*    [in] vlan_id - VLAN id
*    [in] counter_ids - specifies the array of counter ids
*    [in] number_of_counters - number of counters in the array
*    [out] counters - array of resulting counter values.
*
* Return Values:
*    SAI_STATUS_SUCCESS on success
*    Failure status code on error
*/
sai_status_t sai_get_vlan_stats(_In_ sai_object_id_t vlan_id,
                                _In_ uint32_t number_of_counters,
                                _In_ const sai_vlan_stat_t *counter_ids,
                                _Out_ uint64_t *counters) {
  SAI_LOG_ENTER();

  sai_status_t status = SAI_STATUS_SUCCESS;
  switch_counter_t *switch_counters = NULL;
  switch_bd_counter_id_t *vlan_stat_ids = NULL;
  switch_handle_t vlan_handle = 0;
  switch_status_t switch_status = SWITCH_STATUS_SUCCESS;
  uint32_t index = 0;

  switch_status = switch_api_vlan_id_to_handle_get(
      device, (switch_vlan_t)vlan_id, &vlan_handle);
  status = sai_switch_status_to_sai_status(switch_status);
  if (status != SAI_STATUS_SUCCESS) {
    SAI_LOG_ERROR("failed to rremove ports from vlan %d: %s",
                  vlan_id,
                  sai_status_to_string(status));
    return status;
  }

  switch_counters = SAI_MALLOC(sizeof(switch_counter_t) * SWITCH_BD_STATS_MAX);
  if (!switch_counters) {
    status = SAI_STATUS_NO_MEMORY;
    SAI_LOG_ERROR("failed to get vlan stats %d: %s",
                  vlan_id,
                  sai_status_to_string(status));
    return status;
  }

  vlan_stat_ids =
      SAI_MALLOC(sizeof(switch_bd_counter_id_t) * SWITCH_BD_STATS_MAX);
  if (!vlan_stat_ids) {
    status = SAI_STATUS_NO_MEMORY;
    SAI_LOG_ERROR("failed to get vlan stats %d: %s",
                  vlan_id,
                  sai_status_to_string(status));
    SAI_FREE(switch_counters);
    return status;
  }

  for (index = 0; index < SWITCH_BD_STATS_MAX; index++) {
    vlan_stat_ids[index] = index;
  }

  switch_status = switch_api_vlan_stats_get(
      device, vlan_handle, SWITCH_BD_STATS_MAX, vlan_stat_ids, switch_counters);
  status = sai_switch_status_to_sai_status(switch_status);

  if (status != SWITCH_STATUS_SUCCESS) {
    status = SAI_STATUS_NO_MEMORY;
    SAI_LOG_ERROR("failed to get vlan stats %d: %s",
                  vlan_id,
                  sai_status_to_string(status));
    SAI_FREE(vlan_stat_ids);
    SAI_FREE(switch_counters);
    return status;
  }

  switch_vlan_counters_to_sai_vlan_counters(
      number_of_counters, counter_ids, switch_counters, counters);

  SAI_FREE(vlan_stat_ids);
  SAI_FREE(switch_counters);

  SAI_LOG_EXIT();

  return (sai_status_t)status;
}

/*
* VLAN methods table retrieved with sai_api_query()
*/
sai_vlan_api_t vlan_api = {
    .create_vlan = sai_create_vlan_entry,
    .remove_vlan = sai_remove_vlan_entry,
    .set_vlan_attribute = sai_set_vlan_entry_attribute,
    .get_vlan_attribute = sai_get_vlan_entry_attribute,
    .create_vlan_member = sai_create_vlan_member,
    .remove_vlan_member = sai_remove_vlan_member,
    .set_vlan_member_attribute = sai_set_vlan_member_attribute,
    .get_vlan_member_attribute = sai_get_vlan_member_attribute,
    .get_vlan_stats = sai_get_vlan_stats,
    .clear_vlan_stats = sai_clear_vlan_stats};

sai_status_t sai_vlan_initialize(sai_api_service_t *sai_api_service) {
  sai_api_service->vlan_api = vlan_api;
  return SAI_STATUS_SUCCESS;
}
