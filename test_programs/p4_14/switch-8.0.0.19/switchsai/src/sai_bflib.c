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

#include <stdio.h>
#include <syslog.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <linux/if_arp.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <pthread.h>

#include "saiinternal.h"
#include "switchapi/switch.h"
#include "switchapi/switch_device.h"
#include "switchapi/switch_interface.h"
#include "switchapi/switch_hostif.h"

// globals
static sai_api_t api_id = SAI_API_UNSPECIFIED;

#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
#include "bf_switchd/bf_switchd.h"
#include "tofino/bf_pal/dev_intf.h"
extern bf_status_t bf_pal_device_warm_init_end(bf_dev_id_t dev_id);

#define INSTALL_DIR ((const char *)"/bfn/install")
#define CONF_FILE ((const char *)"/usr/share/sonic/hwsku/switch-sai.conf")

#define CONF_FILE_OLD \
  ((const char *)"/bfn/install/share/p4/targets/tofino/switch-sai.conf")

#define SWITCH_NOS_APP_ID 1

// NOS Loader specific context - holds all the necessary settings for Loader
typedef struct bf_switch_nos_context_s {
  bf_switchd_context_t switchd_ctx;
  unsigned int maxSysPorts;
  pthread_t init_threadId;  // warm init delay thread, when active
} bf_switch_nos_context_t;

static bf_switch_nos_context_t *switch_nos_ctx = NULL;

static void *warmInitDone(void *p) {
  sleep(8);
  bf_pal_device_warm_init_end(0);
  switch_nos_ctx->init_threadId = 0;
  return p;
}

static int bfn_sdk_init(char *baseDir, bool warmBoot) {
  int ret = 0;
  switch_handle_t vlan_handle = 0;
  switch_status_t sts;
  switch_api_device_info_t api_device_info;
  unsigned int flags = 0;
  uint16_t recirc_ports = 0;
  static char dir[4096];
  static char conf[4096];
  struct stat stat_buf;
  pthread_attr_t attr;

  /* Allocate memory to hold switchd configuration and state */
  if ((switch_nos_ctx = (bf_switch_nos_context_t *)malloc(
           sizeof(bf_switch_nos_context_t))) == NULL) {
    printf("ERROR: Failed to allocate memory for switch nos context\n");
    return -1;
  }
  memset(switch_nos_ctx, 0, sizeof(bf_switch_nos_context_t));
  // set required directories
  strncpy(dir, baseDir, 4095);
  strncat(dir, INSTALL_DIR, 4095);
  switch_nos_ctx->switchd_ctx.install_dir = dir;
  strncpy(conf, CONF_FILE, 4095);
  if (stat(conf, &stat_buf)) {
    strncpy(conf, baseDir, 4095);
    strncat(conf, CONF_FILE_OLD, 4095);
  }
  switch_nos_ctx->switchd_ctx.conf_file = conf;
  switch_nos_ctx->switchd_ctx.running_in_background = true;
  syslog(LOG_ERR, "BF_SAI: switchd_lib_init with warmBoot: %d\n", warmBoot);
  if (warmBoot != false) {
    // set the fastreconfig flags
    switch_nos_ctx->switchd_ctx.init_mode = BF_DEV_WARM_INIT_FAST_RECFG;
  }
  ret = bf_switchd_lib_init(&(switch_nos_ctx->switchd_ctx));
  if (ret != 0) {
    syslog(LOG_ERR, "ERROR: switchd_lib_init Failed: %d\n", ret);
    return ret;
  }
  sts = switch_api_vlan_create(0, 0x1, &vlan_handle);
  if (sts != 0) syslog(LOG_ERR, "ERROR: VLAN creation failed: %d\n", sts);

  memset(&api_device_info, 0x0, sizeof(api_device_info));
  flags = SWITCH_DEVICE_ATTR_MAX_PORTS;
  sts = switch_api_device_attribute_get(0, flags, &api_device_info);
  switch_api_device_max_recirc_ports_get(device, &recirc_ports);
  for (unsigned int i = 0; i < api_device_info.max_ports - recirc_ports;
       i += 4) {
    switch_handle_t port_handle = 0;
    switch_api_port_info_t api_port_info;
    memset(&api_port_info, 0, sizeof(api_port_info));
    api_port_info.port = i;
    api_port_info.port_speed = SWITCH_PORT_SPEED_100G;
    api_port_info.fec_mode = SWITCH_PORT_FEC_MODE_RS;
    api_port_info.initial_admin_state = 0;
    switch_status_t sts = switch_api_port_add(0, &api_port_info, &port_handle);
    (void)sts;
  }

  // use delay secs to inform  drivers that warm boot is done - not notification
  // from SAI!
  if (warmBoot == true) {
    pthread_attr_init(&attr);
    if ((ret = pthread_create(
             &(switch_nos_ctx->init_threadId), &attr, warmInitDone, NULL)) !=
        0) {
      syslog(LOG_ERR, "ERROR: thread creation failed service: %d\n", ret);
    }
  }
  return 0;
}
#endif

void switch_sai_init() {
  sai_initialize();
  return;
}

static unsigned int initialized = 0;
const char *sai_profile_get_value(_In_ sai_switch_profile_id_t profile_id,
                                  _In_ const char *variable) {
  return NULL;
}

/*
 * Enumerate all the K/V pairs in a profile.
 * Pointer to NULL passed as variable restarts enumeration.
 * Function returns 0 if next value exists, -1 at the end of the list.
 */
int sai_profile_get_next_value(_In_ sai_switch_profile_id_t profile_id,
                               _Out_ const char **variable,
                               _Out_ const char **value) {
  return -1;
}

const service_method_table_t sai_services = {
    .profile_get_value = sai_profile_get_value,
    .profile_get_next_value = sai_profile_get_next_value};

extern int bmv2_model_init();

sai_status_t sai_api_initialize(uint64_t flags,
                                const service_method_table_t *services) {
  sai_status_t status = SAI_STATUS_SUCCESS;
  UNUSED(api_id);
  bool warmbootMode = false;

  if (!initialized) {
    SAI_LOG_WARN("Initializing device");
    if (services) {
      if (services->profile_get_value) {
        const char *bootStr = services->profile_get_value(0, SAI_KEY_BOOT_TYPE);
        if (bootStr) {
          if (atoi(bootStr) == 2)  // fast boot only
            warmbootMode = true;
          syslog(
              LOG_ERR, "BF_SAI: syncd get profilewith warmBoot: %s\n", bootStr);
        } else
          syslog(LOG_ERR, "BF_SAI: syncd get profile FAILED\n");
      } else
        syslog(LOG_ERR, "BF_SAI: syncd service NULL\n");
    }
#if defined(__TARGET_TOFINO__) && !defined(BMV2TOFINO)
    bfn_sdk_init("/opt", warmbootMode);
#else
    bmv2_model_init();
#endif
    initialized = 1;
    sai_initialize();
  }
  return status;
}

sai_status_t sai_api_uninitialize() { return SAI_STATUS_SUCCESS; }
