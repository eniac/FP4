
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

#include "switch_internal.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

switch_status_t switch_pd_ip_fib_entry_add(switch_device_t device,
                                           switch_vrf_id_t vrf,
                                           switch_ip_addr_t *ip_addr,
                                           bool ecmp,
                                           switch_nhop_t nexthop,
                                           switch_route_type_t type,
                                           switch_pd_hdl_t *entry_hdl,
                                           uint32_t flags) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(vrf);
  UNUSED(ip_addr);
  UNUSED(ecmp);
  UNUSED(nexthop);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L3_DISABLE

  p4_pd_dev_target_t p4_pd_device;
  bool host_entry = TRUE;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (ip_addr->type == SWITCH_API_IP_ADDR_V4) {
#ifndef P4_IPV4_DISABLE
    host_entry =
        ip_addr->prefix_len == SWITCH_IPV4_PREFIX_LENGTH_IN_BITS ? TRUE : FALSE;
    if (flags) {
      if (flags & SWITCH_IP_FORCE_HOST_IN_LPM)
        host_entry = FALSE;
      else if (flags & SWITCH_IP_FORCE_HOST_IN_EXACT)
        host_entry = TRUE;
    }
    if (ecmp) {
      if (host_entry) {
        p4_pd_dc_ipv4_fib_match_spec_t v4_match_spec;
        p4_pd_dc_fib_hit_ecmp_action_spec_t v4_action_spec;
        SWITCH_MEMSET(
            &v4_match_spec, 0x0, sizeof(p4_pd_dc_ipv4_fib_match_spec_t));
        SWITCH_MEMSET(
            &v4_action_spec, 0x0, sizeof(p4_pd_dc_fib_hit_ecmp_action_spec_t));

        v4_match_spec.l3_metadata_vrf = vrf;
        v4_match_spec.ipv4_metadata_lkp_ipv4_da = ip_addr->ip.v4addr;
        v4_action_spec.action_ecmp_index = nexthop;
#ifdef IPV4_LOCAL_HOST_TABLE_SIZE
        if (flags & SWITCH_IP_FORCE_HOST_IN_LOCAL_HOST)
          pd_status = p4_pd_dc_ipv4_fib_local_hosts_table_add_with_fib_hit_ecmp(
              switch_cfg_sess_hdl,
              p4_pd_device,
              (p4_pd_dc_ipv4_fib_local_hosts_match_spec_t *)&v4_match_spec,
              &v4_action_spec,
              entry_hdl);
        else
#endif
          pd_status =
              p4_pd_dc_ipv4_fib_table_add_with_fib_hit_ecmp(switch_cfg_sess_hdl,
                                                            p4_pd_device,
                                                            &v4_match_spec,
                                                            &v4_action_spec,
                                                            entry_hdl);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "fib entry add failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }
        if (switch_pd_log_level_debug()) {
          switch_pd_dump_entry_t pd_entry;
          SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
          pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
          pd_entry.match_spec = (switch_uint8_t *)&v4_match_spec;
          pd_entry.match_spec_size = sizeof(v4_match_spec);
          pd_entry.action_spec = (switch_uint8_t *)&v4_action_spec;
          pd_entry.action_spec_size = sizeof(v4_action_spec);
          pd_entry.pd_hdl = *entry_hdl;
          switch_pd_entry_dump(device, &pd_entry);
        }
      } else {
        p4_pd_dc_ipv4_fib_lpm_match_spec_t v4_match_spec;
        p4_pd_dc_fib_hit_ecmp_action_spec_t v4_action_spec;

        SWITCH_MEMSET(
            &v4_match_spec, 0x0, sizeof(p4_pd_dc_ipv4_fib_lpm_match_spec_t));
        SWITCH_MEMSET(
            &v4_action_spec, 0x0, sizeof(p4_pd_dc_fib_hit_ecmp_action_spec_t));
        v4_match_spec.l3_metadata_vrf = vrf;
        v4_match_spec.ipv4_metadata_lkp_ipv4_da = ip_addr->ip.v4addr;
        v4_match_spec.ipv4_metadata_lkp_ipv4_da_prefix_length =
            ip_addr->prefix_len;
        v4_action_spec.action_ecmp_index = nexthop;

        pd_status = p4_pd_dc_ipv4_fib_lpm_table_add_with_fib_hit_ecmp(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &v4_match_spec,
            &v4_action_spec,
            entry_hdl);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "fib entry add failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }

        if (switch_pd_log_level_debug()) {
          switch_pd_dump_entry_t pd_entry;
          SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
          pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
          pd_entry.match_spec = (switch_uint8_t *)&v4_match_spec;
          pd_entry.match_spec_size = sizeof(v4_match_spec);
          pd_entry.action_spec = (switch_uint8_t *)&v4_action_spec;
          pd_entry.action_spec_size = sizeof(v4_action_spec);
          pd_entry.pd_hdl = *entry_hdl;
          switch_pd_entry_dump(device, &pd_entry);
        }
      }
    } else {
      if (host_entry) {
        p4_pd_dc_ipv4_fib_match_spec_t v4_match_spec;
        SWITCH_MEMSET(
            &v4_match_spec, 0x0, sizeof(p4_pd_dc_ipv4_fib_match_spec_t));
        v4_match_spec.l3_metadata_vrf = vrf;
        v4_match_spec.ipv4_metadata_lkp_ipv4_da = ip_addr->ip.v4addr;

        if (type == SWITCH_ROUTE_TYPE_MYIP) {
          p4_pd_dc_fib_hit_myip_action_spec_t v4_action_spec;

          SWITCH_MEMSET(&v4_action_spec,
                        0x0,
                        sizeof(p4_pd_dc_fib_hit_myip_action_spec_t));

          v4_action_spec.action_nexthop_index = nexthop;

#ifdef IPV4_LOCAL_HOST_TABLE_SIZE
          if (flags & SWITCH_IP_FORCE_HOST_IN_LOCAL_HOST)
            pd_status =
                p4_pd_dc_ipv4_fib_local_hosts_table_add_with_fib_hit_myip(
                    switch_cfg_sess_hdl,
                    p4_pd_device,
                    (p4_pd_dc_ipv4_fib_local_hosts_match_spec_t *)&v4_match_spec,
                    &v4_action_spec,
                    entry_hdl);
          else
#endif
            pd_status = p4_pd_dc_ipv4_fib_table_add_with_fib_hit_myip(
                switch_cfg_sess_hdl,
                p4_pd_device,
                &v4_match_spec,
                &v4_action_spec,
                entry_hdl);
        } else {
          p4_pd_dc_fib_hit_nexthop_action_spec_t v4_action_spec;

          SWITCH_MEMSET(&v4_action_spec,
                        0x0,
                        sizeof(p4_pd_dc_fib_hit_nexthop_action_spec_t));

          v4_action_spec.action_nexthop_index = nexthop;

#ifdef IPV4_LOCAL_HOST_TABLE_SIZE
          if (flags & SWITCH_IP_FORCE_HOST_IN_LOCAL_HOST)
            pd_status =
                p4_pd_dc_ipv4_fib_local_hosts_table_add_with_fib_hit_nexthop(
                    switch_cfg_sess_hdl,
                    p4_pd_device,
                    (p4_pd_dc_ipv4_fib_local_hosts_match_spec_t *)&v4_match_spec,
                    &v4_action_spec,
                    entry_hdl);
          else
#endif
            pd_status = p4_pd_dc_ipv4_fib_table_add_with_fib_hit_nexthop(
                switch_cfg_sess_hdl,
                p4_pd_device,
                &v4_match_spec,
                &v4_action_spec,
                entry_hdl);

          if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
            SWITCH_PD_LOG_ERROR(
                "fib entry add failed "
                "on device %d : table %s action %s\n",
                device,
                switch_pd_table_id_to_string(0),
                switch_pd_action_id_to_string(0));
            goto cleanup;
          }

          if (switch_pd_log_level_debug()) {
            switch_pd_dump_entry_t pd_entry;
            SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
            pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
            pd_entry.match_spec = (switch_uint8_t *)&v4_match_spec;
            pd_entry.match_spec_size = sizeof(v4_match_spec);
            pd_entry.action_spec = (switch_uint8_t *)&v4_action_spec;
            pd_entry.action_spec_size = sizeof(v4_action_spec);
            pd_entry.pd_hdl = *entry_hdl;
            switch_pd_entry_dump(device, &pd_entry);
          }
        }
      } else {
        p4_pd_dc_ipv4_fib_lpm_match_spec_t v4_match_spec;
        p4_pd_dc_fib_hit_nexthop_action_spec_t v4_action_spec;

        SWITCH_MEMSET(
            &v4_match_spec, 0x0, sizeof(p4_pd_dc_ipv4_fib_lpm_match_spec_t));
        SWITCH_MEMSET(&v4_action_spec,
                      0x0,
                      sizeof(p4_pd_dc_fib_hit_nexthop_action_spec_t));

        v4_match_spec.l3_metadata_vrf = vrf;
        v4_match_spec.ipv4_metadata_lkp_ipv4_da = ip_addr->ip.v4addr;
        v4_match_spec.ipv4_metadata_lkp_ipv4_da_prefix_length =
            ip_addr->prefix_len;
        v4_action_spec.action_nexthop_index = nexthop;

        pd_status = p4_pd_dc_ipv4_fib_lpm_table_add_with_fib_hit_nexthop(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &v4_match_spec,
            &v4_action_spec,
            entry_hdl);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "fib entry add failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }

        if (switch_pd_log_level_debug()) {
          switch_pd_dump_entry_t pd_entry;
          SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
          pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
          pd_entry.match_spec = (switch_uint8_t *)&v4_match_spec;
          pd_entry.match_spec_size = sizeof(v4_match_spec);
          pd_entry.action_spec = (switch_uint8_t *)&v4_action_spec;
          pd_entry.action_spec_size = sizeof(v4_action_spec);
          pd_entry.pd_hdl = *entry_hdl;
          switch_pd_entry_dump(device, &pd_entry);
        }
      }
    }
