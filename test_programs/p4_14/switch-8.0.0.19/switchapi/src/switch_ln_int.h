/*
Copyright 2013-present Barefoot Networks, Inc.
*/

#ifndef __SWITCH_LN_INT_H__
#define __SWITCH_LN_INT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_LN_HANDLE_SIZE 4096

#define switch_ln_handle_create(_device) \
  switch_handle_create(                  \
      _device, SWITCH_HANDLE_TYPE_LOGICAL_NETWORK, sizeof(switch_ln_info_t))

#define switch_ln_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_LOGICAL_NETWORK, _handle)

#define switch_ln_get(_device, _handle, _info) \
  switch_handle_get(                           \
      _device, SWITCH_HANDLE_TYPE_LOGICAL_NETWORK, _handle, (void **)_info)

typedef struct switch_ln_info_s {
  /** bridge domain handle */
  switch_handle_t bd_handle;

  /** l3 interface handle */
  switch_handle_t l3_intf_handle;

  /** hostif handle */
  switch_handle_t hostif_handle;

} switch_ln_info_t;

switch_status_t switch_ln_init(switch_device_t device);

switch_status_t switch_ln_free(switch_device_t device);

switch_status_t switch_ln_default_entries_add(switch_device_t device);

switch_status_t switch_ln_default_entries_delete(switch_device_t device);

#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_LN_INT_H__ */
