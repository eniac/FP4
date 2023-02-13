#include "switch_internal.h"

CLISH_PLUGIN_SYM(switchapi_show_handle) {
  char *device_str = NULL;
  const char *handle_type_str = NULL;
  const char *handle_str = NULL;
  clish_pargv_t *pargv = NULL;
  const clish_parg_t *parg = NULL;
  switch_device_t device = 0;
  switch_handle_type_t handle_type = SWITCH_HANDLE_TYPE_NONE;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  device_str = (char *)clish_shell_expand_var_ex(
      "device_id", clish_context, SHELL_EXPAND_VIEW);
  if (!device_str) {
    SWITCH_PRINT(clish_context, "device id null");
    return 0;
  }

  device = atoi(device_str);

  pargv = clish_context__get_pargv(clish_context);

  parg = clish_pargv_find_arg(pargv, "handle_type");
  if (parg) {
    handle_type_str = clish_parg__get_value(parg);
    if (handle_type_str) {
      handle_type = atoi(handle_type_str);
    }
  }

  parg = clish_pargv_find_arg(pargv, "handle_value");
  if (parg) {
    handle_str = clish_parg__get_value(parg);
    if (handle_str) {
      handle = atoi(handle_str);
    }
  }

  if (handle_type >= SWITCH_HANDLE_TYPE_MAX) {
    SWITCH_PRINT(clish_context, "handle type invalid: %s\n", handle_type_str);
    return 0;
  }

  if (handle_str) {
    SWITCH_ASSERT(handle != SWITCH_API_INVALID_HANDLE);
    if (handle != SWITCH_API_INVALID_HANDLE) {
      SWITCH_PRINT(clish_context, "handle invalid: %s\n", handle_str);
      return 0;
    }

    status = switch_api_handle_dump(device, handle, clish_context);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "handle dump failed on device %d "
          "handle type %s handle 0x%lx:(%s)\n",
          device,
          handle_type_str,
          handle,
          switch_error_to_string(status));
      SWITCH_PRINT(clish_context,
                   "api handle dump failed: (%s)\n",
                   switch_error_to_string(status));
      return 0;
    }
  } else {
    status = switch_api_handle_dump_all(device, handle_type, clish_context);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "handle dump all failed on device %d "
          "handle type %s: (%s)\n",
          device,
          handle_type_str,
          switch_error_to_string(status));
      SWITCH_PRINT(clish_context,
                   "api handle dump all failed: (%s)\n",
                   switch_error_to_string(status));
      return 0;
    }
  }

  bfshell_string_free(device_str);
  return 0;
}

CLISH_PLUGIN_SYM(switchapi_show_device) {
  char *device_str = NULL;
  clish_pargv_t *pargv = NULL;
  const char *api_type_str = NULL;
  const clish_parg_t *parg = NULL;
  switch_api_type_t api_type = 0;
  switch_device_t device = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  device_str = (char *)clish_shell_expand_var_ex(
      "device_id", clish_context, SHELL_EXPAND_VIEW);
  if (!device_str) {
    SWITCH_PRINT(clish_context, "device id null");
    return 0;
  }

  device = atoi(device_str);

  pargv = clish_context__get_pargv(clish_context);

  if (clish_pargv_find_arg(pargv, "info")) {
    status = switch_api_device_dump(device, clish_context);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("device dump failed on device %d:(%s)\n",
                       device,
                       switch_error_to_string(status));
      SWITCH_PRINT(clish_context,
                   "device dump failed: (%s)\n",
                   switch_error_to_string(status));
      return 0;
    }
  } else {
    parg = clish_pargv_find_arg(pargv, "api_type");
    if (parg) {
      api_type_str = clish_parg__get_value(parg);
      if (api_type_str) {
        api_type = atoi(api_type_str);
      }
    }
    status = switch_api_device_api_dump(device, api_type, clish_context);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("device dump failed on device %d:(%s)\n",
                       device,
                       switch_error_to_string(status));
      SWITCH_PRINT(clish_context,
                   "device dump failed: (%s)\n",
                   switch_error_to_string(status));
      return 0;
    }
  }

  bfshell_string_free(device_str);
  return 0;
}

