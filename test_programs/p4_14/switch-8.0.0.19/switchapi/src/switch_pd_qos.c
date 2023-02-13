/*
Copyright 2013-present Barefoot Networks, Inc.
*/

#include "switch_internal.h"
#include "switch_pd.h"

switch_status_t switch_pd_qos_default_entry_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  UNUSED(p4_pd_device);

#ifdef P4_QOS_CLASSIFICATION_ENABLE
  pd_status = p4_pd_dc_ingress_qos_map_dscp_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "qos map default add failed "
        "on device %d : table %s action %s",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  pd_status = p4_pd_dc_ingress_qos_map_pcp_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "qos map default add failed "
        "on device %d : table %s action %s",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  pd_status = p4_pd_dc_traffic_class_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "qos map default add failed "
        "on device %d : table %s action %s",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_QOS_CLASSIFICATION_ENABLE */

#ifdef P4_SS_QOS_CLASSIFICATION_ENABLE
  pd_status = p4_pd_dc_ingress_qos_map_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ingress qos map default add failed "
        "on device %d : table %s action %s",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_SS_QOS_CLASSIFICATION_ENABLE */

  goto cleanup;

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "qos map table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "qos map table entry default add failed "
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_qos_map_egress_default_entries_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

#if defined(P4_QOS_MARKING_ENABLE)
  p4_pd_dev_target_t p4_pd_device;
  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  pd_status = p4_pd_dc_egress_qos_map_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "qos map default add failed "
        "on device %d : table %s action %s",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  p4_pd_dc_egress_qos_map_match_spec_t match_spec;

  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.qos_metadata_egress_qos_group = 0x0;
  match_spec.qos_metadata_egress_qos_group_mask = 0x0;
  match_spec.qos_metadata_lkp_tc = 0x0;
  match_spec.qos_metadata_lkp_tc_mask = 0x0;
  match_spec.ipv4_valid = 0x1;
  match_spec.ipv4_valid_mask = 0x1;
#ifndef P4_IPV6_DISABLE
  match_spec.ipv6_valid = 0x0;
  match_spec.ipv6_valid_mask = 0x0;
