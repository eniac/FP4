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
#ifdef __TARGET_BMV2__
#define BMV2
#endif

#ifdef __TARGET_TOFINO__
#include <tofino/constants.p4>
#include <tofino/intrinsic_metadata.p4>
#include <tofino/primitives.p4>
#include <tofino/pktgen_headers.p4>
#include <tofino/stateful_alu_blackbox.p4>
#include <tofino/wred_blackbox.p4>
#else
#include "includes/tofino.p4"
#endif

#include "includes/p4features.h"
#include "includes/drop_reason_codes.h"
#include "includes/cpu_reason_codes.h"
#include "includes/p4_pktgen.h"
#include "includes/defines.p4"
#include "includes/headers.p4"
#include "includes/p4_table_sizes.h"
#include "includes/parser.p4"

/* METADATA */
header_type ingress_metadata_t {
    fields {
        ingress_port : 9;                         /* input physical port */
        port_lag_index : PORT_LAG_INDEX_BIT_WIDTH;      /* ingress port index */
        egress_port_lag_index : PORT_LAG_INDEX_BIT_WIDTH;/* egress port index */
        ifindex : IFINDEX_BIT_WIDTH;              /* ingress interface index */
        egress_ifindex : IFINDEX_BIT_WIDTH;       /* egress interface index */
        port_type : 2;                         /* ingress port type */

        outer_bd : BD_BIT_WIDTH;               /* outer BD */
        bd : BD_BIT_WIDTH;                     /* BD */

        drop_flag : 1;                         /* if set, drop the packet */
        drop_reason : 8;                       /* drop reason */

        control_frame: 1;                      /* control frame */
        bypass_lookups : 8;                    /* list of lookups to skip */
    }
}

header_type egress_metadata_t {
    fields {
#ifdef PTP_ENABLE
        capture_tstamp_on_tx : 1;              /* request for packet departure time capture */
#endif
        bypass : 1;                            /* bypass egress pipeline */
        port_type : 2;                         /* egress port type */
        payload_length : 16;                   /* payload length for tunnels */
        smac_idx : 9;                          /* index into source mac table */
        bd : BD_BIT_WIDTH;                     /* egress inner bd */
        outer_bd : BD_BIT_WIDTH;               /* egress inner bd */
        mac_da : 48;                           /* final mac da */
        routed : 1;                            /* is this replica routed */
        same_bd_check : BD_BIT_WIDTH;          /* ingress bd xor egress bd */
        drop_reason : 8;                       /* drop reason */
        ifindex : IFINDEX_BIT_WIDTH;           /* egress interface index */
        egress_port :  9;                      /* original egress port */
    }
}

header_type intrinsic_metadata_t {
    fields {
        mcast_grp : 16;                           /* multicast group */
        // lf_field_list : 32;                       /* Learn filter field list */
        egress_rid : 16;                          /* replication index */
        ingress_global_timestamp : 32;
    }
}

/* Global config information */
header_type global_config_metadata_t {
    fields {
        enable_dod : 1;                        /* Enable Deflection-on-Drop */
        switch_id  : 32;                       /* Switch Id */
    }
}
#ifdef SFLOW_ENABLE
@pragma pa_atomic ingress ingress_metadata.sflow_take_sample
@pragma pa_solitary ingress ingress_metadata.sflow_take_sample
#endif
//@pragma pa_atomic ingress ingress_metadata.port_type
//@pragma pa_solitary ingress ingress_metadata.port_type
@pragma pa_atomic egress egress_metadata.port_type
@pragma pa_solitary egress egress_metadata.port_type
//#ifndef INT_ENABLE
//@pragma pa_atomic ingress ingress_metadata.port_lag_index
//@pragma pa_solitary ingress ingress_metadata.port_lag_index
//@pragma pa_atomic ingress ingress_metadata.ifindex
//@pragma pa_solitary ingress ingress_metadata.ifindex
//@pragma pa_atomic egress ingress_metadata.bd
//@pragma pa_solitary egress ingress_metadata.bd
//#endif
#if defined(FABRIC_PROFILE)
/* This field is part of bridged metadata.  The fabric
   profile puts a lot of pressure on 16-bit containers.
   Even though the natural container size of this field is 16,
   it can safely be allocated in a 32-bit container since its
   MAU cluster size is 1. */
