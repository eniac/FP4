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
#define __UAPI_DEF_IF_NET_DEVICE_FLAGS 1
#include <netinet/in.h>
#include <linux/if.h>
#include <libnl3/netlink/netlink.h>
#include <libnl3/netlink/route/link.h>
#include <libnl3/netlink/route/addr.h>
#include <errno.h>

#include "switchapi/switch_hostif.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_hostif_cpu_interface_create(switch_device_t device) {
  switch_hostif_context_t *hostif_ctx = NULL;
  switch_api_interface_info_t api_intf_info;
  switch_handle_t intf_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_HOSTIF, (void **)&hostif_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif cpu interface create failed on device %d: "
        "hostif context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  hostif_ctx->intf_handle = SWITCH_API_INVALID_HANDLE;

  SWITCH_MEMSET(&api_intf_info, 0x0, sizeof(api_intf_info));
  status = switch_api_device_cpu_port_handle_get(device, &api_intf_info.handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  api_intf_info.type = SWITCH_INTERFACE_TYPE_ACCESS;

  status = switch_api_interface_create(device, &api_intf_info, &intf_handle);
  SWITCH_ASSERT(intf_handle != SWITCH_API_INVALID_HANDLE);

  if (intf_handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "hostif cpu interface create failed on device %d: "
        "hostif interface creeate failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  hostif_ctx->intf_handle = intf_handle;

  return status;
}

switch_status_t switch_hostif_cpu_fabric_default_entry_add(
    switch_device_t device) {
  switch_hostif_context_t *hostif_ctx = NULL;
  switch_handle_t port_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  port_handle = SWITCH_CPU_PORT_ID;
  UNUSED(port_handle);

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_HOSTIF, (void **)&hostif_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif cpu interface create failed on device %d: "
        "hostif context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_pd_ingress_fabric_table_entry_add(device, &hostif_ctx->ing_pd_hdl);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif fabric default entry add failed on device %d: "
        "ingress fabric entry add failed:(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  return status;

cleanup:
  return status;
}

switch_status_t switch_hostif_cpu_fabric_default_entry_delete(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(device);

  return status;
}

switch_status_t switch_hostif_mirror_session_create(switch_device_t device) {
  switch_hostif_context_t *hostif_ctx = NULL;
  switch_handle_t mirror_handle = SWITCH_API_INVALID_HANDLE;
  switch_api_mirror_info_t api_mirror_info;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_HOSTIF, (void **)&hostif_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif mirror session create failed on device %d: "
        "hostif context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&api_mirror_info, 0, sizeof(api_mirror_info));
  api_mirror_info.session_id = SWITCH_CPU_MIRROR_SESSION_ID;
  status = switch_api_device_cpu_port_handle_get(
      device, &api_mirror_info.egress_port_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  api_mirror_info.direction = SWITCH_API_DIRECTION_BOTH;
  api_mirror_info.session_type = SWITCH_MIRROR_SESSION_TYPE_SIMPLE;
  api_mirror_info.mirror_type = SWITCH_MIRROR_TYPE_LOCAL;
  status = switch_api_mirror_session_create(
      device, &api_mirror_info, &mirror_handle);
  SWITCH_ASSERT(mirror_handle != SWITCH_API_INVALID_HANDLE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif mirror session create failed on device %d: "
        "mirror session create failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  hostif_ctx->mirror_handle = mirror_handle;

  return status;
}

switch_status_t switch_hostif_nhop_create(switch_device_t device) {
  switch_hostif_context_t *hostif_ctx = NULL;
  switch_api_nhop_info_t api_nhop_info;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_HOSTIF, (void **)&hostif_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif nhop create failed on device %d: "
        "hostif context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < SWITCH_HOSTIF_REASON_CODE_MAX; index++) {
    hostif_ctx->nhop_handles[index] = SWITCH_API_INVALID_HANDLE;
  }

  for (index = 0; index < SWITCH_HOSTIF_REASON_CODE_MAX; index++) {
    SWITCH_MEMSET(&api_nhop_info, 0x0, sizeof(switch_api_nhop_info_t));
    nhop_handle = SWITCH_API_INVALID_HANDLE;

    switch (index) {
      case SWITCH_HOSTIF_REASON_CODE_GLEAN:
        api_nhop_info.nhop_type = SWITCH_NHOP_TYPE_GLEAN;
        SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(hostif_ctx->intf_handle));
        api_nhop_info.intf_handle = hostif_ctx->intf_handle;
        api_nhop_info.ip_addr.type = SWITCH_API_IP_ADDR_V4;
        api_nhop_info.ip_addr.ip.v4addr = 0xFF000001;
        status = switch_api_nhop_create(device, &api_nhop_info, &nhop_handle);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "hostif nhop create failed on device %d: "
              "nhop glean create failed:(%s)\n",
              device,
              switch_error_to_string(status));
          goto cleanup;
        }
        break;

      case SWITCH_HOSTIF_REASON_CODE_MYIP:
        api_nhop_info.nhop_type = SWITCH_NHOP_TYPE_NONE;
        SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(hostif_ctx->intf_handle));
        api_nhop_info.intf_handle = hostif_ctx->intf_handle;
        api_nhop_info.ip_addr.type = SWITCH_API_IP_ADDR_V4;
        api_nhop_info.ip_addr.ip.v4addr = 0xFF000002;
        status = switch_api_nhop_create(device, &api_nhop_info, &nhop_handle);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "hostif nhop create failed on device %d: "
              "nhop myip create failed:(%s)\n",
              device,
              switch_error_to_string(status));
          goto cleanup;
        }
        break;

      case SWITCH_HOSTIF_REASON_CODE_NULL_DROP:
        api_nhop_info.nhop_type = SWITCH_NHOP_TYPE_DROP;
        SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(hostif_ctx->intf_handle));
        api_nhop_info.intf_handle = hostif_ctx->intf_handle;
        api_nhop_info.ip_addr.type = SWITCH_API_IP_ADDR_V4;
        api_nhop_info.ip_addr.ip.v4addr = 0xFF000003;
        status = switch_api_nhop_create(device, &api_nhop_info, &nhop_handle);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "hostif nhop create failed on device %d: "
              "nhop drop create failed:(%s)\n",
              device,
              switch_error_to_string(status));
          goto cleanup;
        }
        break;

      default:
        nhop_handle = SWITCH_API_INVALID_HANDLE;
        break;
    }

    hostif_ctx->nhop_handles[index] = nhop_handle;
  }
  return status;

cleanup:

  return status;
}

switch_status_t switch_hostif_nhop_delete(switch_device_t device) {
  switch_hostif_context_t *hostif_ctx = NULL;
  switch_handle_t nhop_handle = SWITCH_API_INVALID_HANDLE;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_HOSTIF, (void **)&hostif_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif nhop delete failed on device %d: "
        "hostif context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < SWITCH_HOSTIF_REASON_CODE_MAX; index++) {
    nhop_handle = hostif_ctx->nhop_handles[index];

    switch (index) {
      case SWITCH_HOSTIF_REASON_CODE_GLEAN:
      case SWITCH_HOSTIF_REASON_CODE_MYIP:
      case SWITCH_HOSTIF_REASON_CODE_NULL_DROP:
        if (SWITCH_NHOP_HANDLE(nhop_handle)) {
          status = switch_api_nhop_delete(device, nhop_handle);
          if (status != SWITCH_STATUS_SUCCESS) {
            SWITCH_LOG_ERROR(
                "hostif nhop delete failed on device %d: "
                "nhop delete failed:(%s)\n",
                device,
                switch_error_to_string(status));
            continue;
          }
        }
        break;
      default:
        break;
    }
  }

  for (index = 0; index < SWITCH_HOSTIF_REASON_CODE_MAX; index++) {
    hostif_ctx->nhop_handles[index] = SWITCH_API_INVALID_HANDLE;
  }

  return status;
}

switch_status_t switch_hostif_default_entries_add(switch_device_t device) {
  switch_hostif_context_t *hostif_ctx = NULL;
  switch_api_hostif_rcode_info_t rcode_api_info;
  switch_uint64_t flags = 0;
  switch_handle_t rcode_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_HANDLE_TYPE_HOSTIF, (void **)&hostif_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif default entry add failed on device %d: "
        "hostif context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&rcode_api_info, 0x0, sizeof(rcode_api_info));
  flags |= SWITCH_HOSTIF_RCODE_ATTR_REASON_CODE;
  rcode_api_info.reason_code = SWITCH_HOSTIF_REASON_CODE_NONE;
  // Set the default internal ACL priority
  rcode_api_info.priority = SWITCH_DEFAULT_INTERNAL_ACL_LOW_PRIO_START;
  status = switch_api_hostif_reason_code_create(
      device, flags, &rcode_api_info, &rcode_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif default entry add failed on device %d: "
        "hostif none reason code create failed:(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_hostif_cpu_interface_create(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif default entry add failed on device %d: "
        "hostif cpu interface create failed:(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_hostif_cpu_fabric_default_entry_add(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif default entry add failed on device %d: "
        "hostif fabric entry add failed:(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  // status = switch_hostif_mirror_session_create(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif default entry add failed on device %d: "
        "cpu mirror session create failed:(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_hostif_nhop_create(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif default entry add failed on device %d: "
        "hostif nhop create failed:(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  return status;

cleanup:
  return status;
}

switch_status_t switch_hostif_default_entries_delete(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  switch_hostif_context_t *hostif_ctx = NULL;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_HOSTIF, (void **)&hostif_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif default entry delete failed on device %d: "
        "hostif context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_hostif_nhop_delete(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif default entries delete failed on device %d: "
        "hostif nhop delete failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }

  if (SWITCH_INTERFACE_HANDLE(hostif_ctx->intf_handle)) {
    status = switch_api_interface_delete(device, hostif_ctx->intf_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "hostif default entries delete failed on device %d: "
          "hostif interface delete failed:(%s)\n",
          device,
          switch_error_to_string(status));
    }
  }

  if (SWITCH_MIRROR_HANDLE(hostif_ctx->mirror_handle)) {
    status =
        switch_api_mirror_session_delete(device, hostif_ctx->mirror_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "hostif default entries delete failed on device %d: "
          "hostif mirror session delete failed:(%s)\n",
          device,
          switch_error_to_string(status));
    }
  }

  status = switch_api_hostif_reason_code_delete(
      device, hostif_ctx->rcode_handles[SWITCH_HOSTIF_REASON_CODE_NONE]);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif default entries delete failed on device %d: "
        "hostif reason code delete failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }
  return status;
}

switch_status_t switch_hostif_table_entry_key_init(void *args,
                                                   switch_uint8_t *key,
                                                   switch_uint32_t *len) {
  switch_uint8_t *intf = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(args && key && len);
  if (!args || !key || !len) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hostif hash entry key init failed: "
        "invalid parameters(%s)\n",
        switch_error_to_string(status));
    return status;
  }

  *len = 0;
  intf = (switch_uint8_t *)args;

  SWITCH_MEMCPY(key, intf, SWITCH_HOSTIF_NAME_SIZE);
  *len += SWITCH_HOSTIF_NAME_SIZE;

  SWITCH_ASSERT(*len == SWITCH_HOSTIF_HASH_KEY_SIZE);

  return status;
}

switch_int32_t switch_hostif_entry_hash_compare(const void *key1,
                                                const void *key2) {
  return SWITCH_MEMCMP(key1, key2, SWITCH_HOSTIF_HASH_KEY_SIZE);
}

switch_status_t switch_hostif_init(switch_device_t device) {
  switch_hostif_context_t *hostif_ctx = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_meter_info_t meter_info;

  hostif_ctx = SWITCH_MALLOC(device, sizeof(switch_hostif_context_t), 0x1);
  if (hostif_ctx == NULL) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "hostif init failed on device %d: "
        "hostif context malloc failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(hostif_ctx, 0x0, sizeof(switch_hostif_context_t));

  status = switch_device_api_context_set(
      device, SWITCH_API_TYPE_HOSTIF, (void *)hostif_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif init failed on device %d: "
        "hostif context set failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_handle_type_init(device,
                                   SWITCH_HANDLE_TYPE_HOSTIF_REASON_CODE,
                                   SWITCH_HOSTIF_REASON_CODE_MAX);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif init failed on device %d: "
        "hostif reason code handle init failed:(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_HOSTIF_GROUP, SWITCH_HOSTIF_GROUP_SIZE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif init failed on device %d: "
        "hostif group handle init failed:(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_handle_type_init(
      device, SWITCH_HANDLE_TYPE_HOSTIF, SWITCH_HOSTIF_SIZE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif init failed on device %d: "
        "hostif handle init failed:(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_handle_type_init(device,
                                   SWITCH_HANDLE_TYPE_HOSTIF_RX_FILTER,
                                   SWITCH_HOSTIF_RX_FILTER_SIZE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif init failed on device %d: "
        "hostif rx filter handle init failed:(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_handle_type_init(device,
                                   SWITCH_HANDLE_TYPE_HOSTIF_TX_FILTER,
                                   SWITCH_HOSTIF_TX_FILTER_SIZE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif init failed on device %d: "
        "hostif tx filter handle init failed:(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  hostif_ctx->hostif_hashtable.size = SWITCH_HOSTIF_HASHTABLE_SIZE;
  hostif_ctx->hostif_hashtable.compare_func = switch_hostif_entry_hash_compare;
  hostif_ctx->hostif_hashtable.key_func = switch_hostif_table_entry_key_init;
  hostif_ctx->hostif_hashtable.hash_seed = SWITCH_HOSTIF_HASH_SEED;

  status = SWITCH_HASHTABLE_INIT(&hostif_ctx->hostif_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif init failed on device %d: "
        "hostif hashtable init failed(%s)\n",
        device,
        switch_error_to_string(status));
    goto cleanup;
  }

  for (index = 0; index < SWITCH_MAX_RX_CALLBACK; index++) {
    hostif_ctx->rx_cb_list[index].valid = FALSE;
    hostif_ctx->rx_cb_list[index].app_id = 0;
    hostif_ctx->rx_cb_list[index].cb_fn = NULL;
    hostif_ctx->rx_cb_list[index].cookie = NULL;
  }

  SWITCH_MEMSET(&meter_info, 0, sizeof(switch_meter_info_t));
  meter_info.api_meter_info.meter_type = SWITCH_METER_TYPE_BYTES;
  meter_info.api_meter_info.cir =
      (switch_uint64_t)SWITCH_HOSTIF_DEFAULT_POLICER_RATE / 8;
  meter_info.api_meter_info.pir =
      (switch_uint64_t)SWITCH_HOSTIF_DEFAULT_POLICER_RATE / 8;
  meter_info.api_meter_info.color_source = SWITCH_METER_COLOR_SOURCE_BLIND;
  meter_info.api_meter_info.cbs =
      (switch_uint64_t)SWITCH_HOSTIF_DEFAULT_POLICER_RATE / 8;
  meter_info.api_meter_info.pbs =
      (switch_uint64_t)SWITCH_HOSTIF_DEFAULT_POLICER_RATE / 8;
  status = switch_pd_hostif_meter_set(device, 0, &meter_info, true);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("hostif init failed on device %d: ",
                     "default hostif copp meter set in hw failed %s",
                     device,
                     switch_error_to_string(status));
    return status;
  }

  return status;

cleanup:
  status = switch_hostif_free(device);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  return status;
}

switch_status_t switch_hostif_free(switch_device_t device) {
  switch_hostif_context_t *hostif_ctx = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_HOSTIF, (void **)&hostif_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif free failed on device %d: "
        "hostif context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  SWITCH_ASSERT(hostif_ctx->hostif_hashtable.num_entries == 0);
  status = SWITCH_HASHTABLE_DONE(&hostif_ctx->hostif_hashtable);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif free failed on device %d: "
        "hostif hashtable done failed(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_HOSTIF_GROUP);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif free failed on device %d: "
        "hostif group handle free failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }

  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_HOSTIF);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif free failed on device %d: "
        "hostif handle free failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_HOSTIF_RX_FILTER);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif free failed on device %d: "
        "hostif rx filter handle free failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_handle_type_free(device, SWITCH_HANDLE_TYPE_HOSTIF_TX_FILTER);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif free failed on device %d: "
        "hostif tx filter handle free failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status =
      switch_handle_type_free(device, SWITCH_HANDLE_TYPE_HOSTIF_REASON_CODE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif free failed on device %d: "
        "hostif reason code handle free failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  for (index = 0; index < SWITCH_MAX_RX_CALLBACK; index++) {
    hostif_ctx->rx_cb_list[index].valid = FALSE;
    hostif_ctx->rx_cb_list[index].app_id = 0;
    hostif_ctx->rx_cb_list[index].cb_fn = NULL;
    hostif_ctx->rx_cb_list[index].cookie = NULL;
  }

  return status;
}

switch_status_t switch_api_hostif_group_create_internal(
    const switch_device_t device,
    const switch_hostif_group_t *hif_group,
    switch_handle_t *hif_group_handle) {
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_hostif_group_info_t *hif_group_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_meter_info_t *meter_info = NULL;

  handle = switch_hostif_group_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "hostif group create failed on device %d: "
        "hostif handle create failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_hostif_group_get(device, handle, &hif_group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif group create failed on device %d: "
        "hostif group get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  /*
   * Set the COPP policer rate parameters from the regular policer handle.
   */
  if (hif_group->policer_handle != SWITCH_API_INVALID_HANDLE) {
    status = switch_meter_get(device, hif_group->policer_handle, &meter_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("hostif group create failed on device %d: ",
                       "hostif copp policer get failed for handle 0x%lx",
                       device,
                       hif_group->policer_handle);
      return status;
    }
    status = switch_pd_hostif_meter_set(
        device, handle_to_id(hif_group->policer_handle), meter_info, true);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("hostif group create failed on device %d: ",
                       "hostif copp meter set in hw failed for handle 0x%lx",
                       device,
                       hif_group->policer_handle);
      return status;
    }
    meter_info->meter_type = SWITCH_METER_TYPE_COPP;
  }

  SWITCH_MEMCPY(
      &hif_group_info->hif_group, hif_group, sizeof(switch_hostif_group_t));

  hif_group_info->ref_count = 0;

  *hif_group_handle = handle;

  SWITCH_LOG_DEBUG(
      "hostif group created on device %d handle 0x%lx\n", device, handle);

  return status;
}

