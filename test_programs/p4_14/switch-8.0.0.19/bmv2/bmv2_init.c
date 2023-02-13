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

#include <getopt.h>
#include <assert.h>

#include <bmpd/switch/pd/pd.h>
#include <bm/pdfixed/pd_static.h>
#include <bm/pdfixed/thrift-src/pdfixed_rpc_server.h>
#include <bmpd/switch/thrift-src/pd_rpc_server.h>

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
extern void switch_sai_init();

#ifdef SWITCHLINK_ENABLE
extern int switchlink_init(void);
#endif /* SWITCHLINK_ENABLE */

int bmv2_model_init(bool with_switchsai, bool with_switchlink) {
  int rv = 0;
  /* Start up the PD RPC server */
  void *pd_server_cookie;
  start_bfn_pd_rpc_server(&pd_server_cookie);
  add_to_rpc_server(pd_server_cookie);

  p4_pd_init();
  p4_pd_dc_init();
  p4_pd_dc_assign_device(0, "ipc:///tmp/bmv2-0-notifications.ipc", 10001);

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
