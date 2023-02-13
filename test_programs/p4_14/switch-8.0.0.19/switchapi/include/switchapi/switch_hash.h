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

#ifndef _switch_hash_h_
#define _switch_hash_h_

#include "switch_base_types.h"
#include "switch_handle.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum switch_hash_ipv6_input_fields_ {
  SWITCH_HASH_IPV6_INPUT_SIP_DIP_PROT_SP_DP = 0,
  SWITCH_HASH_IPV6_INPUT_PROT_DP_SIP_SP_DIP = 1,
} switch_hash_ipv6_input_fields_t;

typedef enum switch_hash_ipv4_input_fields_ {
  SWITCH_HASH_IPV4_INPUT_SIP_DIP_PROT_SP_DP = 0,
  SWITCH_HASH_IPV4_INPUT_PROT_DP_DIP_SP_SIP = 1,
} switch_hash_ipv4_input_fields_t;

typedef enum switch_hash_non_ip_input_fields_ {
  SWITCH_HASH_NON_IP_INPUT_IF_SMAC_DMAC_ETYPE = 0,
  SWITCH_HASH_NON_IP_INPUT_ETYPE_SMAC_IF_DMAC = 1,
} switch_hash_non_ip_input_fields_t;

typedef enum switch_hash_ipv6_input_field_attribute_ {
  SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_IP = (1 << 0),
  SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_DST_IP = (1 << 1),
  SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_IP_PROTOCOL = (1 << 2),
  SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_PORT = (1 << 3),
  SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_DST_PORT = (1 << 4),
} switch_hash_ipv6_input_field_attribute_t;

typedef enum switch_hash_ipv4_input_field_attribute_ {
  SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_IP = (1 << 0),
  SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_DST_IP = (1 << 1),
  SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_IP_PROTOCOL = (1 << 2),
  SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_PORT = (1 << 3),
  SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_DST_PORT = (1 << 4),
} switch_hash_ipv4_input_field_attribute_t;

typedef enum switch_hash_non_ip_input_field_attribute_ {
  SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_SRC_MAC = (1 << 0),
  SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_DST_MAC = (1 << 1),
  SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_ETYPE = (1 << 2),
  SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_IF_INDEX = (1 << 3),
} switch_hash_non_ip_input_field_attribute_t;

typedef enum switch_hash_ipv4_input_algorithm_ {
  SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16 = 0x1,
  SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_DECT = 0x2,
  SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_GENIBUS = 0x3,
  SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_DNP = 0x4,
  SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_TELEDISK = 0x5,
  SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC16_CUSTOM = 0x6,
  SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC32 = 0x7,
  SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC32_BZIP2 = 0x8,
  SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC32_C = 0x9,
  SWITCH_HASH_IPV4_INPUT_ALGORITHM_CRC32_D = 0xA,
} switch_hash_ipv4_algorithm_t;

typedef enum switch_hash_ipv6_algorithm_ {
  SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16 = 0x1,
  SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_DECT = 0x2,
  SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_GENIBUS = 0x3,
  SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_DNP = 0x4,
  SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_TELEDISK = 0x5,
  SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC16_CUSTOM = 0x6,
  SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC32 = 0x7,
  SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC32_BZIP2 = 0x8,
  SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC32_C = 0x9,
  SWITCH_HASH_IPV6_INPUT_ALGORITHM_CRC32_D = 0xA,
} switch_hash_ipv6_algorithm_t;

typedef enum switch_hash_non_ip_input_algorithm_ {
  SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16 = 0x1,
  SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_DECT = 0x2,
  SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_GENIBUS = 0x3,
  SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_DNP = 0x4,
  SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC16_TELEDISK = 0x5,
  SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC32 = 0x6,
  SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC32_BZIP2 = 0x7,
  SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC32_C = 0x8,
  SWITCH_HASH_NON_IP_INPUT_ALGORITHM_CRC32_D = 0x9,
} switch_hash_non_ip_algorithm_t;

switch_status_t switch_api_ipv6_hash_input_fields_set(
    switch_device_t device, switch_hash_ipv6_input_fields_t input);

