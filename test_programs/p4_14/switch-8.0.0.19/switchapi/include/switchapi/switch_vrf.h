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

#ifndef __SWITCH_VRF_H__
#define __SWITCH_VRF_H__

#include "switch_base_types.h"
#include "switch_status.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup VRF Virtual Router API
 *  API functions define and manipulate vrf
 *  @{
 */  // begin of vrf API

/***************************************************************************
 * ENUMS
 ***************************************************************************/

/** vrf attributes */
typedef enum switch_vrf_attr_s {
  SWITCH_VRF_ATTR_RMAC_HANDLE = (1 << 0),
  SWITCH_VRF_ATTR_IPV4_UNICAST = (1 << 1),
  SWITCH_VRF_ATTR_IPV6_UNICAST = (1 << 2)
} switch_vrf_attr_t;

/***************************************************************************
 * STRUCTS
 ***************************************************************************/

/** vrf attriute structure */
typedef struct switch_api_vrf_info_s {
  /** ipv4 unicast enabled */
  bool ipv4_unicast_enabled;

  /** ipv6 unicast enabled */
  bool ipv6_unicast_enabled;

  /** rmac handle */
  switch_handle_t rmac_handle;

} switch_api_vrf_info_t;

/***************************************************************************
 * APIS
 ***************************************************************************/

/**
 * @brief Create a virtual router
 *
 * @param[in] device device id
 * @param[in] vrf_id id to uniquely identify a virtual router. vrf id
 *            is optional. If set to 0, an internal vrf id will be allocated.
 * @param[out] vrf_handle virtual router handle
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_vrf_create(const switch_device_t device,
                                      const switch_vrf_t vrf_id,
                                      switch_handle_t *vrf_handle);

/**
 * @brief Delete a virtual router
 *
 * @param[in] device device id
 * @param[in] vrf_handle virtual router handle
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_vrf_delete(const switch_device_t device,
                                      const switch_handle_t vrf_handle);

/**
 * @brief Delete a virtual router by vrf identifier
 *
 * @param[in] device device id
 * @param[in] vrf_id virtual router identifier
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_vrf_id_delete(const switch_device_t device,
                                         const switch_vrf_t vrf_id);

/**
 * @brief Get virtual router handle from vrf identifier
 *
 * @param[in] device device id
 * @param[in] vrf_id virtual router identifier
 * @param[out] vrf_handle virtual router handle
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_vrf_id_to_handle_get(const switch_device_t device,
                                                const switch_vrf_t vrf_id,
                                                switch_handle_t *vrf_handle);

/**
 * @brief Get virtual router identifier from vrf handle
 *
 * @param[in] device device id
 * @param[in] vrf_handle virtual router handle
 * @param[out] vrf_id virtual router identifier
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_vrf_handle_to_id_get(
    const switch_device_t device,
    const switch_handle_t vrf_handle,
    switch_vrf_t *vrf_id);

/**
 * @brief Dump virtual router identifier info in CLI
 *
 * @param[in] device device id
 * @param[in] vrf_handle virtual router handle
 * @param[in] cli_ctx cli context
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_vrf_handle_dump(const switch_device_t device,
                                           const switch_handle_t vrf_handle,
                                           const void *cli_ctx);

/**
 * @brief Set a rmac group to virtual router
 *
 * @param[in] device device id
 * @param[in] vrf_handle virtual router handle
 * @param[in] rmac_handle router mac handle
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_vrf_rmac_handle_set(
    const switch_device_t device,
    const switch_handle_t vrf_handle,
    const switch_handle_t rmac_handle);

/**
 * @brief Get rmac group for a virtual router
 *
 * @param[in] device device id
 * @param[in] vrf_handle virtual router handle
 * @param[out] rmac_handle router mac handle
 *
 * @return #SWITCH_STATUS_SUCCESS if success otherwise error code is
 * returned.
 */
switch_status_t switch_api_vrf_rmac_handle_get(const switch_device_t device,
                                               const switch_handle_t vrf_handle,
                                               switch_handle_t *rmac_handle);

/** @} */  // end of VRF API

#ifdef __cplusplus
}
#endif

#endif /** __SWITCH_VRF_H__ */