#endif /* P4_IPV4_DISABLE */
  } else {
#ifndef P4_IPV6_DISABLE
    host_entry =
        ip_addr->prefix_len == SWITCH_IPV6_PREFIX_LENGTH_IN_BITS ? TRUE : FALSE;
    if (flags) {
      if (flags & SWITCH_IP_FORCE_HOST_IN_LPM)
        host_entry = FALSE;
      else if (flags & SWITCH_IP_FORCE_HOST_IN_EXACT)
        host_entry = TRUE;
    }
    if (ecmp) {
      if (host_entry) {
        p4_pd_dc_ipv6_fib_match_spec_t v6_match_spec;
        p4_pd_dc_fib_hit_ecmp_action_spec_t v6_action_spec;

        SWITCH_MEMSET(
            &v6_match_spec, 0x0, sizeof(p4_pd_dc_ipv6_fib_match_spec_t));
        SWITCH_MEMSET(
            &v6_action_spec, 0x0, sizeof(p4_pd_dc_fib_hit_ecmp_action_spec_t));

        v6_match_spec.l3_metadata_vrf = vrf;
        SWITCH_MEMCPY(&v6_match_spec.ipv6_metadata_lkp_ipv6_da,
                      &ip_addr->ip.v6addr,
                      SWITCH_IPV6_PREFIX_LENGTH);
        v6_action_spec.action_ecmp_index = nexthop;

        pd_status =
            p4_pd_dc_ipv6_fib_table_add_with_fib_hit_ecmp(switch_cfg_sess_hdl,
                                                          p4_pd_device,
                                                          &v6_match_spec,
                                                          &v6_action_spec,
                                                          entry_hdl);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "fib entry add failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }

        if (switch_pd_log_level_debug()) {
          switch_pd_dump_entry_t pd_entry;
          SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
          pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
          pd_entry.match_spec = (switch_uint8_t *)&v6_match_spec;
          pd_entry.match_spec_size = sizeof(v6_match_spec);
          pd_entry.action_spec = (switch_uint8_t *)&v6_action_spec;
          pd_entry.action_spec_size = sizeof(v6_action_spec);
          pd_entry.pd_hdl = *entry_hdl;
          switch_pd_entry_dump(device, &pd_entry);
        }
      } else {
        p4_pd_dc_ipv6_fib_lpm_match_spec_t v6_match_spec;
        p4_pd_dc_fib_hit_ecmp_action_spec_t v6_action_spec;

        SWITCH_MEMSET(
            &v6_match_spec, 0x0, sizeof(p4_pd_dc_ipv6_fib_lpm_match_spec_t));
        SWITCH_MEMSET(
            &v6_action_spec, 0x0, sizeof(p4_pd_dc_fib_hit_ecmp_action_spec_t));

        v6_match_spec.l3_metadata_vrf = vrf;
        SWITCH_MEMCPY(&v6_match_spec.ipv6_metadata_lkp_ipv6_da,
                      &ip_addr->ip.v6addr,
                      SWITCH_IPV6_PREFIX_LENGTH);
        v6_match_spec.ipv6_metadata_lkp_ipv6_da_prefix_length =
            ip_addr->prefix_len;
        v6_action_spec.action_ecmp_index = nexthop;

        pd_status = p4_pd_dc_ipv6_fib_lpm_table_add_with_fib_hit_ecmp(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &v6_match_spec,
            &v6_action_spec,
            entry_hdl);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "fib entry add failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }

        if (switch_pd_log_level_debug()) {
          switch_pd_dump_entry_t pd_entry;
          SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
          pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
          pd_entry.match_spec = (switch_uint8_t *)&v6_match_spec;
          pd_entry.match_spec_size = sizeof(v6_match_spec);
          pd_entry.action_spec = (switch_uint8_t *)&v6_action_spec;
          pd_entry.action_spec_size = sizeof(v6_action_spec);
          pd_entry.pd_hdl = *entry_hdl;
          switch_pd_entry_dump(device, &pd_entry);
        }
      }
    } else {
      if (host_entry) {
        p4_pd_dc_ipv6_fib_match_spec_t v6_match_spec;
        SWITCH_MEMSET(
            &v6_match_spec, 0x0, sizeof(p4_pd_dc_ipv6_fib_match_spec_t));

        v6_match_spec.l3_metadata_vrf = vrf;
        SWITCH_MEMCPY(&v6_match_spec.ipv6_metadata_lkp_ipv6_da,
                      &ip_addr->ip.v6addr,
                      SWITCH_IPV6_PREFIX_LENGTH);
        if (type == SWITCH_ROUTE_TYPE_MYIP) {
          p4_pd_dc_fib_hit_myip_action_spec_t v6_action_spec;

          SWITCH_MEMSET(&v6_action_spec,
                        0x0,
                        sizeof(p4_pd_dc_fib_hit_myip_action_spec_t));

          v6_action_spec.action_nexthop_index = nexthop;

          pd_status =
              p4_pd_dc_ipv6_fib_table_add_with_fib_hit_myip(switch_cfg_sess_hdl,
                                                            p4_pd_device,
                                                            &v6_match_spec,
                                                            &v6_action_spec,
                                                            entry_hdl);

        } else {
          p4_pd_dc_fib_hit_nexthop_action_spec_t v6_action_spec;

          SWITCH_MEMSET(&v6_action_spec,
                        0x0,
                        sizeof(p4_pd_dc_fib_hit_nexthop_action_spec_t));

          v6_action_spec.action_nexthop_index = nexthop;

          pd_status = p4_pd_dc_ipv6_fib_table_add_with_fib_hit_nexthop(
              switch_cfg_sess_hdl,
              p4_pd_device,
              &v6_match_spec,
              &v6_action_spec,
              entry_hdl);
          if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
            SWITCH_PD_LOG_ERROR(
                "fib entry add failed "
                "on device %d : table %s action %s\n",
                device,
                switch_pd_table_id_to_string(0),
                switch_pd_action_id_to_string(0));
            goto cleanup;
          }

          if (switch_pd_log_level_debug()) {
            switch_pd_dump_entry_t pd_entry;
            SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
            pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
            pd_entry.match_spec = (switch_uint8_t *)&v6_match_spec;
            pd_entry.match_spec_size = sizeof(v6_match_spec);
            pd_entry.action_spec = (switch_uint8_t *)&v6_action_spec;
            pd_entry.action_spec_size = sizeof(v6_action_spec);
            pd_entry.pd_hdl = *entry_hdl;
            switch_pd_entry_dump(device, &pd_entry);
          }
        }
      } else {
        p4_pd_dc_ipv6_fib_lpm_match_spec_t v6_match_spec;
        p4_pd_dc_fib_hit_nexthop_action_spec_t v6_action_spec;

        SWITCH_MEMSET(
            &v6_match_spec, 0x0, sizeof(p4_pd_dc_ipv6_fib_lpm_match_spec_t));
        SWITCH_MEMSET(&v6_action_spec,
                      0x0,
                      sizeof(p4_pd_dc_fib_hit_nexthop_action_spec_t));

        v6_match_spec.l3_metadata_vrf = vrf;
        SWITCH_MEMCPY(&v6_match_spec.ipv6_metadata_lkp_ipv6_da,
                      &ip_addr->ip.v6addr,
                      SWITCH_IPV6_PREFIX_LENGTH);
        v6_match_spec.ipv6_metadata_lkp_ipv6_da_prefix_length =
            ip_addr->prefix_len;
        v6_action_spec.action_nexthop_index = nexthop;

        pd_status = p4_pd_dc_ipv6_fib_lpm_table_add_with_fib_hit_nexthop(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &v6_match_spec,
            &v6_action_spec,
            entry_hdl);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "fib entry add failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }

        if (switch_pd_log_level_debug()) {
          switch_pd_dump_entry_t pd_entry;
          SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
          pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
          pd_entry.match_spec = (switch_uint8_t *)&v6_match_spec;
          pd_entry.match_spec_size = sizeof(v6_match_spec);
          pd_entry.action_spec = (switch_uint8_t *)&v6_action_spec;
          pd_entry.action_spec_size = sizeof(v6_action_spec);
          pd_entry.pd_hdl = *entry_hdl;
          switch_pd_entry_dump(device, &pd_entry);
        }
      }
    }
