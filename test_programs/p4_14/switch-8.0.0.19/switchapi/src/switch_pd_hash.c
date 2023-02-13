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
#include "switchapi/switch_hash.h"
#include "switch_internal.h"
#include "switch_pd.h"

#define HASH_INPUT_FIELDS_MAX_ATTRIBUTE 16

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_pd_compute_hashes_entry_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_IPV4_DISABLE
  p4_pd_dc_compute_ipv4_hashes_match_spec_t match_spec_ipv4;
  SWITCH_MEMSET(&match_spec_ipv4, 0, sizeof(match_spec_ipv4));
  match_spec_ipv4.ethernet_valid = 1;
  pd_status = p4_pd_dc_compute_ipv4_hashes_table_add_with_compute_lkp_ipv4_hash(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec_ipv4, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec_ipv4;
    pd_entry.match_spec_size = sizeof(match_spec_ipv4);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "compute ipv4 hashes table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_IPV4_DISABLE */

#ifndef P4_IPV6_DISABLE
  p4_pd_dc_compute_ipv6_hashes_match_spec_t match_spec_ipv6;
  SWITCH_MEMSET(&match_spec_ipv6, 0, sizeof(match_spec_ipv6));
  match_spec_ipv6.ethernet_valid = 1;
  pd_status = p4_pd_dc_compute_ipv6_hashes_table_add_with_compute_lkp_ipv6_hash(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec_ipv6, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec_ipv6;
    pd_entry.match_spec_size = sizeof(match_spec_ipv6);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "compute ipv6 hashes table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_IPV6_DISABLE */

#ifndef P4_L2_DISABLE
  p4_pd_dc_compute_non_ip_hashes_match_spec_t match_spec_non_ip;
  SWITCH_MEMSET(&match_spec_non_ip, 0, sizeof(match_spec_non_ip));
  match_spec_non_ip.ethernet_valid = 1;
  pd_status =
      p4_pd_dc_compute_non_ip_hashes_table_add_with_compute_lkp_non_ip_hash(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec_non_ip, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec_non_ip;
    pd_entry.match_spec_size = sizeof(match_spec_non_ip);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "compute non ip hashes table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#endif /* P4_L2_DISABLE */

  p4_pd_dc_compute_other_hashes_match_spec_t match_spec_other;
  memset(&match_spec_other, 0, sizeof(match_spec_other));
  match_spec_other.ethernet_valid = 1;
  pd_status = p4_pd_dc_compute_other_hashes_table_add_with_compute_other_hashes(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec_other, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "compute other hashes table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#else /* __TARGET_TOFINO__ */

#ifndef P4_IPV4_DISABLE
  pd_status =
      p4_pd_dc_compute_ipv4_hashes_set_default_action_compute_lkp_ipv4_hash(
          switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "compute ipv4 hashes table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_IPV4_DISABLE */

#ifndef P4_IPV6_DISABLE
  pd_status =
      p4_pd_dc_compute_ipv6_hashes_set_default_action_compute_lkp_ipv6_hash(
          switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "compute ipv6 hashes table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_IPV6_DISABLE */
#ifndef P4_L2_DISABLE
  pd_status =
      p4_pd_dc_compute_non_ip_hashes_set_default_action_compute_lkp_non_ip_hash(
          switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "compute non ip hashes table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_L2_DISABLE */
#endif /* __TARGET_TOFINO__ */

  pd_status =
      p4_pd_dc_compute_other_hashes_set_default_action_compute_other_hashes(
          switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "compute other table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "compute hashes table entry init success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "compute hashes table entry init failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_ipv6_hash_input_fields_set(
    switch_device_t device, switch_hash_ipv6_input_fields_t input) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_IPV6_DISABLE

  p4_pd_dc_lkp_ipv6_hash1_input_t field_list;

  switch (input) {
    case SWITCH_HASH_IPV6_INPUT_SIP_DIP_PROT_SP_DP: {
      field_list = P4_PD_DC_LKP_IPV6_HASH1_INPUT_LKP_IPV6_HASH1_FIELDS;
    } break;
    case SWITCH_HASH_IPV6_INPUT_PROT_DP_SIP_SP_DIP: {
      field_list = P4_PD_DC_LKP_IPV6_HASH1_INPUT_LKP_IPV6_HASH1_FIELDS_1;
    } break;
    default:
      return SWITCH_STATUS_INVALID_PARAMETER;
  }

  pd_status = p4_pd_dc_hash_calc_lkp_ipv6_hash1_input_set(
      switch_cfg_sess_hdl, device, field_list);
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV6_DISABLE */
#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv6 hash input fiels set success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv6 hash input fields set failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_ipv4_hash_input_fields_set(
    switch_device_t device, switch_hash_ipv4_input_fields_t input) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_IPV4_DISABLE

  p4_pd_dc_lkp_ipv4_hash1_input_t field_list;

  switch (input) {
    case SWITCH_HASH_IPV4_INPUT_SIP_DIP_PROT_SP_DP: {
      field_list = P4_PD_DC_LKP_IPV4_HASH1_INPUT_LKP_IPV4_HASH1_FIELDS;
    } break;
    case SWITCH_HASH_IPV4_INPUT_PROT_DP_DIP_SP_SIP: {
      field_list = P4_PD_DC_LKP_IPV4_HASH1_INPUT_LKP_IPV4_HASH1_FIELDS_1;
    } break;
    default:
      return SWITCH_STATUS_INVALID_PARAMETER;
  }

  pd_status = p4_pd_dc_hash_calc_lkp_ipv4_hash1_input_set(
      switch_cfg_sess_hdl, device, field_list);
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV4_DISABLE  */
#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv4 hash input fiels set success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv4 hash input fields set failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_non_ip_hash_input_fields_set(
    switch_device_t device, switch_hash_non_ip_input_fields_t input) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_L2_DISABLE

  p4_pd_dc_lkp_non_ip_hash1_input_t field_list;

  switch (input) {
    case SWITCH_HASH_NON_IP_INPUT_IF_SMAC_DMAC_ETYPE: {
      field_list = P4_PD_DC_LKP_NON_IP_HASH1_INPUT_LKP_NON_IP_HASH1_FIELDS;
    } break;
    case SWITCH_HASH_NON_IP_INPUT_ETYPE_SMAC_IF_DMAC: {
      field_list = P4_PD_DC_LKP_NON_IP_HASH1_INPUT_LKP_NON_IP_HASH1_FIELDS_1;
    } break;
    default:
      return SWITCH_STATUS_INVALID_PARAMETER;
  }

  pd_status = p4_pd_dc_hash_calc_lkp_non_ip_hash1_input_set(
      switch_cfg_sess_hdl, device, field_list);
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L2_DISABLE */
#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "non ip hash input fiels set success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "non ip hash input fields set failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_ipv6_hash_input_fields_attribute_set(
    switch_device_t device,
    switch_hash_ipv6_input_fields_t input,
    switch_uint32_t attr_flags) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_IPV6_DISABLE

  p4_pd_dc_lkp_ipv6_hash1_input_t field_list;
  p4_pd_dc_lkp_ipv6_hash1_input_field_attribute_t
      field_attribute[HASH_INPUT_FIELDS_MAX_ATTRIBUTE];
  uint32_t attr_count;

  for (int i = 0; i < HASH_INPUT_FIELDS_MAX_ATTRIBUTE; i++) {
    field_attribute[i].input_field.id = i;
    field_attribute[i].type = P4_PD_INPUT_FIELD_ATTR_TYPE_MASK;
    field_attribute[i].value.val = P4_PD_INPUT_FIELD_EXCLUDED;
  }

  switch (input) {
    case SWITCH_HASH_IPV6_INPUT_SIP_DIP_PROT_SP_DP: {
      field_list = P4_PD_DC_LKP_IPV6_HASH1_INPUT_LKP_IPV6_HASH1_FIELDS;
      if (attr_flags & SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_IP)
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_IPV6_HASH1_FIELDS_IPV6_METADATA_LKP_IPV6_SA]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
      if ((attr_flags & SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_DST_IP))
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_IPV6_HASH1_FIELDS_IPV6_METADATA_LKP_IPV6_DA]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
      if (attr_flags & SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_IP_PROTOCOL)
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_IPV6_HASH1_FIELDS_L3_METADATA_LKP_IP_PROTO]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
      if (attr_flags & SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_PORT)
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_IPV6_HASH1_FIELDS_L3_METADATA_LKP_L4_SPORT]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
      if (attr_flags & SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_DST_PORT)
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_IPV6_HASH1_FIELDS_L3_METADATA_LKP_L4_DPORT]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
    } break;
    case SWITCH_HASH_IPV6_INPUT_PROT_DP_SIP_SP_DIP: {
      field_list = P4_PD_DC_LKP_IPV6_HASH1_INPUT_LKP_IPV6_HASH1_FIELDS_1;
      if (attr_flags & SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_IP)
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_IPV6_HASH1_FIELDS_1_IPV6_METADATA_LKP_IPV6_SA]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
      if (attr_flags & SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_DST_IP)
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_IPV6_HASH1_FIELDS_1_IPV6_METADATA_LKP_IPV6_DA]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
      if (attr_flags & SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_IP_PROTOCOL)
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_IPV6_HASH1_FIELDS_1_L3_METADATA_LKP_IP_PROTO]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
      if (attr_flags & SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_PORT)
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_IPV6_HASH1_FIELDS_1_L3_METADATA_LKP_L4_SPORT]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
      if (attr_flags & SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_DST_PORT)
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_IPV6_HASH1_FIELDS_1_L3_METADATA_LKP_L4_DPORT]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
    } break;
    default:
      p4_pd_complete_operations(switch_cfg_sess_hdl);
      return SWITCH_STATUS_INVALID_PARAMETER;
  }

  pd_status = p4_pd_dc_hash_calc_lkp_ipv6_hash1_input_field_attribute_count_get(
      switch_cfg_sess_hdl, device, field_list, &attr_count);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv6 hash input fields attribute set failed "
        "on device %d, attribute count get failed: %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  } else {
    pd_status = p4_pd_dc_hash_calc_lkp_ipv6_hash1_input_field_attribute_set(
        switch_cfg_sess_hdl, device, field_list, attr_count, field_attribute);
  }

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
#endif /* P4_IPV6_DISABLE */
#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv6 hash input field attribute set success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv6 hash input field attribute set failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_ipv4_hash_input_fields_attribute_set(
    switch_device_t device,
    switch_hash_ipv4_input_fields_t input,
    switch_uint32_t attr_flags) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_IPV4_DISABLE

  p4_pd_dc_lkp_ipv4_hash1_input_t field_list;
  p4_pd_dc_lkp_ipv4_hash1_input_field_attribute_t
      field_attribute[HASH_INPUT_FIELDS_MAX_ATTRIBUTE];
  uint32_t attr_count;

  for (int i = 0; i < HASH_INPUT_FIELDS_MAX_ATTRIBUTE; i++) {
    field_attribute[i].input_field.id = i;
    field_attribute[i].type = P4_PD_INPUT_FIELD_ATTR_TYPE_MASK;
    field_attribute[i].value.val = P4_PD_INPUT_FIELD_EXCLUDED;
  }

  switch (input) {
    case SWITCH_HASH_IPV4_INPUT_SIP_DIP_PROT_SP_DP: {
      field_list = P4_PD_DC_LKP_IPV4_HASH1_INPUT_LKP_IPV4_HASH1_FIELDS;
      if ((attr_flags & SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_IP))
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_IPV4_HASH1_FIELDS_IPV4_METADATA_LKP_IPV4_DA]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
      if ((attr_flags & SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_DST_IP))
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_IPV4_HASH1_FIELDS_IPV4_METADATA_LKP_IPV4_SA]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
      if ((attr_flags & SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_IP_PROTOCOL))
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_IPV4_HASH1_FIELDS_L3_METADATA_LKP_IP_PROTO]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
      if ((attr_flags & SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_PORT))
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_IPV4_HASH1_FIELDS_L3_METADATA_LKP_L4_SPORT]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
      if ((attr_flags & SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_DST_PORT))
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_IPV4_HASH1_FIELDS_L3_METADATA_LKP_L4_DPORT]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
    } break;
    case SWITCH_HASH_IPV4_INPUT_PROT_DP_DIP_SP_SIP: {
      field_list = P4_PD_DC_LKP_IPV4_HASH1_INPUT_LKP_IPV4_HASH1_FIELDS_1;
      if ((attr_flags & SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_IP))
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_IPV4_HASH1_FIELDS_1_IPV4_METADATA_LKP_IPV4_SA]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
      if ((attr_flags & SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_DST_IP))
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_IPV4_HASH1_FIELDS_1_IPV4_METADATA_LKP_IPV4_DA]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
      if ((attr_flags & SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_IP_PROTOCOL))
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_IPV4_HASH1_FIELDS_1_L3_METADATA_LKP_IP_PROTO]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
      if ((attr_flags & SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_PORT))
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_IPV4_HASH1_FIELDS_1_L3_METADATA_LKP_L4_SPORT]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
      if ((attr_flags & SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_DST_PORT))
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_IPV4_HASH1_FIELDS_1_L3_METADATA_LKP_L4_DPORT]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
    } break;
    default:
      p4_pd_complete_operations(switch_cfg_sess_hdl);
      return SWITCH_STATUS_INVALID_PARAMETER;
  }

  pd_status = p4_pd_dc_hash_calc_lkp_ipv4_hash1_input_field_attribute_count_get(
      switch_cfg_sess_hdl, device, field_list, &attr_count);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv4 hash input fields attribute set failed "
        "on device %d, attribute count get failed: %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  } else {
    pd_status = p4_pd_dc_hash_calc_lkp_ipv4_hash1_input_field_attribute_set(
        switch_cfg_sess_hdl, device, field_list, attr_count, field_attribute);
  }

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
#endif /* P4_IPV4_DISABLE */
#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv4 hash input field attribute set success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv4 hash input field attribute set failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_non_ip_hash_input_fields_attribute_set(
    switch_device_t device,
    switch_hash_non_ip_input_fields_t input,
    switch_uint32_t attr_flags) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_L2_DISABLE

  p4_pd_dc_lkp_non_ip_hash1_input_t field_list;
  p4_pd_dc_lkp_non_ip_hash1_input_field_attribute_t
      field_attribute[HASH_INPUT_FIELDS_MAX_ATTRIBUTE];
  uint32_t attr_count;

  for (int i = 0; i < HASH_INPUT_FIELDS_MAX_ATTRIBUTE; i++) {
    field_attribute[i].input_field.id = i;
    field_attribute[i].type = P4_PD_INPUT_FIELD_ATTR_TYPE_MASK;
    field_attribute[i].value.val = P4_PD_INPUT_FIELD_EXCLUDED;
  }

  switch (input) {
    case SWITCH_HASH_NON_IP_INPUT_IF_SMAC_DMAC_ETYPE: {
      field_list = P4_PD_DC_LKP_NON_IP_HASH1_INPUT_LKP_NON_IP_HASH1_FIELDS;
      if ((attr_flags & SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_SRC_MAC))
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_NON_IP_HASH1_FIELDS_L2_METADATA_LKP_MAC_SA]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
      if ((attr_flags & SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_DST_MAC))
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_NON_IP_HASH1_FIELDS_L2_METADATA_LKP_MAC_DA]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
      if ((attr_flags & SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_ETYPE))
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_NON_IP_HASH1_FIELDS_L2_METADATA_LKP_MAC_TYPE]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
      if ((attr_flags & SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_IF_INDEX))
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_NON_IP_HASH1_FIELDS_INGRESS_METADATA_IFINDEX]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
    } break;
    case SWITCH_HASH_NON_IP_INPUT_ETYPE_SMAC_IF_DMAC: {
      field_list = P4_PD_DC_LKP_NON_IP_HASH1_INPUT_LKP_NON_IP_HASH1_FIELDS_1;
      if ((attr_flags & SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_SRC_MAC))
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_NON_IP_HASH1_FIELDS_1_L2_METADATA_LKP_MAC_SA]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
      if ((attr_flags & SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_DST_MAC))
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_NON_IP_HASH1_FIELDS_1_L2_METADATA_LKP_MAC_DA]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
      if ((attr_flags & SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_ETYPE))
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_NON_IP_HASH1_FIELDS_1_L2_METADATA_LKP_MAC_TYPE]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
      if ((attr_flags & SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_IF_INDEX))
        field_attribute
            [P4_PD_INPUT_FIELD_LKP_NON_IP_HASH1_FIELDS_1_INGRESS_METADATA_IFINDEX]
                .value.mask = P4_PD_INPUT_FIELD_INCLUDED;
    } break;
    default:
      p4_pd_complete_operations(switch_cfg_sess_hdl);
      return SWITCH_STATUS_INVALID_PARAMETER;
  }

  pd_status =
      p4_pd_dc_hash_calc_lkp_non_ip_hash1_input_field_attribute_count_get(
          switch_cfg_sess_hdl, device, field_list, &attr_count);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv4 hash input fields attribute set failed "
        "on device %d, attribute count get failed: %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  } else {
    pd_status = p4_pd_dc_hash_calc_lkp_non_ip_hash1_input_field_attribute_set(
        switch_cfg_sess_hdl, device, field_list, attr_count, field_attribute);
  }

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
#endif /* P4_L2_DISABLE */
#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "non ip hash input field attribute set success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "non ip hash input field attribute set failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_ipv6_hash_input_fields_attribute_get(
    switch_device_t device,
    switch_hash_ipv6_input_fields_t input,
    switch_uint32_t *attr_flags) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_uint32_t attr_count;

  UNUSED(status);
  UNUSED(pd_status);
  *attr_flags = 0;
