/*
Copyright 2013-present Barefoot Networks, Inc.
*/

#include "switchapi/switch_meter.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd_common.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_MAX_TXN_SZ 10

static switch_pd_feature_t pd_feature;

switch_status_t switch_pd_feature_set(void) {
  SWITCH_MEMSET(&pd_feature, 0x0, sizeof(pd_feature));

#ifndef P4_ACL_DISABLE
  pd_feature.acl = TRUE;
#endif /* P4_ACL_DISABLE */

#ifndef P4_INGRESS_ACL_RANGE_DISABLE
  pd_feature.ingress_acl_range = TRUE;
#endif /* P4_INGRESS_ACL_RANGE_DISABLE */

#ifndef P4_EGRESS_ACL_RANGE_DISABLE
  pd_feature.egress_acl_range = TRUE;
#endif /* P4_EGRESS_ACL_RANGE_DISABLE */

#ifdef P4_BFD_OFFLOAD_ENABLE
  pd_feature.bfd_offload = TRUE;
#endif /* P4_BFD_OFFLOAD_ENABLE */

#ifdef P4_EGRESS_FILER
  pd_feature.egress_filter = TRUE;
#endif /* P4_EGRESS_FILTER */

#ifdef P4_FAST_FAILOVER_ENABLE
  pd_feature.fast_failover = TRUE;
#endif /* P4_FAST_FAILOVER_ENABLE */

#ifdef P4_FLOWLET_ENABLE
  pd_feature.flowlet = TRUE;
#endif /* P4_FLOWLET_ENABLE */

#ifdef P4_INT_EP_ENABLE
  pd_feature.int_ep = TRUE;
#endif /* P4_INT_EP_ENABLE */

#ifdef P4_INT_OVER_L4_ENABLE
  pd_feature.int_ep = TRUE;
#endif /* P4_INT_OVER_L4_ENABLE */

#ifdef P4_INT_DIGEST_ENABLE
  pd_feature.int_digest = TRUE;
#endif /* P4_INT_DIGEST_ENABLE */

#ifdef P4_INT_TRANSIT_ENABLE
  pd_feature.int_transit = TRUE;
#endif /* P4_INT_TRANSIT_ENABLE */

#ifndef P4_IPV4_DISABLE
  pd_feature.ipv4 = TRUE;
#endif /* P4_IPV4_DISABLE */

#ifndef P4_IPV6_DISABLE
  pd_feature.ipv6 = TRUE;
#endif /* P4_IPV6_DISABLE */

#ifndef P4_L2_DISABLE
  pd_feature.l2 = TRUE;
#endif /* P4_L2_DISABLE */

#ifndef P4_L2_MULTICAST_DISABLE
  pd_feature.l2_multicast = TRUE;
#endif /* P4_L2_MULTICAST_DISABLE */

#ifndef P4_TUNNEL_MULTICAST_DISABLE
  pd_feature.tunnel_multicast = TRUE;
#endif /* P4_TUNNEL_MULTICAST_DISABLE */

#ifndef P4_L3_DISABLE
  pd_feature.l3 = TRUE;
#endif /* P4_L3_DISABLE */

#ifndef P4_L3_MULTICAST_DISABLE
  pd_feature.l3_multicast = TRUE;
#endif /* P4_L3_MULTICAST_DISABLE */

#ifndef P4_MIRROR_DISABLE
  pd_feature.mirror = TRUE;
#endif /* P4_MIRROR_DISABLE */

#ifndef P4_METER_DISABLE
  pd_feature.meter = TRUE;
#endif /* P4_METER_DISABLE */

#ifdef P4_DTEL_DROP_REPORT_ENABLE
  pd_feature.mirror_on_drop = TRUE;
#endif /* P4_DTEL_DROP_REPORT_ENABLE */

#ifndef P4_MPLS_DISABLE
  pd_feature.mpls = TRUE;
#endif /* P4_MPLS_DISABLE */

#ifdef P4_MPLS_UDP_ENABLE
  pd_feature.mpls_udp = TRUE;
#endif /* P4_MPLS_UDP_ENABLE */

#ifndef P4_MULTICAST_DISABLE
  pd_feature.multicast = TRUE;
#endif /* P4_MULTICAST_DISABLE */

#ifndef P4_NAT_DISABLE
  pd_feature.nat = TRUE;
#endif /* P4_NAT_DISABLE */

#ifndef P4_NVGRE_DISABLE
  pd_feature.nvgre = TRUE;
#endif /* P4_NVGRE_DISABLE */

#ifndef P4_GENEVE_DISABLE
  pd_feature.geneve = TRUE;
#endif /* P4_GENEVE_DISABLE */

#ifndef P4_QOS_DISABLE
  pd_feature.qos = TRUE;
#endif /* P4_QOS_DISABLE */

#ifdef P4_ACL_QOS_ENABLE
  pd_feature.acl_qos = TRUE;
#endif /* P4_ACL_QOS_ENABLE */

#ifdef P4_QOS_METERING_ENABLE
  pd_feature.qos_metering = TRUE;
#endif /* P4_QOS_METERING_ENABLE */

#ifndef P4_RACL_DISABLE
  pd_feature.racl = TRUE;
#endif /* P4_RACL_DISABLE */

#ifdef P4_RACL_STATS_ENABLE
  pd_feature.racl_stats = TRUE;
#endif /* P4_RACL_STATS_ENABLE */

#ifdef P4_EGRESS_ACL_STATS_ENABLE
  pd_feature.egress_acl_stats = TRUE;
#endif /* P4_EGRESS_ACL_STATS_ENABLE */

#ifdef P4_EGRESS_OUTER_BD_STATUS_ENABLE
  pd_feature.egress_outer_bd_stats = TRUE;
#endif /* P4_EGRESS_OUTER_BD_STATUS_ENABLE */

#ifdef P4_MIRROR_ACL_STATS_ENABLE
  pd_feature.mirror_acl_stats = TRUE;
#endif /* P4_MIRROR_ACL_STATS_ENABLE */

#ifdef P4_RESILIENT_HASH_ENABLE
  pd_feature.resilient_hash = TRUE;
#endif /* P4_RESILIENT_HASH_ENABLE */

#ifdef P4_SFLOW_ENABLE
  pd_feature.sflow = TRUE;
#endif /* P4_SFLOW_ENABLE */

#ifdef P4_SR_ENABLE
  pd_feature.sr = TRUE;
#endif /* P4_SR_ENABLE */

#ifndef P4_STATS_DISABLE
  pd_feature.stats = TRUE;
#endif /* P4_STATS_DISABLE */

#ifndef P4_STORM_CONTROL_DISABLE
  pd_feature.storm_control = TRUE;
#endif /* P4_STORM_CONTROL_DISABLE */

#ifndef P4_STP_DISABLE
  pd_feature.stp = TRUE;
#endif /* P4_STP_DISABLE */

#ifndef P4_TUNNEL_DISABLE
  pd_feature.tunnel = TRUE;
#endif /* P4_TUNNEL_DISABLE */

#ifndef P4_IPV6_TUNNEL_DISABLE
  pd_feature.ipv6_tunnel = TRUE;
#endif /* P4_IPV6_TUNNEL_DISABLE */

#ifndef P4_URPF_DISABLE
  pd_feature.urpf = TRUE;
#endif /* P4_URPF_DISABLE */

#ifdef P4_WCMP_ENABLE
  pd_feature.wcmp = TRUE;
#endif /* P4_WCMP_ENABLE */

#ifdef P4_MIRROR_ACL_ENABLE
  pd_feature.mirror_acl = TRUE;
#endif /* P4_MIRROR_ACL_ENABLE */

#ifdef P4_DTEL_FLOW_STATE_TRACK_ENABLE
  pd_feature.dtel_apx_stateful = TRUE;
#endif /* P4_DTEL_FLOW_STATE_TRACK_ENABLE */

#ifdef P4_DTEL_QUEUE_REPORT_ENABLE
  pd_feature.dtel_stateless_sup = TRUE;
#endif /* P4_DTEL_QUEUE_REPORT_ENABLE */

#ifdef P4_DTEL_REPORT_LB_ENABLE
  pd_feature.dtel_mirror_lb = TRUE;
#endif /* P4_DTEL_REPORT_LB_ENABLE */

#ifdef P4_DTEL_REPORT_ENABLE
  pd_feature.dtel_report = TRUE;
#endif /* P4_DTEL_REPORT_ENABLE */

#ifdef P4_DTEL_WATCH_INNER_ENABLE
  pd_feature.dtel_watch = TRUE;
#endif /* P4_DTEL_WATCH_INNER_ENABLE */

#ifndef P4_INGRESS_MAC_ACL_DISABLE
  pd_feature.ingress_mac_acl = TRUE;
#endif /* P4_INGRESS_MAC_ACL_DISABLE */

#ifndef P4_EGRESS_MAC_ACL_DISABLE
  pd_feature.egress_mac_acl = TRUE;
#endif /* P4_EGRESS_MAC_ACL_DISABLE */

#ifdef P4_TUNNEL_NEXTHOP_ENABLE
  pd_feature.tunnel_nexthop = TRUE;
#endif /* P4_TUNNEL_NEXTHOP_ENABLE */

#ifdef P4_INGRESS_UC_SELF_FWD_CHECK_DISABLE
  pd_feature.ingress_uc_self_fwd_check_disable = TRUE;
#endif /* P4_INGRESS_UC_SELF_FWD_CHECK_DISABLE */

#ifdef P4_TUNNEL_V4_VXLAN_ONLY
  pd_feature.tunnel_v4_vxlan = TRUE;
#endif /* P4_TUNNEL_V4_VXLAN_ONLY */

#ifdef P4_COPP_COLOR_DROP_ENABLE
  pd_feature.copp_color_drop = TRUE;
#endif /* P4_COPP_COLOR_DROP_ENABLE */

#ifndef P4_SAME_BD_CHECK_DISABLE
  pd_feature.same_bd_check_disable = TRUE;
#endif /* P4_SAME_BD_CHECK_DISABLE */

  return SWITCH_STATUS_SUCCESS;
}

