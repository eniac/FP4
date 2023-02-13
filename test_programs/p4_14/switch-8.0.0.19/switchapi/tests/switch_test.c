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
/**************************************************************************/ /**
  *
  *
  *
  *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <switchapi/switch.h>

#include "switch_test.h"

int main(int argc, char *argv[]) {
  switch_device_t device = 0;

  switch_api_init(device, 32, NULL, TRUE);

  switch_api_device_dump(device, NULL);

  switch_meter_test(device);

  switch_vrf_test(device);

  switch_interface_test(device);

  switch_rmac_test(device);

  switch_vlan_test(device);

  switch_l2_test(device);

  switch_l3_test(device);

  switch_rpf_test(device);

  return 0;
}