switch_status_t switch_api_ipv4_hash_input_fields_set(
    switch_device_t device, switch_hash_ipv4_input_fields_t input);

switch_status_t switch_api_non_ip_hash_input_fields_set(
    switch_device_t device, switch_hash_non_ip_input_fields_t input);

switch_status_t switch_api_ipv6_hash_input_fields_attribute_set(
    switch_device_t device,
    switch_hash_ipv6_input_fields_t input,
    switch_uint32_t attr_flags);

switch_status_t switch_api_ipv4_hash_input_fields_attribute_set(
    switch_device_t device,
    switch_hash_ipv4_input_fields_t input,
    switch_uint32_t attr_flags);

switch_status_t switch_api_non_ip_hash_input_fields_attribute_set(
    switch_device_t device,
    switch_hash_non_ip_input_fields_t input,
    switch_uint32_t attr_flags);

switch_status_t switch_api_ipv6_hash_algorithm_set(
    switch_device_t device, switch_hash_ipv6_algorithm_t algorithm);

switch_status_t switch_api_ipv4_hash_algorithm_set(
    switch_device_t device, switch_hash_ipv4_algorithm_t algorithm);

switch_status_t switch_api_non_ip_hash_algorithm_set(
    switch_device_t device, switch_hash_non_ip_algorithm_t algorithm);

switch_status_t switch_api_ipv6_hash_seed_set(switch_device_t device,
                                              uint64_t seed);

switch_status_t switch_api_ipv4_hash_seed_set(switch_device_t device,
                                              uint64_t seed);

switch_status_t switch_api_non_ip_hash_seed_set(switch_device_t device,
                                                uint64_t seed);

switch_status_t switch_api_ipv6_hash_input_fields_get(
    switch_device_t device, switch_hash_ipv6_input_fields_t *fields);

switch_status_t switch_api_ipv4_hash_input_fields_get(
    switch_device_t device, switch_hash_ipv4_input_fields_t *fields);

switch_status_t switch_api_non_ip_hash_input_fields_get(
    switch_device_t device, switch_hash_non_ip_input_fields_t *fields);

switch_status_t switch_api_ipv6_hash_input_field_attribute_get(
    switch_device_t device,
    switch_hash_ipv6_input_fields_t input,
    switch_uint32_t *attr_flags);

switch_status_t switch_api_ipv4_hash_input_field_attribute_get(
    switch_device_t device,
    switch_hash_ipv4_input_fields_t input,
    switch_uint32_t *attr_flags);

switch_status_t switch_api_non_ip_hash_input_field_attribute_get(
    switch_device_t device,
    switch_hash_non_ip_input_fields_t input,
    switch_uint32_t *attr_flags);

switch_status_t switch_api_ipv6_hash_algorithm_get(
    switch_device_t device, switch_hash_ipv6_algorithm_t *algorithm);

switch_status_t switch_api_ipv4_hash_algorithm_get(
    switch_device_t device, switch_hash_ipv4_algorithm_t *algorithm);

switch_status_t switch_api_non_ip_hash_algorithm_get(
    switch_device_t device, switch_hash_non_ip_algorithm_t *algorithm);

switch_status_t switch_api_ipv6_hash_seed_get(switch_device_t device,
                                              uint64_t *seed);

switch_status_t switch_api_ipv4_hash_seed_get(switch_device_t device,
                                              uint64_t *seed);

switch_status_t switch_api_non_ip_hash_seed_get(switch_device_t device,
                                                uint64_t *seed);

switch_status_t switch_api_lag_hash_seed_set(switch_device_t device,
                                             uint64_t seed);
switch_status_t switch_api_lag_hash_seed_get(switch_device_t device,
                                             uint64_t *seed);
switch_status_t switch_api_ecmp_hash_seed_set(switch_device_t device,
                                              uint64_t seed);
switch_status_t switch_api_ecmp_hash_seed_get(switch_device_t device,
                                              uint64_t *seed);
#ifdef __cplusplus
}
#endif

#endif
