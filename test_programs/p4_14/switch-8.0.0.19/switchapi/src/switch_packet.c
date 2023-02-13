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

#include "switchapi/switch_hostif.h"

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"
#include <signal.h>

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_PACKET_DRIVER

#undef __MODULE__
#define __MODULE__ SWITCH_API_TYPE_PACKET_DRIVER

pthread_t packet_driver_thread;
static pthread_mutex_t cookie_mutex;
static pthread_cond_t cookie_cv;
static int cookie = 0;

switch_status_t switch_packet_knet_cpuif_bind(switch_device_t device) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_knet_info_t *knet_info = NULL;
  struct ifreq ifr;
  struct sockaddr_ll *addr;
  switch_int32_t knet_cpuif_fd = 0;
  switch_int32_t flags = 0;
  switch_int32_t rc = 0;
  char *intf_name = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  pktdriver_ctx = switch_config_packet_driver_context_get();
  knet_info = &pktdriver_ctx->switch_kern_info[device];
  intf_name = knet_info->cpuif_knetdev_name;

  knet_cpuif_fd = switch_socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);
  if (knet_cpuif_fd < 0) {
    SWITCH_LOG_ERROR(
        "knet cpuif bind failed."
        "socket creation failed for %s",
        intf_name);
    return SWITCH_STATUS_FAILURE;
  }

  // set cpu port to be non-blocking
  flags = switch_fcntl(knet_cpuif_fd, F_GETFL, 0);
  if (flags < 0) {
    SWITCH_LOG_ERROR(
        "knet cpuif bind failed."
        "Get flag for interface %s failed",
        intf_name);
    return SWITCH_STATUS_FAILURE;
  }

  flags |= O_NONBLOCK;
  rc = switch_fcntl(knet_cpuif_fd, F_SETFL, flags);
  if (rc < 0) {
    SWITCH_LOG_ERROR(
        "knet cpuif bind failed."
        "Set flag for interface %s failed",
        intf_name);
    return SWITCH_STATUS_FAILURE;
  }

  SWITCH_MEMSET(&ifr, 0x0, sizeof(ifr));
  strncpy(ifr.ifr_name, knet_info->cpuif_knetdev_name, IFNAMSIZ);
  ifr.ifr_flags |= IFF_UP;

  rc = switch_ioctl(knet_cpuif_fd, SIOCSIFFLAGS, &ifr);
  if (rc < 0) {
    SWITCH_LOG_ERROR(
        "knet cpuif bind failed."
        "Failed to bring up interface %s",
        intf_name);
    return SWITCH_STATUS_FAILURE;
  }

  SWITCH_MEMSET(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, knet_info->cpuif_knetdev_name, IFNAMSIZ);
  rc = switch_ioctl(knet_cpuif_fd, SIOCGIFINDEX, (void *)&ifr);
  if (rc < 0) {
    SWITCH_LOG_ERROR(
        "knet cpuif bind failed."
        "IOCTL on %s failed",
        intf_name);
    return SWITCH_STATUS_FAILURE;
  }

  addr = &(knet_info->s_addr);
  // bind to cpu port
  SWITCH_MEMSET(addr, 0, sizeof(struct sockaddr_ll));
  addr->sll_family = AF_PACKET;
  addr->sll_ifindex = ifr.ifr_ifindex;
  addr->sll_protocol = switch_htons(ETH_P_ALL);
  rc = switch_bind(
      knet_cpuif_fd, (struct sockaddr *)addr, sizeof(struct sockaddr_ll));
  SWITCH_ASSERT(rc == 0);
  if (rc < 0) {
    SWITCH_LOG_ERROR(
        "knet cpuif bind failed."
        "knet cpu interface bind failed for %s",
        intf_name);
    return SWITCH_STATUS_FAILURE;
  }

  knet_info->sock_fd = knet_cpuif_fd;

  status = switch_pktdriver_fd_add(device, knet_cpuif_fd);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "knet cpuif bind failed."
        "knet cpu interface bind failed : %s, rc=%u",
        switch_error_to_string(status),
        rc);
    return status;
  }

  return status;
}

switch_status_t switch_packet_hostif_bind(switch_device_t device) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  struct ifreq ifr;
  struct sockaddr_ll addr;
  switch_int32_t cpu_fd = 0;
  switch_int32_t flags = 0;
  switch_int32_t rc = 0;
  char *intf_name = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  pktdriver_ctx = switch_config_packet_driver_context_get();
  intf_name = pktdriver_ctx->intf_name;
  // initialize raw socket
  cpu_fd = switch_socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);
  if (cpu_fd < 0) {
    SWITCH_LOG_ERROR(
        "hostif bind failed."
        "socket creation failed for %s.",
        intf_name);
    return SWITCH_STATUS_FAILURE;
  }

  // set cpu port to be non-blocking
  flags = switch_fcntl(cpu_fd, F_GETFL, 0);
  SWITCH_ASSERT(flags >= 0);

  flags |= O_NONBLOCK;
  rc = switch_fcntl(cpu_fd, F_SETFL, flags);
  if (rc < 0) {
    SWITCH_LOG_ERROR(
        "hostif bind failed."
        "Set flag for interface %s failed",
        intf_name);
    return SWITCH_STATUS_FAILURE;
  }

  // initialize cpu port
  SWITCH_MEMSET(&ifr, 0, sizeof(ifr));
  strncpy(ifr.ifr_name, pktdriver_ctx->intf_name, IFNAMSIZ);
  rc = switch_ioctl(cpu_fd, SIOCGIFINDEX, (void *)&ifr);
  if (rc < 0) {
    SWITCH_LOG_ERROR(
        "hostif bind failed."
        "IOCTL on %s failed",
        intf_name);
    return SWITCH_STATUS_FAILURE;
  }

  // bind to cpu port
  SWITCH_MEMSET(&addr, 0, sizeof(addr));
  addr.sll_family = AF_PACKET;
  addr.sll_ifindex = ifr.ifr_ifindex;
  addr.sll_protocol = switch_htons(ETH_P_ALL);
  rc = switch_bind(cpu_fd, (struct sockaddr *)&addr, sizeof(addr));
  if (rc < 0) {
    SWITCH_LOG_ERROR(
        "hostif bind failed."
        "cpu interface bind failed for %s",
        intf_name);
    return SWITCH_STATUS_FAILURE;
  }

  pktdriver_ctx->cpu_ifindex = ifr.ifr_ifindex;
  pktdriver_ctx->cpu_fd = cpu_fd;

  status = switch_pktdriver_fd_add(device, cpu_fd);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "hostif bind failed."
        "cpu interface bind failed : %s, rc=%u",
        switch_error_to_string(status),
        rc);
    return status;
  }

  return status;
}

switch_status_t switch_pktdriver_init(void) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_device_t device = 0;
  switch_int32_t rc = 0;
  switch_int32_t flags = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_status_t tmp_status = SWITCH_STATUS_SUCCESS;

  pktdriver_ctx = switch_config_packet_driver_context_get();

  pktdriver_ctx->pipe_fd[0] = SWITCH_FD_INVALID;
  pktdriver_ctx->pipe_fd[1] = SWITCH_FD_INVALID;
  pktdriver_ctx->cpu_fd = SWITCH_FD_INVALID;

  rc = switch_pipe(pktdriver_ctx->pipe_fd);
  SWITCH_ASSERT(rc == 0);
  if (rc != 0) {
    status = SWITCH_STATUS_INSUFFICIENT_RESOURCES;
    SWITCH_LOG_ERROR(
        "pktdriver init failed."
        "pipe open failed:(%s)\n",
        switch_error_to_string(status));
    return status;
  }

  rc = switch_fcntl(pktdriver_ctx->pipe_fd[0], F_GETFL, 0);
  if (rc != 0) {
    status = SWITCH_STATUS_INSUFFICIENT_RESOURCES;
    SWITCH_LOG_ERROR(
        "pktdriver init failed."
        "pipe flags get failed:(%s)\n",
        switch_error_to_string(status));
    return status;
  }

  flags = flags | O_NONBLOCK;

  rc = switch_fcntl(pktdriver_ctx->pipe_fd[0], F_SETFL, flags);
  if (rc != 0) {
    status = SWITCH_STATUS_INSUFFICIENT_RESOURCES;
    SWITCH_LOG_ERROR(
        "pktdriver init failed."
        "pipe flags set failed:(%s)\n",
        switch_error_to_string(status));
    return status;
  }

  status = switch_pktdriver_fd_add(device, pktdriver_ctx->pipe_fd[0]);
  if (rc != 0) {
    status = SWITCH_STATUS_INSUFFICIENT_RESOURCES;
    SWITCH_LOG_ERROR("pktdriver init failed.",
                     "pipe fd add failed:(%s)\n",
                     switch_error_to_string(status));
    return status;
  }

  if (pktdriver_ctx->knet_pkt_driver[device]) {
    status = switch_packet_knet_cpuif_bind(device);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("pktdriver init failed.",
                       "knet cpuif bind failed:(%s)\n",
                       switch_error_to_string(status));
      goto cleanup;
    }
  } else {
    pktdriver_ctx->cpu_ifindex = 0;

    SWITCH_MEMCPY(pktdriver_ctx->intf_name,
                  SWITCH_CONFIG_CPU_ETH_INTF(),
                  SWITCH_CONFIG_CPU_ETH_INTF_LEN());

    if (!SWITCH_CONFIG_PCIE()) {
      status = switch_packet_hostif_bind(device);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("pktdriver init failed.",
                         "hostif bind failed:(%s)\n",
                         switch_error_to_string(status));
        goto cleanup;
      }
    }

    status = SWITCH_LIST_INIT(&pktdriver_ctx->rx_filter);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("pktdriver init failed.",
                       "rx filter list init failed:(%s)\n",
                       switch_error_to_string(status));
      goto cleanup;
    }

    status = SWITCH_LIST_INIT(&pktdriver_ctx->tx_filter);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("pktdriver init failed.",
                       "rx filter list init failed:(%s)\n",
                       switch_error_to_string(status));
      goto cleanup;
    }

    status = switch_handle_type_init(device,
                                     SWITCH_HANDLE_TYPE_PKTDRIVER_RX_FILTER,
                                     SWITCH_PKTDRIVER_RX_FILTER_SIZE);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("pktdriver init failed.",
                       "rx handle init failed:(%s)\n",
                       switch_error_to_string(status));
      goto cleanup;
    }

    status = switch_handle_type_init(device,
                                     SWITCH_HANDLE_TYPE_PKTDRIVER_TX_FILTER,
                                     SWITCH_PKTDRIVER_TX_FILTER_SIZE);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("pktdriver init failed.",
                       "tx handle init failed:(%s)\n",
                       switch_error_to_string(status));
      goto cleanup;
    }
  }

  return status;

