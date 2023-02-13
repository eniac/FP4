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

#include <bm/Standard.h>

#include <iostream>
#include <sstream>
#include <iomanip>

namespace bm_runtime { namespace standard {

class StandardHandler : virtual public StandardIf {
public:
  // a trick for testing get_entry, stores the last entry added
  BmMtEntry most_recent_entry{};

  StandardHandler() { }

  std::string ToHex(const std::string& s, bool upper_case = false) {
    std::ostringstream ret;

    for (std::string::size_type i = 0; i < s.length(); i++) {
      ret << std::setw(2) << std::setfill('0') << std::hex
	  << (upper_case ? std::uppercase : std::nouppercase)
	  << (int) static_cast<unsigned char>(s[i]);
    }

    return ret.str();
  }

  void print_spec(const std::vector<std::string> &v) {
    for(const auto &e : v)
      std::cout << ToHex(e) << " ";
    std::cout << std::endl;
  }

  void print_match_param(const BmMatchParam &param) {
    switch(param.type) {
    case BmMatchParamType::type::EXACT:
      std::cout << "EXACT: "
		<< ToHex(param.exact.key);
      break;
    case BmMatchParamType::type::LPM:
      std::cout << "LPM: "
		<< ToHex(param.lpm.key) << "/" << param.lpm.prefix_length;
      break;
    case BmMatchParamType::type::TERNARY:
      std::cout << "TERNARY: "
		<< ToHex(param.ternary.key) << "&&&"
                << ToHex(param.ternary.mask);
      break;
    case BmMatchParamType::type::VALID:
      std::cout << "VALID: "
		<< std::boolalpha << param.valid.key << std::noboolalpha;
      break;
    case BmMatchParamType::type::RANGE:
      std::cout << "RANGE: "
		<< ToHex(param.range.start) << "->" << ToHex(param.range.end_);
      break;
    default:
      assert(0 && "invalid match type");
      break;
    }
    std::cout << std::endl;
  }

  BmEntryHandle bm_mt_add_entry(const int32_t cxt_id, const std::string& table_name, const BmMatchParams& match_key, const std::string& action_name, const BmActionData& action_data, const BmAddEntryOptions& options) {
    std::cout << "bm_mt_add_entry" << std::endl
	      << table_name << std::endl;
    for(const auto &p : match_key)
      print_match_param(p);
    std::cout << action_name << std::endl;
    print_spec(action_data);
    if(options.__isset.priority)
      std::cout << options.priority << std::endl;
    most_recent_entry.entry_handle = 0;
    most_recent_entry.match_key = match_key;
    most_recent_entry.options = options;
    BmActionEntry &action_entry = most_recent_entry.action_entry;
    action_entry.action_type = BmActionEntryType::ACTION_DATA;
    action_entry.__set_action_name(action_name);
    action_entry.__set_action_data(action_data);
    return 0;
  }

  void bm_mt_set_default_action(const int32_t cxt_id, const std::string& table_name, const std::string& action_name, const BmActionData& action_data) {
    std::cout << "bm_mt_set_default_action" << std::endl
	      << table_name << std::endl
	      << action_name << std::endl;
    print_spec(action_data);
  }

  void bm_mt_delete_entry(const int32_t cxt_id, const std::string& table_name, const BmEntryHandle entry_handle) {
    std::cout << "bm_mt_delete_entry" << std::endl
	      << table_name << std::endl
	      << entry_handle << std::endl;
  }

  void bm_mt_modify_entry(const int32_t cxt_id, const std::string& table_name, const BmEntryHandle entry_handle, const std::string &action_name, const BmActionData& action_data) {
    std::cout << "bm_mt_modify_entry" << std::endl
	      << table_name << std::endl
	      << entry_handle << std::endl
	      << action_name << std::endl;
    print_spec(action_data);
  }

  void bm_mt_set_entry_ttl(const int32_t cxt_id, const std::string& table_name, const BmEntryHandle entry_handle, const int32_t timeout_ms) {
    std::cout << "bm_mt_set_entry_ttl" << std::endl
	      << table_name << std::endl
	      << entry_handle << std::endl
	      << timeout_ms << std::endl;
  }

  BmMemberHandle bm_mt_indirect_add_member(const int32_t cxt_id, const std::string& table_name, const std::string& action_name, const BmActionData& action_data) {
    std::cout << "bm_mt_indirect_add_member" << std::endl
	      << table_name << std::endl
	      << action_name << std::endl;
    print_spec(action_data);
    return 0;
  }

