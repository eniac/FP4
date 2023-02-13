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

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_hash_default_entries_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

  return status;
}

switch_status_t switch_hash_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

  return status;
}

switch_status_t switch_api_ipv6_hash_input_fields_set_internal(
    switch_device_t device, switch_hash_ipv6_input_fields_t input) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_LOG_ENTER();
  if (!SWITCH_HASH_TYPE_IS_VALID_IPV6_INPUT_FIELDS(input)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hash ipv6 input fields set failed for device %d, field list %d "
        "invalid "
        "(%s)",
        device,
        input,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_ipv6_hash_input_fields_set(device, input);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ipv6 hash input field list set failed on device %d: (%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  SWITCH_LOG_DEBUG(
      "ipv6 hash input field successfully set on device %d "
      "input field %s (0x%d)",
      device,
      switch_hash_ipv6_input_field_to_string(input),
      input);

  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_ipv4_hash_input_fields_set_internal(
    switch_device_t device, switch_hash_ipv4_input_fields_t input) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_LOG_ENTER();
  if (!SWITCH_HASH_TYPE_IS_VALID_IPV4_INPUT_FIELDS(input)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hash ipv4 input fields set failed for device %d, field list %d "
        "invalid "
        "(%s)",
        device,
        input,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_ipv4_hash_input_fields_set(device, input);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ipv4 hash input field list set failed on device %d: (%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  SWITCH_LOG_DEBUG(
      "ipv4 hash input field successfully set on device %d "
      "input field %s (0x%d)",
      device,
      switch_hash_ipv4_input_field_to_string(input),
      input);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_non_ip_hash_input_fields_set_internal(
    switch_device_t device, switch_hash_non_ip_input_fields_t input) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_LOG_ENTER();
  if (!SWITCH_HASH_TYPE_IS_VALID_NON_IP_INPUT_FIELDS(input)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hash non ip input fields set failed for device %d, field list %d "
        "invalid "
        "(%s)",
        device,
        input,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_non_ip_hash_input_fields_set(device, input);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "non ip hash input field list set failed on device %d: (%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  SWITCH_LOG_DEBUG(
      "non ip hash input field successfully set on device %d "
      "input field %s (0x%d)",
      device,
      switch_hash_non_ip_input_field_to_string(input),
      input);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_ipv6_hash_input_fields_attribute_set_internal(
    switch_device_t device,
    switch_hash_ipv6_input_fields_t input,
    switch_uint32_t attr_flags) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_LOG_ENTER();
  if (!SWITCH_HASH_TYPE_IS_VALID_IPV6_INPUT_FIELDS(input)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "ipv6 hash input fields set failed for device %d, input field %d "
        "invalid "
        "(%s)",
        device,
        input,
        switch_error_to_string(status));
    return status;
  }

  if (!SWITCH_HASH_TYPE_IS_VALID_IPV6_INPUT_FIELDS_ATTRIBUTE(attr_flags)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "ipv6 hash input field attribute set failed for device %d, attribute "
        "flags %x invalid "
        "(%s)",
        device,
        attr_flags,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_pd_ipv6_hash_input_fields_attribute_set(device, input, attr_flags);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ipv6 hash input field attribute set failed on device %d: (%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  SWITCH_LOG_DEBUG(
      "ipv6 hash input field attribute successfully set on device %d "
      "input field %s (0x%d)",
      "attr flags %x",
      device,
      switch_hash_ipv6_input_field_to_string(input),
      input,
      attr_flags);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_ipv4_hash_input_fields_attribute_set_internal(
    switch_device_t device,
    switch_hash_ipv4_input_fields_t input,
    switch_uint32_t attr_flags) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_LOG_ENTER();
  if (!SWITCH_HASH_TYPE_IS_VALID_IPV4_INPUT_FIELDS(input)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "ipv4 hash input fields set failed for device %d, input field %d "
        "invalid "
        "(%s)",
        device,
        input,
        switch_error_to_string(status));
    return status;
  }

  if (!SWITCH_HASH_TYPE_IS_VALID_IPV4_INPUT_FIELDS_ATTRIBUTE(attr_flags)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "ipv4 hash input field attribute set failed for device %d, attribute "
        "flags %x invalid "
        "(%s)",
        device,
        attr_flags,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_pd_ipv4_hash_input_fields_attribute_set(device, input, attr_flags);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ipv4 hash input field attribute set failed on device %d: (%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  SWITCH_LOG_DEBUG(
      "ipv4 hash input field attribute successfully set on device %d "
      "input field %s (0x%d)",
      "attr flags %x",
      device,
      switch_hash_ipv4_input_field_to_string(input),
      input,
      attr_flags);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_non_ip_hash_input_fields_attribute_set_internal(
    switch_device_t device,
    switch_hash_non_ip_input_fields_t input,
    switch_uint32_t attr_flags) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_LOG_ENTER();
  if (!SWITCH_HASH_TYPE_IS_VALID_NON_IP_INPUT_FIELDS(input)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "non ip hash input fields set failed for device %d, input field %d "
        "invalid "
        "(%s)",
        device,
        input,
        switch_error_to_string(status));
    return status;
  }

  if (!SWITCH_HASH_TYPE_IS_VALID_NON_IP_INPUT_FIELDS_ATTRIBUTE(attr_flags)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "non ip hash input field attribute set failed for device %d, attribute "
        "flags %x invalid "
        "(%s)",
        device,
        attr_flags,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_non_ip_hash_input_fields_attribute_set(
      device, input, attr_flags);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "non ip hash input field attribute set failed on device %d: (%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  SWITCH_LOG_DEBUG(
      "non ip hash input field attribute successfully set on device %d "
      "input field %s (0x%d)",
      "attr flags %x",
      device,
      switch_hash_non_ip_input_field_to_string(input),
      input,
      attr_flags);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_ipv6_hash_algorithm_set_internal(
    switch_device_t device, switch_hash_ipv6_algorithm_t algorithm) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_LOG_ENTER();
  if (!SWITCH_HASH_TYPE_IS_VALID_IPV6_ALGORITHM(algorithm)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hash ipv6 input algorithm set failed for device %d, algorithm %d "
        "invalid (%s)",
        device,
        algorithm,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_ipv6_hash_algorithm_set(device, algorithm);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ipv6 hash algorithm set failed on device %d: (%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "ipv6 hash alogrithm successfully set on device %d "
      "algorithm %s (0x%d)",
      device,
      switch_hash_ipv6_algorithm_to_string(algorithm),
      algorithm);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_ipv4_hash_algorithm_set_internal(
    switch_device_t device, switch_hash_ipv4_algorithm_t algorithm) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_LOG_ENTER();
  if (!SWITCH_HASH_TYPE_IS_VALID_IPV4_ALGORITHM(algorithm)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hash ipv4 input algorithm set failed for device %d, algorithm %d "
        "invalid (%s)",
        device,
        algorithm,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_ipv4_hash_algorithm_set(device, algorithm);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ipv4 hash algorithm set failed on device %d: (%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "ipv4 hash alogrithm successfully set on device %d "
      "algorithm %s (0x%d)",
      device,
      switch_hash_ipv4_algorithm_to_string(algorithm),
      algorithm);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_non_ip_hash_algorithm_set_internal(
    switch_device_t device, switch_hash_non_ip_algorithm_t algorithm) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_LOG_ENTER();
  if (!SWITCH_HASH_TYPE_IS_VALID_NON_IP_ALGORITHM(algorithm)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hash non ip input algorithm set failed for device %d, algorithm %d "
        "invalid (%s)",
        device,
        algorithm,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_non_ip_hash_algorithm_set(device, algorithm);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("non ip hash algorithm set failed on device %d: (%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "non ip hash alogrithm successfully set on device %d "
      "algorithm %s (0x%d)",
      device,
      switch_hash_non_ip_algorithm_to_string(algorithm),
      algorithm);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_ipv6_hash_seed_set_internal(switch_device_t device,
                                                       uint64_t seed) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_LOG_ENTER();
  status = switch_pd_ipv6_hash_seed_set(device, seed);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ipv6 hash seed set failed on device %d: (%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "ipv6 hash seed successfully set on device %d "
      "seed (0x%lu)",
      device,
      seed);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_ipv4_hash_seed_set_internal(switch_device_t device,
                                                       uint64_t seed) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_LOG_ENTER();
  status = switch_pd_ipv4_hash_seed_set(device, seed);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ipv4 hash seed set failed on device %d: (%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "ipv4 hash seed successfully set on device %d "
      "seed (0x%lu)",
      device,
      seed);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_non_ip_hash_seed_set_internal(switch_device_t device,
                                                         uint64_t seed) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_LOG_ENTER();
  status = switch_pd_non_ip_hash_seed_set(device, seed);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("non ip hash seed set failed on device %d: (%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "non ip hash seed successfully set on device %d "
      "seed (0x%lu)",
      device,
      seed);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_lag_hash_seed_set_internal(switch_device_t device,
                                                      uint64_t seed) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_LOG_ENTER();
  status = switch_pd_lag_hash_seed_set(device, seed);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag hash seed set failed on device %d: (%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "lag hash seed successfully set on device %d "
      "seed (0x%lu)",
      device,
      seed);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_ecmp_hash_seed_set_internal(switch_device_t device,
                                                       uint64_t seed) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_LOG_ENTER();
  status = switch_pd_ecmp_hash_seed_set(device, seed);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ecmp hash seed set failed on device %d: (%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "ecmp hash seed successfully set on device %d "
      "seed (0x%lu)",
      device,
      seed);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_ipv6_hash_input_fields_get_internal(
    switch_device_t device, switch_hash_ipv6_input_fields_t *input) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_LOG_ENTER();
  if (!input) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hash ipv6 input fields get failed for device %d, input is %p (%s)",
        device,
        input,
        switch_error_to_string(status));
    return status;
  }
  status = switch_pd_ipv6_hash_input_fields_get(device, input);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ipv6 hash input fields get failed on device %d: (%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }
  SWITCH_LOG_DEBUG(
      "ipv6 hash input fields get successful on device %d "
      "input field %s (0x%d)",
      device,
      switch_hash_ipv6_input_field_to_string(*input),
      *input);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_ipv4_hash_input_fields_get_internal(
    switch_device_t device, switch_hash_ipv4_input_fields_t *input) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_LOG_ENTER();
  if (!input) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hash ipv4 input fields get failed for device %d, input is %p (%s)",
        device,
        input,
        switch_error_to_string(status));
    return status;
  }
  status = switch_pd_ipv4_hash_input_fields_get(device, input);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ipv4 hash input fields get failed on device %d: (%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "ipv4 hash input fields get successful on device %d "
      "input field %s (0x%d)",
      device,
      switch_hash_ipv4_input_field_to_string(*input),
      *input);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_non_ip_hash_input_fields_get_internal(
    switch_device_t device, switch_hash_non_ip_input_fields_t *input) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_LOG_ENTER();
  if (!input) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "non ip hash input fields get failed for device %d, input is %p (%s)",
        device,
        input,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_non_ip_hash_input_fields_get(device, input);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("non ip hash input fields get failed on device %d: (%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "non ip hash input fields get successful on device %d "
      "input field %s (0x%d)",
      device,
      switch_hash_non_ip_input_field_to_string(*input),
      *input);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_ipv6_hash_input_field_attribute_get_internal(
    switch_device_t device,
    switch_hash_ipv6_input_fields_t input,
    switch_uint32_t *attr_flags) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  if (!SWITCH_HASH_TYPE_IS_VALID_IPV6_INPUT_FIELDS(input)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hash ipv6 input fields get failed for device %d, field list %d "
        "invalid "
        "(%s)",
        device,
        input,
        switch_error_to_string(status));
    return status;
  }

  if (!attr_flags) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "ipv6 hash fields attribute get failed for device %d, attribute flags "
        "is %p (%s)",
        device,
        attr_flags,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_pd_ipv6_hash_input_fields_attribute_get(device, input, attr_flags);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ipv6 hash input field attribute get failed on device %d: (%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "ipv6 hash input field attribute get successful on device %d "
      "input field attribute %s (0x%d)",
      device,
      switch_hash_ipv6_input_field_attribute_to_string(*attr_flags),
      *attr_flags);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_ipv4_hash_input_field_attribute_get_internal(
    switch_device_t device,
    switch_hash_ipv4_input_fields_t input,
    switch_uint32_t *attr_flags) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();
  if (!SWITCH_HASH_TYPE_IS_VALID_IPV4_INPUT_FIELDS(input)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hash ipv4 input fields get failed for device %d, field list %d "
        "invalid "
        "(%s)",
        device,
        input,
        switch_error_to_string(status));
    return status;
  }
  if (!attr_flags) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "ipv4 hash fields attribute get failed for device %d, attribute flags "
        "is %p (%s)",
        device,
        attr_flags,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_pd_ipv4_hash_input_fields_attribute_get(device, input, attr_flags);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "ipv4 hash input field attribute get failed on device %d: (%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "ipv4 hash input field attribute get successful on device %d "
      "input field attribute %s (0x%d)",
      device,
      switch_hash_ipv4_input_field_attribute_to_string(*attr_flags),
      *attr_flags);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_non_ip_hash_input_field_attribute_get_internal(
    switch_device_t device,
    switch_hash_non_ip_input_fields_t input,
    switch_uint32_t *attr_flags) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  if (!SWITCH_HASH_TYPE_IS_VALID_NON_IP_INPUT_FIELDS(input)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hash non ip input fields get failed for device %d, field list %d "
        "invalid "
        "(%s)",
        device,
        input,
        switch_error_to_string(status));
    return status;
  }
  if (!attr_flags) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "non ip hash fields attribute get failed for device %d, attribute "
        "flags is %p "
        "(%s)",
        device,
        attr_flags,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_non_ip_hash_input_fields_attribute_get(
      device, input, attr_flags);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "non ip hash input field attribute get failed on device %d: (%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "non ip hash input field attribute get successful on device %d "
      "input field attribute %s (0x%d)",
      device,
      switch_hash_non_ip_input_field_attribute_to_string(*attr_flags),
      *attr_flags);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_ipv6_hash_algorithm_get_internal(
    switch_device_t device, switch_hash_ipv6_algorithm_t *algorithm) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_LOG_ENTER();
  if (!algorithm) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "ipv6 hash algorithm get failed for device %d, algorithm is %p (%s)",
        device,
        algorithm,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_ipv6_hash_algorithm_get(device, algorithm);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ipv6 hash algorithm get failed on device %d: (%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "ipv6 hash algorithm get successful on device %d "
      "algorithm %s (0x%d)",
      device,
      switch_hash_ipv6_algorithm_to_string(*algorithm),
      *algorithm);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_ipv4_hash_algorithm_get_internal(
    switch_device_t device, switch_hash_ipv4_algorithm_t *algorithm) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_LOG_ENTER();
  if (!algorithm) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "ipv4 hash algorithm get failed for device %d, algorithm is %p (%s)",
        algorithm,
        algorithm,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_ipv4_hash_algorithm_get(device, algorithm);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ipv4 hash algorithm get failed on device %d: (%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "ipv4 hash algorithm get successful on device %d "
      "algorithm %s (0x%d)",
      device,
      switch_hash_ipv4_algorithm_to_string(*algorithm),
      *algorithm);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_non_ip_hash_algorithm_get_internal(
    switch_device_t device, switch_hash_non_ip_algorithm_t *algorithm) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  SWITCH_LOG_ENTER();
  if (!algorithm) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "non ip hash algorithm get failed for device %d, algorithm is %p (%s)",
        device,
        algorithm,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_non_ip_hash_algorithm_get(device, algorithm);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("non ip hash algorithm get failed on device %d: (%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "non ip hash algorithm get successful on device %d "
      "algorithm %s (0x%d)",
      device,
      switch_hash_non_ip_algorithm_to_string(*algorithm),
      *algorithm);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_ipv6_hash_seed_get_internal(switch_device_t device,
                                                       uint64_t *seed) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();
  if (!seed) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("ipv6 hash seed get failed for device %d, seed is %p (%s)",
                     device,
                     seed,
                     switch_error_to_string(status));
    return status;
  }
  status = switch_pd_ipv6_hash_seed_get(device, seed);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ipv6 hash seed get failed on device %d: (%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "ipv6 hash seed get successful on device %d "
      "seed %lu",
      device,
      *seed);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_ipv4_hash_seed_get_internal(switch_device_t device,
                                                       uint64_t *seed) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();
  if (!seed) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("ipv4 hash seed get failed for device %d, seed is %p (%s)",
                     device,
                     seed,
                     switch_error_to_string(status));
    return status;
  }
  status = switch_pd_ipv4_hash_seed_get(device, seed);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ipv4 hash seed get failed on device %d: (%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "ipv4 hash seed get successful on device %d "
      "seed %lu",
      device,
      *seed);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_non_ip_hash_seed_get_internal(switch_device_t device,
                                                         uint64_t *seed) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();
  if (!seed) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "non ip hash seed get failed for device %d, seed is %p (%s)",
        device,
        seed,
        switch_error_to_string(status));
    return status;
  }
  status = switch_pd_non_ip_hash_seed_get(device, seed);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("non ip hash seed get failed on device %d: (%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "non ip hash seed get successful on device %d "
      "seed %lu",
      device,
      *seed);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_lag_hash_seed_get_internal(switch_device_t device,
                                                      uint64_t *seed) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();
  if (!seed) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("lag hash seed get failed for device %d, seed is %p (%s)",
                     device,
                     seed,
                     switch_error_to_string(status));
    return status;
  }
  status = switch_pd_lag_hash_seed_get(device, seed);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("lag hash seed get failed on device %d: (%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "lag hash seed get successful on device %d "
      "seed %lu",
      device,
      *seed);
  SWITCH_LOG_EXIT();
  return status;
}

