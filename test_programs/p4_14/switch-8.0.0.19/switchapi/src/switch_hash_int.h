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

#ifndef _switch_hash_int_h_
#define _switch_hash_int_h_

#include <switchapi/switch_hash.h>
#include "switch_pd_types.h"

#define SWITCH_HASH_SIZE 16

#define switch_hash_handle_create(_device) \
  switch_handle_create(                    \
      _device, SWITCH_HANDLE_TYPE_HASH, sizeof(switch_hash_info_t))

#define switch_hash_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_HASH, _handle)

#define switch_hash_get(_device, _handle, _info) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_HASH, _handle, (void **)_info)

#define SWITCH_HASH_TYPE_IS_VALID_IPV6_INPUT_FIELDS(input) \
  ((input == SWITCH_HASH_IPV6_INPUT_SIP_DIP_PROT_SP_DP) || \
   (input == SWITCH_HASH_IPV6_INPUT_PROT_DP_SIP_SP_DIP))

#define SWITCH_HASH_TYPE_IS_VALID_IPV4_INPUT_FIELDS(input) \
  ((input == SWITCH_HASH_IPV4_INPUT_SIP_DIP_PROT_SP_DP) || \
   (input == SWITCH_HASH_IPV4_INPUT_PROT_DP_DIP_SP_SIP))

#define SWITCH_HASH_TYPE_IS_VALID_NON_IP_INPUT_FIELDS(input) \
  ((input == SWITCH_HASH_NON_IP_INPUT_IF_SMAC_DMAC_ETYPE) || \
   (input == SWITCH_HASH_NON_IP_INPUT_ETYPE_SMAC_IF_DMAC))

#define SWITCH_HASH_TYPE_IS_VALID_IPV6_ALGORITHM(algorithm)          \
  ((algorithm == SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16) ||          \
   (algorithm == SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_DECT) ||     \
   (algorithm == SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_GENIBUS) ||  \
   (algorithm == SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_DNP) ||      \
   (algorithm == SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_TELEDISK) || \
   (algorithm == SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_CUSTOM) ||   \
   (algorithm == SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC32) ||          \
   (algorithm == SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC32_BZIP2) ||    \
   (algorithm == SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC32_C) ||        \
   (algorithm == SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC32_D))

#define SWITCH_HASH_TYPE_IS_VALID_IPV4_ALGORITHM(algorithm)          \
  ((algorithm == SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16) ||          \
   (algorithm == SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_DECT) ||     \
   (algorithm == SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_GENIBUS) ||  \
   (algorithm == SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_DNP) ||      \
   (algorithm == SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_TELEDISK) || \
   (algorithm == SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_CUSTOM) ||   \
   (algorithm == SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC32) ||          \
   (algorithm == SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC32_BZIP2) ||    \
   (algorithm == SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC32_C) ||        \
   (algorithm == SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC32_D))

#define SWITCH_HASH_TYPE_IS_VALID_NON_IP_ALGORITHM(algorithm)          \
  ((algorithm == SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16) ||          \
   (algorithm == SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_DECT) ||     \
   (algorithm == SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_GENIBUS) ||  \
   (algorithm == SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_DNP) ||      \
   (algorithm == SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_TELEDISK) || \
   (algorithm == SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC32) ||          \
   (algorithm == SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC32_BZIP2) ||    \
   (algorithm == SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC32_C) ||        \
   (algorithm == SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC32_D))

#define SWITCH_HASH_TYPE_IS_VALID_IPV6_INPUT_FIELDS_ATTRIBUTE(attr_flags) \
  !(attr_flags &                                                          \
    ~(SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_IP |                     \
      SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_DST_IP |                     \
      SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_IP_PROTOCOL |                \
      SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_PORT |                   \
      SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_DST_PORT))

#define SWITCH_HASH_TYPE_IS_VALID_IPV4_INPUT_FIELDS_ATTRIBUTE(attr_flags) \
  !(attr_flags &                                                          \
    ~(SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_IP |                     \
      SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_DST_IP |                     \
      SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_IP_PROTOCOL |                \
      SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_PORT |                   \
      SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_DST_PORT))

#define SWITCH_HASH_TYPE_IS_VALID_NON_IP_INPUT_FIELDS_ATTRIBUTE(attr_flags) \
  !(attr_flags &                                                            \
    ~(SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_SRC_MAC |                    \
      SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_DST_MAC |                    \
      SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_ETYPE |                      \
      SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_IF_INDEX))

static inline char *switch_hash_ipv6_input_field_to_string(
    switch_hash_ipv6_input_fields_t input_field) {
  switch (input_field) {
    case SWITCH_HASH_IPV6_INPUT_SIP_DIP_PROT_SP_DP:
      return "SWITCH_HASH_IPV6_INPUT_SIP_DIP_PROT_SP_DP";
    case SWITCH_HASH_IPV6_INPUT_PROT_DP_SIP_SP_DIP:
      return "SWITCH_HASH_IPV6_INPUT_PROT_DP_SIP_SP_DIP";
    default:
      return "INVALID";
  }
}