#endif /* P4_IPV6_DISABLE */
  }

cleanup:
  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* L3_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "fib entry add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "fib entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_ip_fib_entry_update(switch_device_t device,
                                              switch_vrf_id_t vrf,
                                              switch_ip_addr_t *ip_addr,
                                              bool ecmp,
                                              switch_nhop_t nexthop,
                                              switch_pd_hdl_t entry_hdl,
                                              uint32_t flags) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(vrf);
  UNUSED(ip_addr);
  UNUSED(ecmp);
  UNUSED(nexthop);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L3_DISABLE
  bool host_entry = TRUE;

  if (ip_addr->type == SWITCH_API_IP_ADDR_V4) {
#ifndef P4_IPV4_DISABLE
    host_entry =
        ip_addr->prefix_len == SWITCH_IPV4_PREFIX_LENGTH_IN_BITS ? TRUE : FALSE;
    if (flags) {
      if (flags & SWITCH_IP_FORCE_HOST_IN_LPM)
        host_entry = FALSE;
      else if (flags & SWITCH_IP_FORCE_HOST_IN_EXACT)
        host_entry = TRUE;
    }
    if (ecmp) {
      if (host_entry) {
        p4_pd_dc_fib_hit_ecmp_action_spec_t v4_action_spec;
        SWITCH_MEMSET(
            &v4_action_spec, 0x0, sizeof(p4_pd_dc_fib_hit_ecmp_action_spec_t));
        v4_action_spec.action_ecmp_index = nexthop;

#ifdef IPV4_LOCAL_HOST_TABLE_SIZE
        if (flags & SWITCH_IP_FORCE_HOST_IN_LOCAL_HOST)
          pd_status =
              p4_pd_dc_ipv4_fib_local_hosts_table_modify_with_fib_hit_ecmp(
                  switch_cfg_sess_hdl, device, entry_hdl, &v4_action_spec);
        else
#endif
          pd_status = p4_pd_dc_ipv4_fib_table_modify_with_fib_hit_ecmp(
              switch_cfg_sess_hdl, device, entry_hdl, &v4_action_spec);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "fib entry update failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }

        if (switch_pd_log_level_debug()) {
          switch_pd_dump_entry_t pd_entry;
          SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
          pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
          pd_entry.match_spec_size = 0;
          pd_entry.action_spec = (switch_uint8_t *)&v4_action_spec;
          pd_entry.action_spec_size = sizeof(v4_action_spec);
          pd_entry.pd_hdl = entry_hdl;
          switch_pd_entry_dump(device, &pd_entry);
        }
      } else {
        p4_pd_dc_fib_hit_ecmp_action_spec_t v4_action_spec;
        SWITCH_MEMSET(
            &v4_action_spec, 0x0, sizeof(p4_pd_dc_fib_hit_ecmp_action_spec_t));
        v4_action_spec.action_ecmp_index = nexthop;

        pd_status = p4_pd_dc_ipv4_fib_lpm_table_modify_with_fib_hit_ecmp(
            switch_cfg_sess_hdl, device, entry_hdl, &v4_action_spec);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "fib entry update failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }

        if (switch_pd_log_level_debug()) {
          switch_pd_dump_entry_t pd_entry;
          SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
          pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
          pd_entry.match_spec_size = 0;
          pd_entry.action_spec = (switch_uint8_t *)&v4_action_spec;
          pd_entry.action_spec_size = sizeof(v4_action_spec);
          pd_entry.pd_hdl = entry_hdl;
          switch_pd_entry_dump(device, &pd_entry);
        }
      }
    } else {
      if (host_entry) {
        p4_pd_dc_fib_hit_nexthop_action_spec_t v4_action_spec;
        SWITCH_MEMSET(&v4_action_spec,
                      0x0,
                      sizeof(p4_pd_dc_fib_hit_nexthop_action_spec_t));
        v4_action_spec.action_nexthop_index = nexthop;

#ifdef IPV4_LOCAL_HOST_TABLE_SIZE
        if (flags & SWITCH_IP_FORCE_HOST_IN_LOCAL_HOST)
          pd_status =
              p4_pd_dc_ipv4_fib_local_hosts_table_modify_with_fib_hit_nexthop(
                  switch_cfg_sess_hdl, device, entry_hdl, &v4_action_spec);
        else
#endif
          pd_status = p4_pd_dc_ipv4_fib_table_modify_with_fib_hit_nexthop(
              switch_cfg_sess_hdl, device, entry_hdl, &v4_action_spec);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "fib entry update failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }

        if (switch_pd_log_level_debug()) {
          switch_pd_dump_entry_t pd_entry;
          SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
          pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
          pd_entry.match_spec_size = 0;
          pd_entry.action_spec = (switch_uint8_t *)&v4_action_spec;
          pd_entry.action_spec_size = sizeof(v4_action_spec);
          pd_entry.pd_hdl = entry_hdl;
          switch_pd_entry_dump(device, &pd_entry);
        }
      } else {
        p4_pd_dc_fib_hit_nexthop_action_spec_t v4_action_spec;
        SWITCH_MEMSET(
            &v4_action_spec, 0, sizeof(p4_pd_dc_fib_hit_nexthop_action_spec_t));
        v4_action_spec.action_nexthop_index = nexthop;

        pd_status = p4_pd_dc_ipv4_fib_lpm_table_modify_with_fib_hit_nexthop(
            switch_cfg_sess_hdl, device, entry_hdl, &v4_action_spec);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "fib entry update failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }

        if (switch_pd_log_level_debug()) {
          switch_pd_dump_entry_t pd_entry;
          SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
          pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
          pd_entry.match_spec_size = 0;
          pd_entry.action_spec = (switch_uint8_t *)&v4_action_spec;
          pd_entry.action_spec_size = sizeof(v4_action_spec);
          pd_entry.pd_hdl = entry_hdl;
          switch_pd_entry_dump(device, &pd_entry);
        }
      }
    }
#endif /* P4_IPV4_DISABLE */
  } else {
#ifndef P4_IPV6_DISABLE
    host_entry =
        ip_addr->prefix_len == SWITCH_IPV6_PREFIX_LENGTH_IN_BITS ? TRUE : FALSE;
    if (ecmp) {
      if (host_entry) {
        p4_pd_dc_fib_hit_ecmp_action_spec_t v6_action_spec;
        SWITCH_MEMSET(
            &v6_action_spec, 0x0, sizeof(p4_pd_dc_fib_hit_ecmp_action_spec_t));
        v6_action_spec.action_ecmp_index = nexthop;

        pd_status = p4_pd_dc_ipv6_fib_table_modify_with_fib_hit_ecmp(
            switch_cfg_sess_hdl, device, entry_hdl, &v6_action_spec);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "fib entry update failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }

        if (switch_pd_log_level_debug()) {
          switch_pd_dump_entry_t pd_entry;
          SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
          pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
          pd_entry.match_spec_size = 0;
          pd_entry.action_spec = (switch_uint8_t *)&v6_action_spec;
          pd_entry.action_spec_size = sizeof(v6_action_spec);
          pd_entry.pd_hdl = entry_hdl;
          switch_pd_entry_dump(device, &pd_entry);
        }
      } else {
        p4_pd_dc_fib_hit_ecmp_action_spec_t v6_action_spec;
        SWITCH_MEMSET(
            &v6_action_spec, 0x0, sizeof(p4_pd_dc_fib_hit_ecmp_action_spec_t));
        v6_action_spec.action_ecmp_index = nexthop;

        pd_status = p4_pd_dc_ipv6_fib_lpm_table_modify_with_fib_hit_ecmp(
            switch_cfg_sess_hdl, device, entry_hdl, &v6_action_spec);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "fib entry update failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }

        if (switch_pd_log_level_debug()) {
          switch_pd_dump_entry_t pd_entry;
          SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
          pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
          pd_entry.match_spec_size = 0;
          pd_entry.action_spec = (switch_uint8_t *)&v6_action_spec;
          pd_entry.action_spec_size = sizeof(v6_action_spec);
          pd_entry.pd_hdl = entry_hdl;
          switch_pd_entry_dump(device, &pd_entry);
        }
      }
    } else {
      if (host_entry) {
        p4_pd_dc_fib_hit_nexthop_action_spec_t v6_action_spec;
        SWITCH_MEMSET(&v6_action_spec,
                      0x0,
                      sizeof(p4_pd_dc_fib_hit_nexthop_action_spec_t));
        v6_action_spec.action_nexthop_index = nexthop;

        pd_status = p4_pd_dc_ipv6_fib_table_modify_with_fib_hit_nexthop(
            switch_cfg_sess_hdl, device, entry_hdl, &v6_action_spec);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "fib entry update failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }

        if (switch_pd_log_level_debug()) {
          switch_pd_dump_entry_t pd_entry;
          SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
          pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
          pd_entry.match_spec_size = 0;
          pd_entry.action_spec = (switch_uint8_t *)&v6_action_spec;
          pd_entry.action_spec_size = sizeof(v6_action_spec);
          pd_entry.pd_hdl = entry_hdl;
          switch_pd_entry_dump(device, &pd_entry);
        }
      } else {
        p4_pd_dc_fib_hit_nexthop_action_spec_t v6_action_spec;
        SWITCH_MEMSET(&v6_action_spec,
                      0x0,
                      sizeof(p4_pd_dc_fib_hit_nexthop_action_spec_t));
        v6_action_spec.action_nexthop_index = nexthop;

        pd_status = p4_pd_dc_ipv6_fib_lpm_table_modify_with_fib_hit_nexthop(
            switch_cfg_sess_hdl, device, entry_hdl, &v6_action_spec);
        if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
          SWITCH_PD_LOG_ERROR(
              "fib entry update failed "
              "on device %d : table %s action %s\n",
              device,
              switch_pd_table_id_to_string(0),
              switch_pd_action_id_to_string(0));
          goto cleanup;
        }

        if (switch_pd_log_level_debug()) {
          switch_pd_dump_entry_t pd_entry;
          SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
          pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
          pd_entry.match_spec_size = 0;
          pd_entry.action_spec = (switch_uint8_t *)&v6_action_spec;
          pd_entry.action_spec_size = sizeof(v6_action_spec);
          pd_entry.pd_hdl = entry_hdl;
          switch_pd_entry_dump(device, &pd_entry);
        }
      }
    }