switch_status_t switch_api_ecmp_hash_seed_get_internal(switch_device_t device,
                                                       uint64_t *seed) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();
  if (!seed) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("ecmp hash seed get failed for device %d, seed is %p (%s)",
                     device,
                     seed,
                     switch_error_to_string(status));
    return status;
  }
  status = switch_pd_ecmp_hash_seed_get(device, seed);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("ecmp hash seed get failed on device %d: (%s)\n",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "lag hash seed get successful on device %d "
      "seed %lu",
      device,
      *seed);
  SWITCH_LOG_EXIT();
  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_ipv6_hash_input_fields_set(
    switch_device_t device, switch_hash_ipv6_input_fields_t input) {
  SWITCH_MT_WRAP(switch_api_ipv6_hash_input_fields_set_internal(device, input))
}

switch_status_t switch_api_ipv4_hash_input_fields_set(
    switch_device_t device, switch_hash_ipv4_input_fields_t input) {
  SWITCH_MT_WRAP(switch_api_ipv4_hash_input_fields_set_internal(device, input))
}

switch_status_t switch_api_non_ip_hash_input_fields_set(
    switch_device_t device, switch_hash_non_ip_input_fields_t input) {
  SWITCH_MT_WRAP(
      switch_api_non_ip_hash_input_fields_set_internal(device, input))
}