CLISH_PLUGIN_SYM(switchapi_show_table_info) {
  char *device_str = NULL;
  clish_pargv_t *pargv = NULL;
  switch_device_t device = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  device_str = (char *)clish_shell_expand_var_ex(
      "device_id", clish_context, SHELL_EXPAND_VIEW);
  if (!device_str) {
    SWITCH_PRINT(clish_context, "device id null");
    return 0;
  }

  device = atoi(device_str);
  pargv = clish_context__get_pargv(clish_context);

  if (clish_pargv_find_arg(pargv, "info")) {
    status = switch_api_table_sizes_dump(device, clish_context);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("table info dump failed on device %d:(%s)\n",
                       device,
                       switch_error_to_string(status));
      SWITCH_PRINT(clish_context,
                   "table info dump failed: (%s)\n",
                   switch_error_to_string(status));
      return 0;
    }
  }

  bfshell_string_free(device_str);
  return 0;
}

CLISH_PLUGIN_SYM(switchapi_show_config_info) {
  char *device_str = NULL;
  switch_device_t device = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  device_str = (char *)clish_shell_expand_var_ex(
      "device_id", clish_context, SHELL_EXPAND_VIEW);
  if (!device_str) {
    SWITCH_PRINT(clish_context, "device id null");
    return 0;
  }

  device = atoi(device_str);

  status = switch_api_config_dump(device, clish_context);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("config info dump failed on device %d:(%s)\n",
                     device,
                     switch_error_to_string(status));
    SWITCH_PRINT(clish_context,
                 "config info dump failed: (%s)\n",
                 switch_error_to_string(status));
    return 0;
  }

  bfshell_string_free(device_str);
  return 0;
}

CLISH_PLUGIN_SYM(switchapi_show_port_dump) {
  char *device_str = NULL;
  clish_pargv_t *pargv = NULL;
  const clish_parg_t *parg = NULL;
  const char *port_str = NULL;
  const char *handle_str = NULL;
  switch_device_t device = 0;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_port_t port = 0;
  bool handle_valid = FALSE;
  bool port_valid = FALSE;
  bool stats_dump = FALSE;
  bool info_dump = FALSE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  device_str = (char *)clish_shell_expand_var_ex(
      "device_id", clish_context, SHELL_EXPAND_VIEW);
  if (!device_str) {
    SWITCH_PRINT(clish_context, "device id null");
    return 0;
  }

  device = atoi(device_str);
  pargv = clish_context__get_pargv(clish_context);

  parg = clish_pargv_find_arg(pargv, "stats");
  if (parg) {
    stats_dump = TRUE;
  }

  parg = clish_pargv_find_arg(pargv, "info");
  if (parg) {
    info_dump = TRUE;
  }

  parg = clish_pargv_find_arg(pargv, "handle_value");
  if (parg) {
    handle_str = clish_parg__get_value(parg);
    if (handle_str) {
      handle = atoi(handle_str);
    }
    handle_valid = TRUE;
  }

  parg = clish_pargv_find_arg(pargv, "port_num");
  if (parg) {
    port_str = clish_parg__get_value(parg);
    if (port_str) {
      port = atoi(port_str);
    }
    port_valid = TRUE;
  }

  if (stats_dump) {
    if (handle_valid) {
      status = switch_api_port_stats_dump(device, handle, clish_context);
    } else if (port_valid) {
      status = switch_api_port_stats_by_port_number_dump(
          device, port, clish_context);
    }
  } else if (info_dump) {
    if (handle_valid) {
      status = switch_api_port_handle_dump(device, handle, clish_context);
    } else if (port_valid) {
      status =
          switch_api_port_info_by_port_number_dump(device, port, clish_context);
    }
  }

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port stats dump failed on device %d:(%s)\n",
                     device,
                     switch_error_to_string(status));
    SWITCH_PRINT(clish_context,
                 "port stats dump failed: (%s)\n",
                 switch_error_to_string(status));
    return 0;
  }

  return 0;
}

