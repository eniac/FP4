#ifndef _switch_internal_h_
#define _switch_internal_h_

#include "arpa/inet.h"
#include "string.h"
#include "unistd.h"
#include <assert.h>
#include <asm/byteorder.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/if_arp.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#define __USE_GNU 1
#include <pthread.h>
#include <bfutils/Judy.h>
#include "bfutils/clish/shell.h"
#include <bfsys/bf_sal/bf_sys_sem.h>

#include <switchapi/switch_handle.h>
#include <switchapi/switch_base_types.h>
#include <switchapi/switch_log.h>
#include <switchapi/switch_status.h>
#include <switchapi/switch_table.h>
#include <switchapi/switch_id.h>
#include <switchapi/switch_acl.h>
#include <switchapi/switch_vlan.h>
#include <switchapi/switch_stp.h>
#include <switchapi/switch_l2.h>
#include <switchapi/switch_ln.h>
#include <switchapi/switch_hostif.h>
#include <switchapi/switch_tunnel.h>
#include <switchapi/switch_rif.h>
#include <switchapi/switch_interface.h>
#include <switchapi/switch_l3.h>
#include <switchapi/switch_nhop.h>
#include <switchapi/switch_neighbor.h>
#include <switchapi/switch_mirror.h>
#include <switchapi/switch_meter.h>
#include <switchapi/switch_port.h>
#include <switchapi/switch_rmac.h>
#include <switchapi/switch_nat.h>
#include <switchapi/switch_hash.h>
#include <switchapi/switch_wred.h>
#include <switchapi/switch_config.h>
#include <switchapi/switch_device.h>
#include <switchapi/switch_qos.h>
#include <switchapi/switch_buffer.h>
#include <switchapi/switch_queue.h>
#include <switchapi/switch_bfd.h>
#include <switchapi/switch_pktgen.h>
#include <switchapi/switch_ila.h>
#include <switchapi/switch_label.h>
#include <switchapi/switch_vrf.h>
#include <switchapi/switch_lag.h>
#include <switchapi/switch_mcast.h>
#include <switchapi/switch_rpf.h>
#include <switchapi/switch_dtel.h>
#include <switchapi/switch_mpls.h>

#include "switch_types_int.h"
#include "switch_lpm_int.h"
#include "switch_handle_int.h"
#include "switch_acl_int.h"
#include "switch_tunnel_int.h"
#include "switch_mpls_int.h"
#include "switch_rif_int.h"
#include "switch_interface_int.h"
#include "switch_vlan_int.h"
#include "switch_bd_int.h"
#include "switch_nhop_int.h"
#include "switch_neighbor_int.h"
#include "switch_packet_int.h"
#include "switch_hostif_int.h"
#include "switch_stp_int.h"
#include "switch_mirror_int.h"
#include "switch_meter_int.h"
#include "switch_vrf_int.h"
#include "switch_l2_int.h"
#include "switch_l3_int.h"
#include "switch_log_int.h"
#include "switch_device_int.h"
#include "switch_config_int.h"
#include "switch_port_int.h"
#include "switch_lag_int.h"
#include "switch_sflow_int.h"
#include "switch_sr_int.h"
#include "switch_mcast_int.h"
#include "switch_rmac_int.h"
#include "switch_nat_int.h"
#include "switch_buffer_int.h"
#include "switch_qos_int.h"
#include "switch_queue_int.h"
#include "switch_ln_int.h"
#include "switch_pd_common.h"
#include "switch_table_int.h"
#include "switch_bfd_int.h"
#include "switch_dtel_int.h"
#include "switch_hash_int.h"
#include "switch_wred_int.h"
#include "switch_ila_int.h"
#include "switch_label_int.h"
#include "switch_rpf_int.h"
#include "switch_failover_int.h"
#include "switch_pd.h"
#include "switch_pd_failover.h"
#include "switch_pd_pktgen.h"
#include "switch_pd_dtel.h"
#include "switch_scheduler_int.h"
#include "switch_cli_int.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_DEV_ID 0x0
#define SWITCH_DEV_PIPE_ID 0xFFFF

switch_status_t SWITCH_ARRAY_INIT(switch_array_t *array);

switch_uint32_t SWITCH_ARRAY_COUNT(switch_array_t *array);

switch_status_t SWITCH_ARRAY_INSERT(switch_array_t *array,
                                    switch_uint64_t index,
                                    void *ptr);

switch_status_t SWITCH_ARRAY_GET(switch_array_t *array,
                                 switch_uint64_t index,
                                 void **ptr);

switch_status_t SWITCH_ARRAY_DELETE(switch_array_t *array,
                                    switch_uint64_t index);

switch_status_t SWITCH_LIST_INIT(switch_list_t *list);

switch_size_t SWITCH_LIST_COUNT(switch_list_t *list);