switch_status_t switch_api_ipv6_hash_input_fields_attribute_set(
    switch_device_t device,
    switch_hash_ipv6_input_fields_t input,
    switch_uint32_t attr_flags) {
  SWITCH_MT_WRAP(switch_api_ipv6_hash_input_fields_attribute_set_internal(
      device, input, attr_flags))
}

switch_status_t switch_api_ipv4_hash_input_fields_attribute_set(
    switch_device_t device,
    switch_hash_ipv4_input_fields_t input,
    switch_uint32_t attr_flags) {
  SWITCH_MT_WRAP(switch_api_ipv4_hash_input_fields_attribute_set_internal(
      device, input, attr_flags))
}

switch_status_t switch_api_non_ip_hash_input_fields_attribute_set(
    switch_device_t device,
    switch_hash_non_ip_input_fields_t input,
    switch_uint32_t attr_flags) {
  SWITCH_MT_WRAP(switch_api_non_ip_hash_input_fields_attribute_set_internal(
      device, input, attr_flags))
}

switch_status_t switch_api_ipv6_hash_algorithm_set(
    switch_device_t device, switch_hash_ipv6_algorithm_t algorithm) {
  SWITCH_MT_WRAP(switch_api_ipv6_hash_algorithm_set_internal(device, algorithm))
}

