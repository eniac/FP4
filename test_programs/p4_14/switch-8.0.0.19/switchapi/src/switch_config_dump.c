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

/* Local header includes */
#include "switch_internal.h"
#include "switch_pd.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_FEATURE(_en) (_en ? "enabled" : "disabled")

switch_status_t switch_api_config_dump_internal(const switch_device_t device,
                                                const void *cli_ctx) {
  switch_uint16_t index = 0;
  switch_api_type_t api_type = 0;
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  UNUSED(status);

  SWITCH_PRINT(cli_ctx, "\n\t\tconfig info:\n");
  SWITCH_PRINT(cli_ctx,
               "\t\t\tpcie: %s\n",
               config_info.api_switch_config.use_pcie ? "enabled" : "disabled");
  SWITCH_PRINT(
      cli_ctx, "\t\t\tswitch id: %d\n", config_info.config_params.switch_id);

  SWITCH_PRINT(cli_ctx,
               "\t\t\tmax devices: %d\n",
               config_info.api_switch_config.max_devices);
  for (index = 0; index < config_info.api_switch_config.max_devices; index++) {
    SWITCH_PRINT(cli_ctx,
                 "\t\t\t\tdevice id: %d inited %s\n\n",
                 index,
                 config_info.device_inited[index] ? "yes" : "no");
  }

  SWITCH_PRINT(cli_ctx, "\t\t\tsession handle: 0x%lx\n", switch_cfg_sess_hdl);
  SWITCH_PRINT(
      cli_ctx, "\t\t\tmc session handle: 0x%lx\n", switch_cfg_mc_sess_hdl);

  SWITCH_PRINT(cli_ctx, "\t\t\tlog level:\n");

  for (api_type = 0; api_type < SWITCH_API_TYPE_MAX; api_type++) {
    SWITCH_PRINT(
        cli_ctx,
        "\t\t\t\t%s : %s\n",
        switch_api_type_to_string(api_type),
        switch_log_level_to_string(config_info.log_ctx.log_level[api_type]));
  }

  SWITCH_PRINT(cli_ctx, "\t\t\t\n\nfeature enabled:\n");
  switch_pd_feature_t *pd_feature = switch_pd_feature_get();
  if (pd_feature) {
    SWITCH_PRINT(cli_ctx, "\t\t\tl2: %s\n", SWITCH_FEATURE(pd_feature->l2));
    SWITCH_PRINT(cli_ctx, "\t\t\tl3: %s\n", SWITCH_FEATURE(pd_feature->l3));
    SWITCH_PRINT(cli_ctx, "\t\t\tacl: %s\n", SWITCH_FEATURE(pd_feature->acl));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tingress_acl range: %s\n",
                 SWITCH_FEATURE(pd_feature->ingress_acl_range));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tegress acl range: %s\n",
                 SWITCH_FEATURE(pd_feature->egress_acl_range));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tbfd offload: %s\n",
                 SWITCH_FEATURE(pd_feature->bfd_offload));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tegress filter: %s\n",
                 SWITCH_FEATURE(pd_feature->egress_filter));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tfast failover: %s\n",
                 SWITCH_FEATURE(pd_feature->fast_failover));
    SWITCH_PRINT(cli_ctx, "\t\t\tila: %s\n", SWITCH_FEATURE(pd_feature->ila));
    SWITCH_PRINT(
        cli_ctx, "\t\t\tint ep: %s\n", SWITCH_FEATURE(pd_feature->int_ep));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tint transit: %s\n",
                 SWITCH_FEATURE(pd_feature->int_transit));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tint digest: %s\n",
                 SWITCH_FEATURE(pd_feature->int_digest));
    SWITCH_PRINT(cli_ctx, "\t\t\tipsg: %s\n", SWITCH_FEATURE(pd_feature->ipsg));
    SWITCH_PRINT(cli_ctx, "\t\t\tipv4: %s\n", SWITCH_FEATURE(pd_feature->ipv4));
    SWITCH_PRINT(cli_ctx, "\t\t\tipv6: %s\n", SWITCH_FEATURE(pd_feature->ipv6));
    SWITCH_PRINT(
        cli_ctx, "\t\t\tmc: %s\n", SWITCH_FEATURE(pd_feature->multicast));
    SWITCH_PRINT(
        cli_ctx, "\t\t\tl2 mc: %s\n", SWITCH_FEATURE(pd_feature->l2_multicast));
    SWITCH_PRINT(
        cli_ctx, "\t\t\tl3 mc: %s\n", SWITCH_FEATURE(pd_feature->l3_multicast));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\ttunnel mc: %s\n",
                 SWITCH_FEATURE(pd_feature->tunnel_multicast));
    SWITCH_PRINT(
        cli_ctx, "\t\t\tmeter: %s\n", SWITCH_FEATURE(pd_feature->meter));
    SWITCH_PRINT(
        cli_ctx, "\t\t\tmirror: %s\n", SWITCH_FEATURE(pd_feature->mirror));
    SWITCH_PRINT(cli_ctx, "\t\t\tmpls: %s\n", SWITCH_FEATURE(pd_feature->mpls));
    SWITCH_PRINT(
        cli_ctx, "\t\t\tmpls udp: %s\n", SWITCH_FEATURE(pd_feature->mpls_udp));
    SWITCH_PRINT(cli_ctx, "\t\t\tnat: %s\n", SWITCH_FEATURE(pd_feature->nat));
    SWITCH_PRINT(
        cli_ctx, "\t\t\tnvgre: %s\n", SWITCH_FEATURE(pd_feature->nvgre));
    SWITCH_PRINT(
        cli_ctx, "\t\t\tgeneve: %s\n", SWITCH_FEATURE(pd_feature->geneve));
    SWITCH_PRINT(cli_ctx, "\t\t\tqos: %s\n", SWITCH_FEATURE(pd_feature->qos));
    SWITCH_PRINT(
        cli_ctx, "\t\t\tacl qos: %s\n", SWITCH_FEATURE(pd_feature->acl_qos));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tingress qos: %s\n",
                 SWITCH_FEATURE(pd_feature->basic_ingress_qos));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tracl stats: %s\n",
                 SWITCH_FEATURE(pd_feature->racl_stats));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tegress acl: %s\n",
                 SWITCH_FEATURE(pd_feature->egress_acl));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tegress acl stats: %s\n",
                 SWITCH_FEATURE(pd_feature->egress_acl_stats));
    SWITCH_PRINT(
        cli_ctx, "\t\t\tfabric: %s\n", SWITCH_FEATURE(pd_feature->fabric));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tmirror acl stats: %s\n",
                 SWITCH_FEATURE(pd_feature->mirror_acl_stats));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tresilient hash: %s\n",
                 SWITCH_FEATURE(pd_feature->resilient_hash));
    SWITCH_PRINT(cli_ctx, "\t\t\tstp: %s\n", SWITCH_FEATURE(pd_feature->stp));
    SWITCH_PRINT(cli_ctx, "\t\t\tsr: %s\n", SWITCH_FEATURE(pd_feature->sr));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tstorm control: %s\n",
                 SWITCH_FEATURE(pd_feature->storm_control));
    SWITCH_PRINT(
        cli_ctx, "\t\t\tstats: %s\n", SWITCH_FEATURE(pd_feature->stats));
    SWITCH_PRINT(
        cli_ctx, "\t\t\ttunnel: %s\n", SWITCH_FEATURE(pd_feature->tunnel));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tipv6 tunnel: %s\n",
                 SWITCH_FEATURE(pd_feature->ipv6_tunnel));
    SWITCH_PRINT(cli_ctx, "\t\t\turpf: %s\n", SWITCH_FEATURE(pd_feature->urpf));
    SWITCH_PRINT(cli_ctx, "\t\t\twcmp: %s\n", SWITCH_FEATURE(pd_feature->wcmp));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tmirror wcmp: %s\n",
                 SWITCH_FEATURE(pd_feature->mirror_wcmp));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\ttemelemtry apx stateful: %s\n",
                 SWITCH_FEATURE(pd_feature->dtel_apx_stateful));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\ttemelemtry stateless sup: %s\n",
                 SWITCH_FEATURE(pd_feature->dtel_stateless_sup));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\ttemelemtry mirror lb: %s\n",
                 SWITCH_FEATURE(pd_feature->dtel_mirror_lb));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\ttemelemtry report: %s\n",
                 SWITCH_FEATURE(pd_feature->dtel_report));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\ttemelemtry watch: %s\n",
                 SWITCH_FEATURE(pd_feature->dtel_watch));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tingress mac acl: %s\n",
                 SWITCH_FEATURE(pd_feature->ingress_mac_acl));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tegress mac acl: %s\n",
                 SWITCH_FEATURE(pd_feature->egress_mac_acl));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\ttunnel nexthop: %s\n",
                 SWITCH_FEATURE(pd_feature->tunnel_nexthop));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\ttunnel opt: %s\n",
                 SWITCH_FEATURE(pd_feature->tunnel_opt));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tuc self fwd check disable: %s\n",
                 SWITCH_FEATURE(pd_feature->ingress_uc_self_fwd_check_disable));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\ttunnel v4 vxlan: %s\n",
                 SWITCH_FEATURE(pd_feature->tunnel_v4_vxlan));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tcopp color drop: %s\n",
                 SWITCH_FEATURE(pd_feature->copp_color_drop));
    SWITCH_PRINT(cli_ctx,
                 "\t\t\tqos metering: %s\n",
                 SWITCH_FEATURE(pd_feature->qos_metering));
  }

  return status;
}

#ifdef __cplusplus
}
#endif

switch_status_t switch_api_config_dump(const switch_device_t device,
                                       const void *cli_ctx) {
  SWITCH_MT_WRAP(switch_api_config_dump_internal(device, cli_ctx))
}