cleanup:
  tmp_status = switch_pktdriver_free();
  SWITCH_ASSERT(tmp_status == SWITCH_STATUS_SUCCESS);
  return status;
}

switch_status_t switch_pktdriver_free(void) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_device_t device = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  pktdriver_ctx = switch_config_packet_driver_context_get();

  status =
      switch_handle_type_free(device, SWITCH_HANDLE_TYPE_PKTDRIVER_TX_FILTER);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pktdriver free failed on device %d: "
        "tx filter handle free failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }

  status =
      switch_handle_type_free(device, SWITCH_HANDLE_TYPE_PKTDRIVER_RX_FILTER);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pktdriver free failed on device %d: "
        "rx filter handle free failed:(%s)\n",
        device,
        switch_error_to_string(status));
  }

  if (pktdriver_ctx->pipe_fd[0] != SWITCH_FD_INVALID) {
    switch_fd_close(pktdriver_ctx->pipe_fd[0]);
  }

  if (pktdriver_ctx->pipe_fd[1] != SWITCH_FD_INVALID) {
    switch_fd_close(pktdriver_ctx->pipe_fd[1]);
  }

  if (pktdriver_ctx->cpu_fd != SWITCH_FD_INVALID) {
    switch_fd_close(pktdriver_ctx->cpu_fd);
  }

  return status;
}

switch_status_t switch_pktdriver_packet_dump(switch_packet_info_t *pkt_info) {
  switch_uint16_t index = 0;

  if (!pkt_info) {
    return SWITCH_STATUS_SUCCESS;
  }

  SWITCH_LOG_DEBUG("\n++++++++++PACKET START++++++++++\n");
  SWITCH_LOG_DEBUG("pkt type: %s\n",
                   switch_pktdriver_pkttype_to_string(pkt_info->pkt_type));
  SWITCH_LOG_DEBUG("pkt size: %d\n", pkt_info->pkt_size);
  SWITCH_LOG_DEBUG("fd: %d\n", pkt_info->fd);

  /* fabric header */
  SWITCH_LOG_DEBUG("\nFABRIC HEADER\n");
  SWITCH_LOG_DEBUG("eth type: 0x%x\n",
                   pkt_info->pkt_header.fabric_header.ether_type);
  SWITCH_LOG_DEBUG("pkt version: %d\n",
                   pkt_info->pkt_header.fabric_header.packet_version);
  SWITCH_LOG_DEBUG("header version: %d\n",
                   pkt_info->pkt_header.fabric_header.header_version);
  SWITCH_LOG_DEBUG("pkt type: %d\n",
                   pkt_info->pkt_header.fabric_header.packet_type);
  SWITCH_LOG_DEBUG("color: %d\n",
                   pkt_info->pkt_header.fabric_header.fabric_color);
  SWITCH_LOG_DEBUG("qos: %d\n", pkt_info->pkt_header.fabric_header.fabric_qos);
  SWITCH_LOG_DEBUG("device: %d\n",
                   pkt_info->pkt_header.fabric_header.dst_device);
  SWITCH_LOG_DEBUG("port/group: %d\n",
                   pkt_info->pkt_header.fabric_header.dst_port_or_group);

  /* fabric header */
  SWITCH_LOG_DEBUG("\nCPU HEADER\n");
  SWITCH_LOG_DEBUG("eth type: %d\n", pkt_info->pkt_header.cpu_header.tx_bypass);
  SWITCH_LOG_DEBUG("egress queue: %d\n",
                   pkt_info->pkt_header.cpu_header.egress_queue);
  SWITCH_LOG_DEBUG("ingress port: %d\n",
                   pkt_info->pkt_header.cpu_header.ingress_port);
  SWITCH_LOG_DEBUG("ingress ifindex: %d\n",
                   pkt_info->pkt_header.cpu_header.ingress_ifindex);
  SWITCH_LOG_DEBUG("ingress bd: %d\n",
                   pkt_info->pkt_header.cpu_header.ingress_bd);
  SWITCH_LOG_DEBUG("reason code: 0x%x\n",
                   pkt_info->pkt_header.cpu_header.reason_code);

  SWITCH_LOG_DEBUG("\nPACKET\n");
  for (index = 0; index < pkt_info->pkt_size; index++) {
    SWITCH_LOG_DEBUG("%02x ", pkt_info->pkt[index]);
    if (index % 16 == 0) {
      SWITCH_LOG_DEBUG("\n");
    }
  }

  SWITCH_LOG_DEBUG("\n++++++++++PACKET END++++++++++\n");
  return SWITCH_STATUS_SUCCESS;
}

static bool switch_packet_driver_tx_filter_match(
    const switch_uint64_t flags,
    const switch_pktdriver_tx_filter_key_t *tx_key1,
    const switch_pktdriver_tx_filter_key_t *tx_key2) {
  SWITCH_ASSERT(tx_key1 && tx_key2);

  if (flags & SWITCH_PKTDRIVER_TX_FILTER_ATTR_HOSTIF_FD) {
    if (tx_key1->hostif_fd != tx_key2->hostif_fd) {
      return FALSE;
    }
  }

  return TRUE;
}

switch_int32_t switch_packet_driver_tx_filter_priority_compare(
    const void *key1, const void *key2) {
  switch_pktdriver_tx_filter_info_t *tx_info1 = NULL;
  switch_pktdriver_tx_filter_info_t *tx_info2 = NULL;

  SWITCH_ASSERT(key1 && key2);
  if (!key1 || !key2) {
    SWITCH_LOG_ERROR("tx filter priority compare failed: %s",
                     switch_error_to_string(SWITCH_STATUS_INVALID_PARAMETER));
    return -1;
  }

  tx_info1 = (switch_pktdriver_tx_filter_info_t *)key1;
  tx_info2 = (switch_pktdriver_tx_filter_info_t *)key2;

  return (switch_int32_t)tx_info1->priority -
         (switch_int32_t)tx_info2->priority;
}

static bool switch_packet_driver_rx_filter_match(
    const switch_uint64_t flags,
    const switch_pktdriver_rx_filter_key_t *rx_key1,
    const switch_pktdriver_rx_filter_key_t *rx_key2) {
  SWITCH_ASSERT(rx_key1 && rx_key2);

  if (flags & SWITCH_PKTDRIVER_RX_FILTER_ATTR_DEV_PORT) {
    if (rx_key1->dev_port != rx_key2->dev_port) {
      return FALSE;
    }
  }

  if (flags & SWITCH_PKTDRIVER_RX_FILTER_ATTR_IFINDEX) {
    if (rx_key1->ifindex != rx_key2->ifindex) {
      return FALSE;
    }
  }

  if (flags & SWITCH_PKTDRIVER_RX_FILTER_ATTR_BD) {
    if (rx_key1->bd != rx_key2->bd) {
      return FALSE;
    }
  }

  if (flags & SWITCH_PKTDRIVER_RX_FILTER_ATTR_REASON_CODE) {
    if ((rx_key1->reason_code & rx_key1->reason_code_mask) !=
        (rx_key2->reason_code & rx_key1->reason_code_mask)) {
      return FALSE;
    }
  }

  return TRUE;
}

switch_int32_t switch_packet_driver_rx_filter_priority_compare(
    const void *key1, const void *key2) {
  switch_pktdriver_rx_filter_info_t *rx_info1 = NULL;
  switch_pktdriver_rx_filter_info_t *rx_info2 = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  if (!key1 || !key2) {
    SWITCH_LOG_ERROR("packet driver rx filter priority compare failed:(%s)\n",
                     switch_error_to_string(status));
    return -1;
  }

  rx_info1 = (switch_pktdriver_rx_filter_info_t *)key1;
  rx_info2 = (switch_pktdriver_rx_filter_info_t *)key2;

  return (switch_int32_t)rx_info1->priority -
         (switch_int32_t)rx_info2->priority;
}

