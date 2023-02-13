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

#ifndef __SWITCH_CLI_INT_H__
#define __SWITCH_CLI_INT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_CLI_HASHTABLE_PRINT(_cli_ctx, _hashtable, _name)                \
  do {                                                                         \
    SWITCH_PRINT(_cli_ctx, "\t\tHashtable: %s\n", _name);                      \
    SWITCH_PRINT(_cli_ctx, "\t\t\tkey func: 0x%x\n", _hashtable.key_func);     \
    SWITCH_PRINT(                                                              \
        _cli_ctx, "\t\t\tcompare func: 0x%x\n", _hashtable.compare_func);      \
    SWITCH_PRINT(_cli_ctx, "\t\t\tnum entries: %d\n", _hashtable.num_entries); \
    SWITCH_PRINT(_cli_ctx, "\t\t\tsize: %d\n", _hashtable.size);               \
    SWITCH_PRINT(_cli_ctx, "\t\t\thash seed: %d\n", _hashtable.hash_seed);     \
    SWITCH_PRINT(_cli_ctx, "\n");                                              \
  } while (0);

#define SWITCH_CLI_START_ENTRY_STR_PRINT(_cli_ctx) \
  SWITCH_PRINT(_cli_ctx, "\n\t\t====================\n");

#define SWITCH_CLI_END_ENTRY_STR_PRINT(_cli_ctx) \
  SWITCH_PRINT(_cli_ctx, "\t\t====================\n");

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SWITCH_CLI_INT_H__ */