bool SWITCH_LIST_EMPTY(switch_list_t *list);

switch_status_t SWITCH_LIST_SORT(switch_list_t *list,
                                 switch_list_compare_func_t compare_func);

switch_status_t SWITCH_LIST_INSERT(switch_list_t *list,
                                   switch_node_t *node,
                                   void *ptr);

switch_status_t SWITCH_LIST_DELETE(switch_list_t *list, switch_node_t *node);

switch_status_t SWITCH_HASHTABLE_INIT(switch_hashtable_t *hashtable);

switch_size_t SWITCH_HASHTABLE_COUNT(switch_hashtable_t *hashtable);

switch_status_t SWITCH_HASHTABLE_INSERT(switch_hashtable_t *hashtable,
                                        switch_hashnode_t *node,
                                        void *key,
                                        void *data);

switch_status_t SWITCH_HASHTABLE_DELETE(switch_hashtable_t *hashtable,
                                        void *key,
                                        void **data);

switch_status_t SWITCH_HASHTABLE_DELETE_NODE(switch_hashtable_t *hashtable,
                                             switch_hashnode_t *node);

switch_status_t SWITCH_HASHTABLE_SEARCH(switch_hashtable_t *hashtable,
                                        void *key,
                                        void **data);

switch_status_t SWITCH_HASHTABLE_FOREACH_ARG(switch_hashtable_t *hashtable,
                                             void *func,
                                             void *arg);

switch_status_t SWITCH_HASHTABLE_DONE(switch_hashtable_t *hashtable);

char *switch_error_to_string(switch_status_t status);

char *switch_handle_type_to_string(switch_handle_type_t handle_type);

switch_status_t switch_ipv4_to_string(switch_ip4_t ip4,
                                      char *buffer,
                                      switch_int32_t buffer_size,
                                      switch_int32_t *length);

switch_status_t switch_ipv6_to_string(switch_ip6_t ip6,
                                      char *buffer,
                                      switch_int32_t buffer_size,
                                      switch_int32_t *length);

#define MAC_BUFFER_SIZE 18
switch_status_t switch_mac_to_string(switch_mac_addr_t *mac,
                                     char *buffer,
                                     switch_int32_t buffer_size,
                                     switch_int32_t *length);

char *switch_table_id_to_string(switch_table_id_t table_id);

switch_direction_t switch_table_id_to_direction(switch_table_id_t table_id);

char *switch_direction_to_string(switch_direction_t direction);

char *switch_api_type_to_string(switch_api_type_t api_type);

char *switch_packet_type_to_string(switch_packet_type_t packet_type);

bool switch_l3_host_entry(const switch_ip_addr_t *ip_addr);

switch_uint32_t MurmurHash(const void *key,
                           switch_uint32_t length,
                           switch_uint32_t seed);

switch_status_t switch_api_hashtable_dump(const switch_device_t device,
                                          const switch_hashtable_type_t type,
                                          void *cli_ctx);

#define SWITCH_ARRAY_FIRST_GET(__array, __index, __type, __entry) \
  Word_t __index_tmp = 0;                                         \
  Word_t *__pvalue = NULL;                                        \
  __entry = NULL;                                                 \
  if (__array.array) {                                            \
    JLF(__pvalue, __array.array, __index_tmp);                    \
    status = SWITCH_STATUS_ITEM_NOT_FOUND;                        \
    if (__pvalue) {                                               \
      __index = __index_tmp;                                      \
      __entry = (__type *)__pvalue;                               \
      status = SWITCH_STATUS_SUCCESS;                             \
    }                                                             \
  } else {                                                        \
    status = SWITCH_STATUS_SUCCESS;                               \
  }

#define SWITCH_ARRAY_NEXT_GET(__array, __o_index, __n_index, __type, __entry) \
  Word_t __index_tmp = (Word_t)__o_index;                                     \
  Word_t *__pvalue = NULL;                                                    \
  __entry = NULL;                                                             \
  JLN(__pvalue, __array.array, __index_tmp);                                  \
  if (__pvalue) {                                                             \
    __entry = (__type *)__pvalue;                                             \
    __n_index = __index_tmp;                                                  \
  } else {                                                                    \
    __n_index = SWITCH_API_INVALID_HANDLE;                                    \
    __entry = NULL;                                                           \
  }

#define FOR_EACH_IN_ARRAY(__index, __array, __type, __entry)            \
  {                                                                     \
    Word_t *__pvalue = NULL;                                            \
    Word_t *__pvalue_next = NULL;                                       \
    Word_t __index_tmp = __index;                                       \
    JLF(__pvalue, __array.array, __index_tmp);                          \
    __index = __index_tmp;                                              \
    for (; __pvalue; __pvalue = __pvalue_next, __index = __index_tmp) { \
      JLN(__pvalue_next, __array.array, __index_tmp);                   \
      __entry = (__type *)(*__pvalue);

#define FOR_EACH_IN_ARRAY_END() \
  }                             \
  }