  void bm_mt_indirect_delete_member(const int32_t cxt_id, const std::string& table_name, const BmMemberHandle mbr_handle) {
    std::cout << "bm_mt_indirect_delete_member" << std::endl
	      << table_name << std::endl
	      << mbr_handle << std::endl;
  }

  void bm_mt_indirect_modify_member(const int32_t cxt_id, const std::string& table_name, const BmMemberHandle mbr_handle, const std::string& action_name, const BmActionData& action_data) {
    std::cout << "bm_mt_indirect_modify_member" << std::endl
	      << table_name << std::endl
	      << mbr_handle << std::endl
	      << action_name << std::endl;
    print_spec(action_data);
  }

  BmEntryHandle bm_mt_indirect_add_entry(const int32_t cxt_id, const std::string& table_name, const BmMatchParams& match_key, const BmMemberHandle mbr_handle, const BmAddEntryOptions& options) {
    std::cout << "bm_mt_indirect_add_entry" << std::endl
	      << table_name << std::endl;
    for(const auto &p : match_key)
      print_match_param(p);
    std::cout << mbr_handle << std::endl;
    if(options.__isset.priority)
      std::cout << options.priority << std::endl;
    return 0;
  }

  void bm_mt_indirect_modify_entry(const int32_t cxt_id, const std::string& table_name, const BmEntryHandle entry_handle, const BmMemberHandle mbr_handle) {
    std::cout << "bm_mt_indirect_modify_entry" << std::endl
	      << table_name << std::endl
	      << entry_handle << std::endl
	      << mbr_handle << std::endl;
  }

  void bm_mt_indirect_delete_entry(const int32_t cxt_id, const std::string& table_name, const BmEntryHandle entry_handle) {
    std::cout << "bm_mt_indirect_delete_entry" << std::endl
	      << table_name << std::endl
	      << entry_handle << std::endl;
  }

  void bm_mt_indirect_set_entry_ttl(const int32_t cxt_id, const std::string& table_name, const BmEntryHandle entry_handle, const int32_t timeout_ms) {
    std::cout << "bm_mt_indirect_set_entry_ttl" << std::endl
	      << table_name << std::endl
	      << entry_handle << std::endl
	      << timeout_ms << std::endl;
  }

  void bm_mt_indirect_set_default_member(const int32_t cxt_id, const std::string& table_name, const BmMemberHandle mbr_handle) {
    std::cout << "bm_mt_indirect_set_default_member" << std::endl
	      << table_name << std::endl
	      << mbr_handle << std::endl;
  }

  BmGroupHandle bm_mt_indirect_ws_create_group(const int32_t cxt_id, const std::string& table_name) {
    // Your implementation goes here
    printf("bm_mt_indirect_ws_create_group\n");
    return 0;
  }

  void bm_mt_indirect_ws_delete_group(const int32_t cxt_id, const std::string& table_name, const BmGroupHandle grp_handle) {
    // Your implementation goes here
    printf("bm_mt_indirect_ws_delete_group\n");
  }

  void bm_mt_indirect_ws_add_member_to_group(const int32_t cxt_id, const std::string& table_name, const BmMemberHandle mbr_handle, const BmGroupHandle grp_handle) {
    // Your implementation goes here
    printf("bm_mt_indirect_ws_add_member_to_group\n");
  }

  void bm_mt_indirect_ws_remove_member_from_group(const int32_t cxt_id, const std::string& table_name, const BmMemberHandle mbr_handle, const BmGroupHandle grp_handle) {
    // Your implementation goes here
    printf("bm_mt_indirect_ws_remove_member_from_group\n");
  }

  BmEntryHandle bm_mt_indirect_ws_add_entry(const int32_t cxt_id, const std::string& table_name, const BmMatchParams& match_key, const BmGroupHandle grp_handle, const BmAddEntryOptions& options) {
    // Your implementation goes here
    printf("bm_mt_indirect_ws_add_entry\n");
    return 0;
  }

  void bm_mt_indirect_ws_modify_entry(const int32_t cxt_id, const std::string& table_name, const BmEntryHandle entry_handle, const BmGroupHandle grp_handle) {
    // Your implementation goes here
    printf("bm_mt_indirect_ws_modify_entry\n");
  }

  void bm_mt_indirect_ws_set_default_group(const int32_t cxt_id, const std::string& table_name, const BmGroupHandle grp_handle) {
    // Your implementation goes here
    printf("bm_mt_indirect_ws_set_default_group\n");
  }

