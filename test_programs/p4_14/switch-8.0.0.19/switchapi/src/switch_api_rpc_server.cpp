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
#include <iostream>

#include "switch_api_rpc.h"
#include "thrift_cache.h"

#include "switchapi/switch_vlan.h"
#include "switchapi/switch_base_types.h"
#include "switchapi/switch_rif.h"
#include "switchapi/switch_interface.h"
#include "switchapi/switch_l2.h"
#include "switchapi/switch_l3.h"
#include "switchapi/switch_neighbor.h"
#include "switchapi/switch_rmac.h"
#include "switchapi/switch_lag.h"
#include "switchapi/switch_tunnel.h"
#include "switchapi/switch_mpls.h"
#include "switchapi/switch_vrf.h"
#include "switchapi/switch_nhop.h"
#include "switchapi/switch_nat.h"
#include "switchapi/switch_hostif.h"
#include "switchapi/switch_acl.h"
#include "switchapi/switch_mcast.h"
#include "switchapi/switch_rpf.h"
#include "switchapi/switch_stp.h"
#include "switchapi/switch_mirror.h"
#include "switchapi/switch_table.h"
#include "switchapi/switch_hash.h"
#include "switchapi/switch_log.h"
#include "switchapi/switch_meter.h"
#include "switchapi/switch_port.h"
#include "switchapi/switch_config.h"
#include "switchapi/switch_sflow.h"
#include "switchapi/switch_ln.h"
#include "switchapi/switch_failover.h"
#include "switchapi/switch_bfd.h"
#include "switchapi/switch_qos.h"
#include "switchapi/switch_buffer.h"
#include "switchapi/switch_queue.h"
#include "switchapi/switch_ila.h"
#include "switchapi/switch_wred.h"
#include "switchapi/switch_device.h"
#include "switchapi/switch_dtel.h"
#include "arpa/inet.h"

#include <bfsys/bf_sal/bf_sys_intf.h>

#define SWITCH_API_RPC_SERVER_PORT (9091)

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace ::switch_api;

static pthread_mutex_t cookie_mutex;
static pthread_cond_t cookie_cv;
static void *cookie;

void switch_mac_to_string(unsigned char *m, char *d) {
  snprintf(d, 18, "%02x:%02x:%02x:%02x:%02x:%02x",
          m[0], m[1], m[2], m[3], m[4], m[5]);
  return;
}

unsigned int switch_string_to_mac(const std::string s, unsigned char *m) {
  unsigned int i, j = 0;
  memset(m, 0, 6);
  for (i = 0; i < s.size(); i++) {
    char let = s.c_str()[i];
    if (let >= '0' && let <= '9') {
      m[j / 2] = (m[j / 2] << 4) + (let - '0');
      j++;
    } else if (let >= 'a' && let <= 'f') {
      m[j / 2] = (m[j / 2] << 4) + (let - 'a' + 10);
      j++;
    } else if (let >= 'A' && let <= 'F') {
      m[j / 2] = (m[j / 2] << 4) + (let - 'A' + 10);
      j++;
    }
  }
  return (j == 12);
}

unsigned int switch_string_to_v4_ip(const std::string s, unsigned int *m) {
  unsigned char r = 0;
  unsigned int i;
  *m = 0;
  for (i = 0; i < s.size(); i++) {
    char let = s.c_str()[i];
    if (let >= '0' && let <= '9') {
      r = (r * 10) + (let - '0');
    } else {
      *m = (*m << 8) | r;
      r = 0;
    }
  }
  *m = (*m << 8) | (r & 0xFF);
  return (*m);
}

void switch_string_to_v6_ip(const std::string s, unsigned char *v6_ip) {
  const char *v6_str = s.c_str();
  inet_pton(AF_INET6, v6_str, v6_ip);
  return;
}

void switch_parse_ip_address(const switcht_ip_addr_t ip_addr,
                             switch_ip_addr_t *lip_addr) {
  memset(lip_addr, 0, sizeof(switch_ip_addr_t));
  lip_addr->type = (switch_ip_addr_type_t)ip_addr.addr_type;
  lip_addr->prefix_len = ip_addr.prefix_length;
  if (lip_addr->type == SWITCH_API_IP_ADDR_V4) {
    switch_string_to_v4_ip(ip_addr.ipaddr, &(lip_addr->ip.v4addr));
  } else {
    switch_string_to_v6_ip(ip_addr.ipaddr, lip_addr->ip.v6addr.u.addr8);
  }
}

void switch_parse_dtel_watchlist_match_info(
    const switcht_device_t device,
    const std::vector<switcht_twl_key_value_pair_t> &twl_kvp,
    switch_twl_match_info_t *match_info) {
  memset(match_info, 0, sizeof(switch_twl_match_info_t));
  match_info->field_count = twl_kvp.size();
  match_info->fields = (switch_twl_key_value_pair_t *)SWITCH_CALLOC(
      device, sizeof(switch_twl_key_value_pair_t) * twl_kvp.size(), 1);
  std::vector<switcht_twl_key_value_pair_t>::const_iterator iter =
      twl_kvp.begin();
  for (uint32_t i = 0; i < twl_kvp.size(); i++, iter++) {
    ((switch_twl_key_value_pair_t *)match_info->fields + i)->field =
        (switch_twl_field_t)iter->field;
    unsigned long long v =
        (unsigned long long)((switch_twl_field_t)iter->value.value_num);
    memcpy(
        &(((switch_twl_key_value_pair_t *)match_info->fields + i)->value.ipv4),
        &v,
        sizeof(switch_twl_value_t));
    ((switch_twl_key_value_pair_t *)match_info->fields + i)->mask =
        (switch_twl_field_t)iter->mask.value_num;
  }
}

class switch_api_rpcHandler : virtual public ::switch_api_rpcIf {
 public:
  switch_api_rpcHandler() {}

  switcht_status_t switch_api_init(const switcht_device_t device) { return 0; }

  void switch_api_table_get(switcht_table_t &_return,
                                       const switcht_device_t device,
                                       const int16_t table_id) {
    switch_table_t table;

    ::switch_api_table_get(device, (switch_table_id_t)table_id, &table);

    _return.valid = table.valid;
    _return.table_size = table.table_size;
    _return.num_entries = table.num_entries;
    _return.direction = table.direction;
    if (strlen((char*)table.table_name) == 0) {
      _return.table_name = std::string("NA");
    } else {
      _return.table_name = std::string((char*)table.table_name);
    }

    return;
  }

  int16_t switch_api_table_size_get(const switcht_device_t device,
                                    const int16_t table_id) {
    switch_size_t size;

    ::switch_api_table_size_get(device, (switch_table_id_t)table_id, &size);

    return (int16_t)size;
  }

  void switch_api_table_all_get(std::vector<switcht_table_t> &_return,
                                 const switcht_device_t device) {
    switch_table_t *tables = NULL;
    switch_size_t num_counters = 0;
    switcht_table_t _table;

    tables = (switch_table_t*)SWITCH_MALLOC(device,
            sizeof(switch_table_t), SWITCH_TABLE_MAX);
    ::switch_api_table_all_get(device, &num_counters, tables);
    if (num_counters <= 0) {
      return;
    }

    for (int i = 0; i < num_counters; i++) {
      _table.valid = tables[i].valid;
      _table.table_size = tables[i].table_size;
      _table.num_entries = tables[i].num_entries;
      _table.direction = tables[i].direction;
      if (strlen((char*)tables[i].table_name) == 0) {
        _table.table_name = std::string("NA");
      } else {
        _table.table_name = std::string((char*)tables[i].table_name);
      }
      _return.push_back(_table);
    }

    SWITCH_FREE(device, tables);
    return;
  }

  void switch_api_drop_stats_get(std::vector<int64_t> &_return,
                                 const switcht_device_t device) {
    switch_uint64_t *counters = NULL;
    int num_counters = 0;

    ::switch_api_drop_stats_get(device, &num_counters, &counters);
    if (num_counters <= 0) {
      return;
    }

    for (int i = 0; i < num_counters; i++) {
      _return.push_back(counters[i]);
    }

    SWITCH_FREE(device, counters);
    return;
  }

  /* Batch APIs */

  switcht_status_t switch_api_batch_begin() {
    return ::switch_api_batch_begin();
  }

  switcht_status_t switch_api_batch_end(bool hw_synchronous) {
    return ::switch_api_batch_end(hw_synchronous);
  }

  switcht_handle_t switch_api_port_add(const switcht_device_t device,
                                       const switcht_port_t port) {
    switch_handle_t port_handle = 0;
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    switch_api_port_info_t lport_info;
    lport_info.port = port;
    lport_info.port_speed = (switch_port_speed_t)SWITCH_PORT_SPEED_10G;
    lport_info.initial_admin_state = TRUE;
    lport_info.tx_mtu = 1600;
    lport_info.rx_mtu = 1600;
    status = ::switch_api_port_add(device, &lport_info, &port_handle);
    return port_handle;
  }

  switcht_handle_t switch_api_port_add_with_attribute(
      const switcht_device_t device,
      const switcht_api_port_info_t &api_port_info) {
    switch_handle_t port_handle = 0;
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    switch_api_port_info_t lport_info;
    lport_info.port = api_port_info.port;
    lport_info.tx_mtu = api_port_info.tx_mtu;
    lport_info.rx_mtu = api_port_info.rx_mtu;
    lport_info.port_speed = (switch_port_speed_t)api_port_info.port_speed;
    lport_info.initial_admin_state = api_port_info.initial_admin_state;
    lport_info.fec_mode = (switch_port_fec_mode_t)api_port_info.fec_mode;
    status = ::switch_api_port_add(device, &lport_info, &port_handle);
    return port_handle;
  }

  switcht_status_t switch_api_port_delete(const switcht_device_t device,
                                          const switcht_handle_t port_handle) {
    return ::switch_api_port_delete(device, port_handle);
  }

  switcht_status_t switch_api_port_ppg_create(const switcht_device_t device,
                                          const switcht_handle_t port_handle) {
    return ::switch_api_ppg_create(device, port_handle);
  }

  switcht_status_t switch_api_port_ppg_delete(const switcht_device_t device,
                                          const switcht_handle_t port_handle) {
    return ::switch_api_ppg_delete(device, port_handle);
  }


  switcht_status_t switch_api_port_storm_control_set(
      const switcht_device_t device,
      const switcht_port_t port_id,
      const switcht_packet_type_t pkt_type,
      const switcht_handle_t meter_handle) {
    return ::switch_api_port_storm_control_set((switch_device_t)device,
                                               (switch_port_t)port_id,
                                               (switch_packet_type_t)pkt_type,
                                               (switch_handle_t)meter_handle);
  }

  switcht_handle_t switch_api_port_id_to_handle_get(
      const switcht_device_t device, const switcht_port_t port) {
    switch_handle_t port_handle = 0;
    ::switch_api_port_id_to_handle_get(device, port, &port_handle);
    return port_handle;
  }

  int16_t switch_api_port_speed_get(const switcht_device_t device,
                                    const switcht_handle_t port_handle) {
    switch_port_speed_t port_speed;
    ::switch_api_port_speed_get(device, port_handle, &port_speed);
    return (int16_t)port_speed;
  }

  int16_t switch_api_port_auto_neg_get(const switcht_device_t device,
                                       const switcht_handle_t port_handle) {
    switch_port_auto_neg_mode_t an_mode;
    ::switch_api_port_auto_neg_get(device, port_handle, &an_mode);
    return (int16_t)an_mode;
  }

  bool switch_api_port_admin_state_get(const switcht_device_t device,
                                       const switcht_handle_t port_handle) {
    bool admin_state;
    ::switch_api_port_admin_state_get(device, port_handle, &admin_state);
    return admin_state;
  }

  int16_t switch_api_port_oper_status_get(const switcht_device_t device,
                                          const switcht_handle_t port_handle) {
    switch_port_oper_status_t oper_status;
    ::switch_api_port_oper_status_get(device, port_handle, &oper_status);
    return (int16_t)oper_status;
  }

  int16_t switch_api_port_loopback_mode_get(const switcht_device_t device,
                                           const switcht_handle_t port_handle) {
    switch_port_loopback_mode_t lb_mode;
    ::switch_api_port_loopback_mode_get(device, port_handle, &lb_mode);
    return (int16_t)lb_mode;
  }

  int32_t switch_api_port_rx_mtu_get(const switcht_device_t device,
                                     const switcht_handle_t port_handle) {
    switch_uint32_t rx_mtu;
    switch_uint32_t tx_mtu;
    ::switch_api_port_mtu_get(device, port_handle, &rx_mtu, &tx_mtu);
    return (int32_t)rx_mtu;
  }

  int32_t switch_api_port_tx_mtu_get(const switcht_device_t device,
                                     const switcht_handle_t port_handle) {
    switch_uint32_t rx_mtu;
    switch_uint32_t tx_mtu;
    ::switch_api_port_mtu_get(device, port_handle, &rx_mtu, &tx_mtu);
    return (int32_t)tx_mtu;
  }

  void switch_api_port_get(switcht_api_port_info_t &_api_port_info,
                           const switcht_device_t device,
                           int32_t port_number) {
    switch_api_port_info_t port_info;
    memset(&port_info, 0, sizeof(switch_api_port_info_t));

    port_info.port = port_number;

    //::switch_api_port_get(device, &port_info);
    _api_port_info.port = port_info.port;
    _api_port_info.port_speed = port_info.port_speed;
    _api_port_info.tx_mtu = port_info.tx_mtu;
    _api_port_info.rx_mtu = port_info.rx_mtu;
    _api_port_info.initial_admin_state = port_info.initial_admin_state;
    _api_port_info.fec_mode = port_info.fec_mode;
    return;
  }

  switcht_handle_t switch_api_port_storm_control_get(
                                     const switcht_device_t device,
                                     const switcht_handle_t port_handle,
                                     const switcht_packet_type_t pkt_type) {
    switch_handle_t meter_handle;
    ::switch_api_port_storm_control_get(
            device, port_handle, (switch_packet_type_t)pkt_type, &meter_handle);
    return meter_handle;
  }

  switcht_port_t switch_api_port_handle_to_id_get(
                                  const switcht_device_t device,
                                  const switcht_handle_t port_handle) {
    switch_port_t port;
    ::switch_api_port_handle_to_id_get(device, port_handle, &port);
    return port;
  }

  void switch_api_port_stats_get(
      std::vector<int64_t> &_counters,
      const switcht_device_t device,
      const switcht_handle_t port_handle,
      const std::vector<int16_t> &counter_ids) {
    std::vector<int16_t>::const_iterator it = counter_ids.begin();
    int64_t _counter;
    switch_port_counter_id_t *counter_id_list =
        (switch_port_counter_id_t *)SWITCH_MALLOC(
            device, sizeof(switch_port_counter_id_t), counter_ids.size());
    switch_uint64_t *counters = (switch_uint64_t *)SWITCH_MALLOC(
        device, sizeof(switch_uint64_t), counter_ids.size());
    for (uint32_t i = 0; i < counter_ids.size(); i++, it++) {
      counter_id_list[i] = (switch_port_counter_id_t)*it;
    }
    ::switch_api_port_stats_get(
        device, port_handle, counter_ids.size(), counter_id_list, counters);
    for (uint32_t i = 0; i < counter_ids.size(); i++) {
      _counter = counters[i];
      _counters.push_back(_counter);
    }
    SWITCH_FREE(device, counter_id_list);
    SWITCH_FREE(device, counters);
    return;
  }

  switcht_handle_t switch_api_port_ingress_acl_group_get(
                                    const switcht_device_t device,
                                    const switcht_handle_t port_handle) {
    switch_handle_t acl_group_handle;
    ::switch_api_port_ingress_acl_group_get(device, port_handle, &acl_group_handle);
    return acl_group_handle;
  }

  switcht_handle_t switch_api_port_egress_acl_group_get(
                                    const switcht_device_t device,
                                    const switcht_handle_t port_handle) {
    switch_handle_t acl_group_handle;
    ::switch_api_port_egress_acl_group_get(device, port_handle, &acl_group_handle);
    return acl_group_handle;
  }
  int16_t switch_api_port_ingress_acl_label_get(
                                    const switcht_device_t device,
                                    const switcht_handle_t port_handle) {
    switch_uint16_t label;
    ::switch_api_port_ingress_acl_label_get(device, port_handle, &label);
    return (int16_t)label;
  }

  int16_t switch_api_port_egress_acl_label_get(
                                    const switcht_device_t device,
                                    const switcht_handle_t port_handle) {
    switch_uint16_t label;
    ::switch_api_port_egress_acl_label_get(device, port_handle, &label);
    return (int16_t)label;
  }

  int32_t switch_api_port_bind_mode_get(const switcht_device_t device,
                                        const switcht_handle_t port_handle) {
    switch_port_bind_mode_t bind_mode;
    ::switch_api_port_bind_mode_get(device, port_handle, &bind_mode);
    return (int32_t)bind_mode;
  }

  int32_t switch_api_port_max_queues_get(const switcht_device_t device,
                                         const switcht_handle_t port_handle) {
    switch_uint32_t max_queues;
    ::switch_api_port_max_queues_get(device, port_handle, &max_queues);
    return (int32_t)max_queues;
  }

  int32_t switch_api_port_pfc_get(const switcht_device_t device,
                                  const switcht_handle_t port_handle) {
    switch_uint32_t pfc_map;
    ::switch_api_port_pfc_get(device, port_handle, &pfc_map);
    return (int32_t)pfc_map;
  }

  bool switch_api_port_link_tx_pause_get(
                          const switcht_device_t device,
                          const switcht_handle_t port_handle) {
    bool tx_pause, rx_pause;
    ::switch_api_port_link_pause_get(device, port_handle, &rx_pause, &tx_pause);
    return tx_pause;
  }

  bool switch_api_port_link_rx_pause_get(
                          const switcht_device_t device,
                          const switcht_handle_t port_handle) {
    bool tx_pause, rx_pause;
    ::switch_api_port_link_pause_get(device, port_handle, &rx_pause, &tx_pause);
    return rx_pause;
  }

  int16_t switch_api_port_fec_mode_get(
                          const switcht_device_t device,
                          const switcht_handle_t port_handle) {
    switch_port_fec_mode_t fec_mode;
    ::switch_api_port_fec_mode_get(device, port_handle, &fec_mode);
    return (int16_t)fec_mode;
  }

  switcht_handle_t switch_api_port_ingress_mirror_get(
                                    const switcht_device_t device,
                                    const switcht_handle_t port_handle) {
    switch_handle_t mirror_handle;
    ::switch_api_port_ingress_mirror_get(device, port_handle, &mirror_handle);
    return mirror_handle;
  }

  switcht_handle_t switch_api_port_egress_mirror_get(
                                    const switcht_device_t device,
                                    const switcht_handle_t port_handle) {
    switch_handle_t mirror_handle;
    ::switch_api_port_egress_mirror_get(device, port_handle, &mirror_handle);
    return mirror_handle;
  }

  switcht_handle_t switch_api_port_ingress_sflow_handle_get(
                                    const switcht_device_t device,
                                    const switcht_handle_t port_handle) {
    switch_handle_t ingress_sflow_handle;
    ::switch_api_port_ingress_sflow_handle_get(
            device, port_handle, &ingress_sflow_handle);
    return ingress_sflow_handle;
  }

  switcht_handle_t switch_api_port_egress_sflow_handle_get(
                                    const switcht_device_t device,
                                    const switcht_handle_t port_handle) {
    switch_handle_t egress_sflow_handle;
    ::switch_api_port_egress_sflow_handle_get(
            device, port_handle, &egress_sflow_handle);
    return egress_sflow_handle;
  }

  int64_t switch_api_ppg_drop_get(
          const switcht_device_t device,
          const switcht_handle_t ppg_handle) {
    uint64_t num_packets = 0;
    ::switch_api_ppg_drop_get(device, ppg_handle, &num_packets);
    return (int64_t) num_packets;
  }

  switcht_handle_t switch_api_port_ingress_qos_handle_get(
                                    const switcht_device_t device,
                                    const switcht_handle_t port_handle) {
    switch_handle_t ingress_qos_handle;
    switch_handle_t tc_queue_handle;
    switch_handle_t tc_ppg_handle;
    switch_handle_t egress_qos_handle;
    ::switch_api_port_qos_group_get(device, port_handle,
                                    &ingress_qos_handle,
                                    &tc_queue_handle,
                                    &tc_ppg_handle,
                                    &egress_qos_handle);
    return ingress_qos_handle;
  }

  switcht_handle_t switch_api_port_tc_queue_handle_get(
                                    const switcht_device_t device,
                                    const switcht_handle_t port_handle) {
    switch_handle_t ingress_qos_handle;
    switch_handle_t tc_queue_handle;
    switch_handle_t tc_ppg_handle;
    switch_handle_t egress_qos_handle;
    ::switch_api_port_qos_group_get(device, port_handle,
                                    &ingress_qos_handle,
                                    &tc_queue_handle,
                                    &tc_ppg_handle,
                                    &egress_qos_handle);
    return tc_queue_handle;
  }

  switcht_handle_t switch_api_port_tc_ppg_handle_get(
                                    const switcht_device_t device,
                                    const switcht_handle_t port_handle) {
    switch_handle_t ingress_qos_handle;
    switch_handle_t tc_queue_handle;
    switch_handle_t tc_ppg_handle;
    switch_handle_t egress_qos_handle;
    ::switch_api_port_qos_group_get(device, port_handle,
                                    &ingress_qos_handle,
                                    &tc_queue_handle,
                                    &tc_ppg_handle,
                                    &egress_qos_handle);
    return tc_ppg_handle;
  }

  switcht_handle_t switch_api_port_egress_qos_handle_get(
                                    const switcht_device_t device,
                                    const switcht_handle_t port_handle) {
    switch_handle_t ingress_qos_handle;
    switch_handle_t tc_queue_handle;
    switch_handle_t tc_ppg_handle;
    switch_handle_t egress_qos_handle;
    ::switch_api_port_qos_group_get(device, port_handle,
                                    &ingress_qos_handle,
                                    &tc_queue_handle,
                                    &tc_ppg_handle,
                                    &egress_qos_handle);
    return egress_qos_handle;
  }

  int16_t switch_api_port_max_ppg_get(const switcht_device_t device,
                                      const switcht_handle_t port_handle) {
    switch_uint8_t max_ppg;
    ::switch_api_port_max_ppg_get(device, port_handle, &max_ppg);
    return (int16_t)max_ppg;
  }

  void switch_api_port_ppg_get(std::vector<switcht_handle_t> &_ppg_handles,
                               const switcht_device_t device,
                               const switcht_handle_t port_handle) {
    switch_uint8_t max_ppg;
    ::switch_api_port_max_ppg_get(device, port_handle, &max_ppg);
    switch_handle_t *ppg_handles = (switch_handle_t*)SWITCH_MALLOC(
        device, sizeof(switch_handle_t), max_ppg);

    switcht_handle_t _ppg_handle;
    ::switch_api_port_ppg_get(device, port_handle, &max_ppg, ppg_handles);
    for (uint32_t i = 0; i < max_ppg; i++) {
      _ppg_handle = ppg_handles[i];
      _ppg_handles.push_back(_ppg_handle);
    }

    SWITCH_FREE(device, ppg_handles);
    return;
  }

  switcht_handle_t switch_api_port_pfc_priority_to_ppg_get(
                                    const switcht_device_t device,
                                    const switcht_handle_t port_handle) {
    switch_handle_t qos_map_handle;
    ::switch_api_port_pfc_priority_to_ppg_get(
            device, port_handle, &qos_map_handle);
    return qos_map_handle;
  }

  switcht_handle_t switch_api_port_pfc_priority_to_queue_get(
                                    const switcht_device_t device,
                                    const switcht_handle_t port_handle) {
    switch_handle_t qos_map_handle;
    ::switch_api_port_pfc_priority_to_queue_get(
            device, port_handle, &qos_map_handle);
    return qos_map_handle;
  }

  int32_t switch_api_port_queue_scheduler_group_handle_count_get(
                                const switcht_device_t device,
                                const switcht_handle_t port_handle) {
    switch_uint32_t count;
    ::switch_api_port_queue_scheduler_group_handle_count_get(
            device, port_handle, &count);
    return (int32_t)count;
  }

  void switch_api_port_qos_scheduler_group_handles_get(
                                std::vector<switcht_handle_t> &_group_handles,
                                const switcht_device_t device,
                                const switcht_handle_t port_handle) {
    switch_uint32_t count;
    ::switch_api_port_queue_scheduler_group_handle_count_get(
            device, port_handle, &count);
    switch_handle_t *group_handles = (switch_handle_t*)SWITCH_MALLOC(
        device, sizeof(switch_handle_t), count);

    switcht_handle_t _group_handle;
    ::switch_api_port_qos_scheduler_group_handles_get(
            device, port_handle, group_handles);
    for (uint32_t i = 0; i < count; i++) {
      _group_handle = group_handles[i];
      _group_handles.push_back(_group_handle);
    }

    SWITCH_FREE(device, group_handles);
    return;
  }

  switcht_handle_t switch_api_port_scheduler_profile_get(
                                    const switcht_device_t device,
                                    const switcht_handle_t port_handle) {
    switch_handle_t scheduler_handle;
    ::switch_api_port_scheduler_profile_get(
            device, port_handle, &scheduler_handle);
    return scheduler_handle;
  }

  int64_t switch_api_port_ppg_drop_get(const switcht_device_t device,
                                       const switcht_handle_t port_handle) {
    uint64_t drop_count = 0;
    ::switch_api_port_ppg_drop_get(device, port_handle, &drop_count);
    return (int64_t) drop_count;
  }

  int64_t switch_api_port_queue_drop_get(const switcht_device_t device,
                                         const switcht_handle_t port_handle) {
    uint64_t drop_count = 0;
    ::switch_api_port_queue_drop_get(device, port_handle, &drop_count);
    return (int64_t) drop_count;
  }

  void switch_api_storm_control_counters_get(
      std::vector<switcht_counter_t> &_counters,
      const switcht_device_t device,
      const switcht_handle_t meter_handle,
      const std::vector<int16_t> &counter_ids) {
    std::vector<int16_t>::const_iterator it = counter_ids.begin();
    switcht_counter_t _counter;
    switch_meter_counter_t *counter_id_list =
        (switch_meter_counter_t *)SWITCH_MALLOC(
            device, sizeof(switch_meter_counter_t), counter_ids.size());
    switch_counter_t *counters = (switch_counter_t *)SWITCH_MALLOC(
        device, sizeof(switch_counter_t), counter_ids.size());
    for (uint32_t i = 0; i < counter_ids.size(); i++, it++) {
      counter_id_list[i] = (switch_meter_counter_t)*it;
    }
    ::switch_api_storm_control_counters_get(
        device, meter_handle, counter_ids.size(), counter_id_list, counters);
    for (uint32_t i = 0; i < counter_ids.size(); i++) {
      _counter.num_packets = counters[i].num_packets;
      _counter.num_bytes = counters[i].num_bytes;
      _counters.push_back(_counter);
    }
    SWITCH_FREE(device, counter_id_list);
    SWITCH_FREE(device, counters);
    return;
  }

  switcht_handle_t switch_api_vrf_create(const switcht_device_t device,
                                         const switcht_vrf_id_t vrf) {
    switch_handle_t vrf_handle = 0;
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    status = ::switch_api_vrf_create(device, vrf, &vrf_handle);
    return vrf_handle;
  }

  switcht_status_t switch_api_vrf_delete(const switcht_device_t device,
                                         const switcht_handle_t vrf_handle) {
    return ::switch_api_vrf_delete(device, vrf_handle);
  }

  switcht_vrf_id_t switch_api_default_vrf_id_get(
      const switcht_device_t device) {
    switch_handle_t default_vrf_handle = 0;
    switch_vrf_t default_vrf_id = 0;
    ::switch_api_device_default_vrf_get(device,
                                        &default_vrf_id,
                                        &default_vrf_handle);
    return default_vrf_id;
  }

  switcht_handle_t switch_api_vrf_id_to_handle_get(
                                         const switcht_device_t device,
                                         const switcht_vrf_id_t vrf) {
    switch_handle_t vrf_handle = 0;
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    status = ::switch_api_vrf_id_to_handle_get(device, vrf, &vrf_handle);
    return vrf_handle;
  }

  switcht_status_t switch_api_vrf_rmac_handle_set(
      const switcht_device_t device,
      const switcht_handle_t vrf_handle,
      const switcht_handle_t rmac_handle) {
    return ::switch_api_vrf_rmac_handle_set(device, vrf_handle, rmac_handle);
  }

  switcht_handle_t switch_api_vrf_rmac_handle_get(
      const switcht_device_t device,
      const switcht_handle_t vrf_handle) {
    switch_handle_t rmac_handle = 0;
    ::switch_api_vrf_rmac_handle_get(device, vrf_handle, &rmac_handle);
    return rmac_handle;
  }

  switcht_vrf_id_t switch_api_vrf_handle_to_id_get(
                                         const switcht_device_t device,
                                         const switcht_handle_t vrf_handle) {
    switch_vrf_t vrf_id = 0;
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    status = ::switch_api_vrf_handle_to_id_get(device, vrf_handle, &vrf_id);
    return vrf_id;
  }

  switcht_status_t switch_api_port_trust_dscp_set(
      const switcht_device_t device,
      const switcht_handle_t port_handle,
      const bool trust_dscp) {
    return ::switch_api_port_trust_dscp_set(device, port_handle, trust_dscp);
  }

  switcht_status_t switch_api_port_trust_pcp_set(
      const switcht_device_t device,
      const switcht_handle_t port_handle,
      const bool trust_pcp) {
    return ::switch_api_port_trust_pcp_set(device, port_handle, trust_pcp);
  }

  switcht_status_t switch_api_port_ingress_mirror_set(
						      const switcht_device_t device,
						      const switcht_handle_t port_handle,
						      const switcht_handle_t mirror_handle) {
    return ::switch_api_port_ingress_mirror_set(device, port_handle, mirror_handle);
  }

  switcht_status_t switch_api_port_egress_mirror_set(
						      const switcht_device_t device,
						      const switcht_handle_t port_handle,
						      const switcht_handle_t mirror_handle) {
    return ::switch_api_port_egress_mirror_set(device, port_handle, mirror_handle);
  }

  switcht_status_t switch_api_port_learning_enabled_set(
      const switcht_device_t device,
      const switcht_handle_t port_handle,
      const bool learning_enabled) {
    return ::switch_api_port_learning_enabled_set(device, port_handle, learning_enabled);
  }

  switcht_status_t switch_api_port_drop_limit_set(
      const switcht_device_t device,
      const switcht_handle_t port_handle,
      const int32_t num_bytes) {
    return ::switch_api_port_drop_limit_set(device, port_handle, num_bytes);
  }

  switcht_status_t switch_api_port_drop_hysteresis_set(
      const switcht_device_t device,
      const switcht_handle_t port_handle,
      const int32_t num_bytes) {
    return ::switch_api_port_drop_hysteresis_set(
        device, port_handle, num_bytes);
  }

  switcht_status_t switch_api_port_pfc_cos_mapping(
      const switcht_device_t device,
      const switcht_handle_t port_handle,
      const std::vector<int8_t> &cos_to_icos) {
    return 0;
  }

  switcht_status_t switch_api_port_tc_default_set(
      const switcht_device_t device,
      const switcht_handle_t port_handle,
      const int16_t tc) {
    return ::switch_api_port_tc_default_set(device, port_handle, tc);
  }

  switcht_status_t switch_api_port_color_default_set(
      const switcht_device_t device,
      const switcht_handle_t port_handle,
      const switcht_color_t color) {
    return ::switch_api_port_color_default_set(
        device, port_handle, (switch_color_t)color);
  }

  switcht_status_t switch_api_port_qos_group_ingress_set(
      const switcht_device_t device,
      const switcht_handle_t port_handle,
      const switcht_handle_t qos_handle) {
    return ::switch_api_port_qos_group_ingress_set(
        device, port_handle, qos_handle);
  }

  switcht_status_t switch_api_port_qos_group_tc_set(
      const switcht_device_t device,
      const switcht_handle_t port_handle,
      const switcht_handle_t qos_handle) {
    return ::switch_api_port_qos_group_tc_set(device, port_handle, qos_handle);
  }

  switcht_status_t switch_api_port_qos_group_egress_set(
      const switcht_device_t device,
      const switcht_handle_t port_handle,
      const switcht_handle_t qos_handle) {
    return ::switch_api_port_qos_group_egress_set(
        device, port_handle, qos_handle);
  }

  switcht_status_t switch_api_port_bind_mode_set(
      const switcht_device_t device,
      const switcht_handle_t port_handle,
      const int32_t bind_mode) {
    return ::switch_api_port_bind_mode_set(
        device, port_handle, (switch_port_bind_mode_t)bind_mode);
  }

  switcht_status_t switch_api_port_mtu_set(const switcht_device_t device,
                                           const switcht_handle_t port_handle,
                                           const int32_t tx_mtu,
                                           const int32_t rx_mtu) {
    return ::switch_api_port_mtu_set(device, port_handle, tx_mtu, rx_mtu);
  }

  switcht_status_t switch_api_port_stats_clear(
      const switcht_device_t device, const switcht_handle_t port_handle) {
    return ::switch_api_port_stats_clear(device, port_handle);
  }

  switcht_status_t switch_api_port_ingress_acl_label_set(
                const switcht_device_t device,
                const switcht_handle_t port_handle,
                const int16_t label) {
    return ::switch_api_port_ingress_acl_label_set(device, port_handle, label);
  }