switch_status_t switch_pktdriver_rx_filter_info_get(
    switch_pktdriver_rx_filter_key_t *rx_key,
    switch_pktdriver_rx_filter_info_t **rx_info) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_pktdriver_rx_filter_info_t *tmp_rx_info = NULL;
  switch_node_t *node = NULL;
  bool matched = FALSE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  pktdriver_ctx = switch_config_packet_driver_context_get();

  SWITCH_ASSERT(rx_key && rx_info);
  if (!rx_key || !rx_info) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("rx filter get failed: (%s)\n",
                     switch_error_to_string(status));
    return status;
  }

  *rx_info = NULL;

  status = SWITCH_STATUS_ITEM_NOT_FOUND;
  FOR_EACH_IN_LIST(pktdriver_ctx->rx_filter, node) {
    tmp_rx_info = (switch_pktdriver_rx_filter_info_t *)node->data;
    matched = switch_packet_driver_rx_filter_match(
        tmp_rx_info->flags, &tmp_rx_info->rx_key, rx_key);

    if (matched) {
      *rx_info = tmp_rx_info;
      return SWITCH_STATUS_SUCCESS;
    }
  }
  FOR_EACH_IN_LIST_END();

  return status;
}

switch_status_t switch_pktdriver_tx_filter_info_get(
    switch_pktdriver_tx_filter_key_t *tx_key,
    switch_pktdriver_tx_filter_info_t **tx_info) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_pktdriver_tx_filter_info_t *tmp_tx_info = NULL;
  switch_node_t *node = NULL;
  bool matched = FALSE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  pktdriver_ctx = switch_config_packet_driver_context_get();

  SWITCH_ASSERT(tx_key && tx_info);
  if (!tx_key || !tx_info) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("tx filter get failed: (%s)\n",
                     switch_error_to_string(status));
    return status;
  }

  *tx_info = NULL;

  status = SWITCH_STATUS_ITEM_NOT_FOUND;
  FOR_EACH_IN_LIST(pktdriver_ctx->tx_filter, node) {
    tmp_tx_info = (switch_pktdriver_tx_filter_info_t *)node->data;
    matched = switch_packet_driver_tx_filter_match(
        tmp_tx_info->flags, &tmp_tx_info->tx_key, tx_key);

    if (matched) {
      *tx_info = tmp_tx_info;
      return SWITCH_STATUS_SUCCESS;
    }
  }
  FOR_EACH_IN_LIST_END();

  return status;
}

switch_status_t switch_pktdriver_cpu_tx(switch_device_t device,
                                        switch_int8_t *out_packet,
                                        switch_int32_t pkt_size) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_knet_info_t *knet_info = NULL;
  UNUSED(knet_info);
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_int32_t rc = 0;

  pktdriver_ctx = switch_config_packet_driver_context_get();
  if (pktdriver_ctx->knet_pkt_driver[device]) {
    knet_info = &pktdriver_ctx->switch_kern_info[device];
    rc = switch_fd_send(knet_info->sock_fd,
                        out_packet,
                        pkt_size,
                        0x0,
                        (struct sockaddr *)&knet_info->s_addr,
                        sizeof(struct sockaddr_ll));
    SWITCH_ASSERT(rc >= 0);
  } else if (SWITCH_CONFIG_PCIE()) {
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
    bf_pkt *pkt = NULL;
    bf_pkt_tx_ring_t tx_ring = BF_PKT_TX_RING_0;

    if (bf_pkt_alloc(device, &pkt, pkt_size, BF_DMA_CPU_PKT_TRANSMIT_0) != 0) {
      return SWITCH_STATUS_FAILURE;
    }

    /* copy the packet buffer and send it */
    if (bf_pkt_data_copy(pkt, (uint8_t *)out_packet, pkt_size) != 0) {
      SWITCH_LOG_ERROR("bf_pkt_data_copy failed: pkt_size=%d\n", pkt_size);
      bf_pkt_free(device, pkt);
      return SWITCH_STATUS_FAILURE;
    }

    if (bf_pkt_tx(device, pkt, tx_ring, (void *)pkt) != BF_SUCCESS) {
      bf_pkt_free(device, pkt);
    }
#endif /* __TARGET_TOFINO__ || BMV2TOFINO */
  } else {
    struct sockaddr_ll addr;
    SWITCH_MEMSET(&addr, 0x0, sizeof(addr));
    addr.sll_ifindex = pktdriver_ctx->cpu_ifindex;
    rc = switch_fd_send(pktdriver_ctx->cpu_fd,
                        out_packet,
                        pkt_size,
                        0x0,
                        (struct sockaddr *)&addr,
                        sizeof(addr));
    SWITCH_ASSERT(rc >= 0);
  }

  return status;
}

switch_status_t switch_pktdriver_tx(switch_packet_info_t *pkt_info) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_pktdriver_tx_filter_key_t tx_key;
  switch_pktdriver_tx_filter_info_t *tx_info = NULL;
  switch_int8_t out_packet[SWITCH_PACKET_MAX_BUFFER_SIZE];
  switch_bd_t bd = 0;
  switch_uint16_t in_offset = 0;
  switch_uint16_t out_offset = 0;
  switch_uint32_t pkt_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  pktdriver_ctx = switch_config_packet_driver_context_get();

  if (pktdriver_ctx->tx_pkt_trace_enable) {
    switch_pktdriver_packet_dump(pkt_info);
  }

  if (pkt_info->pkt_type == SWITCH_PKTDRIVER_PACKET_TYPE_TX_NETDEV) {
    SWITCH_ASSERT(pkt_info->fd != SWITCH_FD_INVALID);
    if (pkt_info->fd == SWITCH_FD_INVALID) {
      status = SWITCH_STATUS_INVALID_PARAMETER;
      SWITCH_LOG_CRITICAL(
          "packet tx failed: "
          "hostif fd invalid:(%s)\n",
          switch_error_to_string(status));
      return status;
    }
  }

  if (!SWITCH_CONFIG_PCIE() && pktdriver_ctx->cpu_fd == SWITCH_FD_INVALID) {
    SWITCH_LOG_ERROR(
        "packet tx failed: "
        "cpu fd not initialized:(%s)\n",
        switch_error_to_string(status));
    return status;
  }

  pkt_size = pkt_info->pkt_size;

  if (pkt_info->pkt_type == SWITCH_PKTDRIVER_PACKET_TYPE_TX_NETDEV) {
    tx_key.hostif_fd = pkt_info->fd;
    status = switch_pktdriver_tx_filter_info_get(&tx_key, &tx_info);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "pktdriver tx failed for fd %d: "
          "tx filter not found:(%s)\n",
          pkt_info->fd,
          switch_error_to_string(status));
      return status;
    }

    if (tx_info->tx_action.bypass_flags == SWITCH_BYPASS_ALL) {
      SWITCH_PKTINFO_TX_DEV_PORT(pkt_info) = tx_info->tx_action.dev_port;
      SWITCH_PKTINFO_TX_BYPASS(pkt_info) = TRUE;
    } else {
      bd = tx_info->tx_action.bd;
      SWITCH_PKTINFO_INGRESS_BD(pkt_info) = bd;
      SWITCH_PKTINFO_TX_DEV_PORT(pkt_info) = SWITCH_INVALID_HW_PORT;
    }
    SWITCH_PKTINFO_BYPASS_FLAGS(pkt_info) = tx_info->tx_action.bypass_flags;
    pktdriver_ctx->num_tx_netdev_packets++;
    tx_info->num_packets++;
  } else {
    pktdriver_ctx->num_tx_cb_packets++;
  }

  SWITCH_PKTINFO_PACKET_TYPE(pkt_info) = SWITCH_FABRIC_HEADER_TYPE_CPU;
  SWITCH_PKTINFO_ETHER_TYPE(pkt_info) = SWITCH_FABRIC_HEADER_ETHTYPE;
  SWITCH_PKTINFO_TX_EGRESS_QUEUE(pkt_info) =
      SWITCH_PKTDRIVER_TX_EGRESS_QUEUE_DEFAULT;

  SWITCH_MEMSET(out_packet, 0x0, sizeof(out_packet));

  SWITCH_MEMCPY(
      (out_packet + out_offset), pkt_info->pkt, SWITCH_PACKET_HEADER_OFFSET);
  out_offset += SWITCH_PACKET_HEADER_OFFSET;
  in_offset += SWITCH_PACKET_HEADER_OFFSET;

  SWITCH_PACKET_HEADER_HTON(pkt_info->pkt_header);

  SWITCH_MEMCPY((out_packet + out_offset),
                &pkt_info->pkt_header,
                sizeof(switch_packet_header_t));
  out_offset += sizeof(switch_packet_header_t);
  pkt_size += sizeof(switch_packet_header_t);

  SWITCH_MEMCPY((out_packet + out_offset),
                (pkt_info->pkt + in_offset),
                pkt_info->pkt_size - in_offset);

  status = switch_pktdriver_cpu_tx(pkt_info->device, out_packet, pkt_size);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_DEBUG(
        "pktdriver tx failed: "
        "pktdriver cpu tx failed:(%s)\n",
        switch_error_to_string(status));
    return status;
  }

  pktdriver_ctx->num_tx_packets++;

  return status;
}