  void bm_mt_read_counter(BmCounterValue& _return, const int32_t cxt_id, const std::string& table_name, const BmEntryHandle entry_handle) {
    std::cout << "bm_mt_read_counter" << std::endl
	      << table_name << std::endl
	      << entry_handle << std::endl;
  }

  void bm_mt_reset_counters(const int32_t cxt_id, const std::string& table_name) {
    std::cout << "bm_mt_reset_counters" << std::endl
	      << table_name << std::endl;
  }

  void bm_mt_write_counter(const int32_t cxt_id, const std::string& table_name, const BmEntryHandle entry_handle, const BmCounterValue& value) {
    std::cout << "bm_mt_write_counter" << std::endl
	      << table_name << std::endl
	      << entry_handle << std::endl
	      << value.bytes << " " << value.packets << std::endl;
  }

  void bm_mt_set_meter_rates(const int32_t cxt_id, const std::string& table_name, const BmEntryHandle entry_handle, const std::vector<BmMeterRateConfig> & rates) {
    std::cout << "bm_mt_set_meter_rates" << std::endl
              << table_name << std::endl
              << entry_handle << std::endl;
    for(const auto &rate : rates) {
      std::cout << rate.units_per_micros << " " << rate.burst_size << std::endl;
    }
  }

  void bm_mt_get_meter_rates(std::vector<BmMeterRateConfig> & _return, const int32_t cxt_id, const std::string& table_name, const BmEntryHandle entry_handle) {
    // Your implementation goes here
    printf("bm_mt_get_meter_rates\n");
  }

  void bm_mt_get_entries(std::vector<BmMtEntry> & _return, const int32_t cxt_id, const std::string& table_name) {
    // Your implementation goes here
    printf("bm_mt_get_entries\n");
  }

  void bm_mt_get_entry(BmMtEntry& _return, const int32_t cxt_id, const std::string& table_name, const BmEntryHandle entry_handle) {
    std::cout << "bm_mt_get_entry" << std::endl
              << table_name << std::endl
              << entry_handle << std::endl;
    _return = most_recent_entry;
  }

  void bm_mt_get_default_entry(BmActionEntry& _return, const int32_t cxt_id, const std::string& table_name) {
    // Your implementation goes here
    printf("bm_mt_get_default_entry\n");
  }

  void bm_mt_indirect_get_members(std::vector<BmMtIndirectMember> & _return, const int32_t cxt_id, const std::string& table_name) {
    // Your implementation goes here
    printf("bm_mt_indirect_get_members\n");
  }

  void bm_mt_indirect_get_member(BmMtIndirectMember& _return, const int32_t cxt_id, const std::string& table_name, const BmMemberHandle mbr_handle) {
    // Your implementation goes here
    printf("bm_mt_indirect_get_member\n");
  }

  void bm_mt_indirect_ws_get_groups(std::vector<BmMtIndirectWsGroup> & _return, const int32_t cxt_id, const std::string& table_name) {
    // Your implementation goes here
    printf("bm_mt_indirect_ws_get_groups\n");
  }

  void bm_mt_indirect_ws_get_group(BmMtIndirectWsGroup& _return, const int32_t cxt_id, const std::string& table_name, const BmGroupHandle grp_handle) {
    // Your implementation goes here
    printf("bm_mt_indirect_ws_get_group\n");
  }

  void bm_counter_read(BmCounterValue& _return, const int32_t cxt_id, const std::string& counter_name, const int32_t index) {
    std::cout << "bm_counter_read" << std::endl
	      << counter_name << std::endl
	      << index << std::endl;
  }

  void bm_counter_reset_all(const int32_t cxt_id, const std::string& counter_name) {
    std::cout << "bm_counter_reset_all" << std::endl
	      << counter_name << std::endl;
  }

  void bm_counter_write(const int32_t cxt_id, const std::string& counter_name, const int32_t index, const BmCounterValue& value) {
    std::cout << "bm_counter_write" << std::endl
	      << counter_name << std::endl
	      << index << std::endl
	      << value.bytes << " " << value.packets << std::endl;
  }

  void bm_learning_ack(const int32_t cxt_id, const BmLearningListId list_id, const BmLearningBufferId buffer_id, const std::vector<BmLearningSampleId> & sample_ids) {
    // Your implementation goes here
    printf("bm_learning_ack\n");
  }

  void bm_learning_ack_buffer(const int32_t cxt_id, const BmLearningListId list_id, const BmLearningBufferId buffer_id) {
    // Your implementation goes here
    printf("bm_learning_ack_buffer\n");
  }

