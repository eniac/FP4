/*
Copyright 2013-present Barefoot Networks, Inc.
*/

#ifndef _switch_bd_h_
#define _switch_bd_h_

#include "switch_base_types.h"
#include "switch_handle.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum switch_bd_flood_type_ {
  SWITCH_BD_FLOOD_NONE = 0x0,
  SWITCH_BD_FLOOD_UUC = 1 << 0,
  SWITCH_BD_FLOOD_UMC = 1 << 1,
  SWITCH_BD_FLOOD_BCAST = 1 << 2,
} switch_bd_flood_type_t;

typedef enum switch_bd_counter_id_ {
  SWITCH_BD_STATS_IN_UCAST = 0,
  SWITCH_BD_STATS_IN_MCAST = 1,
  SWITCH_BD_STATS_IN_BCAST = 2,
  SWITCH_BD_STATS_IN_DROP = 3,
  SWITCH_BD_STATS_OUT_UCAST = 4,
  SWITCH_BD_STATS_OUT_MCAST = 5,
  SWITCH_BD_STATS_OUT_BCAST = 6,
  SWITCH_BD_STATS_OUT_DROP = 7,
  SWITCH_BD_STATS_MAX = 8
} switch_bd_counter_id_t;

/** @} */  // end of bd

#ifdef __cplusplus
}
#endif

#endif /* _switch_bd_h_ */