switch_status_t switch_api_ipv4_hash_algorithm_set(
    switch_device_t device, switch_hash_ipv4_algorithm_t algorithm) {
  SWITCH_MT_WRAP(switch_api_ipv4_hash_algorithm_set_internal(device, algorithm))
}

switch_status_t switch_api_non_ip_hash_algorithm_set(
    switch_device_t device, switch_hash_non_ip_algorithm_t algorithm) {
  SWITCH_MT_WRAP(
      switch_api_non_ip_hash_algorithm_set_internal(device, algorithm))
}

switch_status_t switch_api_ipv6_hash_seed_set(switch_device_t device,
                                              uint64_t seed) {
  SWITCH_MT_WRAP(switch_api_ipv6_hash_seed_set_internal(device, seed))
}

switch_status_t switch_api_ipv4_hash_seed_set(switch_device_t device,
                                              uint64_t seed) {
  SWITCH_MT_WRAP(switch_api_ipv4_hash_seed_set_internal(device, seed))
}

switch_status_t switch_api_non_ip_hash_seed_set(switch_device_t device,
                                                uint64_t seed) {
  SWITCH_MT_WRAP(switch_api_non_ip_hash_seed_set_internal(device, seed))
}

switch_status_t switch_api_lag_hash_seed_set(switch_device_t device,
                                             uint64_t seed) {
  SWITCH_MT_WRAP(switch_api_lag_hash_seed_set_internal(device, seed));
}