@pragma pa_container_size ingress ingress_metadata.ifindex 32
#endif

// Workaround for COMPILER-788
#if defined(MSDC_PROFILE) || defined(MSDC_L3_PROFILE)
@pragma pa_solitary ingress ingress_metadata.ingress_port
#endif
// Workaround for COMPILER-844
#ifdef INT_EP_ENABLE
@pragma pa_solitary ingress ingress_metadata.ingress_port
#endif
metadata ingress_metadata_t ingress_metadata;

#ifdef DTEL_REPORT_LB_ENABLE
@pragma pa_no_overlay egress egress_metadata.routed
@pragma pa_solitary egress egress_metadata.routed
#endif
// Workaround for COMPILER-844
#ifdef INT_EP_ENABLE
@pragma pa_solitary egress egress_metadata.egress_port
#endif
metadata egress_metadata_t egress_metadata;
metadata intrinsic_metadata_t intrinsic_metadata;
metadata global_config_metadata_t global_config_metadata;

#include "switch_config.p4"
#ifdef OPENFLOW_ENABLE
#include "openflow.p4"
#endif /* OPENFLOW_ENABLE */
#include "port.p4"
#include "l2.p4"
#include "l3.p4"
#include "ipv4.p4"
#include "ipv6.p4"
#include "tunnel.p4"
#include "acl.p4"
#include "nat.p4"
#include "multicast.p4"
#include "nexthop.p4"
#include "rewrite.p4"
#include "security.p4"
#include "fabric.p4"
#include "egress_filter.p4"
#include "mirror.p4"
#include "hashes.p4"
#include "meter.p4"
#include "sflow.p4"
#include "bfd.p4"
#include "qos.p4"
#include "sr.p4"
#include "flowlet.p4"
#include "pktgen.p4"
#include "failover.p4"
#include "ila.p4"
#include "wred.p4"
#include "dtel.p4"
#include "dtel_int.p4"
#include "dtel_postcard.p4"

action nop() {
}

action on_miss() {
}

