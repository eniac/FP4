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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/queue.h>
#include <sys/types.h>
#include <stdio.h>

#include <getopt.h>
#include <assert.h>

extern char *pd_server_str;
static char *p4_name = NULL;
static char *p4_prefix = NULL;
static bool with_switchsai = false;
static bool with_switchlink = false;
extern int bmv2_model_init(char *p4_name,
                           char *p4_prefix,
                           bool with_switchsai,
                           bool with_switchlink);

static void parse_options(int argc, char **argv) {
  struct entry *np = NULL;

  while (1) {
    int option_index = 0;
    /* Options without short equivalents */
    enum long_opts {
      OPT_START = 256,
      OPT_PDSERVER,
      OPT_P4NAME,
      OPT_P4PREFIX,
      OPT_WITH_SWITCHSAI,
      OPT_WITH_SWITCHLINK,
    };
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"pd-server", required_argument, 0, OPT_PDSERVER},
        {"p4-name", required_argument, 0, OPT_P4NAME},
        {"p4-prefix", required_argument, 0, OPT_P4PREFIX},
        {"with-switchsai", no_argument, 0, OPT_WITH_SWITCHSAI},
        {"with-switchlink", no_argument, 0, OPT_WITH_SWITCHLINK},
        {0, 0, 0, 0}};
    int c = getopt_long(argc, argv, "h", long_options, &option_index);
    if (c == -1) {
      break;
    }
    switch (c) {
      case OPT_PDSERVER:
        pd_server_str = strdup(optarg);
        break;
      case OPT_P4NAME:
        p4_name = strdup(optarg);
        break;
      case OPT_P4PREFIX:
        p4_prefix = strdup(optarg);
        break;
      case OPT_WITH_SWITCHSAI:
        with_switchsai = true;
        break;
      case OPT_WITH_SWITCHLINK:
        with_switchsai = true;
        with_switchlink = true;
        break;
      case 'h':
      case '?':
        printf("Drivers! \n");
        printf("Usage: drivers [OPTION]...\n");
        printf("\n");
        printf(" --p4-name=<P4NAME> P4 program to load drivers for\n");
        printf(" --pd-server=IP:PORT Listen for PD RPC calls\n");
        printf(" --with-switchsai\n");
        printf(" --with-switchlink\n");
        printf(" -h,--help Display this help message and exit\n");
        exit(c == 'h' ? 0 : 1);
        break;
    }
  }
}

int main(int argc, char *argv[]) {
  int rv = 0;

  parse_options(argc, argv);

  if (!p4_name) {
    printf("Error: P4 program not specified using --p4-name=<P4NAME>\n");
    return -1;
  }
  if (!p4_prefix) {
    printf("Error: P4 prefix not specified using --p4-prefix=<P4PREFIX>\n");
    return -1;
  }
  printf(
      "Starting drivers for P4-program %s, P4-Prefix %s\n", p4_name, p4_prefix);
  bmv2_model_init(p4_name, p4_prefix, with_switchsai, with_switchlink);

  while (1) pause();

  return rv;
}