#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_IPV6_DISABLE

  p4_pd_dc_lkp_ipv6_hash1_input_t field_list;
  p4_pd_dc_lkp_ipv6_hash1_input_field_attribute_t
      field_attribute[HASH_INPUT_FIELDS_MAX_ATTRIBUTE];

  switch (input) {
    case SWITCH_HASH_IPV6_INPUT_SIP_DIP_PROT_SP_DP: {
      field_list = P4_PD_DC_LKP_IPV6_HASH1_INPUT_LKP_IPV6_HASH1_FIELDS;
    } break;
    case SWITCH_HASH_IPV6_INPUT_PROT_DP_SIP_SP_DIP: {
      field_list = P4_PD_DC_LKP_IPV6_HASH1_INPUT_LKP_IPV6_HASH1_FIELDS_1;
    } break;
    default:
      pd_status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_PD_LOG_ERROR(
          "ipv6 hash input field attribute get failed "
          "on device %d : input field %d invalid \n",
          device,
          input,
          switch_error_to_string(pd_status));
      p4_pd_complete_operations(switch_cfg_sess_hdl);
      return switch_pd_status_to_status(pd_status);
  }

  pd_status = p4_pd_dc_hash_calc_lkp_ipv6_hash1_input_field_attribute_get(
      switch_cfg_sess_hdl,
      device,
      field_list,
      HASH_INPUT_FIELDS_MAX_ATTRIBUTE,
      field_attribute,
      &attr_count);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv6 hash input fields attribute get failed "
        "on device %d, %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  } else {
    if (field_list == P4_PD_DC_LKP_IPV6_HASH1_INPUT_LKP_IPV6_HASH1_FIELDS) {
      for (uint32_t i = 0; i < attr_count; i++) {
        if (field_attribute[i].value.mask == P4_PD_INPUT_FIELD_INCLUDED) {
          switch (i) {
            case P4_PD_INPUT_FIELD_LKP_IPV6_HASH1_FIELDS_IPV6_METADATA_LKP_IPV6_SA: {
              *attr_flags |= SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_IP;
            } break;
            case P4_PD_INPUT_FIELD_LKP_IPV6_HASH1_FIELDS_IPV6_METADATA_LKP_IPV6_DA: {
              *attr_flags |= SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_DST_IP;
            } break;
            case P4_PD_INPUT_FIELD_LKP_IPV6_HASH1_FIELDS_L3_METADATA_LKP_IP_PROTO: {
              *attr_flags |= SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_IP_PROTOCOL;
            } break;
            case P4_PD_INPUT_FIELD_LKP_IPV6_HASH1_FIELDS_L3_METADATA_LKP_L4_SPORT: {
              *attr_flags |= SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_PORT;
            } break;
            case P4_PD_INPUT_FIELD_LKP_IPV6_HASH1_FIELDS_L3_METADATA_LKP_L4_DPORT: {
              *attr_flags |= SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_DST_PORT;
            } break;
          }
        }
      }
    } else if (field_list ==
               P4_PD_DC_LKP_IPV6_HASH1_INPUT_LKP_IPV6_HASH1_FIELDS_1) {
      for (uint32_t i = 0; i < attr_count; i++) {
        if (field_attribute[i].value.mask == P4_PD_INPUT_FIELD_INCLUDED) {
          switch (i) {
            case P4_PD_INPUT_FIELD_LKP_IPV6_HASH1_FIELDS_1_IPV6_METADATA_LKP_IPV6_SA: {
              *attr_flags |= SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_IP;
            } break;
            case P4_PD_INPUT_FIELD_LKP_IPV6_HASH1_FIELDS_1_IPV6_METADATA_LKP_IPV6_DA: {
              *attr_flags |= SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_DST_IP;
            } break;
            case P4_PD_INPUT_FIELD_LKP_IPV6_HASH1_FIELDS_1_L3_METADATA_LKP_IP_PROTO: {
              *attr_flags |= SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_IP_PROTOCOL;
            } break;
            case P4_PD_INPUT_FIELD_LKP_IPV6_HASH1_FIELDS_1_L3_METADATA_LKP_L4_SPORT: {
              *attr_flags |= SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_PORT;
            } break;
            case P4_PD_INPUT_FIELD_LKP_IPV6_HASH1_FIELDS_1_L3_METADATA_LKP_L4_DPORT: {
              *attr_flags |= SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_DST_PORT;
            } break;
          }
        }
      }
    }
  }

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
#endif /* P4_IPV6_DISABLE */
#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv6 hash input field attribute get success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv6 hash input field attribute get failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_ipv4_hash_input_fields_attribute_get(
    switch_device_t device,
    switch_hash_ipv4_input_fields_t input,
    switch_uint32_t *attr_flags) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_uint32_t attr_count;

  UNUSED(status);
  UNUSED(pd_status);
  *attr_flags = 0;