switch_pd_feature_t *switch_pd_feature_get(void) { return &pd_feature; }

switch_status_t switch_pd_batch_begin() {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(pd_status);

#if defined(SWITCH_PD) && defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)

  pd_status = pipe_mgr_begin_batch(switch_cfg_sess_hdl);
  status = switch_pd_status_to_status(pd_status);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR("batch begin failed : %s (pd: 0x%x)\n",
                        switch_error_to_string(status),
                        pd_status);
  }

  pd_status = bf_mc_begin_batch(switch_cfg_mc_sess_hdl);
  status = switch_pd_status_to_status(pd_status);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR("batch begin failed : %s (pd: 0x%x)\n",
                        switch_error_to_string(status),
                        pd_status);
  }

#endif /* SWITCH_PD && __TARGET_TOFINO__ */

  return status;
}

switch_status_t switch_pd_batch_end(bool hw_synchronous) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(pd_status);
  UNUSED(hw_synchronous);

#if defined(SWITCH_PD) && defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)

  pd_status = pipe_mgr_end_batch(switch_cfg_sess_hdl, hw_synchronous);
  status = switch_pd_status_to_status(pd_status);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR("batch end failed : %s (pd: 0x%x)\n",
                        switch_error_to_string(status),
                        pd_status);
  }
  pd_status = bf_mc_end_batch(switch_cfg_mc_sess_hdl, hw_synchronous);
  status = switch_pd_status_to_status(pd_status);

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR("batch end failed : %s (pd: 0x%x)\n",
                        switch_error_to_string(status),
                        pd_status);
  }

