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
#include <saihash.h>
#include <saistatus.h>

#include "saiinternal.h"
#include <switchapi/switch_hash.h>

// Unused for now
/* static sai_api_t api_id = SAI_API_HASH; */

uint32_t sai_field_to_switch_ipv4_hash_field_attribute_get(
    const sai_attribute_t *attribute) {
  uint32_t flags = 0;
  uint32_t index = 0;
  for (index = 0; index < attribute->value.s32list.count; index++) {
    switch (attribute->value.s32list.list[index]) {
      case SAI_NATIVE_HASH_FIELD_SRC_IP:
        flags |= SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_IP;
        break;
      case SAI_NATIVE_HASH_FIELD_DST_IP:
        flags |= SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_DST_IP;
        break;
      case SAI_NATIVE_HASH_FIELD_IP_PROTOCOL:
        flags |= SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_IP_PROTOCOL;
        break;
      case SAI_NATIVE_HASH_FIELD_L4_SRC_PORT:
        flags |= SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_PORT;
        break;
      case SAI_NATIVE_HASH_FIELD_L4_DST_PORT:
        flags |= SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_DST_PORT;
        break;
      default:
        break;
    }
  }
  return flags;
}

void switch_ipv4_hash_field_attribute_sai_field_get(
    uint32_t flags, sai_attribute_t *attribute) {
  uint32_t index = attribute->value.s32list.count;

  if (flags & SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_IP) {
    attribute->value.s32list.list[index] = SAI_NATIVE_HASH_FIELD_SRC_IP;
    index++;
  }

  if (flags & SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_DST_IP) {
    attribute->value.s32list.list[index] = SAI_NATIVE_HASH_FIELD_DST_IP;
    index++;
  }

  if (flags & SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_IP_PROTOCOL) {
    attribute->value.s32list.list[index] = SAI_NATIVE_HASH_FIELD_IP_PROTOCOL;
    index++;
  }

  if (flags & SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_IP_PROTOCOL) {
    attribute->value.s32list.list[index] = SAI_NATIVE_HASH_FIELD_IP_PROTOCOL;
    index++;
  }

  if (flags & SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_SRC_PORT) {
    attribute->value.s32list.list[index] = SAI_NATIVE_HASH_FIELD_L4_SRC_PORT;
    index++;
  }

  if (flags & SWITCH_HASH_IPV4_INPUT_FIELD_ATTRIBUTE_DST_PORT) {
    attribute->value.s32list.list[index] = SAI_NATIVE_HASH_FIELD_L4_DST_PORT;
    index++;
  }

  attribute->value.s32list.count = index;
  return;
}

uint32_t sai_field_to_switch_ipv6_hash_field_attribute_get(
    const sai_attribute_t *attribute) {
  uint32_t flags = 0;
  uint32_t index = 0;
  for (index = 0; index < attribute->value.s32list.count; index++) {
    switch (attribute->value.s32list.list[index]) {
      case SAI_NATIVE_HASH_FIELD_SRC_IP:
        flags |= SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_IP;
        break;
      case SAI_NATIVE_HASH_FIELD_DST_IP:
        flags |= SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_DST_IP;
        break;
      case SAI_NATIVE_HASH_FIELD_IP_PROTOCOL:
        flags |= SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_IP_PROTOCOL;
        break;
      case SAI_NATIVE_HASH_FIELD_L4_SRC_PORT:
        flags |= SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_PORT;
        break;
      case SAI_NATIVE_HASH_FIELD_L4_DST_PORT:
        flags |= SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_DST_PORT;
        break;
      default:
        break;
    }
  }
  return flags;
}

void switch_ipv6_hash_field_attribute_sai_field_get(
    uint32_t flags, sai_attribute_t *attribute) {
  uint32_t index = attribute->value.s32list.count;

  if (flags & SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_IP) {
    attribute->value.s32list.list[index] = SAI_NATIVE_HASH_FIELD_SRC_IP;
    index++;
  }

  if (flags & SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_DST_IP) {
    attribute->value.s32list.list[index] = SAI_NATIVE_HASH_FIELD_DST_IP;
    index++;
  }

  if (flags & SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_IP_PROTOCOL) {
    attribute->value.s32list.list[index] = SAI_NATIVE_HASH_FIELD_IP_PROTOCOL;
    index++;
  }

  if (flags & SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_IP_PROTOCOL) {
    attribute->value.s32list.list[index] = SAI_NATIVE_HASH_FIELD_IP_PROTOCOL;
    index++;
  }

  if (flags & SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_SRC_PORT) {
    attribute->value.s32list.list[index] = SAI_NATIVE_HASH_FIELD_L4_SRC_PORT;
    index++;
  }

  if (flags & SWITCH_HASH_IPV6_INPUT_FIELD_ATTRIBUTE_DST_PORT) {
    attribute->value.s32list.list[index] = SAI_NATIVE_HASH_FIELD_L4_DST_PORT;
    index++;
  }

  attribute->value.s32list.count = index;
  return;
}

uint32_t sai_field_to_switch_non_ip_hash_field_attribute_get(
    const sai_attribute_t *attribute) {
  uint32_t flags = 0;
  uint32_t index = 0;
  for (index = 0; index < attribute->value.s32list.count; index++) {
    switch (attribute->value.s32list.list[index]) {
      case SAI_NATIVE_HASH_FIELD_SRC_MAC:
        flags |= SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_SRC_MAC;
        break;
      case SAI_NATIVE_HASH_FIELD_DST_MAC:
        flags |= SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_DST_MAC;
        break;
      case SAI_NATIVE_HASH_FIELD_ETHERTYPE:
        flags |= SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_ETYPE;
        break;
      default:
        break;
    }
  }
  return flags;
}

