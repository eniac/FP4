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
#ifndef _switch_pd_types_h_
#define _switch_pd_types_h_

#ifdef SWITCH_PD

#ifndef __TARGET_TOFINO__
#ifdef BMV2
#include "bmpd/switch/pd/pd.h"
#include "bm/pdfixed/pd_pre.h"
#include "bm/pdfixed/pd_mirroring.h"
#include "bm/pdfixed/pd_static.h"
#endif /* BMV2 */
#else
#if defined(BMV2TOFINO)
#include "tofinobmpd/switch/pd/pd.h"
#include "tofinobm/pdfixed/pd_pre.h"
#include "tofinobm/pdfixed/pd_mirroring.h"
#include "tofinobm/pdfixed/pd_static.h"
#include "tofinobm/pdfixed/pd_pktgen.h"
#else
#include <tofinopd/switch/pd/pd.h>
#include <tofino/pdfixed/pd_mc.h>
#include <tofino/pdfixed/pd_tm.h>
#include <tofino/pdfixed/pd_devport_mgr.h>
#include <tofino/pdfixed/pd_conn_mgr.h>
#include <tofino/pdfixed/pd_mirror.h>
#include <tofino/pdfixed/pd_conn_mgr.h>
#include <tofino/bf_pal/bf_pal_port_intf.h>
#include <tofino/bf_pal/pltfm_intf.h>
#include <tofino/bf_pal/dev_intf.h>
#include <mc_mgr/mc_mgr_intf.h>
#include "pkt_mgr/pkt_mgr_intf.h"
#include "knet_mgr/bf_knet_if.h"
#include "pre.h"
#endif /* BMV2TOFINO */
#endif /* __TARGET_TOFINO__ */

#if !defined(BMV2) && !defined(BMV2TOFINO)
typedef p4_pd_pool_id_t switch_pd_pool_id_t;
typedef p4_pd_tm_ppg_t switch_tm_ppg_hdl_t;
typedef p4_pd_pvs_hdl_t switch_pd_pvs_hdl_t;
typedef bf_knet_filter_t switch_knet_filter_t;
typedef bf_knet_rx_filter_t switch_knet_rx_filter_t;
typedef bf_knet_tx_action_t switch_knet_tx_action_t;
typedef bf_knet_hostif_knetdev_t switch_knet_hostif_knetdev_t;
typedef bf_knet_cpuif_t switch_knet_cpuif_t;
typedef bf_knet_hostif_t switch_knet_hostif_t;
typedef bf_knet_filter_t switch_knet_filter_t;
typedef bf_knet_packet_mutation_t switch_knet_packet_mutation_t;

#else
typedef uint64_t switch_knet_cpuif_t;
typedef uint64_t switch_knet_hostif_t;
typedef uint64_t switch_knet_filter_t;
typedef uint64_t switch_knet_rx_filter_t;
typedef uint64_t switch_knet_tx_action_t;
typedef uint64_t switch_knet_hostif_knetdev_t;
typedef uint64_t switch_knet_packet_mutation_t;
typedef uint16_t switch_pd_pool_id_t;
typedef uint16_t switch_tm_ppg_hdl_t;
typedef uint32_t switch_pd_pvs_hdl_t;

#endif /* BMV2 && BMV2TOIFNO */

#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)

static inline bf_loopback_mode_e switch_lb_mode_to_pd_lb_mode(
    switch_port_loopback_mode_t lb_mode) {
  switch (lb_mode) {
    case SWITCH_PORT_LOOPBACK_MODE_NONE:
      return BF_LPBK_NONE;
    case SWITCH_PORT_LOOPBACK_MODE_MAC_NEAR:
      return BF_LPBK_MAC_NEAR;
    case SWITCH_PORT_LOOPBACK_MODE_MAC_FAR:
      return BF_LPBK_MAC_FAR;
    case SWITCH_PORT_LOOPBACK_MODE_PHY_NEAR:
      return BF_LPBK_SERDES_NEAR;
    case SWITCH_PORT_LOOPBACK_MODE_PHY_FAR:
      return BF_LPBK_SERDES_FAR;
    default:
      return BF_LPBK_NONE;
  }
}

static inline bf_fec_type_t switch_fec_mode_to_bf_fec_type(
    switch_port_fec_mode_t fec_type) {
  switch (fec_type) {
    case SWITCH_PORT_FEC_MODE_NONE:
      return BF_FEC_TYP_NONE;
    case SWITCH_PORT_FEC_MODE_FC:
      return BF_FEC_TYP_FIRECODE;
    case SWITCH_PORT_FEC_MODE_RS:
      return BF_FEC_TYP_REED_SOLOMON;
    default:
      return BF_FEC_TYP_NONE;
  }
}

#endif /* defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO) */

#else

typedef uint16_t switch_pd_pool_id_t;
typedef uint16_t switch_tm_ppg_hdl_t;

#endif /* SWITCH_PD */

typedef struct switch_pd_feature_s {
  bool l2;
  bool acl;
  bool ingress_acl_range;
  bool egress_acl_range;
  bool bfd_offload;
  bool egress_filter;
  bool fast_failover;
  bool ila;
  bool int_ep;
  bool int_transit;
  bool int_digest;
  bool int_l45;
  bool ipsg;
  bool ipv4;
  bool ipv6;
  bool l3;
  bool l2_multicast;
  bool l3_multicast;
  bool tunnel_multicast;
  bool meter;
  bool mirror;
  bool mirror_on_drop;
  bool mpls;
  bool mpls_udp;
  bool multicast;
  bool nat;
  bool nvgre;
  bool geneve;
  bool pktgen;
  bool qos;
  bool basic_ingress_qos;
  bool racl;
  bool acl_qos;
  bool racl_stats;
  bool egress_acl;
  bool fabric;
  bool egress_acl_stats;
  bool egress_outer_bd_stats;
  bool mirror_acl_stats;
  bool resilient_hash;
  bool sflow;
  bool flowlet;
  bool storm_control;
  bool stats;
  bool sr;
  bool stp;
  bool tunnel;
  bool ipv6_tunnel;
  bool urpf;
  bool wcmp;
  bool mirror_wcmp;
  bool mirror_acl;
  bool dtel_apx_stateful;
  bool dtel_stateless_sup;
  bool dtel_mirror_lb;
  bool dtel_report;
  bool dtel_watch;
  bool ingress_mac_acl;
  bool egress_mac_acl;
  bool tunnel_nexthop;
  bool tunnel_opt;
  bool ingress_uc_self_fwd_check_disable;
  bool tunnel_v4_vxlan;
  bool copp_color_drop;
  bool same_bd_check_disable;
  bool qos_metering;
} switch_pd_feature_t;

switch_status_t switch_pd_feature_set(void);

switch_pd_feature_t *switch_pd_feature_get(void);

/* size of bridge metadata */
#define SWITCH_PACKET_ADJUST_LENGTH -28

#endif /* _switch_pd_types_h_ */
