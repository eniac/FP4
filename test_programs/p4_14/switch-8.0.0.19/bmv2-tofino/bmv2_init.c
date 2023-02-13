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

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dlfcn.h>

#include <getopt.h>
#include <assert.h>

#ifdef SWITCHAPI_ENABLE
#include <tofinobmpd/switch/pd/pd.h>
#include <tofinobmpd/switch/thrift-src/pd_rpc_server.h>
#endif
#include <tofinobm/pdfixed/pd_static.h>
#include <tofinobm/pdfixed/thrift-src/pdfixed_rpc_server.h>

char *pd_server_str = NULL;

/**
 * The maximum number of ports to support:
 * @fixme should be runtime parameter
 */
#define PORT_COUNT 256
#define PD_SERVER_DEFAULT_PORT 9090

/**
 * Check an operation and return if there's an error.
 */
#define CHECK(op)                                                             \
  do {                                                                        \
    int _rv;                                                                  \
    if ((_rv = (op)) < 0) {                                                   \
      fprintf(stderr, "%s: ERROR %d at %s:%d", #op, _rv, __FILE__, __LINE__); \
      return _rv;                                                             \
    }                                                                         \
  } while (0)

#define SWITCH_SAI_THRIFT_RPC_SERVER_PORT "9092"
extern int switch_api_init(int device,
                           unsigned int num_ports,
                           char *cpu_port,
                           bool port_add);
extern int start_switch_api_rpc_server(void);
extern int start_switch_api_packet_driver(void);
extern int start_p4_sai_thrift_rpc_server(char *port);
extern int bf_sys_log_init(void *arg1, void *arg2, void *arg3);
extern void switch_sai_init();

#ifdef SWITCHLINK_ENABLE
extern int switchlink_init(void);
#endif /* SWITCHLINK_ENABLE */

int bmv2_model_init(char *p4_name,
                    char *p4_prefix,
                    bool with_switchsai,
                    bool with_switchlink) {
  int rv = 0;
  char *error;
  void *pd_server_cookie;
  static void *pd_lib_hdl = NULL;
  static void *pd_thrift_lib_hdl = NULL;
  char pd_init_fn_name[80];
  char pd_assign_device_fn_name[80];
  char pd_thrift_add_to_rpc_fn_name[80];
  int (*pd_init_fn)(void);
  int (*pd_assign_device_fn)(
      int dev_id, const char *notif_addr, int rpc_port_num);
  int (*add_to_rpc_fn)(void *);
  typedef void *pvoid_dl_t __attribute__((__may_alias__));

  printf("%s: Loading libpd.so for %s \n", __func__, p4_name);
  pd_lib_hdl = dlopen("libpd.so", RTLD_LAZY | RTLD_GLOBAL);
  if ((error = dlerror()) != NULL) {
    printf("%s: %d: Error in dlopen, err=%s ", __func__, __LINE__, error);
    return -1;
  }

  printf("%s: Loading libpdthrift.so for %s \n", __func__, p4_name);
  pd_thrift_lib_hdl = dlopen("libpdthrift.so", RTLD_LAZY | RTLD_GLOBAL);
  if ((error = dlerror()) != NULL) {
    printf("%s: %d: Error in dlopen, err=%s ", __func__, __LINE__, error);
    return -1;
  }

  /* Retreive pd initialization functions */
  sprintf(pd_init_fn_name, "p4_pd_%s_init", p4_prefix);
  *(pvoid_dl_t *)(&pd_init_fn) = dlsym(pd_lib_hdl, pd_init_fn_name);
  if ((error = dlerror()) != NULL) {
    printf("%s: %d: Error in looking up pd_init func, err=%s ",
           __func__,
           __LINE__,
           error);
    return -1;
  }

  sprintf(pd_assign_device_fn_name, "p4_pd_%s_assign_device", p4_prefix);
  *(pvoid_dl_t *)(&pd_assign_device_fn) =
      dlsym(pd_lib_hdl, pd_assign_device_fn_name);
  if ((error = dlerror()) != NULL) {
    printf("%s: %d: Error in looking up pd_assign_device func, err=%s ",
           __func__,
           __LINE__,
           error);
    return -1;
  }

  /* Retreive pdthrift initialization function */
  sprintf(pd_thrift_add_to_rpc_fn_name, "add_to_rpc_server");
  *(pvoid_dl_t *)(&add_to_rpc_fn) =
      dlsym(pd_thrift_lib_hdl, pd_thrift_add_to_rpc_fn_name);
  if ((error = dlerror()) != NULL) {
    printf("%s: %d: Error in looking up add_to_rpc func, err=%s ",
           __func__,
           __LINE__,
           error);
    return -1;
  }

  /* Start the thrift RPC server */
  start_bfn_pd_rpc_server(&pd_server_cookie);
  /* Add PD thrift service to the RPC server */
  add_to_rpc_fn(pd_server_cookie);

  /* Initialize the logging service */
  if (bf_sys_log_init(NULL, (void *)5, (void *)(32 * 1024)) != 0) {
    printf("%s:%d ERROR: failed to initialize logging service!\n",
           __func__,
           __LINE__);
    return -1;
  }

  /* Initialize the PD fixed library */
  p4_pd_init();
  /* Initialize the PD library */
  pd_init_fn();
  /* Instantiate the bmv2 device */
  pd_assign_device_fn(0, "ipc:///tmp/bmv2-0-notifications.ipc", 10001);

  /* Start up the API RPC server */
  CHECK(switch_api_init(0, 256, "veth251", true));
  CHECK(start_switch_api_rpc_server());

  if (with_switchsai || with_switchlink) {
    CHECK(start_p4_sai_thrift_rpc_server(SWITCH_SAI_THRIFT_RPC_SERVER_PORT));
    switch_sai_init();
  }

#ifdef SWITCHLINK_ENABLE
  if (with_switchlink) {
    CHECK(switchlink_init());
  }
#endif /* SWITCHLINK_ENABLE */

  CHECK(start_switch_api_packet_driver());
  return rv;
}