#endif /* P4_IPV6_DISABLE */
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L3_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "fib entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "fib entry update failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl %lx\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_ip_fib_entry_delete(switch_device_t device,
                                              switch_ip_addr_t *ip_addr,
                                              switch_pd_hdl_t entry_hdl,
                                              uint32_t flags) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(ip_addr);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef L3_DISABLE
  bool host_entry = TRUE;

  if (ip_addr->type == SWITCH_API_IP_ADDR_V4) {
#ifndef IPV4_DISABLE
    host_entry =
        ip_addr->prefix_len == SWITCH_IPV4_PREFIX_LENGTH_IN_BITS ? TRUE : FALSE;
    if (flags) {
      if (flags & SWITCH_IP_FORCE_HOST_IN_LPM)
        host_entry = FALSE;
      else if (flags & SWITCH_IP_FORCE_HOST_IN_EXACT)
        host_entry = TRUE;
    }
    if (host_entry) {
#ifdef IPV4_LOCAL_HOST_TABLE_SIZE
      if (flags & SWITCH_IP_FORCE_HOST_IN_LOCAL_HOST)
        pd_status = p4_pd_dc_ipv4_fib_local_hosts_table_delete(
            switch_cfg_sess_hdl, device, entry_hdl);
      else
#endif
        pd_status = p4_pd_dc_ipv4_fib_table_delete(
            switch_cfg_sess_hdl, device, entry_hdl);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "fib entry delete failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_DELETE;
        pd_entry.match_spec_size = 0;
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } else {
      pd_status = p4_pd_dc_ipv4_fib_lpm_table_delete(
          switch_cfg_sess_hdl, device, entry_hdl);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "fib entry delete failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_DELETE;
        pd_entry.match_spec_size = 0;
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    }
#endif /* IPV4_DISABLE */
  } else {
#ifndef P4_IPV6_DISABLE
    host_entry =
        ip_addr->prefix_len == SWITCH_IPV6_PREFIX_LENGTH_IN_BITS ? TRUE : FALSE;
    if (host_entry) {
      pd_status = p4_pd_dc_ipv6_fib_table_delete(
          switch_cfg_sess_hdl, device, entry_hdl);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "fib entry delete failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_DELETE;
        pd_entry.match_spec_size = 0;
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    } else {
      pd_status = p4_pd_dc_ipv6_fib_lpm_table_delete(
          switch_cfg_sess_hdl, device, entry_hdl);
      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "fib entry delete failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_DELETE;
        pd_entry.match_spec_size = 0;
        pd_entry.action_spec_size = 0;
        pd_entry.pd_hdl = entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }
    }