  switcht_status_t switch_api_port_egress_acl_label_set(
                const switcht_device_t device,
                const switcht_handle_t port_handle,
                const int16_t label) {
    return ::switch_api_port_egress_acl_label_set(device, port_handle, label);
  }

  switcht_status_t switch_api_lag_bind_mode_set(
      const switcht_device_t device,
      const switcht_handle_t port_handle,
      const int32_t bind_mode) {
    return ::switch_api_lag_bind_mode_set(
        device, port_handle, (switch_port_bind_mode_t)bind_mode);
  }

  switcht_handle_t switch_api_router_mac_group_create(
      const switcht_device_t device, const int32_t rmac_type) {
    switch_handle_t rmac_handle = 0;
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    status = ::switch_api_router_mac_group_create(
        device, (switch_rmac_type_t)rmac_type, &rmac_handle);
    return rmac_handle;
  }

  switcht_status_t switch_api_router_mac_group_delete(
      const switcht_device_t device, const switcht_handle_t rmac_handle) {
    return ::switch_api_router_mac_group_delete(device, rmac_handle);
  }

  switcht_status_t switch_api_router_mac_add(const switcht_device_t device,
                                             const switcht_handle_t rmac_handle,
                                             const switcht_mac_addr_t &mac) {
    switch_mac_addr_t lmac;
    switch_string_to_mac(mac, lmac.mac_addr);
    return ::switch_api_router_mac_add(device, rmac_handle, &lmac);
  }

  switcht_status_t switch_api_router_mac_delete(
      const switcht_device_t device,
      const switcht_handle_t rmac_handle,
      const switcht_mac_addr_t &mac) {
    switch_mac_addr_t lmac;
    switch_string_to_mac(mac, lmac.mac_addr);
    return ::switch_api_router_mac_delete(device, rmac_handle, &lmac);
  }

  switcht_handle_t switch_api_default_router_mac_handle_get(
      const switcht_device_t device) {
    switch_handle_t rmac_handle = 0;
    ::switch_api_device_default_rmac_handle_get(device, &rmac_handle);
    return rmac_handle;
  }

  void switch_api_rmac_macs_get(std::vector<switcht_mac_addr_t> &_macs,
                                const switcht_device_t device,
                                const switcht_handle_t rmac_handle) {
    switch_mac_addr_t *lmac = NULL;
    switch_uint16_t num_entries = 0;
    switcht_mac_addr_t _mac;
    ::switch_api_rmac_macs_get(device, rmac_handle, &num_entries, &lmac);
    for (uint16_t i = 0; i < num_entries; i++) {
      char dmac[18];
      switch_mac_to_string(lmac[i].mac_addr, dmac);
      if (strlen(dmac) == 0) {
        _mac = std::string("NA");
      } else {
        _mac = std::string(dmac);
      }
      _macs.push_back(_mac);
    }

    SWITCH_FREE(device, lmac);
    return;
  }

  switcht_handle_t switch_api_interface_create(
      const switcht_device_t device,
      const switcht_interface_info_t &interface_info) {
    switch_handle_t intf_handle = 0;
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    ::switch_api_interface_info_t i_info;
    memset(&i_info, 0, sizeof(::switch_api_interface_info_t));
    i_info.type = (switch_interface_type_t)interface_info.type;
    i_info.rif_handle = interface_info.rif_handle;
    i_info.vlan = interface_info.vlan;
    i_info.handle = interface_info.handle;
    status = ::switch_api_interface_create(device, &i_info, &intf_handle);
    return intf_handle;
  }

  switcht_status_t switch_api_interface_delete(
      const switcht_device_t device, const switcht_handle_t interface_handle) {
    return ::switch_api_interface_delete(device, interface_handle);
  }

  switcht_handle_t switch_api_interface_handle_get(
                        const switcht_device_t device,
                        const switcht_handle_t intf_handle) {
    switch_handle_t port_handle;
    ::switch_api_interface_handle_get(device, intf_handle, &port_handle);
    return port_handle;
  }

  switcht_ifindex_t switch_api_interface_ifindex_get(
                        const switcht_device_t device,
                        const switcht_handle_t interface_handle) {
    switch_ifindex_t ifindex = 0;
    ::switch_api_interface_ifindex_get(device, interface_handle, &ifindex);
    return ifindex;
  }

  switcht_handle_t switch_api_interface_by_type_get(
                        const switcht_device_t device,
                        const switcht_handle_t handle,
                        const switcht_interface_type_t intf_type) {
    switch_handle_t intf_handle = 0;
    ::switch_api_interface_by_type_get(device, handle,
            (switch_interface_type_t)intf_type, &intf_handle);
    return intf_handle;
  }

  switcht_handle_t switch_api_interface_native_vlan_get(
                        const switcht_device_t device,
                        const switcht_handle_t intf_handle) {
    switch_handle_t vlan_handle;
    ::switch_api_interface_native_vlan_get(device, intf_handle, &vlan_handle);
    return vlan_handle;
  }

  switcht_vlan_t switch_api_interface_native_vlan_id_get(
                        const switcht_device_t device,
                        const switcht_handle_t intf_handle) {
    switch_vlan_t vlan_id;
    ::switch_api_interface_native_vlan_id_get(device, intf_handle, &vlan_id);
    return vlan_id;
  }

  void switch_api_interface_attribute_get(
                        switcht_interface_info_t &_intf_info,
                        const switcht_device_t device,
                        const switcht_handle_t intf_handle,
                        const int64_t intf_flags) {
    switch_api_interface_info_t intf_info;
    ::switch_api_interface_attribute_get(
            device, intf_handle, intf_flags, &intf_info);
    _intf_info.type = intf_info.type;
    _intf_info.handle = intf_info.handle;
    _intf_info.rif_handle = _intf_info.rif_handle;
    _intf_info.vlan = intf_info.vlan;
    _intf_info.native_vlan_handle = intf_info.native_vlan_handle;
    _intf_info.flood_enabled = intf_info.flood_enabled;
    return;
  }

  switcht_handle_t switch_api_interface_ln_handle_get(
                        const switcht_device_t device,
                        const switcht_handle_t intf_handle) {
    switch_handle_t ln_handle;
    ::switch_api_interface_ln_handle_get(device, intf_handle, &ln_handle);
    return ln_handle;
  }

  switcht_handle_t switch_api_rif_create(const switcht_device_t device,
                                         const switcht_rif_info_t &rif_info) {
    switch_handle_t rif_handle = 0;
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    ::switch_api_rif_info_t api_rif_info;

    memset(&api_rif_info, 0, sizeof(::switch_api_rif_info_t));
    api_rif_info.rif_type = (switch_rif_type_t)rif_info.rif_type;
    api_rif_info.vrf_handle = rif_info.vrf_handle;
    api_rif_info.rmac_handle = rif_info.rmac_handle;
    api_rif_info.ipv4_urpf_mode = (switch_urpf_mode_t)rif_info.v4_urpf_mode;
    api_rif_info.ipv6_urpf_mode = (switch_urpf_mode_t)rif_info.v6_urpf_mode;
    api_rif_info.ipv4_unicast = rif_info.v4_unicast_enabled;
    api_rif_info.ipv6_unicast = rif_info.v6_unicast_enabled;
    api_rif_info.ipv4_multicast = rif_info.v4_multicast_enabled;
    api_rif_info.ipv6_multicast = rif_info.v6_multicast_enabled;
    api_rif_info.nat_mode = (switch_nat_mode_t)rif_info.nat_mode;
    api_rif_info.intf_handle = rif_info.intf_handle;
    api_rif_info.vlan = rif_info.vlan;
    api_rif_info.ln_handle = rif_info.ln_handle;

    status = ::switch_api_rif_create(device, &api_rif_info, &rif_handle);
    return rif_handle;
  }

  switcht_status_t switch_api_rif_delete(const switcht_device_t device,
                                         const switcht_handle_t rif_handle) {
    return ::switch_api_rif_delete(device, rif_handle);
  }

  switcht_status_t switch_api_rif_ipv4_unicast_enabled_set(
      const switcht_handle_t rif_handle, const int64_t value) {
    return 0;
  }

  switcht_status_t switch_api_rif_ipv6_unicast_enabled_set(
      const switcht_handle_t rif_handle, const int64_t value) {
    return 0;
  }

  switcht_status_t switch_api_rif_ipv4_urpf_mode_set(
      const switcht_device_t device,
      const switcht_handle_t rif_handle,
      const int64_t value) {
    return ::switch_api_rif_ipv4_urpf_mode_set(
        device, rif_handle, (switch_urpf_mode_t)value);
  }

  switcht_status_t switch_api_rif_ipv6_urpf_mode_set(
      const switcht_device_t device,
      const switcht_handle_t rif_handle,
      const int64_t value) {
    return ::switch_api_rif_ipv6_urpf_mode_set(
        device, rif_handle, (switch_urpf_mode_t)value);
  }

  switcht_status_t switch_api_interface_native_vlan_set(
      const switcht_device_t device,
      const switcht_handle_t intf_handle,
      const switcht_handle_t vlan_handle) {
    switch_handle_t member_handle = 0;
    (void)member_handle;
    return ::switch_api_interface_native_vlan_set(
        device, intf_handle, vlan_handle, &member_handle);
  }

  switcht_status_t switch_api_rif_mtu_set(const switcht_device_t device,
                                          const switcht_handle_t rif_handle,
                                          const switcht_handle_t mtu_handle) {
    return ::switch_api_rif_mtu_set(device, rif_handle, mtu_handle);
  }

  switcht_status_t switch_api_rif_ingress_acl_label_set(
                const switcht_device_t device,
                const switcht_handle_t rif_handle,
                const int16_t label) {
    return ::switch_api_rif_ingress_acl_label_set(device, rif_handle, label);
  }

  switcht_status_t switch_api_rif_egress_acl_label_set(
                const switcht_device_t device,
                const switcht_handle_t rif_handle,
                const int16_t label) {
    return ::switch_api_rif_egress_acl_label_set(device, rif_handle, label);
  }

  switcht_handle_t switch_api_rif_vrf_handle_get(
                const switcht_device_t device,
                const switcht_handle_t rif_handle) {
    switch_handle_t vrf_handle;
    ::switch_api_rif_vrf_handle_get(device, rif_handle, &vrf_handle);
    return vrf_handle;
  }

  switcht_handle_t switch_api_rif_intf_handle_get(
                const switcht_device_t device,
                const switcht_handle_t rif_handle) {
    switch_handle_t intf_handle;
    ::switch_api_rif_intf_handle_get(device, rif_handle, &intf_handle);
    return intf_handle;
  }

  bool switch_api_rif_ipv4_unicast_get(
                const switcht_device_t device,
                const switcht_handle_t rif_handle) {
    bool ipv4_unicast;
    ::switch_api_rif_ipv4_unicast_get(device, rif_handle, &ipv4_unicast);
    return ipv4_unicast;
  }

  bool switch_api_rif_ipv6_unicast_get(
                const switcht_device_t device,
                const switcht_handle_t rif_handle) {
    bool ipv6_unicast;
    ::switch_api_rif_ipv6_unicast_get(device, rif_handle, &ipv6_unicast);
    return ipv6_unicast;
  }

  bool switch_api_rif_ipv4_multicast_get(
                const switcht_device_t device,
                const switcht_handle_t rif_handle) {
    bool ipv4_multicast;
    ::switch_api_rif_ipv4_multicast_get(device, rif_handle, &ipv4_multicast);
    return ipv4_multicast;
  }

  bool switch_api_rif_ipv6_multicast_get(
                const switcht_device_t device,
                const switcht_handle_t rif_handle) {
    bool ipv6_multicast;
    ::switch_api_rif_ipv6_multicast_get(device, rif_handle, &ipv6_multicast);
    return ipv6_multicast;
  }

  switcht_handle_t switch_api_rif_mtu_get(
                const switcht_device_t device,
                const switcht_handle_t rif_handle) {
    switch_handle_t mtu;
    ::switch_api_rif_mtu_get(device, rif_handle, &mtu);
    return mtu;
  }

  int16_t switch_api_rif_type_get(
                const switcht_device_t device,
                const switcht_handle_t rif_handle) {
    switch_rif_type_t type;
    ::switch_api_rif_type_get(device, rif_handle, &type);
    return (int16_t)type;
  }

  void switch_api_rif_attribute_get(
                switcht_rif_info_t &_api_rif_info,
                const switcht_device_t device,
                const switcht_handle_t rif_handle,
                const int16_t type) {
    switch_api_rif_info_t rif_info;
    ::switch_api_rif_attribute_get(device, rif_handle, type, &rif_info);
    _api_rif_info.vrf_handle = rif_info.vrf_handle;
    _api_rif_info.rmac_handle = rif_info.rmac_handle;
    _api_rif_info.v4_urpf_mode = rif_info.ipv4_urpf_mode;
    _api_rif_info.v6_urpf_mode = rif_info.ipv6_urpf_mode;
    _api_rif_info.v4_unicast_enabled = rif_info.ipv4_unicast;
    _api_rif_info.v6_unicast_enabled = rif_info.ipv6_unicast;
    _api_rif_info.v4_multicast_enabled = rif_info.ipv4_multicast;
    _api_rif_info.v6_multicast_enabled = rif_info.ipv6_multicast;
    _api_rif_info.handle = rif_info.mtu_handle;
    _api_rif_info.nat_mode = rif_info.nat_mode;
    _api_rif_info.intf_handle = rif_info.intf_handle;
    _api_rif_info.vlan = rif_info.vlan;
    _api_rif_info.ln_handle = rif_info.ln_handle;
    _api_rif_info.rif_type = rif_info.rif_type;
    return;
  }

  switcht_handle_t switch_api_rif_rmac_handle_get(
                const switcht_device_t device,
                const switcht_handle_t rif_handle) {
    switch_handle_t rmac_handle;
    ::switch_api_rif_rmac_handle_get(device, rif_handle, &rmac_handle);
    return rmac_handle;
  }

  switcht_handle_t switch_api_rif_ingress_acl_group_get(
                const switcht_device_t device,
                const switcht_handle_t rif_handle) {
    switch_handle_t acl_group_handle;
    ::switch_api_rif_ingress_acl_group_get(device, rif_handle, &acl_group_handle);
    return acl_group_handle;
  }

  switcht_handle_t switch_api_rif_egress_acl_group_get(
                const switcht_device_t device,
                const switcht_handle_t rif_handle) {
    switch_handle_t acl_group_handle;
    ::switch_api_rif_egress_acl_group_get(device, rif_handle, &acl_group_handle);
    return acl_group_handle;
  }

  int16_t switch_api_rif_ingress_acl_label_get(
                const switcht_device_t device,
                const switcht_handle_t rif_handle) {
    switch_uint16_t label;
    ::switch_api_rif_ingress_acl_label_get(device, rif_handle, &label);
    return (int16_t)label;
  }

  int16_t switch_api_rif_egress_acl_label_get(
                const switcht_device_t device,
                const switcht_handle_t rif_handle) {
    switch_uint16_t label;
    ::switch_api_rif_egress_acl_label_get(device, rif_handle, &label);
    return (int16_t)label;
  }

  int32_t switch_api_rif_bd_get(
                const switcht_device_t device,
                const switcht_handle_t rif_handle) {
    switch_uint32_t bd;
    ::switch_api_rif_bd_get(device, rif_handle, &bd);
    return (int32_t)bd;
  }

  switcht_handle_t switch_api_l3_route_nhop_get(
      const switcht_device_t device,
      const switcht_handle_t vrf,
      const switcht_ip_addr_t &ip_addr) {
    switch_handle_t nhop_handle;
    switch_ip_addr_t lip_addr;
    switch_parse_ip_address(ip_addr, &lip_addr);
    ::switch_api_l3_route_nhop_get(
                             device, vrf,
                             &lip_addr,
                             &nhop_handle);
    return nhop_handle;
  }

  switcht_status_t switch_api_l3_interface_address_add(
      const switcht_device_t device,
      const switcht_handle_t rif_handle,
      const switcht_handle_t vrf_handle,
      const switcht_ip_addr_t &ip_addr) {
    switch_ip_addr_t lip_addr;
    switch_api_route_entry_t api_route_entry;
    switch_parse_ip_address(ip_addr, &lip_addr);
    memset(&api_route_entry, 0x0, sizeof(api_route_entry));
    api_route_entry.rif_handle = rif_handle;
    api_route_entry.vrf_handle = vrf_handle;
    memcpy(&api_route_entry.ip_address, &lip_addr, sizeof(lip_addr));
    return ::switch_api_l3_interface_address_add(device, &api_route_entry);
  }

  switcht_status_t switch_api_l3_interface_address_delete(
      const switcht_device_t device,
      const switcht_handle_t rif_handle,
      const switcht_handle_t vrf_handle,
      const switcht_ip_addr_t &ip_addr) {
    switch_ip_addr_t lip_addr;
    switch_api_route_entry_t api_route_entry;
    switch_parse_ip_address(ip_addr, &lip_addr);
    memset(&api_route_entry, 0x0, sizeof(api_route_entry));
    api_route_entry.rif_handle = rif_handle;
    api_route_entry.vrf_handle = vrf_handle;
    memcpy(&api_route_entry.ip_address, &lip_addr, sizeof(lip_addr));
    return ::switch_api_l3_interface_address_delete(device, &api_route_entry);
  }

  int32_t switch_api_route_table_size_get(const switcht_device_t device) {
    switch_size_t size;
    ::switch_api_route_table_size_get(device, &size);
    return (int32_t)size;
  }

  switcht_handle_t switch_api_nhop_create(const switcht_device_t device,
                                          const switcht_api_nhop_info_t &api_nhop_info) {
    switch_handle_t nhop_handle = 0;
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    switch_api_nhop_info_t lapi_nhop_info;
    memset(&lapi_nhop_info, 0, sizeof(switch_api_nhop_info_t));
    lapi_nhop_info.nhop_type = (switch_nhop_type_t)api_nhop_info.nhop_type;
    lapi_nhop_info.nhop_tunnel_type = (switch_nhop_tunnel_type_t)api_nhop_info.nhop_tunnel_type;
    lapi_nhop_info.rewrite_type = (switch_nhop_tunnel_rewrite_type_t)api_nhop_info.rewrite_type;
    lapi_nhop_info.intf_handle = api_nhop_info.intf_handle;
    lapi_nhop_info.vrf_handle = api_nhop_info.vrf_handle;
    lapi_nhop_info.rif_handle = api_nhop_info.rif_handle;
    lapi_nhop_info.tunnel_handle = api_nhop_info.tunnel_handle;
    lapi_nhop_info.mpls_handle = api_nhop_info.mpls_handle;
    lapi_nhop_info.network_handle = api_nhop_info.network_handle;
    lapi_nhop_info.label_stack_handle = api_nhop_info.label_stack_handle;
    lapi_nhop_info.tunnel_vni = api_nhop_info.tunnel_vni;
    switch_parse_ip_address(api_nhop_info.ip_addr, &lapi_nhop_info.ip_addr);
    switch_string_to_mac(api_nhop_info.mac_addr, lapi_nhop_info.mac_addr.mac_addr);
    status = ::switch_api_nhop_create(device, &lapi_nhop_info, &nhop_handle);
    return nhop_handle;
  }

  switcht_status_t switch_api_nhop_delete(const switcht_device_t device,
                                          const switcht_handle_t handle) {
    return ::switch_api_nhop_delete(device, handle);
  }

  void switch_api_nhop_get(
          switcht_api_nhop_info_t& _api_nhop_info,
          const switcht_device_t device,
          const switcht_handle_t nhop_handle) {
    switch_api_nhop_info_t lapi_nhop_info;
    memset(&lapi_nhop_info, 0x0, sizeof(lapi_nhop_info));
    ::switch_api_nhop_get(device, nhop_handle, &lapi_nhop_info);
    _api_nhop_info.vrf_handle = lapi_nhop_info.vrf_handle;
    _api_nhop_info.network_handle = lapi_nhop_info.network_handle;
    _api_nhop_info.rif_handle = lapi_nhop_info.rif_handle;
    _api_nhop_info.tunnel_handle = lapi_nhop_info.tunnel_handle;
    _api_nhop_info.mpls_handle = lapi_nhop_info.mpls_handle;
    _api_nhop_info.intf_handle = lapi_nhop_info.intf_handle;
    _api_nhop_info.label_stack_handle = lapi_nhop_info.label_stack_handle;
    _api_nhop_info.nhop_type = lapi_nhop_info.nhop_type;
    _api_nhop_info.rewrite_type = lapi_nhop_info.rewrite_type;
    _api_nhop_info.nhop_tunnel_type = lapi_nhop_info.nhop_tunnel_type;
    _api_nhop_info.tunnel_vni = lapi_nhop_info.tunnel_vni;
    return;
  }

  switcht_handle_t switch_api_nhop_handle_get(
      const switcht_device_t device,
      const switcht_nhop_key_t &nhop_key) {
    switch_handle_t nhop_handle = 0;
    switch_nhop_key_t lnhop_key;
    memset(&lnhop_key, 0, sizeof(switch_nhop_key_t));
    lnhop_key.handle = nhop_key.handle;
    switch_parse_ip_address(nhop_key.ip_addr, &lnhop_key.ip_addr);
    ::switch_api_nhop_handle_get(device, &lnhop_key, &nhop_handle);
    return nhop_handle;
  }

  switcht_handle_t switch_api_neighbor_handle_get(
      const switcht_device_t device,
      const switcht_handle_t handle) {
    switch_handle_t neigh_handle = 0;
    ::switch_api_neighbor_handle_get(device, handle, &neigh_handle);
    return neigh_handle;
  }

  int16_t switch_api_nhop_id_type_get(const switcht_device_t device,
                                      const switcht_handle_t handle) {
    switch_nhop_id_type_t type;
    ::switch_api_nhop_id_type_get(device, handle, &type);
    return (int16_t)type;
  }

  int32_t switch_api_nhop_table_size_get(const switcht_device_t device) {
    switch_size_t size;
    ::switch_api_nhop_table_size_get(device, &size);
    return (int32_t)size;
  }

  void switch_api_ecmp_members_get(std::vector<switcht_handle_t> &_handles,
                                   const switcht_device_t device,
                                   const switcht_handle_t handle) {
    switch_handle_t *mbrs = NULL;
    switch_uint16_t num_mbrs = 0;

    ::switch_api_ecmp_members_get(device, handle, &num_mbrs, &mbrs);
    for (uint16_t i = 0; i < num_mbrs; i++) {
      _handles.push_back(mbrs[i]);
    }

    SWITCH_FREE(device, mbrs);
    return;
  }

  switcht_handle_t switch_api_neighbor_create(
      const switcht_device_t device, const switcht_api_neighbor_info_t &api_neighbor_info) {
    ::switch_api_neighbor_info_t lapi_neighbor_info;
    switch_handle_t neighbor_handle = 0;
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    memset(&lapi_neighbor_info, 0x0, sizeof(lapi_neighbor_info));
    lapi_neighbor_info.neighbor_type = (switch_neighbor_type_t) api_neighbor_info.neighbor_type;
    lapi_neighbor_info.neighbor_tunnel_type = (switch_neighbor_tunnel_type_t)api_neighbor_info.neighbor_tunnel_type;
    lapi_neighbor_info.rw_type = (switch_neighbor_rw_type_t)api_neighbor_info.rw_type;
    lapi_neighbor_info.nhop_handle = api_neighbor_info.nhop_handle;
    lapi_neighbor_info.rif_handle = api_neighbor_info.rif_handle;
    switch_parse_ip_address(api_neighbor_info.ip_addr, &lapi_neighbor_info.ip_addr);
    switch_string_to_mac(api_neighbor_info.mac_addr, lapi_neighbor_info.mac_addr.mac_addr);
    status =
        ::switch_api_neighbor_create(device, &lapi_neighbor_info, &neighbor_handle);
    return neighbor_handle;
  }

  switcht_status_t switch_api_neighbor_delete(
      const switcht_device_t device, const switcht_handle_t neighbor_handle) {
    return ::switch_api_neighbor_delete(device, neighbor_handle);
  }

  void switch_api_neighbor_entry_rewrite_mac_get(
      switcht_mac_addr_t &_mac,
      const switcht_device_t device,
      const switcht_handle_t neighbor_handle) {
    switch_mac_addr_t mac;
    char dmac[18];
    ::switch_api_neighbor_entry_rewrite_mac_get(device, neighbor_handle, &mac);
    switch_mac_to_string(mac.mac_addr, dmac);
    if (strlen(dmac) == 0) {
      _mac = std::string("NA");
    } else {
      _mac = std::string(dmac);
    }
    return;
  }

  switcht_status_t switch_api_l3_route_add(const switcht_device_t device,
                                           const switcht_handle_t vrf_handle,
                                           const switcht_ip_addr_t &ip_addr,
                                           const switcht_handle_t nhop_handle) {
    switch_ip_addr_t lip_addr;
    switch_api_route_entry_t api_route_entry;
    switch_parse_ip_address(ip_addr, &lip_addr);
    memset(&api_route_entry, 0x0, sizeof(api_route_entry));
    api_route_entry.nhop_handle = nhop_handle;
    api_route_entry.vrf_handle = vrf_handle;
    memcpy(&api_route_entry.ip_address, &lip_addr, sizeof(lip_addr));
    return ::switch_api_l3_route_add(device, &api_route_entry);
  }

  switcht_status_t switch_api_l3_route_update(
      const switcht_device_t device,
      const switcht_handle_t vrf_handle,
      const switcht_ip_addr_t &ip_addr,
      const switcht_handle_t nhop_handle) {
    switch_ip_addr_t lip_addr;
    switch_api_route_entry_t api_route_entry;
    switch_parse_ip_address(ip_addr, &lip_addr);
    memset(&api_route_entry, 0x0, sizeof(api_route_entry));
    api_route_entry.nhop_handle = nhop_handle;
    api_route_entry.vrf_handle = vrf_handle;
    memcpy(&api_route_entry.ip_address, &lip_addr, sizeof(lip_addr));
    return ::switch_api_l3_route_update(device, &api_route_entry);
  }

  switcht_status_t switch_api_l3_route_delete(
      const switcht_device_t device,
      const switcht_handle_t vrf_handle,
      const switcht_ip_addr_t &ip_addr,
      const switcht_handle_t nhop_handle) {
    switch_ip_addr_t lip_addr;
    switch_api_route_entry_t api_route_entry;
    switch_parse_ip_address(ip_addr, &lip_addr);
    memset(&api_route_entry, 0x0, sizeof(api_route_entry));
    api_route_entry.nhop_handle = nhop_handle;
    api_route_entry.vrf_handle = vrf_handle;
    memcpy(&api_route_entry.ip_address, &lip_addr, sizeof(lip_addr));
    return ::switch_api_l3_route_delete(device, &api_route_entry);
  }

  switcht_handle_t switch_api_vlan_create(const switcht_device_t device,
                                          const switcht_vlan_t vlan_id) {
    switch_handle_t vlan_handle = 0;
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    status = ::switch_api_vlan_create(device, vlan_id, &vlan_handle);
    return vlan_handle;
  }

  switcht_handle_t switch_api_l3_route_lookup(
      const switcht_device_t device,
      const switcht_handle_t vrf_handle,
      const switcht_ip_addr_t &ip_addr) {
    switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
    switch_status_t status;
    switch_ip_addr_t lip_addr;
    switch_api_route_entry_t api_route_entry;
    memset(&lip_addr, 0x0, sizeof(lip_addr));
    switch_parse_ip_address(ip_addr, &lip_addr);
    memset(&api_route_entry, 0x0, sizeof(api_route_entry));
    api_route_entry.nhop_handle = nhop_handle;
    api_route_entry.vrf_handle = vrf_handle;
    memcpy(&api_route_entry.ip_address, &lip_addr, sizeof(lip_addr));
    status =
        ::switch_api_l3_route_lookup(device, &api_route_entry, &nhop_handle);
    return nhop_handle;
  }

  switcht_status_t switch_api_vlan_delete(const switcht_device_t device,
                                          const switcht_handle_t vlan_handle) {
    return ::switch_api_vlan_delete(device, vlan_handle);
  }

  switcht_status_t switch_api_vlan_stats_enable(
      const switcht_device_t device, const switcht_handle_t vlan_handle) {
    return 0;
  }

  switcht_status_t switch_api_vlan_stats_disable(
      const switcht_device_t device, const switcht_handle_t vlan_handle) {
    return 0;
  }

  void switch_api_vlan_stats_get(std::vector<switcht_counter_t> &_counters,
                                 const switcht_device_t device,
                                 const switcht_handle_t vlan_handle,
                                 const std::vector<int16_t> &counter_ids) {
    std::vector<int16_t>::const_iterator it = counter_ids.begin();
    switcht_counter_t _counter;
    switch_bd_counter_id_t *counter_id_list =
        (switch_bd_counter_id_t *)SWITCH_MALLOC(
            device, sizeof(switch_bd_counter_id_t), counter_ids.size());
    switch_counter_t *counters = (switch_counter_t *)SWITCH_MALLOC(
        device, sizeof(switch_counter_t), counter_ids.size());
    for (uint32_t i = 0; i < counter_ids.size(); i++, it++) {
      counter_id_list[i] = (switch_bd_counter_id_t)*it;
    }
    ::switch_api_vlan_stats_get(
        device, vlan_handle, counter_ids.size(), counter_id_list, counters);
    for (uint32_t i = 0; i < counter_ids.size(); i++) {
      _counter.num_packets = counters[i].num_packets;
      _counter.num_bytes = counters[i].num_bytes;
      _counters.push_back(_counter);
    }
    SWITCH_FREE(device, counter_id_list);
    SWITCH_FREE(device, counters);
    return;
  }

  /*
   * MAC action is set to forward by default as some PTF tests doesn't
   * pass mac_action attribute for mac_table create/update.
   */
  switcht_status_t switch_api_mac_table_entry_create(
      const switcht_device_t device,
      const switcht_api_mac_entry_t &mac_entry) {
    ::switch_api_mac_entry_t lmac_entry;
    memset(&lmac_entry, 0x0, sizeof(lmac_entry));
    switch_string_to_mac(mac_entry.mac_addr, lmac_entry.mac.mac_addr);
    lmac_entry.network_handle = mac_entry.network_handle;
    lmac_entry.handle = mac_entry.handle;
    lmac_entry.entry_type = (switch_mac_entry_type_t)mac_entry.entry_type;
    lmac_entry.mac_action = SWITCH_MAC_ACTION_FORWARD;
    switch_parse_ip_address(mac_entry.tunnel_ip, &lmac_entry.ip_addr);
    return ::switch_api_mac_table_entry_add(device, &lmac_entry);
  }

  /*
   * MAC action is forward by default as some PTF tests doesn't
   * pass mac_action attribute for mac_table create/update.
   */
  switcht_status_t switch_api_mac_table_entry_update(
      const switcht_device_t device,
      const switcht_api_mac_entry_t &mac_entry) {
    ::switch_api_mac_entry_t lmac_entry;
    memset(&lmac_entry, 0x0, sizeof(lmac_entry));
    switch_string_to_mac(mac_entry.mac_addr, lmac_entry.mac.mac_addr);
    lmac_entry.network_handle = mac_entry.network_handle;
    lmac_entry.handle = mac_entry.handle;
    lmac_entry.entry_type = (switch_mac_entry_type_t)mac_entry.entry_type;
    lmac_entry.mac_action = SWITCH_MAC_ACTION_FORWARD;
    switch_parse_ip_address(mac_entry.tunnel_ip, &lmac_entry.ip_addr);
    return ::switch_api_mac_table_entry_update(device, &lmac_entry);
  }

  switcht_status_t switch_api_mac_table_entry_delete(
      const switcht_device_t device,
      const switcht_api_mac_entry_t &mac_entry) {
    ::switch_api_mac_entry_t lmac_entry;
    memset(&lmac_entry, 0x0, sizeof(lmac_entry));
    switch_string_to_mac(mac_entry.mac_addr, lmac_entry.mac.mac_addr);
    lmac_entry.network_handle = mac_entry.network_handle;
    return ::switch_api_mac_table_entry_delete(device, &lmac_entry);
  }