switch_status_t switch_pktdriver_netdev_rx(switch_packet_info_t *pkt_info) {
  switch_int32_t rc = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(pkt_info);
  SWITCH_ASSERT(pkt_info->fd != SWITCH_FD_INVALID);

  if (!pkt_info || pkt_info->fd == SWITCH_FD_INVALID) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "pktdriver netdev rx failed: "
        "pkt info invalid:(%s)\n",
        switch_error_to_string(status));
    return status;
  }

  rc = switch_fd_write(pkt_info->fd, pkt_info->pkt, pkt_info->pkt_size);
  if (rc < 0) {
    status = SWITCH_STATUS_FAILURE;
    SWITCH_LOG_ERROR(
        "pktdriver netdev rx failed: "
        "pkt netdev write failed:(%s)\n",
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_pktdriver_bd_to_vlan_mapping_add(switch_device_t device,
                                                        switch_bd_t bd,
                                                        switch_vlan_t vlan) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(vlan < SWITCH_MAX_VLANS);
  pktdriver_ctx = switch_config_packet_driver_context_get();
  pktdriver_ctx->bd_mapping[vlan] = bd;

  return status;
}

switch_status_t switch_pktdriver_bd_to_vlan_mapping_delete(
    switch_device_t device, switch_bd_t bd, switch_vlan_t vlan) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(vlan < SWITCH_MAX_VLANS);
  pktdriver_ctx = switch_config_packet_driver_context_get();
  pktdriver_ctx->bd_mapping[vlan] = 0;

  return status;
}

switch_status_t switch_pktdriver_bd_to_vlan_mapping_get(switch_bd_t bd,
                                                        switch_vlan_t *vlan) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_uint16_t index = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  pktdriver_ctx = switch_config_packet_driver_context_get();

  *vlan = 0;
  status = SWITCH_STATUS_ITEM_NOT_FOUND;

  if (bd == 0) return status;

  for (index = 1; index < SWITCH_MAX_VLANS; index++) {
    if (pktdriver_ctx->bd_mapping[index] == bd) {
      *vlan = index;
      return SWITCH_STATUS_SUCCESS;
    }
  }

  return status;
}

switch_status_t switch_pktdriver_rx_filter_action_transform(
    switch_pktdriver_rx_filter_key_t *rx_key,
    switch_pktdriver_rx_filter_info_t *rx_info,
    switch_packet_info_t *pkt_info) {
  switch_pktdriver_rx_filter_action_t *rx_action = NULL;
  switch_ethernet_header_t *eth_header = NULL;
  switch_vlan_header_t *vlan_header = NULL;
  switch_vlan_t vlan_id = 0;
  switch_uint16_t ether_type = 0;
  switch_uint16_t *vlan_p = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(rx_info && pkt_info);

  rx_action = &rx_info->rx_action;

  if (rx_action->vlan_action == SWITCH_PACKET_VLAN_ACTION_NONE) {
    return status;
  }

  eth_header = (switch_ethernet_header_t *)(pkt_info->pkt);
  ether_type = switch_htons(eth_header->ether_type);

  if (rx_action->vlan_action == SWITCH_PACKET_VLAN_ACTION_ADD) {
    if (ether_type == SWITCH_ETHERTYPE_DOT1Q) {
      return status;
    }

    status = switch_pktdriver_bd_to_vlan_mapping_get(rx_key->bd, &vlan_id);
    if (status != SWITCH_STATUS_SUCCESS) {
      return status;
    }

    SWITCH_MEMCPY(
        pkt_info->pkt + SWITCH_ETH_HEADER_SIZE + SWITCH_VLAN_HEADER_SIZE,
        pkt_info->pkt + SWITCH_ETH_HEADER_SIZE,
        pkt_info->pkt_size - SWITCH_ETH_HEADER_SIZE);
    pkt_info->pkt_size += SWITCH_VLAN_HEADER_SIZE;
    vlan_header = (switch_vlan_header_t *)((switch_uint8_t *)(pkt_info->pkt) +
                                           SWITCH_ETH_HEADER_SIZE);
    vlan_header->tpid = eth_header->ether_type;
    vlan_header->dei = 0;
    vlan_header->pcp = 0;
    vlan_header->vid = vlan_id;
    vlan_p = (switch_uint16_t *)vlan_header;
    *vlan_p = switch_htons(*vlan_p);
    eth_header->ether_type = switch_ntohs(SWITCH_ETHERTYPE_DOT1Q);

  } else if (rx_action->vlan_action == SWITCH_PACKET_VLAN_ACTION_REMOVE) {
    if (ether_type != SWITCH_ETHERTYPE_DOT1Q) {
      return status;
    }

    vlan_header = (switch_vlan_header_t *)((switch_uint8_t *)(pkt_info->pkt) +
                                           SWITCH_ETH_HEADER_SIZE);
    eth_header->ether_type = vlan_header->tpid;

    SWITCH_MEMCPY(
        pkt_info->pkt + SWITCH_ETH_HEADER_SIZE,
        pkt_info->pkt + SWITCH_ETH_HEADER_SIZE + SWITCH_VLAN_HEADER_SIZE,
        pkt_info->pkt_size - SWITCH_ETH_HEADER_SIZE - SWITCH_VLAN_HEADER_SIZE);
    pkt_info->pkt_size -= SWITCH_VLAN_HEADER_SIZE;
  }

  return status;
}

switch_status_t switch_pktdriver_rx(switch_packet_info_t *pkt_info) {
  switch_pktdriver_rx_filter_key_t rx_key;
  switch_pktdriver_rx_filter_info_t *rx_info = NULL;
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_dev_port_t dev_port = 0;
  switch_port_t fp_port = 0;
  switch_hostif_reason_code_t reason_code = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  pktdriver_ctx = switch_config_packet_driver_context_get();

  SWITCH_MEMCPY(&pkt_info->pkt_header,
                pkt_info->pkt + SWITCH_PACKET_HEADER_OFFSET,
                sizeof(switch_packet_header_t));
  SWITCH_MEMCPY(pkt_info->pkt + sizeof(switch_packet_header_t),
                pkt_info->pkt,
                SWITCH_PACKET_HEADER_OFFSET);

  pkt_info->pkt += sizeof(switch_packet_header_t);
  pkt_info->pkt_size = pkt_info->pkt_size - sizeof(switch_packet_header_t);

  SWITCH_PACKET_HEADER_NTOH(pkt_info->pkt_header);

  if (pktdriver_ctx->rx_pkt_trace_enable) {
    switch_pktdriver_packet_dump(pkt_info);
  }

  if (SWITCH_PKTINFO_ETHER_TYPE(pkt_info) != SWITCH_FABRIC_HEADER_ETHTYPE) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "pktdriver rx failed: "
        "fabric header ethertype invalid:(%s)\n",
        switch_error_to_string(status));
    return status;
  }

  reason_code = SWITCH_PKTINFO_REASON_CODE(pkt_info);
  if (reason_code < SWITCH_HOSTIF_REASON_CODE_MAX) {
    pktdriver_ctx->rx_rc_counters[reason_code].num_packets++;
    pktdriver_ctx->rx_rc_counters[reason_code].num_bytes += pkt_info->pkt_size;
  }

  dev_port = SWITCH_PKTINFO_RX_DEV_PORT(pkt_info);
  if (dev_port < SWITCH_MAX_PORTS) {
    status = switch_device_front_port_get(pkt_info->device, dev_port, &fp_port);
    if (status == SWITCH_STATUS_SUCCESS) {
      pktdriver_ctx->rx_port_counters[fp_port].num_packets++;
      pktdriver_ctx->rx_port_counters[fp_port].num_bytes += pkt_info->pkt_size;
    }
  }

  if (!pktdriver_ctx->knet_pkt_driver[pkt_info->device]) {
    SWITCH_MEMSET(&rx_key, 0x0, sizeof(rx_key));
    rx_key.dev_port = SWITCH_PKTINFO_RX_DEV_PORT(pkt_info);
    rx_key.ifindex = SWITCH_PKTINFO_INGRESS_IFINDEX(pkt_info);
    rx_key.bd = SWITCH_PKTINFO_INGRESS_BD(pkt_info);
    rx_key.reason_code = SWITCH_PKTINFO_REASON_CODE(pkt_info);
    status = switch_pktdriver_rx_filter_info_get(&rx_key, &rx_info);
    if (status == SWITCH_STATUS_SUCCESS) {
      pkt_info->fd = rx_info->rx_action.fd;
      switch_pktdriver_rx_filter_action_transform(&rx_key, rx_info, pkt_info);
      status = switch_pktdriver_netdev_rx(pkt_info);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR(
            "pktdriver rx failed for rx key:"
            "dev port: %d ifindex: 0x%x"
            "bd %d rc 0x%x\n",
            rx_key.dev_port,
            rx_key.ifindex,
            rx_key.bd,
            rx_key.reason_code);
        return status;
      }
      rx_info->num_packets++;
      pktdriver_ctx->num_rx_netdev_packets++;
    }
  }

  status = switch_hostif_callback_rx(pkt_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pktdriver rx failed:"
        "hostif rx cb failed:(%s)\n",
        switch_error_to_string(status));
    return status;
  }

  pktdriver_ctx->num_rx_cb_packets++;
  pktdriver_ctx->num_rx_packets++;

  return status;
}

