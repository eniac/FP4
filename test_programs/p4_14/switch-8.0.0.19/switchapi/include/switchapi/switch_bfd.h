/*
 * Copyright 2016-present Barefoot Networks, Inc.
 */

#ifndef _switch_bfd_h
#define _switch_bfd_h

#include "switch_base_types.h"
#include "switch_handle.h"
#include "p4_table_sizes.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_MAX_BFD_SESSIONS MAX_BFD_SESSIONS /* from p4_table_sizes.h */

/*
 * Timer granularity(below) allows for 20ms < timer < 5.1sec
 * #define SWITCH_PKTGEN_BFD_TIMER_USEC 20000
 * XXX using a higher granularity for initial develpoment with model
 */
#define SWITCH_PKTGEN_BFD_TIMER_USEC 500000

/**
*   @defgroup BFD Offload
*  API functions to manage BFD session offload
*  @{
*/

/** BFD Session Information */
typedef struct switch_api_bfd_session_info {
  uint32_t my_disc;             /**< local BFD descriminator */
  uint32_t your_disc;           /**< peer BFD descriminator */
  uint8_t detect_mult;          /**< multiplier for rx timeout */
  uint32_t desired_tx_interval; /**< desired tx interval */
  uint32_t min_rx_interval;     /**< Minimum rx interval */
  uint32_t tx_interval;         /**< Negotiated Tx interval (usec) */
  uint32_t rx_interval;         /**< Negotiated Rx interval (usec) */
  /* remote values - for change detection */
  uint32_t remote_desired_tx_interval; /**< peer desired rx interval (usec) */
  uint32_t remote_min_rx_interval;     /**< peer minimum rx interval (usec) */
  /* echo interval is not used - no offloaded in echo/demand-mode */
  /* transport info */
  switch_ip_addr_t sip;     /**< local ip address */
  switch_ip_addr_t dip;     /**< peer ip address */
  uint16_t sport;           /**< udp source port for the session */
  uint16_t dport;           /**< udp dstport - 1-hop, multihop bfd session */
  switch_handle_t vrf_hdl;  /**< VRF handle */
  switch_handle_t rmac_hdl; /**< router mac handle */
  switch_mac_addr_t rmac;   /**< router mac */
} switch_api_bfd_session_info_t;

/**
 Offload a BFD sesion to datapath
 @param device device on which BFD session is offloaded
 @param bfd_api_info - BFD session information
 @param bfd_handle - Handle returned for the BFD session created
*/
switch_status_t switch_api_bfd_session_create(
    switch_device_t device,
    switch_api_bfd_session_info_t *bfd_api_info,
    switch_handle_t *bfd_handle);

/**
 Delete offloaded BFD sesion from datapath
 @param device device on which BFD session is offloaded
 @param bfd_handle - BFD session handle for the session being deleted
*/
switch_status_t switch_api_bfd_session_delete(switch_device_t device,
                                              switch_handle_t bfd_session_hdl);

/** @} */
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _swtich_bfd_h */
