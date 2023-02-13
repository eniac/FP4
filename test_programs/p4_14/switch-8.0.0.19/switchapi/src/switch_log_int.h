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

#ifndef __SWITCH_LOG_INT_H__
#define __SWITCH_LOG_INT_H__
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>

#include <bfsys/bf_sal/bf_sys_intf.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_LOG_BUFFER_SIZE 600

static inline char *switch_log_level_to_string(switch_log_level_t log_level) {
  switch (log_level) {
    case SWITCH_LOG_LEVEL_NONE:
      return "NONE";
    case SWITCH_LOG_LEVEL_DEBUG:
      return "DEBUG";
    case SWITCH_LOG_LEVEL_DETAIL:
      return "DETAIL";
    case SWITCH_LOG_LEVEL_WARN:
      return "WARN";
    case SWITCH_LOG_LEVEL_ERROR:
      return "ERROR";
    case SWITCH_LOG_LEVEL_CRITICAL:
      return "CRITICAL";
    default:
      return "UNKNOWN";
  }
}

static inline char *switch_time_get() {
  struct timeval t;
  struct tm *info = NULL;
  time_t rawtime;
  static char time_str[80];
  char buffer[50];
  gettimeofday(&t, NULL);
  time(&rawtime);
  info = localtime(&rawtime);
  strftime(buffer, 50, "%X", info);
  switch_snprintf(time_str, 80, "%s:%d", buffer, (int)(t.tv_usec / 1000));
  return time_str;
}