#endif /* P4_IPV6_DISABLE */
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* L3_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "fib entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "fib entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_urpf_bd_table_entry_add(
    switch_device_t device,
    switch_urpf_group_t urpf_group,
    switch_bd_t bd,
    switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(urpf_group);
  UNUSED(bd);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_L3_DISABLE) && !defined(P4_URPF_DISABLE)

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_urpf_bd_match_spec_t match_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_urpf_bd_match_spec_t));
  match_spec.l3_metadata_urpf_bd_group = urpf_group;
  match_spec.ingress_metadata_bd = bd;
  pd_status = p4_pd_dc_urpf_bd_table_add_with_nop(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, entry_hdl);

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = *entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "urpf table add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L3_DISABLE && P4_URPF_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "urpf table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "urpf table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_urpf_bd_table_entry_update(
    switch_device_t device,
    switch_urpf_group_t urpf_group,
    switch_bd_t bd,
    switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(urpf_group);
  UNUSED(bd);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_L3_DISABLE) && !defined(P4_URPF_DISABLE)

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  pd_status = p4_pd_dc_urpf_bd_table_modify_with_nop(
      switch_cfg_sess_hdl, p4_pd_device.device_id, entry_hdl);

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_UPDATE;
    pd_entry.match_spec_size = 0;
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "urpf table update failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L3_DISABLE && P4_URPF_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "urpf table entry update success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "urpf table entry update failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_urpf_bd_table_entry_delete(
    switch_device_t device, switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_L3_DISABLE) && !defined(P4_URPF_DISABLE)

  if (!SWITCH_PD_HANDLE_VALID(entry_hdl)) {
    return status;
  }

  pd_status =
      p4_pd_dc_urpf_bd_table_delete(switch_cfg_sess_hdl, device, entry_hdl);

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DELETE;
    pd_entry.match_spec_size = 0;
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "urpf table entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L3_DISABLE && P4_URPF_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "urpf table entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "urpf table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_urpf_entry_add(switch_device_t device,
                                         switch_vrf_t vrf_id,
                                         switch_ip_addr_t *ip_addr,
                                         switch_urpf_group_t urpf_group,
                                         switch_pd_hdl_t *entry_hdl,
                                         uint32_t flags) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(vrf_id);
  UNUSED(ip_addr);
  UNUSED(urpf_group);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_L3_DISABLE) && !defined(P4_URPF_DISABLE)

  p4_pd_dev_target_t p4_pd_device;
  bool host_entry = TRUE;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

  if (ip_addr->type == SWITCH_API_IP_ADDR_V4) {
#ifndef IPV4_DISABLE
    host_entry =
        (ip_addr->prefix_len == SWITCH_IPV4_PREFIX_LENGTH) ? TRUE : FALSE;
    p4_pd_dc_ipv4_urpf_hit_action_spec_t v4_action_spec;
    SWITCH_MEMSET(&v4_action_spec, 0x0, sizeof(v4_action_spec));
    v4_action_spec.action_urpf_bd_group = urpf_group;
    if (host_entry) {
      p4_pd_dc_ipv4_urpf_match_spec_t v4_match_spec;
      SWITCH_MEMSET(&v4_match_spec, 0x0, sizeof(v4_match_spec));
      v4_match_spec.l3_metadata_vrf = vrf_id;
      v4_match_spec.ipv4_metadata_lkp_ipv4_sa = ip_addr->ip.v4addr;
#ifdef IPV4_LOCAL_HOST_TABLE_SIZE
      if (flags & SWITCH_IP_FORCE_HOST_IN_LOCAL_HOST)
        pd_status = p4_pd_dc_ipv4_urpf_table_add_with_ipv4_local_hosts_urpf_hit(
            switch_cfg_sess_hdl,
            p4_pd_device,
            &v4_match_spec,
            &v4_action_spec,
            entry_hdl);
      else
#endif
        pd_status =
            p4_pd_dc_ipv4_urpf_table_add_with_ipv4_urpf_hit(switch_cfg_sess_hdl,
                                                            p4_pd_device,
                                                            &v4_match_spec,
                                                            &v4_action_spec,
                                                            entry_hdl);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&v4_match_spec;
        pd_entry.match_spec_size = sizeof(v4_match_spec);
        pd_entry.action_spec = (switch_uint8_t *)&v4_action_spec;
        pd_entry.action_spec_size = sizeof(v4_action_spec);
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "urpf table add failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } else {
      p4_pd_dc_ipv4_urpf_lpm_match_spec_t v4_match_spec;
      SWITCH_MEMSET(&v4_match_spec, 0x0, sizeof(v4_match_spec));
      v4_match_spec.l3_metadata_vrf = vrf_id;
      v4_match_spec.ipv4_metadata_lkp_ipv4_sa = ip_addr->ip.v4addr;
      v4_match_spec.ipv4_metadata_lkp_ipv4_sa_prefix_length =
          ip_addr->prefix_len;
      pd_status = p4_pd_dc_ipv4_urpf_lpm_table_add_with_ipv4_urpf_hit(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &v4_match_spec,
          &v4_action_spec,
          entry_hdl);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&v4_match_spec;
        pd_entry.match_spec_size = sizeof(v4_match_spec);
        pd_entry.action_spec = (switch_uint8_t *)&v4_action_spec;
        pd_entry.action_spec_size = sizeof(v4_action_spec);
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "urpf table add failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    }
#endif /* IPV4_DISABLE */
  } else {
#ifndef P4_IPV6_DISABLE
    host_entry =
        (ip_addr->prefix_len == SWITCH_IPV6_PREFIX_LENGTH) ? TRUE : FALSE;
    p4_pd_dc_ipv6_urpf_hit_action_spec_t v6_action_spec;
    SWITCH_MEMSET(&v6_action_spec, 0x0, sizeof(v6_action_spec));
    v6_action_spec.action_urpf_bd_group = urpf_group;
    if (host_entry) {
      p4_pd_dc_ipv6_urpf_match_spec_t v6_match_spec;
      SWITCH_MEMSET(&v6_match_spec, 0x0, sizeof(v6_match_spec));
      v6_match_spec.l3_metadata_vrf = vrf_id;
      SWITCH_MEMCPY(&v6_match_spec.ipv6_metadata_lkp_ipv6_sa,
                    &ip_addr->ip.v6addr,
                    SWITCH_IPV6_PREFIX_LENGTH);
      pd_status =
          p4_pd_dc_ipv6_urpf_table_add_with_ipv6_urpf_hit(switch_cfg_sess_hdl,
                                                          p4_pd_device,
                                                          &v6_match_spec,
                                                          &v6_action_spec,
                                                          entry_hdl);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&v6_match_spec;
        pd_entry.match_spec_size = sizeof(v6_match_spec);
        pd_entry.action_spec = (switch_uint8_t *)&v6_action_spec;
        pd_entry.action_spec_size = sizeof(v6_action_spec);
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "urpf table add failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    } else {
      p4_pd_dc_ipv6_urpf_lpm_match_spec_t v6_match_spec;
      SWITCH_MEMSET(&v6_match_spec, 0x0, sizeof(v6_match_spec));
      v6_match_spec.l3_metadata_vrf = vrf_id;
      SWITCH_MEMCPY(&v6_match_spec.ipv6_metadata_lkp_ipv6_sa,
                    &ip_addr->ip.v6addr,
                    SWITCH_IPV6_PREFIX_LENGTH);
      v6_match_spec.ipv6_metadata_lkp_ipv6_sa_prefix_length =
          ip_addr->prefix_len;
      pd_status = p4_pd_dc_ipv6_urpf_lpm_table_add_with_ipv6_urpf_hit(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &v6_match_spec,
          &v6_action_spec,
          entry_hdl);

      if (switch_pd_log_level_debug()) {
        switch_pd_dump_entry_t pd_entry;
        SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
        pd_entry.entry_type = SWITCH_PD_ENTRY_ADD;
        pd_entry.match_spec = (switch_uint8_t *)&v6_match_spec;
        pd_entry.match_spec_size = sizeof(v6_match_spec);
        pd_entry.action_spec = (switch_uint8_t *)&v6_action_spec;
        pd_entry.action_spec_size = sizeof(v6_action_spec);
        pd_entry.pd_hdl = *entry_hdl;
        switch_pd_entry_dump(device, &pd_entry);
      }

      if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
        SWITCH_PD_LOG_ERROR(
            "urpf table add failed "
            "on device %d : table %s action %s\n",
            device,
            switch_pd_table_id_to_string(0),
            switch_pd_action_id_to_string(0));
        goto cleanup;
      }
    }
#endif /* P4_IPV6_DISABLE */
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L3_DISABLE && P4_URPF_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "urpf table entry add success "
        "on device %d 0x%lx\n",
        device,
        *entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "urpf table entry add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_urpf_entry_update(switch_device_t device,
                                            switch_vrf_t vrf_id,
                                            switch_ip_addr_t *ip_addr,
                                            switch_urpf_group_t urpf_group,
                                            switch_pd_hdl_t entry_hdl,
                                            uint32_t flags) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);
  UNUSED(vrf_id);
  UNUSED(ip_addr);
  UNUSED(urpf_group);
  UNUSED(entry_hdl);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_L3_DISABLE) && !defined(P4_URPF_DISABLE)

  bool host_entry = TRUE;
  if (ip_addr->type == SWITCH_API_IP_ADDR_V4) {
#ifndef IPV4_DISABLE
    host_entry =
        (ip_addr->prefix_len == SWITCH_IPV4_PREFIX_LENGTH) ? TRUE : FALSE;
    p4_pd_dc_ipv4_urpf_hit_action_spec_t v4_action_spec;
    SWITCH_MEMSET(&v4_action_spec, 0x0, sizeof(v4_action_spec));
    v4_action_spec.action_urpf_bd_group = urpf_group;
    if (host_entry) {
#ifdef IPV4_LOCAL_HOST_TABLE_SIZE
      if (flags & SWITCH_IP_FORCE_HOST_IN_LOCAL_HOST)
        pd_status =
            p4_pd_dc_ipv4_local_hosts_urpf_table_modify_with_ipv4_urpf_hit(
                switch_cfg_sess_hdl, device, entry_hdl, &v4_action_spec);
      else
#endif
        pd_status = p4_pd_dc_ipv4_urpf_table_modify_with_ipv4_urpf_hit(
            switch_cfg_sess_hdl, device, entry_hdl, &v4_action_spec);
    } else {
      pd_status = p4_pd_dc_ipv4_urpf_lpm_table_modify_with_ipv4_urpf_hit(
          switch_cfg_sess_hdl, device, entry_hdl, &v4_action_spec);
    }
#endif /* P4_IPV4_DISABLE */
  } else {
#ifndef P4_IPV6_DISABLE
    host_entry =
        (ip_addr->prefix_len == SWITCH_IPV6_PREFIX_LENGTH) ? TRUE : FALSE;
    p4_pd_dc_ipv6_urpf_hit_action_spec_t v6_action_spec;
    SWITCH_MEMSET(&v6_action_spec, 0x0, sizeof(v6_action_spec));
    v6_action_spec.action_urpf_bd_group = urpf_group;
    if (host_entry) {
      pd_status = p4_pd_dc_ipv6_urpf_table_modify_with_ipv6_urpf_hit(
          switch_cfg_sess_hdl, device, entry_hdl, &v6_action_spec);
    } else {
      pd_status = p4_pd_dc_ipv6_urpf_lpm_table_modify_with_ipv6_urpf_hit(
          switch_cfg_sess_hdl, device, entry_hdl, &v6_action_spec);
    }
#endif /* P4_IPV6_DISABLE */
  }
