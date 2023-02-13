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

#ifndef __SWITCH_LOG_H__
#define __SWITCH_LOG_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** log level */
typedef enum switch_log_level_s {
  SWITCH_LOG_LEVEL_NONE = 0,
  SWITCH_LOG_LEVEL_CRITICAL = 1,
  SWITCH_LOG_LEVEL_ERROR = 2,
  SWITCH_LOG_LEVEL_WARN = 3,
  SWITCH_LOG_LEVEL_DETAIL = 4,
  SWITCH_LOG_LEVEL_DEBUG = 5,
  SWITCH_LOG_LEVEL_MAX
} switch_log_level_t;

typedef switch_int32_t(switch_api_log_fn_t)(char *fmt, ...);

typedef switch_int32_t(switch_api_cli_fn_t)(const void *cli_ctx,
                                            char *fmt,
                                            ...);

switch_status_t switch_api_log_function_set(switch_api_log_fn_t *log_fn);

switch_status_t switch_api_log_cli_function_set(switch_api_cli_fn_t *cli_fn);

switch_status_t switch_api_log_level_set(switch_api_type_t api_id,
                                         switch_log_level_t level);

switch_status_t switch_api_log_level_all_set(switch_log_level_t log_level);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SWITCH_LOG_H__ */