CLISH_PLUGIN_SYM(switchapi_port_add_delete) {
  char *device_str = NULL;
  switch_device_t device = 0;
  clish_pargv_t *pargv = NULL;
  const clish_parg_t *parg = NULL;
  const char *port_str = NULL;
  const char *speed_str = NULL;
  switch_port_speed_t port_speed = 0;
  switch_port_t port = 0;
  switch_api_port_info_t api_port_info;
  switch_handle_t port_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  device_str = (char *)clish_shell_expand_var_ex(
      "device_id", clish_context, SHELL_EXPAND_VIEW);
  if (!device_str) {
    SWITCH_PRINT(clish_context, "device id null");
    return 0;
  }

  device = atoi(device_str);
  pargv = clish_context__get_pargv(clish_context);

  if (clish_pargv_find_arg(pargv, "add") ||
      clish_pargv_find_arg(pargv, "delete") ||
      clish_pargv_find_arg(pargv, "enable") ||
      clish_pargv_find_arg(pargv, "disable")) {
    parg = clish_pargv_find_arg(pargv, "port_num");
    port_str = clish_parg__get_value(parg);
    if (port_str) {
      port = atoi(port_str);
    }
    if (!clish_pargv_find_arg(pargv, "add")) {
      status = switch_api_port_id_to_handle_get(device, port, &port_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "port add delete cli failed on device %d port num %d: "
            "port id to handle get failed:(%s)\n",
            device,
            port,
            switch_error_to_string(status));
        return status;
      }
    }
  }

  if (clish_pargv_find_arg(pargv, "add")) {
    parg = clish_pargv_find_arg(pargv, "port_speed");
    speed_str = clish_parg__get_value(parg);
    if (speed_str) {
      port_speed = atoi(speed_str);
    }

    SWITCH_MEMSET(&api_port_info, 0x0, sizeof(api_port_info));
    api_port_info.port = port;
    api_port_info.port_speed = port_speed;
    status = switch_api_port_add(device, &api_port_info, &port_handle);
  } else if (clish_pargv_find_arg(pargv, "delete")) {
    status = switch_api_port_delete(device, port_handle);
  } else if (clish_pargv_find_arg(pargv, "enable")) {
    status = switch_api_port_admin_state_set(device, port_handle, TRUE);
  } else if (clish_pargv_find_arg(pargv, "disable")) {
    status = switch_api_port_admin_state_set(device, port_handle, FALSE);
  } else {
  }

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port add delete cli failed on device %d: %s\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;
}

CLISH_PLUGIN_SYM(switchapi_show_packet_driver_dump) {
  char *device_str = NULL;
  switch_device_t device = 0;
  clish_pargv_t *pargv = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  device_str = (char *)clish_shell_expand_var_ex(
      "device_id", clish_context, SHELL_EXPAND_VIEW);
  if (!device_str) {
    SWITCH_PRINT(clish_context, "device id null");
    return 0;
  }

  device = atoi(device_str);
  pargv = clish_context__get_pargv(clish_context);

  if (clish_pargv_find_arg(pargv, "rc-cpu-counters")) {
    status = switch_pktdriver_rx_rc_counters_dump(device, clish_context);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("show packet rx/tx counters failed on device %d: \n",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  } else if (clish_pargv_find_arg(pargv, "port-cpu-counters")) {
    status = switch_pktdriver_rx_port_counters_dump(device, clish_context);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("show packet rx/tx counters failed on device %d: \n",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  } else if (clish_pargv_find_arg(pargv, "total-cpu-counters")) {
    status = switch_pktdriver_rx_total_counters_dump(device, clish_context);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("show packet rx/tx counters failed on device %d: \n",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  } else {
    status = switch_pktdriver_bd_mapping_dump(device, clish_context);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("show packet bd mapping dump failed on device %d: \n",
                       device,
                       switch_error_to_string(status));
      return status;
    }
  }

  return status;
}

CLISH_PLUGIN_SYM(switchapi_show_vlan_dump) {
  char *device_str = NULL;
  clish_pargv_t *pargv = NULL;
  const clish_parg_t *parg = NULL;
  const char *vlan_str = NULL;
  const char *handle_str = NULL;
  switch_device_t device = 0;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_vlan_t vlan = 0;
  bool handle_valid = FALSE;
  bool vlan_valid = FALSE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  device_str = (char *)clish_shell_expand_var_ex(
      "device_id", clish_context, SHELL_EXPAND_VIEW);
  if (!device_str) {
    SWITCH_PRINT(clish_context, "device id null");
    return 0;
  }

  device = atoi(device_str);
  pargv = clish_context__get_pargv(clish_context);

  parg = clish_pargv_find_arg(pargv, "handle_value");
  if (parg) {
    handle_str = clish_parg__get_value(parg);
    if (handle_str) {
      handle = atoi(handle_str);
    }
    handle_valid = TRUE;
  }

  parg = clish_pargv_find_arg(pargv, "vlan_id");
  if (parg) {
    vlan_str = clish_parg__get_value(parg);
    if (vlan_str) {
      vlan = atoi(vlan_str);
    }
    vlan_valid = TRUE;
  }

  if (handle_valid) {
    status = switch_api_vlan_handle_dump(device, handle, clish_context);
  } else if (vlan_valid) {
    status = switch_api_vlan_id_dump(device, vlan, clish_context);
  }

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("vlan dump failed on device %d:(%s)\n",
                     device,
                     switch_error_to_string(status));
    SWITCH_PRINT(clish_context,
                 "vlan dump failed: (%s)\n",
                 switch_error_to_string(status));
    return 0;
  }

  return 0;
}

CLISH_PLUGIN_SYM(switchapi_show_hostif_interface) {
  char *device_str = NULL;
  clish_pargv_t *pargv = NULL;
  const clish_parg_t *parg = NULL;
  const char *hostif_str = NULL;
  switch_device_t device = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  device_str = (char *)clish_shell_expand_var_ex(
      "device_id", clish_context, SHELL_EXPAND_VIEW);
  if (!device_str) {
    SWITCH_PRINT(clish_context, "device id null");
    return 0;
  }

  device = atoi(device_str);
  pargv = clish_context__get_pargv(clish_context);

  parg = clish_pargv_find_arg(pargv, "intf-name");
  if (!parg) {
    SWITCH_PRINT(clish_context, "interface name null");
    return 0;
  }

  hostif_str = clish_parg__get_value(parg);
  if (!hostif_str) {
    SWITCH_PRINT(clish_context, "hostif name null");
    return 0;
  }

  status = switch_api_hostif_by_name_dump(device, hostif_str, clish_context);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("hostif dump failed on device %d:(%s)\n",
                     device,
                     switch_error_to_string(status));
    SWITCH_PRINT(clish_context,
                 "hostif dump failed: (%s)\n",
                 switch_error_to_string(status));
    return 0;
  }

  return status;
}