#endif /* SWITCH_PD && __TARGET_TOFINO__ */

  return status;
}

switch_status_t switch_pd_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_dev_port_t cpu_eth_dev_port = 0;
  switch_dev_port_t cpu_pcie_dev_port = 0;
  switch_dev_port_t dev_port = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);
  UNUSED(cpu_eth_dev_port);
  UNUSED(cpu_pcie_dev_port);
  UNUSED(dev_port);

#ifdef SWITCH_PD

#if !defined(P4_MULTICAST_DISABLE) || defined(TUNNEL_NEXTHOP_ENABLE)

  pd_status = p4_pd_mc_create_session(&switch_cfg_mc_sess_hdl);

#endif /* P4_MULTICAST_DISABLE */

  pd_status = p4_pd_client_init(&switch_cfg_sess_hdl);

#ifdef __TARGET_TOFINO__

  cpu_eth_dev_port = p4_devport_mgr_eth_cpu_port_get(device);
  cpu_pcie_dev_port = p4_devport_mgr_pcie_cpu_port_get(device);
  dev_port = SWITCH_CONFIG_PCIE() ? cpu_pcie_dev_port : cpu_eth_dev_port;
  pd_status = p4_devport_mgr_set_copy_to_cpu(device, true, dev_port);

#else

  cpu_eth_dev_port = SWITCH_CPU_DEV_PORT_ETH_DEFAULT;
  cpu_pcie_dev_port = SWITCH_CPU_DEV_PORT_PCIE_DEFAULT;

