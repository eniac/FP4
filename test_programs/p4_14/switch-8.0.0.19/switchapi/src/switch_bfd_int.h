/*
 * Copyright 2016-present Barefoot Networks, Inc.
 */

#include "p4_table_sizes.h"
#include "p4_pktgen.h"
#include "switchapi/switch_bfd.h"
#include "switch_pd_types.h"

#ifndef _SWITCH_BFD_INT_H_
#define _SWITCH_BFD_INT_H_

#define switch_bfd_handle_create(_device) \
  switch_handle_create(                   \
      _device, SWITCH_HANDLE_TYPE_BFD, sizeof(switch_bfd_info_t))

#define switch_bfd_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_BFD, _handle)

#define switch_bfd_get(_device, _handle, _info) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_BFD, _handle, (void **)_info)

typedef struct switch_bfd_info {
  switch_api_bfd_session_info_t api_info;
  uint8_t tx_mult;
  uint8_t rx_mult;
  uint16_t session_id;
  /* various entry handles per bfd session */
  switch_pd_hdl_t tx_session_table_entry_hdl;
  switch_pd_hdl_t tx_timer_table_entry_hdl;
  switch_pd_hdl_t rx_timers_table_check_entry_hdl;
  switch_pd_hdl_t rx_session_table_entry_hdl;
  switch_pd_hdl_t rx_timers_table_reset_entry_hdl;
} switch_bfd_info_t;

switch_status_t switch_bfd_init(switch_device_t device);

switch_status_t switch_bfd_free(switch_device_t device);

#endif  // _SWITCH_BFD_INT_H_