switch_status_t switch_pktdriver_knet_cpu_rx(switch_fd_t knet_fd) {
  switch_packet_info_t pkt_info;
  switch_int8_t in_packet[SWITCH_PACKET_MAX_BUFFER_SIZE];
  switch_int32_t pkt_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  pkt_size = switch_fd_read(knet_fd, in_packet, sizeof(in_packet));
  if (pkt_size <= 0) {
    status = SWITCH_STATUS_FAILURE;
    SWITCH_LOG_ERROR("packet knet cpu rx failed: packet size < 0\n");
    return status;
  }

  SWITCH_MEMSET(&pkt_info, 0x0, sizeof(pkt_info));
  pkt_info.pkt_type = SWITCH_PKTDRIVER_PACKET_TYPE_RX_CPU_KNET;
  pkt_info.pkt = in_packet;
  pkt_info.pkt_size = pkt_size;

  status = switch_pktdriver_rx(&pkt_info);
  return status;
}
switch_status_t switch_pktdriver_cpu_eth_rx(switch_fd_t cpu_fd) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_packet_info_t pkt_info;
  switch_int8_t in_packet[SWITCH_PACKET_MAX_BUFFER_SIZE];
  switch_int32_t pkt_size = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  pktdriver_ctx = switch_config_packet_driver_context_get();
  SWITCH_ASSERT(pktdriver_ctx->cpu_fd == cpu_fd);

  pkt_size = switch_fd_read(cpu_fd, in_packet, sizeof(in_packet));
  if (pkt_size <= 0) {
    status = SWITCH_STATUS_FAILURE;
    SWITCH_LOG_ERROR("packet cpu eth rx failed: packet size < 0\n");
    return status;
  }

  SWITCH_MEMSET(&pkt_info, 0x0, sizeof(pkt_info));
  pkt_info.pkt_type = SWITCH_PKTDRIVER_PACKET_TYPE_RX_CPU_ETH;
  pkt_info.pkt = in_packet;
  pkt_info.pkt_size = pkt_size;

  status = switch_pktdriver_rx(&pkt_info);
  return status;
}

switch_status_t switch_pktdriver_netdev_tx(switch_fd_t hostif_fd) {
  switch_packet_info_t pkt_info;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_int32_t pkt_size = 0;
  switch_int8_t in_packet[SWITCH_PACKET_MAX_BUFFER_SIZE];

  SWITCH_ASSERT(hostif_fd != SWITCH_FD_INVALID);

  pkt_size = switch_fd_read(hostif_fd, in_packet, sizeof(in_packet));
  if (pkt_size <= 0) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "pktdriver netdev tx failed for fd %d: "
        "pkt size is less than 0:(%s)\n",
        hostif_fd,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMSET(&pkt_info, 0x0, sizeof(pkt_info));
  pkt_info.fd = hostif_fd;
  pkt_info.pkt = in_packet;
  pkt_info.pkt_size = pkt_size;
  pkt_info.pkt_type = SWITCH_PKTDRIVER_PACKET_TYPE_TX_NETDEV;

  status = switch_pktdriver_tx(&pkt_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pktdriver netdev tx failed for fd %d: "
        "pktdriver cpu tx failed:(%s)\n",
        hostif_fd,
        switch_error_to_string(status));
    return status;
  }

  return status;
}

switch_status_t switch_pktdriver_fd_add(const switch_device_t device,
                                        const switch_fd_t fd) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  pktdriver_ctx = switch_config_packet_driver_context_get();

  SWITCH_ASSERT(fd != SWITCH_FD_INVALID);

  status = SWITCH_ARRAY_INSERT(
      &pktdriver_ctx->fd_array, fd, (void *)((switch_uint64_t)fd));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "packet fd add failed on device %d fd 0x%x: "
        "fd array insert failed(%s)\n",
        device,
        fd,
        switch_error_to_string(status));
    return status;
  }

  switch_pktdriver_pipe_dummy_write(pktdriver_ctx->pipe_fd[1]);

  return status;
}

switch_status_t switch_pktdriver_fd_delete(const switch_device_t device,
                                           const switch_fd_t fd) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  pktdriver_ctx = switch_config_packet_driver_context_get();

  SWITCH_ASSERT(fd != SWITCH_FD_INVALID);

  status = SWITCH_ARRAY_DELETE(&pktdriver_ctx->fd_array, fd);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "packet fd add failed on device %d fd 0x%x: "
        "fd array delete failed(%s)\n",
        device,
        fd,
        switch_error_to_string(status));
    return status;
  }

  switch_pktdriver_pipe_dummy_write(pktdriver_ctx->pipe_fd[1]);

  return status;
}

switch_status_t switch_packet_driver_fd_update(switch_fd_set *read_fds,
                                               switch_int32_t *nfds) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_fd_t hostif_fd = 0;
  switch_fd_t *hostif_tmp_fd = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  switch_int32_t high_fd = 0;
  pktdriver_ctx = switch_config_packet_driver_context_get();

  SWITCH_ASSERT(read_fds != NULL);
  SWITCH_ASSERT(nfds != NULL);
  if (!read_fds || !nfds) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("packet fd update failed", switch_error_to_string(status));
    return status;
  }

  *nfds = 0;
  FD_ZERO(read_fds);

  FOR_EACH_IN_ARRAY(
      hostif_fd, pktdriver_ctx->fd_array, switch_fd_t, hostif_tmp_fd) {
    UNUSED(hostif_tmp_fd);

    FD_SET(hostif_fd, read_fds);
    high_fd = (high_fd > hostif_fd) ? high_fd : hostif_fd;
  }
  FOR_EACH_IN_ARRAY_END();

  *nfds = high_fd + 1;
  return status;
}

static inline int is_knet_fd(switch_fd_t fd) {
  switch_pktdriver_context_t *pktdriver_ctx =
      switch_config_packet_driver_context_get();
  for (int i = 0; i < SWITCH_MAX_DEVICE; i++) {
    if (pktdriver_ctx->knet_pkt_driver[i]) {
      if (pktdriver_ctx->switch_kern_info[i].sock_fd == fd) return 1;
    }
  }
  return 0;
}

switch_status_t switch_packet_demux(switch_fd_set *read_fds) {
  switch_pktdriver_context_t *pktdriver_ctx =
      switch_config_packet_driver_context_get();
  switch_fd_t *tmp_fd = NULL;
  switch_fd_t fd = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(read_fds != NULL);
  if (!read_fds) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR("packet demux failed: %s", switch_error_to_string(status));
    return status;
  }

  FOR_EACH_IN_ARRAY(fd, pktdriver_ctx->fd_array, switch_fd_t, tmp_fd) {
    UNUSED(tmp_fd);

    if (pktdriver_ctx->cpu_fd == fd &&
        FD_ISSET(pktdriver_ctx->cpu_fd, read_fds)) {
      status = switch_pktdriver_cpu_eth_rx(pktdriver_ctx->cpu_fd);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("packet demux failed from cpu fd 0x%lx: %s\n",
                         pktdriver_ctx->cpu_fd,
                         switch_error_to_string(status));
        return status;
      }
    } else if (pktdriver_ctx->pipe_fd[0] == fd &&
               FD_ISSET(pktdriver_ctx->pipe_fd[0], read_fds)) {
      SWITCH_ASSERT(fd == pktdriver_ctx->pipe_fd[0]);
      switch_pktdriver_pipe_dummy_read(pktdriver_ctx->pipe_fd[0]);
    } else if (is_knet_fd(fd) && FD_ISSET(fd, read_fds)) {
      status = switch_pktdriver_knet_cpu_rx(fd);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("packet demux failed from knet cpu fd %lx: %s\n",
                         fd,
                         switch_error_to_string(status));
        return status;
      }
    } else if (FD_ISSET(fd, read_fds)) {
      status = switch_pktdriver_netdev_tx(fd);
      if (status != SWITCH_STATUS_SUCCESS) {
        SWITCH_LOG_ERROR("packet demux failed from fd %d: %s\n",
                         fd,
                         switch_error_to_string(status));
      }
    }
  }
  FOR_EACH_IN_ARRAY_END();

  return status;
}

static void *switch_packet_driver(void *args) {
  switch_fd_set read_fds;
  switch_int32_t num_fd = 0;
  switch_int32_t rc = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_status_t tmp_status = SWITCH_STATUS_SUCCESS;
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  status = switch_pktdriver_init();
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("packet driver hostif init failed:(%s)",
                     switch_error_to_string(status));
    goto cleanup;
  }
  pthread_mutex_lock(&cookie_mutex);
  cookie = 1;
  pthread_cond_signal(&cookie_cv);
  pthread_mutex_unlock(&cookie_mutex);

  while (TRUE) {
    status = switch_packet_driver_fd_update(&read_fds, &num_fd);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("packet driver fd update failed: %s",
                       switch_error_to_string(status));
      goto cleanup;
    }

    if (!num_fd) {
      continue;
    }

    rc = switch_select(num_fd, &read_fds, NULL, NULL, NULL);
    if (rc < 0) {
      SWITCH_LOG_ERROR("packet driver select failed: %s",
                       switch_error_to_string(status));
      goto cleanup;
    }

    if (rc == 0) {
      SWITCH_LOG_DETAIL("packet driver without fds. ignoring");
      continue;
    }

    switch_packet_demux(&read_fds);
  }