#endif /* __TARGET_TOFINO__ */

  switch_device_cpu_eth_dev_port_set(device, cpu_eth_dev_port);
  switch_device_cpu_pcie_dev_port_set(device, cpu_pcie_dev_port);
  status = switch_pd_status_to_status(pd_status);

  switch_pd_feature_set();

  switch_pd_table_init();

#endif /* SWITCH_PD */

  return status;
}
switch_status_t switch_pd_free(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#if !defined(P4_MULTICAST_DISABLE) || defined(TUNNEL_NEXTHOP_ENABLE)
#ifdef __TARGET_TOFINO__
  p4_pd_mc_destroy_session(switch_cfg_mc_sess_hdl);
#else
  p4_pd_mc_delete_session(switch_cfg_mc_sess_hdl);
#endif /* __TARGET_TOFINO__ */
#endif /* P4_MULTICAST_DISABLE */
  switch_pd_mac_learn_callback_deregister(device);
  pd_status = p4_pd_client_cleanup(switch_cfg_sess_hdl);
  return status;
}

switch_status_t switch_pd_max_ports_get(switch_device_t device,
                                        switch_uint32_t *max_ports) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(max_ports);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
  pd_status = bf_pal_max_ports_get(device, max_ports);
  status = switch_pd_status_to_status(pd_status);
#else
  //*max_ports = SWITCH_MAX_PORTS;
  *max_ports = 256;
#endif /* __TARGET_TOFINO__ && BMV2TOFINO */

#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_port_list_get(switch_device_t device,
                                        switch_uint32_t max_ports,
                                        switch_port_t *fp_list,
                                        switch_dev_port_t *dev_port_list) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_uint32_t index = 0;

  UNUSED(device);
  UNUSED(max_ports);
  UNUSED(fp_list);
  UNUSED(dev_port_list);

  UNUSED(status);
  UNUSED(pd_status);
  UNUSED(index);

#ifdef SWITCH_PD
  for (index = 0; index < max_ports; index++) {
    fp_list[index] = index;
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
    pd_status = bf_pal_fp_idx_to_dev_port_map(
        device, fp_list[index], &dev_port_list[index]);
    status = switch_pd_status_to_status(pd_status);
#else
    dev_port_list[index] = index;
#endif /* __TARGET_TOFINO__ && BMV2TOFINO */
  }

#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_max_pipes_get(switch_device_t device,
                                        switch_uint32_t *max_pipes) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(max_pipes);

#ifdef SWITCH_PD
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
  pd_status = bf_pal_num_pipes_get(device, max_pipes);
  status = switch_pd_status_to_status(pd_status);
#endif /* __TARGET_TOFINO__ && BMV2TOFINO */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_recirc_port_list_get(
    switch_device_t device,
    switch_uint32_t *max_recirc_ports,
    switch_port_t *recirc_port_list,
    switch_dev_port_t *recirc_dev_port_list) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_uint32_t index = 0;
  switch_uint32_t num_ports = 0;
  switch_uint32_t start_recirc_index = 0, end_recirc_index = 0;
  switch_dev_port_t dev_port = 0;

  UNUSED(device)
  UNUSED(recirc_port_list);
  UNUSED(recirc_dev_port_list);
  UNUSED(status);
  UNUSED(pd_status);
  UNUSED(index);
  UNUSED(max_recirc_ports);
  UNUSED(dev_port);
  UNUSED(start_recirc_index);
  UNUSED(end_recirc_index);
  UNUSED(num_ports);