#endif /* P4_L3_DISABLE && P4_URPF_DISABLE */

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_urpf_entry_delete(switch_device_t device,
                                            switch_vrf_id_t vrf_id,
                                            switch_ip_addr_t *ip_addr,
                                            switch_pd_hdl_t entry_hdl,
                                            uint32_t flags) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_L3_DISABLE) && !defined(P4_URPF_DISABLE)

  bool host_entry = TRUE;
  if (ip_addr->type == SWITCH_API_IP_ADDR_V4) {
#ifndef IPV4_DISABLE
    host_entry =
        (ip_addr->prefix_len == SWITCH_IPV4_PREFIX_LENGTH) ? TRUE : FALSE;
    if (host_entry) {
#ifdef IPV4_LOCAL_HOST_TABLE_SIZE
      if (flags & SWITCH_IP_FORCE_HOST_IN_LOCAL_HOST)
        pd_status = p4_pd_dc_ipv4_local_hosts_urpf_table_delete(
            switch_cfg_sess_hdl, device, entry_hdl);
      else
#endif
        pd_status = p4_pd_dc_ipv4_urpf_table_delete(
            switch_cfg_sess_hdl, device, entry_hdl);
    } else {
      pd_status = p4_pd_dc_ipv4_urpf_lpm_table_delete(
          switch_cfg_sess_hdl, device, entry_hdl);
    }
#endif /* P4_IPV4_DISABLE */
  } else {
#ifndef P4_IPV6_DISABLE
    host_entry =
        (ip_addr->prefix_len == SWITCH_IPV6_PREFIX_LENGTH) ? TRUE : FALSE;
    if (host_entry) {
      pd_status = p4_pd_dc_ipv6_urpf_table_delete(
          switch_cfg_sess_hdl, device, entry_hdl);
    } else {
      pd_status = p4_pd_dc_ipv6_urpf_lpm_table_delete(
          switch_cfg_sess_hdl, device, entry_hdl);
    }
#endif /* P4_IPV6_DISABLE */
  }

  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DELETE;
    pd_entry.match_spec_size = 0;
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "urpf table entry delete failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L3_DISABLE && P4_URPF_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "rid table entry delete success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "rid table entry delete failed "
        "on device %d : %s"
        "(pd: 0x%x) for pd_hdl\n",
        device,
        switch_error_to_string(status),
        pd_status,
        entry_hdl);
  }
  return status;
}

switch_status_t switch_pd_validate_outer_ip_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L3_DISABLE

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