cleanup:
  tmp_status = switch_pktdriver_free();
  if (tmp_status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_DETAIL("packet driver cleanup failed!");
  }
  return NULL;
}

static inline uint64_t bit_mask(uint64_t x) {
  uint64_t temp = 1;
  return (x >= sizeof(uint64_t) * CHAR_BIT) ? (uint64_t)-1 : (temp << x) - 1;
}

#if !defined(BMV2) && !defined(BMV2TOFINO)
static inline switch_status_t switch_knet_rx_filter_create(
    const switch_device_t device,
    const switch_pktdriver_rx_filter_priority_t priority,
    const switch_uint64_t flags,
    const switch_pktdriver_rx_filter_key_t *rx_key,
    const switch_pktdriver_rx_filter_action_t *rx_action,
    switch_handle_t *rx_filter_handle) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_knet_info_t *knet_info = NULL;
  switch_knet_rx_filter_t rx_filter;
  switch_cpu_header_t *filter_packet_hdr = NULL;
  switch_cpu_header_t *mask_packet_hdr = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  size_t cpu_hdr_offset = 0;

  SWITCH_MEMSET(&rx_filter, 0, sizeof(switch_knet_rx_filter_t));
  rx_filter.spec.priority = priority;
  /* We index into an array hence -1 */
  cpu_hdr_offset =
      sizeof(switch_ethernet_header_t) + sizeof(switch_fabric_header_t) - 2;
  // Check filter size vs packet size
  filter_packet_hdr =
      (switch_cpu_header_t *)(rx_filter.spec.filter + cpu_hdr_offset);
  mask_packet_hdr =
      (switch_cpu_header_t *)(rx_filter.spec.mask + cpu_hdr_offset);

  if (flags & SWITCH_PKTDRIVER_RX_FILTER_ATTR_DEV_PORT) {
    filter_packet_hdr->ingress_port = switch_htons(rx_key->dev_port);
    mask_packet_hdr->ingress_port =
        bit_mask(8 * sizeof(filter_packet_hdr->ingress_port));
  }
  if (flags & SWITCH_PKTDRIVER_RX_FILTER_ATTR_BD) {
    filter_packet_hdr->ingress_bd = switch_htons(rx_key->bd);
    mask_packet_hdr->ingress_bd =
        bit_mask(8 * sizeof(filter_packet_hdr->ingress_bd));
  }
  if (flags & SWITCH_PKTDRIVER_RX_FILTER_ATTR_REASON_CODE) {
    filter_packet_hdr->reason_code = switch_htons(rx_key->reason_code);
    mask_packet_hdr->reason_code =
        bit_mask(8 * sizeof(filter_packet_hdr->reason_code));
  }
  if (flags & SWITCH_PKTDRIVER_RX_FILTER_ATTR_IFINDEX) {
    filter_packet_hdr->ingress_ifindex = switch_htons(rx_key->ifindex);
    mask_packet_hdr->ingress_ifindex =
        bit_mask(8 * sizeof(filter_packet_hdr->ingress_ifindex));
  }
  rx_filter.spec.filter_size = sizeof(switch_ethernet_header_t) +
                               sizeof(switch_cpu_header_t) +
                               sizeof(switch_fabric_header_t);

  rx_filter.action.dest_proto = 0;
  if (rx_action->knet_hostif_handle) {
    rx_filter.action.dest_type = BF_KNET_FILTER_DESTINATION_HOSTIF;
    rx_filter.action.knet_hostif_id = rx_action->knet_hostif_handle;
    rx_filter.action.count = 1;
    rx_filter.action.pkt_mutation =
        SWITCH_CALLOC(device, sizeof(switch_knet_packet_mutation_t), 1);
    if (rx_filter.action.pkt_mutation == NULL) {
      status = SWITCH_STATUS_NO_MEMORY;
      SWITCH_LOG_ERROR(
          "knet hostif rx filter create failed on device %d: "
          "knet action mutation malloc failed:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
    rx_filter.action.pkt_mutation[0].mutation_type = BF_KNET_RX_MUT_STRIP;
    rx_filter.action.pkt_mutation[0].offset =
        offsetof(switch_ethernet_header_t, ether_type);
    rx_filter.action.pkt_mutation[0].len = sizeof(switch_packet_header_t);
  } else {
    rx_filter.action.dest_type = BF_KNET_FILTER_DESTINATION_CPUIF;
    rx_filter.action.count = 0;
  }

  pktdriver_ctx = switch_config_packet_driver_context_get();
  knet_info = &pktdriver_ctx->switch_kern_info[device];
  status = switch_pd_status_to_status(
      bf_knet_rx_filter_add(knet_info->knet_cpuif_id, &rx_filter));
  if (status != SWITCH_STATUS_SUCCESS) goto ret;

  *rx_filter_handle = rx_filter.spec.filter_id;
  status = SWITCH_STATUS_SUCCESS;
ret:
  if (rx_filter.action.count > 0)
    SWITCH_FREE(device, rx_filter.action.pkt_mutation);
  return status;
}
#endif

switch_status_t switch_pktdriver_rx_filter_create_internal(
    const switch_device_t device,
    const switch_pktdriver_rx_filter_priority_t priority,
    const switch_uint64_t flags,
    const switch_pktdriver_rx_filter_key_t *rx_key,
    const switch_pktdriver_rx_filter_action_t *rx_action,
    switch_handle_t *rx_filter_handle) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_pktdriver_rx_filter_info_t *rx_info = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_ASSERT(rx_key && rx_action);
  if (!rx_key || !rx_action) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "pktdriver rx filter create failed on device %d: "
        "parameters null:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  pktdriver_ctx = switch_config_packet_driver_context_get();
  if (pktdriver_ctx->knet_pkt_driver[device]) {
#if !defined(BMV2) && !defined(BMV2TOFINO)
    status = switch_knet_rx_filter_create(
        device, priority, flags, rx_key, rx_action, rx_filter_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "pktdriver rx filter create failed on device %d: "
          "knet rx filter add failed:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
#endif
    return status;
  }

  handle = switch_pktdriver_rx_filter_handle_create(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pktdriver rx filter create failed on device %d: "
        "rx filter handle create failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pktdriver_rx_filter_get(device, handle, &rx_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pktdriver rx filter create failed on device %d: "
        "rx filter handle get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(&rx_info->rx_key, rx_key, sizeof(*rx_key));
  SWITCH_MEMCPY(&rx_info->rx_action, rx_action, sizeof(*rx_action));

  status =
      SWITCH_LIST_INSERT(&pktdriver_ctx->rx_filter, &rx_info->node, rx_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pktdriver rx filter create failed on device %d: "
        "rx filter list insert failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_LIST_SORT(&pktdriver_ctx->rx_filter,
                            switch_packet_driver_rx_filter_priority_compare);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pktdriver rx filter create failed on device %d: "
        "rx filter sort failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  rx_info->flags = flags;
  rx_info->priority = priority;
  *rx_filter_handle = handle;

  SWITCH_LOG_DEBUG(
      "packet driver rx filter created on device %d "
      "handle 0x%lx\n",
      device,
      handle);

  return status;
}

switch_status_t switch_pktdriver_rx_filter_delete_internal(
    const switch_device_t device, const switch_handle_t rx_filter_handle) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_pktdriver_rx_filter_info_t *rx_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  pktdriver_ctx = switch_config_packet_driver_context_get();

  if (pktdriver_ctx->knet_pkt_driver[device]) {
#if !defined(BMV2) && !defined(BMV2TOFINO)
    status = switch_pd_status_to_status(bf_knet_rx_filter_delete(
        pktdriver_ctx->switch_kern_info[device].knet_cpuif_id,
        rx_filter_handle));
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "pktdriver rx filter delete failed on device %d: "
          "knet filter delete failed:(%s)\n",
          device,
          switch_error_to_string(status));
      return status;
    }
#endif
    return status;
  }

  SWITCH_ASSERT(SWITCH_PKTDRIVER_RX_FILTER_HANDLE(rx_filter_handle));
  if (!SWITCH_PKTDRIVER_RX_FILTER_HANDLE(rx_filter_handle)) {
    SWITCH_LOG_ERROR(
        "packet driver rx filter delete failed on device %d "
        "handle 0x%lx: rx filter handle invalid:(%s)\n",
        device,
        rx_filter_handle,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pktdriver_rx_filter_get(device, rx_filter_handle, &rx_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "packet driver rx filter delete failed on device %d "
        "handle 0x%lx: rx filter get failed:(%s)\n",
        device,
        rx_filter_handle,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_LIST_DELETE(&pktdriver_ctx->rx_filter, &rx_info->node);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "packet driver rx filter delete failed on device %d "
        "handle 0x%lx: rx filter list delete failed:(%s)\n",
        device,
        rx_filter_handle,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "packet driver rx filter deleted on device %d "
      "handle 0x%lx\n",
      device,
      rx_filter_handle);

  return status;
}

#if !defined(BMV2) && !defined(BMV2TOFINO)
static switch_status_t switch_knet_tx_filter_create(
    const switch_device_t device,
    const switch_pktdriver_tx_filter_priority_t priority,
    const switch_uint64_t flags,
    const switch_pktdriver_tx_filter_key_t *tx_key,
    const switch_pktdriver_tx_filter_action_t *tx_action,
    switch_handle_t *tx_filter_handle) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_knet_info_t *knet_info = NULL;
  switch_knet_tx_action_t knet_tx_action;
  switch_packet_header_t *packet_hdr = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(tx_filter_handle);

  SWITCH_MEMSET(&knet_tx_action, 0, sizeof(switch_knet_tx_action_t));
  knet_tx_action.count = 1;
  knet_tx_action.pkt_mutation =
      SWITCH_CALLOC(device, sizeof(switch_knet_packet_mutation_t), 1);
  if (knet_tx_action.pkt_mutation == NULL) {
    status = SWITCH_STATUS_NO_MEMORY;
    SWITCH_LOG_ERROR(
        "knet hostif tx filter create failed on device %d: "
        "knet action mutation malloc failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }
  knet_tx_action.pkt_mutation[0].mutation_type = BF_KNET_RX_MUT_INSERT;
  knet_tx_action.pkt_mutation[0].offset =
      offsetof(switch_ethernet_header_t, ether_type);
  knet_tx_action.pkt_mutation[0].len = sizeof(switch_packet_header_t);
  if (knet_tx_action.pkt_mutation[0].offset +
          knet_tx_action.pkt_mutation[0].len <
      BF_KNET_DATA_BYTES_MAX) {
    packet_hdr =
        (switch_packet_header_t *)(knet_tx_action.pkt_mutation[0].data);
  } else {
    status = SWITCH_STATUS_FAILURE;
    SWITCH_LOG_ERROR(
        "knet hostif tx filter create failed on device %d: "
        "knet action mutation index %d is greater than knet max muation index "
        "%d\n",
        device,
        knet_tx_action.pkt_mutation[0].offset +
            knet_tx_action.pkt_mutation[0].len,
        BF_KNET_DATA_BYTES_MAX);
    return status;
  }

  SWITCH_MEMSET(knet_tx_action.pkt_mutation[0].data, 0, BF_KNET_DATA_BYTES_MAX);
  if (tx_action->bypass_flags == SWITCH_BYPASS_ALL) {
    packet_hdr->fabric_header.dst_port_or_group =
        switch_htons(tx_action->dev_port);
    packet_hdr->cpu_header.tx_bypass = true;
  } else {
    packet_hdr->fabric_header.dst_port_or_group = SWITCH_INVALID_HW_PORT;
    packet_hdr->cpu_header.ingress_bd = switch_htons(tx_action->bd);
  }
  packet_hdr->cpu_header.reason_code = switch_htons(tx_action->bypass_flags);
  packet_hdr->fabric_header.packet_type = SWITCH_FABRIC_HEADER_TYPE_CPU;
  packet_hdr->fabric_header.ether_type =
      switch_htons(SWITCH_FABRIC_HEADER_ETHTYPE);
  packet_hdr->cpu_header.egress_queue =
      SWITCH_PKTDRIVER_TX_EGRESS_QUEUE_DEFAULT;

  pktdriver_ctx = switch_config_packet_driver_context_get();
  knet_info = &pktdriver_ctx->switch_kern_info[device];
#if !defined(BMV2) && !defined(BMV2TOFINO)
  status = switch_pd_status_to_status(bf_knet_tx_action_add(
      knet_info->knet_cpuif_id, tx_key->knet_hostif_handle, &knet_tx_action));
#endif
  return status;
}
#endif

switch_status_t switch_pktdriver_tx_filter_create_internal(
    const switch_device_t device,
    const switch_pktdriver_tx_filter_priority_t priority,
    const switch_uint64_t flags,
    const switch_pktdriver_tx_filter_key_t *tx_key,
    const switch_pktdriver_tx_filter_action_t *tx_action,
    switch_handle_t *tx_filter_handle) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_pktdriver_tx_filter_info_t *tx_info = NULL;
  switch_handle_t handle = SWITCH_API_INVALID_HANDLE;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  pktdriver_ctx = switch_config_packet_driver_context_get();

  SWITCH_ASSERT(tx_key && tx_action);
  if (!tx_key || !tx_action) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "pktdriver tx filter create failed on device %d: "
        "parameters null:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  if (pktdriver_ctx->knet_pkt_driver[device]) {
#if !defined(BMV2) && !defined(BMV2TOFINO)
    status = switch_knet_tx_filter_create(
        device, priority, flags, tx_key, tx_action, tx_filter_handle);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "pktdriver tx filter create failed on device %d handle 0x%lx: "
          "knet tx filter add failed:(%s)\n",
          device,
          tx_key->knet_hostif_handle,
          switch_error_to_string(status));
      return status;
    }