  switch_status_t switch_api_mac_table_entry_flush(
      const switcht_device_t device,
      const switcht_uint64_t flush_type,
      const switcht_handle_t network_handle,
      const switcht_handle_t intf_handle) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    status = ::switch_api_mac_table_entry_flush(device,
                                                flush_type,
                                                network_handle,
                                                intf_handle,
                                                SWITCH_MAC_ENTRY_DYNAMIC);
    if (status == SWITCH_STATUS_SUCCESS) {
      status = ::switch_api_mac_table_entry_flush(device,
                                                  flush_type,
                                                  network_handle,
                                                  intf_handle,
                                                  SWITCH_MAC_ENTRY_STATIC);
    }
    return status;
  }

  switcht_handle_t switch_api_mac_entry_handle_get(
      const switcht_device_t device,
      const switcht_handle_t vlan_handle,
      const switcht_mac_addr_t &mac) {
    switch_handle_t mac_handle;
    ::switch_api_mac_entry_t mac_entry;
    switch_string_to_mac(mac, mac_entry.mac.mac_addr);
    mac_entry.network_handle = vlan_handle;
    ::switch_api_mac_entry_handle_get(device, &mac_entry, &mac_handle);
    return mac_handle;
  }

  int16_t switch_api_mac_entry_type_get(
      const switcht_device_t device,
      const switcht_handle_t vlan_handle,
      const switcht_mac_addr_t &mac) {
    switch_mac_entry_type_t entry_type;
    ::switch_api_mac_entry_t mac_entry;
    switch_string_to_mac(mac, mac_entry.mac.mac_addr);
    mac_entry.network_handle = vlan_handle;
    ::switch_api_mac_entry_type_get(device, &mac_entry, &entry_type);
    return (int16_t)entry_type;
  }

  switcht_handle_t switch_api_mac_entry_intf_handle_get(
      const switcht_device_t device,
      const switcht_handle_t vlan_handle,
      const switcht_mac_addr_t &mac) {
    switch_handle_t intf_handle;
    ::switch_api_mac_entry_t mac_entry;
    switch_string_to_mac(mac, mac_entry.mac.mac_addr);
    mac_entry.network_handle = vlan_handle;
    ::switch_api_mac_entry_port_id_get(device, &mac_entry, &intf_handle);
    return intf_handle;
  }

  int16_t switch_api_mac_entry_packet_action_get(
      const switcht_device_t device,
      const switcht_handle_t vlan_handle,
      const switcht_mac_addr_t &mac) {
    switch_mac_action_t mac_action;
    ::switch_api_mac_entry_t mac_entry;
    switch_string_to_mac(mac, mac_entry.mac.mac_addr);
    mac_entry.network_handle = vlan_handle;
    ::switch_api_mac_entry_packet_action_get(device, &mac_entry, &mac_action);
    return (int16_t)mac_action;
  }

  switcht_handle_t switch_api_ecmp_create(const switcht_device_t device) {
    switch_handle_t ecmp_handle = 0;
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    status = ::switch_api_ecmp_create(device, &ecmp_handle);
    return ecmp_handle;
  }

  switcht_status_t switch_api_ecmp_delete(const switcht_device_t device,
                                          const switcht_handle_t handle) {
    return ::switch_api_ecmp_delete(device, handle);
  }

  switcht_status_t switch_api_ecmp_member_add(
      const switcht_device_t device,
      const switcht_handle_t handle,
      const int16_t nhop_count,
      const std::vector<switcht_handle_t> &nhop_handle) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    std::vector<switcht_handle_t>::const_iterator it = nhop_handle.begin();
    switch_handle_t *nhop_handle_list = (switch_handle_t *)SWITCH_MALLOC(
        device, sizeof(switch_handle_t), nhop_handle.size());
    for (uint32_t i = 0; i < nhop_handle.size(); i++, it++) {
      nhop_handle_list[i] = (switch_handle_t)*it;
    }
    status = ::switch_api_ecmp_member_add(
        device, handle, nhop_count, nhop_handle_list, NULL);
    SWITCH_FREE(device, nhop_handle_list);
    return status;
  }

  switcht_status_t switch_api_ecmp_member_delete(
      const switcht_device_t device,
      const switcht_handle_t handle,
      const int16_t nhop_count,
      const std::vector<switcht_handle_t> &nhop_handle) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    std::vector<switcht_handle_t>::const_iterator it = nhop_handle.begin();
    switch_handle_t *nhop_handle_list = (switch_handle_t *)SWITCH_MALLOC(
        device, sizeof(switch_handle_t), nhop_handle.size());
    for (uint32_t i = 0; i < nhop_handle.size(); i++, it++) {
      nhop_handle_list[i] = (switch_handle_t)*it;
    }
    status = ::switch_api_ecmp_member_delete(
        device, handle, nhop_count, nhop_handle_list);
    SWITCH_FREE(device, nhop_handle_list);
    return status;
  }

  switcht_status_t switch_api_l3_ecmp_member_activate(
      const switcht_device_t device,
      const switcht_handle_t handle,
      const int16_t nhop_count,
      const std::vector<switcht_handle_t> &nhop_handle) {
    switch_status_t status = 0;
    std::vector<switcht_handle_t>::const_iterator it = nhop_handle.begin();

    switch_handle_t *nhop_handle_list = (switch_handle_t *)SWITCH_MALLOC(
        device, sizeof(switch_handle_t), nhop_handle.size());
    for (uint32_t i = 0; i < nhop_handle.size(); i++, it++) {
      nhop_handle_list[i] = (switch_handle_t)*it;
    }
    status = ::switch_api_ecmp_member_activate(
        device, handle, nhop_count, nhop_handle_list);
    SWITCH_FREE(device, nhop_handle_list);
    return status;
  }

  switcht_status_t switch_api_l3_ecmp_member_deactivate(
      const switcht_device_t device,
      const switcht_handle_t handle,
      const int16_t nhop_count,
      const std::vector<switcht_handle_t> &nhop_handle) {
    switch_status_t status = 0;
    std::vector<switcht_handle_t>::const_iterator it = nhop_handle.begin();

    switch_handle_t *nhop_handle_list = (switch_handle_t *)SWITCH_MALLOC(
        device, sizeof(switch_handle_t), nhop_handle.size());
    for (uint32_t i = 0; i < nhop_handle.size(); i++, it++) {
      nhop_handle_list[i] = (switch_handle_t)*it;
    }
    status = ::switch_api_ecmp_member_deactivate(
        device, handle, nhop_count, nhop_handle_list);
    SWITCH_FREE(device, nhop_handle_list);
    return status;
  }

  switcht_handle_t switch_api_l3_wcmp_create(const switcht_device_t device) {
    switch_handle_t wcmp_handle = 0;
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    status = ::switch_api_wcmp_create(device, &wcmp_handle);
    return wcmp_handle;
  }

  switcht_status_t switch_api_l3_wcmp_delete(const switcht_device_t device,
                                             const switcht_handle_t handle) {
    return ::switch_api_wcmp_delete(device, handle);
  }

  switcht_status_t switch_api_l3_wcmp_member_add(
      const switcht_device_t device,
      const switcht_handle_t handle,
      const int16_t nhop_count,
      const std::vector<switcht_handle_t> &nhop_handle,
      const std::vector<int16_t> &nhop_weight) {
    switch_status_t status = 0;
    std::vector<switcht_handle_t>::const_iterator handle_it =
        nhop_handle.begin();
    switch_handle_t *nhop_handle_list = (switch_handle_t *)SWITCH_MALLOC(
        device, sizeof(switch_handle_t), nhop_handle.size());
    for (uint32_t i = 0; i < nhop_handle.size(); i++, handle_it++) {
      nhop_handle_list[i] = (switch_handle_t)*handle_it;
    }
    std::vector<int16_t>::const_iterator weight_it = nhop_weight.begin();
    uint16_t *nhop_weight_list =
        (uint16_t *)SWITCH_MALLOC(device, sizeof(uint16_t), nhop_weight.size());
    for (uint32_t i = 0; i < nhop_weight.size(); i++, weight_it++) {
      nhop_weight_list[i] = (uint16_t)*weight_it;
    }
    status = ::switch_api_wcmp_member_add(
        device, handle, nhop_count, nhop_handle_list, nhop_weight_list);
    SWITCH_FREE(device, nhop_handle_list);
    SWITCH_FREE(device, nhop_weight_list);
    return status;
  }

  switcht_status_t switch_api_l3_wcmp_member_modify(
      const switcht_device_t device,
      const switcht_handle_t handle,
      const int16_t nhop_count,
      const std::vector<switcht_handle_t> &nhop_handle,
      const std::vector<int16_t> &nhop_weight) {
    switch_status_t status = 0;
    std::vector<switcht_handle_t>::const_iterator handle_it =
        nhop_handle.begin();
    switch_handle_t *nhop_handle_list = (switch_handle_t *)SWITCH_MALLOC(
        device, sizeof(switch_handle_t), nhop_handle.size());
    for (uint32_t i = 0; i < nhop_handle.size(); i++, handle_it++) {
      nhop_handle_list[i] = (switch_handle_t)*handle_it;
    }
    std::vector<int16_t>::const_iterator weight_it = nhop_weight.begin();
    uint16_t *nhop_weight_list =
        (uint16_t *)SWITCH_MALLOC(device, sizeof(uint16_t), nhop_weight.size());
    for (uint32_t i = 0; i < nhop_weight.size(); i++, weight_it++) {
      nhop_weight_list[i] = (uint16_t)*weight_it;
    }
    status = ::switch_api_wcmp_member_modify(
        device, handle, nhop_count, nhop_handle_list, nhop_weight_list);
    SWITCH_FREE(device, nhop_handle_list);
    SWITCH_FREE(device, nhop_weight_list);
    return status;
  }

  switcht_status_t switch_api_l3_wcmp_member_delete(
      const switcht_device_t device,
      const switcht_handle_t handle,
      const int16_t nhop_count,
      const std::vector<switcht_handle_t> &nhop_handle) {
    switch_status_t status = 0;
    std::vector<switcht_handle_t>::const_iterator it = nhop_handle.begin();
    switch_handle_t *nhop_handle_list = (switch_handle_t *)SWITCH_MALLOC(
        device, sizeof(switch_handle_t), nhop_handle.size());
    for (uint32_t i = 0; i < nhop_handle.size(); i++, it++) {
      nhop_handle_list[i] = (switch_handle_t)*it;
    }
    status = ::switch_api_wcmp_member_delete(
        device, handle, nhop_count, nhop_handle_list);
    SWITCH_FREE(device, nhop_handle_list);
    return status;
  }

  switcht_handle_t switch_api_lag_create(const switcht_device_t device) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    switch_handle_t lag_handle = 0;
    status = ::switch_api_lag_create(device, &lag_handle);
    return lag_handle;
  }

  switcht_status_t switch_api_lag_delete(const switcht_device_t device,
                                         const switcht_handle_t lag_handle) {
    return ::switch_api_lag_delete(device, lag_handle);
  }

  switcht_status_t switch_api_lag_member_add(
      const switcht_device_t device,
      const switcht_handle_t lag_handle,
      const switcht_direction_t side,
      const switcht_handle_t port_handle) {
    return ::switch_api_lag_member_add(
        device, lag_handle, (switch_direction_t)side, port_handle);
  }

  switcht_status_t switch_api_lag_member_delete(
      const switcht_device_t device,
      const switcht_handle_t lag_handle,
      const switcht_direction_t side,
      const switcht_handle_t port_handle) {
    return ::switch_api_lag_member_delete(
        device, lag_handle, (switch_direction_t)side, port_handle);
  }

  switcht_status_t switch_api_lag_member_activate(
      const switcht_device_t device,
      const switcht_handle_t lag_handle,
      const switcht_handle_t port_handle) {
    return ::switch_api_lag_member_activate(device, lag_handle, port_handle);
  }

  switcht_status_t switch_api_lag_member_deactivate(
      const switcht_device_t device,
      const switcht_handle_t lag_handle,
      const switcht_handle_t port_handle) {
    return ::switch_api_lag_member_deactivate(device, lag_handle, port_handle);
  }

  switcht_status_t switch_api_lag_ingress_acl_label_set(
                const switcht_device_t device,
                const switcht_handle_t lag_handle,
                const int16_t label) {
    return ::switch_api_lag_ingress_acl_label_set(device, lag_handle, label);
  }

  switcht_status_t switch_api_lag_egress_acl_label_set(
                const switcht_device_t device,
                const switcht_handle_t lag_handle,
                const int16_t label) {
    return ::switch_api_lag_egress_acl_label_set(device, lag_handle, label);
  }

  switcht_handle_t switch_api_lag_ingress_acl_group_get(
                const switcht_device_t device,
                const switcht_handle_t lag_handle) {
    switch_handle_t acl_group;
    ::switch_api_lag_ingress_acl_group_get(device, lag_handle, &acl_group);
    return acl_group;
  }

  switcht_handle_t switch_api_lag_egress_acl_group_get(
                const switcht_device_t device,
                const switcht_handle_t lag_handle) {
    switch_handle_t acl_group;
    ::switch_api_lag_egress_acl_group_get(device, lag_handle, &acl_group);
    return acl_group;
  }

  int16_t switch_api_lag_ingress_acl_label_get(
                const switcht_device_t device,
                const switcht_handle_t lag_handle) {
    switch_uint16_t label;;
    ::switch_api_lag_ingress_acl_label_get(device, lag_handle, &label);
    return label;
  }

  int16_t switch_api_lag_egress_acl_label_get(
                const switcht_device_t device,
                const switcht_handle_t lag_handle) {
    switch_uint16_t label;;
    ::switch_api_lag_egress_acl_label_get(device, lag_handle, &label);
    return label;
  }

  int32_t switch_api_lag_bind_mode_get(
                const switcht_device_t device,
                const switcht_handle_t lag_handle) {
    switch_port_bind_mode_t bind_mode;
    ::switch_api_lag_bind_mode_get(device, lag_handle, &bind_mode);
    return (int32_t)bind_mode;
  }

  void switch_api_lag_members_get(
                std::vector<switcht_handle_t> &_member_handles,
                const switcht_device_t device,
                const switcht_handle_t lag_handle) {
    switch_handle_t *member_handles;
    switch_uint32_t count = 0;
    ::switch_api_lag_member_count_get(device, lag_handle, &count);
    member_handles = (switch_handle_t*)SWITCH_MALLOC(
            device, sizeof(switch_handle_t), count);
    ::switch_api_lag_members_get(device, lag_handle, member_handles);
    for (uint8_t i = 0; i < count; i++) {
      _member_handles.push_back(member_handles[i]);
    }

    SWITCH_FREE(device, member_handles);
    return;
  }

  int32_t switch_api_lag_member_count_get(
                const switcht_device_t device,
                const switcht_handle_t lag_handle) {
    switch_uint32_t count;
    ::switch_api_lag_member_count_get(device, lag_handle, &count);
    return (int32_t)count;
  }

  switcht_handle_t switch_api_lag_member_port_handle_get(
                const switcht_device_t device,
                const switcht_handle_t lag_member_handle) {
    switch_handle_t port_handle;
    ::switch_api_lag_member_port_handle_get(
            device, lag_member_handle, &port_handle);
    return port_handle;
  }

  switcht_handle_t swich_api_lag_handle_from_lag_member_get(
                const switcht_device_t device,
                const switcht_handle_t lag_member_handle) {
    switch_handle_t lag_handle;
    ::swich_api_lag_handle_from_lag_member_get(
            device, lag_member_handle, &lag_handle);
    return lag_handle;
  }

  switcht_status_t switch_api_fast_failover_enable(
      const switcht_device_t device) {
    return ::switch_api_fast_failover_enable(device);
  }

  switcht_status_t switch_api_fast_failover_disable(
      const switcht_device_t device) {
    return ::switch_api_fast_failover_disable(device);
  }

  switcht_handle_t switch_api_logical_network_create(
      const switcht_device_t device, const switcht_logical_network_t &info) {
    switch_handle_t ln_handle = 0;
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    status = ::switch_api_logical_network_create(device, &ln_handle);
    return ln_handle;
  }

  switcht_status_t switch_api_logical_network_delete(
      const switcht_device_t device, const switcht_handle_t network_handle) {
    return ::switch_api_logical_network_delete(device, network_handle);
  }

  switcht_status_t switch_api_logical_network_learning_set(
      const switcht_device_t device,
      const switcht_handle_t ln_handle,
      const bool enable) {
    return ::switch_api_logical_network_learning_set(device, ln_handle, enable);
  }

  bool switch_api_logical_network_learning_get(
      const switcht_device_t device,
      const switcht_handle_t ln_handle) {
    bool learning;
    ::switch_api_logical_network_learning_get(device, ln_handle, &learning);
    return learning;
  }

  int32_t switch_api_logical_network_bd_get(
                const switcht_device_t device,
                const switcht_handle_t ln_handle) {
    switch_uint32_t bd;
    ::switch_api_logical_network_bd_get(device, ln_handle, &bd);
    return (int32_t)bd;
  }

  void switch_api_logical_network_stats_get(std::vector<switcht_counter_t> &_counters,
                                 const switcht_device_t device,
                                 const switcht_handle_t ln_handle,
                                 const std::vector<int16_t> &counter_ids) {
    std::vector<int16_t>::const_iterator it = counter_ids.begin();
    switcht_counter_t _counter;
    switch_bd_counter_id_t *counter_id_list =
        (switch_bd_counter_id_t *)SWITCH_MALLOC(
            device, sizeof(switch_bd_counter_id_t), counter_ids.size());
    switch_counter_t *counters = (switch_counter_t *)SWITCH_MALLOC(
        device, sizeof(switch_counter_t), counter_ids.size());
    for (uint32_t i = 0; i < counter_ids.size(); i++, it++) {
      counter_id_list[i] = (switch_bd_counter_id_t)*it;
    }
    ::switch_api_logical_network_stats_get(
        device, ln_handle, counter_ids.size(), counter_id_list, counters);
    for (uint32_t i = 0; i < counter_ids.size(); i++) {
      _counter.num_packets = counters[i].num_packets;
      _counter.num_bytes = counters[i].num_bytes;
      _counters.push_back(_counter);
    }
    SWITCH_FREE(device, counter_id_list);
    SWITCH_FREE(device, counters);
    return;
  }

  switcht_handle_t switch_api_tunnel_create(
      const switcht_device_t device,
      const switcht_api_tunnel_info_t& api_tunnel_info) {
      switch_handle_t tunnel_handle = 0;
      switch_api_tunnel_info_t ltunnel_info;
      memset(&ltunnel_info, 0x0, sizeof(ltunnel_info));
      ltunnel_info.tunnel_type = (switch_tunnel_type_t) api_tunnel_info.tunnel_type;
      ltunnel_info.entry_type = (switch_tunnel_entry_type_t) api_tunnel_info.entry_type;
      ltunnel_info.ip_type = (switch_tunnel_ip_addr_type_t)api_tunnel_info.ip_type;
      ltunnel_info.direction = (switch_direction_t) api_tunnel_info.direction;
      ltunnel_info.decap_mapper_handle = api_tunnel_info.decap_mapper_handle;
      ltunnel_info.encap_mapper_handle = api_tunnel_info.encap_mapper_handle;
      ltunnel_info.underlay_rif_handle = api_tunnel_info.underlay_rif_handle;
      ltunnel_info.overlay_rif_handle = api_tunnel_info.overlay_rif_handle;
      ltunnel_info.erspan_span_id = api_tunnel_info.erspan_span_id;
      switch_parse_ip_address(api_tunnel_info.src_ip, &ltunnel_info.src_ip);
      ::switch_api_tunnel_create(device, &ltunnel_info, &tunnel_handle);
      return tunnel_handle;
  }

  switcht_status_t switch_api_tunnel_delete(
      const switcht_device_t device,
      const switcht_handle_t tunnel_handle) {
      return ::switch_api_tunnel_delete(device, tunnel_handle);
  }

  switcht_handle_t switch_api_tunnel_term_create(
      const switcht_device_t device,
      const switcht_api_tunnel_term_info_t& api_tunnel_term_info) {
      switch_handle_t tunnel_term_handle = 0;
      switch_api_tunnel_term_info_t ltunnel_term_info;
      memset(&ltunnel_term_info, 0x0, sizeof(ltunnel_term_info));
      switch_parse_ip_address(api_tunnel_term_info.src_ip, &ltunnel_term_info.src_ip);
      switch_parse_ip_address(api_tunnel_term_info.dst_ip, &ltunnel_term_info.dst_ip);
      ltunnel_term_info.vrf_handle = api_tunnel_term_info.vrf_handle;
      ltunnel_term_info.tunnel_handle = api_tunnel_term_info.tunnel_handle;
      ltunnel_term_info.tunnel_type = (switch_tunnel_type_t) api_tunnel_term_info.tunnel_type;
      ltunnel_term_info.term_entry_type = (switch_tunnel_term_entry_type_t) api_tunnel_term_info.term_entry_type;
      ::switch_api_tunnel_term_create(device, &ltunnel_term_info, &tunnel_term_handle);
      return tunnel_term_handle;
  }

  switcht_status_t switch_api_tunnel_term_delete(
      const switcht_device_t device,
      const switcht_handle_t tunnel_term_handle) {
      return ::switch_api_tunnel_term_delete(device, tunnel_term_handle);
  }

  switcht_handle_t switch_api_tunnel_mapper_create(
      const switcht_device_t device,
      const switcht_api_tunnel_mapper_t& tunnel_mapper) {
      switch_handle_t mapper_handle = 0;
      switch_api_tunnel_mapper_t ltunnel_mapper;
      memset(&ltunnel_mapper, 0x0, sizeof(ltunnel_mapper));
      ltunnel_mapper.tunnel_map_type = (switch_tunnel_map_type_t) tunnel_mapper.tunnel_map_type;
      ::switch_api_tunnel_mapper_create(device, &ltunnel_mapper, &mapper_handle);
      return mapper_handle;
  }

  switcht_status_t switch_api_tunnel_mapper_delete(
      const switcht_device_t device,
      const switcht_handle_t mapper_handle) {
      return ::switch_api_tunnel_mapper_delete(device, mapper_handle);
  }

  switcht_handle_t switch_api_tunnel_mapper_entry_create(
      const switcht_device_t device,
      const switcht_api_tunnel_mapper_entry_t& tunnel_mapper_entry) {
      switch_handle_t mapper_handle = 0;
      switch_api_tunnel_mapper_entry_t ltunnel_mapper_entry;
      memset(&ltunnel_mapper_entry, 0x0, sizeof(ltunnel_mapper_entry));
      ltunnel_mapper_entry.tunnel_map_type = (switch_tunnel_map_type_t) tunnel_mapper_entry.tunnel_map_type;
      ltunnel_mapper_entry.vrf_handle = tunnel_mapper_entry.vrf_handle;
      ltunnel_mapper_entry.ln_handle = tunnel_mapper_entry.ln_handle;
      ltunnel_mapper_entry.vlan_handle = tunnel_mapper_entry.vlan_handle;
      ltunnel_mapper_entry.tunnel_vni = tunnel_mapper_entry.tunnel_vni;
      ltunnel_mapper_entry.tunnel_mapper_handle = tunnel_mapper_entry.tunnel_mapper_handle;
      ::switch_api_tunnel_mapper_entry_create(device, &ltunnel_mapper_entry, &mapper_handle);
      return mapper_handle;
  }

  switcht_status_t switch_api_tunnel_mapper_entry_delete(
      const switcht_device_t device,
      const switcht_handle_t mapper_entry_handle) {
      return ::switch_api_tunnel_mapper_entry_delete(device, mapper_entry_handle);
  }

  switcht_handle_t switch_api_mpls_tunnel_create(
      const switcht_device_t device,
      const switcht_api_mpls_info_t& api_mpls_info) {
      switch_handle_t mpls_handle = 0;
      switch_api_mpls_info_t lapi_mpls_info;
      memset(&lapi_mpls_info, 0x0, sizeof(lapi_mpls_info));
      lapi_mpls_info.tunnel_type = (switch_mpls_tunnel_type_t) api_mpls_info.tunnel_type;
      lapi_mpls_info.mpls_type = (switch_mpls_type_t) api_mpls_info.mpls_type;
      lapi_mpls_info.mpls_mode = (switch_mpls_mode_t) api_mpls_info.mpls_mode;
      lapi_mpls_info.vrf_handle = api_mpls_info.vrf_handle;
      lapi_mpls_info.network_handle = api_mpls_info.network_handle;
      lapi_mpls_info.nhop_handle = api_mpls_info.nhop_handle;
      lapi_mpls_info.intf_handle = api_mpls_info.intf_handle;
      lapi_mpls_info.swap_label = api_mpls_info.swap_label;
      lapi_mpls_info.pop_label = api_mpls_info.pop_label;
      lapi_mpls_info.pop_count = api_mpls_info.pop_count;
      switch_string_to_mac(api_mpls_info.mac_addr, lapi_mpls_info.mac_addr.mac_addr);
      ::switch_api_mpls_tunnel_create(device, &lapi_mpls_info, &mpls_handle);
      return mpls_handle;
  }

  switcht_status_t switch_api_mpls_tunnel_delete(
      const switcht_device_t device,
      const switcht_handle_t mpls_handle) {
      return ::switch_api_mpls_tunnel_delete(device, mpls_handle);
  }

  switcht_handle_t switch_api_mpls_label_stack_create(
      const switcht_device_t device,
      const switcht_mpls_label_stack_t& label_stack) {
    switch_handle_t label_stack_handle = 0;
    switcht_mpls_t mpls;
    std::vector<switcht_mpls_t>::const_iterator it = label_stack.label_list.begin();
    switch_mpls_label_stack_t llabel_stack;
    memset(&llabel_stack, 0x0, sizeof(llabel_stack));
    for (uint32_t i = 0; i < label_stack.label_list.size(); i++, it++) {
      mpls = (switcht_mpls_t)*it;
      llabel_stack.label_list[i].label = mpls.label;
      llabel_stack.label_list[i].exp = mpls.exp;
      llabel_stack.label_list[i].ttl = mpls.ttl;
    }

    llabel_stack.num_labels = label_stack.label_list.size();
    llabel_stack.bos = label_stack.bos;

    ::switch_api_mpls_label_stack_create(device, &llabel_stack, &label_stack_handle);
    return label_stack_handle;
  }

  switcht_status_t switch_api_mpls_label_stack_delete(
      const switcht_device_t device,
      const switcht_handle_t label_stack_handle) {
      return ::switch_api_mpls_label_stack_delete(device, label_stack_handle);
  }

  switcht_status_t switch_api_logical_network_member_add(
      const switcht_device_t device,
      const switcht_handle_t network_handle,
      const switcht_handle_t interface_handle) {
    return ::switch_api_logical_network_member_add(
        device, network_handle, interface_handle);
  }

  switcht_status_t switch_api_logical_network_member_remove(
      const switcht_device_t device,
      const switcht_handle_t network_handle,
      const switcht_handle_t interface_handle) {
    return ::switch_api_logical_network_member_remove(
        device, network_handle, interface_handle);
  }

  switcht_status_t switch_api_vlan_member_add(
      const switcht_device_t device,
      const switcht_handle_t vlan_handle,
      const switcht_handle_t intf_handle) {
    switch_handle_t member_handle = 0;
    switch_handle_t lintf_handle = intf_handle;
    return (::switch_api_vlan_member_add(
        device, vlan_handle, lintf_handle, &member_handle));
  }

  switcht_status_t switch_api_vlan_member_remove(
      const switcht_device_t device,
      const switcht_handle_t vlan_handle,
      const switcht_handle_t intf_handle) {
    switch_handle_t lintf_handle = intf_handle;
    return (::switch_api_vlan_member_remove(device, vlan_handle, lintf_handle));
  }

  switcht_vlan_t switch_api_vlan_member_vlan_id_get(
      const switcht_device_t device,
      const switcht_handle_t vlan_member_handle) {
    switch_vlan_t vlan_id;
    ::switch_api_vlan_member_vlan_id_get(device, vlan_member_handle, &vlan_id);
    return vlan_id;
  }

  bool switch_api_vlan_member_vlan_tagging_mode_get(
      const switcht_device_t device,
      const switcht_handle_t vlan_member_handle) {
    bool tagging_mode;
    ::switch_api_vlan_member_vlan_tagging_mode_get(
            device, vlan_member_handle, &tagging_mode);
    return tagging_mode;
  }

  switcht_handle_t switch_api_vlan_member_intf_handle_get(
      const switcht_device_t device,
      const switcht_handle_t vlan_member_handle) {
    switch_handle_t intf_handle;
    ::switch_api_vlan_member_intf_handle_get(
            device, vlan_member_handle, &intf_handle);
    return intf_handle;
  }

  void switch_api_vlan_interfaces_get(
      std::vector<switcht_handle_t> &_member_handles,
      const switcht_device_t device,
      const switcht_handle_t vlan_handle) {
    switch_vlan_interface_t *mbrs = NULL;
    switch_uint16_t count = 0;

    ::switch_api_vlan_interfaces_get(device, vlan_handle, &count, &mbrs);
    for (uint16_t i = 0; i < count; i++) {
      _member_handles.push_back(mbrs[i].member_handle);
    }

    SWITCH_FREE(device, mbrs);
    return;
  }

  switcht_status_t switch_api_vlan_ingress_acl_label_set(
                const switcht_device_t device,
                const switcht_handle_t vlan_handle,
                const int16_t label) {
    return ::switch_api_vlan_ingress_acl_label_set(device, vlan_handle, label);
  }

  switcht_status_t switch_api_vlan_egress_acl_label_set(
                const switcht_device_t device,
                const switcht_handle_t vlan_handle,
                const int16_t label) {
    return ::switch_api_vlan_egress_acl_label_set(device, vlan_handle, label);
  }

  switcht_handle_t switch_api_vlan_ingress_acl_group_get(
      const switcht_device_t device,
      const switcht_handle_t vlan_handle) {
    switch_handle_t acl_group;
    ::switch_api_vlan_ingress_acl_group_get(device, vlan_handle, &acl_group);
    return acl_group;
  }

  switcht_handle_t switch_api_vlan_egress_acl_group_get(
      const switcht_device_t device,
      const switcht_handle_t vlan_handle) {
    switch_handle_t acl_group;
    ::switch_api_vlan_egress_acl_group_get(device, vlan_handle, &acl_group);
    return acl_group;
  }

  int16_t switch_api_vlan_ingress_acl_label_get(
      const switcht_device_t device,
      const switcht_handle_t vlan_handle) {
    switch_uint16_t label;
    ::switch_api_vlan_ingress_acl_label_get(device, vlan_handle, &label);
    return (int16_t)label;
  }

  int16_t switch_api_vlan_egress_acl_label_get(
      const switcht_device_t device,
      const switcht_handle_t vlan_handle) {
    switch_uint16_t label;
    ::switch_api_vlan_egress_acl_label_get(device, vlan_handle, &label);
    return (int16_t)label;
  }

  int32_t switch_api_vlan_bd_get(
      const switcht_device_t device,
      const switcht_handle_t vlan_handle) {
    switch_uint32_t bd;
    ::switch_api_vlan_bd_get(device, vlan_handle, &bd);
    return (int32_t)bd;
  }

  switcht_handle_t switch_api_stp_group_create(
      const switcht_device_t device, const switcht_stp_mode_t stp_mode) {
    switch_handle_t stp_handle = 0;
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    status = ::switch_api_stp_group_create(
        device, (switch_stp_mode_t)stp_mode, &stp_handle);
    return stp_handle;
  }

  switcht_status_t switch_api_stp_group_delete(
      const switcht_device_t device, const switcht_handle_t stp_handle) {
    return ::switch_api_stp_group_delete(device, stp_handle);
  }

  switcht_status_t switch_api_stp_group_member_add(
      const switcht_device_t device,
      const switcht_handle_t stp_handle,
      const switcht_handle_t network_handle) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    return ::switch_api_stp_group_member_add(
        device, stp_handle, network_handle);
  }

  switcht_status_t switch_api_stp_group_member_remove(
      const switcht_device_t device,
      const switcht_handle_t stp_handle,
      const switcht_handle_t network_handle) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    return ::switch_api_stp_group_member_remove(
        device, stp_handle, network_handle);
  }

  switcht_status_t switch_api_stp_port_state_set(
      const switcht_device_t device,
      const switcht_handle_t stp_handle,
      const switcht_handle_t handle,
      const switcht_stp_state_t stp_state) {
    return ::switch_api_stp_interface_state_set(
        device, stp_handle, handle, (switch_stp_state_t)stp_state);
  }

  switcht_stp_state_t switch_api_stp_port_state_get(
      const switcht_device_t device,
      const switcht_handle_t stg_handle,
      const switcht_handle_t intf_handle) {
    switch_stp_state_t stp_state;
    ::switch_api_stp_interface_state_get(
        device, stg_handle, intf_handle, &stp_state);
    return stp_state;
  }

  void switch_api_stp_group_members_get(
      std::vector<switcht_handle_t> &_network_handles,
      const switcht_device_t device,
      const switcht_handle_t stp_handle) {
    switch_handle_t *network_handles = NULL;
    switch_uint16_t size;
    ::switch_api_stp_group_members_get(
          device, stp_handle, &size, &network_handles);
    for (uint16_t i = 0; i < size; i++) {
      _network_handles.push_back(network_handles[i]);
    }
    SWITCH_FREE(device, network_handles);
    return;
  }

  void switch_api_stp_interfaces_get(
      std::vector<switcht_handle_t> &_intf_handles,
      const switcht_device_t device,
      const switcht_handle_t stp_handle) {
    switch_handle_t *intf_handles = NULL;
    switch_uint16_t size;
    ::switch_api_stp_interfaces_get(device, stp_handle, &size, &intf_handles);
    for (uint16_t i = 0; i < size; i++) {
      _intf_handles.push_back(intf_handles[i]);
    }
    SWITCH_FREE(device, intf_handles);
    return;
  }

  switcht_status_t switch_api_ila_delete(const switcht_device_t device,
                                         const switcht_handle_t vrf_handle,
                                         const switcht_ip_addr_t &sir) {
    ::switch_api_ila_info_t ila_info;
    memset(&ila_info, 0, sizeof(::switch_api_ila_info_t));
    switch_parse_ip_address(sir, &ila_info.sir_addr);
    ila_info.vrf_handle = vrf_handle;
    return ::switch_api_ila_delete(device, &ila_info);
  }

  switcht_status_t switch_api_ila_update(const switcht_device_t device,
                                         const switcht_handle_t vrf_handle,
                                         const switcht_ip_addr_t &sir,
                                         const switcht_ip_addr_t &ila_addr,
                                         const switcht_handle_t nhop_handle) {
    ::switch_api_ila_info_t ila_info;
    switch_ip_addr_t lila_addr;
    memset(&ila_info, 0, sizeof(::switch_api_ila_info_t));
    switch_parse_ip_address(sir, &ila_info.sir_addr);
    switch_parse_ip_address(ila_addr, &lila_addr);
    ila_info.vrf_handle = vrf_handle;
    return ::switch_api_ila_update(device, &ila_info, lila_addr, nhop_handle);
  }

  switcht_status_t switch_api_ila_add(const switcht_device_t device,
                                      const switcht_handle_t vrf_handle,
                                      const switcht_ip_addr_t &sir,
                                      const switcht_ip_addr_t &ila_addr,
                                      const switcht_handle_t nhop_handle) {
    ::switch_api_ila_info_t ila_info;
    switch_ip_addr_t lila_addr;
    memset(&ila_info, 0, sizeof(::switch_api_ila_info_t));
    switch_parse_ip_address(sir, &ila_info.sir_addr);
    switch_parse_ip_address(ila_addr, &lila_addr);

    ila_info.vrf_handle = vrf_handle;
    return ::switch_api_ila_add(device, &ila_info, lila_addr, nhop_handle);
  }

  switcht_handle_t switch_api_ila_lookup(const switcht_device_t device,
                                         const switcht_handle_t vrf_handle,
                                         const switcht_ip_addr_t &sir) {
    ::switch_api_ila_info_t ila_info;
    switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
    switch_ip_addr_t lila_addr;
    memset(&ila_info, 0, sizeof(::switch_api_ila_info_t));
    switch_parse_ip_address(sir, &ila_info.sir_addr);
    ila_info.vrf_handle = vrf_handle;
    ::switch_api_ila_get(device, &ila_info, &lila_addr, &nhop_handle);
    return nhop_handle;
  }

  switcht_status_t switch_api_nat_create(const switcht_device_t device,
                                         const switcht_nat_info_t &info) {
    ::switch_api_nat_info_t nat_info;
    memset(&nat_info, 0, sizeof(::switch_api_nat_info_t));

    nat_info.nat_rw_type = (switch_nat_rw_type_t)info.nat_rw_type;
    switch (nat_info.nat_rw_type) {
      case SWITCH_NAT_RW_TYPE_SRC_TCP:
      case SWITCH_NAT_RW_TYPE_SRC_UDP:
        nat_info.protocol = info.protocol;
        nat_info.src_port = info.src_port;
        nat_info.rw_src_port = info.rw_src_port;
      case SWITCH_NAT_RW_TYPE_SRC:
        nat_info.src_ip.type = SWITCH_API_IP_ADDR_V4;
        switch_string_to_v4_ip(info.src_ip.ipaddr, &nat_info.src_ip.ip.v4addr);
        nat_info.rw_src_ip.type = SWITCH_API_IP_ADDR_V4;
        switch_string_to_v4_ip(info.rw_src_ip.ipaddr,
                               &nat_info.rw_src_ip.ip.v4addr);
        break;

      case SWITCH_NAT_RW_TYPE_DST_TCP:
      case SWITCH_NAT_RW_TYPE_DST_UDP:
        nat_info.protocol = info.protocol;
        nat_info.dst_port = info.dst_port;
        nat_info.rw_dst_port = info.rw_dst_port;
      case SWITCH_NAT_RW_TYPE_DST:
        nat_info.dst_ip.type = SWITCH_API_IP_ADDR_V4;
        switch_string_to_v4_ip(info.dst_ip.ipaddr, &nat_info.dst_ip.ip.v4addr);
        nat_info.rw_dst_ip.type = SWITCH_API_IP_ADDR_V4;
        switch_string_to_v4_ip(info.rw_dst_ip.ipaddr,
                               &nat_info.rw_dst_ip.ip.v4addr);
        break;

      case SWITCH_NAT_RW_TYPE_SRC_DST_TCP:
      case SWITCH_NAT_RW_TYPE_SRC_DST_UDP:
        nat_info.protocol = info.protocol;
        nat_info.src_port = info.src_port;
        nat_info.dst_port = info.dst_port;
        nat_info.rw_src_port = info.rw_src_port;
        nat_info.rw_dst_port = info.rw_dst_port;
      case SWITCH_NAT_RW_TYPE_SRC_DST:
        nat_info.src_ip.type = SWITCH_API_IP_ADDR_V4;
        switch_string_to_v4_ip(info.src_ip.ipaddr, &nat_info.src_ip.ip.v4addr);
        nat_info.dst_ip.type = SWITCH_API_IP_ADDR_V4;
        switch_string_to_v4_ip(info.dst_ip.ipaddr, &nat_info.dst_ip.ip.v4addr);
        nat_info.rw_src_ip.type = SWITCH_API_IP_ADDR_V4;
        switch_string_to_v4_ip(info.rw_src_ip.ipaddr,
                               &nat_info.rw_src_ip.ip.v4addr);
        nat_info.rw_dst_ip.type = SWITCH_API_IP_ADDR_V4;
        switch_string_to_v4_ip(info.rw_dst_ip.ipaddr,
                               &nat_info.rw_dst_ip.ip.v4addr);
        break;
    }

    nat_info.vrf_handle = info.vrf_handle;
    nat_info.nhop_handle = info.nhop_handle;
    return (::switch_api_nat_add(device, &nat_info));
  }

  switcht_status_t switch_api_nat_delete(const switcht_device_t device,
                                         const switcht_nat_info_t &info) {
    ::switch_api_nat_info_t nat_info;
    memset(&nat_info, 0, sizeof(::switch_api_nat_info_t));

    nat_info.nat_rw_type = (switch_nat_rw_type_t)info.nat_rw_type;
    switch (nat_info.nat_rw_type) {
      case SWITCH_NAT_RW_TYPE_SRC_TCP:
      case SWITCH_NAT_RW_TYPE_SRC_UDP:
        nat_info.protocol = info.protocol;
        nat_info.src_port = info.src_port;
      case SWITCH_NAT_RW_TYPE_SRC:
        nat_info.src_ip.type = SWITCH_API_IP_ADDR_V4;
        switch_string_to_v4_ip(info.src_ip.ipaddr, &nat_info.src_ip.ip.v4addr);
        break;

      case SWITCH_NAT_RW_TYPE_DST_TCP:
      case SWITCH_NAT_RW_TYPE_DST_UDP:
        nat_info.protocol = info.protocol;
        nat_info.dst_port = info.dst_port;
      case SWITCH_NAT_RW_TYPE_DST:
        nat_info.dst_ip.type = SWITCH_API_IP_ADDR_V4;
        switch_string_to_v4_ip(info.dst_ip.ipaddr, &nat_info.dst_ip.ip.v4addr);
        break;

      case SWITCH_NAT_RW_TYPE_SRC_DST_TCP:
      case SWITCH_NAT_RW_TYPE_SRC_DST_UDP:
        nat_info.protocol = info.protocol;
        nat_info.src_port = info.src_port;
        nat_info.dst_port = info.dst_port;
      case SWITCH_NAT_RW_TYPE_SRC_DST:
        nat_info.src_ip.type = SWITCH_API_IP_ADDR_V4;
        switch_string_to_v4_ip(info.src_ip.ipaddr, &nat_info.src_ip.ip.v4addr);
        nat_info.dst_ip.type = SWITCH_API_IP_ADDR_V4;
        switch_string_to_v4_ip(info.dst_ip.ipaddr, &nat_info.dst_ip.ip.v4addr);
        break;
    }

    nat_info.vrf_handle = info.vrf_handle;
    return (::switch_api_nat_delete(device, &nat_info));
  }

  // ACL

  switcht_handle_t switch_api_acl_list_create(
      const switcht_device_t device,
      const switcht_direction_t direction,
      const switcht_acl_type_t type,
      const switcht_handle_type_t bp_type) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    switch_handle_t acl_handle = 0;
    status = ::switch_api_acl_list_create(device,
                                          (switch_direction_t)direction,
                                          (switch_acl_type_t)type,
                                          (switch_handle_type_t)bp_type,
                                          &acl_handle);
    return acl_handle;
  }

  switcht_status_t switch_api_acl_list_delete(const switcht_device_t device,
                                              const switcht_handle_t handle) {
    return ::switch_api_acl_list_delete(device, handle);
  }

  switcht_handle_t switch_api_acl_mac_rule_create(
      const switcht_device_t device,
      const switcht_handle_t acl_handle,
      const int32_t priority,
      const int32_t key_value_count,
      const std::vector<switcht_acl_ip_key_value_pair_t> &acl_kvp,
      const switcht_acl_action_t action,
      const switcht_acl_action_params_t &action_params,
      const switcht_acl_opt_action_params_t &opt_action_params) {
    switch_handle_t handle;
    switch_acl_action_params_t ap;
    switch_acl_opt_action_params_t oap;

    std::vector<switcht_acl_mac_key_value_pair_t>::const_iterator f =
        acl_kvp.begin();

    void *fields = SWITCH_CALLOC(
        device, sizeof(switch_acl_mac_key_value_pair_t) * acl_kvp.size(), 1);
    for (uint32_t i = 0; i < acl_kvp.size(); i++, f++) {
      ((switch_acl_mac_key_value_pair_t *)fields + i)->field =
          (switch_acl_mac_field_t)f->field;
      switch ((switch_acl_mac_field_t)f->field) {
        case SWITCH_ACL_MAC_FIELD_SOURCE_MAC:
        case SWITCH_ACL_MAC_FIELD_DEST_MAC: {
          unsigned char *mac =
              (unsigned char *)(((switch_acl_mac_key_value_pair_t *)fields + i)
                                    ->value.source_mac.mac_addr);
          switch_string_to_mac(f->value.value_str, mac);
          unsigned char *mac_mask =
              (unsigned char *)(&(((switch_acl_mac_key_value_pair_t *)fields +
                                   i)->mask.u.mask));
          switch_string_to_mac(f->mask.value_str, mac_mask);
          break;
        }
        default: {
          unsigned long long v =
              (unsigned long long)((switch_acl_mac_field_t)f->value.value_num);
          memcpy((((switch_acl_mac_key_value_pair_t *)fields + i)
                      ->value.source_mac.mac_addr),
                 &v,
                 sizeof(switch_acl_mac_value));
          ((switch_acl_mac_key_value_pair_t *)fields + i)->mask.u.mask16 =
              (switch_acl_mac_field_t)f->mask.value_num;
          break;
        }
      }
    }
    memset(&ap, 0, sizeof(switch_acl_action_params_t));
    switch ((switch_acl_action_t)action) {
      case SWITCH_ACL_ACTION_REDIRECT:
        ap.redirect.handle = action_params.redirect.handle;
        break;
      case SWITCH_ACL_ACTION_REDIRECT_TO_CPU:
        ap.cpu_redirect.reason_code = action_params.cpu_redirect.reason_code;
        break;
      default:
        break;
    }

    oap.mirror_handle = opt_action_params.mirror_handle;
    oap.meter_handle = opt_action_params.meter_handle;
    oap.counter_handle = opt_action_params.counter_handle;
    oap.nat_mode = (switch_nat_mode_t)opt_action_params.nat_mode;

    /*status =*/ ::switch_api_acl_rule_create(device,
                                              acl_handle,
                                              priority,
                                              key_value_count,
                                              fields,
                                              (switch_acl_action_t)action,
                                              &ap,
                                              &oap,
                                              &handle);
    SWITCH_FREE(device, fields);
    return handle;
  }

  switcht_handle_t switch_api_acl_ip_rule_create(
      const switcht_device_t device,
      const switcht_handle_t acl_handle,
      const int32_t priority,
      const int32_t key_value_count,
      const std::vector<switcht_acl_ip_key_value_pair_t> &acl_kvp,
      const switcht_acl_action_t action,
      const switcht_acl_action_params_t &action_params,
      const switcht_acl_opt_action_params_t &opt_action_params) {
    switch_handle_t handle;
    switch_acl_action_params_t ap;
    switch_acl_opt_action_params_t oap;

    std::vector<switcht_acl_ip_key_value_pair_t>::const_iterator f =
        acl_kvp.begin();

    void *fields = SWITCH_CALLOC(
        device, sizeof(switch_acl_ip_key_value_pair_t) * acl_kvp.size(), 1);
    for (uint32_t i = 0; i < acl_kvp.size(); i++, f++) {
      unsigned long long v =
          (unsigned long long)((switch_acl_ip_field_t)f->value.value_num);
      ((switch_acl_ip_key_value_pair_t *)fields + i)->field =
          (switch_acl_ip_field_t)f->field;
      memcpy(
          &(((switch_acl_ip_key_value_pair_t *)fields + i)->value.ipv4_source),
          &v,
          sizeof(switch_acl_ip_value));
      ((switch_acl_ip_key_value_pair_t *)fields + i)->mask.u.mask =
          (switch_acl_ip_field_t)f->mask.value_num;
    }
    memset(&ap, 0, sizeof(switch_acl_action_params_t));
    switch ((switch_acl_action_t)action) {
      case SWITCH_ACL_ACTION_REDIRECT:
        ap.redirect.handle = action_params.redirect.handle;
        break;
      case SWITCH_ACL_ACTION_REDIRECT_TO_CPU:
        ap.cpu_redirect.reason_code = action_params.cpu_redirect.reason_code;
        break;
      default:
        break;
    }

    oap.mirror_handle = opt_action_params.mirror_handle;
    oap.meter_handle = opt_action_params.meter_handle;
    oap.counter_handle = opt_action_params.counter_handle;
    oap.nat_mode = (switch_nat_mode_t)opt_action_params.nat_mode;

    /*status =*/ ::switch_api_acl_rule_create(device,
                                              acl_handle,
                                              priority,
                                              key_value_count,
                                              fields,
                                              (switch_acl_action_t)action,
                                              &ap,
                                              &oap,
                                              &handle);
    SWITCH_FREE(device, fields);
    return handle;
  }

  switcht_handle_t switch_api_acl_ipv6_rule_create(
      const switcht_device_t device,
      const switcht_handle_t acl_handle,
      const int32_t priority,
      const int32_t key_value_count,
      const std::vector<switcht_acl_ipv6_key_value_pair_t> &acl_kvp,
      const switcht_acl_action_t action,
      const switcht_acl_action_params_t &action_params,
      const switcht_acl_opt_action_params_t &opt_action_params) {
    switch_handle_t handle;
    switch_acl_action_params_t ap;
    switch_acl_opt_action_params_t oap;

    std::vector<switcht_acl_ipv6_key_value_pair_t>::const_iterator f =
        acl_kvp.begin();

    void *fields = SWITCH_CALLOC(
        device, sizeof(switch_acl_ipv6_key_value_pair_t) * acl_kvp.size(), 1);
    for (uint32_t i = 0; i < acl_kvp.size(); i++, f++) {
      ((switch_acl_ipv6_key_value_pair_t *)fields + i)->field =
          (switch_acl_ipv6_field_t)f->field;
      switch ((switch_acl_ipv6_field_t)f->field) {
        case SWITCH_ACL_IPV6_FIELD_IPV6_SRC:
        case SWITCH_ACL_IPV6_FIELD_IPV6_DEST: {
          unsigned char *v6_ip =
              (unsigned char *)(&((switch_acl_ipv6_key_value_pair_t *)fields +
                                  i)->value.ipv6_source);
          switch_string_to_v6_ip(f->value.value_str, v6_ip);
          unsigned char *v6_mask =
              (unsigned char *)(&((switch_acl_ipv6_key_value_pair_t *)fields +
                                  i)->mask.u.mask);
          switch_string_to_v6_ip(f->mask.value_str, v6_mask);
          break;
        }
        default:
          break;
      }
    }
    memset(&ap, 0, sizeof(switch_acl_action_params_t));
    switch ((switch_acl_action_t)action) {
      case SWITCH_ACL_ACTION_REDIRECT:
        ap.redirect.handle = action_params.redirect.handle;
        break;
      case SWITCH_ACL_ACTION_REDIRECT_TO_CPU:
        ap.cpu_redirect.reason_code = action_params.cpu_redirect.reason_code;
        break;
      default:
        break;
    }

    memset(&oap, 0, sizeof(switch_acl_opt_action_params_t));
    oap.mirror_handle = opt_action_params.mirror_handle;
    oap.meter_handle = opt_action_params.meter_handle;
    oap.counter_handle = opt_action_params.counter_handle;
    oap.nat_mode = (switch_nat_mode_t)opt_action_params.nat_mode;

    /*status =*/ ::switch_api_acl_rule_create(device,
                                              acl_handle,
                                              priority,
                                              key_value_count,
                                              fields,
                                              (switch_acl_action_t)action,
                                              &ap,
                                              &oap,
                                              &handle);
    SWITCH_FREE(device, fields);
    return handle;
  }

  switcht_handle_t switch_api_acl_ipv6racl_rule_create(
      const switcht_device_t device,
      const switcht_handle_t acl_handle,
      const int32_t priority,
      const int32_t key_value_count,
      const std::vector<switcht_acl_ipv6racl_key_value_pair_t> &acl_kvp,
      const switcht_acl_action_t action,
      const switcht_acl_action_params_t &action_params,
      const switcht_acl_opt_action_params_t &opt_action_params) {
    switch_handle_t handle;
    switch_acl_action_params_t ap;
    switch_acl_opt_action_params_t oap;

    std::vector<switcht_acl_ipv6racl_key_value_pair_t>::const_iterator f =
        acl_kvp.begin();

    void *fields = SWITCH_CALLOC(
        device,
        sizeof(switch_acl_ipv6_racl_key_value_pair_t) * acl_kvp.size(),
        1);
    for (uint32_t i = 0; i < acl_kvp.size(); i++, f++) {
      ((switch_acl_ipv6_racl_key_value_pair_t *)fields + i)->field =
          (switch_acl_ipv6_racl_field_t)f->field;
      switch ((switch_acl_ipv6_racl_field_t)f->field) {
        case SWITCH_ACL_IPV6_FIELD_IPV6_SRC:
        case SWITCH_ACL_IPV6_FIELD_IPV6_DEST: {
          unsigned char *v6_ip =
              (unsigned char *)(&((switch_acl_ipv6_racl_key_value_pair_t *)
                                      fields +
                                  i)->value.ipv6_source);
          switch_string_to_v6_ip(f->value.value_str, v6_ip);
          unsigned char *v6_mask =
              (unsigned char *)(&((switch_acl_ipv6_racl_key_value_pair_t *)
                                      fields +
                                  i)->mask.u.mask);
          switch_string_to_v6_ip(f->mask.value_str, v6_mask);
          break;
        }
        default:
          break;
      }
    }
    memset(&ap, 0, sizeof(switch_acl_action_params_t));
    switch ((switch_acl_action_t)action) {
      case SWITCH_ACL_ACTION_REDIRECT:
        ap.redirect.handle = action_params.redirect.handle;
        break;
      case SWITCH_ACL_ACTION_REDIRECT_TO_CPU:
        ap.cpu_redirect.reason_code = action_params.cpu_redirect.reason_code;
        break;
      default:
        break;
    }

    oap.mirror_handle = opt_action_params.mirror_handle;
    oap.meter_handle = opt_action_params.meter_handle;
    oap.counter_handle = opt_action_params.counter_handle;
    oap.nat_mode = (switch_nat_mode_t)opt_action_params.nat_mode;

    /*status =*/ ::switch_api_acl_rule_create(device,
                                              acl_handle,
                                              priority,
                                              key_value_count,
                                              fields,
                                              (switch_acl_action_t)action,
                                              &ap,
                                              &oap,
                                              &handle);
    SWITCH_FREE(device, fields);
    return handle;
  }

  switcht_handle_t switch_api_acl_ipracl_rule_create(
      const switcht_device_t device,
      const switcht_handle_t acl_handle,
      const int32_t priority,
      const int32_t key_value_count,
      const std::vector<switcht_acl_ipracl_key_value_pair_t> &acl_kvp,
      const switcht_acl_action_t action,
      const switcht_acl_action_params_t &action_params,
      const switcht_acl_opt_action_params_t &opt_action_params) {
    switch_handle_t handle;
    switch_acl_action_params_t ap;
    switch_acl_opt_action_params_t oap;

    std::vector<switcht_acl_ipracl_key_value_pair_t>::const_iterator f =
        acl_kvp.begin();

    void *fields = SWITCH_CALLOC(
        device,
        sizeof(switch_acl_ip_racl_key_value_pair_t) * acl_kvp.size(),
        1);
    for (uint32_t i = 0; i < acl_kvp.size(); i++, f++) {
      unsigned long long v =
          (unsigned long long)((switch_acl_ip_racl_field_t)f->value.value_num);
      ((switch_acl_ip_racl_key_value_pair_t *)fields + i)->field =
          (switch_acl_ip_racl_field_t)f->field;
      memcpy(&(((switch_acl_ip_racl_key_value_pair_t *)fields + i)
                   ->value.ipv4_source),
             &v,
             sizeof(switch_acl_ip_racl_value));
      ((switch_acl_ip_racl_key_value_pair_t *)fields + i)->mask.u.mask =
          (switch_acl_ip_racl_field_t)f->mask.value_num;
    }
    memset(&ap, 0, sizeof(switch_acl_action_params_t));
    switch ((switch_acl_action_t)action) {
      case SWITCH_ACL_ACTION_REDIRECT:
        ap.redirect.handle = action_params.redirect.handle;
        break;
      case SWITCH_ACL_ACTION_REDIRECT_TO_CPU:
        ap.cpu_redirect.reason_code = action_params.cpu_redirect.reason_code;
        break;
      default:
        break;
    }

    oap.mirror_handle = opt_action_params.mirror_handle;
    oap.meter_handle = opt_action_params.meter_handle;
    oap.counter_handle = opt_action_params.counter_handle;
    oap.nat_mode = (switch_nat_mode_t)opt_action_params.nat_mode;

    /*status =*/ ::switch_api_acl_rule_create(device,
                                              acl_handle,
                                              priority,
                                              key_value_count,
                                              fields,
                                              (switch_acl_action_t)action,
                                              &ap,
                                              &oap,
                                              &handle);
    SWITCH_FREE(device, fields);
    return handle;
  }

  switcht_handle_t switch_api_acl_ipv6_mirror_rule_create(
      const switcht_device_t device,
      const switcht_handle_t acl_handle,
      const int32_t priority,
      const int32_t key_value_count,
      const std::vector<switcht_acl_ipv6_mirror_key_value_pair_t> &acl_kvp,
      const switcht_acl_action_t action,
      const switcht_acl_action_params_t &action_params,
      const switcht_acl_opt_action_params_t &opt_action_params) {
    switch_handle_t handle;
    switch_acl_action_params_t ap;
    switch_acl_opt_action_params_t oap;

    std::vector<switcht_acl_ipv6_mirror_key_value_pair_t>::const_iterator f =
        acl_kvp.begin();

    void *fields = SWITCH_CALLOC(
        device,
        sizeof(switch_acl_ipv6_mirror_acl_key_value_pair_t) * acl_kvp.size(),
        1);
    for (uint32_t i = 0; i < acl_kvp.size(); i++, f++) {
      ((switch_acl_ipv6_mirror_acl_key_value_pair_t *)fields + i)->field =
          (switch_acl_ipv6_mirror_acl_field_t)f->field;
      switch ((switch_acl_ipv6_racl_field_t)f->field) {
        case SWITCH_ACL_IPV6_FIELD_IPV6_SRC:
        case SWITCH_ACL_IPV6_FIELD_IPV6_DEST: {
          unsigned char *v6_ip =
              (unsigned char *)(&((switch_acl_ipv6_mirror_acl_key_value_pair_t *)
                                      fields +
                                  i)->value.ipv6_source);
          switch_string_to_v6_ip(f->value.value_str, v6_ip);
          unsigned char *v6_mask =
              (unsigned char *)(&((switch_acl_ipv6_mirror_acl_key_value_pair_t *)
                                      fields +
                                  i)->mask.u.mask);
          switch_string_to_v6_ip(f->mask.value_str, v6_mask);
          break;
        }
        default:
          break;
      }
    }
    memset(&ap, 0, sizeof(switch_acl_action_params_t));
    switch ((switch_acl_action_t)action) {
      case SWITCH_ACL_ACTION_REDIRECT:
        ap.redirect.handle = action_params.redirect.handle;
        break;
      case SWITCH_ACL_ACTION_REDIRECT_TO_CPU:
        ap.cpu_redirect.reason_code = action_params.cpu_redirect.reason_code;
        break;
      default:
        break;
    }

    oap.mirror_handle = opt_action_params.mirror_handle;
    oap.meter_handle = opt_action_params.meter_handle;
    oap.counter_handle = opt_action_params.counter_handle;
    oap.nat_mode = (switch_nat_mode_t)opt_action_params.nat_mode;

    /*status =*/ ::switch_api_acl_rule_create(device,
                                              acl_handle,
                                              priority,
                                              key_value_count,
                                              fields,
                                              (switch_acl_action_t)action,
                                              &ap,
                                              &oap,
                                              &handle);
    SWITCH_FREE(device, fields);
    return handle;
  }

  switcht_handle_t switch_api_acl_ip_mirror_rule_create(
      const switcht_device_t device,
      const switcht_handle_t acl_handle,
      const int32_t priority,
      const int32_t key_value_count,
      const std::vector<switcht_acl_ip_mirror_key_value_pair_t> &acl_kvp,
      const switcht_acl_action_t action,
      const switcht_acl_action_params_t &action_params,
      const switcht_acl_opt_action_params_t &opt_action_params) {
    std::vector<switcht_acl_ip_mirror_key_value_pair_t>::const_iterator f =
        acl_kvp.begin();
    switch_acl_action_params_t ap;
    switch_acl_opt_action_params_t oap;
    switch_handle_t handle;

    void *fields = SWITCH_CALLOC(
        device,
        sizeof(switch_acl_ip_mirror_acl_key_value_pair_t) * acl_kvp.size(),
        1);

    for (uint32_t i = 0; i < acl_kvp.size(); i++, f++) {
      unsigned long long v =
          (unsigned long long)((switch_acl_ip_mirror_acl_field_t)
                                   f->value.value_num);
      ((switch_acl_ip_mirror_acl_key_value_pair_t *)fields + i)->field =
          (switch_acl_ip_mirror_acl_field_t)f->field;
      memcpy(&(((switch_acl_ip_mirror_acl_key_value_pair_t *)fields + i)
                   ->value.ipv4_source),
             &v,
             sizeof(switch_acl_ip_mirror_acl_value));
      ((switch_acl_ip_mirror_acl_key_value_pair_t *)fields + i)->mask.u.mask =
          (switch_acl_ip_mirror_acl_field_t)f->mask.value_num;
    }
    oap.mirror_handle = opt_action_params.mirror_handle;
    oap.meter_handle = opt_action_params.meter_handle;
    oap.counter_handle = opt_action_params.counter_handle;
    oap.nat_mode = (switch_nat_mode_t)opt_action_params.nat_mode;

    /*status =*/ ::switch_api_acl_rule_create(device,
                                              acl_handle,
                                              priority,
                                              key_value_count,
                                              fields,
                                              (switch_acl_action_t)action,
                                              &ap,
                                              &oap,
                                              &handle);
    SWITCH_FREE(device, fields);
    return handle;
  }

  switcht_handle_t switch_api_acl_system_rule_create(
      const switcht_device_t device,
      const switcht_handle_t acl_handle,
      const int32_t priority,
      const int32_t key_value_count,
      const std::vector<switcht_acl_system_key_value_pair_t> &acl_kvp,
      const switcht_acl_action_t action,
      const switcht_acl_action_params_t &action_params,
      const switcht_acl_opt_action_params_t &opt_action_params) {
    switch_handle_t handle;
    std::vector<switcht_acl_system_key_value_pair_t>::const_iterator f =
        acl_kvp.begin();
    switch_acl_action_params_t ap;
    switch_acl_opt_action_params_t oap;

    void *fields = SWITCH_CALLOC(
        device, sizeof(switch_acl_system_key_value_pair_t) * acl_kvp.size(), 1);
    for (uint32_t i = 0; i < acl_kvp.size(); i++, f++) {
      unsigned long long v =
          (unsigned long long)((switch_acl_system_field_t)f->value.value_num);
      ((switch_acl_system_key_value_pair_t *)fields + i)->field =
          (switch_acl_system_field_t)f->field;
      memcpy(
          &(((switch_acl_system_key_value_pair_t *)fields + i)->value.eth_type),
          &v,
          sizeof(switch_acl_system_value));
      ((switch_acl_system_key_value_pair_t *)fields + i)->mask.u.mask =
          (switch_acl_system_field_t)f->mask.value_num;
    }
    memset(&ap, 0, sizeof(switch_acl_action_params_t));
    switch ((switch_acl_action_t)action) {
      case SWITCH_ACL_ACTION_REDIRECT_TO_CPU:
        ap.cpu_redirect.reason_code = action_params.cpu_redirect.reason_code;
        break;
      case SWITCH_ACL_ACTION_DROP:
        ap.drop.reason_code = action_params.drop.reason_code;
        break;
      default:
        break;
    }

    memset(&oap, 0, sizeof(switch_acl_opt_action_params_t));
    oap.mirror_handle = opt_action_params.mirror_handle;
    oap.meter_handle = opt_action_params.meter_handle;
    oap.counter_handle = opt_action_params.counter_handle;
    oap.nat_mode = (switch_nat_mode_t)opt_action_params.nat_mode;

    /*status =*/ ::switch_api_acl_rule_create(device,
                                              acl_handle,
                                              priority,
                                              key_value_count,
                                              fields,
                                              (switch_acl_action_t)action,
                                              &ap,
                                              &oap,
                                              &handle);
    SWITCH_FREE(device, fields);
    return handle;
  }

  switcht_handle_t switch_api_acl_egress_system_rule_create(
      const switcht_device_t device,
      const switcht_handle_t acl_handle,
      const int32_t priority,
      const int32_t key_value_count,
      const std::vector<switcht_acl_egress_system_key_value_pair_t> &acl_kvp,
      const switcht_acl_action_t action,
      const switcht_acl_action_params_t &action_params,
      const switcht_acl_opt_action_params_t &opt_action_params) {
    switch_handle_t handle;
    std::vector<switcht_acl_egress_system_key_value_pair_t>::const_iterator f =
        acl_kvp.begin();
    switch_acl_action_params_t ap;
    switch_acl_opt_action_params_t oap;

    void *fields = SWITCH_CALLOC(
        device, sizeof(switch_acl_egress_system_key_value_pair_t) * acl_kvp.size(), 1);
    for (uint32_t i = 0; i < acl_kvp.size(); i++, f++) {
      unsigned long long v =
          (unsigned long long)((switch_acl_egress_system_field_t)f->value.value_num);
      ((switch_acl_egress_system_key_value_pair_t *)fields + i)->field =
          (switch_acl_egress_system_field_t)f->field;
      memcpy(&(((switch_acl_egress_system_key_value_pair_t *)fields + i)->value.egr_port),
             &v,
             sizeof(switch_acl_egress_system_value_t));
      ((switch_acl_egress_system_key_value_pair_t *)fields + i)->mask.u.mask =
          (switch_acl_egress_system_field_t)f->mask.value_num;
    }

    memset(&ap, 0, sizeof(switch_acl_action_params_t));
    switch ((switch_acl_action_t)action) {
      case SWITCH_ACL_EGRESS_SYSTEM_ACTION_REDIRECT_TO_CPU:
        ap.cpu_redirect.reason_code = action_params.cpu_redirect.reason_code;
        break;
      case SWITCH_ACL_EGRESS_SYSTEM_ACTION_DROP:
        ap.drop.reason_code = action_params.drop.reason_code;
        break;
      default:
        break;
    }

    oap.mirror_handle = opt_action_params.mirror_handle;
    oap.meter_handle = opt_action_params.meter_handle;
    oap.counter_handle = opt_action_params.counter_handle;
    oap.nat_mode = (switch_nat_mode_t)opt_action_params.nat_mode;

    ::switch_api_acl_rule_create(device,
                                 acl_handle,
                                 priority,
                                 key_value_count,
                                 fields,
                                 (switch_acl_action_t)action,
                                 &ap,
                                 &oap,
                                 &handle);
    SWITCH_FREE(device, fields);
    return handle;
  }

  switcht_status_t switch_api_acl_rule_delete(const switcht_device_t device,
                                              const switcht_handle_t acl_handle,
                                              const switcht_handle_t ace) {
    return ::switch_api_acl_rule_delete(device, acl_handle, ace);
  }

  switcht_status_t switch_api_acl_entry_action_set(
      const switcht_device_t device,
      const switcht_handle_t ace_handle,
      const int32_t priority,
      const switcht_acl_action_t action,
      const switcht_acl_action_params_t &action_params,
      const switcht_acl_opt_action_params_t &opt_action_params) {
    switch_acl_action_params_t ap;
    switch_acl_opt_action_params_t oap;

    memset(&ap, 0, sizeof(switch_acl_action_params_t));
    switch ((switch_acl_action_t)action) {
      case SWITCH_ACL_ACTION_REDIRECT:
        ap.redirect.handle = action_params.redirect.handle;
        break;
      case SWITCH_ACL_ACTION_REDIRECT_TO_CPU:
        ap.cpu_redirect.reason_code = action_params.cpu_redirect.reason_code;
        break;
      case SWITCH_ACL_ACTION_DROP:
        ap.drop.reason_code = action_params.drop.reason_code;
        break;
      default:
        break;
    }

    oap.mirror_handle = opt_action_params.mirror_handle;
    oap.meter_handle = opt_action_params.meter_handle;
    oap.counter_handle = opt_action_params.counter_handle;
    oap.nat_mode = (switch_nat_mode_t)opt_action_params.nat_mode;

    return ::switch_api_acl_entry_action_set(device,
                                             ace_handle,
                                             priority,
                                             (switch_acl_action_t)action,
                                             &ap,
                                             &oap);
  }

  switcht_status_t switch_api_acl_entry_egress_system_action_set(
      const switcht_device_t device,
      const switcht_handle_t ace_handle,
      const int32_t priority,
      const switcht_acl_action_t action,
      const switcht_acl_action_params_t &action_params,
      const switcht_acl_opt_action_params_t &opt_action_params) {
    switch_acl_action_params_t ap;
    switch_acl_opt_action_params_t oap;

    memset(&ap, 0, sizeof(switch_acl_action_params_t));
    switch ((switch_acl_action_t)action) {
      case SWITCH_ACL_EGRESS_SYSTEM_ACTION_REDIRECT_TO_CPU:
        ap.cpu_redirect.reason_code = action_params.cpu_redirect.reason_code;
        break;
      case SWITCH_ACL_EGRESS_SYSTEM_ACTION_DROP:
        ap.drop.reason_code = action_params.drop.reason_code;
        break;
      default:
        break;
    }

    oap.mirror_handle = opt_action_params.mirror_handle;
    oap.meter_handle = opt_action_params.meter_handle;
    oap.counter_handle = opt_action_params.counter_handle;
    oap.nat_mode = (switch_nat_mode_t)opt_action_params.nat_mode;

    return ::switch_api_acl_entry_action_set(device,
                                             ace_handle,
                                             priority,
                                             (switch_acl_action_t)action,
                                             &ap,
                                             &oap);
  }

  switcht_status_t switch_api_acl_reference(
      const switcht_device_t device,
      const switcht_handle_t acl_handle,
      const switcht_handle_t interface_handle) {
    return ::switch_api_acl_reference(device, acl_handle, interface_handle);
  }

  switcht_status_t switch_api_acl_dereference(
      const switcht_device_t device,
      const switcht_handle_t acl_handle,
      const switcht_handle_t interface_handle) {
    return ::switch_api_acl_dereference(device, acl_handle, interface_handle);
  }

  switcht_status_t switch_api_ingress_acl_reference(
      const switcht_device_t device,
      const switcht_handle_t acl_handle,
      const switcht_handle_t interface_handle) {
    return ::switch_api_ingress_acl_reference(device, acl_handle, interface_handle);
  }

  switcht_status_t switch_api_ingress_acl_dereference(
      const switcht_device_t device,
      const switcht_handle_t acl_handle,
      const switcht_handle_t interface_handle) {
    return ::switch_api_ingress_acl_dereference(device, acl_handle, interface_handle);
  }

  switcht_status_t switch_api_egress_acl_reference(
      const switcht_device_t device,
      const switcht_handle_t acl_handle,
      const switcht_handle_t interface_handle) {
    return ::switch_api_egress_acl_reference(device, acl_handle, interface_handle);
  }

  switcht_status_t switch_api_egress_acl_dereference(
      const switcht_device_t device,
      const switcht_handle_t acl_handle,
      const switcht_handle_t interface_handle) {
    return ::switch_api_egress_acl_dereference(device, acl_handle, interface_handle);
  }

  switcht_handle_t switch_api_acl_counter_create(
      const switcht_device_t device) {
    switch_handle_t counter_handle = 0;
    ::switch_api_acl_counter_create(device, &counter_handle);
    return counter_handle;
  }

  switcht_status_t switch_api_acl_counter_delete(
      const switcht_device_t device, const switcht_handle_t counter_handle) {
    return ::switch_api_acl_counter_delete(device, counter_handle);
  }

  void switch_api_acl_stats_get(switcht_counter_t &_counter,
                                const switcht_device_t device,
                                const switcht_handle_t counter_handle) {
    switch_counter_t counter;
    memset(&counter, 0, sizeof(switch_counter_t));
    ::switch_api_acl_counter_get(
        device, (switch_handle_t)counter_handle, &counter);

    _counter.num_packets = counter.num_packets;
    _counter.num_bytes = counter.num_bytes;
    return;
  }

  switcht_handle_t switch_api_racl_counter_create(
      const switcht_device_t device) {
    switch_handle_t counter_handle = 0;
    ::switch_api_racl_counter_create(device, &counter_handle);
    return counter_handle;
  }

  switcht_status_t switch_api_racl_counter_delete(
      const switcht_device_t device, const switcht_handle_t counter_handle) {
    return ::switch_api_racl_counter_delete(device, counter_handle);
  }

  void switch_api_racl_stats_get(switcht_counter_t &_counter,
                                 const switcht_device_t device,
                                 const switcht_handle_t counter_handle) {
    switch_counter_t counter;
    memset(&counter, 0, sizeof(switch_counter_t));
    ::switch_api_racl_counter_get(
        device, (switch_handle_t)counter_handle, &counter);

    _counter.num_packets = counter.num_packets;
    _counter.num_bytes = counter.num_bytes;
    return;
  }

  switcht_handle_t switch_api_egress_acl_counter_create(
      const switcht_device_t device) {
    switch_handle_t counter_handle = 0;
    ::switch_api_egress_acl_counter_create(device, &counter_handle);
    return counter_handle;
  }

  switcht_status_t switch_api_egress_acl_counter_delete(
      const switcht_device_t device, const switcht_handle_t counter_handle) {
    return ::switch_api_egress_acl_counter_delete(device, counter_handle);
  }

  void switch_api_egress_acl_stats_get(switcht_counter_t &_counter,
                                       const switcht_device_t device,
                                       const switcht_handle_t counter_handle) {
    switch_counter_t counter;
    memset(&counter, 0, sizeof(switch_counter_t));
    ::switch_api_egress_acl_counter_get(
        device, (switch_handle_t)counter_handle, &counter);

    _counter.num_packets = counter.num_packets;
    _counter.num_bytes = counter.num_bytes;
    return;
  }

  switcht_handle_t switch_api_acl_range_create(
      const switcht_device_t device,
      const switcht_direction_t direction,
      const switcht_range_type_t range_type,
      const switcht_range_t &range) {
    switch_handle_t range_handle = 0;
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    switch_range_t api_range;
    memset(&api_range, 0x0, sizeof(api_range));
    api_range.start_value = range.start_value;
    api_range.end_value = range.end_value;
    status = ::switch_api_acl_range_create(device,
                                           (switch_direction_t)direction,
                                           (switch_range_type_t)range_type,
                                           &api_range,
                                           &range_handle);
    return range_handle;
  }

  switcht_status_t switch_api_acl_range_update(
      const switcht_device_t device,
      const switcht_handle_t range_handle,
      const switcht_range_t &range) {
    switch_range_t api_range;
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    memset(&api_range, 0x0, sizeof(api_range));
    api_range.start_value = range.start_value;
    api_range.end_value = range.end_value;
    status = ::switch_api_acl_range_update(device, range_handle, &api_range);
    return status;
  }

  switcht_status_t switch_api_acl_range_delete(
      const switcht_device_t device, const switcht_handle_t range_handle) {
    return ::switch_api_acl_range_delete(device, range_handle);
  }

  switcht_acl_type_t switch_api_acl_type_get(
                                    const switcht_device_t device,
                                    const switcht_handle_t acl_handle) {
    switch_acl_type_t acl_type;

    ::switch_api_acl_type_get(device, acl_handle, &acl_type);

    return acl_type;
  }

  switcht_range_type_t switch_api_acl_range_type_get(
                                    const switcht_device_t device,
                                    const switcht_handle_t range_handle) {
    switch_range_type_t range_type;

    ::switch_api_acl_range_type_get(device, range_handle, &range_type);

    return range_type;
  }

  void switch_api_acl_range_get(switcht_range_t &_range,
                                const switcht_device_t device,
                                const switcht_handle_t range_handle) {
    switch_range_t range;

    ::switch_api_acl_range_get(device, range_handle, &range);

    _range.start_value = range.start_value;
    _range.end_value = range.end_value;

    return;
  }

  void switch_api_acl_entry_action_get(switcht_acl_action_spec_t &_spec,
                                         const switcht_device_t device,
                                         const switcht_handle_t ace_handle) {
    switch_acl_action_t action;
    switch_acl_action_params_t action_params;
    switch_acl_opt_action_params_t opt_action_params;

    ::switch_api_acl_entry_action_get(device, ace_handle, &action,
                            &action_params, &opt_action_params);

    _spec.action = action;
    _spec.action_params.redirect.handle = action_params.redirect.handle;
    _spec.action_params.cpu_redirect.reason_code =
          action_params.cpu_redirect.reason_code;
    _spec.opt_action_params.mirror_handle = opt_action_params.mirror_handle;
    _spec.opt_action_params.meter_handle = opt_action_params.meter_handle;
    _spec.opt_action_params.counter_handle = opt_action_params.counter_handle;
    _spec.opt_action_params.nat_mode = opt_action_params.nat_mode;

    return;
  }

  int16_t switch_api_acl_entry_rules_count_get(
                                    const switcht_device_t device,
                                    const switcht_handle_t ace_handle) {
    switch_uint16_t count;

    ::switch_api_acl_entry_rules_count_get(device, ace_handle, &count);

    return (int16_t)count;
  }

  switcht_handle_t switch_api_acl_entry_acl_table_get(
                                    const switcht_device_t device,
                                    const switcht_handle_t ace_handle) {
    switch_handle_t acl_handle;

    ::switch_api_acl_entry_acl_table_get(device, ace_handle, &acl_handle);

    return acl_handle;
  }

  switcht_handle_t switch_api_rpf_create(const switcht_device_t device,
                                         const switcht_rpf_type_t rpf_type,
                                         const switcht_mcast_mode_t pim_mode) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;

    switch_handle_t rpf_group_handle;
    status = ::switch_api_rpf_group_create(device,
                                           (switch_rpf_type_t)rpf_type,
                                           (switch_mcast_mode_t)pim_mode,
                                           &rpf_group_handle);

    return rpf_group_handle;
  }

  switcht_status_t switch_api_rpf_delete(
      const switcht_device_t device, const switcht_handle_t rpf_group_handle) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;

    status = ::switch_api_rpf_group_delete(device, rpf_group_handle);

    return status;
  }

  switcht_status_t switch_api_rpf_member_add(
      const switcht_device_t device,
      const switcht_handle_t rpf_group_handle,
      const switcht_handle_t rif_handle) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;

    status = ::switch_api_rpf_member_add(device, rpf_group_handle, rif_handle);

    return status;
  }

  switcht_status_t switch_api_rpf_member_delete(
      const switcht_device_t device,
      const switcht_handle_t rpf_group_handle,
      const switcht_handle_t rif_handle) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;

    status =
        ::switch_api_rpf_member_delete(device, rpf_group_handle, rif_handle);

    return status;
  }

  void switch_api_rpf_members_get(
      std::vector<switcht_handle_t> &_rpf_handles,
      const switcht_device_t device,
      const switcht_handle_t rpf_group_handle) {
    switch_handle_t *rpf_handles = NULL;
    switch_size_t size;
    ::switch_api_rpf_members_get(device, rpf_group_handle, &size, &rpf_handles);
    for (uint32_t i = 0; i < size; i++) {
      _rpf_handles.push_back(rpf_handles[i]);
    }

    SWITCH_FREE(device, rpf_handles);
    return;
  }

  switcht_handle_t switch_api_multicast_tree_create(
      const switcht_device_t device) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    switch_handle_t mgid_handle = 0;
    status = ::switch_api_multicast_index_create(device, &mgid_handle);
    return mgid_handle;
  }

  switcht_status_t switch_api_multicast_tree_delete(
      const switcht_device_t device, const switcht_handle_t mgid_handle) {
    return ::switch_api_multicast_index_delete(device, mgid_handle);
  }

  switcht_status_t switch_api_multicast_member_add(
      const switcht_device_t device,
      const switcht_handle_t mgid_handle,
      const std::vector<switcht_mcast_member_t> &mbrs) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    std::vector<switcht_mcast_member_t>::const_iterator it = mbrs.begin();

    switch_mcast_member_t *mbr_list = (switch_mcast_member_t *)SWITCH_MALLOC(
        device, sizeof(switch_mcast_member_t), mbrs.size());

    for (uint32_t i = 0; i < mbrs.size(); i++, it++) {
      mbr_list[i].network_handle = ((switcht_mcast_member_t)*it).network_handle;
      mbr_list[i].handle = ((switcht_mcast_member_t)*it).intf_handle;
    }

    status = ::switch_api_multicast_member_add(
        device, mgid_handle, mbrs.size(), mbr_list);
    SWITCH_FREE(device, mbr_list);
    return status;
  }

  switcht_status_t switch_api_multicast_member_delete(
      const switcht_device_t device,
      const switcht_handle_t mgid_handle,
      const std::vector<switcht_mcast_member_t> &mbrs) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    std::vector<switcht_mcast_member_t>::const_iterator it = mbrs.begin();

    switch_mcast_member_t *mbr_list = (switch_mcast_member_t *)SWITCH_MALLOC(
        device, sizeof(switch_mcast_member_t), mbrs.size());

    for (uint32_t i = 0; i < mbrs.size(); i++, it++) {
      mbr_list[i].network_handle = ((switcht_mcast_member_t)*it).network_handle;
      mbr_list[i].handle = ((switcht_mcast_member_t)*it).intf_handle;
    }
    status = ::switch_api_multicast_member_delete(
        device, mgid_handle, mbrs.size(), mbr_list);
    SWITCH_FREE(device, mbr_list);
    return status;
  }

  switcht_status_t switch_api_multicast_ecmp_nhop_add(
      const switcht_device_t device,
      const switcht_handle_t mgid_handle,
      const switcht_handle_t ecmp_nhop_handle) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;

    status = ::switch_api_multicast_ecmp_member_add(
        device, mgid_handle, ecmp_nhop_handle);

    return status;
  }

  switcht_status_t switch_api_multicast_ecmp_nhop_delete(
      const switcht_device_t device,
      const switcht_handle_t mgid_handle,
      const switcht_handle_t ecmp_nhop_handle) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;

    status = ::switch_api_multicast_ecmp_member_delete(
        device, mgid_handle, ecmp_nhop_handle);

    return status;
  }

  switcht_status_t switch_api_multicast_mroute_add(
      const switcht_device_t device,
      const switcht_handle_t mgid_handle,
      const switcht_handle_t rpf_handle,
      const switcht_handle_t vrf_handle,
      const switcht_ip_addr_t &src_ip,
      const switcht_ip_addr_t &grp_ip,
      const switcht_mcast_mode_t mc_mode) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;

    switch_ip_addr_t src_ip_addr;
    switch_ip_addr_t grp_ip_addr;
    switch_parse_ip_address(src_ip, &src_ip_addr);
    switch_parse_ip_address(grp_ip, &grp_ip_addr);

    status = ::switch_api_multicast_mroute_add(device,
                                               mgid_handle,
                                               rpf_handle,
                                               vrf_handle,
                                               &src_ip_addr,
                                               &grp_ip_addr,
                                               (switch_mcast_mode_t)mc_mode);
    return status;
  }

  switcht_status_t switch_api_multicast_mroute_delete(
      const switcht_device_t device,
      const switcht_handle_t vrf_handle,
      const switcht_ip_addr_t &src_ip,
      const switcht_ip_addr_t &grp_ip) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;

    switch_ip_addr_t src_ip_addr;
    switch_ip_addr_t grp_ip_addr;
    switch_parse_ip_address(src_ip, &src_ip_addr);
    switch_parse_ip_address(grp_ip, &grp_ip_addr);

    status = ::switch_api_multicast_mroute_delete(
        device, vrf_handle, &src_ip_addr, &grp_ip_addr);
    return status;
  }

  void switch_api_multicast_mroute_tree_get(
      switcht_mroute_tree_t &_mroute_tree,
      const switcht_device_t device,
      const switcht_handle_t vrf_handle,
      const switcht_ip_addr_t &src_ip,
      const switcht_ip_addr_t &grp_ip) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    switch_handle_t mgid_handle = 0;
    switch_handle_t rpf_handle = 0;

    switch_ip_addr_t src_ip_addr;
    switch_ip_addr_t grp_ip_addr;
    switch_parse_ip_address(src_ip, &src_ip_addr);
    switch_parse_ip_address(grp_ip, &grp_ip_addr);

    status = ::switch_api_multicast_mroute_tree_get(device,
                                                    vrf_handle,
                                                    &src_ip_addr,
                                                    &grp_ip_addr,
                                                    &mgid_handle,
                                                    &rpf_handle);

    _mroute_tree.mgid_handle = mgid_handle;
    _mroute_tree.rpf_handle = rpf_handle;
    return;
  }

  switcht_status_t switch_api_multicast_mroute_miss_mgid_set(
      const switcht_device_t device,
      const switcht_handle_t mgid_handle,
      const switcht_handle_t vlan_handle) {

    return ::switch_api_multicast_mroute_miss_mgid_set(device,
                                                       mgid_handle,
                                                       vlan_handle);
  }

  void switch_api_multicast_mroute_stats_get(
      switcht_counter_t &_counter,
      const switcht_device_t device,
      const switcht_handle_t vrf_handle,
      const switcht_ip_addr_t &src_ip,
      const switcht_ip_addr_t &grp_ip) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;

    switch_ip_addr_t src_ip_addr;
    switch_ip_addr_t grp_ip_addr;
    switch_parse_ip_address(src_ip, &src_ip_addr);
    switch_parse_ip_address(grp_ip, &grp_ip_addr);

    switch_counter_t counter;
    memset(&counter, 0, sizeof(switch_counter_t));

    status = ::switch_api_multicast_mroute_stats_get(device,
                                                  vrf_handle,
                                                  &src_ip_addr,
                                                  &grp_ip_addr,
                                                  &counter);
    _counter.num_packets = counter.num_packets;
    _counter.num_bytes = _counter.num_bytes;
    return;
  }

  switcht_status_t switch_api_multicast_mroute_mgid_set(
      const switcht_device_t device,
      const switcht_handle_t mgid_handle,
      const switcht_handle_t vrf_handle,
      const switcht_ip_addr_t &src_ip,
      const switcht_ip_addr_t &grp_ip,
      const switcht_mcast_mode_t mc_mode) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;

    switch_ip_addr_t src_ip_addr;
    switch_ip_addr_t grp_ip_addr;
    switch_parse_ip_address(src_ip, &src_ip_addr);
    switch_parse_ip_address(grp_ip, &grp_ip_addr);

    status =
        ::switch_api_multicast_mroute_mgid_set(device,
                                               mgid_handle,
                                               vrf_handle,
                                               &src_ip_addr,
                                               &grp_ip_addr,
                                               (switch_mcast_mode_t)mc_mode);
    return status;
  }

  switcht_status_t switch_api_multicast_mroute_rpf_set(
      const switcht_device_t device,
      const switcht_handle_t rpf_handle,
      const switcht_handle_t vrf_handle,
      const switcht_ip_addr_t &src_ip,
      const switcht_ip_addr_t &grp_ip,
      const switcht_mcast_mode_t mc_mode) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;

    switch_ip_addr_t src_ip_addr;
    switch_ip_addr_t grp_ip_addr;
    switch_parse_ip_address(src_ip, &src_ip_addr);
    switch_parse_ip_address(grp_ip, &grp_ip_addr);

    status =
        ::switch_api_multicast_mroute_rpf_set(device,
                                              rpf_handle,
                                              vrf_handle,
                                              &src_ip_addr,
                                              &grp_ip_addr,
                                              (switch_mcast_mode_t)mc_mode);
    return status;
  }

  switcht_status_t switch_api_multicast_l2route_add(
      const switcht_device_t device,
      const switcht_handle_t mgid_handle,
      const switcht_handle_t bd_handle,
      const switcht_ip_addr_t &src_ip,
      const switcht_ip_addr_t &grp_ip) {
    switch_ip_addr_t src_ip_addr;
    switch_ip_addr_t grp_ip_addr;
    switch_parse_ip_address(src_ip, &src_ip_addr);
    switch_parse_ip_address(grp_ip, &grp_ip_addr);
    return ::switch_api_multicast_l2route_add(
        device, mgid_handle, bd_handle, &src_ip_addr, &grp_ip_addr);
  }

  switcht_status_t switch_api_multicast_l2route_delete(
      const switcht_device_t device,
      const switcht_handle_t bd_handle,
      const switcht_ip_addr_t &src_ip,
      const switcht_ip_addr_t &grp_ip) {
    switch_ip_addr_t src_ip_addr;
    switch_ip_addr_t grp_ip_addr;
    switch_parse_ip_address(src_ip, &src_ip_addr);
    switch_parse_ip_address(grp_ip, &grp_ip_addr);
    return ::switch_api_multicast_l2route_delete(
        device, bd_handle, &src_ip_addr, &grp_ip_addr);
  }

  switcht_handle_t switch_api_multicast_l2route_tree_get(
      const switcht_device_t device,
      const switcht_handle_t bd_handle,
      const switcht_ip_addr_t &src_ip,
      const switcht_ip_addr_t &grp_ip) {
    switch_handle_t mgid_handle = 0;
    switch_ip_addr_t src_ip_addr;
    switch_ip_addr_t grp_ip_addr;
    switch_parse_ip_address(src_ip, &src_ip_addr);
    switch_parse_ip_address(grp_ip, &grp_ip_addr);
    ::switch_api_multicast_l2route_tree_get(
         device, bd_handle, &src_ip_addr, &grp_ip_addr, &mgid_handle);
    return mgid_handle;
  }

  switcht_status_t switch_api_vlan_learning_set(
      const switcht_device_t device,
      const switcht_handle_t vlan_handle,
      const bool enable) {
    return ::switch_api_vlan_learning_set(device, vlan_handle, enable);
  }

  bool switch_api_vlan_learning_get(
      const switcht_device_t device,
      const switcht_handle_t vlan_handle) {
    bool enable;
    ::switch_api_vlan_learning_get(device, vlan_handle, &enable);
    return enable;
  }

  void switch_api_vlan_attribute_get(
      switcht_api_vlan_info_t &_vlan_info,
      const switcht_device_t device,
      const switcht_handle_t vlan_handle,
      const int64_t flags) {
    switch_api_vlan_info_t api_vlan_info;
    ::switch_api_vlan_attribute_get(device, vlan_handle, flags, &api_vlan_info);

    _vlan_info.learning_enabled = api_vlan_info.learning_enabled;
    _vlan_info.igmp_snooping_enabled = api_vlan_info.igmp_snooping_enabled;
    _vlan_info.mld_snooping_enabled = api_vlan_info.mld_snooping_enabled;
    _vlan_info.aging_interval = api_vlan_info.aging_interval;
    _vlan_info.stp_handle = api_vlan_info.stp_handle;
    _vlan_info.mrpf_group = api_vlan_info.mrpf_group;

    return;
  }

  switcht_handle_t switch_api_vlan_id_to_handle_get(
      const switcht_device_t device,
      const switcht_vlan_t vlan_id) {
    switch_handle_t vlan_handle;
    ::switch_api_vlan_id_to_handle_get(device, vlan_id, &vlan_handle);
    return vlan_handle;
  }
  switcht_vlan_t switch_api_vlan_handle_to_id_get(
      const switcht_device_t device,
      const switcht_handle_t vlan_handle) {
    switch_vlan_t vlan_id;
    ::switch_api_vlan_handle_to_id_get(device, vlan_handle, &vlan_id);
    return vlan_id;
  }

  switch_status_t switch_api_vlan_igmp_snooping_set(
      const switcht_device_t device,
      const switcht_handle_t vlan_handle,
      const bool enable) {
    return ::switch_api_vlan_igmp_snooping_set(device, vlan_handle, enable);
  }

  bool switch_api_vlan_igmp_snooping_get(
      const switcht_device_t device,
      const switcht_handle_t vlan_handle) {
    bool enable;
    ::switch_api_vlan_igmp_snooping_get(device, vlan_handle, &enable);
    return enable;
  }

  switcht_status_t switch_api_vlan_mld_snooping_set(
      const switcht_device_t device,
      const switcht_handle_t vlan_handle,
      const bool enable) {
    return ::switch_api_vlan_mld_snooping_set(device, vlan_handle, enable);
  }

  bool switch_api_vlan_mld_snooping_get(
      const switcht_device_t device,
      const switcht_handle_t vlan_handle) {
    bool enable;
    ::switch_api_vlan_mld_snooping_get(device, vlan_handle, &enable);
    return enable;
  }

  switcht_status_t switch_api_vlan_mrpf_group_set(
      const switcht_device_t device,
      const switcht_handle_t vlan_handle,
      const int64_t value) {
    return ::switch_api_vlan_mrpf_group_set(device, vlan_handle, value);
  }

  int64_t switch_api_vlan_mrpf_group_get(
      const switcht_device_t device,
      const switcht_handle_t vlan_handle) {
    switch_mrpf_group_t mrpf_group;
    ::switch_api_vlan_mrpf_group_get(device, vlan_handle, &mrpf_group);
    return mrpf_group;
  }

  switcht_status_t switch_api_vlan_stp_handle_set(
      const switcht_device_t device,
      const switcht_handle_t vlan_handle,
      const switcht_handle_t stp_handle) {
    return ::switch_api_vlan_stp_handle_set(device, vlan_handle, stp_handle);
  }

  switcht_handle_t switch_api_vlan_stp_handle_get(
      const switcht_device_t device,
      const switcht_handle_t vlan_handle) {
    switch_handle_t stp_handle;
    ::switch_api_vlan_stp_handle_get(device, vlan_handle, &stp_handle);
    return stp_handle;
  }

  switcht_handle_t switch_api_mirror_session_create(
      const switcht_device_t device,
      const switcht_mirror_info_t &api_mirror_info) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    switch_handle_t mirror_handle = 0;
    ::switch_api_mirror_info_t lapi_mirror_info;
    memset(&lapi_mirror_info, 0, sizeof(::switch_api_mirror_info_t));
    lapi_mirror_info.mirror_type =
        (switch_mirror_type_t)api_mirror_info.mirror_type;
    lapi_mirror_info.session_type =
        (switch_mirror_session_type_t)api_mirror_info.session_type;
    lapi_mirror_info.span_mode =
        (switch_mirror_span_mode_t)api_mirror_info.span_mode;
    lapi_mirror_info.tos = api_mirror_info.tos;
    lapi_mirror_info.max_pkt_len = api_mirror_info.max_pkt_len;
    lapi_mirror_info.egress_port_handle = api_mirror_info.egress_port_handle;
    lapi_mirror_info.direction = (switch_direction_t)api_mirror_info.direction;
    lapi_mirror_info.session_id = api_mirror_info.session_id;
    lapi_mirror_info.nhop_handle = api_mirror_info.nhop_handle;
    lapi_mirror_info.extract_len = api_mirror_info.extract_len;
    lapi_mirror_info.timeout_usec = api_mirror_info.timeout_usec;
    lapi_mirror_info.vlan_tpid = api_mirror_info.vlan_tpid;
    lapi_mirror_info.ttl = api_mirror_info.ttl;
    lapi_mirror_info.vlan_id = api_mirror_info.vlan_id;
    lapi_mirror_info.vrf_handle = api_mirror_info.vrf_handle;
    switch_parse_ip_address(api_mirror_info.src_ip, &lapi_mirror_info.src_ip);
    switch_parse_ip_address(api_mirror_info.dst_ip, &lapi_mirror_info.dst_ip);
    switch_string_to_mac(api_mirror_info.src_mac, lapi_mirror_info.src_mac.mac_addr);
    switch_string_to_mac(api_mirror_info.dst_mac, lapi_mirror_info.dst_mac.mac_addr);
    status = ::switch_api_mirror_session_create(
        device, &lapi_mirror_info, &mirror_handle);
    return mirror_handle;
  }

  switcht_status_t switch_api_mirror_session_update(
      const switcht_device_t device,
      const switcht_handle_t mirror_handle,
      const switcht_mirror_info_t &api_mirror_info) {
    ::switch_api_mirror_info_t lapi_mirror_info;
    memset(&lapi_mirror_info, 0, sizeof(::switch_api_mirror_info_t));
    lapi_mirror_info.mirror_type =
        (switch_mirror_type_t)api_mirror_info.mirror_type;
    lapi_mirror_info.session_type =
        (switch_mirror_session_type_t)api_mirror_info.session_type;
    lapi_mirror_info.tos = api_mirror_info.tos;
    lapi_mirror_info.cos = api_mirror_info.cos;
    lapi_mirror_info.vlan_tpid = api_mirror_info.vlan_tpid;
    lapi_mirror_info.max_pkt_len = api_mirror_info.max_pkt_len;
    lapi_mirror_info.egress_port_handle = api_mirror_info.egress_port_handle;
    lapi_mirror_info.direction = (switch_direction_t)api_mirror_info.direction;
    lapi_mirror_info.session_id = api_mirror_info.session_id;
    lapi_mirror_info.nhop_handle = api_mirror_info.nhop_handle;
    lapi_mirror_info.extract_len = api_mirror_info.extract_len;
    lapi_mirror_info.timeout_usec = api_mirror_info.timeout_usec;
    lapi_mirror_info.ttl = api_mirror_info.ttl;
    lapi_mirror_info.vlan_id = api_mirror_info.vlan_id;
    lapi_mirror_info.vrf_handle = api_mirror_info.vrf_handle;
    switch_parse_ip_address(api_mirror_info.src_ip, &lapi_mirror_info.src_ip);
    switch_parse_ip_address(api_mirror_info.dst_ip, &lapi_mirror_info.dst_ip);
    switch_string_to_mac(api_mirror_info.src_mac, lapi_mirror_info.src_mac.mac_addr);
    switch_string_to_mac(api_mirror_info.dst_mac, lapi_mirror_info.dst_mac.mac_addr);
    lapi_mirror_info.span_mode = (switch_mirror_span_mode_t) api_mirror_info.span_mode;
    return ::switch_api_mirror_session_update(
        device, mirror_handle, 0x0, &lapi_mirror_info);
  }

  int16_t switch_api_mirror_session_type_get(
      const switcht_device_t device,
      const switcht_handle_t mirror_handle) {
    switch_mirror_type_t type;
    ::switch_api_mirror_session_type_get(device, mirror_handle, &type);
    return (int16_t)type;
  }

  void switch_api_mirror_session_info_get(
      switcht_mirror_info_t &_api_mirror_info,
      const switcht_device_t device,
      const switcht_handle_t mirror_handle) {
    switch_api_mirror_info_t api_mirror_info;
    ::switch_api_mirror_session_info_get(
          device, mirror_handle, &api_mirror_info);
    _api_mirror_info.mirror_type = api_mirror_info.mirror_type;
    _api_mirror_info.session_type = api_mirror_info.session_type;
    _api_mirror_info.cos = api_mirror_info.cos;
    _api_mirror_info.tos = api_mirror_info.tos;
    _api_mirror_info.ttl = api_mirror_info.ttl;
    _api_mirror_info.max_pkt_len = api_mirror_info.max_pkt_len;
    _api_mirror_info.egress_port_handle = api_mirror_info.egress_port_handle;
    _api_mirror_info.direction = api_mirror_info.direction;
    _api_mirror_info.session_id = api_mirror_info.session_id;
    _api_mirror_info.nhop_handle = api_mirror_info.nhop_handle;
    _api_mirror_info.extract_len = api_mirror_info.extract_len;
    _api_mirror_info.timeout_usec = api_mirror_info.timeout_usec;
    return;
  }

  switcht_status_t switch_api_mirror_session_delete(
      const switcht_device_t device, const switcht_handle_t mirror_handle) {
    return ::switch_api_mirror_session_delete(device, mirror_handle);
  }

  switcht_status_t switch_api_dtel_switch_id_set(
      const switcht_device_t device, const int32_t switch_id) {
    return ::switch_api_dtel_switch_id_set(device, switch_id);
  }

  switcht_status_t switch_api_dtel_report_udp_dstport_set(
      const switcht_device_t device, const int16_t dest_udp_port) {
    return ::switch_api_dtel_report_udp_dstport_set(device, dest_udp_port);
  }

  switcht_status_t switch_api_dtel_report_session_add(
      const switcht_device_t device, const switcht_mirror_id_t mirror_id) {
    return ::switch_api_dtel_report_session_add(device, mirror_id);
  }

  switcht_status_t switch_api_dtel_report_session_delete(
      const switcht_device_t device, const switcht_mirror_id_t mirror_id) {
    return ::switch_api_dtel_report_session_delete(device, mirror_id);
  }

  switcht_status_t switch_api_dtel_flow_state_clear_cycle(
      const switcht_device_t device, const int16_t cycle) {
    return ::switch_api_dtel_flow_state_clear_cycle(device, cycle);
  }

  switcht_status_t switch_api_dtel_latency_quantization_shift(
      const switcht_device_t device, const int8_t quant_shift) {
    return ::switch_api_dtel_latency_quantization_shift(device,
                                                             quant_shift);
  }

  switcht_status_t switch_api_dtel_queue_report_create(
      const switcht_device_t device,
      const int16_t port,
      const int16_t queue,
      const int32_t depth_threshold,
      const int32_t latency_threshold,
      const int16_t report_quota_during_breach,
      const bool report_tail_drops) {
    return ::switch_api_dtel_queue_report_create(
        device,
        port,
        queue,
        depth_threshold,
        latency_threshold,
        report_quota_during_breach,
        report_tail_drops);
  }

  switcht_status_t switch_api_dtel_queue_report_update(
      const switcht_device_t device,
      const int16_t port,
      const int16_t queue,
      const int32_t depth_threshold,
      const int32_t latency_threshold,
      const int16_t report_quota_during_breach,
      const bool report_tail_drops) {
    return ::switch_api_dtel_queue_report_update(
        device,
        port,
        queue,
        depth_threshold,
        latency_threshold,
        report_quota_during_breach,
        report_tail_drops);
  }

  switcht_status_t switch_api_dtel_queue_report_delete(
      const switcht_device_t device, const int16_t port, const int16_t queue) {
    return ::switch_api_dtel_queue_report_delete(device, port, queue);
  }

  int16_t switch_api_dtel_queue_remaining_report_quota_during_breach_get(
                                 const switcht_device_t device,
                                 const int16_t port, const int16_t queue){
    uint16_t quota = 0;
    ::switch_api_dtel_queue_remaining_report_quota_during_breach_get(
        device, port, queue, &quota);
    return quota;
  }

  switcht_status_t switch_api_dtel_int_watchlist_entry_create(
      const switcht_device_t device,
      const std::vector<switcht_twl_key_value_pair_t> &twl_kvp,
      const int32_t priority,
      const bool watch,
      const switcht_twl_int_params_t &action_params) {
    switch_twl_match_info_t match_info;
    switch_parse_dtel_watchlist_match_info(device, twl_kvp, &match_info);

    switch_twl_action_params_t ap;
    memset(&ap, 0, sizeof(switch_twl_action_params_t));
    ap._int.session_id = action_params.session_id;
    ap._int.report_all_packets = action_params.report_all_packets;
    ap._int.flow_sample_percent = action_params.flow_sample_percent;

    switcht_status_t status = ::switch_api_dtel_watchlist_entry_create(
        device, SWITCH_DTEL_TYPE_INT, &match_info, priority, watch, &ap);
    SWITCH_FREE(device, match_info.fields);
    return status;
  }

  switcht_status_t switch_api_dtel_report_sequence_number_set(
      const switcht_device_t device,
      const int16_t mirror_session_id,
      const int32_t value) {
    return ::switch_api_dtel_report_sequence_number_set(
        device, mirror_session_id, value);
  }

  void switch_api_dtel_report_sequence_number_get(
      std::vector<int32_t> &values,
      const switcht_device_t device,
      const int16_t mirror_session_id,
      const int8_t max_num) {
    switch_uint32_t *seq_numbers = (switch_uint32_t *)SWITCH_MALLOC(
        device, sizeof(switch_uint32_t), max_num);
    uint8_t max_num_ = max_num;
    ::switch_api_dtel_report_sequence_number_get(
        device, mirror_session_id, seq_numbers, &max_num_);
    for (uint8_t i = 0; i < max_num_; i++) {
      values.push_back((int32_t)seq_numbers[i]);
    }
    SWITCH_FREE(device, seq_numbers);
    return;
  }

  int8_t switch_api_dtel_event_get_dscp(
      const switcht_device_t device,
      const switcht_dtel_event_type_t event_type) {
    switch_uint8_t dscp;
    switcht_status_t status = ::switch_api_dtel_event_get_dscp(
        device, (switch_dtel_event_type_t)event_type, &dscp);
    return dscp;
  }

  switcht_status_t switch_api_dtel_event_set_dscp(
      const switcht_device_t device,
      const switcht_dtel_event_type_t event_type,
      const int8_t dscp) {
    return ::switch_api_dtel_event_set_dscp(
        device, (switch_dtel_event_type_t)event_type, dscp);
  }

  switcht_status_t switch_api_dtel_int_watchlist_entry_update(
      const switcht_device_t device,
      const std::vector<switcht_twl_key_value_pair_t> &twl_kvp,
      const int32_t priority,
      const bool watch,
      const switcht_twl_int_params_t &action_params) {
    switch_twl_match_info_t match_info;
    switch_parse_dtel_watchlist_match_info(device, twl_kvp, &match_info);

    switch_twl_action_params_t ap;
    memset(&ap, 0, sizeof(switch_twl_action_params_t));
    ap._int.session_id = action_params.session_id;
    ap._int.report_all_packets = action_params.report_all_packets;
    ap._int.flow_sample_percent = action_params.flow_sample_percent;

    switcht_status_t status = ::switch_api_dtel_watchlist_entry_update(
        device, SWITCH_DTEL_TYPE_INT, &match_info, priority, watch, &ap);
    SWITCH_FREE(device, match_info.fields);
    return status;
  }

  switcht_status_t switch_api_dtel_int_watchlist_entry_delete(
      const switcht_device_t device,
      const std::vector<switcht_twl_key_value_pair_t> &twl_kvp) {
    switch_twl_match_info_t match_info;
    switch_parse_dtel_watchlist_match_info(device, twl_kvp, &match_info);
    switcht_status_t status = ::switch_api_dtel_watchlist_entry_delete(
        device, SWITCH_DTEL_TYPE_INT, &match_info);
    SWITCH_FREE(device, match_info.fields);
    return status;
  }

  switcht_status_t switch_api_dtel_int_watchlist_clear(
      const switcht_device_t device) {
    return ::switch_api_dtel_watchlist_clear(device,
                                                  SWITCH_DTEL_TYPE_INT);
  }

  switcht_status_t switch_api_dtel_int_transit_enable(
      const switcht_device_t device) {
    return ::switch_api_dtel_int_transit_enable(device);
  }

  switcht_status_t switch_api_dtel_int_transit_disable(
      const switcht_device_t device) {
    return ::switch_api_dtel_int_transit_disable(device);
  }

  switcht_status_t switch_api_dtel_int_endpoint_enable(
      const switcht_device_t device) {
    return ::switch_api_dtel_int_endpoint_enable(device);
  }

  switcht_status_t switch_api_dtel_int_endpoint_disable(
      const switcht_device_t device) {
    return ::switch_api_dtel_int_endpoint_disable(device);
  }

  switcht_status_t switch_api_dtel_int_session_create(
      const switcht_device_t device,
      const int16_t session_id,
      const int16_t instruction,
      const int8_t max_hop) {
    return ::switch_api_dtel_int_session_create(
        device, session_id, instruction, max_hop);
  }

  switcht_status_t switch_api_dtel_int_session_update(
      const switcht_device_t device,
      const int16_t session_id,
      const int16_t instruction,
      const int8_t max_hop) {
    return ::switch_api_dtel_int_session_update(
        device, session_id, instruction, max_hop);
  }

  switcht_status_t switch_api_dtel_int_session_delete(
      const switcht_device_t device, const int16_t session_id) {
    return ::switch_api_dtel_int_session_delete(device, session_id);
  }

  switcht_status_t switch_api_dtel_int_sink_ports_add(
      const switcht_device_t device, const int16_t port) {
    return ::switch_api_dtel_int_sink_ports_add(device, port);
  }

  switcht_status_t switch_api_dtel_int_sink_ports_delete(
      const switcht_device_t device, const int16_t port) {
    return ::switch_api_dtel_int_sink_ports_delete(device, port);
  }

  switcht_status_t switch_api_dtel_int_dscp_value_set(
      const switcht_device_t device, const int8_t value, const int8_t mask) {
    return ::switch_api_dtel_int_dscp_value_set(device, value, mask);
  }

  switcht_status_t switch_api_dtel_int_marker_set(
      const switcht_device_t device, const int8_t proto, const int64_t marker) {
    return ::switch_api_dtel_int_marker_set(device, proto, marker);
  }

  int64_t switch_api_dtel_int_marker_get(
      const switcht_device_t device, const int8_t proto) {
      switch_uint64_t marker;
    ::switch_api_dtel_int_marker_get(device, proto, &marker);
    return (int64_t)marker;
  }

  switcht_status_t switch_api_dtel_int_marker_delete(
      const switcht_device_t device, const int8_t proto) {
    return ::switch_api_dtel_int_marker_delete(device, proto);
  }

  switcht_status_t switch_api_dtel_int_marker_port_clear(
      const switcht_device_t device, const int8_t proto) {
    return ::switch_api_dtel_int_marker_port_clear(device, proto);
  }

  switcht_status_t switch_api_dtel_int_marker_port_add(
      const switcht_device_t device, const int8_t proto,
      const int16_t value, const int16_t mask) {
    return ::switch_api_dtel_int_marker_port_add(
            device, proto, value, mask);
  }

  switcht_status_t switch_api_dtel_int_marker_port_delete(
      const switcht_device_t device, const int8_t proto,
      const int16_t value, const int16_t mask) {
    return ::switch_api_dtel_int_marker_port_delete(
            device, proto,value, mask);
  }

  switcht_status_t switch_api_dtel_postcard_enable(
      const switcht_device_t device) {
    return ::switch_api_dtel_postcard_enable(device);
  }

  switcht_status_t switch_api_dtel_postcard_disable(
      const switcht_device_t device) {
    return ::switch_api_dtel_postcard_disable(device);
  }

  switcht_status_t switch_api_dtel_postcard_watchlist_entry_create(
      const switcht_device_t device,
      const std::vector<switcht_twl_key_value_pair_t> &twl_kvp,
      const int32_t priority,
      const bool watch,
      const switcht_twl_postcard_params_t &action_params) {
    switch_twl_match_info_t match_info;
    switch_parse_dtel_watchlist_match_info(device, twl_kvp, &match_info);

    switch_twl_action_params_t ap;
    memset(&ap, 0, sizeof(switch_twl_action_params_t));
    ap._postcard.report_all_packets = action_params.report_all_packets;
    ap._postcard.flow_sample_percent = action_params.flow_sample_percent;

    switcht_status_t status =
        ::switch_api_dtel_watchlist_entry_create(device,
                                                 SWITCH_DTEL_TYPE_POSTCARD,
                                                 &match_info,
                                                 priority,
                                                 watch,
                                                 &ap);
    SWITCH_FREE(device, match_info.fields);
    return status;
  }

  switcht_status_t switch_api_dtel_postcard_watchlist_entry_update(
      const switcht_device_t device,
      const std::vector<switcht_twl_key_value_pair_t> &twl_kvp,
      const int32_t priority,
      const bool watch,
      const switcht_twl_postcard_params_t &action_params) {
    switch_twl_match_info_t match_info;
    switch_parse_dtel_watchlist_match_info(device, twl_kvp, &match_info);

    switch_twl_action_params_t ap;
    memset(&ap, 0, sizeof(switch_twl_action_params_t));
    ap._postcard.report_all_packets = action_params.report_all_packets;
    ap._postcard.flow_sample_percent = action_params.flow_sample_percent;

    switcht_status_t status =
        ::switch_api_dtel_watchlist_entry_update(device,
                                                 SWITCH_DTEL_TYPE_POSTCARD,
                                                 &match_info,
                                                 priority,
                                                 watch,
                                                 &ap);
    SWITCH_FREE(device, match_info.fields);
    return status;
  }

  switcht_status_t switch_api_dtel_postcard_watchlist_entry_delete(
      const switcht_device_t device,
      const std::vector<switcht_twl_key_value_pair_t> &twl_kvp) {
    switch_twl_match_info_t match_info;
    switch_parse_dtel_watchlist_match_info(device, twl_kvp, &match_info);
    switcht_status_t status = ::switch_api_dtel_watchlist_entry_delete(
        device, SWITCH_DTEL_TYPE_POSTCARD, &match_info);
    SWITCH_FREE(device, match_info.fields);
    return status;
  }

  switcht_status_t switch_api_dtel_postcard_watchlist_clear(
      const switcht_device_t device) {
    return ::switch_api_dtel_watchlist_clear(
        device, SWITCH_DTEL_TYPE_POSTCARD);
  }

  switcht_status_t switch_api_dtel_drop_report_enable(
      const switcht_device_t device) {
    return ::switch_api_dtel_drop_report_enable(device);
  }

  switcht_status_t switch_api_dtel_drop_report_disable(
      const switcht_device_t device) {
    return ::switch_api_dtel_drop_report_disable(device);
  }

  switcht_status_t switch_api_dtel_drop_watchlist_entry_create(
      const switcht_device_t device,
      const std::vector<switcht_twl_key_value_pair_t> &twl_kvp,
      const int32_t priority,
      const bool watch,
      const switcht_twl_mod_params_t &action_params) {
    switch_twl_match_info_t match_info;
    switch_parse_dtel_watchlist_match_info(device, twl_kvp, &match_info);

    switch_twl_action_params_t ap;
    memset(&ap, 0, sizeof(switch_twl_action_params_t));
    ap._mod.report_queue_tail_drops = action_params.report_queue_tail_drops;

    switcht_status_t status = ::switch_api_dtel_watchlist_entry_create(
        device, SWITCH_DTEL_TYPE_MOD, &match_info, priority, watch, &ap);
    SWITCH_FREE(device, match_info.fields);
    return status;
  }

  switcht_status_t switch_api_dtel_drop_watchlist_entry_update(
      const switcht_device_t device,
      const std::vector<switcht_twl_key_value_pair_t> &twl_kvp,
      const int32_t priority,
      const bool watch,
      const switcht_twl_mod_params_t &action_params) {
    switch_twl_match_info_t match_info;
    switch_parse_dtel_watchlist_match_info(device, twl_kvp, &match_info);

    switch_twl_action_params_t ap;
    memset(&ap, 0, sizeof(switch_twl_action_params_t));
    ap._mod.report_queue_tail_drops = action_params.report_queue_tail_drops;

    switcht_status_t status = ::switch_api_dtel_watchlist_entry_update(
        device, SWITCH_DTEL_TYPE_MOD, &match_info, priority, watch, &ap);
    SWITCH_FREE(device, match_info.fields);
    return status;
  }

  switcht_status_t switch_api_dtel_drop_watchlist_entry_delete(
      const switcht_device_t device,
      const std::vector<switcht_twl_key_value_pair_t> &twl_kvp) {
    switch_twl_match_info_t match_info;
    switch_parse_dtel_watchlist_match_info(device, twl_kvp, &match_info);
    switcht_status_t status = ::switch_api_dtel_watchlist_entry_delete(
        device, SWITCH_DTEL_TYPE_MOD, &match_info);
    SWITCH_FREE(device, match_info.fields);
    return status;
  }

  switcht_status_t switch_api_dtel_drop_watchlist_clear(
      const switcht_device_t device) {
    return ::switch_api_dtel_watchlist_clear(device,
                                                  SWITCH_DTEL_TYPE_MOD);
  }

  switcht_status_t switch_api_flowlet_switching_set(
      const switcht_device_t device, const int32_t enable_flowlet) {
    return ::switch_api_flowlet_switching_set(device, enable_flowlet);
  }

  switcht_status_t switch_api_set_switch_id(const switcht_device_t device,
                                            const int32_t switch_id) {
    return ::switch_api_switch_id_set(device, switch_id);
  }

  switcht_handle_t switch_api_sflow_session_create(
      const switcht_device_t device,
      const switcht_sflow_info_t &api_sflow_info) {
    ::switch_api_sflow_session_info_t lapi_sflow_info;
    switch_handle_t sflow_handle = 0;
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    memset(&lapi_sflow_info, 0, sizeof(::switch_api_sflow_session_info_t));
    lapi_sflow_info.timeout_usec = api_sflow_info.timeout_usec;
    lapi_sflow_info.sample_rate = api_sflow_info.sample_rate;
    lapi_sflow_info.extract_len = api_sflow_info.extract_len;
    lapi_sflow_info.collector_type =
        (switch_sflow_collector_type_e)api_sflow_info.collector_type;
    lapi_sflow_info.sample_mode =
        (switch_sflow_sample_mode_e)api_sflow_info.sample_mode;
    lapi_sflow_info.egress_port_handle =
        (switch_handle_t)api_sflow_info.egress_port_hdl;
    status = ::switch_api_sflow_session_create(
        device, &lapi_sflow_info, &sflow_handle);
    return sflow_handle;
  }

  switcht_status_t switch_api_sflow_session_delete(
      const switcht_device_t device,
      const switcht_handle_t sflow_hdl,
      const bool all_cleanup) {
    return ::switch_api_sflow_session_delete(
        device, (switch_handle_t)((uint32_t)sflow_hdl), all_cleanup);
  }

  switcht_handle_t switch_api_sflow_session_attach(
      const switcht_device_t device,
      const switcht_handle_t sflow_handle,
      const switcht_direction_t direction,
      const int32_t priority,
      const int32_t sample_rate,
      const std::vector<switcht_sflow_key_value_pair_t> &kvp) {
    std::vector<switcht_sflow_key_value_pair_t>::const_iterator f;
    uint32_t i = 0;
    switch_handle_t entry_hdl = -1;

    switch_sflow_match_key_value_pair_t *lkvp =
        (switch_sflow_match_key_value_pair_t *)SWITCH_CALLOC(
            device,
            sizeof(switch_sflow_match_key_value_pair_t) * kvp.size(),
            1);

    for (f = kvp.begin(); f != kvp.end(); f++) {
      bool key_valid = true;
      switch (f->field) {
        case SWITCH_SFLOW_MATCH_PORT:
          lkvp[i].value.port = (uint32_t)f->value.value_num;
          lkvp[i].mask.u.mask = (uint32_t)f->mask.value_num;
          break;
        case SWITCH_SFLOW_MATCH_VLAN:
          lkvp[i].value.vlan = (uint32_t)f->value.value_num;
          lkvp[i].mask.u.mask = (uint32_t)f->mask.value_num;
          break;
        case SWITCH_SFLOW_MATCH_SIP:
          lkvp[i].value.sip = (uint32_t)f->value.value_num;
          lkvp[i].mask.u.mask = (uint32_t)f->mask.value_num;
          break;
        case SWITCH_SFLOW_MATCH_DIP:
          lkvp[i].value.dip = (uint32_t)f->value.value_num;
          lkvp[i].mask.u.mask = (uint32_t)f->mask.value_num;
          break;
        default:
          key_valid = false;
          break;
      }
      lkvp[i].field = (switch_sflow_match_field_t)f->field;
      if (key_valid) {
        i++;
      }
    }
    ::switch_api_sflow_session_attach(device,
                                      sflow_handle,
                                      (switch_direction_t)direction,
                                      priority,
                                      sample_rate,
                                      i,
                                      lkvp,
                                      &entry_hdl);
    return entry_hdl;
  }

  switcht_status_t switch_api_sflow_session_detach(
      const switcht_device_t device,
      const switcht_handle_t sflow_handle,
      const switcht_handle_t entry_handle) {
    return ::switch_api_sflow_session_detach(
        device,
        (switch_handle_t)((uint32_t)sflow_handle),
        (switch_handle_t)((uint32_t)entry_handle));
  }

  switcht_status_t switch_api_mac_table_learning_timeout_set(
      const switcht_device_t device, const int32_t timeout) {
    return ::switch_api_mac_table_set_learning_timeout(device, timeout);
  }

  /* BFD APIs */
  switcht_handle_t switch_api_bfd_session_create(
      const switcht_device_t device,
      const switcht_bfd_session_info_t &api_bfd_info) {
    switch_api_bfd_session_info_t lapi_bfd_info;
    switch_handle_t entry_hdl;

    memset(&lapi_bfd_info, 0, sizeof(switch_api_bfd_session_info_t));
    lapi_bfd_info.my_disc = api_bfd_info.my_disc;
    lapi_bfd_info.your_disc = api_bfd_info.your_disc;
    lapi_bfd_info.detect_mult =
        api_bfd_info.detect_mult; /* used for rx timeout */
    lapi_bfd_info.desired_tx_interval =
        api_bfd_info.desired_tx_interval; /* usec - goes in pkt*/
    lapi_bfd_info.min_rx_interval =
        api_bfd_info.min_rx_interval; /* usec - goes in pkt*/
    lapi_bfd_info.tx_interval =
        api_bfd_info.tx_interval; /* usec - negotiated val */
    lapi_bfd_info.rx_interval =
        api_bfd_info.rx_interval; /* usec - negotiated val */
    // echo interval is not used - no offloaded in echo/demand-mode
    lapi_bfd_info.remote_desired_tx_interval =
        api_bfd_info.remote_desired_tx_interval; /* usec - goes in pkt*/
    lapi_bfd_info.remote_min_rx_interval =
        api_bfd_info.remote_min_rx_interval; /* usec - goes in pkt*/
    /* transport info */
    switch_parse_ip_address(api_bfd_info.sip, &lapi_bfd_info.sip);
    switch_parse_ip_address(api_bfd_info.dip, &lapi_bfd_info.dip);
    lapi_bfd_info.sport = api_bfd_info.sport;
    lapi_bfd_info.dport = api_bfd_info.dport;  // 1hop, multihop bfd session
    lapi_bfd_info.vrf_hdl = api_bfd_info.vrf_hdl;
    lapi_bfd_info.rmac_hdl = api_bfd_info.rmac_hdl;
    switch_string_to_mac(api_bfd_info.rmac, lapi_bfd_info.rmac.mac_addr);

    ::switch_api_bfd_session_create(device, &lapi_bfd_info, &entry_hdl);
    return (switcht_handle_t)entry_hdl;
  }

  switcht_status_t switch_api_bfd_session_delete(
      const switcht_device_t device, const switcht_handle_t bfd_hdl) {
    return ::switch_api_bfd_session_delete(device, bfd_hdl);
  }

  switcht_status_t switch_api_vlan_aging_interval_set(
      const switcht_device_t device,
      const switcht_handle_t vlan_handle,
      const int32_t value) {
    return ::switch_api_vlan_aging_interval_set(device, vlan_handle, value);
  }

  int32_t switch_api_vlan_aging_interval_get(
      const switcht_device_t device, const switcht_handle_t vlan_handle) {
    int32_t aging_interval = 0;
    ::switch_api_vlan_aging_interval_get(device, vlan_handle, &aging_interval);
    return aging_interval;
  }

  switcht_handle_t switch_api_hostif_group_create(
      const switcht_device_t device,
      const switcht_hostif_group_t &hostif_group) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    switch_handle_t hif_group_handle = SWITCH_API_INVALID_HANDLE;
    switch_hostif_group_t lhostif_group;
    lhostif_group.queue_handle = hostif_group.queue_handle;
    lhostif_group.policer_handle = hostif_group.policer_handle;
    status = ::switch_api_hostif_group_create(
        device, &lhostif_group, &hif_group_handle);
    return hif_group_handle;
  }

  switcht_status_t switch_api_hostif_group_delete(
      const switcht_device_t device, const switcht_handle_t hif_group_handle) {
    return ::switch_api_hostif_group_delete(device, hif_group_handle);
  }

  switcht_handle_t switch_api_hostif_reason_code_create(
      const switcht_device_t device,
      const switcht_uint64_t flags,
      const switcht_hostif_rcode_info_t &rcode_api_info) {
    switch_api_hostif_rcode_info_t lrcode_api_info;
    switch_handle_t rcode_handle = 0;
    lrcode_api_info.reason_code =
        (switch_hostif_reason_code_t)rcode_api_info.reason_code;
    lrcode_api_info.action = (switch_acl_action_t)rcode_api_info.action;
    lrcode_api_info.priority = rcode_api_info.priority;
    lrcode_api_info.hostif_group_id = rcode_api_info.hostif_group_id;
    ::switch_api_hostif_reason_code_create(
        device, flags, &lrcode_api_info, &rcode_handle);
    return rcode_handle;
  }

  switcht_status_t switch_api_hostif_reason_code_delete(
      const switcht_device_t device, const switcht_handle_t rcode_handle) {
    return ::switch_api_hostif_reason_code_delete(device, rcode_handle);
  }

  switcht_handle_t switch_api_hostif_create(const switcht_device_t device,
                                            const switcht_uint64_t flags,
                                            const switcht_hostif_t &hostif) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    switch_handle_t hif_handle = SWITCH_API_INVALID_HANDLE;
    switch_hostif_t lhostif;
    memset(&lhostif, 0, sizeof(lhostif));
    strncpy(
        lhostif.intf_name, hostif.intf_name.c_str(), SWITCH_HOSTIF_NAME_SIZE);
    lhostif.handle = (switch_handle_t)hostif.handle;
    if (flags & SWITCH_HOSTIF_ATTR_MAC_ADDRESS) {
      switch_string_to_mac(hostif.mac, lhostif.mac.mac_addr);
    }

    if (flags & SWITCH_HOSTIF_ATTR_IPV4_ADDRESS) {
      switch_parse_ip_address(hostif.v4addr, &lhostif.v4addr);
    }

    if (flags & SWITCH_HOSTIF_ATTR_IPV6_ADDRESS) {
      switch_parse_ip_address(hostif.v6addr, &lhostif.v6addr);
    }

    if (flags & SWITCH_HOSTIF_ATTR_VLAN_ACTION) {
      lhostif.vlan_action = (switch_hostif_vlan_action_t)hostif.vlan_action;
    }

    if (flags & SWITCH_HOSTIF_ATTR_OPER_STATUS) {
      lhostif.operstatus = hostif.operstatus;
    }

    if (flags & SWITCH_HOSTIF_ATTR_ADMIN_STATE) {
      lhostif.admin_state = hostif.admin_state;
    }

    status = ::switch_api_hostif_create(device, flags, &lhostif, &hif_handle);
    return hif_handle;
  }

  switcht_status_t switch_api_hostif_delete(const switcht_device_t device,
                                            const switcht_handle_t hif_handle) {
    return ::switch_api_hostif_delete(device, hif_handle);
  }

  switcht_handle_t switch_api_hostif_rx_filter_create(
      const switcht_device_t device,
      const switcht_uint64_t flags,
      const int32_t priority,
      const switcht_hostif_rx_filter_key_t &rx_key,
      const switcht_hostif_rx_filter_action_t &rx_action) {
    return 0;
  }

  switcht_status_t switch_api_hostif_rx_filter_delete(
      const switcht_device_t device, const switcht_handle_t filter_handle) {
    return ::switch_api_hostif_rx_filter_delete(device, filter_handle);
  }

  switcht_handle_t switch_api_hostif_tx_filter_create(
      const switcht_device_t device,
      const switcht_uint64_t flags,
      const int32_t priority,
      const switcht_hostif_tx_filter_key_t &tx_key,
      const switcht_hostif_tx_filter_action_t &tx_action) {
    return 0;
  }

  switcht_status_t switch_api_hostif_tx_filter_delete(
      const switcht_device_t device, const switcht_handle_t filter_handle) {
    return ::switch_api_hostif_tx_filter_delete(device, filter_handle);
  }

  switcht_handle_t switch_api_hostif_meter_create(
      const switcht_device_t device,
      const switcht_meter_info_t &api_meter_info) {
    switch_handle_t meter_handle = 0;
    ::switch_api_meter_t api_meter;
    memset(&api_meter, 0, sizeof(::switch_api_meter_t));
    api_meter.meter_mode = (switch_meter_mode_t)api_meter_info.meter_mode;
    api_meter.color_source =
        (switch_meter_color_source_t)api_meter_info.color_source;
    api_meter.meter_type = (switch_meter_type_t)api_meter_info.meter_type;
    api_meter.cbs = api_meter_info.cbs;
    api_meter.pbs = api_meter_info.pbs;
    api_meter.cir = api_meter_info.cir;
    api_meter.pir = api_meter_info.pir;
    api_meter.action[SWITCH_COLOR_GREEN] =
        (switch_acl_action_t)api_meter_info.green_action;
    api_meter.action[SWITCH_COLOR_YELLOW] =
        (switch_acl_action_t)api_meter_info.yellow_action;
    api_meter.action[SWITCH_COLOR_RED] =
        (switch_acl_action_t)api_meter_info.red_action;
    ::switch_api_hostif_meter_create(device, &api_meter, &meter_handle);
    return meter_handle;
  }

  switcht_status_t switch_api_hostif_meter_delete(
      const switcht_device_t device, const switcht_handle_t meter_handle) {
    return ::switch_api_hostif_meter_delete(device, meter_handle);
  }

  switcht_handle_t switch_api_hostif_nhop_get(
      const switcht_device_t device,
      const switcht_hostif_reason_code_t reason_code) {
    switch_handle_t nhop_handle;
    ::switch_api_hostif_nhop_get(
        device, (switch_hostif_reason_code_t)reason_code, &nhop_handle);
    return nhop_handle;
  }

  switcht_handle_t switch_api_hostif_handle_get(
      const switcht_device_t device,
      const std::string& intf_name) {
    switch_handle_t hostif_handle;
    switch_char_t lintf_name[SWITCH_HOSTIF_NAME_SIZE];

    memset(lintf_name, 0, sizeof(lintf_name));
    strncpy(lintf_name, intf_name.c_str(), SWITCH_HOSTIF_NAME_SIZE);

    ::switch_api_hostif_handle_get(device, lintf_name, &hostif_handle);
    return hostif_handle;
  }

  void switch_api_hostif_group_get(switcht_hostif_group_t &_hostif_group,
                                   const switcht_device_t device,
                                   const switcht_handle_t hostif_group_hdl) {
    switch_hostif_group_t hostif_group;
    ::switch_api_hostif_group_get(device, hostif_group_hdl, &hostif_group);
    _hostif_group.queue_handle = hostif_group.queue_handle;
    _hostif_group.policer_handle = hostif_group.policer_handle;
    return;
  }

  bool switch_api_hostif_oper_state_get(const switcht_device_t device,
                                   const switcht_handle_t hostif_handle) {
    bool oper_state;
    ::switch_api_hostif_oper_state_get(device, hostif_handle, &oper_state);
    return oper_state;
  }

  switcht_handle_t switch_api_meter_create(
      const switcht_device_t device,
      const switcht_meter_info_t &api_meter_info) {
    ::switch_api_meter_t api_meter;
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    switch_handle_t meter_handle = 0;
    memset(&api_meter, 0, sizeof(::switch_api_meter_t));
    api_meter.meter_mode = (switch_meter_mode_t)api_meter_info.meter_mode;
    api_meter.color_source =
        (switch_meter_color_source_t)api_meter_info.color_source;
    api_meter.meter_type = (switch_meter_type_t)api_meter_info.meter_type;
    api_meter.cbs = api_meter_info.cbs;
    api_meter.pbs = api_meter_info.pbs;
    api_meter.cir = api_meter_info.cir;
    api_meter.pir = api_meter_info.pir;
    api_meter.action[SWITCH_COLOR_GREEN] =
        (switch_acl_action_t)api_meter_info.green_action;
    api_meter.action[SWITCH_COLOR_YELLOW] =
        (switch_acl_action_t)api_meter_info.yellow_action;
    api_meter.action[SWITCH_COLOR_RED] =
        (switch_acl_action_t)api_meter_info.red_action;
    status = ::switch_api_meter_create(device, &api_meter, &meter_handle);
    return meter_handle;
  }

  switcht_status_t switch_api_meter_update(
      const switcht_device_t device,
      const switcht_handle_t meter_handle,
      const switcht_uint64_t meter_flags,
      const switcht_meter_info_t &api_meter_info) {
    ::switch_api_meter_t api_meter;
    memset(&api_meter, 0, sizeof(::switch_api_meter_t));
    api_meter.meter_mode = (switch_meter_mode_t)api_meter_info.meter_mode;
    api_meter.color_source =
        (switch_meter_color_source_t)api_meter_info.color_source;
    api_meter.meter_type = (switch_meter_type_t)api_meter_info.meter_type;
    api_meter.cbs = api_meter_info.cbs;
    api_meter.pbs = api_meter_info.pbs;
    api_meter.cir = api_meter_info.cir;
    api_meter.pir = api_meter_info.pir;
    api_meter.action[SWITCH_COLOR_GREEN] =
        (switch_acl_action_t)api_meter_info.green_action;
    api_meter.action[SWITCH_COLOR_YELLOW] =
        (switch_acl_action_t)api_meter_info.yellow_action;
    api_meter.action[SWITCH_COLOR_RED] =
        (switch_acl_action_t)api_meter_info.red_action;
    return ::switch_api_meter_update(
        device, meter_handle, meter_flags, &api_meter);
  }

  switcht_status_t switch_api_meter_delete(
      const switcht_device_t device, const switcht_handle_t meter_handle) {
    return ::switch_api_meter_delete(device, meter_handle);
  }

  void switch_api_meter_get(
      switcht_meter_info_t &_api_meter,
      const switcht_device_t device,
      const switcht_handle_t meter_handle) {
    switch_api_meter_t api_meter;
    ::switch_api_meter_get(device, meter_handle, &api_meter);
    _api_meter.meter_mode = api_meter.meter_mode;
    _api_meter.color_source = api_meter.color_source;
    _api_meter.meter_type = api_meter.meter_type;
    _api_meter.cbs = api_meter.cbs;
    _api_meter.pbs = api_meter.pbs;
    _api_meter.cir = api_meter.cir;
    _api_meter.pir = api_meter.pir;
    _api_meter.green_action = api_meter.action[SWITCH_COLOR_GREEN];
    _api_meter.yellow_action = api_meter.action[SWITCH_COLOR_YELLOW];
    _api_meter.red_action = api_meter.action[SWITCH_COLOR_RED];;
    return;
  }

  void switch_api_meter_stats_get(std::vector<switcht_counter_t> &_counters,
                                  const switcht_device_t device,
                                  const switcht_handle_t meter_handle,
                                  const std::vector<int16_t> &counter_ids) {
    std::vector<int16_t>::const_iterator it = counter_ids.begin();
    switcht_counter_t _counter;
    switch_meter_counter_t *counter_id_list = NULL;
    counter_id_list = (switch_meter_counter_t *)SWITCH_MALLOC(
        device, sizeof(switch_meter_counter_t), counter_ids.size());
    switch_counter_t *counters = (switch_counter_t *)SWITCH_MALLOC(
        device, sizeof(switch_counter_t), counter_ids.size());
    for (uint32_t i = 0; i < counter_ids.size(); i++, it++) {
      counter_id_list[i] = (switch_meter_counter_t)*it;
    }
    ::switch_api_meter_counters_get(
        device, meter_handle, counter_ids.size(), counter_id_list, counters);
    for (uint32_t i = 0; i < counter_ids.size(); i++) {
      _counter.num_packets = counters[i].num_packets;
      _counter.num_bytes = counters[i].num_bytes;
      _counters.push_back(_counter);
    }
    SWITCH_FREE(device, counter_id_list);
    SWITCH_FREE(device, counters);
    return;
  }

  switcht_handle_t switch_api_wred_create(
      const switcht_device_t device, const switcht_wred_info_t &api_wred_info) {
    switch_handle_t wred_handle = 0;
    ::switch_api_wred_info_t api_wred;
    memset(&api_wred, 0, sizeof(::switch_api_wred_info_t));
    api_wred.enable = api_wred_info.enable;
    api_wred.ecn_mark = api_wred_info.ecn_mark;
    api_wred.min_threshold = api_wred_info.min_threshold;
    api_wred.max_threshold = api_wred_info.max_threshold;
    api_wred.max_probability = api_wred_info.max_probability;
    api_wred.time_constant = api_wred_info.time_constant;
    ::switch_api_wred_create(device, &api_wred, &wred_handle);
    return wred_handle;
  }

  switcht_status_t switch_api_wred_update(
      const switcht_device_t device,
      const switcht_handle_t wred_handle,
      const switcht_wred_info_t &api_wred_info) {
    ::switch_api_wred_info_t api_wred;
    memset(&api_wred, 0, sizeof(::switch_api_wred_info_t));
    api_wred.enable = api_wred_info.enable;
    api_wred.ecn_mark = api_wred_info.ecn_mark;
    api_wred.min_threshold = api_wred_info.min_threshold;
    api_wred.min_threshold = api_wred_info.max_threshold;
    api_wred.max_probability = api_wred_info.max_probability;
    api_wred.time_constant = api_wred_info.time_constant;
    ::switch_api_wred_update(device, wred_handle, &api_wred);
  }

  switcht_status_t switch_api_wred_delete(const switcht_device_t device,
                                          const switcht_handle_t wred_handle) {
    return ::switch_api_wred_delete(device, wred_handle);
  }

  void switch_api_wred_get(switcht_wred_info_t &api_wred_info,
                           const switcht_device_t device,
                           const switcht_handle_t wred_handle) {
    switch_api_wred_info_t api_wred;
    memset(&api_wred, 0, sizeof(::switch_api_wred_info_t));

    ::switch_api_wred_get(device, wred_handle, &api_wred);

    api_wred_info.enable = api_wred.enable;
    api_wred_info.ecn_mark = api_wred.ecn_mark;
    api_wred_info.min_threshold = api_wred.min_threshold;
    api_wred_info.max_threshold = api_wred.max_threshold;
    api_wred_info.max_probability = api_wred.max_probability;
    api_wred_info.time_constant = api_wred.time_constant;

    return;
  }

  void switch_api_wred_profile_get(switcht_wred_profile_info_t &_wred_profile,
                                   const switcht_device_t device,
                                   const switcht_handle_t profile_handle) {
    switch_api_wred_profile_info_t profile_info;
    ::switch_api_wred_profile_get(device, profile_handle, &profile_info);
    _wred_profile.min_threshold_yellow = profile_info.min_threshold[SWITCH_COLOR_YELLOW];
    _wred_profile.max_threshold_yellow = profile_info.max_threshold[SWITCH_COLOR_YELLOW];
    _wred_profile.enable_yellow = profile_info.enable[SWITCH_COLOR_YELLOW];
    _wred_profile.probability_yellow = profile_info.probability[SWITCH_COLOR_YELLOW];
    _wred_profile.ecn_mark_yellow = profile_info.ecn_mark[SWITCH_COLOR_YELLOW];
    _wred_profile.min_threshold_green = profile_info.min_threshold[SWITCH_COLOR_GREEN];
    _wred_profile.max_threshold_green = profile_info.max_threshold[SWITCH_COLOR_GREEN];
    _wred_profile.enable_green = profile_info.enable[SWITCH_COLOR_GREEN];
    _wred_profile.probability_green = profile_info.probability[SWITCH_COLOR_GREEN];
    _wred_profile.ecn_mark_green = profile_info.ecn_mark[SWITCH_COLOR_GREEN];
    _wred_profile.min_threshold_red = profile_info.min_threshold[SWITCH_COLOR_RED];
    _wred_profile.max_threshold_red = profile_info.max_threshold[SWITCH_COLOR_RED];
    _wred_profile.enable_red = profile_info.enable[SWITCH_COLOR_RED];
    _wred_profile.probability_red = profile_info.probability[SWITCH_COLOR_RED];
    _wred_profile.ecn_mark_red = profile_info.ecn_mark[SWITCH_COLOR_RED];
    return;
  }

  switcht_handle_t switch_api_queue_wred_profile_get(
      const switcht_device_t device,
      const switcht_handle_t queue_handle) {
    switch_handle_t profile_handle = 0;
    ::switch_api_queue_wred_profile_get(device, queue_handle, &profile_handle);
    return profile_handle;
  }

  switcht_status_t switch_api_wred_attach(
      const switcht_device_t device,
      const switcht_handle_t queue_handle,
      const switcht_meter_counter_t packet_color,
      const switcht_handle_t wred_handle) {
    return ::switch_api_wred_attach(device,
                                    queue_handle,
                                    (switch_meter_counter_t)packet_color,
                                    wred_handle);
  }

  switcht_status_t switch_api_wred_detach(
      const switcht_device_t device,
      const switcht_handle_t queue_handle,
      const switcht_meter_counter_t packet_color) {
    return ::switch_api_wred_detach(
        device, queue_handle, (switch_meter_counter_t)packet_color);
  }

  void switch_api_wred_stats_get(std::vector<switcht_counter_t> &counters,
                                 const switcht_device_t device,
                                 const switcht_handle_t queue_handle,
                                 const std::vector<int16_t> &counter_ids) {
    std::vector<int16_t>::const_iterator it = counter_ids.begin();
    switcht_counter_t counter_;
    switch_counter_t *counters_ = (switch_counter_t *)SWITCH_MALLOC(
        device, sizeof(switch_counter_t), counter_ids.size());
    switch_wred_counter_t *counter_id_list =
        (switch_wred_counter_t *)SWITCH_MALLOC(
            device, sizeof(switch_wred_counter_t), counter_ids.size());
    for (uint32_t i = 0; i < counter_ids.size(); ++i, ++it) {
      counter_id_list[i] = (switch_wred_counter_t)*it;
    }
    ::switch_api_wred_stats_get(
        device, queue_handle, counter_ids.size(), counter_id_list, counters_);

    for (uint32_t i = 0; i < counter_ids.size(); ++i) {
      counter_.num_packets = counters_[i].num_packets;
      counter_.num_bytes = counters_[i].num_bytes;
      counters.push_back(counter_);
    }

    SWITCH_FREE(device, counter_id_list);
    SWITCH_FREE(device, counters_);
    return;
  }

  switcht_status_t switch_api_wred_stats_clear(
      const switcht_device_t device,
      const switcht_handle_t queue_handle,
      const std::vector<int16_t> &counter_ids) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    std::vector<int16_t>::const_iterator it = counter_ids.begin();
    switch_wred_counter_t *counter_id_list =
        (switch_wred_counter_t *)SWITCH_MALLOC(
            device, sizeof(switch_wred_counter_t), counter_ids.size());
    for (uint32_t i = 0; i < counter_ids.size(); ++i, ++it) {
      counter_id_list[i] = (switch_wred_counter_t)*it;
    }
    status = ::switch_api_wred_stats_clear(
        device, queue_handle, counter_ids.size(), counter_id_list);
    SWITCH_FREE(device, counter_id_list);
    return status;
  }

  switcht_status_t switch_api_port_cos_mapping(
      const switcht_device_t device,
      const switcht_handle_t port_handle,
      const switcht_handle_t ppg_handle,
      const int8_t cos_bmp) {
    return ::switch_api_port_cos_mapping(
        device, port_handle, ppg_handle, cos_bmp);
  }

  switcht_status_t switch_api_ppg_lossless_enable(
      const switcht_device_t device,
      const switcht_handle_t ppg_handle,
      const bool enable) {
    return ::switch_api_ppg_lossless_enable(device, ppg_handle, enable);
  }

  void switch_api_ppg_get(std::vector<switcht_handle_t> &ppg_handles,
                          const switcht_device_t device,
                          const switcht_handle_t port_handle) {
    switch_handle_t *ppg_handles_tmp = NULL;
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    uint8_t num_ppg = 0;

    ppg_handles_tmp = (switch_handle_t *)SWITCH_MALLOC(
        device, sizeof(switch_handle_t), SWITCH_MAX_QUEUE);
    // status =
    //    ::switch_api_ppg_get(device, port_handle, &num_ppg, ppg_handles_tmp);
    for (uint32_t i = 0; i < num_ppg; i++) {
      ppg_handles.push_back(ppg_handles_tmp[i]);
    }

    SWITCH_FREE(device, ppg_handles_tmp);
    return;
  }

  switcht_status_t switch_api_ppg_guaranteed_limit_set(
      const switcht_device_t device,
      const switcht_handle_t ppg_handle,
      const int32_t num_bytes) {
    return ::switch_api_ppg_guaranteed_limit_set(device, ppg_handle, num_bytes);
  }

  switcht_status_t switch_api_ppg_skid_limit_set(
      const switcht_device_t device,
      const switcht_handle_t ppg_handle,
      const int32_t num_bytes) {
    return ::switch_api_ppg_skid_limit_set(device, ppg_handle, num_bytes);
  }

  switcht_status_t switch_api_ppg_skid_hysteresis_set(
      const switcht_device_t device,
      const switcht_handle_t ppg_handle,
      const int32_t num_bytes) {
    return ::switch_api_ppg_skid_hysteresis_set(device, ppg_handle, num_bytes);
  }

  switcht_handle_t switch_api_buffer_pool_create(
      const switcht_device_t device, const switcht_buffer_pool_t &buffer_pool) {
    ::switch_api_buffer_pool_t api_buffer_pool;
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    switch_handle_t pool_handle = 0;
    memset(&api_buffer_pool, 0, sizeof(switch_api_buffer_pool_t));
    api_buffer_pool.direction = (switch_direction_t)buffer_pool.dir;
    api_buffer_pool.pool_size = buffer_pool.pool_size;
    api_buffer_pool.threshold_mode =
        (switch_buffer_threshold_mode_t)buffer_pool.threshold;
    api_buffer_pool.xoff_size = buffer_pool.xoff_size;
    api_buffer_pool.shared_size = buffer_pool.shared_size;
    status =
        ::switch_api_buffer_pool_create(device, api_buffer_pool, &pool_handle);
    return pool_handle;
  }

  switcht_status_t switch_api_buffer_pool_delete(
      const switcht_device_t device,
      const switcht_handle_t buffer_pool_handle) {
    return ::switch_api_buffer_pool_delete(device, buffer_pool_handle);
  }

  switcht_handle_t switch_api_buffer_profile_create(
      const switcht_device_t device,
      const switcht_buffer_profile_t &api_buffer_info) {
    switch_handle_t buffer_profile_handle = 0;
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    ::switch_api_buffer_profile_t buffer_profile;
    memset(&buffer_profile, 0x0, sizeof(buffer_profile));
    buffer_profile.threshold_mode =
        (switch_buffer_threshold_mode_t)api_buffer_info.threshold_mode;
    buffer_profile.threshold = api_buffer_info.threshold;
    buffer_profile.pool_handle = api_buffer_info.pool_handle;
    buffer_profile.buffer_size = api_buffer_info.buffer_size;
    buffer_profile.xoff_threshold = api_buffer_info.xoff_threshold;
    buffer_profile.xon_threshold = api_buffer_info.xon_threshold;
    status = ::switch_api_buffer_profile_create(
        device, &buffer_profile, &buffer_profile_handle);
    return buffer_profile_handle;
  }

  switcht_status_t switch_api_buffer_profile_delete(
      const switcht_device_t device,
      const switcht_handle_t buffer_profile_handle) {
    return ::switch_api_buffer_profile_delete(device, buffer_profile_handle);
  }

  int16_t switch_api_buffer_pool_threshold_mode_get(
      const switcht_device_t device,
      const switcht_handle_t pool_handle) {
    switch_buffer_threshold_mode_t threshold_mode;
    ::switch_api_buffer_pool_threshold_mode_get(
            device, pool_handle, &threshold_mode);
    return (int16_t)threshold_mode;
  }

  int32_t switch_api_buffer_pool_size_get(
      const switcht_device_t device,
      const switcht_handle_t pool_handle) {
    switch_uint32_t size;
    ::switch_api_buffer_pool_size_get(device, pool_handle, &size);
    return (int32_t)size;
  }

  switcht_direction_t switch_api_buffer_pool_type_get(
      const switcht_device_t device,
      const switcht_handle_t pool_handle) {
    switch_direction_t dir;
    ::switch_api_buffer_pool_type_get(device, pool_handle, &dir);
    return dir;
  }

  switcht_status_t switch_api_ppg_buffer_profile_set(
      const switcht_device_t device,
      const switcht_handle_t ppg_handle,
      const switcht_handle_t buffer_profile_handle) {
    return ::switch_api_priority_group_buffer_profile_set(
        device, ppg_handle, buffer_profile_handle);
  }

  switcht_handle_t switch_api_ppg_buffer_profile_get(
      const switcht_device_t device,
      const switcht_handle_t ppg_handle) {
    switch_handle_t buffer_profile_handle;
    ::switch_api_priority_group_buffer_profile_get(
        device, ppg_handle, &buffer_profile_handle);
    return buffer_profile_handle;
  }

  switcht_status_t switch_api_queue_buffer_profile_set(
      const switcht_device_t device,
      const switcht_handle_t queue_handle,
      const switcht_handle_t buffer_profile_handle) {
    return ::switch_api_queue_buffer_profile_set(
        device, queue_handle, buffer_profile_handle);
  }

  switcht_handle_t switch_api_queue_buffer_profile_get(
      const switcht_device_t device,
      const switcht_handle_t queue_handle) {
    switch_handle_t buffer_profile_handle;
    ::switch_api_queue_buffer_profile_get(
        device, queue_handle, &buffer_profile_handle);
    return buffer_profile_handle;
  }

  switcht_handle_t switch_api_priority_group_port_get(
      const switcht_device_t device,
      const switcht_handle_t pg_handle) {
    switch_handle_t port_handle;
    ::switch_api_priority_group_port_get(
        device, pg_handle, &port_handle);
    return port_handle;
  }

  int64_t switch_api_queue_drop_get(
          const switcht_device_t device,
          const switcht_handle_t queue_handle) {
    uint64_t num_packets = 0;
    ::switch_api_queue_drop_get(device, queue_handle, &num_packets);
    return (int64_t) num_packets;
  }

  int16_t switch_api_max_ingress_pool_get(const switcht_device_t device) {
    switch_uint8_t pool_size;
    ::switch_api_max_ingress_pool_get(device, &pool_size);
    return (int16_t)pool_size;
  }

  int16_t switch_api_max_egress_pool_get(const switcht_device_t device) {
    switch_uint8_t pool_size;
    ::switch_api_max_egress_pool_get(device, &pool_size);
    return (int16_t)pool_size;
  }

  int64_t switch_api_total_buffer_size_get(const switcht_device_t device) {
    switch_uint64_t pool_size;
    ::switch_api_total_buffer_size_get(device, &pool_size);
    return (int64_t)pool_size;
  }

  int32_t switch_api_priority_group_index_get(
      const switcht_device_t device,
      const switcht_handle_t pg_handle) {
    switch_uint32_t index;
    ::switch_api_priority_group_index_get(
        device, pg_handle, &index);
    return (int32_t)index;
  }

  void switch_api_buffer_profile_info_get(
      switcht_buffer_profile_t &_profile_info,
      const switcht_device_t device,
      const switcht_handle_t buffer_profile_handle) {
    switch_api_buffer_profile_t profile_info;
    ::switch_api_buffer_profile_info_get(
            device, buffer_profile_handle, &profile_info);
    _profile_info.threshold_mode = profile_info.threshold_mode;
    _profile_info.threshold = profile_info.threshold;
    _profile_info.pool_handle = profile_info.pool_handle;
    _profile_info.buffer_size = profile_info.buffer_size;
    _profile_info.xoff_threshold = profile_info.xoff_threshold;
    _profile_info.xon_threshold = profile_info.xon_threshold;
    return;
  }

  int32_t switch_api_buffer_pool_usage_get(
          const switcht_device_t device,
          const switcht_handle_t profile_handle) {
    uint32_t co_bytes = 0;
    uint32_t wm_bytes = 0;
    ::switch_api_buffer_pool_usage_get(device, profile_handle, &co_bytes, &wm_bytes);
    return (int32_t) co_bytes;
  }

  int32_t switch_api_buffer_pool_xoff_size_get(
      const switcht_device_t device,
      const switcht_handle_t pool_handle) {
    switch_uint32_t xoff_size;
    ::switch_api_buffer_pool_xoff_size_get(device, pool_handle, &xoff_size);
    return (int32_t)xoff_size;
  }

  switcht_status_t switch_api_buffer_skid_limit_set(
      const switcht_device_t device, const int32_t num_bytes) {
    return ::switch_api_buffer_skid_limit_set(device, num_bytes);
  }

  switcht_status_t switch_api_buffer_skid_hysteresis_set(
      const switcht_device_t device, const int32_t num_bytes) {
    return ::switch_api_buffer_skid_hysteresis_set(device, num_bytes);
  }

  switcht_status_t switch_api_buffer_pool_pfc_limit(
      const switcht_device_t device,
      const switcht_handle_t pool_handle,
      const int8_t icos,
      const int32_t num_bytes) {
    return ::switch_api_buffer_pool_pfc_limit(
        device, pool_handle, icos, num_bytes);
  }

  switcht_status_t switch_api_buffer_pool_color_drop_enable(
      const switcht_device_t device,
      const switcht_handle_t pool_handle,
      const bool enable) {
    return ::switch_api_buffer_pool_color_drop_enable(
        device, pool_handle, enable);
  }

  switcht_status_t switch_api_buffer_pool_color_limit_set(
      const switcht_device_t device,
      const switcht_handle_t pool_handle,
      const switcht_color_t color,
      const int32_t num_bytes) {
    return ::switch_api_buffer_pool_color_limit_set(
        device, pool_handle, (switch_color_t)color, num_bytes);
  }

  switcht_status_t switch_api_buffer_pool_color_hysteresis_set(
      const switcht_device_t device,
      const switcht_color_t color,
      const int32_t num_bytes) {
    return ::switch_api_buffer_pool_color_hysteresis_set(
        device, (switch_color_t)color, num_bytes);
  }

  switcht_handle_t switch_api_qos_map_ingress_create(
      const switcht_device_t device,
      const switcht_qos_map_type_t qos_map_type,
      const std::vector<switcht_qos_map_t> &qos_map) {
    switch_handle_t qos_map_handle = 0;
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    std::vector<switcht_qos_map_t>::const_iterator it = qos_map.begin();

    switch_qos_map_t *qos_map_list = (switch_qos_map_t *)SWITCH_MALLOC(
        device, sizeof(switch_qos_map_t), qos_map.size());
    memset(qos_map_list, 0x0, sizeof(switch_qos_map_t) * qos_map.size());

    for (uint32_t i = 0; i < qos_map.size(); i++, it++) {
      const switcht_qos_map_t qos_map_tmp = *it;
      switch (qos_map_type) {
        case SWITCH_QOS_MAP_INGRESS_DSCP_TO_TC:
          qos_map_list[i].dscp = qos_map_tmp.dscp;
          qos_map_list[i].tc = qos_map_tmp.tc;
          break;
        case SWITCH_QOS_MAP_INGRESS_TOS_TO_TC:
          qos_map_list[i].tos = qos_map_tmp.tos;
          qos_map_list[i].tc = qos_map_tmp.tc;
          break;
        case SWITCH_QOS_MAP_INGRESS_PCP_TO_TC:
          qos_map_list[i].pcp = qos_map_tmp.pcp;
          qos_map_list[i].tc = qos_map_tmp.tc;
          break;
        case SWITCH_QOS_MAP_INGRESS_DSCP_TO_COLOR:
          qos_map_list[i].dscp = qos_map_tmp.dscp;
          qos_map_list[i].color = (switch_color_t)qos_map_tmp.color;
          break;
        case SWITCH_QOS_MAP_INGRESS_TOS_TO_COLOR:
          qos_map_list[i].tos = qos_map_tmp.tos;
          qos_map_list[i].color = (switch_color_t)qos_map_tmp.color;
          break;
        case SWITCH_QOS_MAP_INGRESS_PCP_TO_COLOR:
          qos_map_list[i].pcp = qos_map_tmp.pcp;
          qos_map_list[i].color = (switch_color_t)qos_map_tmp.color;
          break;
        case SWITCH_QOS_MAP_INGRESS_DSCP_TO_TC_AND_COLOR:
          qos_map_list[i].dscp = qos_map_tmp.dscp;
          qos_map_list[i].tc = qos_map_tmp.tc;
          qos_map_list[i].color = (switch_color_t)qos_map_tmp.color;
          break;
        case SWITCH_QOS_MAP_INGRESS_TOS_TO_TC_AND_COLOR:
          qos_map_list[i].tos = qos_map_tmp.tos;
          qos_map_list[i].tc = qos_map_tmp.tc;
          qos_map_list[i].color = (switch_color_t)qos_map_tmp.color;
          break;
        case SWITCH_QOS_MAP_INGRESS_PCP_TO_TC_AND_COLOR:
          qos_map_list[i].pcp = qos_map_tmp.pcp;
          qos_map_list[i].tc = qos_map_tmp.tc;
          qos_map_list[i].color = (switch_color_t)qos_map_tmp.color;
          break;
        case SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS:
          qos_map_list[i].tc = qos_map_tmp.tc;
          qos_map_list[i].icos = qos_map_tmp.icos;
          break;
        case SWITCH_QOS_MAP_INGRESS_TC_TO_QUEUE:
          qos_map_list[i].tc = qos_map_tmp.tc;
          qos_map_list[i].qid = qos_map_tmp.qid;
          break;
        case SWITCH_QOS_MAP_INGRESS_TC_TO_ICOS_AND_QUEUE:
          qos_map_list[i].tc = qos_map_tmp.tc;
          qos_map_list[i].icos = qos_map_tmp.icos;
          qos_map_list[i].qid = qos_map_tmp.qid;
          break;
        case SWITCH_QOS_MAP_INGRESS_DSCP_TO_QID_AND_TC_AND_COLOR:
          qos_map_list[i].dscp = qos_map_tmp.dscp;
          qos_map_list[i].tc = qos_map_tmp.tc;
          qos_map_list[i].icos = qos_map_tmp.icos;
          qos_map_list[i].qid = qos_map_tmp.qid;
          qos_map_list[i].color = (switch_color_t)qos_map_tmp.color;
          break;
        case SWITCH_QOS_MAP_INGRESS_PCP_TO_QID_AND_TC_AND_COLOR:
          qos_map_list[i].pcp = qos_map_tmp.pcp;
          qos_map_list[i].tc = qos_map_tmp.tc;
          qos_map_list[i].icos = qos_map_tmp.icos;
          qos_map_list[i].qid = qos_map_tmp.qid;
          qos_map_list[i].color = (switch_color_t)qos_map_tmp.color;
          break;
        case SWITCH_QOS_MAP_INGRESS_TOS_TO_QID_AND_TC_AND_COLOR:
          qos_map_list[i].tos = qos_map_tmp.tos;
          qos_map_list[i].tc = qos_map_tmp.tc;
          qos_map_list[i].icos = qos_map_tmp.icos;
          qos_map_list[i].qid = qos_map_tmp.qid;
          qos_map_list[i].color = (switch_color_t)qos_map_tmp.color;
          break;
      }
    }
    status = ::switch_api_qos_map_ingress_create(
        device,
        (switch_qos_map_ingress_t)qos_map_type,
        qos_map.size(),
        qos_map_list,
        &qos_map_handle);
    SWITCH_FREE(device, qos_map_list);
    return qos_map_handle;
  }

  switcht_status_t switch_api_qos_map_ingress_delete(
      const switcht_device_t device, const switcht_handle_t qos_map_handle) {
    return ::switch_api_qos_map_ingress_delete(device, qos_map_handle);
  }

  switcht_handle_t switch_api_qos_map_egress_create(
      const switcht_device_t device,
      const switcht_qos_map_type_t qos_map_type,
      const std::vector<switcht_qos_map_t> &qos_map) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    switch_handle_t qos_map_handle = 0;
    std::vector<switcht_qos_map_t>::const_iterator it = qos_map.begin();

    switch_qos_map_t *qos_map_list = (switch_qos_map_t *)SWITCH_MALLOC(
        device, sizeof(switch_qos_map_t), qos_map.size());
    memset(qos_map_list, 0x0, sizeof(switch_qos_map_t) * qos_map.size());

    for (uint32_t i = 0; i < qos_map.size(); i++, it++) {
      const switcht_qos_map_t qos_map_tmp = *it;
      switch (qos_map_type) {
        case SWITCH_QOS_MAP_EGRESS_TC_TO_DSCP:
          qos_map_list[i].dscp = qos_map_tmp.dscp;
          qos_map_list[i].tc = qos_map_tmp.tc;
          break;
        case SWITCH_QOS_MAP_EGRESS_TC_TO_TOS:
          qos_map_list[i].tos = qos_map_tmp.tos;
          qos_map_list[i].tc = qos_map_tmp.tc;
          break;
        case SWITCH_QOS_MAP_EGRESS_TC_TO_PCP:
          qos_map_list[i].pcp = qos_map_tmp.pcp;
          qos_map_list[i].tc = qos_map_tmp.tc;
          break;
        case SWITCH_QOS_MAP_EGRESS_COLOR_TO_DSCP:
          qos_map_list[i].dscp = qos_map_tmp.dscp;
          qos_map_list[i].color = (switch_color_t)qos_map_tmp.color;
          break;
        case SWITCH_QOS_MAP_EGRESS_COLOR_TO_TOS:
          qos_map_list[i].tos = qos_map_tmp.tos;
          qos_map_list[i].color = (switch_color_t)qos_map_tmp.color;
          break;
        case SWITCH_QOS_MAP_EGRESS_COLOR_TO_PCP:
          qos_map_list[i].pcp = qos_map_tmp.pcp;
          qos_map_list[i].color = (switch_color_t)qos_map_tmp.color;
          break;
        case SWITCH_QOS_MAP_EGRESS_TC_AND_COLOR_TO_DSCP:
          qos_map_list[i].dscp = qos_map_tmp.dscp;
          qos_map_list[i].tc = qos_map_tmp.tc;
          qos_map_list[i].color = (switch_color_t)qos_map_tmp.color;
          break;
        case SWITCH_QOS_MAP_EGRESS_TC_AND_COLOR_TO_TOS:
          qos_map_list[i].tos = qos_map_tmp.tos;
          qos_map_list[i].tc = qos_map_tmp.tc;
          qos_map_list[i].color = (switch_color_t)qos_map_tmp.color;
          break;
        case SWITCH_QOS_MAP_EGRESS_TC_AND_COLOR_TO_PCP:
          qos_map_list[i].pcp = qos_map_tmp.pcp;
          qos_map_list[i].tc = qos_map_tmp.tc;
          qos_map_list[i].color = (switch_color_t)qos_map_tmp.color;
          break;
      }
    }
    status = ::switch_api_qos_map_egress_create(
        device,
        (switch_qos_map_egress_t)qos_map_type,
        qos_map.size(),
        qos_map_list,
        &qos_map_handle);
    SWITCH_FREE(device, qos_map_list);
    return qos_map_handle;
  }

  switcht_status_t switch_api_qos_map_egress_delete(
      const switcht_device_t device, const switcht_handle_t qos_map_handle) {
    return ::switch_api_qos_map_egress_delete(device, qos_map_handle);
  }

  int16_t switch_api_qos_map_dir_get(
      const switcht_device_t device, const switcht_handle_t qos_map_handle) {
    switch_direction_t dir;
    switch_qos_map_ingress_t ig_map_type;
    switch_qos_map_egress_t eg_map_type;
    ::switch_api_qos_map_type_get(
          device, qos_map_handle, &dir, &ig_map_type, &eg_map_type);
    return (int16_t)dir;
  }

  int16_t switch_api_qos_map_ig_map_type_get(
      const switcht_device_t device, const switcht_handle_t qos_map_handle) {
    switch_direction_t dir;
    switch_qos_map_ingress_t ig_map_type;
    switch_qos_map_egress_t eg_map_type;
    ::switch_api_qos_map_type_get(
          device, qos_map_handle, &dir, &ig_map_type, &eg_map_type);
    return (int16_t)ig_map_type;
  }

  int16_t switch_api_qos_map_eg_map_type_get(
      const switcht_device_t device, const switcht_handle_t qos_map_handle) {
    switch_direction_t dir;
    switch_qos_map_ingress_t ig_map_type;
    switch_qos_map_egress_t eg_map_type;
    ::switch_api_qos_map_type_get(
          device, qos_map_handle, &dir, &ig_map_type, &eg_map_type);
    return (int16_t)eg_map_type;
  }

  void switch_api_qos_map_list_get(
      std::vector<switcht_qos_map_t> &_qos_map_list,
      const switcht_device_t device,
      const switcht_handle_t qos_map_handle) {
    switch_qos_map_t *qos_map_list = NULL;
    switcht_qos_map_t _qos_map;
    switch_uint32_t num_entries = 0;
    ::switch_api_qos_map_list_get(
          device, qos_map_handle, &qos_map_list, &num_entries);
    for (uint32_t i = 0; i < num_entries; i++) {
      _qos_map.dscp = qos_map_list[i].dscp;
      _qos_map.pcp = qos_map_list[i].pcp;
      _qos_map.tc = qos_map_list[i].tc;
      _qos_map.color = qos_map_list[i].color;
      _qos_map.icos = qos_map_list[i].icos;
      _qos_map.qid = qos_map_list[i].qid;
      _qos_map_list.push_back(_qos_map);
    }

    SWITCH_FREE(device, qos_map_list);
    return;
  }

  switcht_handle_t switch_api_scheduler_create(
      const switcht_device_t device,
      const switcht_scheduler_info_t &api_scheduler_info) {
    return 0;
  }

  switcht_status_t switch_api_scheduler_delete(
      const switcht_device_t device, const switcht_handle_t scheduler_handle) {
    return 0;
  }

  switcht_handle_t switch_api_scheduler_group_child_handle_get(
      const switcht_device_t device,
      const switcht_handle_t scheduler_group) {
    return 0;
  }

  int32_t switch_api_scheduler_group_child_count_get(
      const switcht_device_t device,
      const switcht_handle_t scheduler_group) {
    return 0;
  }

  switcht_handle_t switch_api_scheduler_group_profile_get(
      const switcht_device_t device,
      const switcht_handle_t scheduler_group_handle) {
    return 0;
  }

  void switch_api_scheduler_config_get(
      switcht_scheduler_info_t &_sched_info,
      const switcht_device_t device,
      const switcht_handle_t scheduler_handle) {
    /*
    switch_scheduler_api_info_t sched_info;
    ::switch_api_scheduler_config_get(device, scheduler_handle, &sched_info);
    _sched_info.scheduler_type = sched_info.scheduler_type;
    _sched_info.shaper_type = sched_info.shaper_type;
    _sched_info.priority = sched_info.priority;
    _sched_info.rem_bw_priority = sched_info.rem_bw_priority;
    _sched_info.weight = sched_info.weight;
    _sched_info.min_burst_size = sched_info.min_burst_size;
    _sched_info.min_rate = sched_info.min_rate;
    _sched_info.max_burst_size = sched_info.max_burst_size;
    _sched_info.max_rate = sched_info.max_rate;
    */
    return;
  }

  void switch_api_scheduler_group_config_get(
      switcht_scheduler_group_info_t &_sched_group_info,
      const switcht_device_t device,
      const switcht_handle_t scheduler_handle) {
    /*
    switch_scheduler_group_api_info_t sched_group_info;
    ::switch_api_scheduler_group_config_get(
          device, scheduler_group_handle, &sched_group_info);
    _sched_group_info.group_type = sched_group_info.group_type;
    _sched_group_info.scheduler_handle = sched_group_info.scheduler_handle;
    _sched_group_info.port_handle = sched_group_info.port_handle;
    _sched_group_info.queue_handle = sched_group_info.queue_handle;
    */
    return;
  }

  void switch_api_queues_get(std::vector<switcht_handle_t> &queue_handles,
                             const switcht_device_t device,
                             const switcht_handle_t port_handle) {
    switch_handle_t *queue_handles_tmp = NULL;
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    uint32_t num_queues = 0;

    queue_handles_tmp = (switch_handle_t *)SWITCH_MALLOC(
        device, sizeof(switch_handle_t), SWITCH_MAX_QUEUE);
    status = ::switch_api_queues_get(
        device, port_handle, &num_queues, queue_handles_tmp);
    for (uint32_t i = 0; i < num_queues; i++) {
      queue_handles.push_back(queue_handles_tmp[i]);
    }

    SWITCH_FREE(device, queue_handles_tmp);
    return;
  }

  switcht_status_t switch_api_queue_color_drop_enable(
      const switcht_device_t device,
      const switcht_handle_t queue_handle,
      const bool enable) {
    return ::switch_api_queue_color_drop_enable(device, queue_handle, enable);
  }

  switcht_status_t switch_api_queue_color_limit_set(
      const switcht_device_t device,
      const switcht_handle_t queue_handle,
      const switcht_color_t color,
      const int32_t limit) {
    return ::switch_api_queue_color_limit_set(
        device, queue_handle, (switch_color_t)color, limit);
  }

  switcht_status_t switch_api_queue_color_hysteresis_set(
      const switcht_device_t device,
      const switcht_handle_t queue_handle,
      const switcht_color_t color,
      const int32_t limit) {
    return ::switch_api_queue_color_hysteresis_set(
        device, queue_handle, (switch_color_t)color, limit);
  }

  switcht_status_t switch_api_queue_pfc_cos_mapping(
      const switcht_device_t device,
      const switcht_handle_t queue_handle,
      const int8_t cos) {
    return ::switch_api_queue_pfc_cos_mapping(device, queue_handle, cos);
  }

  int32_t switch_api_max_queues_get(const switcht_device_t device) {
    switch_uint32_t max_queues;
    ::switch_api_max_queues_get(device, &max_queues);
    return (int32_t)max_queues;
  }

  int32_t switch_api_max_cpu_queues_get(const switcht_device_t device) {
    switch_uint32_t max_queues;
    ::switch_api_max_cpu_queues_get(device, &max_queues);
    return (int32_t)max_queues;
  }

  int32_t switch_api_max_traffic_class_get(const switcht_device_t device) {
    switch_uint32_t tr_classes;
    ::switch_api_max_traffic_class_get(device, &tr_classes);
    return (int32_t)tr_classes;
  }

  int16_t switch_api_queue_index_get(const switcht_device_t device,
                                     const switcht_handle_t handle) {
    switch_uint8_t index;
    ::switch_api_queue_index_get(device, handle, &index);
    return (int16_t)index;
  }

  switcht_handle_t switch_api_queue_port_get(const switcht_device_t device,
                                     const switcht_handle_t handle) {
    switch_handle_t port_handle;
    ::switch_api_queue_port_get(device, handle, &port_handle);
    return port_handle;
  }

  switcht_status_t switch_api_dtel_tail_drop_deflection_queue_set(
      const switcht_device_t device,
      const switcht_pipe_t pipe,
      const switcht_handle_t queue_handle) {
    return ::switch_api_dtel_tail_drop_deflection_queue_set(
        device, pipe, queue_handle);
  }

  switcht_handle_t switch_api_l3_mtu_create(const switcht_device_t device,
                                            const int64_t flags,
                                            const int32_t mtu) {
    switch_handle_t mtu_handle = 0;
    ::switch_api_l3_mtu_create(device, flags, mtu, &mtu_handle);
    return mtu_handle;
  }

  switcht_status_t switch_api_l3_mtu_update(const switcht_device_t device,
                                            const switcht_handle_t mtu_handle,
                                            const int32_t mtu) {
    return ::switch_api_l3_mtu_update(device, mtu_handle, mtu);
  }

  switcht_status_t switch_api_l3_mtu_delete(const switcht_device_t device,
                                            const switcht_handle_t mtu_handle) {
    return ::switch_api_l3_mtu_delete(device, mtu_handle);
  }

  int16_t switch_api_l3_mtu_get(const switcht_device_t device,
                                const switcht_handle_t mtu_handle) {
    switch_mtu_t mtu;
    ::switch_api_l3_mtu_get(device, mtu_handle, &mtu);
    return (int16_t)mtu;
  }

  void switch_api_device_attribute_get(
      switcht_api_device_info_t &_api_device_info,
      const switcht_device_t device,
      const int64_t flags) {
    switch_api_device_info_t device_info;
    ::switch_api_device_attribute_get(device, flags, &device_info);
    _api_device_info.default_vrf = device_info.default_vrf;
    _api_device_info.vrf_handle = device_info.vrf_handle;
    _api_device_info.default_vlan = device_info.default_vlan;
    _api_device_info.vlan_handle = device_info.vlan_handle;
    _api_device_info.rmac_handle = device_info.rmac_handle;
    _api_device_info.max_lag_groups = device_info.max_lag_groups;
    _api_device_info.max_lag_members = device_info.max_lag_members;
    _api_device_info.max_ecmp_groups = device_info.max_ecmp_groups;
    _api_device_info.max_ecmp_members = device_info.max_ecmp_members;
    _api_device_info.lag_hash_flags = device_info.lag_hash_flags;
    _api_device_info.ecmp_hash_flags = device_info.ecmp_hash_flags;
    _api_device_info.default_log_level = device_info.default_log_level;
    _api_device_info.install_dmac = device_info.install_dmac;
    _api_device_info.max_vrf = device_info.max_vrf;
    _api_device_info.max_ports = device_info.max_ports;
    _api_device_info.num_active_ports = device_info.num_active_ports;
    _api_device_info.max_port_mtu = device_info.max_port_mtu;
    for (uint16_t i = 0; i < device_info.port_list.num_handles; i++) {
      _api_device_info.port_list.push_back(device_info.port_list.handles[i]);
    }
    _api_device_info.eth_cpu_port = device_info.eth_cpu_port;
    _api_device_info.pcie_cpu_port = device_info.pcie_cpu_port;
    _api_device_info.refresh_interval = device_info.refresh_interval;
    _api_device_info.aging_interval = device_info.aging_interval;
    return;
  }

  switcht_handle_t switch_api_device_default_rmac_handle_get(
      const switcht_device_t device) {
    switch_handle_t rmac_handle = 0;
    ::switch_api_device_default_rmac_handle_get(device, &rmac_handle);
    return rmac_handle;
  }

  switcht_handle_t switch_api_device_default_vrf_handle_get(
      const switcht_device_t device) {
    switch_handle_t vrf_handle = 0;
    switch_vrf_t vrf_id = 0;
    ::switch_api_device_default_vrf_get(device, &vrf_id, &vrf_handle);
    return vrf_handle;
  }

  switcht_vrf_id_t switch_api_device_default_vrf_id_get(
      const switcht_device_t device) {
    switch_handle_t vrf_handle = 0;
    switch_vrf_t vrf_id = 0;
    ::switch_api_device_default_vrf_get(device, &vrf_id, &vrf_handle);
    return vrf_id;
  }

  switcht_handle_t switch_api_device_default_vlan_handle_get(
      const switcht_device_t device) {
    switch_handle_t vlan_handle = 0;
    switch_vlan_t vlan_id = 0;
    ::switch_api_device_default_vlan_get(device, &vlan_id, &vlan_handle);
    return vlan_handle;
  }

  switcht_vlan_t switch_api_device_default_vlan_id_get(
      const switcht_device_t device) {
    switch_handle_t vlan_handle = 0;
    switch_vlan_t vlan_id = 0;
    ::switch_api_device_default_vlan_get(device, &vlan_id, &vlan_handle);
    return vlan_id;
  }

  switcht_handle_t switch_api_device_cpu_port_handle_get(
      const switcht_device_t device) {
    switch_handle_t port_handle = 0;
    ::switch_api_device_cpu_port_handle_get(device, &port_handle);
    return port_handle;
  }

  switcht_port_t switch_api_device_cpu_port_get(const switcht_device_t device) {
    switch_port_t port = 0;
    ::switch_api_device_cpu_port_get(device, &port);
    return port;
  }

  switcht_port_t switch_api_device_cpu_eth_port_get(
      const switcht_device_t device) {
    switch_port_t port = 0;
    ::switch_api_device_cpu_eth_port_get(device, &port);
    return port;
  }

  switcht_port_t switch_api_device_cpu_pcie_port_get(
      const switcht_device_t device) {
    switch_port_t port = 0;
    ::switch_api_device_cpu_pcie_port_get(device, &port);
    return port;
  }

  int32_t switch_api_device_counter_refresh_interval_get(
      const switcht_device_t device) {
    switch_uint32_t refresh_interval = 0;
    ::switch_api_device_counter_refresh_interval_get(device, &refresh_interval);
    return (int32_t)refresh_interval;
  }

  switcht_status_t switch_api_device_mac_aging_interval_set(
      const switcht_device_t device, const int32_t aging_interval) {
    return ::switch_api_device_mac_aging_interval_set(device, aging_interval);
  }

  int32_t switch_api_device_mac_aging_interval_get(
      const switcht_device_t device) {
    int32_t aging_interval = 0;
    ::switch_api_device_mac_aging_interval_get(device, &aging_interval);
    return aging_interval;
  }

  switcht_handle_t switch_api_device_recirc_port_get(
      const switcht_device_t device,
      switcht_pipe_t pipe_id) {
    switch_handle_t handle;
    ::switch_api_device_recirc_port_get(device, pipe_id, &handle);
    return handle;
  }

  int16_t switch_api_device_max_recirc_ports_get(
      const switcht_device_t device) {
    switch_uint16_t num_ports;
    ::switch_api_device_max_recirc_ports_get(device, &num_ports);
    return (int16_t)num_ports;
  }

  switcht_acl_action_t switch_api_device_dmac_miss_packet_action_get(
      const switcht_device_t device,
      switcht_packet_type_t pkt_type) {
    switch_acl_action_t action;
    ::switch_api_device_dmac_miss_packet_action_get(
          device, (switch_packet_type_t)pkt_type, &action);
    return action;
  }

  bool switch_api_device_cut_through_mode_get(
      const switcht_device_t device) {
    bool enable;
    ::switch_api_device_cut_through_mode_get(device, &enable);
    return enable;
  }

  void switch_api_config_smac_program_set(
    const switcht_device_t device, bool flag) {
    ::switch_api_config_smac_program_set(device, flag);
  }

  switch_status_t switch_api_device_mac_learning_set(
    const switcht_device_t device, bool enable) {
    ::switch_api_device_mac_learning_set(device, enable);
  }

  bool switch_api_device_mac_learning_get(
    const switcht_device_t device) {
    bool enable = 0;
    ::switch_api_device_mac_learning_get(device, &enable);
    return enable;
  }

  void switch_api_handles_get(std::vector<switcht_handle_t> &_handles,
                                  const switcht_device_t device,
                                  const switcht_handle_type_t type) {
    switch_handle_t *handles = NULL;
    switch_size_t num_handles = 0;
    ::switch_api_handles_get(device,
                             (switch_handle_type_t)type,
                             &num_handles,
                             &handles);
    for (uint32_t i = 0; i < num_handles; i++) {
      _handles.push_back(handles[i]);
    }

    SWITCH_FREE(device, handles);
    return;
  }

  int32_t switch_api_route_entry_add_perf_test(
      const switcht_device_t device,
      const std::vector<switcht_route_entry_t> &route_entries) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    int32_t rate = -1;
    uint32_t index = 0;

    bf_sys_trace_level_set(BF_MOD_SWITCHAPI, BF_LOG_ERR);
    bf_sys_log_level_set(BF_MOD_SWITCHAPI, BF_LOG_DEST_FILE, BF_LOG_ERR);

    std::vector<switcht_route_entry_t>::const_iterator it =
        route_entries.begin();
    switch_api_route_entry_t *api_route_entries =
        (switch_api_route_entry_t *)SWITCH_MALLOC(
            device, sizeof(switch_api_route_entry_t), route_entries.size());

    for (uint32_t i = 0; i < route_entries.size(); ++i, ++it) {
      api_route_entries[i].vrf_handle = it->vrf_handle;
      api_route_entries[i].rif_handle = it->rif_handle;
      api_route_entries[i].nhop_handle = it->nhop_handle;
      switch_parse_ip_address(it->ip_addr, &api_route_entries[i].ip_address);
    }

    ::switch_api_log_level_all_set(SWITCH_LOG_LEVEL_ERROR);

    struct timespec start = {0}, end = {0}, diff = {0};
    clock_gettime(CLOCK_MONOTONIC, &start);

    ::switch_api_batch_begin();

    for (; index < route_entries.size(); ++index) {
      status = ::switch_api_l3_route_add(device, &api_route_entries[index]);
      if (status != SWITCH_STATUS_SUCCESS) {
        return rate;
      }
    }

    ::switch_api_batch_end(false);

    clock_gettime(CLOCK_MONOTONIC, &end);
    diff.tv_sec = end.tv_sec - start.tv_sec;
    diff.tv_nsec = end.tv_nsec - start.tv_nsec;
    if (diff.tv_nsec < 0) {
      diff.tv_sec -= 1;
      diff.tv_nsec += 1000000000;
    }

    uint64_t microseconds = diff.tv_sec * 1000000 + diff.tv_nsec / 1000;
    double ops_per_microsecond = (double)route_entries.size() / microseconds;
    rate = ops_per_microsecond * 1000000;

    SWITCH_FREE(device, api_route_entries);
    return rate;
  }

  int32_t switch_api_mac_entry_add_perf_test(
      const switcht_device_t device,
      const std::vector<switcht_api_mac_entry_t> &mac_entries) {
    switch_status_t status = SWITCH_STATUS_SUCCESS;
    int32_t rate = -1;
    uint32_t index = 0;
    std::vector<switcht_api_mac_entry_t>::const_iterator it = mac_entries.begin();
    switch_api_mac_entry_t *api_mac_entries =
        (switch_api_mac_entry_t *)SWITCH_MALLOC(
            device, sizeof(switch_api_mac_entry_t), mac_entries.size());

    bf_sys_trace_level_set(BF_MOD_SWITCHAPI, BF_LOG_ERR);
    bf_sys_log_level_set(BF_MOD_SWITCHAPI, BF_LOG_DEST_FILE, BF_LOG_ERR);

    for (uint32_t i = 0; i < mac_entries.size(); ++i, ++it) {
      api_mac_entries[i].network_handle = it->network_handle;
      api_mac_entries[i].handle = it->handle;
      api_mac_entries[i].entry_type = (switch_mac_entry_type_t)it->entry_type;
      switch_string_to_mac(it->mac_addr, api_mac_entries[i].mac.mac_addr);
    }

    ::switch_api_log_level_all_set(SWITCH_LOG_LEVEL_ERROR);

    struct timespec start = {0}, end = {0}, diff = {0};
    clock_gettime(CLOCK_MONOTONIC, &start);

    ::switch_api_batch_begin();

    for (; index < mac_entries.size(); ++index) {
      status =
          ::switch_api_mac_table_entry_add(device, &api_mac_entries[index]);
      if (status != SWITCH_STATUS_SUCCESS) {
        return rate;
      }
    }

    ::switch_api_batch_end(false);

    clock_gettime(CLOCK_MONOTONIC, &end);
    diff.tv_sec = end.tv_sec - start.tv_sec;
    diff.tv_nsec = end.tv_nsec - start.tv_nsec;
    if (diff.tv_nsec < 0) {
      diff.tv_sec -= 1;
      diff.tv_nsec += 1000000000;
    }

    uint64_t microseconds = diff.tv_sec * 1000000 + diff.tv_nsec / 1000;
    double ops_per_microsecond = (double)mac_entries.size() / microseconds;
    rate = ops_per_microsecond * 1000000;

    SWITCH_FREE(device, api_mac_entries);

    return rate;
  }
  switcht_status_t switch_api_ipv6_hash_input_fields_set(const switcht_device_t device, const int32_t fields) {
    switch_hash_ipv6_input_fields_t hash_fields;
    switch (fields) {
    case SWITCH_HASH_IPV6_INPUT_SIP_DIP_PROT_SP_DP:
      hash_fields = SWITCH_HASH_IPV6_INPUT_SIP_DIP_PROT_SP_DP;
      break;
    case SWITCH_HASH_IPV6_INPUT_PROT_DP_SIP_SP_DIP:
      hash_fields = SWITCH_HASH_IPV6_INPUT_PROT_DP_SIP_SP_DIP;
      break;
    default:
      return SWITCH_STATUS_INVALID_PARAMETER;
    }
    int32_t status =  ::switch_api_ipv6_hash_input_fields_set(device, hash_fields);

    return status;
  }

  switcht_status_t switch_api_ipv4_hash_input_fields_set(const switcht_device_t device, const int32_t fields) {
    switch_hash_ipv4_input_fields_t hash_fields;
    switch (fields) {
    case SWITCH_HASH_IPV4_INPUT_SIP_DIP_PROT_SP_DP:
      hash_fields = SWITCH_HASH_IPV4_INPUT_SIP_DIP_PROT_SP_DP;
      break;
    case SWITCH_HASH_IPV4_INPUT_PROT_DP_DIP_SP_SIP:
      hash_fields = SWITCH_HASH_IPV4_INPUT_PROT_DP_DIP_SP_SIP;
      break;
    default:
      return SWITCH_STATUS_INVALID_PARAMETER;
    }

    int32_t status =  ::switch_api_ipv4_hash_input_fields_set(device, hash_fields);

    return status;
  }

  switcht_status_t switch_api_non_ip_hash_input_fields_set(const switcht_device_t device, const int32_t fields) {
    switch_hash_non_ip_input_fields_t hash_fields;
    switch (fields) {
    case SWITCH_HASH_NON_IP_INPUT_IF_SMAC_DMAC_ETYPE:
      hash_fields = SWITCH_HASH_NON_IP_INPUT_IF_SMAC_DMAC_ETYPE;
      break;
    case SWITCH_HASH_NON_IP_INPUT_ETYPE_SMAC_IF_DMAC:
      hash_fields = SWITCH_HASH_NON_IP_INPUT_ETYPE_SMAC_IF_DMAC;
      break;
    default:
      return SWITCH_STATUS_INVALID_PARAMETER;
    }

    int32_t status =  ::switch_api_non_ip_hash_input_fields_set(device, hash_fields);

    return status;
  }

  switcht_status_t switch_api_ipv6_hash_input_fields_attribute_set(const switcht_device_t device, const int32_t fields, const int32_t attr_flags) {
    switch_hash_ipv6_input_fields_t hash_fields;
    switch (fields) {
    case SWITCH_HASH_IPV6_INPUT_SIP_DIP_PROT_SP_DP:
      hash_fields = SWITCH_HASH_IPV6_INPUT_SIP_DIP_PROT_SP_DP;
      break;
    case SWITCH_HASH_IPV6_INPUT_PROT_DP_SIP_SP_DIP:
      hash_fields = SWITCH_HASH_IPV6_INPUT_PROT_DP_SIP_SP_DIP;
      break;
    default:
      return SWITCH_STATUS_INVALID_PARAMETER;
    }

    int32_t status =  ::switch_api_ipv6_hash_input_fields_attribute_set(device, hash_fields, attr_flags);

    return status;
  }

  switcht_status_t switch_api_ipv4_hash_input_fields_attribute_set(const switcht_device_t device, const int32_t fields, const int32_t attr_flags) {
    switch_hash_ipv4_input_fields_t hash_fields;
    switch (fields) {
    case SWITCH_HASH_IPV4_INPUT_SIP_DIP_PROT_SP_DP:
      hash_fields = SWITCH_HASH_IPV4_INPUT_SIP_DIP_PROT_SP_DP;
      break;
    case SWITCH_HASH_IPV4_INPUT_PROT_DP_DIP_SP_SIP:
      hash_fields = SWITCH_HASH_IPV4_INPUT_PROT_DP_DIP_SP_SIP;
      break;
    default:
      return SWITCH_STATUS_INVALID_PARAMETER;
    }

    int32_t status =  ::switch_api_ipv4_hash_input_fields_attribute_set(device, hash_fields, attr_flags);

    return status;
  }

  switcht_status_t switch_api_non_ip_hash_input_fields_attribute_set(const switcht_device_t device, const int32_t fields, const int32_t attr_flags) {
    switch_hash_non_ip_input_fields_t hash_fields;
    switch (fields) {
    case SWITCH_HASH_NON_IP_INPUT_IF_SMAC_DMAC_ETYPE:
      hash_fields = SWITCH_HASH_NON_IP_INPUT_IF_SMAC_DMAC_ETYPE;
      break;
    case SWITCH_HASH_NON_IP_INPUT_ETYPE_SMAC_IF_DMAC:
      hash_fields = SWITCH_HASH_NON_IP_INPUT_ETYPE_SMAC_IF_DMAC;
      break;
    default:
      return SWITCH_STATUS_INVALID_PARAMETER;
    }

    int32_t status =  ::switch_api_non_ip_hash_input_fields_attribute_set(device, hash_fields, attr_flags);

    return status;
  }

  switcht_status_t switch_api_ipv6_hash_algorithm_set(const switcht_device_t device, const int32_t algorithm) {
    switch_hash_ipv6_algorithm_t hash_algorithm;
    switch (algorithm) {
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16:
      hash_algorithm = SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16;
      break;
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_DECT:
      hash_algorithm = SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_DECT;
      break;
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_GENIBUS:
      hash_algorithm = SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_GENIBUS;
      break;
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_DNP:
      hash_algorithm = SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_DNP;
      break;
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_TELEDISK:
      hash_algorithm = SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_TELEDISK;
      break;
    default:
      return SWITCH_STATUS_INVALID_PARAMETER;
    }

    int32_t status =  ::switch_api_ipv6_hash_algorithm_set(device, hash_algorithm);

    return status;
  }

  switcht_status_t switch_api_ipv4_hash_algorithm_set(const switcht_device_t device, const int32_t algorithm) {
    switch_hash_ipv4_algorithm_t hash_algorithm;
    switch (algorithm) {
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16:
      hash_algorithm = SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16;
      break;
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_DECT:
      hash_algorithm = SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_DECT;
      break;
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_GENIBUS:
      hash_algorithm = SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_GENIBUS;
      break;
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_DNP:
      hash_algorithm = SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_DNP;
      break;
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_TELEDISK:
      hash_algorithm = SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_TELEDISK;
      break;
    default:
      return SWITCH_STATUS_INVALID_PARAMETER;
    }

    int32_t status =  ::switch_api_ipv4_hash_algorithm_set(device, hash_algorithm);

    return status;
  }

  switcht_status_t switch_api_non_ip_hash_algorithm_set(const switcht_device_t device, const int32_t algorithm) {
    switch_hash_non_ip_algorithm_t hash_algorithm;
    switch (algorithm) {
    case SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16:
      hash_algorithm = SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16;
      break;
    case SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_DECT:
      hash_algorithm = SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_DECT;
      break;
    case SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_GENIBUS:
      hash_algorithm = SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_GENIBUS;
      break;
    case SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_DNP:
      hash_algorithm = SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_DNP;
      break;
    case SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_TELEDISK:
      hash_algorithm = SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_TELEDISK;
      break;
    default:
      return SWITCH_STATUS_INVALID_PARAMETER;
    }
    int32_t status =  ::switch_api_non_ip_hash_algorithm_set(device, hash_algorithm);

    return status;
  }

  switcht_status_t switch_api_ipv6_hash_seed_set(const switcht_device_t device, const int64_t seed) {
    int32_t status =  ::switch_api_ipv6_hash_seed_set(device, seed);

    return status;
  }

  switcht_status_t switch_api_ipv4_hash_seed_set(const switcht_device_t device, const int64_t seed) {
    int32_t status =  ::switch_api_ipv4_hash_seed_set(device, seed);

    return status;
  }

  switcht_status_t switch_api_non_ip_hash_seed_set(const switcht_device_t device, const int64_t seed) {
    int32_t status =  ::switch_api_non_ip_hash_seed_set(device, seed);

    return status;
  }

  void switch_api_ipv6_hash_input_fields_get(switch_hash_ipv6_input_fields_res_t& _return, const switcht_device_t device) {
    switch_hash_ipv6_input_fields_t fields;
    int32_t status =  ::switch_api_ipv6_hash_input_fields_get(device, &fields);
    _return.status = status;
    if(status == SWITCH_STATUS_SUCCESS) {
      _return.fields = fields;
    }
  }

  void switch_api_ipv4_hash_input_fields_get(switch_hash_ipv4_input_fields_res_t& _return, const switcht_device_t device) {
    switch_hash_ipv4_input_fields_t fields;
    int32_t status =  ::switch_api_ipv4_hash_input_fields_get(device, &fields);
    _return.status = status;
    if(status == SWITCH_STATUS_SUCCESS) {
      _return.fields = fields;
    }
  }

  void switch_api_non_ip_hash_input_fields_get(switch_hash_non_ip_input_fields_res_t& _return, const switcht_device_t device) {
    switch_hash_non_ip_input_fields_t fields;
    int32_t status =  ::switch_api_non_ip_hash_input_fields_get(device, &fields);
    _return.status = status;
    if(status == SWITCH_STATUS_SUCCESS) {
      _return.fields = fields;
    }
  }

  void switch_api_ipv6_hash_input_fields_attribute_get(switch_hash_input_fields_attribute_res_t& _return, const switcht_device_t device, const int32_t  fields) {
    switch_hash_ipv6_input_fields_t hash_fields;
    switch_uint32_t attr_flags;
    switch (fields) {
    case SWITCH_HASH_IPV6_INPUT_SIP_DIP_PROT_SP_DP:
      hash_fields = SWITCH_HASH_IPV6_INPUT_SIP_DIP_PROT_SP_DP;
      break;
    case SWITCH_HASH_IPV6_INPUT_PROT_DP_SIP_SP_DIP:
      hash_fields = SWITCH_HASH_IPV6_INPUT_PROT_DP_SIP_SP_DIP;
      break;
    default:
      _return.status = SWITCH_STATUS_INVALID_PARAMETER;
      return;
    }

    int32_t status =  ::switch_api_ipv6_hash_input_field_attribute_get(device, hash_fields, &attr_flags);
    _return.status = status;
    if(status == SWITCH_STATUS_SUCCESS) {
      _return.attr_flags = attr_flags;
    }
  }

  void switch_api_ipv4_hash_input_fields_attribute_get(switch_hash_input_fields_attribute_res_t& _return, const switcht_device_t device, const int32_t  fields) {
    switch_hash_ipv4_input_fields_t hash_fields;
    switch_uint32_t attr_flags;
    switch (fields) {
    case SWITCH_HASH_IPV4_INPUT_SIP_DIP_PROT_SP_DP:
      hash_fields = SWITCH_HASH_IPV4_INPUT_SIP_DIP_PROT_SP_DP;
      break;
    case SWITCH_HASH_IPV4_INPUT_PROT_DP_DIP_SP_SIP:
      hash_fields = SWITCH_HASH_IPV4_INPUT_PROT_DP_DIP_SP_SIP;
      break;
    default:
      _return.status = SWITCH_STATUS_INVALID_PARAMETER;
      return;
    }

    int32_t status =  ::switch_api_ipv4_hash_input_field_attribute_get(device, hash_fields, &attr_flags);
    _return.status = status;
    if(status == SWITCH_STATUS_SUCCESS) {
      _return.attr_flags = attr_flags;
    }
  }
  void switch_api_non_ip_hash_input_fields_attribute_get(switch_hash_input_fields_attribute_res_t& _return, const switcht_device_t device, const int32_t fields) {
    switch_hash_non_ip_input_fields_t hash_fields;
    switch_uint32_t attr_flags;
    switch (fields) {
    case SWITCH_HASH_NON_IP_INPUT_IF_SMAC_DMAC_ETYPE:
      hash_fields = SWITCH_HASH_NON_IP_INPUT_IF_SMAC_DMAC_ETYPE;
      break;
    case SWITCH_HASH_NON_IP_INPUT_ETYPE_SMAC_IF_DMAC:
      hash_fields = SWITCH_HASH_NON_IP_INPUT_ETYPE_SMAC_IF_DMAC;
      break;
    default:
      _return.status = SWITCH_STATUS_INVALID_PARAMETER;
      return;
    }

    int32_t status =  ::switch_api_non_ip_hash_input_field_attribute_get(device, hash_fields, &attr_flags);
    _return.status = status;
    if(status == SWITCH_STATUS_SUCCESS) {
      _return.attr_flags = attr_flags;
    }
  }

  void switch_api_ipv6_hash_algorithm_get(switch_hash_ipv6_algo_res_t& _return, const switcht_device_t device) {
    switch_hash_ipv6_algorithm_t algorithm;
    int32_t status =  ::switch_api_ipv6_hash_algorithm_get(device, &algorithm);
    _return.status = status;
    if(status == SWITCH_STATUS_SUCCESS) {
      _return.algorithm = algorithm;
    }
  }

  void switch_api_ipv4_hash_algorithm_get(switch_hash_ipv4_algo_res_t& _return, const switcht_device_t device) {
    switch_hash_ipv4_algorithm_t algorithm;
    int32_t status =  ::switch_api_ipv4_hash_algorithm_get(device, &algorithm);
    _return.status = status;
    if(status == SWITCH_STATUS_SUCCESS) {
      _return.algorithm = algorithm;
    }
  }

  void switch_api_non_ip_hash_algorithm_get(switch_hash_non_ip_algo_res_t& _return, const switcht_device_t device) {
    switch_hash_non_ip_algorithm_t algorithm;
    int32_t status =  ::switch_api_non_ip_hash_algorithm_get(device, &algorithm);
    _return.status = status;
    if(status == SWITCH_STATUS_SUCCESS) {
      _return.algorithm = algorithm;
    }
  }

  void switch_api_ipv6_hash_seed_get(switch_hash_ipv6_seed_res_t& _return, const switcht_device_t device) {
    uint64_t seed;
    int32_t status =  ::switch_api_ipv6_hash_seed_get(device, &seed);
    _return.status = status;
    if(status == SWITCH_STATUS_SUCCESS) {
      _return.seed = seed;
    }
  }

  void switch_api_ipv4_hash_seed_get(switch_hash_ipv4_seed_res_t& _return, const switcht_device_t device) {
    uint64_t seed;
    int32_t status =  ::switch_api_ipv4_hash_seed_get(device, &seed);
    _return.status = status;
    if(status == SWITCH_STATUS_SUCCESS) {
      _return.seed = seed;
    }
  }

  void switch_api_non_ip_hash_seed_get(switch_hash_non_ip_seed_res_t& _return, const switcht_device_t device) {
    uint64_t seed;
    int32_t status =  ::switch_api_non_ip_hash_seed_get(device, &seed);
    _return.status = status;
    if(status == SWITCH_STATUS_SUCCESS) {
      _return.seed = seed;
    }
  }

  void switch_api_lag_hash_seed_set(const switcht_device_t device,
                                    const int64_t seed) {
    int32_t status = ::switch_api_lag_hash_seed_set(device, seed);
    if (status != SWITCH_STATUS_SUCCESS) {
      InvalidSwitchOperation iop;
      iop.code = status;
      throw iop;
    }
  }

  void switch_api_ecmp_hash_seed_set(const switcht_device_t device,
                                     const int64_t seed) {
    int32_t status = ::switch_api_ecmp_hash_seed_set(device, seed);
    if (status != SWITCH_STATUS_SUCCESS) {
      InvalidSwitchOperation iop;
      iop.code = status;
      throw iop;
    }
  }

  int64_t switch_api_lag_hash_seed_get(const switcht_device_t device) {
    uint64_t seed;
    int32_t status = ::switch_api_lag_hash_seed_get(device, &seed);
    if (status != SWITCH_STATUS_SUCCESS) {
      InvalidSwitchOperation iop;
      iop.code = status;
      throw iop;
    }
    return seed;
  }

  int64_t switch_api_ecmp_hash_seed_get(const switcht_device_t device) {
    uint64_t seed;
    int32_t status = ::switch_api_ecmp_hash_seed_get(device, &seed);
    if (status != SWITCH_STATUS_SUCCESS) {
      InvalidSwitchOperation iop;
      iop.code = status;
      throw iop;
    }
    return seed;
  }
};