switch_status_t switch_api_ecmp_hash_seed_set(switch_device_t device,
                                              uint64_t seed) {
  SWITCH_MT_WRAP(switch_api_ecmp_hash_seed_set_internal(device, seed));
}

switch_status_t switch_api_ipv6_hash_input_fields_get(
    switch_device_t device, switch_hash_ipv6_input_fields_t *fields) {
  SWITCH_MT_WRAP(switch_api_ipv6_hash_input_fields_get_internal(device, fields))
}

switch_status_t switch_api_ipv4_hash_input_fields_get(
    switch_device_t device, switch_hash_ipv4_input_fields_t *fields) {
  SWITCH_MT_WRAP(switch_api_ipv4_hash_input_fields_get_internal(device, fields))
}

switch_status_t switch_api_non_ip_hash_input_fields_get(
    switch_device_t device, switch_hash_non_ip_input_fields_t *fields) {
  SWITCH_MT_WRAP(
      switch_api_non_ip_hash_input_fields_get_internal(device, fields))
}

switch_status_t switch_api_ipv6_hash_input_field_attribute_get(
    switch_device_t device,
    switch_hash_ipv6_input_fields_t input,
    switch_uint32_t *attr_flags) {
  SWITCH_MT_WRAP(switch_api_ipv6_hash_input_field_attribute_get_internal(
      device, input, attr_flags))
}

