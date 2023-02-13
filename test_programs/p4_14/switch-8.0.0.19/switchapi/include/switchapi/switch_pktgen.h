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

#ifndef _switch_pktgen_h
#define _switch_pktgen_h

#include "switch_base_types.h"
#include "switch_handle.h"
#include "arpa/inet.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define ETHERTYPE_BF_PKTGEN 0x9001
#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_IPV6 0x86dd

#define IP_PROTOCOLS_UDP 17

#define UDP_PORT_BFD_1HOP 3784
#define UDP_PORT_BFD_ECHO 3785
#define UDP_PORT_BFD_MHOP 4784

// common packet header definitions - could use it from netinet files on linux
typedef struct ethernet_header {
  uint8_t mac_da[6];
  uint8_t mac_sa[6];
  uint16_t ether_type;
} ethernet_header_t;

typedef struct ipv4_header {
  uint8_t ver_ihl;
  uint8_t diffserv;
  uint16_t total_len;
  uint16_t ipid;
  uint16_t flags_offset;
  uint8_t ttl;
  uint8_t proto;
  uint16_t hdr_chksum;
  uint32_t sip;
  uint32_t dip;
} ipv4_header_t;

typedef struct udp_header {
  uint16_t sport;
  uint16_t dport;
  uint16_t len;
  uint16_t chksum;
} udp_header_t;

typedef struct bfd_header {
  uint8_t ver_diag;     // ver(3), diag(5)
  uint8_t state_flags;  // state(2), flags(6)
  uint8_t detect_mult;
  uint8_t len;
  uint32_t my_disc;
  uint32_t your_disc;
  uint32_t desired_tx_interval;
  uint32_t required_rx_interval;
  uint32_t required_echo_rx_interval;
} bfd_header_t;

/*
 * extension header must be used by all pktgen apps
 * it must match the mac_sa, eth_type portion of the ethernet header
 * apps can re-purpose the 6 pad bytes to carry whatever information
 * that the app needs
 */
typedef struct switch_pktgen_ext_header {
  uint8_t pad[6];
  uint16_t ether_type;
} switch_pktgen_ext_header_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _swtich_pktgen_h */
