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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define TEST_ASSERT(x) assert(x)

inline void SWITCH_START_TEST(char *module_name) {
  printf("\n################ %s_TEST_START ##############\n", module_name);
}

inline void SWITCH_END_TEST(char *module_name) {
  printf("\n################ %s_TEST_END ##############\n", module_name);
}

switch_status_t switch_meter_test(switch_device_t device);

switch_status_t switch_vrf_test(switch_device_t device);

switch_status_t switch_interface_test(switch_device_t device);

switch_status_t switch_rmac_test(switch_device_t device);

switch_status_t switch_vlan_test(switch_device_t device);

switch_status_t switch_l2_test(switch_device_t device);

switch_status_t switch_l3_test(switch_device_t device);

switch_status_t switch_rpf_test(switch_device_t device);

#ifdef __cplusplus
}
#endif