void switch_non_ip_hash_field_attribute_sai_field_get(
    uint32_t flags, sai_attribute_t *attribute) {
  uint32_t index = attribute->value.s32list.count;

  if (flags & SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_SRC_MAC) {
    attribute->value.s32list.list[index] = SAI_NATIVE_HASH_FIELD_SRC_MAC;
    index++;
  }

  if (flags & SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_DST_MAC) {
    attribute->value.s32list.list[index] = SAI_NATIVE_HASH_FIELD_DST_MAC;
    index++;
  }

  if (flags & SWITCH_HASH_NON_IP_INPUT_FIELD_ATTRIBUTE_ETYPE) {
    attribute->value.s32list.list[index] = SAI_NATIVE_HASH_FIELD_ETHERTYPE;
    index++;
  }

  attribute->value.s32list.count = index;
  return;
}

sai_status_t sai_create_hash(_Out_ sai_object_id_t *hash_id,
                             _In_ sai_object_id_t switch_id,
                             _In_ uint32_t attr_count,
                             _In_ const sai_attribute_t *attr_list) {
  return SAI_STATUS_NOT_SUPPORTED;
}

sai_status_t sai_remove_hash(_In_ sai_object_id_t hash_id) {
  return SAI_STATUS_NOT_SUPPORTED;
}

sai_status_t sai_set_hash_attribute(_In_ sai_object_id_t hash_id,
                                    _In_ const sai_attribute_t *attr) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  uint32_t ipv4_flags = 0;
  uint32_t ipv6_flags = 0;
  uint32_t nonip_flags = 0;
  switch_hash_ipv6_input_fields_t ipv6_fields = 0;
  switch_hash_ipv4_input_fields_t ipv4_fields = 0;
  switch_hash_non_ip_input_fields_t nonip_fields = 0;
  switch (attr->id) {
    case SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST:
      ipv4_flags = sai_field_to_switch_ipv4_hash_field_attribute_get(attr);
      ipv6_flags = sai_field_to_switch_ipv6_hash_field_attribute_get(attr);
      nonip_flags = sai_field_to_switch_non_ip_hash_field_attribute_get(attr);
      switch_api_ipv4_hash_input_fields_get(device, &ipv4_fields);
      switch_api_ipv6_hash_input_fields_get(device, &ipv6_fields);
      switch_api_non_ip_hash_input_fields_get(device, &nonip_fields);
      if (nonip_flags) {
        switch_api_non_ip_hash_input_fields_attribute_set(
            device, nonip_fields, nonip_flags);
      }
      if (ipv4_flags) {
        switch_api_ipv4_hash_input_fields_attribute_set(
            device, ipv4_fields, ipv4_flags);
      }
      if (ipv6_flags) {
        switch_api_ipv6_hash_input_fields_attribute_set(
            device, ipv6_fields, ipv6_flags);
      }
      break;
    default:
      status = SAI_STATUS_NOT_SUPPORTED;
      break;
  }

  return status;
}

sai_status_t sai_get_hash_attribute(_In_ sai_object_id_t hash_id,
                                    _In_ uint32_t attr_count,
                                    _Inout_ sai_attribute_t *attr_list) {
  sai_attribute_t *attribute = NULL;
  sai_status_t status = SAI_STATUS_SUCCESS;
  uint32_t index = 0;
  uint32_t ipv4_flags = 0;
  uint32_t nonip_flags = 0;
  switch_hash_ipv4_input_fields_t ipv4_fields = 0;
  switch_hash_non_ip_input_fields_t nonip_fields = 0;

  for (index = 0; index < attr_count; index++) {
    attribute = &attr_list[index];
    switch (attribute->id) {
      case SAI_HASH_ATTR_NATIVE_HASH_FIELD_LIST:
        switch_api_ipv4_hash_input_fields_get(device, &ipv4_fields);
        switch_api_non_ip_hash_input_fields_get(device, &nonip_fields);
        switch_api_ipv4_hash_input_field_attribute_get(
            device, ipv4_fields, &ipv4_flags);
        switch_api_non_ip_hash_input_field_attribute_get(
            device, ipv4_fields, &nonip_flags);
        attribute->value.s32list.count = 0;
        if (ipv4_flags) {
          switch_ipv4_hash_field_attribute_sai_field_get(ipv4_flags, attribute);
        }
        if (nonip_flags) {
          switch_non_ip_hash_field_attribute_sai_field_get(ipv4_flags,
                                                           attribute);
        }
        break;
      default:
        status = SAI_STATUS_NOT_SUPPORTED;
    }
  }

  return status;
}

sai_hash_api_t hash_api = {
    .create_hash = sai_create_hash,
    .remove_hash = sai_remove_hash,
    .set_hash_attribute = sai_set_hash_attribute,
    .get_hash_attribute = sai_get_hash_attribute,
};

sai_status_t sai_hash_initialize(sai_api_service_t *sai_api_service) {
  sai_api_service->hash_api = hash_api;
  return SAI_STATUS_SUCCESS;
}