#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_IPV4_DISABLE

  p4_pd_dc_lkp_ipv4_hash1_input_t field_list;
  p4_pd_dc_lkp_ipv4_hash1_input_field_attribute_t
      field_attribute[HASH_INPUT_FIELDS_MAX_ATTRIBUTE];

  switch (input) {
    case SWITCH_HASH_IPV4_INPUT_SIP_DIP_PROT_SP_DP: {
      field_list = P4_PD_DC_LKP_IPV4_HASH1_INPUT_LKP_IPV4_HASH1_FIELDS;
    } break;
    case SWITCH_HASH_IPV4_INPUT_PROT_DP_DIP_SP_SIP: {
      field_list = P4_PD_DC_LKP_IPV4_HASH1_INPUT_LKP_IPV4_HASH1_FIELDS_1;
    } break;
    default:
      pd_status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_PD_LOG_ERROR(
          "ipv4 hash input field attribute get failed "
          "on device %d : input field %d invalid \n",
          device,
          input,
          switch_error_to_string(pd_status));
      p4_pd_complete_operations(switch_cfg_sess_hdl);
      return switch_pd_status_to_status(pd_status);
  }

  pd_status = p4_pd_dc_hash_calc_lkp_ipv4_hash1_input_field_attribute_get(
      switch_cfg_sess_hdl,
      device,
      field_list,
      HASH_INPUT_FIELDS_MAX_ATTRIBUTE,
      field_attribute,
      &attr_count);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv4 hash input fields attribute get failed "
        "on device %d, %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  } else {
    if (field_list == P4_PD_DC_LKP_IPV4_HASH1_INPUT_LKP_IPV4_HASH1_FIELDS) {
      for (uint32_t i = 0; i < attr_count; i++) {
        if (field_attribute[i].value.mask == P4_PD_INPUT_FIELD_INCLUDED) {
          switch (i) {
            case P4_PD_INPUT_FIELD_LKP_IPV4_HASH1_FIELDS_L3_METADATA_LKP_L4_DPORT: {
              *attr_flags |= SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_DST_PORT;
            } break;
            case P4_PD_INPUT_FIELD_LKP_IPV4_HASH1_FIELDS_L3_METADATA_LKP_L4_SPORT: {
              *attr_flags |= SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_PORT;
            } break;
            case P4_PD_INPUT_FIELD_LKP_IPV4_HASH1_FIELDS_L3_METADATA_LKP_IP_PROTO: {
              *attr_flags |= SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_IP_PROTOCOL;
            } break;
            case P4_PD_INPUT_FIELD_LKP_IPV4_HASH1_FIELDS_IPV4_METADATA_LKP_IPV4_DA: {
              *attr_flags |= SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_DST_IP;
            } break;
            case P4_PD_INPUT_FIELD_LKP_IPV4_HASH1_FIELDS_IPV4_METADATA_LKP_IPV4_SA: {
              *attr_flags |= SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_IP;
            } break;
          }
        }
      }
    } else if (field_list ==
               P4_PD_DC_LKP_IPV4_HASH1_INPUT_LKP_IPV4_HASH1_FIELDS_1) {
      for (uint32_t i = 0; i < attr_count; i++) {
        if (field_attribute[i].value.mask == P4_PD_INPUT_FIELD_INCLUDED) {
          switch (i) {
            case P4_PD_INPUT_FIELD_LKP_IPV4_HASH1_FIELDS_1_IPV4_METADATA_LKP_IPV4_SA: {
              *attr_flags |= SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_IP;
            } break;
            case P4_PD_INPUT_FIELD_LKP_IPV4_HASH1_FIELDS_1_L3_METADATA_LKP_L4_SPORT: {
              *attr_flags |= SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_PORT;
            } break;
            case P4_PD_INPUT_FIELD_LKP_IPV4_HASH1_FIELDS_1_IPV4_METADATA_LKP_IPV4_DA: {
              *attr_flags |= SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_DST_IP;
            } break;
            case P4_PD_INPUT_FIELD_LKP_IPV4_HASH1_FIELDS_1_L3_METADATA_LKP_L4_DPORT: {
              *attr_flags |= SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_DST_PORT;
            } break;
            case P4_PD_INPUT_FIELD_LKP_IPV4_HASH1_FIELDS_1_L3_METADATA_LKP_IP_PROTO: {
              *attr_flags |= SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_IP_PROTOCOL;
            } break;
          }
        }
      }
    }
  }

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
#endif /* P4_IPV4_DISABLE */
#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv4 hash input field attribute get success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv4 hash input field attribute get failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_non_ip_hash_input_fields_attribute_get(
    switch_device_t device,
    switch_hash_non_ip_input_fields_t input,
    switch_uint32_t *attr_flags) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_uint32_t attr_count;

  UNUSED(status);
  UNUSED(pd_status);
  *attr_flags = 0;