switch_status_t switch_api_ipv4_hash_input_field_attribute_get(
    switch_device_t device,
    switch_hash_ipv4_input_fields_t input,
    switch_uint32_t *attr_flags) {
  SWITCH_MT_WRAP(switch_api_ipv4_hash_input_field_attribute_get_internal(
      device, input, attr_flags))
}

switch_status_t switch_api_non_ip_hash_input_field_attribute_get(
    switch_device_t device,
    switch_hash_non_ip_input_fields_t input,
    switch_uint32_t *attr_flags) {
  SWITCH_MT_WRAP(switch_api_non_ip_hash_input_field_attribute_get_internal(
      device, input, attr_flags))
}

switch_status_t switch_api_ipv6_hash_algorithm_get(
    switch_device_t device, switch_hash_ipv6_algorithm_t *algorithm) {
  SWITCH_MT_WRAP(switch_api_ipv6_hash_algorithm_get_internal(device, algorithm))
}

switch_status_t switch_api_ipv4_hash_algorithm_get(
    switch_device_t device, switch_hash_ipv4_algorithm_t *algorithm) {
  SWITCH_MT_WRAP(switch_api_ipv4_hash_algorithm_get_internal(device, algorithm))
}

switch_status_t switch_api_non_ip_hash_algorithm_get(
    switch_device_t device, switch_hash_non_ip_algorithm_t *algorithm) {
  SWITCH_MT_WRAP(
      switch_api_non_ip_hash_algorithm_get_internal(device, algorithm))
}

switch_status_t switch_api_ipv6_hash_seed_get(switch_device_t device,
                                              uint64_t *seed) {
  SWITCH_MT_WRAP(switch_api_ipv6_hash_seed_get_internal(device, seed))
}

switch_status_t switch_api_ipv4_hash_seed_get(switch_device_t device,
                                              uint64_t *seed) {
  SWITCH_MT_WRAP(switch_api_ipv4_hash_seed_get_internal(device, seed))
}

switch_status_t switch_api_non_ip_hash_seed_get(switch_device_t device,
                                                uint64_t *seed) {
  SWITCH_MT_WRAP(switch_api_non_ip_hash_seed_get_internal(device, seed))
}

switch_status_t switch_api_lag_hash_seed_get(switch_device_t device,
                                             uint64_t *seed) {
  SWITCH_MT_WRAP(switch_api_lag_hash_seed_get_internal(device, seed));
}

switch_status_t switch_api_ecmp_hash_seed_get(switch_device_t device,
                                              uint64_t *seed) {
  SWITCH_MT_WRAP(switch_api_ecmp_hash_seed_get_internal(device, seed));
}