#ifdef SWITCH_PD
  for (index = 0; index < SWITCH_MAX_RECIRC_PORTS; index++) {
    recirc_port_list[index] = SWITCH_CPU_PORT_PCIE_DEFAULT + index;
    recirc_dev_port_list[index] = SWITCH_CPU_PORT_PCIE_DEFAULT + index;
  }
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
  pd_status = bf_pal_recirc_port_range_get(
      device, &start_recirc_index, &end_recirc_index);
  status = switch_pd_status_to_status(pd_status);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR("Failed to get recirc port ranges on device %d",
                        device);
    return status;
  }
  /*
   * BF PAL layer sends all 4 recirc port indices per pipe. Switchapi populates
   * only one per pipe.
   */
  for (index = start_recirc_index; index <= end_recirc_index; index += 4) {
    recirc_port_list[num_ports] = index;
    pd_status = bf_pal_recirc_port_to_dev_port_map(device, index, &dev_port);
    status = switch_pd_status_to_status(pd_status);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_PD_LOG_ERROR(
          "Failed to get recirc port mapping on device %d for port %d",
          device,
          index);
      return status;
    }
    recirc_dev_port_list[num_ports] = dev_port;
    num_ports++;
  }
#endif /* __TARGET_TOFINO__ && BMV2TOFINO */

  // One Recirc port per pipe.
  *max_recirc_ports = num_ports;

#endif /* SWITCH_PD */
  return status;
}

char *switch_pd_table_id_to_string(switch_pd_table_id_t table_id) {
  UNUSED(table_id);

  return "table";
}

char *switch_pd_action_id_to_string(switch_pd_action_id_t action_id) {
  UNUSED(action_id);

  return "action";
}

switch_status_t switch_pd_status_to_status(switch_pd_status_t pd_status) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(status);
  UNUSED(pd_status);

#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
  switch (pd_status) {
    case BF_SUCCESS:
      status = SWITCH_STATUS_SUCCESS;
      break;

    case BF_NO_SYS_RESOURCES:
      status = SWITCH_STATUS_INSUFFICIENT_RESOURCES;
      break;

    case BF_ALREADY_EXISTS:
      status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
      break;

    case BF_IN_USE:
      status = SWITCH_STATUS_RESOURCE_IN_USE;
      break;

    case BF_HW_COMM_FAIL:
      status = SWITCH_STATUS_HW_FAILURE;
      break;

    case BF_OBJECT_NOT_FOUND:
      status = SWITCH_STATUS_ITEM_NOT_FOUND;
      break;

    case BF_NOT_IMPLEMENTED:
      status = SWITCH_STATUS_NOT_IMPLEMENTED;
      break;

    case BF_INVALID_ARG:
      status = SWITCH_STATUS_INVALID_PARAMETER;
      break;

    default:
      status = SWITCH_STATUS_PD_FAILURE;
      break;
  }
#else
  switch (pd_status) {
#if 0
    case TABLE_FULL:
      status = SWITCH_STATUS_TABLE_FULL;
      break;

    case EXPIRED_HANDLE:
    case INVALID_GRP_HANDLE:
    case INVALID_HANDLE:
    case EXPIRED_HANDLE:
    case INVALID_MBR_HANDLE:
      status = SWITCH_STATUS_INVALID_PD_HANDLE;
      break;

    case DUPLICATE_ENTRY:
    case MBR_ALREADY_IN_GRP:
      status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
      break;

    case MBR_NOT_IN_GRP:
      status = SWITCH_STATUS_ITEM_NOT_FOUND;
      break;

    case GRP_STILL_USED:
    case MBR_STILL_USED:
      status = SWITCH_STATUS_RESOURCE_IN_USE;
      break;
#endif
    default:
      status = SWITCH_STATUS_SUCCESS;
      break;
  }
#endif /* __TARGET_TOFINO && !BMV2TOFINO */

  return status;
}

switch_pd_status_t switch_status_to_pd_status(switch_status_t status) {
  UNUSED(status);

  return SWITCH_PD_STATUS_SUCCESS;
}