static void *api_rpc_server_thread(void *args) {
  int port = SWITCH_API_RPC_SERVER_PORT;
  shared_ptr<switch_api_rpcHandler> handler(new switch_api_rpcHandler());
  shared_ptr<TProcessor> processor(new switch_api_rpcProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(
      new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  TSimpleServer server(
      processor, serverTransport, transportFactory, protocolFactory);
  /* set thread name to "api_thrift" */
  pthread_setname_np(pthread_self(), "api_thrift");
  pthread_mutex_lock(&cookie_mutex);
  cookie = (void *)processor.get();
  pthread_cond_signal(&cookie_cv);
  pthread_mutex_unlock(&cookie_mutex);
  server.serve();
  return NULL;
}

static pthread_t api_rpc_thread;

extern "C" {
int start_switch_api_rpc_server(void) {
  std::cerr << "Starting API RPC server on port " << SWITCH_API_RPC_SERVER_PORT
            << std::endl;
  cookie = NULL;
  pthread_mutex_init(&cookie_mutex, NULL);
  pthread_cond_init(&cookie_cv, NULL);
  int status =
      pthread_create(&api_rpc_thread, NULL, api_rpc_server_thread, NULL);
  if (status) return status;
  pthread_mutex_lock(&cookie_mutex);
  while (!cookie) {
    pthread_cond_wait(&cookie_cv, &cookie_mutex);
  }
  pthread_mutex_unlock(&cookie_mutex);
  pthread_mutex_destroy(&cookie_mutex);
  pthread_cond_destroy(&cookie_cv);
  return status;
}

int stop_switch_api_rpc_server(void) {
  int status = pthread_cancel(api_rpc_thread);
  if (status == 0) {
    int s = pthread_join(api_rpc_thread, NULL);
  }
  return status;
}

int start_switch_api_rpc_server0(char *) {
  return start_switch_api_rpc_server();
}
}