control ingress {
    /* input mapping - derive an ifindex */
    process_ingress_port_mapping();

    /* read and apply system configuration parametes */
    process_global_params();
#ifdef PKTGEN_ENABLE
    if (VALID_PKTGEN_PACKET) {
        /* process pkt_gen generated packets */
        process_pktgen();
    } else {
#endif /* PKTGEN_ENABLE */
    /* process outer packet headers */
    // process_validate_outer_header();

    /* process bfd rx packets */
    process_bfd_rx_packet();

#ifdef OPENFLOW_ENABLE
    if (ingress_metadata.port_type == PORT_TYPE_CPU) {
        apply(packet_out);
    }
#endif /* OPENFLOW_ENABLE */

    /* derive bd and its properties  */
    process_port_vlan_mapping();

    /* SRv6 endpoint lookup */
    process_srv6();

    /* spanning tree state checks */
    process_spanning_tree();

    /* ingress fabric processing */
    process_ingress_fabric();

#if !defined(TUNNEL_PARSING_DISABLE)
    /* tunnel termination processing */
    process_tunnel();
#endif /* !TUNNEL_PARSING_DISABLE */

    /* IPSG */
    process_ip_sourceguard();

    /* ingress sflow determination */
    process_ingress_sflow();

    /* storm control */
    process_storm_control();

#ifdef PKTGEN_ENABLE
    }
#endif
    /* common (tx and rx) bfd processing */
    process_bfd_packet();

#if defined(L3_HEAVY_INT_LEAF_PROFILE) || defined(GENERIC_INT_LEAF_PROFILE)
    process_dtel_ingress_prepare();
#endif

#ifdef FABRIC_ENABLE
    if (ingress_metadata.port_type != PORT_TYPE_FABRIC) {
#endif
#ifdef TRANSIENT_LOOP_PREVENTION
        apply(neighbor_detect);
#endif
#ifndef MPLS_DISABLE
    if (not (valid(mpls[0]) and (l3_metadata.fib_hit == TRUE))) {
#endif /* MPLS_DISABLE */
    /* validate packet */
    process_validate_packet();

    /* perform ingress l4 port range */
    process_ingress_l4port();

    /* l2 lookups */
    process_mac();

#if !defined(ACL_SWAP)
    /* port and vlan ACL */
    if (l3_metadata.lkp_ip_type == IPTYPE_NONE) {
        process_mac_acl();
    } else {
        process_ip_acl();
    }
#endif

#if !defined(ENT_DC_GENERAL_PROFILE)
#if defined(INGRESS_PORT_MIRROR_ENABLE)
    process_ingress_port_mirroring();
#endif /* INGRESS_PORT_MIRROR_ENABLE */
#endif /* ENT_DC_GENERAL_PROFILE */

#if defined(GENERIC_INT_LEAF_PROFILE) || defined(L3_HEAVY_INT_LEAF_PROFILE) || \
    defined(ENT_DC_GENERAL_PROFILE) || defined(MSDC_L3_PROFILE)
    if (l3_metadata.lkp_ip_type == IPTYPE_NONE) {
        process_mac_acl();
    }
#endif

    if (l2_metadata.lkp_pkt_type == L2_UNICAST) {
#if defined(L2_DISABLE) && defined(L2_MULTICAST_DISABLE) && defined(L3_MULTICAST_DISABLE)
        {
            {
#else
        apply(rmac) {
            rmac_hit {
#endif /* L2_DISABLE && L2_MULTICAST_DISABLE && L3_MULTICAST_DISABLE */
                if (DO_LOOKUP(L3)) {
                    if ((l3_metadata.lkp_ip_type == IPTYPE_IPV4) and
                        (ipv4_metadata.ipv4_unicast_enabled == TRUE)) {
                            /* router ACL/PBR */
#ifndef RACL_SWAP
                            process_ipv4_racl();
#endif /* !RACL_SWAP */
                            process_ipv4_urpf();
                            process_ipv4_fib();
#ifdef RACL_SWAP
                            process_ipv4_racl();
#endif /* RACL_SWAP */

                    } else {
                        if ((l3_metadata.lkp_ip_type == IPTYPE_IPV6) and
                            (ipv6_metadata.ipv6_unicast_enabled == TRUE)) {
                            /* router ACL/PBR */
#ifndef RACL_SWAP
                            process_ipv6_racl();
#endif /* !RACL_SWAP */
                            process_ipv6_urpf();
                            process_ipv6_fib();
#ifdef RACL_SWAP
                            process_ipv6_racl();
#endif /* RACL_SWAP */
                        }
                    }
                    process_urpf_bd();
                }
            }
        }
    } else {
        process_multicast();
    }

#if defined(GENERIC_INT_LEAF_PROFILE)
    process_dtel_make_upstream_digest();
    process_dtel_int_set_sink();
#endif

#ifdef L3_HEAVY_INT_LEAF_PROFILE
    process_dtel_mod_watchlist();
    process_dtel_int_sink();
#endif

#if defined(ACL_SWAP)
    /* port and vlan ACL */
    if (l3_metadata.lkp_ip_type == IPTYPE_NONE) {
#if !defined(GENERIC_INT_LEAF_PROFILE) && !defined(L3_HEAVY_INT_LEAF_PROFILE) && !defined(ENT_DC_GENERAL_PROFILE) && !defined(MSDC_L3_PROFILE)
        process_mac_acl();
#endif
    } else {
        process_ip_acl();
    }
#endif

    /* ingress NAT */
    process_ingress_nat();

#ifdef ENT_DC_AGGR_PROFILE
    /* ingress qos map */
    process_ingress_qos_map();

    /* FCoE ACL */
    apply(fcoe_acl);
#endif /* ENT_DC_AGGR_PROFILE */

#ifndef MPLS_DISABLE
    }
#endif /* MPLS_DISABLE */
#ifdef FABRIC_ENABLE
    }
#endif

#if !defined(GENERIC_INT_LEAF_PROFILE) && !defined(L3_HEAVY_INT_LEAF_PROFILE)
    /* prepare metadata for DTel */
    process_dtel_ingress_prepare();
#endif

#ifndef L3_HEAVY_INT_LEAF_PROFILE
    /* int_sink process for packets with int_header */
    process_dtel_int_sink();
#endif

#ifdef L3_HEAVY_INT_LEAF_PROFILE
    process_hashes_1();

    process_dtel_int_watchlist();
#endif

    /* compute hashes based on packet type  */
#ifndef L3_HEAVY_INT_LEAF_PROFILE
    process_hashes_1();
#endif
    process_hashes_2();

    process_meter_index();

    /* apply DTel watchlist */
    process_dtel_watchlist();

#ifdef L3_HEAVY_INT_LEAF_PROFILE
    /* decide final forwarding choice */
    process_fwd_results();
#endif

    /* INT i2e mirror */
    process_dtel_int_upstream_report();

#ifdef FABRIC_ENABLE
    if (ingress_metadata.port_type != PORT_TYPE_FABRIC) {
#endif /* FABRIC_ENABLE */
    /* update statistics */
    process_ingress_bd_stats();
    process_ingress_acl_stats();
    process_storm_control_stats();
#ifndef L3_HEAVY_INT_LEAF_PROFILE
    /* decide final forwarding choice */
    process_fwd_results();
#endif
    process_meter_action();

#ifndef ENT_DC_AGGR_PROFILE
    /* ingress qos map */
    process_ingress_qos_map();
#endif

    /* IPv4 Mirror ACL */
    if (l3_metadata.lkp_ip_type == IPTYPE_IPV4) {
        process_ipv4_mirror_acl();
    }

#if defined(ENT_DC_GENERAL_PROFILE)
#if defined(INGRESS_PORT_MIRROR_ENABLE)
    process_ingress_port_mirroring();
#endif
#endif /* ENT_DC_GENERAL_PROFILE */

    /* flowlet */
    process_flowlet();

    /* ecmp/nexthop lookup */
    process_nexthop();

    /* set queue id for tm */
    process_traffic_class();

    /* IPv6 Mirror ACL */
    if (l3_metadata.lkp_ip_type == IPTYPE_IPV6) {
        process_ipv6_mirror_acl();
    }

#ifndef L3_HEAVY_INT_LEAF_PROFILE
    process_dtel_mod_watchlist();
#endif

    if (ingress_metadata.egress_ifindex == IFINDEX_FLOOD) {
        /* resolve multicast index for flooding */
        process_multicast_flooding();
    } else {
        if (tunnel_metadata.tunnel_dst_index != 0) {
            /* tunnel id */
            process_tunnel_id();
        } else {
            /* resolve final egress port for unicast traffic */
            process_lag();
        }
    }

#ifdef OPENFLOW_ENABLE
    /* openflow processing for ingress */
    process_ofpat_ingress();
#endif /* OPENFLOW_ENABLE */

    /* generate learn notify digest if permitted */
    process_mac_learning();
#ifdef FABRIC_ENABLE
    }
#endif /* FABRIC_ENABLE */

    /* IPv6 Mirror ACL */
    process_ingress_mirror_acl_stats();

    /* resolve fabric port to destination device */
    process_fabric_lag();

    /* apply DTel queue related watchlist after queue is chosen */
    process_dtel_queue_watchlist();

    /* RACL stats */
    process_ingress_racl_stats();

    if (ingress_metadata.port_type != PORT_TYPE_FABRIC) {
        /* system acls */
        process_system_acl();
    }

    process_ecn_acl();
}

control egress {

#ifdef OPENFLOW_ENABLE
    if (openflow_metadata.ofvalid == TRUE) {
        process_ofpat_egress();
    } else {
#endif /* OPENFLOW_ENABLE */
    /*
     * if bfd rx pkt is for recirc to correct pipe,
     * skip the rest of the pipeline
     */
    process_bfd_recirc();

    /* Process lag selection fallback */
    process_lag_fallback();

    /* Egress Port Mirroring */
#if defined(EGRESS_PORT_MIRROR_ENABLE)
    if (not pkt_is_mirrored) {
        process_egress_port_mirroring();
    }
#endif /* EGRESS_PORT_MIRROR_ENABLE */

    /* Record egress port for telemetry in case of DoD */
    if (not pkt_is_mirrored) {
        process_dtel_record_egress_port();
    }

    /* check for -ve mirrored pkt */
    if (egress_metadata.bypass == FALSE) {
        if (eg_intr_md.deflection_flag == FALSE) {

            /* multi-destination replication */
            process_rid();

            /* check if pkt is mirrored */
            if (not pkt_is_mirrored) {
                process_egress_bfd_packet();
                process_dtel_prepare_egress();
            } else {
                /* mirror processing */
#ifndef MIRROR_SWAP
                process_mirroring();
#endif
                process_bfd_mirror_to_cpu();
            }

            /* multi-destination replication */
            process_replication();

            if (not pkt_is_mirrored) {
                /* DTel processing -- detect local change and e2e */
                process_dtel_local_report1();
            }

#ifdef L3_HEAVY_INT_LEAF_PROFILE
            apply(egress_port_mapping);
            if (not pkt_is_mirrored) {
                process_dtel_int_sink_ports();
            }
            if(egress_metadata.port_type == PORT_TYPE_NORMAL) { {
#else
            /* determine egress port properties */
            apply(egress_port_mapping) {
                egress_port_type_normal {
#endif /* L3_HEAVY_INT_LEAF_PROFILE */

#ifdef REWRITE_SWAP
                    /* apply nexthop_index based packet rewrites */
                    process_rewrite();
#endif /* REWRITE_SWAP */

                    if (pkt_is_not_mirrored) {
                        /* strip vlan header */
                        process_vlan_decap();
                    }

#if !defined(TUNNEL_PARSING_DISABLE)
                    /* perform tunnel decap */
                    process_tunnel_decap();
#endif /* !TUNNEL_PARSING_DISABLE */

                    /* egress qos map */
                    process_egress_qos_map();

#ifdef DTEL_QUEUE_REPORT_ENABLE
                }
            }
            if (not pkt_is_mirrored) {
                process_dtel_queue_alert_update();
            }
            if(egress_metadata.port_type == PORT_TYPE_NORMAL) { {
#endif /* DTEL_QUEUE_REPORT_ENABLE */

                    /* process segment routing rewrite */
                    process_srv6_rewrite();

#ifndef REWRITE_SWAP
                /* apply nexthop_index based packet rewrites */
                    process_rewrite();
#endif /* !REWRITE_SWAP */
                }
            }

            if (not pkt_is_mirrored) {
                /* DTel processing -- detect local change and e2e */
                process_dtel_local_report2();
            }
#ifdef MIRROR_SWAP
            else {
                process_mirroring();
            }
#endif

            if (egress_metadata.port_type == PORT_TYPE_NORMAL) {

                /* perform egress l4 port range */
                process_egress_l4port();

                /* egress bd properties */
                process_egress_bd();

                /* egress acl */
                process_egress_acl();

                /* wred processing */
                process_wred();

                /* rewrite source/destination mac if needed */
                process_mac_rewrite();

                /* egress nat processing */
                process_egress_nat();

#if !defined(ENT_DC_GENERAL_PROFILE)
                /* update egress bd stats */
                process_egress_bd_stats();
#endif /* ENT_DC_GENERAL_PROFILE */

                /* update egress acl stats */
                process_egress_acl_stats();
            }

            if (pkt_is_mirrored) {
                /* DTel processing -- convert h/w port to frontend port */
                process_dtel_port_convert();
            } else {
                /* DTel processing -- insert header */
                process_dtel_insert();
            }

#if !defined(TUNNEL_PARSING_DISABLE)
            /* perform tunnel encap */
            process_tunnel_encap();
#endif /* !TUNNEL_PARSING_DISABLE */

            /* egress mtu checks */
            process_mtu();

            /* update L4 checksums (if needed) */
            process_l4_checksum();

            /* DTel processing -- update header */
            if (pkt_is_mirrored) {
                process_dtel_report_encap();
            } else {
                process_dtel_insert_2();
            }

            if (egress_metadata.port_type == PORT_TYPE_NORMAL) {
                /* egress vlan translation */
                process_vlan_xlate();
            }

#if defined(ENT_DC_GENERAL_PROFILE)
            /* update egress bd stats */
            process_egress_bd_stats();
#endif /* ENT_DC_GENERAL_PROFILE */

            /* egress filter */
            process_egress_filter();
        } else {
            process_dtel_deflect_on_drop();
        }
    }

    /* apply egress acl */
    process_egress_system_acl();
#ifdef OPENFLOW_ENABLE
    }
#endif /* OPENFLOW_ENABLE */
}
