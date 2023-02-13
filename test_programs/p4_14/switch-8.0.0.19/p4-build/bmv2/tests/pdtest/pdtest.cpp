/* Copyright 2013-present Barefoot Networks, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Antonin Bas (antonin@barefootnetworks.com)
 *
 */

#include <iostream>
#include <cstring>
#include <thread>
#include <chrono>
#include <cassert>

#include <thrift_endpoint.h>

#include <bm/pdfixed/pd_static.h>
#include <bm/pdfixed/pd_pre.h>
#include <bm/pdfixed/pd_mirroring.h>

#include <pd/pd_tables.h>
#include <pd/pd_meters.h>
#include <pd/pd_counters.h>
#include <pd/pd_registers.h>
#include <pd/pd.h>

#define DEVICE_THRIFT_PORT 9090

int main() {
  start_server();

  p4_pd_init();

  p4_pd_dev_target_t dev_tgt = {0, 0xFF};
  p4_pd_entry_hdl_t entry_hdl;

  /* P4 dependent initialization */
  p4_pd_pdtest_init();
  p4_pd_pdtest_assign_device(dev_tgt.device_id, NULL, DEVICE_THRIFT_PORT);

  p4_pd_sess_hdl_t sess_hdl;
  p4_pd_client_init(&sess_hdl);

  std::cerr << "session handle is " << sess_hdl << std::endl;

  /* TEST BEGIN */

  p4_pd_pdtest_actionA_action_spec actionA_action_spec =
    {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
  p4_pd_pdtest_actionB_action_spec actionB_action_spec =
    {0xab};

  p4_pd_bytes_meter_spec_t meter_spec;
  meter_spec.cir_kbps = 8000;
  meter_spec.cburst_kbits = 8;
  meter_spec.pir_kbps = 16000;
  meter_spec.pburst_kbits = 8;

  // right now PD assumes everything is passed in network byte order, so this
  // will actually be interpreted as byte string "bb00aa00"
  p4_pd_pdtest_ExactOne_match_spec_t ExactOne_match_spec = {0x00aa00bb};
  p4_pd_pdtest_ExactOne_table_add_with_actionA(sess_hdl, dev_tgt,
                                               &ExactOne_match_spec,
                                               &actionA_action_spec,
                                               &meter_spec,
                                               &entry_hdl);

  // the thrift endpoint remembers the last entry added
  p4_pd_pdtest_ExactOne_match_spec_t ExactOne_match_spec_1;
  char *action_name;
  uint8_t action_data[128];
  int num_action_bytes;
  p4_pd_pdtest_ExactOne_get_entry(sess_hdl, dev_tgt.device_id, entry_hdl,
                                  true, &ExactOne_match_spec_1, &action_name,
                                  action_data, &num_action_bytes);
  assert(ExactOne_match_spec_1.header_test_field32 == 0x00aa00bb);
  assert(std::string(action_name) == "actionA");
  assert(!std::memcmp(action_data, actionA_action_spec.action_param,
                      sizeof(actionA_action_spec.action_param)));

  p4_pd_pdtest_ExactOne_table_modify_with_actionB(sess_hdl, dev_tgt.device_id,
                                                  entry_hdl,
                                                  &actionB_action_spec,
                                                  &meter_spec);

  p4_pd_counter_value_t counter_value;
  p4_pd_pdtest_ExactOne_read_counter(sess_hdl, dev_tgt, entry_hdl, &counter_value);

  p4_pd_pdtest_ExactOne_reset_counters(sess_hdl, dev_tgt);

  p4_pd_pdtest_ExactOne_table_delete(sess_hdl, dev_tgt.device_id, entry_hdl);

  p4_pd_pdtest_ExactOneAgeing_match_spec_t ExactOneAgeing_match_spec = {0x00aa00bb};
  p4_pd_pdtest_ExactOneAgeing_table_add_with_actionA(sess_hdl, dev_tgt,
                                                     &ExactOneAgeing_match_spec,
                                                     &actionA_action_spec,
                                                     2000, // ttl
                                                     &entry_hdl);

  p4_pd_pdtest_LpmOne_match_spec_t LpmOne_match_spec = {0x12345678, 12};
  p4_pd_pdtest_LpmOne_table_add_with_actionA(sess_hdl, dev_tgt,
                                             &LpmOne_match_spec,
                                             &actionA_action_spec,
                                             &entry_hdl);

  p4_pd_pdtest_TernaryOne_match_spec_t TernaryOne_match_spec = {0x10101010,
                                                                0xff000a00};
  p4_pd_pdtest_TernaryOne_table_add_with_actionA(sess_hdl, dev_tgt,
                                                 &TernaryOne_match_spec,
                                                 22 /* priority */,
                                                 &actionA_action_spec,
                                                 &entry_hdl);

  p4_pd_pdtest_RangeOne_match_spec_t RangeOne_match_spec = {0x22000000,
                                                            0x4b140000};
  p4_pd_pdtest_RangeOne_table_add_with_actionA(sess_hdl, dev_tgt,
                                               &RangeOne_match_spec,
                                               22 /* priority */,
                                               &actionA_action_spec,
                                               &entry_hdl);

  /* we want the 20-bit key to be abcde (in network order)
     which means the 4 bytes are 00 0a bc de (in network order)
     which means the int is de bc 0a 00 (in host order) */
  p4_pd_pdtest_ExactOneNA_match_spec_t ExactOneNA_match_spec = {0xdebc0a00};
  p4_pd_pdtest_ExactOneNA_table_add_with_actionA(sess_hdl, dev_tgt,
                                                 &ExactOneNA_match_spec,
                                                 &actionA_action_spec,
                                                 &entry_hdl);

  p4_pd_pdtest_ExactTwo_match_spec_t ExactTwo_match_spec = {0xaabbccdd, 0xeeff};
  p4_pd_pdtest_ExactTwo_table_add_with_actionA(sess_hdl, dev_tgt,
                                               &ExactTwo_match_spec,
                                               &actionA_action_spec,
                                               &entry_hdl);

  p4_pd_pdtest_ExactAndValid_match_spec_t ExactAndValid_match_spec = {0xaabbccdd, 1u};
  p4_pd_pdtest_ExactAndValid_table_add_with_actionA(sess_hdl, dev_tgt,
                                                    &ExactAndValid_match_spec,
                                                    &actionA_action_spec,
                                                    &entry_hdl);

  p4_pd_pdtest_ExactOne_set_default_action_actionA(sess_hdl, dev_tgt,
                                                   &actionA_action_spec,
                                                   &meter_spec,
                                                   &entry_hdl);

  /* indirect table */

  p4_pd_mbr_hdl_t mbr_hdl;

  p4_pd_pdtest_ActProf_add_member_with_actionA(sess_hdl, dev_tgt,
                                               &actionA_action_spec,
                                               &mbr_hdl);

  p4_pd_pdtest_ActProf_modify_member_with_actionB(sess_hdl, dev_tgt.device_id,
                                                  mbr_hdl,
                                                  &actionB_action_spec);

  p4_pd_pdtest_Indirect_match_spec_t Indirect_match_spec = {0xaabbccdd};
  p4_pd_pdtest_Indirect_add_entry(sess_hdl, dev_tgt,
                                  &Indirect_match_spec, mbr_hdl,
                                  &entry_hdl);

  p4_pd_pdtest_Indirect_table_delete(sess_hdl, dev_tgt.device_id, entry_hdl);

  p4_pd_pdtest_Indirect_set_default_entry(sess_hdl, dev_tgt,
                                          mbr_hdl, &entry_hdl);

  p4_pd_pdtest_ActProf_del_member(sess_hdl, dev_tgt.device_id, mbr_hdl);

  /* meter test */

  // indirect meter
  p4_pd_pdtest_meter_set_MeterA(sess_hdl, dev_tgt, 16, &meter_spec);

  // direct meter
  p4_pd_pdtest_meter_set_ExactOne_meter(sess_hdl, dev_tgt, 18, &meter_spec);

  /* multicast */

  uint8_t port_map[PRE_PORTS_MAX];
  std::memset(port_map, 0, sizeof(port_map));
  port_map[0] = 1;
  port_map[1] = (1 << 5) + (1 << 2);
  uint8_t lag_map[PRE_LAG_MAX];
  std::memset(lag_map, 0, sizeof(lag_map));
  lag_map[0] = (1 << 7);
  lag_map[1] = (1 << 5);
  p4_pd_entry_hdl_t node_hdl;
  p4_pd_mc_node_create(sess_hdl, dev_tgt.device_id, 21, port_map, lag_map, &node_hdl);

  /* counters */

  // indirect counter
  p4_pd_pdtest_counter_read_CounterA(sess_hdl, dev_tgt, 17, 0);
  counter_value.bytes = 12;
  counter_value.packets = 34;
  p4_pd_pdtest_counter_write_CounterA(sess_hdl, dev_tgt, 17, counter_value);

  p4_pd_pdtest_counter_hw_sync_CounterA(
      sess_hdl, dev_tgt,
      [](int d, void *c){ std::cout << "hw_sync_CounterA\n"; }, nullptr);
  std::this_thread::sleep_for(std::chrono::seconds(2));

  // direct counter
  p4_pd_pdtest_counter_read_ExactOne_counter(sess_hdl, dev_tgt, 18, 0);
  p4_pd_pdtest_counter_write_ExactOne_counter(sess_hdl, dev_tgt, 18, counter_value);

  /* registers */

  p4_pd_pdtest_register_reset_RegisterA(sess_hdl, dev_tgt);

  /* mirroring */
  p4_pd_mirror_session_create(sess_hdl, dev_tgt,
                              PD_MIRROR_TYPE_NORM, PD_DIR_INGRESS,
                              11, 12,
                              4096, 0, false,
                              0, 0, nullptr, 0);

  /* learning */
  uint64_t timeout_us = 64000;  // 64 ms
  p4_pd_pdtest_set_learning_timeout(sess_hdl, dev_tgt.device_id, timeout_us);

  /* END TEST */

  p4_pd_pdtest_remove_device(dev_tgt.device_id);

  p4_pd_client_cleanup(sess_hdl);

  p4_pd_cleanup();

  return 0;
}