switch_status_t switch_pd_entry_dump(switch_device_t device,
                                     switch_pd_dump_entry_t *pd_entry) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  char log_buffer[SWITCH_LOG_BUFFER_SIZE];
  switch_uint16_t index = 0;
  switch_uint16_t length = 0;
  bool print_newline = FALSE;

  UNUSED(device);
  UNUSED(pd_entry);

  SWITCH_ASSERT(pd_entry);

  length += switch_snprintf(
      log_buffer + length, SWITCH_LOG_BUFFER_SIZE, "\n\n\tENTRY_DUMP:\n");

  length +=
      switch_snprintf(log_buffer + length,
                      SWITCH_LOG_BUFFER_SIZE - length,
                      "\tentry type: %s\n",
                      switch_pd_entry_type_to_string(pd_entry->entry_type));

  if (pd_entry->match_spec_size) {
    length += switch_snprintf(log_buffer + length,
                              SWITCH_LOG_BUFFER_SIZE - length,
                              "\tmatch spec:\n\t\t");
  }

  for (index = 0; index < pd_entry->match_spec_size; index++) {
    length += switch_snprintf(log_buffer + length,
                              SWITCH_LOG_BUFFER_SIZE - length,
                              "%02x ",
                              pd_entry->match_spec[index]);
    if (((index + 1) % 16) == 0) {
      length += switch_snprintf(
          log_buffer + length, SWITCH_LOG_BUFFER_SIZE - length, "\n\t\t");
    }
    print_newline = TRUE;
  }

  if (print_newline) {
    print_newline = FALSE;
    length += switch_snprintf(
        log_buffer + length, SWITCH_LOG_BUFFER_SIZE - length, "\n");
  }

  if (pd_entry->action_spec_size) {
    length += switch_snprintf(log_buffer + length,
                              SWITCH_LOG_BUFFER_SIZE - length,
                              "\taction spec:\n\t\t");
  }

  for (index = 0; index < pd_entry->action_spec_size; index++) {
    length += switch_snprintf(log_buffer + length,
                              SWITCH_LOG_BUFFER_SIZE - length,
                              "%02x ",
                              pd_entry->action_spec[index]);
    if (((index + 1) % 16) == 0) {
      length += switch_snprintf(
          log_buffer + length, SWITCH_LOG_BUFFER_SIZE - length, "\n\t\t");
    }
    print_newline = TRUE;
  }

  if (print_newline) {
    length += switch_snprintf(
        log_buffer + length, SWITCH_LOG_BUFFER_SIZE - length, "\n");
  }

  length += switch_snprintf(log_buffer + length,
                            SWITCH_LOG_BUFFER_SIZE - length,
                            "\tpd handle: %lx\n",
                            (switch_uint64_t)pd_entry->pd_hdl);

  length += switch_snprintf(log_buffer + length,
                            SWITCH_LOG_BUFFER_SIZE - length,
                            "\tpd group handle: %lx\n",
                            (switch_uint64_t)pd_entry->pd_grp_hdl);

  length += switch_snprintf(log_buffer + length,
                            SWITCH_LOG_BUFFER_SIZE - length,
                            "\tpd member handle: %lx\n\n",
                            (switch_uint64_t)pd_entry->pd_mbr_hdl);

  SWITCH_PD_LOG_DEBUG("%s", log_buffer);

  return status;
}

bool switch_pd_platform_type_model(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  bool sw_model = TRUE;

#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
  status = bf_pal_pltfm_type_get(device, &sw_model);
  if (status != SWITCH_STATUS_SUCCESS) {
    sw_model = TRUE;
  }
#endif /* __TARGET_TOFINO__ && BMV2TOFINO */

  return sw_model;
}

switch_int32_t switch_pd_counter_read_flags(switch_device_t device) {
  switch_int32_t read_flags = 0;
  switch_uint32_t refresh_interval = 0;

  switch_api_device_counter_refresh_interval_get(device, &refresh_interval);
  if (refresh_interval == 0) {
    read_flags |= COUNTER_READ_HW_SYNC;
  }

  return read_flags;
}

#ifdef __cplusplus
}
#endif