#endif
    return status;
  }

  handle = switch_pktdriver_tx_filter_handle_create(device);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pktdriver tx filter create failed on device %d: "
        "tx filter handle create failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pktdriver_tx_filter_get(device, handle, &tx_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pktdriver tx filter create failed on device %d: "
        "tx filter handle get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_MEMCPY(&tx_info->tx_key, tx_key, sizeof(*tx_key));
  SWITCH_MEMCPY(&tx_info->tx_action, tx_action, sizeof(*tx_action));

  status =
      SWITCH_LIST_INSERT(&pktdriver_ctx->tx_filter, &tx_info->node, tx_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pktdriver tx filter create failed on device %d: "
        "tx filter list insert failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_LIST_SORT(&pktdriver_ctx->tx_filter,
                            switch_packet_driver_tx_filter_priority_compare);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pktdriver tx filter create failed on device %d: "
        "tx filter sort failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  tx_info->flags = flags;
  tx_info->priority = priority;
  *tx_filter_handle = handle;

  SWITCH_LOG_DEBUG(
      "packet driver tx filter created on device %d "
      "handle 0x%lx\n",
      device,
      handle);

  return status;
}

switch_status_t switch_pktdriver_tx_filter_delete_internal(
    const switch_device_t device, const switch_handle_t tx_filter_handle) {
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_pktdriver_tx_filter_info_t *tx_info = NULL;
  switch_knet_info_t *knet_info = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(knet_info);

  pktdriver_ctx = switch_config_packet_driver_context_get();
  if (pktdriver_ctx->knet_pkt_driver[device]) {
#if !defined(BMV2) && !defined(BMV2TOFINO)
    knet_info = &pktdriver_ctx->switch_kern_info[device];
    status = switch_pd_status_to_status(
        bf_knet_tx_action_delete(knet_info->knet_cpuif_id, tx_filter_handle));
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "pktdriver tx filter delete failed on device %d handle 0x%lx : "
          "knet tx action delete failed: (%s)\n",
          device,
          tx_filter_handle,
          switch_error_to_string(status));
      return status;
    }
#endif
    return status;
  }

  SWITCH_ASSERT(SWITCH_PKTDRIVER_TX_FILTER_HANDLE(tx_filter_handle));
  if (!SWITCH_PKTDRIVER_TX_FILTER_HANDLE(tx_filter_handle)) {
    SWITCH_LOG_ERROR(
        "packet driver tx filter delete failed on device %d "
        "handle 0x%lx: tx filter handle invalid:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = switch_pktdriver_tx_filter_get(device, tx_filter_handle, &tx_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "packet driver tx filter delete failed on device %d "
        "handle 0x%lx: tx filter get failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  status = SWITCH_LIST_DELETE(&pktdriver_ctx->tx_filter, &tx_info->node);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "packet driver tx filter delete failed on device %d "
        "handle 0x%lx: tx filter list delete failed:(%s)\n",
        device,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_LOG_DEBUG(
      "packet driver tx filter deleted on device %d "
      "handle 0x%lx\n",
      device,
      tx_filter_handle);

  return status;
}

#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)

bf_status_t switch_pktdriver_cpu_pcie_rx(bf_dev_id_t device,
                                         bf_pkt *pkt,
                                         void *cookie,
                                         bf_pkt_rx_ring_t rx_ring) {
  bf_pkt *orig_pkt = NULL;
  static char in_packet[SWITCH_PACKET_MAX_BUFFER_SIZE];
  switch_packet_info_t pkt_info;
  char *pkt_buf = NULL;
  char *bufp = NULL;
  switch_uint32_t packet_size = 0;
  switch_int32_t pkt_len = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  /* save a copy of the original packet */
  orig_pkt = pkt;

  /* assemble the received packet */
  bufp = &in_packet[0];
  do {
    pkt_buf = (char *)bf_pkt_get_pkt_data(pkt);
    pkt_len = bf_pkt_get_pkt_size(pkt);
    if ((packet_size + pkt_len) > SWITCH_PACKET_MAX_BUFFER_SIZE) {
      SWITCH_LOG_ERROR("Packet too large to Transmit - SKipping\n");
      break;
    }
    memcpy(bufp, pkt_buf, pkt_len);
    bufp += pkt_len;
    packet_size += pkt_len;
    pkt = bf_pkt_get_nextseg(pkt);
  } while (pkt);

  /* free the packet */
  bf_pkt_free(device, orig_pkt);
  /* process the received packet buffer */
  SWITCH_MEMSET(&pkt_info, 0x0, sizeof(pkt_info));
  pkt_info.pkt_type = SWITCH_PKTDRIVER_PACKET_TYPE_RX_CPU_PCIE;
  pkt_info.pkt = in_packet;
  pkt_info.pkt_size = packet_size;

  status = switch_pktdriver_rx(&pkt_info);
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "pktdriver cpu pcie rx failed: "
        "pktdriver rx failed:(%s)\n",
        switch_error_to_string(status));
    return 0;
  }

  return 0;
}

