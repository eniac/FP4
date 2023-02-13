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

#ifndef __SWITCH_RMAC_H__
#define __SWITCH_RMAC_H__

#include "switch_base_types.h"
#include "switch_status.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup RMAC Router MAC API
 *  API functions define and manipulate router mac groups
 *  @{
 */  // begin of Router MAC API

/***************************************************************************
 * ENUMS
 ***************************************************************************/

/**
 * @brief Router mac type
 *
 * Outer or inner rmac table is programmed based on the rmac type.
 * source mac rewrite index is allocated only for the first mac entry.
 * smac index is not reallocated or reprogrammed when the first mac is
 * deleted.
 */
typedef enum switch_rmac_type_s {
  SWITCH_RMAC_TYPE_OUTER = (1 << 0),
  SWITCH_RMAC_TYPE_INNER = (1 << 1),
  SWITCH_RMAC_TYPE_ALL = 0x3
} switch_rmac_type_t;

/***************************************************************************
 * APIS
 ***************************************************************************/

/**
 * @brief Create a router mac group
 *
 * @param[in] device device id
 * @param[in] rmac_type router mac type - outer/inner/all
 * @param[out] rmac_handle router mac handle
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_router_mac_group_create(
    const switch_device_t device,
    const switch_rmac_type_t rmac_type,
    switch_handle_t *rmac_handle);

/**
 * @brief Delete a router mac group
 *
 * @param[in] device device id
 * @param[in] rmac_handle router mac handle
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_router_mac_group_delete(
    const switch_device_t device, const switch_handle_t rmac_handle);

/**
 * @brief Create a router mac group with rmacs
 *
 * @param[in] device device id
 * @param[in] num_macs number of router mac addresses
 * @param[in] mac router mac array
 * @param[out] rmac_handle router mac handle
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_router_mac_group_create_with_macs(
    const switch_device_t device,
    const switch_size_t num_macs,
    const switch_mac_addr_t *mac,
    switch_handle_t *rmac_handle);

/**
 * @brief Add a router mac to rmac group
 *
 * @param[in] device device id
 * @param[in] rmac_handle router mac handle
 * @param[in] mac mac address
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_router_mac_add(const switch_device_t device,
                                          const switch_handle_t rmac_handle,
                                          const switch_mac_addr_t *mac);

/**
 * @brief Delete a router mac from rmac group
 *
 * @param[in] device device id
 * @param[in] rmac_handle router mac handle
 * @param[in] mac mac address
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_router_mac_delete(const switch_device_t device,
                                             const switch_handle_t rmac_handle,
                                             const switch_mac_addr_t *mac);

/**
 * @brief Returns all router macs belonging to a router mac group
 *
 * @param[in] device device id
 * @param[in] rmac_handle router mac handle
 * @param[out] num_entries number of mac entries
 * @param[out] macs mac array
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_rmac_macs_get(const switch_device_t device,
                                         const switch_handle_t rmac_handle,
                                         switch_uint16_t *num_entries,
                                         switch_mac_addr_t **macs);

/**
 * @brief Dumps router mac info using clish context
 *
 * @param[in] device device id
 * @param[in] rmac_handle router mac handle
 * @param[in] cli_ctx cli context
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_rmac_handle_dump(const switch_device_t device,
                                            const switch_handle_t rmac_handle,
                                            const void *cli_ctx);

/** @} */  // end of Router MAC API

#ifdef __cplusplus
}
#endif

#endif /** __SWITCH_RMAC_H__ */