#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_IPV4_DISABLE

  p4_pd_dc_lkp_non_ip_hash1_input_t field_list;
  p4_pd_dc_lkp_non_ip_hash1_input_field_attribute_t
      field_attribute[HASH_INPUT_FIELDS_MAX_ATTRIBUTE];

  switch (input) {
    case SWITCH_HASH_NON_IP_INPUT_IF_SMAC_DMAC_ETYPE: {
      field_list = P4_PD_DC_LKP_NON_IP_HASH1_INPUT_LKP_NON_IP_HASH1_FIELDS;
    } break;
    case SWITCH_HASH_NON_IP_INPUT_ETYPE_SMAC_IF_DMAC: {
      field_list = P4_PD_DC_LKP_NON_IP_HASH1_INPUT_LKP_NON_IP_HASH1_FIELDS_1;
    } break;
    default:
      pd_status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_PD_LOG_ERROR(
          "non ip hash input field attribute get failed "
          "on device %d : input field %d invalid \n",
          device,
          input,
          switch_error_to_string(pd_status));
      p4_pd_complete_operations(switch_cfg_sess_hdl);
      return switch_pd_status_to_status(pd_status);
  }

  pd_status = p4_pd_dc_hash_calc_lkp_non_ip_hash1_input_field_attribute_get(
      switch_cfg_sess_hdl,
      device,
      field_list,
      HASH_INPUT_FIELDS_MAX_ATTRIBUTE,
      field_attribute,
      &attr_count);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "non ip hash input fields attribute get failed "
        "on device %d, %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  } else {
    if (field_list == P4_PD_DC_LKP_NON_IP_HASH1_INPUT_LKP_NON_IP_HASH1_FIELDS) {
      for (uint32_t i = 0; i < attr_count; i++) {
        if (field_attribute[i].value.mask == P4_PD_INPUT_FIELD_INCLUDED) {
          switch (i) {
            case P4_PD_INPUT_FIELD_LKP_NON_IP_HASH1_FIELDS_L2_METADATA_LKP_MAC_TYPE: {
              *attr_flags |= SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_ETYPE;
            } break;
            case P4_PD_INPUT_FIELD_LKP_NON_IP_HASH1_FIELDS_L2_METADATA_LKP_MAC_DA: {
              *attr_flags |= SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_DST_MAC;
            } break;
            case P4_PD_INPUT_FIELD_LKP_NON_IP_HASH1_FIELDS_L2_METADATA_LKP_MAC_SA: {
              *attr_flags |= SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_SRC_MAC;
            } break;
            case P4_PD_INPUT_FIELD_LKP_NON_IP_HASH1_FIELDS_INGRESS_METADATA_IFINDEX: {
              *attr_flags |= SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_IF_INDEX;
            } break;
          }
        }
      }
    } else if (field_list ==
               P4_PD_DC_LKP_NON_IP_HASH1_INPUT_LKP_NON_IP_HASH1_FIELDS_1) {
      for (uint32_t i = 0; i < attr_count; i++) {
        if (field_attribute[i].value.mask == P4_PD_INPUT_FIELD_INCLUDED) {
          switch (i) {
            case P4_PD_INPUT_FIELD_LKP_NON_IP_HASH1_FIELDS_1_L2_METADATA_LKP_MAC_DA: {
              *attr_flags |= SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_DST_MAC;
            } break;
            case P4_PD_INPUT_FIELD_LKP_NON_IP_HASH1_FIELDS_1_INGRESS_METADATA_IFINDEX: {
              *attr_flags |= SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_IF_INDEX;
            } break;
            case P4_PD_INPUT_FIELD_LKP_NON_IP_HASH1_FIELDS_1_L2_METADATA_LKP_MAC_SA: {
              *attr_flags |= SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_SRC_MAC;
            } break;
            case P4_PD_INPUT_FIELD_LKP_NON_IP_HASH1_FIELDS_1_L2_METADATA_LKP_MAC_TYPE: {
              *attr_flags |= SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_ETYPE;
            } break;
          }
        }
      }
    }
  }

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
#endif /* P4_IPV4_DISABLE */
#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "non ip hash input field attribute get success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "non ip hash input field attribute get failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_ipv6_hash_algorithm_set(
    switch_device_t device, switch_hash_ipv6_algorithm_t algorithm) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_IPV6_DISABLE
  p4_pd_dc_lkp_ipv6_hash1_algo_t pd_algo;

  switch (algorithm) {
#if defined(BMV2) && defined(INT_ENABLE)
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_CUSTOM: {
      pd_algo = P4_PD_DC_LKP_IPV6_HASH1_ALGORITHM_CRC16_CUSTOM;
    } break;
#elif !defined(P4_HASH_32BIT_ENABLE) /*16 Bit Hash Algorithm */
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16: {
      pd_algo = P4_PD_DC_LKP_IPV6_HASH1_ALGORITHM_CRC16;
    } break;
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_DECT: {
      pd_algo = P4_PD_DC_LKP_IPV6_HASH1_ALGORITHM_CRC_16_DECT;
    } break;
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_GENIBUS: {
      pd_algo = P4_PD_DC_LKP_IPV6_HASH1_ALGORITHM_CRC_16_GENIBUS;
    } break;
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_DNP: {
      pd_algo = P4_PD_DC_LKP_IPV6_HASH1_ALGORITHM_CRC_16_DNP;
    } break;
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_TELEDISK: {
      pd_algo = P4_PD_DC_LKP_IPV6_HASH1_ALGORITHM_CRC_16_TELEDISK;
    } break;
#else                                /* 32 Bit Hash Algorithm */
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC32: {
      pd_algo = P4_PD_DC_LKP_IPV6_HASH1_ALGORITHM_CRC32;
    } break;
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC32_BZIP2: {
      pd_algo = P4_PD_DC_LKP_IPV6_HASH1_ALGORITHM_CRC_32_BZIP2;
    } break;
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC32_C: {
      pd_algo = P4_PD_DC_LKP_IPV6_HASH1_ALGORITHM_CRC_32C;
    } break;
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC32_D: {
      pd_algo = P4_PD_DC_LKP_IPV6_HASH1_ALGORITHM_CRC_32D;
    } break;
#endif
    default:
      goto cleanup;
  }
  pd_status = p4_pd_dc_hash_calc_lkp_ipv6_hash1_algorithm_set(
      switch_cfg_sess_hdl, device, pd_algo);
cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV6_DISABLE */
#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv6 hash algorithm set success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv6 hash algorithm set failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_ipv4_hash_algorithm_set(
    switch_device_t device, switch_hash_ipv4_algorithm_t algorithm) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_IPV4_DISABLE
  p4_pd_dc_lkp_ipv4_hash1_algo_t pd_algo;

  switch (algorithm) {
#if defined(BMV2) && defined(INT_ENABLE)
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_CUSTOM: {
      pd_algo = P4_PD_DC_LKP_IPV4_HASH1_ALGORITHM_CRC16_CUSTOM;
    } break;
#elif !defined(P4_HASH_32BIT_ENABLE) /*16 Bit Hash Algorithm */
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16: {
      pd_algo = P4_PD_DC_LKP_IPV4_HASH1_ALGORITHM_CRC16;
    } break;
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_DECT: {
      pd_algo = P4_PD_DC_LKP_IPV4_HASH1_ALGORITHM_CRC_16_DECT;
    } break;
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_GENIBUS: {
      pd_algo = P4_PD_DC_LKP_IPV4_HASH1_ALGORITHM_CRC_16_GENIBUS;
    } break;
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_DNP: {
      pd_algo = P4_PD_DC_LKP_IPV4_HASH1_ALGORITHM_CRC_16_DNP;
    } break;
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_TELEDISK: {
      pd_algo = P4_PD_DC_LKP_IPV4_HASH1_ALGORITHM_CRC_16_TELEDISK;
    } break;
#else                                /* 32 Bit Hash Algorithm */
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC32: {
      pd_algo = P4_PD_DC_LKP_IPV4_HASH1_ALGORITHM_CRC32;
    } break;
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC32_BZIP2: {
      pd_algo = P4_PD_DC_LKP_IPV4_HASH1_ALGORITHM_CRC_32_BZIP2;
    } break;
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC32_C: {
      pd_algo = P4_PD_DC_LKP_IPV4_HASH1_ALGORITHM_CRC_32C;
    } break;
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC32_D: {
      pd_algo = P4_PD_DC_LKP_IPV4_HASH1_ALGORITHM_CRC_32D;
    } break;
#endif
    default:
      goto cleanup;
  }
  pd_status = p4_pd_dc_hash_calc_lkp_ipv4_hash1_algorithm_set(
      switch_cfg_sess_hdl, device, pd_algo);
cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV4_DISABLE */
#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv4 hash algorithm set success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv4 hash algorithm set failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_non_ip_hash_algorithm_set(
    switch_device_t device, switch_hash_non_ip_algorithm_t algorithm) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_L2_DISABLE
  p4_pd_dc_lkp_non_ip_hash1_algo_t pd_algo;

  switch (algorithm) {
#if defined(BMV2) && defined(INT_ENABLE)
    case SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_CUSTOM: {
      pd_algo = P4_PD_DC_LKP_NON_IP_HASH1_ALGORITHM_CRC16_CUSTOM;
    } break;
#elif !defined(P4_HASH_32BIT_ENABLE) /*16 Bit Hash Algorithm */
    case SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16: {
      pd_algo = P4_PD_DC_LKP_NON_IP_HASH1_ALGORITHM_CRC16;
    } break;
    case SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_DECT: {
      pd_algo = P4_PD_DC_LKP_NON_IP_HASH1_ALGORITHM_CRC_16_DECT;
    } break;
    case SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_GENIBUS: {
      pd_algo = P4_PD_DC_LKP_NON_IP_HASH1_ALGORITHM_CRC_16_GENIBUS;
    } break;
    case SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_DNP: {
      pd_algo = P4_PD_DC_LKP_NON_IP_HASH1_ALGORITHM_CRC_16_DNP;
    } break;
    case SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_TELEDISK: {
      pd_algo = P4_PD_DC_LKP_NON_IP_HASH1_ALGORITHM_CRC_16_TELEDISK;
    } break;
#else                                /* 32 Bit Hash Algorithm */
    case SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC32: {
      pd_algo = P4_PD_DC_LKP_NON_IP_HASH1_ALGORITHM_CRC32;
    } break;
    case SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC32_BZIP2: {
      pd_algo = P4_PD_DC_LKP_NON_IP_HASH1_ALGORITHM_CRC_32_BZIP2;
    } break;
    case SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC32_C: {
      pd_algo = P4_PD_DC_LKP_NON_IP_HASH1_ALGORITHM_CRC_32C;
    } break;
    case SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC32_D: {
      pd_algo = P4_PD_DC_LKP_NON_IP_HASH1_ALGORITHM_CRC_32D;
    } break;
#endif
    default:
      goto cleanup;
  }
  pd_status = p4_pd_dc_hash_calc_lkp_non_ip_hash1_algorithm_set(
      switch_cfg_sess_hdl, device, pd_algo);
cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L2_DISABLE */
#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "non ip hash algorithm set success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "non ip hash algorithm set failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_ipv6_hash_seed_set(switch_device_t device,
                                             uint64_t seed) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_IPV6_DISABLE
  pd_status = p4_pd_dc_hash_calc_lkp_ipv6_hash1_seed_set(
      switch_cfg_sess_hdl, device, seed);
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV6_DISABLE */
#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv6  hash seed set success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv6 hash seed set failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_ipv4_hash_seed_set(switch_device_t device,
                                             uint64_t seed) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_IPV4_DISABLE

  pd_status = p4_pd_dc_hash_calc_lkp_ipv4_hash1_seed_set(
      switch_cfg_sess_hdl, device, seed);
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV4_DISABLE */
#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv4  hash seed set success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv4 hash seed set failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_non_ip_hash_seed_set(switch_device_t device,
                                               uint64_t seed) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_L2_DISABLE

  pd_status = p4_pd_dc_hash_calc_lkp_non_ip_hash1_seed_set(
      switch_cfg_sess_hdl, device, seed);
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L2_DISABLE */
#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "non ip hash seed set success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "non ip hash seed set failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_lag_hash_seed_set(switch_device_t device,
                                            uint64_t seed) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
  status =
      p4_pd_dc_hash_calc_lag_hash_seed_set(switch_cfg_sess_hdl, device, seed);
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "lag hash seed set success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "lag hash seed set failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_ecmp_hash_seed_set(switch_device_t device,
                                             uint64_t seed) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
  status =
      p4_pd_dc_hash_calc_ecmp_hash_seed_set(switch_cfg_sess_hdl, device, seed);
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ecmp hash seed set success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ecmp hash seed set failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_ipv6_hash_input_fields_get(
    switch_device_t device, switch_hash_ipv6_input_fields_t *field_list) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_IPV6_DISABLE

  p4_pd_dc_lkp_ipv6_hash1_input_t pd_field_list;

  pd_status = p4_pd_dc_hash_calc_lkp_ipv6_hash1_input_get(
      switch_cfg_sess_hdl, device, &pd_field_list);
  switch (pd_field_list) {
    case P4_PD_DC_LKP_IPV6_HASH1_INPUT_LKP_IPV6_HASH1_FIELDS: {
      *field_list = SWITCH_HASH_IPV6_INPUT_SIP_DIP_PROT_SP_DP;
    } break;
    case P4_PD_DC_LKP_IPV6_HASH1_INPUT_LKP_IPV6_HASH1_FIELDS_1: {
      *field_list = SWITCH_HASH_IPV6_INPUT_PROT_DP_SIP_SP_DIP;
    } break;
  }
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
#endif /* P4_IPV6_DISABLE */
#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv6 hash input fiels get success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv6 hash input fields get failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_ipv4_hash_input_fields_get(
    switch_device_t device, switch_hash_ipv4_input_fields_t *field_list) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_IPV4_DISABLE

  p4_pd_dc_lkp_ipv4_hash1_input_t pd_field_list;

  pd_status = p4_pd_dc_hash_calc_lkp_ipv4_hash1_input_get(
      switch_cfg_sess_hdl, device, &pd_field_list);
  switch (pd_field_list) {
    case P4_PD_DC_LKP_IPV4_HASH1_INPUT_LKP_IPV4_HASH1_FIELDS: {
      *field_list = SWITCH_HASH_IPV4_INPUT_SIP_DIP_PROT_SP_DP;
    } break;
    case P4_PD_DC_LKP_IPV4_HASH1_INPUT_LKP_IPV4_HASH1_FIELDS_1: {
      *field_list = SWITCH_HASH_IPV4_INPUT_PROT_DP_DIP_SP_SIP;
    } break;
  }
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
#endif /* P4_IPV4_DISABLE */
#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv4 hash input fields get success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv4 hash input fields get failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_non_ip_hash_input_fields_get(
    switch_device_t device, switch_hash_non_ip_input_fields_t *field_list) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_L2_DISABLE

  p4_pd_dc_lkp_non_ip_hash1_input_t pd_field_list;

  pd_status = p4_pd_dc_hash_calc_lkp_non_ip_hash1_input_get(
      switch_cfg_sess_hdl, device, &pd_field_list);
  switch (pd_field_list) {
    case P4_PD_DC_LKP_NON_IP_HASH1_INPUT_LKP_NON_IP_HASH1_FIELDS: {
      *field_list = SWITCH_HASH_NON_IP_INPUT_IF_SMAC_DMAC_ETYPE;
    } break;
    case P4_PD_DC_LKP_NON_IP_HASH1_INPUT_LKP_NON_IP_HASH1_FIELDS_1: {
      *field_list = SWITCH_HASH_NON_IP_INPUT_ETYPE_SMAC_IF_DMAC;
    } break;
  }
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
#endif /* P4_L2_DISABLE */
#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "non ip hash input fields get success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "non ip hash input fields get failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_ipv6_hash_algorithm_get(
    switch_device_t device, switch_hash_ipv6_algorithm_t *algorithm) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_IPV6_DISABLE
  p4_pd_dc_lkp_ipv6_hash1_algo_t pd_algo;

  pd_status = p4_pd_dc_hash_calc_lkp_ipv6_hash1_algorithm_get(
      switch_cfg_sess_hdl, device, &pd_algo);
  switch (pd_algo) {
#if defined(BMV2) && defined(INT_ENABLE)
    case P4_PD_DC_LKP_IPV6_HASH1_ALGORITHM_CRC16_CUSTOM: {
      *algorithm = SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_CUSTOM;
    } break;
#elif !defined(P4_HASH_32BIT_ENABLE) /*16 Bit Hash Algorithm */
    case P4_PD_DC_LKP_IPV6_HASH1_ALGORITHM_CRC16: {
      *algorithm = SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16;
    } break;
    case P4_PD_DC_LKP_IPV6_HASH1_ALGORITHM_CRC_16_DECT: {
      *algorithm = SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_DECT;
    } break;
    case P4_PD_DC_LKP_IPV6_HASH1_ALGORITHM_CRC_16_GENIBUS: {
      *algorithm = SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_GENIBUS;
    } break;
    case P4_PD_DC_LKP_IPV6_HASH1_ALGORITHM_CRC_16_DNP: {
      *algorithm = SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_DNP;
    } break;
    case P4_PD_DC_LKP_IPV6_HASH1_ALGORITHM_CRC_16_TELEDISK: {
      *algorithm = SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_TELEDISK;
    } break;
#else                                /* 32 Bit Hash Algorithm */
    case P4_PD_DC_LKP_IPV6_HASH1_ALGORITHM_CRC32: {
      *algorithm = SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC32;
    } break;
    case P4_PD_DC_LKP_IPV6_HASH1_ALGORITHM_CRC_32_BZIP2: {
      *algorithm = SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC32_BZIP2;
    } break;
    case P4_PD_DC_LKP_IPV6_HASH1_ALGORITHM_CRC_32C: {
      *algorithm = SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC32_C;
    } break;
    case P4_PD_DC_LKP_IPV6_HASH1_ALGORITHM_CRC_32D: {
      *algorithm = SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC32_D;
    } break;
#endif
  }
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV6_DISABLE */
#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv6 hash algorithm get success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv6 hash algorithm get failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_ipv4_hash_algorithm_get(
    switch_device_t device, switch_hash_ipv4_algorithm_t *algorithm) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_IPV4_DISABLE
  p4_pd_dc_lkp_ipv4_hash1_algo_t pd_algo;

  pd_status = p4_pd_dc_hash_calc_lkp_ipv4_hash1_algorithm_get(
      switch_cfg_sess_hdl, device, &pd_algo);
  switch (pd_algo) {
#if defined(BMV2) && defined(INT_ENABLE)
    case P4_PD_DC_LKP_IPV4_HASH1_ALGORITHM_CRC16_CUSTOM: {
      *algorithm = SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_CUSTOM;
    } break;
#elif !defined(P4_HASH_32BIT_ENABLE) /*16 Bit Hash Algorithm */
    case P4_PD_DC_LKP_IPV4_HASH1_ALGORITHM_CRC16: {
      *algorithm = SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16;
    } break;
    case P4_PD_DC_LKP_IPV4_HASH1_ALGORITHM_CRC_16_DECT: {
      *algorithm = SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_DECT;
    } break;
    case P4_PD_DC_LKP_IPV4_HASH1_ALGORITHM_CRC_16_GENIBUS: {
      *algorithm = SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_GENIBUS;
    } break;
    case P4_PD_DC_LKP_IPV4_HASH1_ALGORITHM_CRC_16_DNP: {
      *algorithm = SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_DNP;
    } break;
    case P4_PD_DC_LKP_IPV4_HASH1_ALGORITHM_CRC_16_TELEDISK: {
      *algorithm = SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_TELEDISK;
    } break;
#else                                /* 32 Bit Hash Algorithm */
    case P4_PD_DC_LKP_IPV4_HASH1_ALGORITHM_CRC32: {
      *algorithm = SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC32;
    } break;
    case P4_PD_DC_LKP_IPV4_HASH1_ALGORITHM_CRC_32_BZIP2: {
      *algorithm = SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC32_BZIP2;
    } break;
    case P4_PD_DC_LKP_IPV4_HASH1_ALGORITHM_CRC_32C: {
      *algorithm = SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC32_C;
    } break;
    case P4_PD_DC_LKP_IPV4_HASH1_ALGORITHM_CRC_32D: {
      *algorithm = SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC32_D;
    } break;
#endif
  }
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV4_DISABLE */
#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv4 hash algorithm get success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv4 hash algorithm get failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_non_ip_hash_algorithm_get(
    switch_device_t device, switch_hash_non_ip_algorithm_t *algorithm) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_L2_DISABLE
  p4_pd_dc_lkp_non_ip_hash1_algo_t pd_algo;

  pd_status = p4_pd_dc_hash_calc_lkp_non_ip_hash1_algorithm_get(
      switch_cfg_sess_hdl, device, &pd_algo);
  switch (pd_algo) {
#if defined(BMV2) && defined(INT_ENABLE)
    case P4_PD_DC_LKP_NON_IP_HASH1_ALGORITHM_CRC16_CUSTOM: {
      *algorithm = SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_CUSTOM;
    } break;
#elif !defined(P4_HASH_32BIT_ENABLE) /*16 Bit Hash Algorithm */
    case P4_PD_DC_LKP_NON_IP_HASH1_ALGORITHM_CRC16: {
      *algorithm = SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16;
    } break;
    case P4_PD_DC_LKP_NON_IP_HASH1_ALGORITHM_CRC_16_DECT: {
      *algorithm = SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_DECT;
    } break;
    case P4_PD_DC_LKP_NON_IP_HASH1_ALGORITHM_CRC_16_GENIBUS: {
      *algorithm = SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_GENIBUS;
    } break;
    case P4_PD_DC_LKP_NON_IP_HASH1_ALGORITHM_CRC_16_DNP: {
      *algorithm = SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_DNP;
    } break;
    case P4_PD_DC_LKP_NON_IP_HASH1_ALGORITHM_CRC_16_TELEDISK: {
      *algorithm = SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_TELEDISK;
    } break;
#else                                /* 32 Bit Hash Algorithm */
    case P4_PD_DC_LKP_NON_IP_HASH1_ALGORITHM_CRC32: {
      *algorithm = SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC32;
    } break;
    case P4_PD_DC_LKP_NON_IP_HASH1_ALGORITHM_CRC_32_BZIP2: {
      *algorithm = SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC32_BZIP2;
    } break;
    case P4_PD_DC_LKP_NON_IP_HASH1_ALGORITHM_CRC_32C: {
      *algorithm = SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC32_C;
    } break;
    case P4_PD_DC_LKP_NON_IP_HASH1_ALGORITHM_CRC_32D: {
      *algorithm = SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC32_D;
    } break;
#endif
  }
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);
#endif /* P4_L2_DISABLE */
#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "non ip hash algorithm get success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "non ip hash algorithm get failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_ipv6_hash_seed_get(switch_device_t device,
                                             uint64_t *seed) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_IPV6_DISABLE
  pd_status = p4_pd_dc_hash_calc_lkp_ipv6_hash1_seed_get(
      switch_cfg_sess_hdl, device, seed);
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV6_DISABLE */
#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv6 hash seed get success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv6 hash seed get failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_ipv4_hash_seed_get(switch_device_t device,
                                             uint64_t *seed) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_IPV4_DISABLE
  pd_status = p4_pd_dc_hash_calc_lkp_ipv4_hash1_seed_get(
      switch_cfg_sess_hdl, device, seed);
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_IPV4_DISABLE */
#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ipv4 hash seed get success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ipv4 hash seed get failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_non_ip_hash_seed_get(switch_device_t device,
                                               uint64_t *seed) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#ifndef P4_L2_DISABLE
  pd_status = p4_pd_dc_hash_calc_lkp_non_ip_hash1_seed_get(
      switch_cfg_sess_hdl, device, seed);
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L2_DISABLE */
#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "non ip hash seed get success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "non ip hash seed get failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_lag_hash_seed_get(switch_device_t device,
                                            uint64_t *seed) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
  status =
      p4_pd_dc_hash_calc_lag_hash_seed_get(switch_cfg_sess_hdl, device, seed);
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "lag hash seed get success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "lag hash seed get failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_ecmp_hash_seed_get(switch_device_t device,
                                             uint64_t *seed) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
  status =
      p4_pd_dc_hash_calc_ecmp_hash_seed_get(switch_cfg_sess_hdl, device, seed);
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* TARGET_TOFINO && !BMV2TOFINO  */
#endif /* SWITCH_PD */
  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "ecmp hash seed get success "
        "on device %d\n",
        device);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ecmp hash seed get failed "
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
