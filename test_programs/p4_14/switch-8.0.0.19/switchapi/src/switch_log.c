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

#include "switchapi/switch.h"

/* Local header includes */
#include "switch_internal.h"
#include <execinfo.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_LOG_BACKTRACE_SIZE 10
void print_trace(void) {
  void *array[SWITCH_LOG_BACKTRACE_SIZE];
  int size;
  char **strings;
  int i;

  size = backtrace(array, SWITCH_LOG_BACKTRACE_SIZE);
  strings = backtrace_symbols(array, size);

  for (i = 0; i < size; i++) printf("\t%s\n", strings[i]);

  free(strings);
}

switch_int32_t switch_default_logger(char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  print_trace();
  va_end(args);
  return 0;
}

switch_int32_t switch_default_print(const void *cli_ctx, char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
  return 0;
}

switch_status_t switch_log_init(switch_log_level_t log_level) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  status = switch_api_log_level_all_set(log_level);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_log_function_set(switch_default_logger);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  status = switch_api_log_cli_function_set(switch_default_print);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  return status;
}

switch_status_t switch_log_free(switch_log_level_t log_level) {
  switch_api_type_t api_type = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  for (api_type = SWITCH_API_TYPE_PORT; api_type < SWITCH_API_TYPE_MAX;
       api_type++) {
    status = switch_api_log_level_set(api_type, log_level);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  }

  status = switch_api_log_function_set(NULL);
  SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);

  return status;
}

switch_status_t switch_api_log_function_set(switch_api_log_fn_t *log_fn) {
  switch_logging_context_t *log_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  if (!SWITCH_CONFIG_INITALIZED()) {
    status = SWITCH_STATUS_UNINITIALIZED;
    SWITCH_LOG_ERROR(
        "log function set failed: "
        "switch config uninitialized(%s)",
        switch_error_to_string(status));
    return status;
  }

  log_ctx = switch_config_logging_context_get();

  log_ctx->log_fn = log_fn;

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_log_cli_function_set(switch_api_cli_fn_t *cli_fn) {
  switch_logging_context_t *log_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  if (!SWITCH_CONFIG_INITALIZED()) {
    status = SWITCH_STATUS_UNINITIALIZED;
    SWITCH_LOG_ERROR(
        "log cli function set failed: "
        "switch config uninitialized(%s)",
        switch_error_to_string(status));
    return status;
  }

  log_ctx = switch_config_logging_context_get();

  log_ctx->cli_fn = cli_fn;

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_log_level_set(switch_api_type_t api_type,
                                         switch_log_level_t log_level) {
  switch_logging_context_t *log_ctx = NULL;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  SWITCH_LOG_ENTER();

  SWITCH_ASSERT(api_type < SWITCH_API_TYPE_MAX);
  if (api_type >= SWITCH_API_TYPE_MAX) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "log level set failed for api type %d log level %d: "
        "invalid api type(%s)",
        api_type,
        log_level,
        switch_error_to_string(status));
    return status;
  }

  SWITCH_ASSERT(log_level < SWITCH_LOG_LEVEL_MAX);
  if (log_level >= SWITCH_LOG_LEVEL_MAX) {
    status = SWITCH_STATUS_INVALID_PARAMETER;
    SWITCH_LOG_ERROR(
        "log level set failed for api type %d log level %d: "
        "invalid log level(%s)",
        api_type,
        log_level,
        switch_error_to_string(status));
    return status;
  }

  if (!SWITCH_CONFIG_INITALIZED()) {
    status = SWITCH_STATUS_UNINITIALIZED;
    SWITCH_LOG_ERROR(
        "log level set failed for api type %d log level %d: "
        "switch config uninitialized(%s)",
        api_type,
        log_level,
        switch_error_to_string(status));
    return status;
  }

  log_ctx = switch_config_logging_context_get();

  log_ctx->log_level[api_type] = log_level;

  SWITCH_LOG_DEBUG("api type %s log level set to %s",
                   switch_api_type_to_string(api_type),
                   switch_log_level_to_string(log_level));

  SWITCH_LOG_EXIT();

  return status;
}

switch_status_t switch_api_log_level_all_set(switch_log_level_t log_level) {
  switch_api_type_t api_type = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  for (api_type = SWITCH_API_TYPE_PORT; api_type < SWITCH_API_TYPE_MAX;
       api_type++) {
    status = switch_api_log_level_set(api_type, log_level);
    SWITCH_ASSERT(status == SWITCH_STATUS_SUCCESS);
  }

  bf_sys_trace_level_set(BF_MOD_SWITCHAPI, log_level);
  bf_sys_log_level_set(BF_MOD_SWITCHAPI, BF_LOG_DEST_FILE, log_level);

  SWITCH_LOG_DEBUG("log level set to %s",
                   switch_log_level_to_string(log_level));

  return status;
}

#ifdef __cplusplus
}
#endif