#define SWITCH_LOG(_log_level, fmt, arg...)                      \
  do {                                                           \
    switch_logging_context_t *_log_ctx = NULL;                   \
    _log_ctx = switch_config_logging_context_get();              \
    if (_log_ctx && _log_ctx->log_fn) {                          \
      if (_log_level <= _log_ctx->log_level[__MODULE__]) {       \
        _log_ctx->log_fn("[%s] [%s] [%s:%s:%d] \n" fmt,          \
                         switch_time_get(),                      \
                         switch_log_level_to_string(_log_level), \
                         "API",                                  \
                         __FUNCTION__,                           \
                         __LINE__,                               \
                         ##arg);                                 \
      }                                                          \
    }                                                            \
  } while (0);

#if 0
#define SWITCH_PRINT(_cli_ctx, fmt, arg...)         \
  do {                                              \
    switch_logging_context_t *_log_ctx = NULL;      \
    _log_ctx = switch_config_logging_context_get(); \
    if (_log_ctx && _log_ctx->cli_fn) {             \
      _log_ctx->cli_fn(_cli_ctx, fmt, ##arg);       \
    }                                               \
  } while (0);
#endif
#define SWITCH_PRINT bfshell_printf

#ifdef SWITCH_DEF_LOG_ENABLE

#define SWITCH_LOG_ENTER() \
  SWITCH_LOG(SWITCH_LOG_LEVEL_DEBUG, "Entering %s\n", __FUNCTION__)

#define SWITCH_LOG_EXIT() \
  SWITCH_LOG(SWITCH_LOG_LEVEL_DEBUG, "Exiting %s\n", __FUNCTION__)

#define SWITCH_LOG_DEBUG(fmt, arg...) \
  SWITCH_LOG(SWITCH_LOG_LEVEL_DEBUG, fmt, ##arg)

#define SWITCH_LOG_DETAIL(fmt, arg...) \
  SWITCH_LOG(SWITCH_LOG_LEVEL_DETAIL, fmt, ##arg)

#define SWITCH_LOG_WARN(fmt, arg...) \
  SWITCH_LOG(SWITCH_LOG_LEVEL_WARN, fmt, ##arg)

#define SWITCH_LOG_ERROR(fmt, arg...) \
  SWITCH_LOG(SWITCH_LOG_LEVEL_ERROR, fmt, ##arg)

#define SWITCH_LOG_CRITICAL(fmt, arg...) \
  SWITCH_LOG(SWITCH_LOG_LEVEL_CRITICAL, fmt, ##arg)

#define SWITCH_PD_LOG_DEBUG(fmt, arg...) \
  SWITCH_LOG(SWITCH_LOG_LEVEL_DEBUG, fmt, ##arg)

#define SWITCH_PD_LOG_ERROR(fmt, arg...) \
  SWITCH_LOG(SWITCH_LOG_LEVEL_ERROR, fmt, ##arg)

#else
#define SWITCH_LOG_ENTER() \
  bf_sys_log_and_trace(    \
      BF_MOD_SWITCHAPI, BF_LOG_DBG, "Entering %s\n", __FUNCTION__)

#define SWITCH_LOG_EXIT() \
  bf_sys_log_and_trace(   \
      BF_MOD_SWITCHAPI, BF_LOG_DBG, "Exiting %s\n", __FUNCTION__)

#define SWITCH_LOG_DEBUG(fmt, arg...)                                      \
  bf_sys_log_and_trace(BF_MOD_SWITCHAPI,                                   \
                       BF_LOG_DBG,                                         \
                       "%s:%d [%s] " fmt,                                  \
                       __FUNCTION__,                                       \
                       __LINE__,                                           \
                       switch_log_level_to_string(SWITCH_LOG_LEVEL_DEBUG), \
                       ##arg)

#define SWITCH_LOG_DETAIL(fmt, arg...)                                      \
  bf_sys_log_and_trace(BF_MOD_SWITCHAPI,                                    \
                       BF_LOG_INFO,                                         \
                       "%s:%d [%s] " fmt,                                   \
                       __FUNCTION__,                                        \
                       __LINE__,                                            \
                       switch_log_level_to_string(SWITCH_LOG_LEVEL_DETAIL), \
                       ##arg)

#define SWITCH_LOG_WARN(fmt, arg...)                                      \
  bf_sys_log_and_trace(BF_MOD_SWITCHAPI,                                  \
                       BF_LOG_WARN,                                       \
                       "%s:%d [%s] " fmt,                                 \
                       __FUNCTION__,                                      \
                       __LINE__,                                          \
                       switch_log_level_to_string(SWITCH_LOG_LEVEL_WARN), \
                       ##arg)

#define SWITCH_LOG_ERROR(fmt, arg...)                                      \
  bf_sys_log_and_trace(BF_MOD_SWITCHAPI,                                   \
                       BF_LOG_ERR,                                         \
                       "%s:%d [%s] " fmt,                                  \
                       __FUNCTION__,                                       \
                       __LINE__,                                           \
                       switch_log_level_to_string(SWITCH_LOG_LEVEL_ERROR), \
                       ##arg)

#define SWITCH_LOG_CRITICAL(fmt, arg...)                                      \
  bf_sys_log_and_trace(BF_MOD_SWITCHAPI,                                      \
                       BF_LOG_CRIT,                                           \
                       "%s:%d [%s] " fmt,                                     \
                       __FUNCTION__,                                          \
                       __LINE__,                                              \
                       switch_log_level_to_string(SWITCH_LOG_LEVEL_CRITICAL), \
                       ##arg)

#define SWITCH_PD_LOG_DEBUG(fmt, arg...)                                   \
  bf_sys_log_and_trace(BF_MOD_SWITCHAPI,                                   \
                       BF_LOG_DBG,                                         \
                       "%s:%d [%s] " fmt,                                  \
                       __FUNCTION__,                                       \
                       __LINE__,                                           \
                       switch_log_level_to_string(SWITCH_LOG_LEVEL_DEBUG), \
                       ##arg)

#define SWITCH_PD_LOG_ERROR(fmt, arg...)                                   \
  bf_sys_log_and_trace(BF_MOD_SWITCHAPI,                                   \
                       BF_LOG_ERR,                                         \
                       "%s:%d [%s] " fmt,                                  \
                       __FUNCTION__,                                       \
                       __LINE__,                                           \
                       switch_log_level_to_string(SWITCH_LOG_LEVEL_ERROR), \
                       ##arg)
#endif

/**
 * Check if equal, log error and return
 */
#define CHECK_RET(x, ret)                                                      \
  do {                                                                         \
    if (x) {                                                                   \
      SWITCH_LOG_ERROR("ERROR %s at (%s)\n", switch_error_to_string(ret), #x); \
      return ret;                                                              \
    }                                                                          \
  } while (0)

/**
 * Check if true, log error and clean
 */

#define CHECK_CLEAN(x, ret)                                                    \
  do {                                                                         \
    if (x) {                                                                   \
      status = ret;                                                            \
      SWITCH_LOG_ERROR("ERROR %s at (%s)\n", switch_error_to_string(ret), #x); \
      goto clean;                                                              \
    }                                                                          \
  } while (0)

#define CHECK_LOG(x)                           \
  do {                                         \
    if (x) {                                   \
      SWITCH_LOG_ERROR("%s: ERROR %s \n", #x); \
    }                                          \
  } while (0)

/** log structs */
typedef struct switch_logging_context_s {
  /** api log levels */
  switch_log_level_t log_level[SWITCH_API_TYPE_MAX];

  /** api logging function */
  switch_api_log_fn_t *log_fn;

  /** api cli function */
  switch_api_cli_fn_t *cli_fn;

} switch_logging_context_t;

switch_status_t switch_log_init(switch_log_level_t log_level);

switch_status_t switch_log_free(switch_log_level_t log_level);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SWITCH_LOG_INT_H__ */