#define FOR_EACH_IN_LIST(__list, __node)   \
  {                                        \
    node = tommy_list_head(&__list.list);  \
    switch_node_t *__next_node = NULL;     \
    for (; __node; __node = __next_node) { \
      __next_node = node->next;

#define FOR_EACH_IN_LIST_END() \
  }                            \
  }

#define SWITCH_HW_FLAG_ISSET(_info, _pd_entry) _info->hw_flags &_pd_entry

#define SWITCH_HW_FLAG_SET(_info, _pd_entry) _info->hw_flags |= _pd_entry

#define SWITCH_HW_FLAG_CLEAR(_info, _pd_entry) _info->hw_flags &= ~(_pd_entry)

#define SWITCH_HASHTABLE_ITERATOR(_hashtable, _func, _arg) \
  tommy_hashtable_foreach_arg(_hashtable, _func, _arg)

#define SWITCH_MT_LOCK(_device)                           \
  do {                                                    \
    switch_device_context_t *__device_ctx = NULL;         \
    switch_status_t __status =                            \
        switch_device_context_get(device, &__device_ctx); \
    if (__status != SWITCH_STATUS_SUCCESS) {              \
      SWITCH_LOG_ERROR(                                   \
          "unable to acquire a lock for this device %d: " \
          " as device config context get failed(%s)",     \
          device,                                         \
          switch_error_to_string(__status));              \
      return __status;                                    \
    }                                                     \
    bf_sys_rmutex_t *__mtx = &__device_ctx->mtx;          \
    bf_sys_rmutex_lock(__mtx);                            \
  } while (0);

#define SWITCH_MT_UNLOCK(_device)                         \
  do {                                                    \
    switch_device_context_t *__device_ctx = NULL;         \
    switch_status_t __status =                            \
        switch_device_context_get(device, &__device_ctx); \
    if (__status != SWITCH_STATUS_SUCCESS) {              \
      SWITCH_LOG_ERROR(                                   \
          "unable to free a lock for this device %d: "    \
          " as device config context get failed(%s)",     \
          device,                                         \
          switch_error_to_string(__status));              \
      return __status;                                    \
    }                                                     \
    bf_sys_rmutex_t *__mtx = &__device_ctx->mtx;          \
    bf_sys_rmutex_unlock(__mtx);                          \
  } while (0);

#define SWITCH_MT_WRAP(__fn)                                                   \
  switch_device_context_t *__device_ctx = NULL;                                \
  switch_status_t __status = switch_device_context_get(device, &__device_ctx); \
  if (__status != SWITCH_STATUS_SUCCESS) {                                     \
    SWITCH_LOG_ERROR(                                                          \
        "unable to acquire a lock for this device %d: "                        \
        " as device config context get failed(%s)",                            \
        device,                                                                \
        switch_error_to_string(__status));                                     \
    return __status;                                                           \
  }                                                                            \
  bf_sys_rmutex_t *__mtx = &__device_ctx->mtx;                                 \
  bf_sys_rmutex_lock(__mtx);                                                   \
  __status = __fn;                                                             \
  bf_sys_rmutex_unlock(__mtx);                                                 \
  return __status;

#define SWITCH_FAST_RECONFIG(__device)                    \
  switch_device_context_t *__device_ctx = NULL;           \
  switch_status_t __status =                              \
      switch_device_context_get(__device, &__device_ctx); \
  if (__status != SWITCH_STATUS_SUCCESS) {                \
    SWITCH_LOG_ERROR(                                     \
        "unable to acquire a lock for this device %d: "   \
        " as device config context get failed(%s)",       \
        __device,                                         \
        switch_error_to_string(__status));                \
    return __status;                                      \
  }                                                       \
  if (__device_ctx->warm_init) {                          \
    return __status;                                      \
  }
bool switch_api_debug_mode_get();

#define SWITCH_ASSERT(x)                              \
  if (switch_api_debug_mode_get()) {                  \
    assert(x);                                        \
  } else {                                            \
    CHECK_RET(!(x), SWITCH_STATUS_INVALID_PARAMETER); \
  }

#define SWITCH_TIMER_CREATE(_timer, _period_secs, _cb_fn, _data) \
  bf_sys_timer_create(_timer, 0x0, _period_secs * 1000, _cb_fn, _data)

#define SWITCH_TIMER_DELETE(_timer) bf_sys_timer_del(_timer)

#define SWITCH_TIMER_START(_timer) bf_sys_timer_start(_timer)

#define SWITCH_TIMER_STOP(_timer) bf_sys_timer_stop(_timer)

#ifdef __cplusplus
}
#endif

#endif /* _switch_internal_h_ */