static inline char *switch_hash_ipv4_input_field_to_string(
    switch_hash_ipv4_input_fields_t input_field) {
  switch (input_field) {
    case SWITCH_HASH_IPV4_INPUT_SIP_DIP_PROT_SP_DP:
      return "SWITCH_HASH_IPV4_INPUT_SIP_DIP_PROT_SP_DP";
    case SWITCH_HASH_IPV4_INPUT_PROT_DP_DIP_SP_SIP:
      return "SWITCH_HASH_IPV4_INPUT_PROT_DP_DIP_SP_SIP";
    default:
      return "INVALID";
  }
}

static inline char *switch_hash_non_ip_input_field_to_string(
    switch_hash_non_ip_input_fields_t input_field) {
  switch (input_field) {
    case SWITCH_HASH_NON_IP_INPUT_IF_SMAC_DMAC_ETYPE:
      return "SWITCH_HASH_NON_IP_INPUT_IF_SMAC_DMAC_ETYPE";
    case SWITCH_HASH_NON_IP_INPUT_ETYPE_SMAC_IF_DMAC:
      return "SWITCH_HASH_NON_IP_INPUT_ETYPE_SMAC_IF_DMAC";
    default:
      return "INVALID";
  }
}

static inline char *switch_hash_ipv6_input_field_attribute_to_string(
    switch_hash_ipv6_input_field_attribute_t attr_mask) {
  switch (attr_mask) {
    case SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_IP:
      return "SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_IP";
    case SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_DST_IP:
      return "SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_DST_IP";
    case SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_IP_PROTOCOL:
      return "SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_IP_PROTOCOL";
    case SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_PORT:
      return "SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_PORT";
    case SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_DST_PORT:
      return "SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_DST_PORT";
    default:
      return "INVALID";
  }
}

static inline char *switch_hash_ipv4_input_field_attribute_to_string(
    switch_hash_ipv4_input_field_attribute_t attr_mask) {
  switch (attr_mask) {
    case SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_IP:
      return "SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_IP";
    case SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_DST_IP:
      return "SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_DST_IP";
    case SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_IP_PROTOCOL:
      return "SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_IP_PROTOCOL";
    case SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_PORT:
      return "SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_PORT";
    case SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_DST_PORT:
      return "SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_DST_PORT";
    default:
      return "INVALID";
  }
}

static inline char *switch_hash_non_ip_input_field_attribute_to_string(
    switch_hash_non_ip_input_field_attribute_t attr_mask) {
  switch (attr_mask) {
    case SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_SRC_MAC:
      return "SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_SRC_MAC";
    case SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_DST_MAC:
      return "SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_DST_MAC";
    case SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_ETYPE:
      return "SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_ETYPE";
    case SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_IF_INDEX:
      return "SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_IF_INDEX";
    default:
      return "INVALID";
  }
}

static inline char *switch_hash_ipv6_algorithm_to_string(
    switch_hash_ipv6_algorithm_t algorithm) {
  switch (algorithm) {
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16:
      return "SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16";
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_DECT:
      return "SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_DECT";
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_GENIBUS:
      return "SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_GENIBUS";
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_DNP:
      return "SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_DNP";
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_TELEDISK:
      return "SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_TELEDISK";
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_CUSTOM:
      return "SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_CUSTOM";
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC32:
      return "SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC32";
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC32_BZIP2:
      return "SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC32_BZIP2";
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC32_C:
      return "SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC32_C";
    case SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC32_D:
      return "SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC32_D";
    default:
      return "INVALID";
  }
}

static inline char *switch_hash_ipv4_algorithm_to_string(
    switch_hash_ipv4_algorithm_t algorithm) {
  switch (algorithm) {
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16:
      return "SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16";
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_DECT:
      return "SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_DECT";
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_GENIBUS:
      return "SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_GENIBUS";
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_DNP:
      return "SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_DNP";
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_TELEDISK:
      return "SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_TELEDISK";
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_CUSTOM:
      return "SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_CUSTOM";
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC32:
      return "SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC32";
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC32_BZIP2:
      return "SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC32_BZIP2";
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC32_C:
      return "SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC32_C";
    case SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC32_D:
      return "SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC32_D";
    default:
      return "INVALID";
  }
}

static inline char *switch_hash_non_ip_algorithm_to_string(
    switch_hash_non_ip_algorithm_t algorithm) {
  switch (algorithm) {
    case SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16:
      return "SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16";
    case SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_DECT:
      return "SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_DECT";
    case SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_GENIBUS:
      return "SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_GENIBUS";
    case SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_DNP:
      return "SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_DNP";
    case SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_TELEDISK:
      return "SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_TELEDISK";
    case SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC32:
      return "SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC32";
    case SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC32_BZIP2:
      return "SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC32_BZIP2";
    case SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC32_C:
      return "SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC32_C";
    case SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC32_D:
      return "SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC32_D";
    default:
      return "INVALID";
  }
}

switch_status_t switch_hash_default_entries_add(switch_device_t device);

switch_status_t switch_hash_default_entries_delete(switch_device_t device);

#endif /* _switch_hash_int_h */