#endif /* P4_IPV6_DISABLE */

  pd_status =
      p4_pd_dc_egress_qos_map_table_add_with_set_ip_dscp_marking_from_ipv4(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, 10000, &entry_hdl);

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress qos map table default entries add failed "
        "on device %d : table %s action %s",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#ifndef P4_IPV6_DISABLE
  match_spec.ipv4_valid = 0x0;
  match_spec.ipv4_valid_mask = 0x0;
  match_spec.ipv6_valid = 0x1;
  match_spec.ipv6_valid_mask = 0x1;

  pd_status =
      p4_pd_dc_egress_qos_map_table_add_with_set_ip_dscp_marking_from_ipv4(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, 10000, &entry_hdl);

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress qos map table default entries add failed "
        "on device %d : table %s action %s",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_IPV6_DISABLE */

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_QOS_MARKING_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress qos map default entries add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress qos map default entries add failed"
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_qos_map_ingress_entry_add(
    switch_device_t device,
    switch_qos_map_ingress_t qos_map_type,
    switch_qos_group_t qos_group_id,
    switch_qos_map_t *qos_map,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(qos_map_type);
  UNUSED(qos_group_id);
  UNUSED(qos_map);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  UNUSED(p4_pd_device);

  switch (qos_map_type) {
#ifdef P4_QOS_CLASSIFICATION_ENABLE
    case SWITCH_QOS_MAP_INGRESS_DSCP_TO_TC: {
      p4_pd_dc_ingress_qos_map_dscp_match_spec_t match_spec;
      p4_pd_dc_set_ingress_tc_action_spec_t action_spec;
      SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      match_spec.qos_metadata_ingress_qos_group = qos_group_id;
      match_spec.qos_metadata_ingress_qos_group_mask = 0xFF;
      match_spec.l3_metadata_lkp_dscp = (qos_map->dscp << 2);
      match_spec.l3_metadata_lkp_dscp_mask = 0xFC;
      action_spec.action_tc = qos_map->tc;
      pd_status = p4_pd_dc_ingress_qos_map_dscp_table_add_with_set_ingress_tc(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          1000,
          &action_spec,
          entry_hdl);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    case SWITCH_QOS_MAP_INGRESS_PCP_TO_TC: {
      p4_pd_dc_ingress_qos_map_pcp_match_spec_t match_spec;
      p4_pd_dc_set_ingress_tc_action_spec_t action_spec;
      match_spec.qos_metadata_ingress_qos_group = qos_group_id;
      match_spec.qos_metadata_ingress_qos_group_mask = 0xFF;
      match_spec.l2_metadata_lkp_pcp = qos_map->pcp;
      match_spec.l2_metadata_lkp_pcp_mask = 0xFF;
      action_spec.action_tc = qos_map->tc;
      pd_status = p4_pd_dc_ingress_qos_map_pcp_table_add_with_set_ingress_tc(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          1000,
          &action_spec,
          entry_hdl);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    case SWITCH_QOS_MAP_INGRESS_TOS_TO_TC: {
      p4_pd_dc_ingress_qos_map_dscp_match_spec_t match_spec;
      p4_pd_dc_set_ingress_tc_action_spec_t action_spec;
      SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      match_spec.qos_metadata_ingress_qos_group = qos_group_id;
      match_spec.qos_metadata_ingress_qos_group_mask = 0xFF;
      match_spec.l3_metadata_lkp_dscp = qos_map->tos;
      match_spec.l3_metadata_lkp_dscp_mask = 0xFF;
      action_spec.action_tc = qos_map->tc;
      pd_status = p4_pd_dc_ingress_qos_map_dscp_table_add_with_set_ingress_tc(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          1000,
          &action_spec,
          entry_hdl);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    case SWITCH_QOS_MAP_INGRESS_DSCP_TO_COLOR: {
      p4_pd_dc_ingress_qos_map_dscp_match_spec_t match_spec;
      p4_pd_dc_set_ingress_color_action_spec_t action_spec;
      SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      match_spec.qos_metadata_ingress_qos_group = qos_group_id;
      match_spec.qos_metadata_ingress_qos_group_mask = 0xFF;
      match_spec.l3_metadata_lkp_dscp = (qos_map->dscp << 2);
      match_spec.l3_metadata_lkp_dscp_mask = 0xFC;
      action_spec.action_color = qos_map->color;
      pd_status =
          p4_pd_dc_ingress_qos_map_dscp_table_add_with_set_ingress_color(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              1000,
              &action_spec,
              entry_hdl);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    case SWITCH_QOS_MAP_INGRESS_PCP_TO_COLOR: {
      p4_pd_dc_ingress_qos_map_pcp_match_spec_t match_spec;
      p4_pd_dc_set_ingress_color_action_spec_t action_spec;
      SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      match_spec.qos_metadata_ingress_qos_group = qos_group_id;
      match_spec.qos_metadata_ingress_qos_group_mask = 0xFF;
      match_spec.l2_metadata_lkp_pcp = qos_map->pcp;
      match_spec.l2_metadata_lkp_pcp_mask = 0xFF;
      action_spec.action_color = qos_map->color;
      pd_status = p4_pd_dc_ingress_qos_map_pcp_table_add_with_set_ingress_color(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          1000,
          &action_spec,
          entry_hdl);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    case SWITCH_QOS_MAP_INGRESS_TOS_TO_COLOR: {
      p4_pd_dc_ingress_qos_map_dscp_match_spec_t match_spec;
      p4_pd_dc_set_ingress_color_action_spec_t action_spec;
      SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      match_spec.qos_metadata_ingress_qos_group = qos_group_id;
      match_spec.qos_metadata_ingress_qos_group_mask = 0xFF;
      match_spec.l3_metadata_lkp_dscp = qos_map->tos;
      match_spec.l3_metadata_lkp_dscp_mask = 0xFF;
      action_spec.action_color = qos_map->color;
      pd_status =
          p4_pd_dc_ingress_qos_map_dscp_table_add_with_set_ingress_color(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              1000,
              &action_spec,
              entry_hdl);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    case SWITCH_QOS_MAP_INGRESS_TOS_TO_TC_AND_COLOR: {
      p4_pd_dc_ingress_qos_map_dscp_match_spec_t match_spec;
      p4_pd_dc_set_ingress_tc_and_color_action_spec_t action_spec;
      SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      match_spec.qos_metadata_ingress_qos_group = qos_group_id;
      match_spec.qos_metadata_ingress_qos_group_mask = 0xFF;
      match_spec.l3_metadata_lkp_dscp = qos_map->tos;
      match_spec.l3_metadata_lkp_dscp_mask = 0xFF;
      action_spec.action_tc = qos_map->tc;
      action_spec.action_color = qos_map->color;
      pd_status =
          p4_pd_dc_ingress_qos_map_dscp_table_add_with_set_ingress_tc_and_color(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              1000,
              &action_spec,
              entry_hdl);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    case SWITCH_QOS_MAP_INGRESS_DSCP_TO_TC_AND_COLOR: {
      p4_pd_dc_ingress_qos_map_dscp_match_spec_t match_spec;
      p4_pd_dc_set_ingress_tc_and_color_action_spec_t action_spec;
      SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      match_spec.qos_metadata_ingress_qos_group = qos_group_id;
      match_spec.qos_metadata_ingress_qos_group_mask = 0xFF;
      match_spec.l3_metadata_lkp_dscp = (qos_map->dscp << 2);
      match_spec.l3_metadata_lkp_dscp_mask = 0xFC;
      action_spec.action_tc = qos_map->tc;
      action_spec.action_color = qos_map->color;
      pd_status =
          p4_pd_dc_ingress_qos_map_dscp_table_add_with_set_ingress_tc_and_color(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              1000,
              &action_spec,
              entry_hdl);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    case SWITCH_QOS_MAP_INGRESS_PCP_TO_TC_AND_COLOR: {
      p4_pd_dc_ingress_qos_map_pcp_match_spec_t match_spec;
      p4_pd_dc_set_ingress_tc_and_color_action_spec_t action_spec;
      SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      match_spec.qos_metadata_ingress_qos_group = qos_group_id;
      match_spec.qos_metadata_ingress_qos_group_mask = 0xFF;
      match_spec.l2_metadata_lkp_pcp = qos_map->pcp;
      match_spec.l2_metadata_lkp_pcp_mask = 0xFF;
      action_spec.action_tc = qos_map->tc;
      action_spec.action_color = qos_map->color;
      pd_status =
          p4_pd_dc_ingress_qos_map_pcp_table_add_with_set_ingress_tc_and_color(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              1000,
              &action_spec,
              entry_hdl);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;

    case SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS: {
      p4_pd_dc_traffic_class_match_spec_t match_spec;
      p4_pd_dc_set_icos_action_spec_t action_spec;
      memset(&match_spec, 0, sizeof(p4_pd_dc_traffic_class_match_spec_t));
      memset(&action_spec, 0, sizeof(p4_pd_dc_set_icos_action_spec_t));
#ifndef SWITCH_GLOBAL_TC_ICOS_QUEUE_TABLE
      match_spec.qos_metadata_tc_qos_group = qos_group_id;
      match_spec.qos_metadata_tc_qos_group_mask = 0xFF;
#endif
      match_spec.qos_metadata_lkp_tc = qos_map->tc;
      match_spec.qos_metadata_lkp_tc_mask = 0xFF;
      action_spec.action_icos = qos_map->icos;
      pd_status =
          p4_pd_dc_traffic_class_table_add_with_set_icos(switch_cfg_sess_hdl,
                                                         p4_pd_device,
                                                         &match_spec,
                                                         1000,
                                                         &action_spec,
                                                         entry_hdl);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    case SWITCH_QOS_MAP_INGRESS_TC_TO_QUEUE: {
      p4_pd_dc_traffic_class_match_spec_t match_spec;
      p4_pd_dc_set_queue_action_spec_t action_spec;
      memset(&match_spec, 0, sizeof(p4_pd_dc_traffic_class_match_spec_t));
      memset(&action_spec, 0, sizeof(p4_pd_dc_set_queue_action_spec_t));
#ifndef SWITCH_GLOBAL_TC_ICOS_QUEUE_TABLE
      match_spec.qos_metadata_tc_qos_group = qos_group_id;
      match_spec.qos_metadata_tc_qos_group_mask = 0xFF;
#endif
      match_spec.qos_metadata_lkp_tc = qos_map->tc;
      match_spec.qos_metadata_lkp_tc_mask = 0xFF;
      action_spec.action_qid = qos_map->qid;
      pd_status =
          p4_pd_dc_traffic_class_table_add_with_set_queue(switch_cfg_sess_hdl,
                                                          p4_pd_device,
                                                          &match_spec,
                                                          1000,
                                                          &action_spec,
                                                          entry_hdl);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    case SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS_AND_QUEUE: {
      p4_pd_dc_traffic_class_match_spec_t match_spec;
      p4_pd_dc_set_icos_and_queue_action_spec_t action_spec;
      memset(&match_spec, 0, sizeof(p4_pd_dc_traffic_class_match_spec_t));
      memset(
          &action_spec, 0, sizeof(p4_pd_dc_set_icos_and_queue_action_spec_t));
#ifndef SWITCH_GLOBAL_TC_ICOS_QUEUE_TABLE
      match_spec.qos_metadata_tc_qos_group = qos_group_id;
      match_spec.qos_metadata_tc_qos_group_mask = 0xFF;
#endif
      match_spec.qos_metadata_lkp_tc = qos_map->tc;
      match_spec.qos_metadata_lkp_tc_mask = 0xFF;
      action_spec.action_qid = qos_map->qid;
      action_spec.action_icos = qos_map->icos;
      pd_status = p4_pd_dc_traffic_class_table_add_with_set_icos_and_queue(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          1000,
          &action_spec,
          entry_hdl);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
#endif /* P4_QOS_CLASSIFICATION_ENABLE */

#ifdef P4_SS_QOS_CLASSIFICATION_ENABLE
    case SWITCH_QOS_MAP_INGRESS_DSCP_TO_QID_AND_TC_AND_COLOR: {
      p4_pd_dc_ingress_qos_map_match_spec_t match_spec;
      p4_pd_dc_set_ingress_qid_and_tc_and_color_action_spec_t action_spec;
      SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      match_spec.qos_metadata_ingress_qos_group = qos_group_id;
      match_spec.qos_metadata_ingress_qos_group_mask = 0xFF;
      match_spec.l3_metadata_lkp_dscp = (qos_map->dscp << 2);
      match_spec.l3_metadata_lkp_dscp_mask = 0xFC;
      match_spec.qos_metadata_trust_dscp = 0x1;
      match_spec.qos_metadata_trust_dscp_mask = 0xFF;

      action_spec.action_qid = qos_map->qid;
      action_spec.action_tc = qos_map->tc;
      action_spec.action_color = qos_map->color;
      pd_status =
          p4_pd_dc_ingress_qos_map_table_add_with_set_ingress_qid_and_tc_and_color(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              1000,
              &action_spec,
              entry_hdl);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;

    case SWITCH_QOS_MAP_INGRESS_TOS_TO_QID_AND_TC_AND_COLOR: {
      p4_pd_dc_ingress_qos_map_match_spec_t match_spec;
      p4_pd_dc_set_ingress_qid_and_tc_and_color_action_spec_t action_spec;
      SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      match_spec.qos_metadata_ingress_qos_group = qos_group_id;
      match_spec.qos_metadata_ingress_qos_group_mask = 0xFF;
      match_spec.l3_metadata_lkp_dscp = qos_map->tos;
      match_spec.l3_metadata_lkp_dscp_mask = 0xFC;
      match_spec.qos_metadata_trust_dscp = 0x1;
      match_spec.qos_metadata_trust_dscp_mask = 0xFF;

      action_spec.action_qid = qos_map->qid;
      action_spec.action_tc = qos_map->tc;
      action_spec.action_color = qos_map->color;
      pd_status =
          p4_pd_dc_ingress_qos_map_table_add_with_set_ingress_qid_and_tc_and_color(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              1000,
              &action_spec,
              entry_hdl);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;

    case SWITCH_QOS_MAP_INGRESS_PCP_TO_QID_AND_TC_COLOR: {
      p4_pd_dc_ingress_qos_map_match_spec_t match_spec;
      p4_pd_dc_set_ingress_qid_and_tc_and_color_action_spec_t action_spec;
      SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      match_spec.qos_metadata_ingress_qos_group = qos_group_id;
      match_spec.qos_metadata_ingress_qos_group_mask = 0xFF;
      match_spec.l2_metadata_lkp_pcp = qos_map->pcp;
      match_spec.l2_metadata_lkp_pcp_mask = 0xFF;
      match_spec.qos_metadata_trust_pcp = 0x1;
      match_spec.qos_metadata_trust_pcp_mask = 0xFF;
      match_spec.qos_metadata_trust_dscp = 0x0;
      match_spec.qos_metadata_trust_dscp_mask = 0xFF;

      action_spec.action_qid = qos_map->qid;
      action_spec.action_tc = qos_map->tc;
      action_spec.action_color = qos_map->color;
      pd_status =
          p4_pd_dc_ingress_qos_map_table_add_with_set_ingress_qid_and_tc_and_color(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &match_spec,
              1000,
              &action_spec,
              entry_hdl);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
#endif /* P4_SS_QOS_CLASSIFICATION_ENABLE */

    default:
      status = SWITCH_STATUS_INVALID_PARAMETER;
      goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "qos map entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "qos map entry add failed"
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_qos_map_ingress_entry_update(
    switch_device_t device,
    switch_qos_map_ingress_t qos_map_type,
    switch_qos_group_t qos_group_id,
    switch_qos_map_t *qos_map,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(qos_map_type);
  UNUSED(qos_group_id);
  UNUSED(qos_map);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  UNUSED(p4_pd_device);

  switch (qos_map_type) {
#ifdef P4_QOS_CLASSIFICATION_ENABLE
    case SWITCH_QOS_MAP_INGRESS_DSCP_TO_TC:
    case SWITCH_QOS_MAP_INGRESS_TOS_TO_TC: {
      p4_pd_dc_set_ingress_tc_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
      action_spec.action_tc = qos_map->tc;
      pd_status =
          p4_pd_dc_ingress_qos_map_dscp_table_modify_with_set_ingress_tc(
              switch_cfg_sess_hdl,
              p4_pd_device.device_id,
              entry_hdl,
              &action_spec);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    case SWITCH_QOS_MAP_INGRESS_PCP_TO_TC: {
      p4_pd_dc_set_ingress_tc_action_spec_t action_spec;
      action_spec.action_tc = qos_map->tc;
      pd_status = p4_pd_dc_ingress_qos_map_pcp_table_modify_with_set_ingress_tc(
          switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl, &action_spec);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    case SWITCH_QOS_MAP_INGRESS_DSCP_TO_COLOR:
    case SWITCH_QOS_MAP_INGRESS_TOS_TO_COLOR: {
      p4_pd_dc_set_ingress_color_action_spec_t action_spec;
      action_spec.action_color = qos_map->color;
      pd_status =
          p4_pd_dc_ingress_qos_map_dscp_table_modify_with_set_ingress_color(
              switch_cfg_sess_hdl,
              p4_pd_device.device_id,
              entry_hdl,
              &action_spec);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    case SWITCH_QOS_MAP_INGRESS_PCP_TO_COLOR: {
      p4_pd_dc_set_ingress_color_action_spec_t action_spec;
      action_spec.action_color = qos_map->color;
      pd_status =
          p4_pd_dc_ingress_qos_map_pcp_table_modify_with_set_ingress_color(
              switch_cfg_sess_hdl,
              p4_pd_device.device_id,
              entry_hdl,
              &action_spec);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    case SWITCH_QOS_MAP_INGRESS_DSCP_TO_TC_AND_COLOR:
    case SWITCH_QOS_MAP_INGRESS_TOS_TO_TC_AND_COLOR: {
      p4_pd_dc_set_ingress_tc_and_color_action_spec_t action_spec;
      action_spec.action_tc = qos_map->tc;
      action_spec.action_color = qos_map->color;
      pd_status =
          p4_pd_dc_ingress_qos_map_dscp_table_modify_with_set_ingress_tc_and_color(
              switch_cfg_sess_hdl,
              p4_pd_device.device_id,
              entry_hdl,
              &action_spec);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    case SWITCH_QOS_MAP_INGRESS_PCP_TO_TC_AND_COLOR: {
      p4_pd_dc_set_ingress_tc_and_color_action_spec_t action_spec;
      action_spec.action_tc = qos_map->tc;
      action_spec.action_color = qos_map->color;
      pd_status =
          p4_pd_dc_ingress_qos_map_pcp_table_modify_with_set_ingress_tc_and_color(
              switch_cfg_sess_hdl,
              p4_pd_device.device_id,
              entry_hdl,
              &action_spec);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;

    case SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS: {
      p4_pd_dc_set_icos_action_spec_t action_spec;
      memset(&action_spec, 0, sizeof(p4_pd_dc_set_icos_action_spec_t));
      action_spec.action_icos = qos_map->icos;
      pd_status = p4_pd_dc_traffic_class_table_modify_with_set_icos(
          switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl, &action_spec);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    case SWITCH_QOS_MAP_INGRESS_TC_TO_QUEUE: {
      p4_pd_dc_set_queue_action_spec_t action_spec;
      memset(&action_spec, 0, sizeof(p4_pd_dc_set_queue_action_spec_t));
      action_spec.action_qid = qos_map->qid;
      pd_status = p4_pd_dc_traffic_class_table_modify_with_set_queue(
          switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl, &action_spec);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    case SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS_AND_QUEUE: {
      p4_pd_dc_set_icos_and_queue_action_spec_t action_spec;
      memset(
          &action_spec, 0, sizeof(p4_pd_dc_set_icos_and_queue_action_spec_t));
      action_spec.action_qid = qos_map->qid;
      action_spec.action_icos = qos_map->icos;
      pd_status = p4_pd_dc_traffic_class_table_modify_with_set_icos_and_queue(
          switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl, &action_spec);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
#endif /* P4_QOS_CLASSIFICATION_ENABLE */

#ifdef P4_SS_QOS_CLASSIFICATION_ENABLE
    case SWITCH_QOS_MAP_INGRESS_DSCP_TO_QID_AND_TC_AND_COLOR:
    case SWITCH_QOS_MAP_INGRESS_TOS_TO_QID_AND_TC_AND_COLOR:
    case SWITCH_QOS_MAP_INGRESS_PCP_TO_QID_AND_TC_COLOR: {
      p4_pd_dc_set_ingress_qid_and_tc_and_color_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));

      action_spec.action_qid = qos_map->qid;
      action_spec.action_tc = qos_map->tc;
      action_spec.action_color = qos_map->color;
      pd_status =
          p4_pd_dc_ingress_qos_map_table_modify_with_set_ingress_qid_and_tc_and_color(
              switch_cfg_sess_hdl,
              p4_pd_device.device_id,
              entry_hdl,
              &action_spec);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;

#endif /* P4_SS_QOS_CLASSIFICATION_ENABLE */

    default:
      status = SWITCH_STATUS_INVALID_PARAMETER;
      goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "qos map entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "qos map entry add failed"
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_qos_map_ingress_entry_delete(
    switch_device_t device,
    switch_qos_map_ingress_t qos_map_type,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(qos_map_type);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  switch (qos_map_type) {
#ifdef P4_QOS_CLASSIFICATION_ENABLE
    case SWITCH_QOS_MAP_INGRESS_DSCP_TO_TC:
    case SWITCH_QOS_MAP_INGRESS_DSCP_TO_COLOR:
    case SWITCH_QOS_MAP_INGRESS_DSCP_TO_TC_AND_COLOR:
    case SWITCH_QOS_MAP_INGRESS_TOS_TO_TC:
    case SWITCH_QOS_MAP_INGRESS_TOS_TO_COLOR:
    case SWITCH_QOS_MAP_INGRESS_TOS_TO_TC_AND_COLOR:
      pd_status = p4_pd_dc_ingress_qos_map_dscp_table_delete(
          switch_cfg_sess_hdl, device, entry_hdl);
      break;
    case SWITCH_QOS_MAP_INGRESS_PCP_TO_TC:
    case SWITCH_QOS_MAP_INGRESS_PCP_TO_COLOR:
    case SWITCH_QOS_MAP_INGRESS_PCP_TO_TC_AND_COLOR:
      pd_status = p4_pd_dc_ingress_qos_map_pcp_table_delete(
          switch_cfg_sess_hdl, device, entry_hdl);
      break;
    case SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS:
    case SWITCH_QOS_MAP_INGRESS_TC_TO_QUEUE:
    case SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS_AND_QUEUE:
      pd_status = p4_pd_dc_traffic_class_table_delete(
          switch_cfg_sess_hdl, device, entry_hdl);
      break;
#endif /* P4_QOS_CLASSIFICATION_ENABLE */
#ifdef P4_SS_QOS_CLASSIFICATION_ENABLE
    case SWITCH_QOS_MAP_INGRESS_DSCP_TO_QID_AND_TC_AND_COLOR:
    case SWITCH_QOS_MAP_INGRESS_TOS_TO_QID_AND_TC_AND_COLOR:
    case SWITCH_QOS_MAP_INGRESS_PCP_TO_QID_AND_TC_AND_COLOR:
      pd_status = p4_pd_dc_ingress_qos_map_table_delete(
          switch_cfg_sess_hdl, device, entry_hdl);
      break;
#endif /* P4_SS_QOS_CLASSIFICATION_ENABLE */
    default:
      status = SWITCH_STATUS_INVALID_PARAMETER;
      break;
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DELETE;
    pd_entry.match_spec_size = 0;
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ingress qos map table entry delete failed "
        "on device %d : table %s action %s",
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
        "ingress qos map table entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "ingress qos map table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_qos_map_egress_entry_add(
    switch_device_t device,
    switch_qos_map_egress_t qos_map_type,
    switch_qos_group_t qos_group_id,
    switch_qos_map_t *qos_map,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(qos_map_type);
  UNUSED(qos_group_id);
  UNUSED(qos_map);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(P4_QOS_MARKING_ENABLE)

  p4_pd_dc_egress_qos_map_match_spec_t match_spec;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.qos_metadata_egress_qos_group = qos_group_id;
  match_spec.qos_metadata_egress_qos_group_mask = 0xFF;
  match_spec.qos_metadata_lkp_tc = qos_map->tc;
  match_spec.qos_metadata_lkp_tc_mask = 0xFF;
  match_spec.ipv4_valid = 0x0;
  match_spec.ipv4_valid_mask = 0x0;
#ifndef P4_IPV6_DISABLE
  match_spec.ipv6_valid = 0x0;
  match_spec.ipv6_valid_mask = 0x0;
#endif /* P4_IPV6_DISABLE */

  switch (qos_map_type) {
    case SWITCH_QOS_MAP_EGRESS_TC_TO_DSCP:
    case SWITCH_QOS_MAP_EGRESS_COLOR_TO_DSCP:
    case SWITCH_QOS_MAP_EGRESS_TC_AND_COLOR_TO_DSCP: {
      p4_pd_dc_set_ip_dscp_marking_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_dscp = (qos_map->dscp << 2);
      pd_status = p4_pd_dc_egress_qos_map_table_add_with_set_ip_dscp_marking(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          1000,
          &action_spec,
          entry_hdl);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    case SWITCH_QOS_MAP_EGRESS_TC_TO_TOS:
    case SWITCH_QOS_MAP_EGRESS_COLOR_TO_TOS:
    case SWITCH_QOS_MAP_EGRESS_TC_AND_COLOR_TO_TOS: {
      p4_pd_dc_set_ip_dscp_marking_action_spec_t action_spec;
      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
      action_spec.action_dscp = qos_map->tos;
      pd_status = p4_pd_dc_egress_qos_map_table_add_with_set_ip_dscp_marking(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          1000,
          &action_spec,
          entry_hdl);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
        pd_entry.match_spec_size = sizeof(match_spec);
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "qos map table add failed "
            "on device %d : table %s action %s",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } break;
    //    case SWITCH_QOS_MAP_EGRESS_TC_TO_PCP:
    //    case SWITCH_QOS_MAP_EGRESS_COLOR_TO_PCP:
    //    case SWITCH_QOS_MAP_EGRESS_TC_AND_COLOR_TO_PCP: {
    //      p4_pd_dc_set_vlan_pcp_marking_action_spec_t action_spec;
    //      SWITCH_MEMSET(&action_spec, 0, sizeof(action_spec));
    //      action_spec.action_pcp = qos_map->pcp;
    //      pd_status =
    //      p4_pd_dc_egress_qos_map_table_add_with_set_vlan_pcp_marking(
    //          switch_cfg_sess_hdl, p4_pd_device, &match_spec, 1000,
    //          &action_spec,
    //          entry_hdl);
    //
    //      if (switch_pd_log_level_debug()) {
    //        switch_pd_dump_entry_t pd_entry;
    //        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    //        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    //        pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    //        pd_entry.match_spec_size = sizeof(match_spec);
    //        pd_entry.action_spec_size = 0;
    //        pd_entry.pd_hdl = *entry_hdl;
    //        switch_pd_entry_dump(device, &pd_entry);
    //      }
    //
    //      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    //        SWITCH_PD_LOG_ERROR(
    //            "qos map table add failed "
    //            "on device %d : table %s action %s",
    //            device,
    //            switch_pd_table_id_to_string(0),
    //            switch_pd_action_id_to_string(0));
    //        goto cleanup;
    //      }
    //    } break;
    default:
      status = SWITCH_STATUS_INVALID_PARAMETER;
  }
cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_QOS_MARKING_ENABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "qos map entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "qos map entry add failed"
        "on device %d : %s (pd: 0x%x)",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_qos_map_egress_entry_delete(
    switch_device_t device,
    switch_qos_map_egress_t qos_map_type,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(qos_map_type);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if defined(P4_QOS_MARKING_ENABLE)

  pd_status = p4_pd_dc_egress_qos_map_table_delete(
      switch_cfg_sess_hdl, device, entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DELETE;
    pd_entry.match_spec_size = 0;
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "egress qos map table entry delete failed "
        "on device %d : table %s action %s",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_QOS_MARKING_ENABLE*/
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "egress qos map table entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "egress qos map table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}