switch_status_t switch_api_hostif_group_delete_internal(
    const switch_device_t device, const switch_handle_t hif_group_handle) {
  switch_hostif_group_info_t *hostif_group_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_HOSTIF_GROUP_HANDLE(hif_group_handle));
  if (!SWITCH_HOSTIF_GROUP_HANDLE(hif_group_handle)) {
    SWITCH_LOG_ERROR(
        "hostif group delete failed on device %d: "
        "hostif group handle invalid:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_hostif_group_get(
      device, hif_group_handle, (void **)&hostif_group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif group delete failed on device %d: "
        "hostif group get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (hostif_group_info->ref_count > 0) {
    status = SWITCH_STATUS_RESOURCE_IN_USE;
    SWITCH_LOG_ERROR(
        "hostif group delete failed on device %d: "
        "hostif group in use:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_hostif_group_handle_delete(device, hif_group_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif group delete failed on device %d: "
        "hostif group in use:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("hostif group deleted on device %d handle 0x%lx\n",
                   device,
                   hif_group_handle);

  return status;
}

switch_status_t switch_api_hostif_group_meter_set_internal(
    switch_device_t device,
    switch_handle_t group_handle,
    switch_handle_t meter_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_meter_info_t *meter_info = NULL;
  switch_hostif_group_info_t *hif_group_info = NULL;

  status = switch_hostif_group_get(device, group_handle, &hif_group_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif group set failed on device %d: "
        "hostif group get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  status = switch_meter_get(device, meter_handle, &meter_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif meter set failed on device %d: "
        "meter handle get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_hostif_meter_set(
      device, handle_to_id(meter_handle), meter_info, TRUE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif meter create failed on device %d: "
        "meter pd set failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  hif_group_info->hif_group.policer_handle = meter_handle;
  return status;
}

switch_status_t switch_api_hostif_reason_code_create_internal(
    const switch_device_t device,
    const switch_uint64_t flags,
    const switch_api_hostif_rcode_info_t *rcode_api_info,
    switch_handle_t *rcode_handle) {
  switch_hostif_context_t *hostif_ctx = NULL;
  switch_hostif_rcode_info_t *rcode_info = NULL;
  switch_hostif_group_t *hostif_group_info = NULL;
  switch_acl_action_t acl_action;
  switch_acl_action_params_t action_params;
  switch_acl_opt_action_params_t opt_action_params;
  switch_handle_t acl_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t system_acl_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t egress_system_acl_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t ace_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t counter_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t system_counter_handle = SWITCH_API_INVALID_HANDLE;
  switch_uint16_t priority;
  switch_uint16_t range_index = 0;
  int field_count = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_queue_info_t *queue_info = NULL;
  switch_handle_t queue_handle = SWITCH_API_INVALID_HANDLE;
  switch_qid_t queue_id = 0;

  SWITCH_ASSERT(rcode_api_info && rcode_handle);
  if (!rcode_api_info || !rcode_handle) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hostif reason code create failed on device %d: "
        "null parameters:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "hostif reason code create on device %d rc %s(%d) flags 0x%lx\n",
      device,
      switch_hostif_code_to_string(rcode_api_info->reason_code),
      rcode_api_info->reason_code,
      flags);

  // Set the default priority if priority is not set.
  if (rcode_api_info->priority) {
    priority = rcode_api_info->priority;
  } else {
    priority = SWITCH_API_ACL_ENTRY_MINIMUM_PRIORITY;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_HOSTIF, (void **)&hostif_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif reason code create failed on device %d rc %s(%d): "
        "hostif context get failed:(%s)\n",
        device,
        switch_hostif_code_to_string(rcode_api_info->reason_code),
        rcode_api_info->reason_code,
        switch_error_to_string(status));
    return status;
  }

  if (SWITCH_HOSTIF_RCODE_HANDLE(
          hostif_ctx->rcode_handles[rcode_api_info->reason_code])) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    SWITCH_LOG_ERROR(
        "hostif reason code create failed on device %d rc %s(%d): "
        "rcode exists:(%s)\n",
        device,
        switch_hostif_code_to_string(rcode_api_info->reason_code),
        rcode_api_info->reason_code,
        switch_error_to_string(status));
    return status;
  }

  handle = switch_hostif_rcode_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "hostif reason code create failed on device %d rc %s(%d): "
        "hostif rcode handle create failed:(%s)\n",
        device,
        switch_hostif_code_to_string(rcode_api_info->reason_code),
        rcode_api_info->reason_code,
        switch_error_to_string(status));
    return status;
  }

  status = switch_hostif_rcode_get(device, handle, &rcode_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif reason code create failed on device %d rc %s: "
        "hostif rcode handle get failed:(%s)\n",
        device,
        switch_hostif_code_to_string(rcode_api_info->reason_code),
        rcode_api_info->reason_code,
        switch_error_to_string(status));
    return status;
  }

  hostif_ctx->rcode_handles[rcode_api_info->reason_code] = handle;

  status = switch_api_acl_counter_create(device, &counter_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif reason code create failed on device %d "
        "reason code %s(%d): acl counter create failed(%s)\n",
        device,
        switch_hostif_code_to_string(rcode_api_info->reason_code),
        rcode_api_info->reason_code,
        switch_error_to_string(status));
    goto cleanup;
  }

  status = switch_api_acl_counter_create(device, &system_counter_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif reason code create failed on device %d "
        "reason code %s(%d): system acl counter create failed(%s) \n",
        device,
        switch_hostif_code_to_string(rcode_api_info->reason_code),
        rcode_api_info->reason_code,
        switch_error_to_string(status));
    goto cleanup;
  }

  /*
   * dummy hostif reason code created for packets destined to cpu
   *  with no reason code
   */
  if (rcode_api_info->reason_code == SWITCH_HOSTIF_REASON_CODE_NONE) {
    return status;
  }

  SWITCH_MEMCPY(&rcode_info->rcode_api_info,
                rcode_api_info,
                sizeof(switch_api_hostif_rcode_info_t));

  if (flags & SWITCH_HOSTIF_RCODE_ATTR_HOSTIF_GROUP) {
    SWITCH_ASSERT(SWITCH_HOSTIF_GROUP_HANDLE(rcode_api_info->hostif_group_id));
    status = switch_hostif_group_get(
        device, rcode_api_info->hostif_group_id, &hostif_group_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "hostif reason code create failed on device %d rc %s(%d): "
          "hostif group get failed:(%s)\n",
          device,
          switch_hostif_code_to_string(rcode_api_info->reason_code),
          rcode_api_info->reason_code,
          switch_error_to_string(status));
      return status;
    }

    SWITCH_ASSERT(hostif_group_info != NULL);
    queue_handle = hostif_group_info->queue_handle;
    SWITCH_ASSERT(SWITCH_QUEUE_HANDLE(queue_handle));
    if (!SWITCH_QUEUE_HANDLE(queue_handle)) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR(
          "hostif reason code create failed on device %d "
          "reason code %s: system acl list create failed"
          " , queue handle(%s) \n",
          device,
          switch_hostif_code_to_string(rcode_api_info->reason_code),
          switch_error_to_string(status));
      return status;
    }

    status = switch_queue_get(device, queue_handle, &queue_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR(
          "hostif reason code create failed on device %d "
          "reason code %s: system acl list create failed"
          " , queue info get error (%s) \n",
          device,
          switch_hostif_code_to_string(rcode_api_info->reason_code),
          switch_error_to_string(status));
      return status;
    }
    queue_id = queue_info->queue_id;
  }

  switch (rcode_api_info->reason_code) {
    case SWITCH_HOSTIF_REASON_CODE_STP: {
      // stp bpdu, redirect to cpu
      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_MAC,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s(%d): mac acl list create failed(%s)\n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            rcode_api_info->reason_code,
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_mac_key_value_pair_t acl_kvp[SWITCH_ACL_MAC_FIELD_MAX];
      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      acl_kvp[field_count].field = SWITCH_ACL_MAC_FIELD_DEST_MAC;
      acl_kvp[field_count].value.dest_mac.mac_addr[0] = 0x01;
      acl_kvp[field_count].value.dest_mac.mac_addr[1] = 0x80;
      acl_kvp[field_count].value.dest_mac.mac_addr[2] = 0xC2;
      acl_kvp[field_count].value.dest_mac.mac_addr[3] = 0x00;
      acl_kvp[field_count].value.dest_mac.mac_addr[4] = 0x00;
      acl_kvp[field_count].value.dest_mac.mac_addr[5] = 0x00;
      acl_kvp[field_count].mask.u.mask = 0xFFFFFFFFFFFF;
      field_count++;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      opt_action_params.counter_handle = counter_handle;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s(%d): mac acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            rcode_api_info->reason_code,
            switch_error_to_string(status));
        goto cleanup;
      }

      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      acl_kvp[field_count].field = SWITCH_ACL_MAC_FIELD_DEST_MAC;
      acl_kvp[field_count].value.dest_mac.mac_addr[0] = 0x01;
      acl_kvp[field_count].value.dest_mac.mac_addr[1] = 0x00;
      acl_kvp[field_count].value.dest_mac.mac_addr[2] = 0x0C;
      acl_kvp[field_count].value.dest_mac.mac_addr[3] = 0xCC;
      acl_kvp[field_count].value.dest_mac.mac_addr[4] = 0xCC;
      acl_kvp[field_count].value.dest_mac.mac_addr[5] = 0xCD;
      acl_kvp[field_count].mask.u.mask = 0xFFFFFFFFFFFF;
      field_count++;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      opt_action_params.counter_handle = counter_handle;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s(%d): mac acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            rcode_api_info->reason_code,
            switch_error_to_string(status));
        goto cleanup;
      }

      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_SYSTEM,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &system_acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s(%d): system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            rcode_api_info->reason_code,
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_system_key_value_pair_t
          system_acl_kvp[SWITCH_ACL_SYSTEM_FIELD_MAX];
      SWITCH_MEMSET(&system_acl_kvp, 0, sizeof(system_acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_REASON_CODE;
      system_acl_kvp[field_count].value.reason_code =
          rcode_api_info->reason_code;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      acl_action = rcode_api_info->action;
      field_count++;

      if (hostif_group_info) {
        opt_action_params.queue_id = queue_id;
        opt_action_params.meter_handle = hostif_group_info->policer_handle;
      }
      opt_action_params.counter_handle = system_counter_handle;

      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority++,
                                          field_count,
                                          system_acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s(%d): system acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            rcode_api_info->reason_code,
            switch_error_to_string(status));
        goto cleanup;
      }

      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_LACP: {
      // lacp bpdu, redirect to cpu
      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_MAC,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s(%d): mac acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            rcode_api_info->reason_code,
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_mac_key_value_pair_t acl_kvp[SWITCH_ACL_MAC_FIELD_MAX];
      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      acl_kvp[field_count].field = SWITCH_ACL_MAC_FIELD_DEST_MAC;
      acl_kvp[field_count].value.dest_mac.mac_addr[0] = 0x01;
      acl_kvp[field_count].value.dest_mac.mac_addr[1] = 0x80;
      acl_kvp[field_count].value.dest_mac.mac_addr[2] = 0xC2;
      acl_kvp[field_count].value.dest_mac.mac_addr[3] = 0x00;
      acl_kvp[field_count].value.dest_mac.mac_addr[4] = 0x00;
      acl_kvp[field_count].value.dest_mac.mac_addr[5] = 0x02;
      acl_kvp[field_count].mask.u.mask = 0xFFFFFFFFFFFF;
      acl_action = rcode_api_info->action;
      field_count++;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      opt_action_params.counter_handle = counter_handle;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: mac acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_SYSTEM,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &system_acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_system_key_value_pair_t
          system_acl_kvp[SWITCH_ACL_SYSTEM_FIELD_MAX];
      SWITCH_MEMSET(&system_acl_kvp, 0, sizeof(system_acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_REASON_CODE;
      system_acl_kvp[field_count].value.reason_code =
          rcode_api_info->reason_code;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;

      if (hostif_group_info) {
        opt_action_params.queue_id = queue_id;
        opt_action_params.meter_handle = hostif_group_info->policer_handle;
      }
      opt_action_params.counter_handle = system_counter_handle;

      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority++,
                                          field_count,
                                          system_acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_LLDP: {
      // lacp frame, redirect to cpu
      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_MAC,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: mac acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_mac_key_value_pair_t acl_kvp[SWITCH_ACL_MAC_FIELD_MAX];
      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      acl_kvp[field_count].field = SWITCH_ACL_MAC_FIELD_DEST_MAC;
      acl_kvp[field_count].value.dest_mac.mac_addr[0] = 0x01;
      acl_kvp[field_count].value.dest_mac.mac_addr[1] = 0x80;
      acl_kvp[field_count].value.dest_mac.mac_addr[2] = 0xC2;
      acl_kvp[field_count].value.dest_mac.mac_addr[3] = 0x00;
      acl_kvp[field_count].value.dest_mac.mac_addr[4] = 0x00;
      acl_kvp[field_count].value.dest_mac.mac_addr[5] = 0x0e;
      acl_kvp[field_count].mask.u.mask = 0xFFFFFFFFFFFF;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_MAC_FIELD_ETH_TYPE;
      acl_kvp[field_count].value.eth_type = 0x88CC;
      acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      opt_action_params.counter_handle = counter_handle;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority++,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: mac acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      acl_kvp[field_count].field = SWITCH_ACL_MAC_FIELD_DEST_MAC;
      acl_kvp[field_count].value.dest_mac.mac_addr[0] = 0x01;
      acl_kvp[field_count].value.dest_mac.mac_addr[1] = 0x80;
      acl_kvp[field_count].value.dest_mac.mac_addr[2] = 0xC2;
      acl_kvp[field_count].value.dest_mac.mac_addr[3] = 0x00;
      acl_kvp[field_count].value.dest_mac.mac_addr[4] = 0x00;
      acl_kvp[field_count].value.dest_mac.mac_addr[5] = 0x03;
      acl_kvp[field_count].mask.u.mask = 0xFFFFFFFFFFFF;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_MAC_FIELD_ETH_TYPE;
      acl_kvp[field_count].value.eth_type = 0x88CC;
      acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      opt_action_params.counter_handle = counter_handle;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority++,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      acl_kvp[field_count].field = SWITCH_ACL_MAC_FIELD_DEST_MAC;
      acl_kvp[field_count].value.dest_mac.mac_addr[0] = 0x01;
      acl_kvp[field_count].value.dest_mac.mac_addr[1] = 0x80;
      acl_kvp[field_count].value.dest_mac.mac_addr[2] = 0xC2;
      acl_kvp[field_count].value.dest_mac.mac_addr[3] = 0x00;
      acl_kvp[field_count].value.dest_mac.mac_addr[4] = 0x00;
      acl_kvp[field_count].value.dest_mac.mac_addr[5] = 0x00;
      acl_kvp[field_count].mask.u.mask = 0xFFFFFFFFFFFF;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_MAC_FIELD_ETH_TYPE;
      acl_kvp[field_count].value.eth_type = 0x88CC;
      acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      opt_action_params.counter_handle = counter_handle;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_SYSTEM,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &system_acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_system_key_value_pair_t
          system_acl_kvp[SWITCH_ACL_SYSTEM_FIELD_MAX];
      SWITCH_MEMSET(&system_acl_kvp, 0, sizeof(system_acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_REASON_CODE;
      system_acl_kvp[field_count].value.reason_code =
          rcode_api_info->reason_code;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;

      if (hostif_group_info) {
        opt_action_params.queue_id = queue_id;
        opt_action_params.meter_handle = hostif_group_info->policer_handle;
      }
      opt_action_params.counter_handle = system_counter_handle;

      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority++,
                                          field_count,
                                          system_acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_OSPF: {
      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_IP,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_ip_key_value_pair_t acl_kvp[SWITCH_ACL_IP_FIELD_MAX];
      // All OSPF routers 224.0.0.5, copy to cpu
      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_IPV4_DEST;
      acl_kvp[field_count].value.ipv4_dest = 0xE0000005;
      acl_kvp[field_count].mask.u.mask = 0xFFFFFFFF;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_IP_PROTO;
      acl_kvp[field_count].value.ip_proto = 89;
      acl_kvp[field_count].mask.u.mask = 0xFF;
      field_count++;
      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      opt_action_params.counter_handle = counter_handle;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      // All OSPF designated routes (DRs) 224.0.0.6, copy to cpu
      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_IPV4_DEST;
      acl_kvp[field_count].value.ipv4_dest = 0xE0000006;
      acl_kvp[field_count].mask.u.mask = 0xFFFFFFFF;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_IP_PROTO;
      acl_kvp[field_count].value.ip_proto = 89;
      acl_kvp[field_count].mask.u.mask = 0xFF;
      field_count++;
      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      opt_action_params.counter_handle = counter_handle;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_SYSTEM,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &system_acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_system_key_value_pair_t
          system_acl_kvp[SWITCH_ACL_SYSTEM_FIELD_MAX];
      SWITCH_MEMSET(&system_acl_kvp, 0, sizeof(system_acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_REASON_CODE;
      system_acl_kvp[field_count].value.reason_code =
          rcode_api_info->reason_code;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_IPV4_ENABLED;
      system_acl_kvp[field_count].value.ipv4_enabled = TRUE;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFFFFFF;
      field_count++;

      if (hostif_group_info) {
        opt_action_params.queue_id = queue_id;
        opt_action_params.meter_handle = hostif_group_info->policer_handle;
      }
      opt_action_params.counter_handle = system_counter_handle;

      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority++,
                                          field_count,
                                          system_acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_VRRP: {
      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_IP,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_ip_key_value_pair_t acl_kvp[SWITCH_ACL_IP_FIELD_MAX];
      // All VRRP packets destined to  224.0.0.18, redirect to cpu
      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_IPV4_DEST;
      acl_kvp[field_count].value.ipv4_dest = 0xE0000012;
      acl_kvp[field_count].mask.u.mask = 0xFFFFFFFF;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_IP_PROTO;
      acl_kvp[field_count].value.ip_proto = SWITCH_HOSTIF_IP_PROTO_VRRP;
      acl_kvp[field_count].mask.u.mask = 0xFF;
      field_count++;
      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_SYSTEM,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &system_acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_system_key_value_pair_t
          system_acl_kvp[SWITCH_ACL_SYSTEM_FIELD_MAX];
      SWITCH_MEMSET(&system_acl_kvp, 0, sizeof(system_acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_REASON_CODE;
      system_acl_kvp[field_count].value.reason_code =
          rcode_api_info->reason_code;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_IPV4_ENABLED;
      system_acl_kvp[field_count].value.ipv4_enabled = TRUE;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFFFFFF;
      field_count++;

      if (hostif_group_info) {
        opt_action_params.queue_id = queue_id;
        opt_action_params.meter_handle = hostif_group_info->policer_handle;
      }

      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority++,
                                          field_count,
                                          system_acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_OSPFV6: {
      // All OSPFv3 routers ff02::5, copy to cpu
      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_IPV6,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ipv6 acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_ipv6_key_value_pair_t acl_kvp[SWITCH_ACL_IPV6_FIELD_MAX];
      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      acl_kvp[field_count].field = SWITCH_ACL_IPV6_FIELD_IP_PROTO;
      acl_kvp[field_count].value.ip_proto = 89;
      acl_kvp[field_count].mask.u.mask.u.addr8[0] = 0xFF;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_IPV6_FIELD_IPV6_DEST;
      inet_pton(
          AF_INET6, "FF02::5", acl_kvp[field_count].value.ipv6_dest.u.addr8);
      SWITCH_MEMSET(acl_kvp[field_count].mask.u.mask.u.addr8, 0xFF, 16);
      field_count++;
      acl_action = rcode_api_info->action;
      opt_action_params.counter_handle = counter_handle;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;

      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority++,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ipv6 acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      // All OSPFv3 designated routes (DRs) ff02::6, copy to cpu
      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      acl_kvp[field_count].field = SWITCH_ACL_IPV6_FIELD_IP_PROTO;
      acl_kvp[field_count].value.ip_proto = 89;
      acl_kvp[field_count].mask.u.mask.u.addr8[0] = 0xFF;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_IPV6_FIELD_IPV6_DEST;
      inet_pton(
          AF_INET6, "FF02::6", acl_kvp[field_count].value.ipv6_dest.u.addr8);
      SWITCH_MEMSET(acl_kvp[field_count].mask.u.mask.u.addr8, 0xFF, 16);
      field_count++;
      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      opt_action_params.counter_handle = counter_handle;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ipv6 acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_SYSTEM,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &system_acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_system_key_value_pair_t
          system_acl_kvp[SWITCH_ACL_SYSTEM_FIELD_MAX];
      SWITCH_MEMSET(&system_acl_kvp, 0, sizeof(system_acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_REASON_CODE;
      system_acl_kvp[field_count].value.reason_code =
          rcode_api_info->reason_code;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_IPV6_ENABLED;
      system_acl_kvp[field_count].value.ipv6_enabled = TRUE;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFFFFFF;
      field_count++;

      if (hostif_group_info) {
        opt_action_params.queue_id = queue_id;
        opt_action_params.meter_handle = hostif_group_info->policer_handle;
      }
      opt_action_params.counter_handle = system_counter_handle;

      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority++,
                                          field_count,
                                          system_acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_BGP: {
      switch_range_t switch_range;
      switch_handle_t range_handle = SWITCH_API_INVALID_HANDLE;

      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_IP,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_ip_key_value_pair_t acl_kvp[SWITCH_ACL_IP_FIELD_MAX];
      // TCP dest port equal to 179(BGP), copy to cpu
      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      SWITCH_MEMSET(&switch_range, 0, sizeof(switch_range_t));

      switch_range.start_value = SWITCH_HOSTIF_BGP_PORT;
      switch_range.end_value = SWITCH_HOSTIF_BGP_PORT;
      status = switch_api_acl_range_create(device,
                                           SWITCH_API_DIRECTION_INGRESS,
                                           SWITCH_RANGE_TYPE_DST_PORT,
                                           &switch_range,
                                           &range_handle);

      rcode_info->range_handles[range_index++] = range_handle;

      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_L4_DEST_PORT_RANGE;
      acl_kvp[field_count].value.dport_range_handle = range_handle;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_IP_PROTO;
      acl_kvp[field_count].value.ip_proto = SWITCH_HOSTIF_IP_PROTO_TCP;
      acl_kvp[field_count].mask.u.mask = 0xFF;
      field_count++;
      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      // TCP src port equal to 179(BGP), copy to cpu
      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      SWITCH_MEMSET(&switch_range, 0, sizeof(switch_range_t));

      switch_range.start_value = SWITCH_HOSTIF_BGP_PORT;
      switch_range.end_value = SWITCH_HOSTIF_BGP_PORT;
      status = switch_api_acl_range_create(device,
                                           SWITCH_API_DIRECTION_INGRESS,
                                           SWITCH_RANGE_TYPE_SRC_PORT,
                                           &switch_range,
                                           &range_handle);

      rcode_info->range_handles[range_index++] = range_handle;

      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT_RANGE;
      acl_kvp[field_count].value.sport_range_handle = range_handle;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_IP_PROTO;
      acl_kvp[field_count].value.ip_proto = SWITCH_HOSTIF_IP_PROTO_TCP;
      acl_kvp[field_count].mask.u.mask = 0xFF;
      field_count++;

      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_SYSTEM,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &system_acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_system_key_value_pair_t
          system_acl_kvp[SWITCH_ACL_SYSTEM_FIELD_MAX];
      SWITCH_MEMSET(&system_acl_kvp, 0, sizeof(system_acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_REASON_CODE;
      system_acl_kvp[field_count].value.reason_code =
          rcode_api_info->reason_code;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_IPV4_ENABLED;
      system_acl_kvp[field_count].value.ipv4_enabled = TRUE;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFFFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_FIB_HIT_MYIP;
      system_acl_kvp[field_count].value.fib_hit_myip = TRUE;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFFFFFF;
      field_count++;

      if (hostif_group_info) {
        opt_action_params.queue_id = queue_id;
        opt_action_params.meter_handle = hostif_group_info->policer_handle;
      }

      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority++,
                                          field_count,
                                          system_acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }
      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_BFD_RX: {
      switch_range_t switch_range;
      switch_handle_t range_handle = SWITCH_API_INVALID_HANDLE;

      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_IP,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_ip_key_value_pair_t acl_kvp[SWITCH_ACL_IP_FIELD_MAX];
      // UDP dest port equal to 3784(BFD), redirect to cpu
      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      SWITCH_MEMSET(&switch_range, 0, sizeof(switch_range_t));

      switch_range.start_value = SWITCH_HOSTIF_BFD_DST_PORT;
      switch_range.end_value = SWITCH_HOSTIF_BFD_DST_PORT;
      status = switch_api_acl_range_create(device,
                                           SWITCH_API_DIRECTION_INGRESS,
                                           SWITCH_RANGE_TYPE_DST_PORT,
                                           &switch_range,
                                           &range_handle);

      rcode_info->range_handles[range_index++] = range_handle;

      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_L4_DEST_PORT_RANGE;
      acl_kvp[field_count].value.dport_range_handle = range_handle;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_IP_PROTO;
      acl_kvp[field_count].value.ip_proto = SWITCH_HOSTIF_IP_PROTO_UDP;
      acl_kvp[field_count].mask.u.mask = 0xFF;
      field_count++;
      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }
      rcode_info->range_handles[range_index++] = range_handle;

      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_SYSTEM,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &system_acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_system_key_value_pair_t
          system_acl_kvp[SWITCH_ACL_SYSTEM_FIELD_MAX];
      SWITCH_MEMSET(&system_acl_kvp, 0, sizeof(system_acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_REASON_CODE;
      system_acl_kvp[field_count].value.reason_code =
          rcode_api_info->reason_code;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_IPV4_ENABLED;
      system_acl_kvp[field_count].value.ipv4_enabled = TRUE;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFFFFFF;
      field_count++;
      if (hostif_group_info) {
        opt_action_params.queue_id = queue_id;
        opt_action_params.meter_handle = hostif_group_info->policer_handle;
      }

      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority++,
                                          field_count,
                                          system_acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }
      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_PTP: {
      // Match on PTP ethertype and redirect the packet to CPU
      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_SYSTEM,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &system_acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_system_key_value_pair_t
          system_acl_kvp[SWITCH_ACL_SYSTEM_FIELD_MAX];
      SWITCH_MEMSET(&system_acl_kvp, 0, sizeof(system_acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_ETH_TYPE;
      system_acl_kvp[field_count].value.eth_type = SWITCH_ETHERTYPE_PTP;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;

      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      opt_action_params.counter_handle = counter_handle;
      if (hostif_group_info) {
        opt_action_params.queue_id = queue_id;
        opt_action_params.meter_handle = hostif_group_info->policer_handle;
      }

      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority,
                                          field_count,
                                          system_acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      /* Match on UDP Dest port and redirect-to-cpu */
      switch_range_t switch_range;
      switch_handle_t range_handle = SWITCH_API_INVALID_HANDLE;
      priority = 55;

      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_IP,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_ip_key_value_pair_t acl_kvp[SWITCH_ACL_IP_FIELD_MAX];
      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      SWITCH_MEMSET(&switch_range, 0, sizeof(switch_range_t));

      switch_range.start_value = SWITCH_HOSTIF_PTP_DST_PORT1;
      switch_range.end_value = SWITCH_HOSTIF_PTP_DST_PORT2;
      status = switch_api_acl_range_create(device,
                                           SWITCH_API_DIRECTION_INGRESS,
                                           SWITCH_RANGE_TYPE_DST_PORT,
                                           &switch_range,
                                           &range_handle);

      rcode_info->range_handles[range_index++] = range_handle;

      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_L4_DEST_PORT_RANGE;
      acl_kvp[field_count].value.dport_range_handle = range_handle;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_IP_PROTO;
      acl_kvp[field_count].value.ip_proto = SWITCH_HOSTIF_IP_PROTO_UDP;
      acl_kvp[field_count].mask.u.mask = 0xFF;
      field_count++;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      SWITCH_MEMSET(&system_acl_kvp, 0, sizeof(system_acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_REASON_CODE;
      system_acl_kvp[field_count].value.reason_code =
          rcode_api_info->reason_code;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      if (hostif_group_info) {
        opt_action_params.queue_id = queue_id;
        opt_action_params.meter_handle = hostif_group_info->policer_handle;
      }

      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority++,
                                          field_count,
                                          system_acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      /* Egress System ACL to match on reason code and insert timestamp header
       */
      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_EGRESS,
                                          SWITCH_ACL_TYPE_EGRESS_SYSTEM,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &egress_system_acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: egress system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_egress_system_key_value_pair_t
          egr_acl_kvp[SWITCH_ACL_EGRESS_SYSTEM_FIELD_MAX];
      SWITCH_MEMSET(&egr_acl_kvp, 0, sizeof(egr_acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      egr_acl_kvp[field_count].field =
          SWITCH_ACL_EGRESS_SYSTEM_FIELD_REASON_CODE;
      egr_acl_kvp[field_count].value.reason_code =
          SWITCH_HOSTIF_REASON_CODE_PTP;
      egr_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      acl_action = SWITCH_ACL_EGRESS_SYSTEM_ACTION_INSERT_CPU_TIMESTAMP;
      opt_action_params.counter_handle = counter_handle;
      status = switch_api_acl_rule_create(device,
                                          egress_system_acl_handle,
                                          priority,
                                          field_count,
                                          egr_acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: egress system acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_IPV6_NEIGHBOR_DISCOVERY: {
      // IPV6 ND packet, copy to cpu
      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_IPV6,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ipv6 acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_ipv6_key_value_pair_t acl_kvp[SWITCH_ACL_IPV6_FIELD_MAX];
      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      acl_kvp[field_count].field = SWITCH_ACL_IPV6_FIELD_IP_PROTO;
      acl_kvp[field_count].value.ip_proto = 58;
      acl_kvp[field_count].mask.u.mask.u.addr8[0] = 0xFF;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_IPV6_FIELD_ICMP_TYPE;
      acl_kvp[field_count].value.icmp_type = 135;
      acl_kvp[field_count].mask.u.mask.u.addr8[0] = 0xFF;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_IPV6_FIELD_IPV6_DEST;
      inet_pton(AF_INET6,
                "FF02::1:FF00:0",
                acl_kvp[field_count].value.ipv6_dest.u.addr8);
      inet_pton(AF_INET6,
                "FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FF00:0",
                acl_kvp[field_count].mask.u.mask.u.addr8);
      field_count++;
      acl_action = rcode_api_info->action;
      opt_action_params.counter_handle = counter_handle;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority++,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ipv6 acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      acl_kvp[field_count].field = SWITCH_ACL_IPV6_FIELD_IP_PROTO;
      acl_kvp[field_count].value.ip_proto = 58;
      acl_kvp[field_count].mask.u.mask.u.addr8[0] = 0xFF;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_IPV6_FIELD_ICMP_TYPE;
      acl_kvp[field_count].value.icmp_type = 136;
      acl_kvp[field_count].mask.u.mask.u.addr8[0] = 0xFF;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_IPV6_FIELD_IPV6_DEST;
      inet_pton(AF_INET6,
                "FF02::1:FF00:0",
                acl_kvp[field_count].value.ipv6_dest.u.addr8);
      inet_pton(AF_INET6,
                "FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FF00:0",
                acl_kvp[field_count].mask.u.mask.u.addr8);
      field_count++;
      acl_action = rcode_api_info->action;
      opt_action_params.counter_handle = counter_handle;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ipv6 acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_SYSTEM,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &system_acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_system_key_value_pair_t
          system_acl_kvp[SWITCH_ACL_SYSTEM_FIELD_MAX];
      SWITCH_MEMSET(&system_acl_kvp, 0, sizeof(system_acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_REASON_CODE;
      system_acl_kvp[field_count].value.reason_code =
          rcode_api_info->reason_code;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_IPV6_ENABLED;
      system_acl_kvp[field_count].value.ipv6_enabled = TRUE;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFFFFFF;
      field_count++;

      if (hostif_group_info) {
        opt_action_params.queue_id = queue_id;
        opt_action_params.meter_handle = hostif_group_info->policer_handle;
      }
      opt_action_params.counter_handle = system_counter_handle;

      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority++,
                                          field_count,
                                          system_acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_PIM: {
      // PIM packet, copy to cpu
      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_IP,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_ip_key_value_pair_t acl_kvp[SWITCH_ACL_IP_FIELD_MAX];
      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_IP_PROTO;
      acl_kvp[field_count].value.ip_proto = 103;
      acl_kvp[field_count].mask.u.mask = 0xFF;
      field_count++;
      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      opt_action_params.counter_handle = counter_handle;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_SYSTEM,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &system_acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_system_key_value_pair_t
          system_acl_kvp[SWITCH_ACL_SYSTEM_FIELD_MAX];
      SWITCH_MEMSET(&system_acl_kvp, 0, sizeof(system_acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_REASON_CODE;
      system_acl_kvp[field_count].value.reason_code =
          rcode_api_info->reason_code;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_IPV4_ENABLED;
      system_acl_kvp[field_count].value.ipv4_enabled = TRUE;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFFFFFF;
      field_count++;

      if (hostif_group_info) {
        opt_action_params.queue_id = queue_id;
        opt_action_params.meter_handle = hostif_group_info->policer_handle;
      }
      opt_action_params.counter_handle = system_counter_handle;

      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority++,
                                          field_count,
                                          system_acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_IGMP_TYPE_QUERY:
    case SWITCH_HOSTIF_REASON_CODE_IGMP_TYPE_LEAVE:
    case SWITCH_HOSTIF_REASON_CODE_IGMP_TYPE_V1_REPORT:
    case SWITCH_HOSTIF_REASON_CODE_IGMP_TYPE_V3_REPORT:
    case SWITCH_HOSTIF_REASON_CODE_IGMP_TYPE_V2_REPORT: {
      status = switch_hostif_rcode_handle_delete(device, handle);
      SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
      SWITCH_LOG_ERROR(
          "hostif reason code create failed on device %d "
          "reason code: %s is unsupported \n",
          device,
          switch_hostif_code_to_string(rcode_api_info->reason_code));
      return SWITCH_STATUS_NOT_SUPPORTED;
      break;
    }
    case SWITCH_HOSTIF_REASON_CODE_IGMP: {
      // IGMPv2 report packet, copy to cpu
      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_IP,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_ip_key_value_pair_t acl_kvp[SWITCH_ACL_IP_FIELD_MAX];
      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_IP_PROTO;
      acl_kvp[field_count].value.ip_proto = 2;
      acl_kvp[field_count].mask.u.mask = 0xFF;
      field_count++;
      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      opt_action_params.counter_handle = counter_handle;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl rule create failed(%s)\n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_SYSTEM,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &system_acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s)\n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_system_key_value_pair_t
          system_acl_kvp[SWITCH_ACL_SYSTEM_FIELD_MAX];
      SWITCH_MEMSET(&system_acl_kvp, 0, sizeof(system_acl_kvp));
      field_count = 0;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_REASON_CODE;
      system_acl_kvp[field_count].value.reason_code =
          rcode_api_info->reason_code;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;

      if (hostif_group_info) {
        opt_action_params.queue_id = queue_id;
        opt_action_params.meter_handle = hostif_group_info->policer_handle;
      }
      opt_action_params.counter_handle = system_counter_handle;

      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority++,
                                          field_count,
                                          system_acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl rule create failed(%s)\n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_ARP_REQUEST: {
      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_SYSTEM,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &system_acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s)\n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      // Broadcast ARP Request
      switch_acl_system_key_value_pair_t
          system_acl_kvp[SWITCH_ACL_SYSTEM_FIELD_MAX];
      SWITCH_MEMSET(&system_acl_kvp, 0, sizeof(system_acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_ETH_TYPE;
      system_acl_kvp[field_count].value.eth_type = SWITCH_ETHERTYPE_ARP;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_PACKET_TYPE;
      system_acl_kvp[field_count].value.packet_type =
          SWITCH_PACKET_TYPE_BROADCAST;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_ARP_OPCODE;
      system_acl_kvp[field_count].value.arp_opcode = SWITCH_ARP_OPCODE_REQ;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_IPV4_ENABLED;
      system_acl_kvp[field_count].value.ipv4_enabled = TRUE;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFFFFFF;
      field_count++;

      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      if (hostif_group_info) {
        opt_action_params.queue_id = queue_id;
        opt_action_params.meter_handle = hostif_group_info->policer_handle;
      }
      opt_action_params.counter_handle = system_counter_handle;

      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority,
                                          field_count,
                                          system_acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl rule create failed(%s)\n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      // Unicast ARP Request sent to router mac
      SWITCH_MEMSET(&system_acl_kvp, 0, sizeof(system_acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_ETH_TYPE;
      system_acl_kvp[field_count].value.eth_type = SWITCH_ETHERTYPE_ARP;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_PACKET_TYPE;
      system_acl_kvp[field_count].value.packet_type =
          SWITCH_PACKET_TYPE_UNICAST;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_ARP_OPCODE;
      system_acl_kvp[field_count].value.arp_opcode = SWITCH_ARP_OPCODE_REQ;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_IPV4_ENABLED;
      system_acl_kvp[field_count].value.ipv4_enabled = TRUE;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFFFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_RMAC_HIT;
      system_acl_kvp[field_count].value.rmac_hit = TRUE;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFFFFFF;
      field_count++;

      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      if (hostif_group_info) {
        opt_action_params.queue_id = queue_id;
        opt_action_params.meter_handle = hostif_group_info->policer_handle;
      }
      opt_action_params.counter_handle = system_counter_handle;

      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority++,
                                          field_count,
                                          system_acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl rule create failed(%s)\n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_ARP_RESPONSE: {
      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_SYSTEM,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &system_acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s)\n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      // Broadcast ARP Response (very rare)
      switch_acl_system_key_value_pair_t
          system_acl_kvp[SWITCH_ACL_SYSTEM_FIELD_MAX];
      SWITCH_MEMSET(&system_acl_kvp, 0, sizeof(system_acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_ETH_TYPE;
      system_acl_kvp[field_count].value.eth_type = SWITCH_ETHERTYPE_ARP;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_PACKET_TYPE;
      system_acl_kvp[field_count].value.packet_type =
          SWITCH_PACKET_TYPE_BROADCAST;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_ARP_OPCODE;
      system_acl_kvp[field_count].value.arp_opcode = SWITCH_ARP_OPCODE_RES;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_IPV4_ENABLED;
      system_acl_kvp[field_count].value.ipv4_enabled = TRUE;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFFFFFF;
      field_count++;

      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      if (hostif_group_info) {
        opt_action_params.queue_id = queue_id;
        opt_action_params.meter_handle = hostif_group_info->policer_handle;
      }
      opt_action_params.counter_handle = system_counter_handle;

      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority,
                                          field_count,
                                          system_acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl rule create failed(%s)\n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      // Unicast ARP Response sent to router mac
      SWITCH_MEMSET(&system_acl_kvp, 0, sizeof(system_acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_ETH_TYPE;
      system_acl_kvp[field_count].value.eth_type = SWITCH_ETHERTYPE_ARP;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_PACKET_TYPE;
      system_acl_kvp[field_count].value.packet_type =
          SWITCH_PACKET_TYPE_UNICAST;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_ARP_OPCODE;
      system_acl_kvp[field_count].value.arp_opcode = SWITCH_ARP_OPCODE_RES;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_IPV4_ENABLED;
      system_acl_kvp[field_count].value.ipv4_enabled = TRUE;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFFFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_RMAC_HIT;
      system_acl_kvp[field_count].value.rmac_hit = TRUE;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFFFFFF;
      field_count++;

      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      if (hostif_group_info) {
        opt_action_params.queue_id = queue_id;
        opt_action_params.meter_handle = hostif_group_info->policer_handle;
      }
      opt_action_params.counter_handle = system_counter_handle;

      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority++,
                                          field_count,
                                          system_acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl rule create failed(%s)\n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_TTL_ERROR: {
      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_SYSTEM,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &system_acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_system_key_value_pair_t acl_kvp[SWITCH_ACL_SYSTEM_FIELD_MAX];
      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_TTL;
      acl_kvp[field_count].value.ttl = 0x0;
      acl_kvp[field_count].mask.u.mask = 0xFF;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_ROUTED;
      acl_kvp[field_count].value.routed = 1;
      acl_kvp[field_count].mask.u.mask = 0xFF;
      field_count++;
      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      opt_action_params.counter_handle = counter_handle;
      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority,
                                          field_count,
                                          acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_TTL;
      acl_kvp[field_count].value.ttl = 0x1;
      acl_kvp[field_count].mask.u.mask = 0xFF;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_ROUTED;
      acl_kvp[field_count].value.routed = 1;
      acl_kvp[field_count].mask.u.mask = 0xFF;
      field_count++;
      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      opt_action_params.counter_handle = system_counter_handle;
      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority,
                                          field_count,
                                          acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_BROADCAST: {
      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_MAC,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: mac acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_mac_key_value_pair_t acl_kvp[SWITCH_ACL_MAC_FIELD_MAX];
      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      acl_kvp[field_count].field = SWITCH_ACL_MAC_FIELD_DEST_MAC;
      acl_kvp[field_count].value.dest_mac.mac_addr[0] = 0xFF;
      acl_kvp[field_count].value.dest_mac.mac_addr[1] = 0xFF;
      acl_kvp[field_count].value.dest_mac.mac_addr[2] = 0xFF;
      acl_kvp[field_count].value.dest_mac.mac_addr[3] = 0xFF;
      acl_kvp[field_count].value.dest_mac.mac_addr[4] = 0xFf;
      acl_kvp[field_count].value.dest_mac.mac_addr[5] = 0xFF;
      acl_kvp[field_count].mask.u.mask = 0xFFFFFFFFFFFF;
      field_count++;
      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      opt_action_params.counter_handle = counter_handle;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: mac acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_SYSTEM,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &system_acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_system_key_value_pair_t
          system_acl_kvp[SWITCH_ACL_SYSTEM_FIELD_MAX];
      SWITCH_MEMSET(&system_acl_kvp, 0, sizeof(system_acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_REASON_CODE;
      system_acl_kvp[field_count].value.reason_code =
          rcode_api_info->reason_code;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_IPV4_ENABLED;
      system_acl_kvp[field_count].value.ipv4_enabled = TRUE;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFFFFFF;
      field_count++;

      if (hostif_group_info) {
        opt_action_params.meter_handle = hostif_group_info->policer_handle;
        opt_action_params.queue_id = queue_id;
      }
      opt_action_params.counter_handle = system_counter_handle;

      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority++,
                                          field_count,
                                          system_acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_SFLOW_SAMPLE: {
      // SFLOW reasoncode is generated from different ACLs, which are created
      // when SFLOW session is created, the acl_handle is not stored here.
      rcode_info->acl_handle = SWITCH_API_INVALID_HANDLE;
      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_BGPV6: {
      switch_range_t switch_range;
      switch_handle_t range_handle = SWITCH_API_INVALID_HANDLE;

      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_IPV6,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ipv6 acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_ipv6_key_value_pair_t acl_kvp[SWITCH_ACL_IPV6_FIELD_MAX];
      // TCP dest port equal to 179(BGP), copy to cpu
      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      SWITCH_MEMSET(&switch_range, 0, sizeof(switch_range_t));

      switch_range.start_value = SWITCH_HOSTIF_BGP_PORT;
      switch_range.end_value = SWITCH_HOSTIF_BGP_PORT;
      status = switch_api_acl_range_create(device,
                                           SWITCH_API_DIRECTION_INGRESS,
                                           SWITCH_RANGE_TYPE_DST_PORT,
                                           &switch_range,
                                           &range_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: acl range create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }
      rcode_info->range_handles[range_index++] = range_handle;

      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_L4_DEST_PORT_RANGE;
      acl_kvp[field_count].value.dport_range_handle = range_handle;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_IP_PROTO;
      acl_kvp[field_count].value.ip_proto = SWITCH_HOSTIF_IP_PROTO_TCP;

      SWITCH_MEMSET(&acl_kvp[field_count].mask.u.mask,
                    255,
                    sizeof(acl_kvp[field_count].mask));
      field_count++;

      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority++,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);

      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ipv6 acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      // TCP src port equal to 179(BGP), copy to cpu
      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      SWITCH_MEMSET(&switch_range, 0, sizeof(switch_range_t));

      switch_range.start_value = SWITCH_HOSTIF_BGP_PORT;
      switch_range.end_value = SWITCH_HOSTIF_BGP_PORT;
      status = switch_api_acl_range_create(device,
                                           SWITCH_API_DIRECTION_INGRESS,
                                           SWITCH_RANGE_TYPE_SRC_PORT,
                                           &switch_range,
                                           &range_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: acl range create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      rcode_info->range_handles[range_index++] = range_handle;

      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT_RANGE;
      acl_kvp[field_count].value.sport_range_handle = range_handle;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_IP_PROTO;
      acl_kvp[field_count].value.ip_proto = SWITCH_HOSTIF_IP_PROTO_TCP;
      SWITCH_MEMSET(&acl_kvp[field_count].mask.u.mask,
                    255,
                    sizeof(acl_kvp[field_count].mask));
      field_count++;
      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority++,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_SYSTEM,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &system_acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_system_key_value_pair_t
          system_acl_kvp[SWITCH_ACL_SYSTEM_FIELD_MAX];
      SWITCH_MEMSET(&system_acl_kvp, 0, sizeof(system_acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_REASON_CODE;
      system_acl_kvp[field_count].value.reason_code =
          rcode_api_info->reason_code;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_IPV6_ENABLED;
      system_acl_kvp[field_count].value.ipv6_enabled = TRUE;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFFFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_FIB_HIT_MYIP;
      system_acl_kvp[field_count].value.fib_hit_myip = TRUE;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFFFFFF;
      field_count++;

      if (hostif_group_info) {
        opt_action_params.queue_id = queue_id;
        opt_action_params.meter_handle = hostif_group_info->policer_handle;
      }

      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority++,
                                          field_count,
                                          system_acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_EAPOL: {
      break;
    }
    case SWITCH_HOSTIF_REASON_CODE_PVRST: {
      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_DHCPV6: {
      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_VRRPV6: {
      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_IPV6_MLD_V1_V2: {
      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_IPV6_MLD_V1_REPORT: {
      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_IPV6_MLD_V1_DONE: {
      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_L3_MTU_ERROR: {
      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_SSH: {
      switch_range_t switch_range;
      switch_handle_t dport_range_handle = SWITCH_API_INVALID_HANDLE;
      switch_handle_t sport_range_handle = SWITCH_API_INVALID_HANDLE;

      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_IP,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_ip_key_value_pair_t acl_kvp[SWITCH_ACL_IP_FIELD_MAX];
      // TCP dest port equal to 22(SSH), redirect to cpu
      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      SWITCH_MEMSET(&switch_range, 0, sizeof(switch_range_t));

      switch_range.start_value = SWITCH_HOSTIF_SSH_PORT;
      switch_range.end_value = SWITCH_HOSTIF_SSH_PORT;
      status = switch_api_acl_range_create(device,
                                           SWITCH_API_DIRECTION_INGRESS,
                                           SWITCH_RANGE_TYPE_DST_PORT,
                                           &switch_range,
                                           &dport_range_handle);

      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_L4_DEST_PORT_RANGE;
      acl_kvp[field_count].value.dport_range_handle = dport_range_handle;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_IP_PROTO;
      acl_kvp[field_count].value.ip_proto = SWITCH_HOSTIF_IP_PROTO_TCP;
      acl_kvp[field_count].mask.u.mask = 0xFF;
      field_count++;

      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      opt_action_params.counter_handle = counter_handle;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      // TCP src port equal to 22(SSH), redirect to cpu
      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      SWITCH_MEMSET(&switch_range, 0, sizeof(switch_range_t));

      switch_range.start_value = SWITCH_HOSTIF_SSH_PORT;
      switch_range.end_value = SWITCH_HOSTIF_SSH_PORT;
      status = switch_api_acl_range_create(device,
                                           SWITCH_API_DIRECTION_INGRESS,
                                           SWITCH_RANGE_TYPE_SRC_PORT,
                                           &switch_range,
                                           &sport_range_handle);

      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT_RANGE;
      acl_kvp[field_count].value.sport_range_handle = sport_range_handle;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_IP_PROTO;
      acl_kvp[field_count].value.ip_proto = SWITCH_HOSTIF_IP_PROTO_TCP;
      acl_kvp[field_count].mask.u.mask = 0xFF;
      field_count++;

      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      opt_action_params.counter_handle = counter_handle;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT_RANGE;
      acl_kvp[field_count].value.sport_range_handle = sport_range_handle;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_L4_DEST_PORT_RANGE;
      acl_kvp[field_count].value.dport_range_handle = dport_range_handle;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_IP_PROTO;
      acl_kvp[field_count].value.ip_proto = SWITCH_HOSTIF_IP_PROTO_TCP;
      acl_kvp[field_count].mask.u.mask = 0xFF;
      field_count++;
      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      opt_action_params.counter_handle = counter_handle;

      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_SYSTEM,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &system_acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_system_key_value_pair_t
          system_acl_kvp[SWITCH_ACL_SYSTEM_FIELD_MAX];
      SWITCH_MEMSET(&system_acl_kvp, 0, sizeof(system_acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_REASON_CODE;
      system_acl_kvp[field_count].value.reason_code =
          rcode_api_info->reason_code;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_FIB_HIT_MYIP;
      system_acl_kvp[field_count].value.fib_hit_myip = TRUE;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFFFFFF;
      field_count++;

      if (hostif_group_info) {
        opt_action_params.queue_id = queue_id;
        opt_action_params.meter_handle = hostif_group_info->policer_handle;
      }
      opt_action_params.counter_handle = system_counter_handle;
      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;

      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority++,
                                          field_count,
                                          system_acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      rcode_info->range_handles[range_index++] = sport_range_handle;
      rcode_info->range_handles[range_index++] = dport_range_handle;
      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_DHCP: {
      switch_range_t switch_range;
      switch_handle_t port1_src_range_handle = SWITCH_API_INVALID_HANDLE;
      switch_handle_t port1_dst_range_handle = SWITCH_API_INVALID_HANDLE;
      switch_handle_t port2_src_range_handle = SWITCH_API_INVALID_HANDLE;
      switch_handle_t port2_dst_range_handle = SWITCH_API_INVALID_HANDLE;
      SWITCH_MEMSET(&switch_range, 0, sizeof(switch_range_t));

      switch_range.start_value = SWITCH_HOSTIF_DHCP_PORT1;
      switch_range.end_value = SWITCH_HOSTIF_DHCP_PORT1;

      status = switch_api_acl_range_create(device,
                                           SWITCH_API_DIRECTION_INGRESS,
                                           SWITCH_RANGE_TYPE_SRC_PORT,
                                           &switch_range,
                                           &port1_src_range_handle);
      status = switch_api_acl_range_create(device,
                                           SWITCH_API_DIRECTION_INGRESS,
                                           SWITCH_RANGE_TYPE_DST_PORT,
                                           &switch_range,
                                           &port1_dst_range_handle);

      SWITCH_MEMSET(&switch_range, 0, sizeof(switch_range_t));

      switch_range.start_value = SWITCH_HOSTIF_DHCP_PORT2;
      switch_range.end_value = SWITCH_HOSTIF_DHCP_PORT2;

      status = switch_api_acl_range_create(device,
                                           SWITCH_API_DIRECTION_INGRESS,
                                           SWITCH_RANGE_TYPE_SRC_PORT,
                                           &switch_range,
                                           &port2_src_range_handle);
      status = switch_api_acl_range_create(device,
                                           SWITCH_API_DIRECTION_INGRESS,
                                           SWITCH_RANGE_TYPE_DST_PORT,
                                           &switch_range,
                                           &port2_dst_range_handle);

      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_IP,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_ip_key_value_pair_t acl_kvp[SWITCH_ACL_IP_FIELD_MAX];
      // UDP sport equal to 67 and dport equal to 68(DHCP), redirect to cpu
      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT_RANGE;
      acl_kvp[field_count].value.sport_range_handle = port1_src_range_handle;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_L4_DEST_PORT_RANGE;
      acl_kvp[field_count].value.dport_range_handle = port2_dst_range_handle;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_IP_PROTO;
      acl_kvp[field_count].value.ip_proto = SWITCH_HOSTIF_IP_PROTO_UDP;
      acl_kvp[field_count].mask.u.mask = 0xFF;
      field_count++;
      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      opt_action_params.counter_handle = counter_handle;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      // UDP sport equal to 68 and dport equal to 67(DHCP), redirect to cpu
      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_L4_SOURCE_PORT_RANGE;
      acl_kvp[field_count].value.sport_range_handle = port2_src_range_handle;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_L4_DEST_PORT_RANGE;
      acl_kvp[field_count].value.dport_range_handle = port1_dst_range_handle;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_IP_PROTO;
      acl_kvp[field_count].value.ip_proto = SWITCH_HOSTIF_IP_PROTO_UDP;
      acl_kvp[field_count].mask.u.mask = 0xFF;
      field_count++;
      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      opt_action_params.counter_handle = counter_handle;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_SYSTEM,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &system_acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_system_key_value_pair_t
          system_acl_kvp[SWITCH_ACL_SYSTEM_FIELD_MAX];
      SWITCH_MEMSET(&system_acl_kvp, 0, sizeof(system_acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_REASON_CODE;
      system_acl_kvp[field_count].value.reason_code =
          rcode_api_info->reason_code;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      opt_action_params.counter_handle = system_counter_handle;
      if (hostif_group_info) {
        opt_action_params.queue_id = queue_id;
        opt_action_params.meter_handle = hostif_group_info->policer_handle;
      }
      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority++,
                                          field_count,
                                          system_acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }
      rcode_info->range_handles[range_index++] = port1_src_range_handle;
      rcode_info->range_handles[range_index++] = port1_dst_range_handle;
      rcode_info->range_handles[range_index++] = port2_src_range_handle;
      rcode_info->range_handles[range_index++] = port2_dst_range_handle;
      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_SNMP: {
      switch_range_t switch_range;
      switch_handle_t range_handle = SWITCH_API_INVALID_HANDLE;

      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_IP,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_ip_key_value_pair_t acl_kvp[SWITCH_ACL_IP_FIELD_MAX];
      // UDP dest port equal to 161(SNMP), redirect to cpu
      SWITCH_MEMSET(&acl_kvp, 0, sizeof(acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      SWITCH_MEMSET(&switch_range, 0, sizeof(switch_range_t));

      switch_range.start_value = SWITCH_HOSTIF_SNMP_PORT;
      switch_range.end_value = SWITCH_HOSTIF_SNMP_PORT;
      status = switch_api_acl_range_create(device,
                                           SWITCH_API_DIRECTION_INGRESS,
                                           SWITCH_RANGE_TYPE_DST_PORT,
                                           &switch_range,
                                           &range_handle);

      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_L4_DEST_PORT_RANGE;
      acl_kvp[field_count].value.dport_range_handle = range_handle;
      field_count++;
      acl_kvp[field_count].field = SWITCH_ACL_IP_FIELD_IP_PROTO;
      acl_kvp[field_count].value.ip_proto = SWITCH_HOSTIF_IP_PROTO_UDP;
      acl_kvp[field_count].mask.u.mask = 0xFF;
      field_count++;
      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      opt_action_params.counter_handle = counter_handle;
      status = switch_api_acl_rule_create(device,
                                          acl_handle,
                                          priority,
                                          field_count,
                                          acl_kvp,
                                          SWITCH_ACL_ACTION_PERMIT,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: ip acl rule create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_SYSTEM,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &system_acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_system_key_value_pair_t
          system_acl_kvp[SWITCH_ACL_SYSTEM_FIELD_MAX];
      SWITCH_MEMSET(&system_acl_kvp, 0, sizeof(system_acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_REASON_CODE;
      system_acl_kvp[field_count].value.reason_code =
          rcode_api_info->reason_code;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFF;
      field_count++;
      if (hostif_group_info) {
        opt_action_params.queue_id = queue_id;
        opt_action_params.meter_handle = hostif_group_info->policer_handle;
      }
      opt_action_params.counter_handle = system_counter_handle;

      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority++,
                                          field_count,
                                          system_acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }

      rcode_info->range_handles[range_index++] = range_handle;
      break;
    }

    case SWITCH_HOSTIF_REASON_CODE_MYIP:

      status = switch_api_acl_list_create(device,
                                          SWITCH_API_DIRECTION_INGRESS,
                                          SWITCH_ACL_TYPE_SYSTEM,
                                          SWITCH_HANDLE_TYPE_NONE,
                                          &system_acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s(%d): system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            rcode_api_info->reason_code,
            switch_error_to_string(status));
        goto cleanup;
      }

      switch_acl_system_key_value_pair_t
          system_acl_kvp[SWITCH_ACL_SYSTEM_FIELD_MAX];
      SWITCH_MEMSET(&system_acl_kvp, 0, sizeof(system_acl_kvp));
      SWITCH_MEMSET(&action_params, 0, sizeof(switch_acl_action_params_t));
      SWITCH_MEMSET(
          &opt_action_params, 0, sizeof(switch_acl_opt_action_params_t));
      field_count = 0;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_IPV4_ENABLED;
      system_acl_kvp[field_count].value.ipv4_enabled = TRUE;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFFFFFF;
      field_count++;
      system_acl_kvp[field_count].field = SWITCH_ACL_SYSTEM_FIELD_FIB_HIT_MYIP;
      system_acl_kvp[field_count].value.fib_hit_myip = TRUE;
      system_acl_kvp[field_count].mask.u.mask = 0xFFFFFFFF;
      field_count++;

      acl_action = rcode_api_info->action;
      action_params.cpu_redirect.reason_code = rcode_api_info->reason_code;
      opt_action_params.counter_handle = system_counter_handle;
      if (hostif_group_info) {
        opt_action_params.queue_id = queue_id;
        opt_action_params.meter_handle = hostif_group_info->policer_handle;
      }

      status = switch_api_acl_rule_create(device,
                                          system_acl_handle,
                                          priority,
                                          field_count,
                                          system_acl_kvp,
                                          acl_action,
                                          &action_params,
                                          &opt_action_params,
                                          &ace_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code create failed on device %d "
            "reason code %s: system acl list create failed(%s) \n",
            device,
            switch_hostif_code_to_string(rcode_api_info->reason_code),
            switch_error_to_string(status));
        goto cleanup;
      }
      break;

    default:
      status = SWITCH_STATUS_NOT_SUPPORTED;
      break;
  }

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("hostif reason code create failed for code %d: %s\n",
                     rcode_api_info->reason_code,
                     switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "hostif reason code created on device %d rcode handle 0x%lx "
      "acl handle 0x%lx system acl handle 0x%lx\n",
      device,
      handle,
      acl_handle,
      system_acl_handle);

  rcode_info->acl_handle = acl_handle;
  rcode_info->system_acl_handle = system_acl_handle;
  rcode_info->counter_handle = counter_handle;
  rcode_info->system_counter_handle = system_counter_handle;
  *rcode_handle = handle;

  return status;

cleanup:
  return status;
}

static int switch_hostif_set_interface_oper_state(
    const switch_device_t device,
    const char *intf_name,
    bool state,
    switch_hostif_info_t *hostif_info) {
  int err;
  struct nl_sock *sock;
  struct rtnl_link *p_rtnl_link, *change;
  unsigned char opstate = 0;
  bool sock_created = false;

  if (hostif_info->hostif.nl_sock_get_fn == NULL) {
    sock = nl_socket_alloc();
    if (!sock) {
      SWITCH_LOG_ERROR("could not allocate netlink socket.\n");
      return -1;
    }
    // connect to socket
    if ((err = nl_connect(sock, NETLINK_ROUTE))) {
      SWITCH_LOG_ERROR("netlink error: %s\n", nl_geterror(err));
      nl_socket_free(sock);
      return -1;
    }
    sock_created = true;
  } else {
    sock = hostif_info->hostif.nl_sock_get_fn(intf_name,
                                              hostif_info->hostif_handle);
    if (!sock) {
      SWITCH_LOG_ERROR("could not get netlink socket via cb.\n");
      return -1;
    }
  }

  nl_socket_disable_seq_check(sock);
  if (rtnl_link_get_kernel(sock, 0, intf_name, &p_rtnl_link) < 0)
    SWITCH_LOG_ERROR("Cannot get link by name %s\n", intf_name);

  change = rtnl_link_alloc();
  opstate = rtnl_link_get_operstate(p_rtnl_link);
  rtnl_link_set_operstate(change, opstate);
  if (state == 0) {
    rtnl_link_set_carrier(change, 0);
    rtnl_link_set_operstate(change, IF_OPER_LOWERLAYERDOWN);
  } else {
    rtnl_link_set_carrier(change, 1);
    rtnl_link_set_operstate(change, IF_OPER_UP);
  }

  err = rtnl_link_change(sock, p_rtnl_link, change, 0);

  rtnl_link_put(p_rtnl_link);
  rtnl_link_put(change);

  if (sock_created) {
    nl_close(sock);
    nl_socket_free(sock);
  }
  return 0;
}

switch_status_t switch_api_hostif_oper_state_set_internal(
    switch_device_t device, switch_handle_t hostif_handle, bool oper_state) {
  switch_hostif_info_t *hostif_info = NULL;
  switch_status_t status = SWITCH_STATUS_FAILURE;

  SWITCH_ASSERT(SWITCH_HOSTIF_HANDLE(hostif_handle));
  if (hostif_handle) {
    status = switch_hostif_get(device, hostif_handle, &hostif_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "hostif set state failed on device %d: "
          "hostif get failed:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
    status = switch_hostif_set_interface_oper_state(
        device, hostif_info->hostif.intf_name, oper_state, hostif_info);
    hostif_info->hostif.operstatus = oper_state;
  }

  return status;
}

switch_status_t switch_api_hostif_oper_state_get_internal(
    switch_device_t device, switch_handle_t hostif_handle, bool *oper_state) {
  switch_hostif_info_t *hostif_info = NULL;
  switch_status_t status = SWITCH_STATUS_FAILURE;

  SWITCH_ASSERT(SWITCH_HOSTIF_HANDLE(hostif_handle));
  if (hostif_handle) {
    status = switch_hostif_get(device, hostif_handle, &hostif_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "hostif get state failed on device %d: "
          "hostif get failed:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
    *oper_state = hostif_info->hostif.operstatus;
  }
  return status;
}

switch_status_t switch_api_hostif_reason_code_update_internal(
    const switch_device_t device,
    const switch_handle_t hostif_rcode_handle,
    const switch_uint64_t flags,
    const switch_api_hostif_rcode_info_t *rcode_api_info) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  return status;
}

switch_status_t switch_api_hostif_reason_code_delete_internal(
    const switch_device_t device, const switch_handle_t rcode_handle) {
  switch_hostif_context_t *hostif_ctx = NULL;
  switch_hostif_rcode_info_t *rcode_info = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_HOSTIF_RCODE_HANDLE(rcode_handle));
  if (!SWITCH_HOSTIF_RCODE_HANDLE(rcode_handle)) {
    SWITCH_LOG_ERROR(
        "hostif reason code delete failed on "
        "device %d handle 0x%lx: "
        "rcode handle invalid(%s)\n",
        device,
        rcode_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_HOSTIF, (void **)&hostif_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif reason code delete failed on "
        "device %d handle 0x%lx: "
        "hostif context get failed(%s)\n",
        device,
        rcode_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_hostif_rcode_get(device, rcode_handle, &rcode_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif reason code delete failed on "
        "device %d handle 0x%lx: "
        "hostif rcode get failed(%s)\n",
        device,
        rcode_handle,
        switch_error_to_string(status));
    return status;
  }

  hostif_ctx->rcode_handles[rcode_info->rcode_api_info.reason_code] =
      SWITCH_API_INVALID_HANDLE;

  if (rcode_info->rcode_api_info.reason_code !=
      SWITCH_HOSTIF_REASON_CODE_NONE) {
    if (SWITCH_ACL_HANDLE(rcode_info->acl_handle)) {
      status = switch_api_acl_list_delete(device, rcode_info->acl_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code delete failed on "
            "device %d handle 0x%lx: "
            "hostif acl list delete failed(%s)\n",
            device,
            rcode_handle,
            switch_error_to_string(status));
        return status;
      }
    }

    if (SWITCH_ACL_COUNTER_HANDLE(rcode_info->counter_handle)) {
      status =
          switch_api_acl_counter_delete(device, rcode_info->counter_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code delete failed on "
            "device %d handle 0x%lx: "
            "hostif counter delete failed(%s)\n",
            device,
            rcode_handle,
            switch_error_to_string(status));
        return status;
      }
    }

    if (SWITCH_ACL_COUNTER_HANDLE(rcode_info->system_counter_handle)) {
      status = switch_api_acl_counter_delete(device,
                                             rcode_info->system_counter_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif reason code delete failed on "
            "device %d handle 0x%lx: "
            "hostif counter delete failed(%s)\n",
            device,
            rcode_handle,
            switch_error_to_string(status));
        return status;
      }
    }
  }

  for (index = 0; index < SWITCH_HOSTIF_RANGE_HANDLE_MAX; index++) {
    if (SWITCH_RANGE_HANDLE(rcode_info->range_handles[index])) {
      status =
          switch_api_acl_range_delete(device, rcode_info->range_handles[index]);
      SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS)
    }
  }

  if (SWITCH_ACL_HANDLE(rcode_info->system_acl_handle)) {
    status = switch_api_acl_list_delete(device, rcode_info->system_acl_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "hostif reason code delete failed on "
          "device %d handle 0x%lx: "
          "hostif system acl list delete ailed(%s)\n",
          device,
          rcode_handle,
          switch_error_to_string(status));
      return status;
    }
  }

  status = switch_hostif_rcode_handle_delete(device, rcode_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_DEBUG(
      "hostif reason code handle deleted on device %d handle 0x%lx\n",
      device,
      rcode_handle);

  return status;
}

switch_status_t switch_api_hostif_reason_code_get(
    const switch_device_t device,
    const switch_handle_t rcode_handle,
    switch_hostif_reason_code_t *reason_code) {
  switch_hostif_context_t *hostif_ctx = NULL;
  switch_hostif_rcode_info_t *rcode_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(SWITCH_HOSTIF_RCODE_HANDLE(rcode_handle));
  if (!SWITCH_HOSTIF_RCODE_HANDLE(rcode_handle)) {
    SWITCH_LOG_ERROR(
        "hostif reason code delete failed on "
        "device %d handle 0x%lx: "
        "rcode handle invalid(%s)\n",
        device,
        rcode_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_HOSTIF, (void **)&hostif_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif reason code delete failed on "
        "device %d handle 0x%lx: "
        "hostif context get failed(%s)\n",
        device,
        rcode_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_hostif_rcode_get(device, rcode_handle, &rcode_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif reason code delete failed on "
        "device %d handle 0x%lx: "
        "hostif rcode get failed(%s)\n",
        device,
        rcode_handle,
        switch_error_to_string(status));
    return status;
  }
  return status;
}

switch_status_t switch_api_hostif_rx_callback_register_internal(
    switch_device_t device,
    switch_app_id_t app_id,
    switch_hostif_rx_callback_fn cb_fn,
    void *cookie) {
  switch_hostif_context_t *hostif_ctx = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_HOSTIF, (void **)&hostif_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif rx callback register failed on device %d "
        "app id %d: hostif context get failed:(%s)\n",
        device,
        app_id,
        switch_error_to_string(status));
    return status;
  }

  for (index = 0; index < SWITCH_MAX_RX_CALLBACK; index++) {
    if (hostif_ctx->rx_cb_list[index].valid) {
      if (hostif_ctx->rx_cb_list[index].app_id == app_id) {
        hostif_ctx->rx_cb_list[index].cb_fn = cb_fn;
        hostif_ctx->rx_cb_list[index].cookie = cookie;
        return status;
      }
    }
  }

  status = SWITCH_STATUS_INSUFFICIENT_RESOURCES;
  for (index = 0; index < SWITCH_MAX_RX_CALLBACK; index++) {
    if (!hostif_ctx->rx_cb_list[index].valid) {
      hostif_ctx->rx_cb_list[index].valid = TRUE;
      hostif_ctx->rx_cb_list[index].app_id = app_id;
      hostif_ctx->rx_cb_list[index].cb_fn = cb_fn;
      hostif_ctx->rx_cb_list[index].cookie = cookie;
      status = SWITCH_STATUS_SUCCESS;
      return status;
    }
  }

  SWITCH_LOG_DEBUG(
      "hostif rx callback registered on device %d app id %d\n", device, app_id);

  return status;
}

switch_status_t switch_api_hostif_rx_callback_deregister_internal(
    switch_device_t device,
    switch_app_id_t app_id,
    switch_hostif_rx_callback_fn cb_fn) {
  switch_hostif_context_t *hostif_ctx = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_HOSTIF, (void **)&hostif_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif rx callback deregister failed on device %d "
        "app id %d: hostif context get failed:(%s)\n",
        device,
        app_id,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_STATUS_ITEM_NOT_FOUND;
  for (index = 0; index < SWITCH_MAX_RX_CALLBACK; index++) {
    if (hostif_ctx->rx_cb_list[index].valid) {
      if (hostif_ctx->rx_cb_list[index].app_id == app_id) {
        hostif_ctx->rx_cb_list[index].valid = FALSE;
        hostif_ctx->rx_cb_list[index].cb_fn = NULL;
        hostif_ctx->rx_cb_list[index].cookie = NULL;
        hostif_ctx->rx_cb_list[index].app_id = 0;
        status = SWITCH_STATUS_SUCCESS;
        return status;
      }
    }
  }

  SWITCH_LOG_DEBUG("hostif rx callback deregistered on device %d app id %d\n",
                   device,
                   app_id);

  return status;
}

switch_status_t switch_api_hostif_tx_packet(
    switch_hostif_packet_t *hostif_packet) {
  switch_packet_info_t *pkt_info_tmp = NULL;
  switch_packet_info_t pkt_info;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_bd_t bd = 0;
  switch_dev_port_t dev_port = 0;
  switch_device_t device = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_MEMSET(&pkt_info, 0x0, sizeof(pkt_info));
  pkt_info_tmp = &pkt_info;
  pkt_info.pkt = hostif_packet->pkt;
  pkt_info.pkt_size = hostif_packet->pkt_size;
  pkt_info.pkt_type = SWITCH_PKTDRIVER_PACKET_TYPE_TX_CB;

  device = hostif_packet->device;
  pkt_info.device = device;

  if (hostif_packet->bypass_flags == SWITCH_BYPASS_ALL) {
    SWITCH_ASSERT(SWITCH_PORT_HANDLE(hostif_packet->handle));
    status = SWITCH_STATUS_INVALID_PARAMETER;
    if (!SWITCH_PORT_HANDLE(hostif_packet->handle)) {
      SWITCH_LOG_ERROR(
          "hostif packet tx failed: "
          "port handle invalid 0x%lx\n",
          hostif_packet->handle);
      return status;
    }

    SWITCH_PORT_DEV_PORT_GET(device, hostif_packet->handle, dev_port, status);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    SWITCH_PKTINFO_TX_BYPASS(pkt_info_tmp) = TRUE;
  } else {
    status =
        switch_bd_handle_get(device, hostif_packet->network_handle, &bd_handle);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    bd = handle_to_id(bd_handle);
  }

  SWITCH_PKTINFO_TX_DEV_PORT(pkt_info_tmp) = dev_port;
  SWITCH_PKTINFO_INGRESS_BD(pkt_info_tmp) = bd;
  SWITCH_PKTINFO_BYPASS_FLAGS(pkt_info_tmp) = hostif_packet->bypass_flags;

  status = switch_pktdriver_tx(&pkt_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif packet tx failed:"
        "pktdriver cpu tx failed:(%s)\n",
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_hostif_rx_filter_create_internal(
    const switch_device_t device,
    const switch_hostif_rx_filter_priority_t priority,
    const switch_uint64_t flags,
    const switch_hostif_rx_filter_key_t *rx_key,
    const switch_hostif_rx_filter_action_t *rx_action,
    switch_handle_t *rx_filter_handle) {
  switch_hostif_rx_filter_info_t *rx_filter_info = NULL;
  switch_hostif_info_t *hostif_info = NULL;
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_pktdriver_rx_filter_key_t rx_nf_key;
  switch_pktdriver_rx_filter_action_t rx_nf_action;
  switch_pktdriver_rx_filter_priority_t rx_nf_priority = 0;
  switch_uint64_t rx_nf_flags = 0;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_knet_filter_t knet_filter_handle = 0;

  UNUSED(knet_filter_handle);
  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(rx_key && rx_action);
  if (!rx_key || !rx_action) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hostif rx filter create failed on device %d: "
        "parameters invalid:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  handle = switch_hostif_rx_filter_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "hostif rx filter create failed on device %d: "
        "handle allocate failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_hostif_rx_filter_get(device, handle, &rx_filter_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif rx filter create failed on device %d: "
        "hostif rx filter get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (flags & SWITCH_HOSTIF_RX_FILTER_ATTR_GLOBAL) {
    *rx_filter_handle = handle;
    rx_filter_info->flags = SWITCH_HOSTIF_RX_FILTER_ATTR_GLOBAL;
    return status;
  }

  if (flags & SWITCH_HOSTIF_RX_FILTER_ATTR_PORT_HANDLE) {
    SWITCH_ASSERT(SWITCH_PORT_HANDLE(rx_key->port_handle));
    SWITCH_PORT_DEV_PORT_GET(
        device, rx_key->port_handle, rx_filter_info->dev_port, status);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    if (status != SWITCH_STATUS_SUCCESS) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR(
          "hostif rx filter create failed on device %d: "
          "port handle invalid:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  if (flags & SWITCH_HOSTIF_RX_FILTER_ATTR_INTF_HANDLE) {
    SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(rx_key->intf_handle));
    SWITCH_IFINDEX_GET(
        device, rx_key->intf_handle, rx_filter_info->ifindex, status);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "hostif rx filter create failed on device %d: "
          "interface handle invalid:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  if (flags & SWITCH_HOSTIF_RX_FILTER_ATTR_HANDLE) {
    status = switch_bd_handle_get(device, rx_key->handle, &bd_handle);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "hostif rx filter create failed on device %d: "
          "handle invalid:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
    rx_filter_info->bd = handle_to_id(bd_handle);
  }

  if (flags & SWITCH_HOSTIF_RX_FILTER_ATTR_REASON_CODE) {
    SWITCH_ASSERT(rx_key->reason_code < SWITCH_HOSTIF_REASON_CODE_MAX);
    rx_filter_info->reason_code = rx_key->reason_code;
    rx_filter_info->reason_code_mask = rx_key->reason_code_mask;
  }

  if (rx_action->hostif_handle) {
    SWITCH_ASSERT(SWITCH_HOSTIF_HANDLE(rx_action->hostif_handle));
    if (!SWITCH_HOSTIF_HANDLE(rx_action->hostif_handle)) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_ERROR(
          "hostif rx filter create failed on device %d: "
          "hostif handle invalid:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }

    status = switch_hostif_get(device, rx_action->hostif_handle, &hostif_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "hostif rx filter create failed on device %d: "
          "hostif get failed:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }

    rx_filter_info->hostif_fd = hostif_info->hostif_fd;
    rx_filter_info->knet_hostif_handle = hostif_info->knet_hostif_handle;
  }

  rx_filter_info->priority = priority;

  SWITCH_MEMSET(&rx_nf_key, 0x0, sizeof(rx_nf_key));
  SWITCH_MEMSET(&rx_nf_action, 0x0, sizeof(rx_nf_action));
  SWITCH_MEMCPY(
      &rx_filter_info->rx_key, rx_key, sizeof(switch_hostif_rx_filter_key_t));
  SWITCH_MEMCPY(&rx_filter_info->rx_action,
                rx_action,
                sizeof(switch_hostif_rx_filter_action_t));
  rx_nf_key.dev_port = rx_filter_info->dev_port;
  rx_nf_key.ifindex = rx_filter_info->ifindex;
  rx_nf_key.bd = rx_filter_info->bd;
  rx_nf_key.reason_code = rx_filter_info->reason_code;
  rx_nf_key.reason_code_mask = rx_filter_info->reason_code_mask;
  rx_nf_action.fd = rx_filter_info->hostif_fd;
  rx_nf_action.knet_hostif_handle = rx_filter_info->knet_hostif_handle;
  rx_nf_action.vlan_action = switch_hostif_vlan_action_to_pktdriver_vlan_action(
      rx_action->vlan_action);
  rx_nf_priority = switch_hostif_rx_priority_to_pktdriver_rx_priority(priority);
  rx_nf_flags = switch_hostif_rx_flags_to_pktdriver_rx_flags(flags);

  status = switch_pktdriver_rx_filter_create(device,
                                             rx_nf_priority,
                                             rx_nf_flags,
                                             &rx_nf_key,
                                             &rx_nf_action,
                                             &rx_filter_info->filter_handle);

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif rx filter create failed on device %d: "
        "pkt driver filter add failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  pktdriver_ctx = switch_config_packet_driver_context_get();
  if (pktdriver_ctx->knet_pkt_driver[device]) {
    rx_filter_info->knet_filter_handle = rx_filter_info->filter_handle;
    rx_filter_info->filter_handle = SWITCH_API_INVALID_HANDLE;
  }

  *rx_filter_handle = handle;

  SWITCH_LOG_DEBUG(
      "hositf rx filter created on device %d handle 0x%lx\n", device, handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_hostif_rx_filter_delete_internal(
    const switch_device_t device, const switch_handle_t rx_filter_handle) {
  switch_hostif_rx_filter_info_t *rx_filter_info = NULL;
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_HOSTIF_RX_FILTER_HANDLE(rx_filter_handle));
  if (!SWITCH_HOSTIF_RX_FILTER_HANDLE(rx_filter_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hostif rx filter delete failed on device %d: "
        "rx filter handle invalid:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_hostif_rx_filter_get(device, rx_filter_handle, &rx_filter_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif rx filter delete failed on device %d: "
        "hostif rx filter get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (rx_filter_info->flags != SWITCH_HOSTIF_RX_FILTER_ATTR_GLOBAL) {
    pktdriver_ctx = switch_config_packet_driver_context_get();
    if (pktdriver_ctx->knet_pkt_driver[device]) {
      status = switch_pktdriver_rx_filter_delete(
          device, rx_filter_info->knet_filter_handle);
    } else {
      status = switch_pktdriver_rx_filter_delete(device,
                                                 rx_filter_info->filter_handle);
    }
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "hostif rx filter delete failed on device %d: "
          "pkt driver filter delete failed:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
  }

  status = switch_hostif_rx_filter_handle_delete(device, rx_filter_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif rx filter delete failed on device %d handle 0x%lx : "
        "hostif rx filter delete failed:(%s)\n",
        device,
        rx_filter_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("hositf rx filter deleted on device %d handle 0x%lx\n",
                   device,
                   rx_filter_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_hostif_tx_filter_create_internal(
    const switch_device_t device,
    const switch_hostif_tx_filter_priority_t priority,
    const switch_uint64_t flags,
    const switch_hostif_tx_filter_key_t *tx_key,
    const switch_hostif_tx_filter_action_t *tx_action,
    switch_handle_t *tx_filter_handle) {
  switch_hostif_tx_filter_info_t *tx_filter_info = NULL;
  switch_pktdriver_tx_filter_key_t tx_nf_key;
  switch_pktdriver_tx_filter_action_t tx_nf_action;
  switch_pktdriver_tx_filter_priority_t tx_nf_priority = 0;
  switch_uint64_t tx_nf_flags = 0;
  switch_hostif_info_t *hostif_info = NULL;
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t port_handle = SWITCH_API_INVALID_HANDLE;
  switch_dev_port_t dev_port = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(tx_key && tx_action);
  if (!tx_key || !tx_action) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hostif tx filter create failed on device %d: "
        "parameters invalid:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  handle = switch_hostif_tx_filter_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "hostif tx filter create failed on device %d: "
        "handle allocate failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_hostif_tx_filter_get(device, handle, &tx_filter_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif tx filter create failed on device %d: "
        "hostif tx filter get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (!(flags & SWITCH_HOSTIF_TX_FILTER_ATTR_HOSTIF_HANDLE)) {
    SWITCH_LOG_ERROR(
        "hostif tx filter create failed on device %d: "
        "hostif handle is mandatory:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_HOSTIF_HANDLE(tx_key->hostif_handle));
  if (!SWITCH_HOSTIF_HANDLE(tx_key->hostif_handle)) {
    SWITCH_LOG_ERROR(
        "hostif tx filter create failed on device %d handle 0x%lx: "
        "hostif handle invalid:(%s)\n",
        device,
        tx_key->hostif_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_hostif_get(device, tx_key->hostif_handle, &hostif_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif tx filter create failed on device %d handle 0x%lx: "
        "hostif get failed:(%s)\n",
        device,
        tx_key->hostif_handle,
        switch_error_to_string(status));
    return status;
  }

  if (flags & SWITCH_HOSTIF_TX_FILTER_ATTR_VLAN_ID) {
  }

  if (tx_action->bypass_flags == SWITCH_BYPASS_ALL) {
    if (SWITCH_PORT_HANDLE(tx_action->handle)) {
      port_handle = tx_action->handle;
    } else if (SWITCH_RIF_HANDLE(tx_action->handle)) {
      status =
          switch_rif_attr_handle_get(device, tx_action->handle, &port_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif tx filter create failed on device %d handle 0x%lx: "
            "rif port handle get failed:(%s)\n",
            device,
            tx_key->hostif_handle,
            switch_error_to_string(status));
        return status;
      }

      SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
      if (!SWITCH_PORT_HANDLE(port_handle)) {
        SWITCH_LOG_ERROR(
            "hostif tx filter create failed on device %d handle 0x%lx: "
            "port handle invalid:(%s)\n",
            device,
            tx_key->hostif_handle,
            switch_error_to_string(status));
        return status;
      }
    } else if (SWITCH_INTERFACE_HANDLE(tx_action->handle)) {
      status = switch_api_interface_handle_get(
          device, tx_action->handle, &port_handle);
      SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
      SWITCH_ASSERT(SWITCH_PORT_HANDLE(port_handle));
      if (!SWITCH_PORT_HANDLE(port_handle)) {
        SWITCH_LOG_ERROR(
            "hostif tx filter create failed on device %d handle 0x%lx: "
            "port handle invalid:(%s)\n",
            device,
            tx_key->hostif_handle,
            switch_error_to_string(status));
        return status;
      }
    } else {
      SWITCH_ASSERT(0);
    }

    SWITCH_PORT_DEV_PORT_GET(device, port_handle, dev_port, status);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  } else {
    status = switch_bd_handle_get(device, tx_action->handle, &bd_handle);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    SWITCH_ASSERT(SWITCH_BD_HANDLE(bd_handle));
  }

  tx_filter_info->dev_port = dev_port;
  tx_filter_info->bd = handle_to_id(bd_handle);
  tx_filter_info->hostif_fd = hostif_info->hostif_fd;

  SWITCH_MEMCPY(
      &tx_filter_info->tx_key, tx_key, sizeof(switch_hostif_tx_filter_key_t));
  SWITCH_MEMCPY(&tx_filter_info->tx_action,
                tx_action,
                sizeof(switch_hostif_tx_filter_action_t));

  SWITCH_MEMSET(&tx_nf_key, 0x0, sizeof(tx_nf_key));
  SWITCH_MEMSET(&tx_nf_action, 0x0, sizeof(tx_nf_action));
  tx_nf_key.hostif_fd = hostif_info->hostif_fd;
  tx_nf_key.knet_hostif_handle = hostif_info->knet_hostif_handle;
  tx_nf_action.bypass_flags = tx_action->bypass_flags;
  tx_nf_action.dev_port = dev_port;
  tx_nf_action.bd = handle_to_id(bd_handle);
  tx_nf_priority = switch_hostif_tx_priority_to_pktdriver_tx_priority(priority);
  tx_nf_flags = switch_hostif_tx_flags_to_pktdriver_tx_flags(flags);

  status = switch_pktdriver_tx_filter_create(device,
                                             tx_nf_priority,
                                             tx_nf_flags,
                                             &tx_nf_key,
                                             &tx_nf_action,
                                             &tx_filter_info->filter_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif tx filter create failed on device %d handle 0x%lx: "
        "tx filter add failed:(%s)\n",
        device,
        tx_action->handle,
        switch_error_to_string(status));
    return status;
  }

  pktdriver_ctx = switch_config_packet_driver_context_get();
  if (pktdriver_ctx->knet_pkt_driver[device]) {
    tx_filter_info->knet_hostif_handle = tx_filter_info->knet_hostif_handle;
    tx_filter_info->filter_handle = SWITCH_API_INVALID_HANDLE;
  }
  *tx_filter_handle = handle;

  SWITCH_LOG_DEBUG("tx filter created on device %d tx filter handle 0x%lx\n",
                   device,
                   handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_hostif_tx_filter_delete_internal(
    const switch_device_t device, const switch_handle_t tx_filter_handle) {
  switch_hostif_tx_filter_info_t *tx_filter_info = NULL;
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_knet_info_t *knet_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(knet_info);
  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(SWITCH_HOSTIF_TX_FILTER_HANDLE(tx_filter_handle));
  if (!SWITCH_HOSTIF_TX_FILTER_HANDLE(tx_filter_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hostif tx filter delete failed on device %d handle 0x%lx : "
        "tx filter handle invalid:(%s)\n",
        device,
        tx_filter_handle,
        switch_error_to_string(status));
    return status;
  }

  status =
      switch_hostif_tx_filter_get(device, tx_filter_handle, &tx_filter_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif tx filter delete failed on device %d handle 0x%lx : "
        "hostif tx filter get failed:(%s)\n",
        device,
        tx_filter_handle,
        switch_error_to_string(status));
    return status;
  }

  pktdriver_ctx = switch_config_packet_driver_context_get();
  if (pktdriver_ctx->knet_pkt_driver[device]) {
    status = switch_pktdriver_tx_filter_delete(
        device, tx_filter_info->knet_hostif_handle);
  } else {
    status = switch_pktdriver_tx_filter_delete(device,
                                               tx_filter_info->filter_handle);
  }

  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif tx filter delete failed on device %d handle 0x%lx : "
        "pkt driver filter delete failed:(%s)\n",
        device,
        tx_filter_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_hostif_tx_filter_handle_delete(device, tx_filter_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif tx filter delete failed on device %d handle 0x%lx : "
        "hostif tx filter delete failed:(%s)\n",
        device,
        tx_filter_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG("hositf rx filter deleted on device %d handle 0x%lx\n",
                   device,
                   tx_filter_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_hostif_create_internal(
    const switch_device_t device,
    const switch_uint64_t flags,
    const switch_hostif_t *hostif,
    switch_handle_t *hostif_handle) {
  switch_hostif_info_t *hostif_info = NULL;
  switch_hostif_context_t *hostif_ctx = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_fd_t sock_fd = SWITCH_FD_INVALID;
  switch_fd_t hostif_fd = SWITCH_FD_INVALID;
  struct ifreq ifr;
  switch_int32_t rc = 0;
  switch_int32_t fdflags = 0;
  switch_char_t *intf_name = NULL;
  switch_hostif_rx_filter_key_t rx_key;
  switch_hostif_rx_filter_action_t rx_action;
  switch_hostif_rx_filter_priority_t rx_priority;
  switch_uint64_t rx_flags = 0;
  switch_hostif_tx_filter_key_t tx_key;
  switch_hostif_tx_filter_action_t tx_action;
  switch_hostif_tx_filter_priority_t tx_priority;
  switch_uint64_t tx_flags = 0;
  switch_hostif_vlan_action_t vlan_action = SWITCH_HOSTIF_VLAN_ACTION_NONE;
  switch_status_t tmp_status = SWITCH_STATUS_SUCCESS;
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_knet_hostif_knetdev_t hostif_knetdev;
  switch_knet_info_t *knet_info = NULL;
  bool filter = TRUE;

  UNUSED(knet_info);
  UNUSED(hostif_knetdev);
  SWITCH_LOG_ENTER();

  if (!(flags & SWITCH_HOSTIF_ATTR_INTERFACE_NAME)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hostif create failed on device %d for hostif %s: "
        "hostif interface name not set:(%s)\n",
        device,
        hostif->intf_name,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_HOSTIF, (void **)&hostif_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif create failed on device %d for hostif %s: "
        "hostif context get failed:(%s)\n",
        device,
        hostif->intf_name,
        switch_error_to_string(status));
    return status;
  }

  if (!hostif || !hostif_handle) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hostif create failed on device %d for hostif %s: "
        "hostif parameters invalid:(%s)\n",
        device,
        hostif->intf_name,
        switch_error_to_string(status));
    return status;
  }

  *hostif_handle = SWITCH_API_INVALID_HANDLE;

  status = SWITCH_HASHTABLE_SEARCH(&hostif_ctx->hostif_hashtable,
                                   (void *)hostif->intf_name,
                                   (void **)&hostif_info);
  if (status == SWITCH_STATUS_SUCCESS) {
    status = SWITCH_STATUS_ITEM_ALREADY_EXISTS;
    SWITCH_LOG_ERROR(
        "hostif create failed on device %d interface %s:"
        "interface already exists:(%s)\n",
        device,
        hostif->intf_name,
        switch_error_to_string(status));
    return status;
  }

  handle = switch_hostif_handle_create(device);
  if (handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "hostif create failed on device %d interface %s:"
        "handle create failed:(%s)\n",
        device,
        hostif->intf_name,
        switch_error_to_string(status));
    return status;
  }

  status = switch_hostif_get(device, handle, &hostif_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif create failed on device %d interface %s:"
        "hostif get failed:(%s)\n",
        device,
        hostif->intf_name,
        switch_error_to_string(status));
    goto cleanup;
  }

  SWITCH_MEMCPY(&hostif_info->hostif, hostif, sizeof(switch_hostif_t));
  hostif_info->hostif_fd = SWITCH_FD_INVALID;

  pktdriver_ctx = switch_config_packet_driver_context_get();
  if (pktdriver_ctx->knet_pkt_driver[device]) {
#if !defined(BMV2) && !defined(BMV2TOFINO)
    knet_info = &pktdriver_ctx->switch_kern_info[device];
    strncpy(hostif_knetdev.name, hostif_info->hostif.intf_name, IFNAMSIZ);
    status = switch_pd_status_to_status(
        bf_knet_hostif_kndev_add(knet_info->knet_cpuif_id, &hostif_knetdev));
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "hostif create failed on device %d interface %s:"
          "knet hostif create failed:(%s)\n",
          device,
          hostif->intf_name,
          switch_error_to_string(status));
      goto cleanup;
    }
    hostif_info->knet_hostif_handle = hostif_knetdev.knet_hostif_id;
#endif
  } else {
    hostif_fd = switch_open("/dev/net/bf_tun", O_RDWR);
    if (hostif_fd < 0) {
      // fallback on linux native tuntap driver
      hostif_fd = switch_open("/dev/net/tun", O_RDWR);
      if (hostif_fd < 0) {
        SWITCH_LOG_ERROR(
            "hostif create failed on device %d interface %s:"
            "netdev create failed:(%s)\n",
            device,
            hostif->intf_name,
            switch_error_to_string(status));
        goto cleanup;
      }
    }

    SWITCH_MEMSET(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    intf_name = hostif_info->hostif.intf_name;
    strncpy(ifr.ifr_name, intf_name, IFNAMSIZ);
    rc = switch_ioctl(hostif_fd, TUNSETIFF, (void *)&ifr);
    if (rc < 0) {
      SWITCH_LOG_ERROR(
          "hostif create failed on device %d interface %s:"
          "netdev ioctl failed:(%s)\n",
          device,
          hostif->intf_name,
          switch_error_to_string(status));
      goto cleanup;
    }

    rc = switch_fcntl(hostif_fd, F_GETFL, fdflags);
    fdflags |= O_NONBLOCK;
    rc = switch_fcntl(hostif_fd, F_SETFL, fdflags);
    if (rc < 0) {
      SWITCH_LOG_ERROR(
          "hostif create failed on device %d interface %s:"
          "netdev flags set failed:(%s)\n",
          device,
          hostif->intf_name,
          switch_error_to_string(status));
      goto knet_cleanup;
    }
  }

  sock_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (sock_fd < 0) {
    SWITCH_LOG_ERROR(
        "hostif create failed on device %d interface %s:"
        "netdev socket create failed:(%s)\n",
        device,
        hostif->intf_name,
        switch_error_to_string(status));
    goto knet_cleanup;
  }

  if (flags & SWITCH_HOSTIF_ATTR_IPV4_ADDRESS) {
    struct sockaddr_in sin;
    SWITCH_MEMSET(&sin, 0x0, sizeof(struct sockaddr));
    SWITCH_MEMSET(&ifr, 0x0, sizeof(ifr));
    strncpy(ifr.ifr_name, hostif_info->hostif.intf_name, IFNAMSIZ);
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = ntohl(hostif->v4addr.ip.v4addr);
    SWITCH_MEMCPY(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));
    rc = switch_ioctl(sock_fd, SIOCSIFADDR, &ifr);
    SWITCH_ASSERT(rc == 0);
    sin.sin_addr.s_addr =
        ntohl(SWITCH_IPV4_COMPUTE_MASK(hostif->v4addr.prefix_len));
    SWITCH_MEMCPY(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));
    rc = switch_ioctl(sock_fd, SIOCSIFNETMASK, &ifr);
    SWITCH_ASSERT(rc == 0);
  }

  if (flags & SWITCH_HOSTIF_ATTR_IPV6_ADDRESS) {
#if 0
    struct sockaddr_in6 sin6;
    struct in6_ifreq ifr6;
    SWITCH_MEMSET(&sin6, 0x0, sizeof(struct sockaddr));
    SWITCH_MEMSET(&ifr6, 0x0, sizeof(ifr6));
    sin6.sin6_family = AF_INET6;
    SWITCH_MEMCPY(&ifr6.ifr6_addr, &sin6, sizeof(struct sockaddr));
    rc = switch_ioctl(sock_fd, SIOCSIFADDR, &ifr6);
    SWITCH_ASSERT(rc == 0);
#endif
  }

  if (flags & SWITCH_HOSTIF_ATTR_MAC_ADDRESS) {
    SWITCH_MEMSET(&ifr, 0x0, sizeof(ifr));
    SWITCH_MEMCPY(
        ifr.ifr_hwaddr.sa_data, hostif->mac.mac_addr, SWITCH_MAC_LENGTH);
    strncpy(ifr.ifr_name, hostif_info->hostif.intf_name, IFNAMSIZ);
    ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
    rc = switch_ioctl(sock_fd, SIOCSIFHWADDR, &ifr);
    SWITCH_ASSERT(rc == 0);
  }

  SWITCH_MEMSET(&ifr, 0x0, sizeof(ifr));
  strncpy(ifr.ifr_name, hostif_info->hostif.intf_name, IFNAMSIZ);
  ifr.ifr_flags &= ~IFF_UP;
  if (flags & SWITCH_HOSTIF_ATTR_ADMIN_STATE) {
    if (hostif->admin_state) {
      ifr.ifr_flags |= IFF_UP;
    } else {
      ifr.ifr_flags &= ~IFF_UP;
    }
  }

  rc = switch_ioctl(sock_fd, SIOCSIFFLAGS, &ifr);
  SWITCH_ASSERT(rc == 0);

  if (flags & SWITCH_HOSTIF_ATTR_VLAN_ACTION) {
    vlan_action = hostif->vlan_action;
  }

  if (!pktdriver_ctx->knet_pkt_driver[device]) {
    status = switch_pktdriver_fd_add(device, hostif_fd);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "hostif create failed on device %d interface %s:"
          "hostif fd add failed:(%s)\n",
          device,
          hostif->intf_name,
          switch_error_to_string(status));
      goto knet_cleanup;
    }
  }

  status = SWITCH_HASHTABLE_INSERT(&hostif_ctx->hostif_hashtable,
                                   &(hostif_info->node),
                                   (void *)(&hostif->intf_name),
                                   (void *)(hostif_info));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif create failed on device %d interface %s:"
        "hashtable insert failed:(%s)\n",
        device,
        hostif->intf_name,
        switch_error_to_string(status));
    return status;
  }

  hostif_info->hostif_fd = hostif_fd;
  hostif_info->flags = flags;

  hostif_info->hostif_handle = handle;
  hostif_info->rx_filter_handle = SWITCH_API_INVALID_HANDLE;
  hostif_info->tx_filter_handle = SWITCH_API_INVALID_HANDLE;

  if (flags & SWITCH_HOSTIF_ATTR_HANDLE) {
    SWITCH_HOSTIF_HANDLE_SET(device, hostif->handle, handle, status);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "hostif create failed on device %d interface %s:"
          "hostif handle set failed:(%s)\n",
          device,
          hostif->intf_name,
          switch_error_to_string(status));
      return status;
    }

    SWITCH_MEMSET(&rx_key, 0x0, sizeof(rx_key));
    SWITCH_MEMSET(&rx_action, 0x0, sizeof(rx_action));
    SWITCH_HOSTIF_RX_FILTER_DEFAULT(rx_key,
                                    rx_action,
                                    rx_priority,
                                    rx_flags,
                                    hostif->handle,
                                    handle,
                                    filter);
    if (filter) {
      rx_action.vlan_action = vlan_action;
      status =
          switch_api_hostif_rx_filter_create(device,
                                             rx_priority,
                                             rx_flags,
                                             &rx_key,
                                             &rx_action,
                                             &hostif_info->rx_filter_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif create failed on device %d interface %s:"
            "hostif rx filter create failed:(%s)\n",
            device,
            hostif->intf_name,
            switch_error_to_string(status));
        return status;
      }
    }

    SWITCH_MEMSET(&tx_key, 0x0, sizeof(tx_key));
    SWITCH_MEMSET(&tx_action, 0x0, sizeof(tx_action));
    SWITCH_HOSTIF_TX_FILTER_DEFAULT(tx_key,
                                    tx_action,
                                    tx_priority,
                                    tx_flags,
                                    hostif->handle,
                                    handle,
                                    filter);
    if (filter) {
      status =
          switch_api_hostif_tx_filter_create(device,
                                             tx_priority,
                                             tx_flags,
                                             &tx_key,
                                             &tx_action,
                                             &hostif_info->tx_filter_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif create failed on device %d interface %s:"
            "hostif tx filter create failed:(%s)\n",
            device,
            hostif->intf_name,
            switch_error_to_string(status));
        return status;
      }
    }
  }

  status = switch_hostif_set_interface_oper_state(
      device, hostif->intf_name, hostif->operstatus, hostif_info);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  *hostif_handle = handle;
  switch_fd_close(sock_fd);

  SWITCH_LOG_EXIT();

  return status;

knet_cleanup:
#if !defined(BMV2) && !defined(BMV2TOFINO)
  status = switch_pd_status_to_status(bf_knet_hostif_kndev_delete(
      knet_info->knet_cpuif_id, hostif_info->knet_hostif_handle));
#endif
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
cleanup:
  tmp_status = switch_hostif_handle_delete(device, handle);
  SWITCH_ASSERT(tmp_status == SWITCH_STATUS_SUCCESS);
  switch_fd_close(sock_fd);
  return status;
}

switch_status_t switch_api_hostif_delete_internal(
    const switch_device_t device, const switch_handle_t hostif_handle) {
  switch_hostif_info_t *hostif_info = NULL;
  switch_hostif_context_t *hostif_ctx = NULL;
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_knet_info_t *knet_info = NULL;
  switch_int32_t rc = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(knet_info);
  SWITCH_LOG_ENTER();

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_HOSTIF, (void **)&hostif_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif delete failed on device %d hostif handle 0x%lx: "
        "hostif context get failed:(%s)\n",
        device,
        hostif_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_HOSTIF_HANDLE(hostif_handle));
  if (!(SWITCH_HOSTIF_HANDLE(hostif_handle))) {
    status = SWITCH_STATUS_INVALID_HANDLE;
    SWITCH_LOG_ERROR(
        "hostif delete failed on device %d hostif handle 0x%lx: "
        "hostif handle failed:(%s)\n",
        device,
        hostif_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_hostif_get(device, hostif_handle, &hostif_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif delete failed on device %d hostif handle 0x%lx: "
        "hostif get failed:(%s)\n",
        device,
        hostif_handle,
        switch_error_to_string(status));
    return status;
  }

  pktdriver_ctx = switch_config_packet_driver_context_get();
  if (!pktdriver_ctx->knet_pkt_driver[device]) {
    status = switch_pktdriver_fd_delete(device, hostif_info->hostif_fd);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "hostif delete failed on device %d hostif handle 0x%lx: "
          "pkt driver fd delete failed:(%s)\n",
          device,
          hostif_handle,
          switch_error_to_string(status));
      return status;
    }
  } else {
#if !defined(BMV2) && !defined(BMV2TOFINO)
    knet_info = &pktdriver_ctx->switch_kern_info[device];
    status = switch_pd_status_to_status(bf_knet_hostif_kndev_delete(
        knet_info->knet_cpuif_id, hostif_info->knet_hostif_handle));
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "hostif delete failed on device %d interface %s:"
          "knet hostif delete failed:(%s)\n",
          device,
          hostif_info->hostif.intf_name,
          switch_error_to_string(status));
      return status;
    }
#endif
  }

  status = SWITCH_HASHTABLE_DELETE(&hostif_ctx->hostif_hashtable,
                                   (void *)(&hostif_info->hostif.intf_name),
                                   (void **)&hostif_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif delete failed on device %d hostif handle 0x%lx:"
        "hashtable delete failed:(%s)\n",
        device,
        hostif_handle,
        switch_error_to_string(status));
    return status;
  }

  if (hostif_info->flags & SWITCH_HOSTIF_ATTR_HANDLE) {
    SWITCH_HOSTIF_HANDLE_SET(
        device, hostif_info->hostif.handle, SWITCH_API_INVALID_HANDLE, status);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "hostif delete failed on device %d interface %s:"
          "hostif handle set failed:(%s)\n",
          device,
          hostif_info->hostif.intf_name,
          switch_error_to_string(status));
      return status;
    }

    if (SWITCH_HOSTIF_RX_FILTER_HANDLE(hostif_info->rx_filter_handle)) {
      status = switch_api_hostif_rx_filter_delete(
          device, hostif_info->rx_filter_handle);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "hostif delete failed on device %d interface %s:"
            "hostif rx filter delete failed:(%s)\n",
            device,
            hostif_info->hostif.intf_name,
            switch_error_to_string(status));
        return status;
      }
    }

    if (SWITCH_HOSTIF_TX_FILTER_HANDLE(hostif_info->tx_filter_handle)) {
      if (!pktdriver_ctx->knet_pkt_driver[device]) {
        status = switch_api_hostif_tx_filter_delete(
            device, hostif_info->tx_filter_handle);
        if (status != SWITCH_STATUS_SUCCESS) {
          SWITCH_LOG_ERROR(
              "hostif delete failed on device %d interface %s:"
              "hostif tx filter delete failed:(%s)\n",
              device,
              hostif_info->hostif.intf_name,
              switch_error_to_string(status));
          return status;
        }
      }
    }
  }

  if (!pktdriver_ctx->knet_pkt_driver[device]) {
    rc = switch_fd_close(hostif_info->hostif_fd);
    SWITCH_ASSERT(rc == 0);
  }
  status = switch_hostif_handle_delete(device, hostif_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_DEBUG("hostif deleted on device %d hostif handle 0x%lx\n",
                   device,
                   hostif_handle);

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_hostif_nhop_get_internal(
    switch_device_t device,
    switch_hostif_reason_code_t rcode,
    switch_handle_t *nhop_handle) {
  switch_hostif_context_t *hostif_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(nhop_handle != NULL);
  if (!nhop_handle) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hostif nhop get failed on device %d rcode %d: "
        "parameters invalid:(%s)\n",
        device,
        rcode,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_HOSTIF, (void **)&hostif_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif nhop get failed on device %d rcode %d: "
        "hostif context get failed:(%s)\n",
        device,
        rcode,
        switch_error_to_string(status));
    return status;
  }

  if (rcode > SWITCH_HOSTIF_REASON_CODE_MAX) {
    SWITCH_LOG_ERROR(
        "hostif nhop get failed on device %d rcode %d: "
        "hostif reason code invalid:(%s)\n",
        device,
        rcode,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_NHOP_HANDLE(hostif_ctx->nhop_handles[rcode]));
  if (!SWITCH_NHOP_HANDLE(hostif_ctx->nhop_handles[rcode])) {
    SWITCH_LOG_ERROR(
        "hostif nhop get failed on device %d rcode %d: "
        "hostif nhop invalid:(%s)\n",
        device,
        rcode,
        switch_error_to_string(status));
    return status;
  }

  *nhop_handle = hostif_ctx->nhop_handles[rcode];

  return status;
}

switch_status_t switch_api_hostif_cpu_intf_info_get(
    switch_device_t device, switch_interface_info_t **intf_info) {
  switch_hostif_context_t *hostif_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(intf_info != NULL);
  if (!intf_info) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hostif cpu ifindex get failed on device %d: "
        "parameters invalid:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_HOSTIF, (void **)&hostif_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif cpu ifindex get failed on device %d: "
        "hostif context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(hostif_ctx->intf_handle));
  if (!SWITCH_INTERFACE_HANDLE(hostif_ctx->intf_handle)) {
    SWITCH_LOG_ERROR(
        "hostif cpu ifindex get failed on device %d: "
        "hostif interface handle invalid:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_interface_get(device, hostif_ctx->intf_handle, intf_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif cpu ifindex get failed on device %d: "
        "hostif interface get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_api_hostif_cpu_intf_handle_get(
    switch_device_t device, switch_handle_t *intf_handle) {
  switch_hostif_context_t *hostif_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_HOSTIF, (void **)&hostif_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif cpu ifindex get failed on device %d: "
        "hostif context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(SWITCH_INTERFACE_HANDLE(hostif_ctx->intf_handle));
  if (!SWITCH_INTERFACE_HANDLE(hostif_ctx->intf_handle)) {
    SWITCH_LOG_ERROR(
        "hostif cpu ifindex get failed on device %d: "
        "hostif interface handle invalid:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  *intf_handle = hostif_ctx->intf_handle;

  return status;
}
switch_status_t switch_api_hostif_meter_create_internal(
    const switch_device_t device,
    const switch_api_meter_t *api_meter_info,
    switch_handle_t *meter_handle) {
  switch_meter_info_t *meter_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(api_meter_info != NULL);
  SWITCH_ASSERT(meter_handle != NULL);
  if (!api_meter_info || !meter_handle) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hostif meter create failed on device %d: "
        "parameters invalid:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  *meter_handle = switch_meter_handle_create(device);
  if (*meter_handle == SWITCH_API_INVALID_HANDLE) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "hostif meter create failed on device %d: "
        "meter handle create failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_meter_get(device, *meter_handle, &meter_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif meter create failed on device %d: "
        "meter handle get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(
      &meter_info->api_meter_info, api_meter_info, sizeof(switch_api_meter_t));

  status = switch_pd_hostif_meter_set(
      device, handle_to_id(*meter_handle), meter_info, TRUE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif meter create failed on device %d: "
        "meter pd set failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "hostif meter created on device %d handle 0x%lx", device, *meter_handle);

  return status;
}

switch_status_t switch_api_hostif_meter_delete_internal(
    switch_device_t device, switch_handle_t meter_handle) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_meter_info_t *meter_info = NULL;

  SWITCH_ASSERT(SWITCH_METER_HANDLE(meter_handle));
  if (!SWITCH_METER_HANDLE(meter_handle)) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hostif meter delete failed on device %d handle 0x%lx: "
        "meter handle invalid:(%s)\n",
        device,
        meter_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_meter_get(device, meter_handle, &meter_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif meter delete failed on device %d handle 0x%lx: "
        "meter get failed:(%s)\n",
        device,
        meter_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pd_hostif_meter_set(
      device, handle_to_id(meter_handle), meter_info, FALSE);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif meter delete failed on device %d handle 0x%lx: "
        "meter pd set failed:(%s)\n",
        device,
        meter_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_meter_handle_delete(device, meter_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  SWITCH_LOG_DEBUG(
      "hostif meter deleted on device %d handle 0x%lx", device, meter_handle);

  return status;
}

switch_status_t switch_hostif_callback_rx(switch_packet_info_t *pkt_info) {
  switch_hostif_context_t *hostif_ctx = NULL;
  switch_bd_info_t *bd_info = NULL;
  switch_port_info_t *port_info = NULL;
  switch_hostif_packet_t hostif_pkt;
  switch_handle_t port_handle = SWITCH_API_INVALID_HANDLE;
  switch_uint16_t index = 0;
  switch_port_t port = 0;
  switch_dev_port_t dev_port = 0;
  switch_device_t device = 0;
  switch_bd_t bd = 0;
  switch_ifindex_t ifindex = 0;
  switch_handle_t intf_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t network_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t bd_handle = SWITCH_API_INVALID_HANDLE;
  switch_handle_t lag_handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(pkt_info);

  device = pkt_info->device;

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_HOSTIF, (void **)&hostif_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif callback rx failed on device %d: "
        "hostif context get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "hostif packet rx recevied on device %d ifindex 0x%x "
      "bd %d dev port %d\n",
      device,
      SWITCH_PKTINFO_INGRESS_IFINDEX(pkt_info),
      SWITCH_PKTINFO_INGRESS_BD(pkt_info),
      SWITCH_PKTINFO_RX_DEV_PORT(pkt_info));

  dev_port = SWITCH_PKTINFO_RX_DEV_PORT(pkt_info);
  status = switch_port_dev_port_to_handle_get(device, dev_port, &port_handle);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  status = switch_api_port_handle_to_id_get(device, port_handle, &port);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  ifindex = SWITCH_PKTINFO_INGRESS_IFINDEX(pkt_info);
  status = switch_interface_handle_get(device, ifindex, &intf_handle);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_DEBUG(
        "hostif callback rx failed on device %d ifindex 0x%x: "
        "interface get failed:(%s)\n",
        device,
        ifindex,
        switch_error_to_string(status));
    return SWITCH_STATUS_SUCCESS;
  }

  if (SWITCH_PORT_HANDLE(port_handle)) {
    status = switch_port_get(device, port_handle, &port_info);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
    lag_handle = port_info->lag_handle;
  }

  bd = SWITCH_PKTINFO_INGRESS_BD(pkt_info);
  if (bd != SWITCH_BD_INVALID) {
    bd_handle = id_to_handle(SWITCH_HANDLE_TYPE_BD, bd);
    status = switch_bd_get(device, bd_handle, &bd_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_DEBUG(
          "hostif callback rx failed on device %d bd %d: "
          "bd get failed:(%s)\n",
          device,
          bd,
          switch_error_to_string(status));
      return SWITCH_STATUS_SUCCESS;
    }

    if (SWITCH_NETWORK_HANDLE(bd_info->handle)) {
      network_handle = bd_info->handle;
    }
  }

  SWITCH_MEMSET(&hostif_pkt, 0x0, sizeof(hostif_pkt));
  hostif_pkt.device = pkt_info->device;
  hostif_pkt.pkt = pkt_info->pkt;
  hostif_pkt.pkt_size = pkt_info->pkt_size;
  hostif_pkt.reason_code = SWITCH_PKTINFO_REASON_CODE(pkt_info);
  hostif_pkt.handle = port_handle;
  hostif_pkt.port = port;
  hostif_pkt.network_handle = network_handle;
  hostif_pkt.intf_handle = intf_handle;
  hostif_pkt.lag_handle = lag_handle;

  SWITCH_LOG_DETAIL(
      "hostif callback rx on device %d port handle 0x%lx dev port %d "
      "bd %d bd handle 0x%lx network handle 0x%lx intf handle 0x%lx "
      "packet size %d\n",
      device,
      port_handle,
      dev_port,
      bd,
      bd_handle,
      network_handle,
      intf_handle,
      pkt_info->pkt_size);

  for (index = 0; index < SWITCH_MAX_RX_CALLBACK; index++) {
    if (hostif_ctx->rx_cb_list[index].valid) {
      hostif_pkt.cookie = hostif_ctx->rx_cb_list[index].cookie;
      hostif_ctx->rx_cb_list[index].cb_fn(&hostif_pkt);
      SWITCH_LOG_DEBUG("hostif callback rx done for app %d\n",
                       hostif_ctx->rx_cb_list[index].app_id);
    }
  }

  return status;
}

switch_status_t switch_api_hostif_handle_get_internal(
    const switch_device_t device,
    const char *intf_name,
    switch_handle_t *hostif_handle) {
  switch_hostif_info_t *hostif_info = NULL;
  switch_hostif_context_t *hostif_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!intf_name) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "hostif handle get failed on device %d for hostif %s: "
        "hostif interface name invalid:(%s)\n",
        device,
        intf_name,
        switch_error_to_string(status));
    return status;
  }

  status = switch_device_api_context_get(
      device, SWITCH_API_TYPE_HOSTIF, (void **)&hostif_ctx);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif handle get failed on device %d for hostif %s: "
        "hostif context get failed:(%s)\n",
        device,
        intf_name,
        switch_error_to_string(status));
    return status;
  }

  *hostif_handle = SWITCH_API_INVALID_HANDLE;

  status = SWITCH_HASHTABLE_SEARCH(
      &hostif_ctx->hostif_hashtable, (void *)intf_name, (void **)&hostif_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif handle get failed on device %d for hostif %s: "
        "hostif hashtable search failed:(%s)\n",
        device,
        intf_name,
        switch_error_to_string(status));
    return status;
  }

  *hostif_handle = hostif_info->hostif_handle;

  SWITCH_LOG_DEBUG("hostif handle get on device %d intf name %s handle 0x%lx\n",
                   device,
                   intf_name,
                   hostif_info->hostif_handle);
  return status;
}

switch_status_t switch_api_hostif_reason_code_create(
    const switch_device_t device,
    const switch_uint64_t flags,
    const switch_api_hostif_rcode_info_t *rcode_api_info,
    switch_handle_t *rcode_handle) {
  SWITCH_MT_WRAP(switch_api_hostif_reason_code_create_internal(
      device, flags, rcode_api_info, rcode_handle))
}

switch_status_t switch_api_hostif_delete(const switch_device_t device,
                                         const switch_handle_t hostif_handle) {
  SWITCH_MT_WRAP(switch_api_hostif_delete_internal(device, hostif_handle))
}

switch_status_t switch_api_hostif_create(const switch_device_t device,
                                         const switch_uint64_t flags,
                                         const switch_hostif_t *hostif,
                                         switch_handle_t *hostif_handle) {
  SWITCH_MT_WRAP(
      switch_api_hostif_create_internal(device, flags, hostif, hostif_handle))
}

switch_status_t switch_api_hostif_meter_delete(switch_device_t device,
                                               switch_handle_t meter_handle) {
  SWITCH_MT_WRAP(switch_api_hostif_meter_delete_internal(device, meter_handle))
}

switch_status_t switch_api_hostif_reason_code_update(
    const switch_device_t device,
    const switch_handle_t rcode_handle,
    const switch_uint64_t flags,
    const switch_api_hostif_rcode_info_t *rcode_api_info) {
  SWITCH_MT_WRAP(switch_api_hostif_reason_code_update_internal(
      device, rcode_handle, flags, rcode_api_info))
}

switch_status_t switch_api_hostif_reason_code_delete(
    const switch_device_t device, const switch_handle_t rcode_handle) {
  SWITCH_MT_WRAP(
      switch_api_hostif_reason_code_delete_internal(device, rcode_handle))
}

switch_status_t switch_api_hostif_meter_create(
    const switch_device_t device,
    const switch_api_meter_t *api_meter_info,
    switch_handle_t *meter_handle) {
  SWITCH_MT_WRAP(switch_api_hostif_meter_create_internal(
      device, api_meter_info, meter_handle))
}

switch_status_t switch_api_hostif_group_create(
    const switch_device_t device,
    const switch_hostif_group_t *hif_group,
    switch_handle_t *hif_group_handle) {
  SWITCH_MT_WRAP(switch_api_hostif_group_create_internal(
      device, hif_group, hif_group_handle))
}

switch_status_t switch_api_hostif_group_delete(
    const switch_device_t device, const switch_handle_t hif_group_handle) {
  SWITCH_MT_WRAP(
      switch_api_hostif_group_delete_internal(device, hif_group_handle))
}

switch_status_t switch_api_hostif_nhop_get(switch_device_t device,
                                           switch_hostif_reason_code_t rcode,
                                           switch_handle_t *nhop_handle) {
  SWITCH_MT_WRAP(
      switch_api_hostif_nhop_get_internal(device, rcode, nhop_handle))
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_hostif_tx_filter_create(
    const switch_device_t device,
    const switch_hostif_tx_filter_priority_t priority,
    const switch_uint64_t flags,
    const switch_hostif_tx_filter_key_t *tx_key,
    const switch_hostif_tx_filter_action_t *tx_action,
    switch_handle_t *tx_filter_handle) {
  SWITCH_MT_WRAP(switch_api_hostif_tx_filter_create_internal(
      device, priority, flags, tx_key, tx_action, tx_filter_handle))
}

switch_status_t switch_api_hostif_rx_callback_register(
    switch_device_t device,
    switch_app_id_t app_id,
    switch_hostif_rx_callback_fn cb_fn,
    void *cookie) {
  SWITCH_MT_WRAP(switch_api_hostif_rx_callback_register_internal(
      device, app_id, cb_fn, cookie))
}

switch_status_t switch_api_hostif_rx_callback_deregister(
    switch_device_t device,
    switch_app_id_t app_id,
    switch_hostif_rx_callback_fn cb_fn) {
  SWITCH_MT_WRAP(
      switch_api_hostif_rx_callback_deregister_internal(device, app_id, cb_fn))
}

switch_status_t switch_api_hostif_rx_filter_create(
    const switch_device_t device,
    const switch_hostif_rx_filter_priority_t priority,
    const switch_uint64_t flags,
    const switch_hostif_rx_filter_key_t *rx_key,
    const switch_hostif_rx_filter_action_t *rx_action,
    switch_handle_t *rx_filter_handle) {
  SWITCH_MT_WRAP(switch_api_hostif_rx_filter_create_internal(
      device, priority, flags, rx_key, rx_action, rx_filter_handle))
}

switch_status_t switch_api_hostif_tx_filter_delete(
    const switch_device_t device, const switch_handle_t tx_filter_handle) {
  SWITCH_MT_WRAP(
      switch_api_hostif_tx_filter_delete_internal(device, tx_filter_handle))
}

switch_status_t switch_api_hostif_rx_filter_delete(
    const switch_device_t device, const switch_handle_t rx_filter_handle) {
  SWITCH_MT_WRAP(
      switch_api_hostif_rx_filter_delete_internal(device, rx_filter_handle))
}

switch_status_t switch_api_hostif_handle_get(const switch_device_t device,
                                             const char *intf_name,
                                             switch_handle_t *hostif_handle) {
  SWITCH_MT_WRAP(
      switch_api_hostif_handle_get_internal(device, intf_name, hostif_handle))
}

switch_status_t switch_api_hostif_group_get_internal(
    const switch_device_t device,
    switch_handle_t handle,
    switch_hostif_group_t *hostif_group) {
  switch_hostif_group_info_t *hif_group_info = NULL;
  switch_hostif_group_get(device, handle, &hif_group_info);
  SWITCH_MEMCPY(
      hostif_group, &hif_group_info->hif_group, sizeof(switch_hostif_group_t));
  return SWITCH_STATUS_SUCCESS;
}

switch_status_t switch_api_hostif_group_get(
    const switch_device_t device,
    switch_handle_t handle,
    switch_hostif_group_t *hostif_group) {
  SWITCH_MT_WRAP(
      switch_api_hostif_group_get_internal(device, handle, hostif_group));
}

switch_status_t switch_api_hostif_group_meter_set(
    const switch_device_t device,
    switch_handle_t handle,
    switch_handle_t meter_handle) {
  SWITCH_MT_WRAP(
      switch_api_hostif_group_meter_set_internal(device, handle, meter_handle))
}

switch_status_t switch_api_hostif_oper_state_set(switch_device_t device,
                                                 switch_handle_t hostif_handle,
                                                 bool oper_state) {
  SWITCH_MT_WRAP(switch_api_hostif_oper_state_set_internal(
      device, hostif_handle, oper_state));
}

switch_status_t switch_api_hostif_oper_state_get(switch_device_t device,
                                                 switch_handle_t hostif_handle,
                                                 bool *oper_state) {
  SWITCH_MT_WRAP(switch_api_hostif_oper_state_get_internal(
      device, hostif_handle, oper_state));
}