#ifndef P4_IPV4_DISABLE
  p4_pd_dc_validate_outer_ipv4_packet_match_spec_t match_spec;
  p4_pd_dc_set_malformed_outer_ipv4_packet_action_spec_t action_spec;

  /* default entry */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  action_spec.action_drop_reason = DROP_OUTER_IP_MISS;
  pd_status =
      p4_pd_dc_validate_outer_ipv4_packet_set_default_action_set_malformed_outer_ipv4_packet(
          switch_cfg_sess_hdl, p4_pd_device, &action_spec, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DEFAULT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate ipv4 table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  /* ipv4 src is loopback */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  match_spec.ipv4_srcAddr = 0x7f000000;
  match_spec.ipv4_srcAddr_mask = 0xff000000;
  action_spec.action_drop_reason = DROP_OUTER_IP_SRC_LOOPBACK;
  pd_status =
      p4_pd_dc_validate_outer_ipv4_packet_table_add_with_set_malformed_outer_ipv4_packet(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          10,
          &action_spec,
          &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DEFAULT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate ipv4 table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  /* ipv4 src is multicast */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  match_spec.ipv4_srcAddr = 0xe0000000;
  match_spec.ipv4_srcAddr_mask = 0xf0000000;
  action_spec.action_drop_reason = DROP_OUTER_IP_SRC_MULTICAST;
  pd_status =
      p4_pd_dc_validate_outer_ipv4_packet_table_add_with_set_malformed_outer_ipv4_packet(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          11,
          &action_spec,
          &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DEFAULT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate ipv4 table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  /* ttl is zero */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  match_spec.ipv4_ttl_mask = 0xff;
  action_spec.action_drop_reason = DROP_OUTER_IP_TTL_ZERO;
  pd_status =
      p4_pd_dc_validate_outer_ipv4_packet_table_add_with_set_malformed_outer_ipv4_packet(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          12,
          &action_spec,
          &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DEFAULT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate ipv4 table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  /* invalid ihl */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  match_spec.ipv4_ihl = 0;
  match_spec.ipv4_ihl_mask = 0xfc;
  action_spec.action_drop_reason = DROP_OUTER_IP_IHL_INVALID;
  pd_status =
      p4_pd_dc_validate_outer_ipv4_packet_table_add_with_set_malformed_outer_ipv4_packet(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          13,
          &action_spec,
          &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DEFAULT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate ipv4 table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  match_spec.ipv4_ihl = 4;
  match_spec.ipv4_ihl_mask = 0xff;
  action_spec.action_drop_reason = DROP_OUTER_IP_IHL_INVALID;
  pd_status =
      p4_pd_dc_validate_outer_ipv4_packet_table_add_with_set_malformed_outer_ipv4_packet(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          13,
          &action_spec,
          &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DEFAULT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate ipv4 table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  /* version is 4 and packet is okay */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  match_spec.ipv4_version = 0x04;
  match_spec.ipv4_version_mask = 0xff;
  pd_status =
      p4_pd_dc_validate_outer_ipv4_packet_table_add_with_set_valid_outer_ipv4_packet(
          switch_cfg_sess_hdl, p4_pd_device, &match_spec, 14, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DEFAULT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate ipv4 table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  /* invalid version */
  SWITCH_MEMSET(&match_spec, 0x0, sizeof(match_spec));
  SWITCH_MEMSET(&action_spec, 0x0, sizeof(action_spec));
  action_spec.action_drop_reason = DROP_OUTER_IP_VERSION_INVALID;
  pd_status =
      p4_pd_dc_validate_outer_ipv4_packet_table_add_with_set_malformed_outer_ipv4_packet(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &match_spec,
          15,
          &action_spec,
          &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DEFAULT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&action_spec;
    pd_entry.action_spec_size = sizeof(action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate ipv4 table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_IPV4_DISABLE */

#ifndef P4_IPV6_DISABLE
  p4_pd_dc_validate_outer_ipv6_packet_match_spec_t v6_match_spec;
  p4_pd_dc_set_malformed_outer_ipv6_packet_action_spec_t v6_action_spec;

  /* default entry */
  SWITCH_MEMSET(&v6_match_spec, 0x0, sizeof(v6_match_spec));
  SWITCH_MEMSET(&v6_action_spec, 0x0, sizeof(v6_action_spec));
  v6_action_spec.action_drop_reason = DROP_OUTER_IP_MISS;
  pd_status =
      p4_pd_dc_validate_outer_ipv6_packet_set_default_action_set_malformed_outer_ipv6_packet(
          switch_cfg_sess_hdl, p4_pd_device, &v6_action_spec, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DEFAULT;
    pd_entry.match_spec = (switch_uint8_t *)&v6_match_spec;
    pd_entry.match_spec_size = sizeof(v6_match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&v6_action_spec;
    pd_entry.action_spec_size = sizeof(v6_action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate ipv6 table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  /* ipv6 src is multicast */
  SWITCH_MEMSET(&v6_match_spec, 0x0, sizeof(v6_match_spec));
  SWITCH_MEMSET(&v6_action_spec, 0x0, sizeof(v6_action_spec));

  v6_match_spec.ipv6_srcAddr[0] = 0xff;
  v6_match_spec.ipv6_srcAddr_mask[0] = 0xff;

  v6_action_spec.action_drop_reason = DROP_OUTER_IP_SRC_MULTICAST;
  pd_status =
      p4_pd_dc_validate_outer_ipv6_packet_table_add_with_set_malformed_outer_ipv6_packet(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &v6_match_spec,
          11,
          &v6_action_spec,
          &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DEFAULT;
    pd_entry.match_spec = (switch_uint8_t *)&v6_match_spec;
    pd_entry.match_spec_size = sizeof(v6_match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&v6_action_spec;
    pd_entry.action_spec_size = sizeof(v6_action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate ipv6 table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  /* ttl is zero */
  SWITCH_MEMSET(&v6_match_spec, 0x0, sizeof(v6_match_spec));
  SWITCH_MEMSET(&v6_action_spec, 0x0, sizeof(v6_action_spec));
  v6_match_spec.ipv6_hopLimit_mask = 0xff;
  v6_action_spec.action_drop_reason = DROP_OUTER_IP_TTL_ZERO;
  pd_status =
      p4_pd_dc_validate_outer_ipv6_packet_table_add_with_set_malformed_outer_ipv6_packet(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &v6_match_spec,
          12,
          &v6_action_spec,
          &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DEFAULT;
    pd_entry.match_spec = (switch_uint8_t *)&v6_match_spec;
    pd_entry.match_spec_size = sizeof(v6_match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&v6_action_spec;
    pd_entry.action_spec_size = sizeof(v6_action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate ipv6 table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  /* version is 6 and packet is okay */
  SWITCH_MEMSET(&v6_match_spec, 0x0, sizeof(v6_match_spec));
  v6_match_spec.ipv6_version = 0x06;
  v6_match_spec.ipv6_version_mask = 0xff;
  pd_status =
      p4_pd_dc_validate_outer_ipv6_packet_table_add_with_set_valid_outer_ipv6_packet(
          switch_cfg_sess_hdl, p4_pd_device, &v6_match_spec, 13, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DEFAULT;
    pd_entry.match_spec = (switch_uint8_t *)&v6_match_spec;
    pd_entry.match_spec_size = sizeof(v6_match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate ipv6 table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  /* invalid version */
  SWITCH_MEMSET(&v6_match_spec, 0x0, sizeof(v6_match_spec));
  SWITCH_MEMSET(&v6_action_spec, 0x0, sizeof(v6_action_spec));
  v6_action_spec.action_drop_reason = DROP_OUTER_IP_VERSION_INVALID;
  pd_status =
      p4_pd_dc_validate_outer_ipv6_packet_table_add_with_set_malformed_outer_ipv6_packet(
          switch_cfg_sess_hdl,
          p4_pd_device,
          &v6_match_spec,
          14,
          &v6_action_spec,
          &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_DEFAULT;
    pd_entry.match_spec = (switch_uint8_t *)&v6_match_spec;
    pd_entry.match_spec_size = sizeof(v6_match_spec);
    pd_entry.action_spec = (switch_uint8_t *)&v6_action_spec;
    pd_entry.action_spec_size = sizeof(v6_action_spec);
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "validate ipv6 table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#endif /* P4_IPV6_DISABLE */

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L3_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "validate ip table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "validate ip table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_ip_fib_default_entry_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L3_DISABLE

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

#ifndef P4_IPV4_DISABLE
#ifdef IPV4_LOCAL_HOST_TABLE_SIZE
  pd_status = p4_pd_dc_ipv4_fib_local_hosts_set_default_action_on_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv4 fib local hosts table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif
  pd_status = p4_pd_dc_ipv4_fib_set_default_action_on_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv4 fib table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  pd_status = p4_pd_dc_ipv4_fib_lpm_set_default_action_on_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv4 fib lpm table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_IPV4_DISABLE */

#ifndef P4_IPV6_DISABLE
  pd_status = p4_pd_dc_ipv6_fib_set_default_action_on_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv6 fib table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  pd_status = p4_pd_dc_ipv6_fib_lpm_set_default_action_on_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv6 fib lpm table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_IPV6_DISABLE */

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L3_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "fib table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "fib table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_ip_urpf_default_entry_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#if !defined(P4_L3_DISABLE) && !defined(P4_URPF_DISABLE)

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

#ifndef P4_IPV4_DISABLE
#ifdef IPV4_LOCAL_HOST_TABLE_SIZE
  pd_status = p4_pd_dc_ipv4_local_hosts_urpf_set_default_action_on_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv4 urpf local hosts table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif
  pd_status = p4_pd_dc_ipv4_urpf_set_default_action_on_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv4 urpf table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  pd_status = p4_pd_dc_ipv4_urpf_lpm_set_default_action_urpf_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv4 urpf lpm table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_IPV4_DISABLE */

#ifndef P4_IPV6_DISABLE
  pd_status = p4_pd_dc_ipv6_urpf_set_default_action_on_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv6 urpf table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

  pd_status = p4_pd_dc_ipv6_urpf_lpm_set_default_action_urpf_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "ipv6 urpf lpm table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#endif /* P4_IPV6_DISABLE */

  pd_status = p4_pd_dc_urpf_bd_set_default_action_urpf_bd_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "urpf bd table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L3_DISABLE && P4_URPF_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "urpf table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "urpf table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

switch_status_t switch_pd_l3_rewrite_table_entry_init(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L3_DISABLE

  p4_pd_dev_target_t p4_pd_device;
  p4_pd_dc_l3_rewrite_match_spec_t match_spec;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = SWITCH_DEV_PIPE_ID;

#ifndef P4_L3_MULTICAST_DISABLE

  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.ipv4_valid = 0x1;
  match_spec.ipv4_dstAddr = 0xe0000000;
  match_spec.ipv4_dstAddr_mask = 0xf0000000;
  pd_status = p4_pd_dc_l3_rewrite_table_add_with_ipv4_multicast_rewrite(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, 100, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "l3 ipv4 mcast rewrite table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#endif /* P4_L3_MULTICAST_DISABLE */

  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.ipv4_valid = 0x1;
  pd_status = p4_pd_dc_l3_rewrite_table_add_with_ipv4_unicast_rewrite(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, 101, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "l3 ipv4 ucast rewrite table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#if !defined(P4_IPV6_DISABLE)
  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.ipv6_valid = 0x1;
  match_spec.ipv6_dstAddr[0] = 0xff;
  match_spec.ipv6_dstAddr_mask[0] = 0xff;
#ifndef P4_L3_MULTICAST_DISABLE
  pd_status = p4_pd_dc_l3_rewrite_table_add_with_ipv6_multicast_rewrite(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, 200, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "l3 ipv6 mcast rewrite table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#endif /* P4_L3_MULTICAST_DISABLE */

  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.ipv6_valid = 0x1;
  pd_status = p4_pd_dc_l3_rewrite_table_add_with_ipv6_unicast_rewrite(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, 201, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "l3 ipv6 ucast rewrite table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

#endif /* !P4_IPV6_DISABLE */

#ifndef P4_MPLS_DISABLE
  SWITCH_MEMSET(&match_spec, 0, sizeof(match_spec));
  match_spec.mpls_0__valid = 0x1;
  pd_status = p4_pd_dc_l3_rewrite_table_add_with_mpls_rewrite(
      switch_cfg_sess_hdl, p4_pd_device, &match_spec, 300, &entry_hdl);
  if (switch_pd_log_level_debug()) {
    switch_pd_dump_entry_t pd_entry;
    SWITCH_MEMSET(&pd_entry, 0x0, sizeof(pd_entry));
    pd_entry.entry_type = SWITCH_PD_ENTRY_INIT;
    pd_entry.match_spec = (switch_uint8_t *)&match_spec;
    pd_entry.match_spec_size = sizeof(match_spec);
    pd_entry.action_spec_size = 0;
    pd_entry.pd_hdl = entry_hdl;
    switch_pd_entry_dump(device, &pd_entry);
  }

  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "l3 mpls rewrite table init failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }
#endif /* P4_MPLS_DISABLE */

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L3_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "l3 rewrite table entry init success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "l3 rewrite table entry init failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_mtu_table_entry_add(switch_device_t device,
                                              switch_mtu_id_t mtu_index,
                                              switch_mtu_t mtu,
                                              bool ipv4,
                                              switch_pd_hdl_t *entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L3_DISABLE

  p4_pd_dc_mtu_match_spec_t match_spec;
  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  SWITCH_MEMSET(&match_spec, 0, sizeof(p4_pd_dc_mtu_match_spec_t));
  match_spec.l3_metadata_mtu_index = mtu_index;

  if (ipv4) {
    p4_pd_dc_ipv4_mtu_check_action_spec_t action_spec;
    SWITCH_MEMSET(
        &action_spec, 0, sizeof(p4_pd_dc_ipv4_mtu_check_action_spec_t));
    action_spec.action_l3_mtu = mtu;
    match_spec.ipv4_valid = 1;
    match_spec.ipv6_valid = 0;
    pd_status = p4_pd_dc_mtu_table_add_with_ipv4_mtu_check(switch_cfg_sess_hdl,
                                                           p4_pd_device,
                                                           &match_spec,
                                                           &action_spec,
                                                           entry_hdl);
  } else {
#ifndef P4_IPV6_DISABLE
    p4_pd_dc_ipv6_mtu_check_action_spec_t action_spec;
    SWITCH_MEMSET(
        &action_spec, 0, sizeof(p4_pd_dc_ipv6_mtu_check_action_spec_t));
    action_spec.action_l3_mtu = mtu;
    match_spec.ipv4_valid = 0;
    match_spec.ipv6_valid = 1;
    pd_status = p4_pd_dc_mtu_table_add_with_ipv6_mtu_check(switch_cfg_sess_hdl,
                                                           p4_pd_device,
                                                           &match_spec,
                                                           &action_spec,
                                                           entry_hdl);
#endif /* P4_IPV6_DISABLE */
  }

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L3_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_mtu_table_entry_update(switch_device_t device,
                                                 switch_mtu_id_t mtu_index,
                                                 switch_mtu_t mtu,
                                                 bool ipv4,
                                                 switch_pd_hdl_t entry_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L3_DISABLE

  if (ipv4) {
    p4_pd_dc_ipv4_mtu_check_action_spec_t action_spec;
    SWITCH_MEMSET(
        &action_spec, 0, sizeof(p4_pd_dc_ipv4_mtu_check_action_spec_t));
    action_spec.action_l3_mtu = mtu;
    pd_status = p4_pd_dc_mtu_table_modify_with_ipv4_mtu_check(
        switch_cfg_sess_hdl, device, entry_hdl, &action_spec);
  } else {
#ifndef P4_IPV6_DISABLE
    p4_pd_dc_ipv6_mtu_check_action_spec_t action_spec;
    SWITCH_MEMSET(
        &action_spec, 0, sizeof(p4_pd_dc_ipv6_mtu_check_action_spec_t));
    action_spec.action_l3_mtu = mtu;
    pd_status = p4_pd_dc_mtu_table_modify_with_ipv6_mtu_check(
        switch_cfg_sess_hdl, device, entry_hdl, &action_spec);
#endif /* P4_IPV6_DISABLE */
  }

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L3_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_mtu_table_entry_delete(switch_device_t device,
                                                 switch_pd_hdl_t pd_hdl) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  SWITCH_FAST_RECONFIG(device)

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L3_DISABLE

  pd_status = p4_pd_dc_mtu_table_delete(switch_cfg_sess_hdl, device, pd_hdl);

  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L3_DISABLE */
#endif /* SWITCH_PD */

  return status;
}

switch_status_t switch_pd_mtu_table_default_entry_add(switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  UNUSED(p4_pd_device);

#ifndef P4_L3_DISABLE
  pd_status = p4_pd_dc_mtu_set_default_action_mtu_miss(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "mtu table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L3_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "mtu table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "mtu table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }
  return status;
}

switch_status_t switch_pd_l3_rewrite_table_default_entry_add(
    switch_device_t device) {
  switch_status_t status = SWITCH_STATUS_SUCCESS;
  switch_pd_status_t pd_status = SWITCH_PD_STATUS_SUCCESS;
  switch_pd_hdl_t entry_hdl = 0;

  UNUSED(device);

  UNUSED(status);
  UNUSED(pd_status);

#ifdef SWITCH_PD
#ifndef P4_L3_DISABLE

  p4_pd_dev_target_t p4_pd_device;

  p4_pd_device.device_id = device;
  p4_pd_device.dev_pipe_id = PD_DEV_PIPE_ALL;

  pd_status = p4_pd_dc_l3_rewrite_set_default_action_nop(
      switch_cfg_sess_hdl, p4_pd_device, &entry_hdl);
  if (pd_status != SWITCH_PD_STATUS_SUCCESS) {
    SWITCH_PD_LOG_ERROR(
        "l3 rewrite table default add failed "
        "on device %d : table %s action %s\n",
        device,
        switch_pd_table_id_to_string(0),
        switch_pd_action_id_to_string(0));
    goto cleanup;
  }

cleanup:

  status = switch_pd_status_to_status(pd_status);
  p4_pd_complete_operations(switch_cfg_sess_hdl);

#endif /* P4_L3_DISABLE */
#endif /* SWITCH_PD */

  if (status == SWITCH_STATUS_SUCCESS) {
    SWITCH_PD_LOG_DEBUG(
        "l3 rewrite table entry default add success "
        "on device %d 0x%lx\n",
        device,
        entry_hdl);
  } else {
    SWITCH_PD_LOG_ERROR(
        "l3 rewrite table entry default add failed "
        "on device %d : %s (pd: 0x%x)\n",
        device,
        switch_error_to_string(status),
        pd_status);
  }

  return status;
}

#ifdef __cplusplus
}
#endif