CLISH_PLUGIN_SYM(switchapi_debug_pktdriver_trace_enable_disable) {
  char *device_str = NULL;
  clish_pargv_t *pargv = NULL;
  switch_device_t device = 0;
  bool enable = FALSE;
  bool rx = FALSE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  device_str = (char *)clish_shell_expand_var_ex(
      "device_id", clish_context, SHELL_EXPAND_VIEW);
  if (!device_str) {
    SWITCH_PRINT(clish_context, "device id null");
    return 0;
  }

  device = atoi(device_str);
  pargv = clish_context__get_pargv(clish_context);

  if (clish_pargv_find_arg(pargv, "rx")) {
    rx = TRUE;
  }

  if (clish_pargv_find_arg(pargv, "enable")) {
    enable = TRUE;
  }

  status =
      switch_pktdriver_rx_tx_debug_enable(device, rx, enable, clish_context);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("rx tx debug enable/disable failed on device %d:(%s)\n",
                     device,
                     switch_error_to_string(status));
    SWITCH_PRINT(clish_context,
                 "rx tx debug enable/disable failed on device %d:(%s)\n",
                 switch_error_to_string(status));
    return 0;
  }

  return status;
}

CLISH_PLUGIN_SYM(switchapi_debug_log_level_set) {
  char *device_str = NULL;
  clish_pargv_t *pargv = NULL;
  switch_device_t device = 0;
  const clish_parg_t *parg = NULL;
  const char *log_level_str = NULL;
  switch_log_level_t log_level = SWITCH_LOG_LEVEL_NONE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  device_str = (char *)clish_shell_expand_var_ex(
      "device_id", clish_context, SHELL_EXPAND_VIEW);
  if (!device_str) {
    SWITCH_PRINT(clish_context, "device id null");
    return 0;
  }

  device = atoi(device_str);
  pargv = clish_context__get_pargv(clish_context);

  parg = clish_pargv_find_arg(pargv, "log_level");
  if (parg) {
    log_level_str = clish_parg__get_value(parg);
    if (log_level_str) {
      log_level = atoi(log_level_str);
    }
  }

  status = switch_api_log_level_all_set(log_level);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("debug log level set failed on device %d:(%s)\n",
                     device,
                     switch_error_to_string(status));
    SWITCH_PRINT(clish_context,
                 "debug log level set failed on device %d:(%s)\n",
                 switch_error_to_string(status));
    return 0;
  }

  return status;
}

CLISH_PLUGIN_SYM(switchapi_port_stats_clear) {
  char *device_str = NULL;
  clish_pargv_t *pargv = NULL;
  const clish_parg_t *parg = NULL;
  const char *port_str = NULL;
  const char *handle_str = NULL;
  switch_device_t device = 0;
  switch_handle_t port_handle = SWITCH_API_INVALID_HANDLE;
  switch_port_t port = 0;
  bool port_valid = FALSE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  device_str = (char *)clish_shell_expand_var_ex(
      "device_id", clish_context, SHELL_EXPAND_VIEW);
  if (!device_str) {
    SWITCH_PRINT(clish_context, "device id null");
    return 0;
  }

  device = atoi(device_str);
  pargv = clish_context__get_pargv(clish_context);

  parg = clish_pargv_find_arg(pargv, "handle_value");
  if (parg) {
    handle_str = clish_parg__get_value(parg);
    if (handle_str) {
      port_handle = atoi(handle_str);
    }
  }

  parg = clish_pargv_find_arg(pargv, "port_num");
  if (parg) {
    port_str = clish_parg__get_value(parg);
    if (port_str) {
      port = atoi(port_str);
    }
    port_valid = TRUE;
  }

  if (port_valid) {
    status = switch_api_port_id_to_handle_get(device, port, &port_handle);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  }

  SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
  status = switch_api_port_stats_clear(device, port_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("port clear stats failed on device %d:(%s)\n",
                     device,
                     switch_error_to_string(status));
    SWITCH_PRINT(clish_context,
                 "port clear stats failed: (%s)\n",
                 switch_error_to_string(status));
    return 0;
  }

  return 0;
}