  void bm_learning_set_timeout(const int32_t cxt_id, const BmLearningListId list_id, const int32_t timeout_ms) {
    std::cout << "bm_learning_set_timeout" << std::endl
              << list_id << std::endl
              << timeout_ms << std::endl;
  }

  void bm_learning_set_buffer_size(const int32_t cxt_id, const BmLearningListId list_id, const int32_t nb_samples) {
    // Your implementation goes here
    printf("bm_learning_set_buffer_size\n");
  }

  void bm_load_new_config(const std::string& config_str) {
    // Your implementation goes here
    printf("bm_load_new_config\n");
  }

  void bm_swap_configs() {
    // Your implementation goes here
    printf("bm_swap_configs\n");
  }

  void bm_meter_array_set_rates(const int32_t cxt_id, const std::string& meter_array_name, const std::vector<BmMeterRateConfig> & rates) {
    // Your implementation goes here
    printf("bm_meter_array_set_rates\n");
  }

  void bm_meter_set_rates(const int32_t cxt_id, const std::string& meter_array_name, const int32_t index, const std::vector<BmMeterRateConfig> & rates) {
    std::cout << "bm_meter_set_rates" << std::endl
	      << meter_array_name << std::endl
	      << index << std::endl;
    for(const auto &rate : rates) {
      std::cout << rate.units_per_micros << " " << rate.burst_size << std::endl;
    }
  }

  void bm_meter_get_rates(std::vector<BmMeterRateConfig> & _return, const int32_t cxt_id, const std::string& meter_array_name, const int32_t index) {
    // Your implementation goes here
    printf("bm_meter_get_rates\n");
  }

  BmRegisterValue bm_register_read(const int32_t cxt_id, const std::string& register_name, const int32_t index) {
    // Your implementation goes here
    printf("bm_register_read\n");
    return 0;
  }

  void bm_register_write(const int32_t cxt_id, const std::string& register_name, const int32_t index, const BmRegisterValue value) {
    // Your implementation goes here
    printf("bm_register_write\n");
  }

  void bm_register_write_range(const int32_t cxt_id, const std::string& register_array_name, const int32_t from, const int32_t to, const BmRegisterValue value) {
    // Your implementation goes here
    printf("bm_register_write_range\n");
  }

  void bm_register_reset(const int32_t cxt_id, const std::string& register_array_name) {
    std::cout << "bm_register_reset" << std::endl
              << register_array_name << std::endl;
  }

  void bm_parse_vset_add(const int32_t cxt_id, const std::string& parse_vset_name, const BmParseVSetValue& value) {
    // Your implementation goes here
    printf("bm_parse_vset_add\n");
  }

  void bm_parse_vset_remove(const int32_t cxt_id, const std::string& parse_vset_name, const BmParseVSetValue& value) {
    // Your implementation goes here
    printf("bm_parse_vset_remove\n");
  }

  void bm_dev_mgr_add_port(const std::string& iface_name, const int32_t port_num, const std::string& pcap_path) {
    // Your implementation goes here
    printf("bm_dev_mgr_add_port\n");
  }

  void bm_dev_mgr_remove_port(const int32_t port_num) {
    // Your implementation goes here
    printf("bm_dev_mgr_remove_port\n");
  }

  void bm_dev_mgr_show_ports(std::vector<DevMgrPortInfo> & _return) {
    // Your implementation goes here
    printf("bm_dev_mgr_show_ports\n");
  }

  void bm_mgmt_get_info(BmConfig& _return) {
    // Your implementation goes here
    printf("bm_mgmt_get_info\n");
  }

  void bm_set_crc16_custom_parameters(const int32_t cxt_id, const std::string& calc_name, const BmCrc16Config& crc16_config) {
    // Your implementation goes here
    printf("bm_set_crc16_custom_parameters\n");
  }

  void bm_set_crc32_custom_parameters(const int32_t cxt_id, const std::string& calc_name, const BmCrc32Config& crc32_config) {
    // Your implementation goes here
    printf("bm_set_crc32_custom_parameters\n");
  }

  void bm_reset_state() {
    // Your implementation goes here
    printf("bm_reset_state\n");
  }

  void bm_get_config(std::string& _return) {
    // Your implementation goes here
    printf("bm_get_config\n");
  }

  void bm_get_config_md5(std::string& _return) {
    // Your implementation goes here
    printf("bm_get_config_md5\n");
  }

  void bm_serialize_state(std::string& _return) {
    // Your implementation goes here
    printf("bm_serialize_state\n");
  }

};

} }