static bf_status_t switch_pktdriver_tx_complete(bf_dev_id_t device,
                                                bf_pkt_tx_ring_t tx_ring,
                                                uint64_t tx_cookie,
                                                uint32_t status) {
  /* free the packet */

  bf_pkt *pkt = (bf_pkt *)(uintptr_t)tx_cookie;
  bf_pkt_free(device, pkt);
  return 0;
}
#endif /* __TARGET_TOFINO__ || BMV2TOFINO */

switch_status_t switch_pktdriver_callback_register(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(status);

#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
  bf_pkt_tx_ring_t tx_ring;
  bf_pkt_rx_ring_t rx_ring;

  /* register callback for TX complete */
  for (tx_ring = BF_PKT_TX_RING_0; tx_ring < BF_PKT_TX_RING_MAX; tx_ring++) {
    bf_pkt_tx_done_notif_register(
        device, switch_pktdriver_tx_complete, tx_ring);
  }

  /* register callback for RX */
  for (rx_ring = BF_PKT_RX_RING_0; rx_ring < BF_PKT_RX_RING_MAX; rx_ring++) {
    bf_pkt_rx_register(device, switch_pktdriver_cpu_pcie_rx, rx_ring, 0);
  }
#endif /* __TARGET_TOFINO__ || BMV2TOFINO */

  return status;
}

switch_status_t switch_pktdriver_callback_degister(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(status);

#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
  bf_pkt_tx_ring_t tx_ring;
  bf_pkt_rx_ring_t rx_ring;

  /* register callback for TX complete */
  for (tx_ring = BF_PKT_TX_RING_0; tx_ring < BF_PKT_TX_RING_MAX; tx_ring++) {
    bf_pkt_tx_done_notif_deregister(device, tx_ring);
  }

  /* register callback for RX */
  for (rx_ring = BF_PKT_RX_RING_0; rx_ring < BF_PKT_RX_RING_MAX; rx_ring++) {
    bf_pkt_rx_deregister(device, rx_ring);
  }
#endif /* __TARGET_TOFINO__ || BMV2TOFINO */

  return status;
}

switch_int32_t start_switch_api_packet_driver(void) {
  switch_pktdriver_callback_register(0);
  pthread_mutex_init(&cookie_mutex, NULL);
  pthread_cond_init(&cookie_cv, NULL);
  int status =
      pthread_create(&packet_driver_thread, NULL, switch_packet_driver, NULL);
  if (status) return status;
  pthread_setname_np(packet_driver_thread, "bf_swapi_pkdrv");
  pthread_mutex_lock(&cookie_mutex);
  while (!cookie) {
    pthread_cond_wait(&cookie_cv, &cookie_mutex);
  }
  pthread_mutex_unlock(&cookie_mutex);
  pthread_mutex_destroy(&cookie_mutex);
  pthread_cond_destroy(&cookie_cv);
  return SWITCH_STATUS_SUCCESS;
}

switch_int32_t stop_switch_api_packet_driver(void) {
  int status = pthread_cancel(packet_driver_thread);
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_knet_info_t *knet_info = NULL;
  switch_status_t tmp_status = SWITCH_STATUS_SUCCESS;

  if (status == 0) {
    pthread_join(packet_driver_thread, NULL);
  }
  pktdriver_ctx = switch_config_packet_driver_context_get();
  if (pktdriver_ctx->knet_pkt_driver[0]) {
    knet_info = &pktdriver_ctx->switch_kern_info[0];
    if (knet_info->sock_fd != SWITCH_FD_INVALID) {
      switch_fd_close(knet_info->sock_fd);
    }
  } else {
    tmp_status = switch_pktdriver_free();
    if (tmp_status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR("packet driver cleanup failed!");
    }
  }
  tmp_status = switch_pktdriver_callback_degister(0);
  if (tmp_status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR("packet driver cleanup failed!");
  }
  return SWITCH_STATUS_SUCCESS;
}

bool switch_pktdriver_mode_is_kernel_internal(const switch_device_t device) {
  UNUSED(device);
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
  return bf_knet_module_is_inited();
#else
  return false;
#endif
}

switch_status_t switch_pktdriver_knet_device_add_internal(
    const switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_knet_info_t *knet_info = NULL;
  char cpuif_netdev_name[IFNAMSIZ] = "";

#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
  pktdriver_ctx = switch_config_packet_driver_context_get();
  status = switch_pd_status_to_status(
      bf_pal_cpuif_netdev_name_get(device, cpuif_netdev_name, IFNAMSIZ));
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  knet_info = &pktdriver_ctx->switch_kern_info[device];
  SWITCH_MEMSET(knet_info, 0x0, sizeof(switch_knet_info_t));
  status = switch_pd_status_to_status(
      bf_knet_cpuif_ndev_add(cpuif_netdev_name,
                             knet_info->cpuif_knetdev_name,
                             &knet_info->knet_cpuif_id));
  if (status != SWITCH_STATUS_SUCCESS) {
    SWITCH_LOG_ERROR(
        "device add failed for device %d:"
        "knet cpuif add failed for %s:(%s)",
        device,
        cpuif_netdev_name,
        switch_error_to_string(status));

    return SWITCH_STATUS_FAILURE;
  }
  pktdriver_ctx->knet_pkt_driver[device] = true;
#endif
  return status;
}

switch_status_t switch_pktdriver_knet_device_delete_internal(
    const switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pktdriver_context_t *pktdriver_ctx = NULL;
  switch_knet_info_t *knet_info = NULL;

  pktdriver_ctx = switch_config_packet_driver_context_get();
  if (pktdriver_ctx->knet_pkt_driver[device]) {
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
    status = switch_pktdriver_knet_device_delete(device);
    knet_info = &pktdriver_ctx->switch_kern_info[device];
    status = bf_knet_cpuif_ndev_delete(knet_info->knet_cpuif_id);
    if (status != SWITCH_STATUS_SUCCESS) {
      SWITCH_LOG_ERROR(
          "device delete failed on device %d"
          "knet delete failed for cpuif id %lu: %s",
          device,
          knet_info->knet_cpuif_id,
          switch_error_to_string(status));
      return status;
    }
#endif
  }
  return status;
}

switch_status_t switch_pktdriver_tx_filter_create(
    const switch_device_t device,
    const switch_pktdriver_tx_filter_priority_t priority,
    const switch_uint64_t flags,
    const switch_pktdriver_tx_filter_key_t *tx_key,
    const switch_pktdriver_tx_filter_action_t *tx_action,
    switch_handle_t *tx_filter_handle) {
  SWITCH_MT_WRAP(switch_pktdriver_tx_filter_create_internal(
      device, priority, flags, tx_key, tx_action, tx_filter_handle));
}

switch_status_t switch_pktdriver_tx_filter_delete(
    const switch_device_t device, const switch_handle_t tx_filter_handle) {
  SWITCH_MT_WRAP(
      switch_pktdriver_tx_filter_delete_internal(device, tx_filter_handle));
}

switch_status_t switch_pktdriver_rx_filter_delete(
    const switch_device_t device, const switch_handle_t rx_filter_handle) {
  SWITCH_MT_WRAP(
      switch_pktdriver_rx_filter_delete_internal(device, rx_filter_handle));
}

switch_status_t switch_pktdriver_rx_filter_create(
    const switch_device_t device,
    const switch_pktdriver_rx_filter_priority_t priority,
    const switch_uint64_t flags,
    const switch_pktdriver_rx_filter_key_t *rx_key,
    const switch_pktdriver_rx_filter_action_t *rx_action,
    switch_handle_t *rx_filter_handle) {
  SWITCH_MT_WRAP(switch_pktdriver_rx_filter_create_internal(
      device, priority, flags, rx_key, rx_action, rx_filter_handle));
}

bool switch_pktdriver_mode_is_kernel(const switch_device_t device) {
  SWITCH_MT_WRAP(switch_pktdriver_mode_is_kernel_internal(device));
}

switch_status_t switch_pktdriver_knet_device_add(const switch_device_t device) {
  SWITCH_MT_WRAP(switch_pktdriver_knet_device_add_internal(device));
}

switch_status_t switch_pktdriver_knet_device_delete(
    const switch_device_t device) {
  SWITCH_MT_WRAP(switch_pktdriver_knet_device_delete_internal(device));
}