CLISH_PLUGIN_SYM(switchapi_show_drop_stats) {
  char *device_str = NULL;
  switch_device_t device = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  device_str = (char *)clish_shell_expand_var_ex(
      "device_id", clish_context, SHELL_EXPAND_VIEW);
  if (!device_str) {
    SWITCH_PRINT(clish_context, "device id null");
    return 0;
  }

  device = atoi(device_str);

  status = switch_acl_drop_stats_dump(device, clish_context);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("acl drop stats dump failed on device %d:(%s)\n",
                     device,
                     switch_error_to_string(status));
    SWITCH_PRINT(clish_context,
                 "acl drop stats dump failed: (%s)\n",
                 switch_error_to_string(status));
    return 0;
  }

  bfshell_string_free(device_str);
  return 0;
}

CLISH_PLUGIN_SYM(switchapi_show_hashtable_info) {
  char *device_str = NULL;
  clish_pargv_t *pargv = NULL;
  const char *hashtable_type_str = NULL;
  const clish_parg_t *parg = NULL;
  switch_hashtable_type_t hashtable_type = 0;
  switch_device_t device = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  device_str = (char *)clish_shell_expand_var_ex(
      "device_id", clish_context, SHELL_EXPAND_VIEW);
  if (!device_str) {
    SWITCH_PRINT(clish_context, "device id null");
    return 0;
  }

  device = atoi(device_str);

  pargv = clish_context__get_pargv(clish_context);

  parg = clish_pargv_find_arg(pargv, "hashtable_type");
  if (parg) {
    hashtable_type_str = clish_parg__get_value(parg);
    if (hashtable_type_str) {
      hashtable_type = atoi(hashtable_type_str);
    }

    status = switch_api_hashtable_dump(device, hashtable_type, clish_context);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("hashtable dump failed on device %d:(%s)\n",
                       device,
                       switch_error_to_string(status));
      SWITCH_PRINT(clish_context,
                   "hashtable dump failed: (%s)\n",
                   switch_error_to_string(status));
      return 0;
    }
  }

  bfshell_string_free(device_str);
  return 0;
}

CLISH_PLUGIN_INIT(switchapi) {
  clish_plugin_add_sym(plugin, switchapi_show_handle, "switchapi_show_handle");
  clish_plugin_add_sym(plugin, switchapi_show_device, "switchapi_show_device");
  clish_plugin_add_sym(
      plugin, switchapi_show_table_info, "switchapi_show_table_info");
  clish_plugin_add_sym(
      plugin, switchapi_show_config_info, "switchapi_show_config_info");
  clish_plugin_add_sym(
      plugin, switchapi_show_port_dump, "switchapi_show_port_dump");
  clish_plugin_add_sym(
      plugin, switchapi_show_vlan_dump, "switchapi_show_vlan_dump");
  clish_plugin_add_sym(
      plugin, switchapi_port_add_delete, "switchapi_port_add_delete");
  clish_plugin_add_sym(plugin,
                       switchapi_show_packet_driver_dump,
                       "switchapi_show_packet_driver_dump");
  clish_plugin_add_sym(plugin,
                       switchapi_show_hostif_interface,
                       "switchapi_show_hostif_interface");
  clish_plugin_add_sym(plugin,
                       switchapi_debug_pktdriver_trace_enable_disable,
                       "switchapi_debug_pktdriver_trace_enable_disable");
  clish_plugin_add_sym(
      plugin, switchapi_debug_log_level_set, "switchapi_debug_log_level_set");

  clish_plugin_add_sym(
      plugin, switchapi_port_stats_clear, "switchapi_port_stats_clear");

  clish_plugin_add_sym(
      plugin, switchapi_show_drop_stats, "switchapi_show_drop_stats");
  clish_plugin_add_sym(
      plugin, switchapi_show_hashtable_info, "switchapi_show_hashtable_info");

  return 0;
}
