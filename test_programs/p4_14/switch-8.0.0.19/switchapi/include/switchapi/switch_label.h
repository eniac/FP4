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

#ifndef __SWITCH_LABEL_H__
#define __SWITCH_LABEL_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum switch_label_type_ {
  SWITCH_LABEL_TYPE_NONE = 0x0,
  SWITCH_LABEL_TYPE_VLAN = 0x1,
  SWITCH_LABEL_TYPE_INTERFACE = 0x2
} switch_label_type_t;

switch_status_t switch_api_label_create(switch_device_t device,
                                        switch_label_type_t label_type,
                                        switch_handle_t *label_handle);

switch_status_t switch_api_label_delete(switch_device_t device,
                                        switch_handle_t label_handle);

switch_status_t switch_api_label_member_add(switch_device_t device,
                                            switch_handle_t label_handle,
                                            switch_handle_t handle);

switch_status_t switch_api_label_member_delete(switch_device_t device,
                                               switch_handle_t label_handle,
                                               switch_handle_t handle);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_LABEL_H__ */
