/*
Copyright 2013-present Barefoot Networks, Inc.
*/

#ifndef _switch_bd_int_h_
#define _switch_bd_int_h_

#include "switchapi/switch_base_types.h"
#include "switchapi/switch_handle.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SWITCH_XID_SIZE 16384

#define SWITCH_AGING_INTERVAL_INVALID -1

#define SWITCH_PV_HASH_KEY_SIZE sizeof(switch_pv_key_t)

#define SWITCH_BD_INVALID 0

#define switch_bd_handle_create(_device) \
  switch_handle_create(device, SWITCH_HANDLE_TYPE_BD, sizeof(switch_bd_info_t))

#define switch_bd_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_BD, _handle)

#define switch_bd_get(_device, _handle, _info) \
  switch_handle_get(device, SWITCH_HANDLE_TYPE_BD, _handle, (void **)_info)

#define SWITCH_BD_MEMBER_INIT(_d, _b)                     \
  do {                                                    \
    if (_b) {                                             \
      SWITCH_MEMSET(_b, 0x0, sizeof(switch_bd_member_t)); \
      _b->handle = SWITCH_API_INVALID_HANDLE;             \
      _b->outer_vlan = 0;                                 \
      _b->inner_vlan = 0;                                 \
      _b->member_handle = 0;                              \
      _b->pv_tagged_entry = SWITCH_PD_INVALID_HANDLE;     \
      _b->pv_untagged_entry = SWITCH_PD_INVALID_HANDLE;   \
      _b->xlate_entry = SWITCH_PD_INVALID_HANDLE;         \
    }                                                     \
  } while (0);

#define switch_bd_member_handle_create(_device) \
  switch_handle_create(                         \
      _device, SWITCH_HANDLE_TYPE_BD_MEMBER, sizeof(switch_bd_member_t))

#define switch_bd_member_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_BD_MEMBER, _handle)

#define switch_bd_member_get(_device, _handle, _info) \
  switch_handle_get(                                  \
      _device, SWITCH_HANDLE_TYPE_BD_MEMBER, _handle, (void **)_info)

switch_status_t switch_bd_handle_get(switch_device_t device,
                                     switch_handle_t network_handle,
                                     switch_handle_t *bd_handle);

typedef enum switch_bd_type_s {
  SWITCH_BD_TYPE_NONE = 0x0,
  SWITCH_BD_TYPE_VLAN = 0x1,
  SWITCH_BD_TYPE_LN = 0x2,
  SWITCH_BD_TYPE_L3 = 0x3,
  SWITCH_BD_TYPE_VRF = 0x4
} switch_bd_type_t;

static inline char *switch_bd_type_to_string(switch_bd_type_t bd_type) {
  switch (bd_type) {
    case SWITCH_BD_TYPE_NONE:
      return "none";
    case SWITCH_BD_TYPE_VLAN:
      return "vlan";
    case SWITCH_BD_TYPE_LN:
      return "ln";
    case SWITCH_BD_TYPE_L3:
      return "l3";
    case SWITCH_BD_TYPE_VRF:
      return "vrf";
    default:
      return "unknown";
  }
}

typedef struct switch_bd_member_s {
  /** list node */
  switch_node_t node;

  /** interface handle */
  switch_handle_t handle;

  /** outer vlan */
  switch_vlan_t outer_vlan;

  /** inner vlan */
  switch_vlan_t inner_vlan;

  /** inner vlan programmed in hardware */
  switch_vlan_t pv_hw_inner_vlan;

  /** outer vlan programmed in hardware */
  switch_vlan_t pv_hw_outer_vlan;

  /** parent bd */
  switch_handle_t bd_handle;

  /** member handle - self pointer */
  switch_handle_t member_handle;

  /** multicast replication id */
  switch_rid_t rid;

  /** spanning tree state */
  switch_stp_state_t stp_state;

  /** hardware entry handles */

  /** port vlan membership untagged hw entry */
  switch_pd_hdl_t pv_untagged_entry;

  /** port vlan membership tagged hw entry */
  switch_pd_hdl_t pv_tagged_entry;

  /** vlan xlate hw entry */
  switch_pd_hdl_t xlate_entry;

  /** tunnel membership - ethernet/ipv4/ipv6 */
  switch_pd_hdl_t tunnel_hw_entry[3];

  /** egress vni hw entry */
  switch_pd_hdl_t egress_bd_hw_entry;

  /** egress outer bd hw entry */
  switch_pd_hdl_t egress_outer_bd_hw_entry;

  /** port vlan ifindex mapping hw entry */
  switch_pd_hdl_t pv_ifindex_entry;

  /** flags to indicate the hardware handles */
  switch_uint64_t hw_flags;

} switch_bd_member_t;

typedef struct switch_bd_stats_ {
  switch_id_t stats_id[SWITCH_BD_STATS_MAX];
  switch_pd_hdl_t stats_pd_hdl[SWITCH_BD_STATS_MAX];
  switch_counter_t counters[SWITCH_BD_STATS_MAX];
} switch_bd_stats_t;

typedef enum switch_bd_pd_entry_s {
  SWITCH_BD_INGRESS_PD_ENTRY = (1 << 0),
  SWITCH_BD_EGRESS_PD_ENTRY = (1 << 1),
  SWITCH_BD_UUC_FLOODING_PD_ENTRY = (1 << 2),
  SWITCH_BD_UMC_FLOODING_PD_ENTRY = (1 << 3),
  SWITCH_BD_BCAST_FLOODING_PD_ENTRY = (1 << 4),
  SWITCH_BD_MROUTERS_FLOODING_PD_ENTRY = (1 << 5),
  SWITCH_BD_EGRESS_OUTER_PD_ENTRY = (1 << 6),
} switch_bd_pd_entry_t;

typedef enum switch_bd_member_pd_entry_s {
  SWITCH_BD_MEMBER_PD_PV_TAGGED_ENTRY = (1 << 0),
  SWITCH_BD_MEMBER_PD_PV_UNTAGGED_ENTRY = (1 << 1),
  SWITCH_BD_MEMBER_PD_XLATE_ENTRY = (1 << 2),
  SWITCH_BD_MEMBER_PD_PV_IFINDEX_ENTRY = (1 << 3),
  SWITCH_BD_MEMBER_PD_MCAST_MEMBER_ENTRY = (1 << 4),
} switch_bd_member_pd_entry_t;

typedef enum switch_bd_flags_s {
  SWITCH_BD_ATTR_VRF_HANDLE = 1 << 0,
  SWITCH_BD_ATTR_RMAC_HANDLE = 1 << 1,
  SWITCH_BD_ATTR_STP_HANDLE = 1 << 2,
  SWITCH_BD_ATTR_UUC_MC_HANDLE = 1 << 4,
  SWITCH_BD_ATTR_UMC_MC_HANDLE = 1 << 5,
  SWITCH_BD_ATTR_BCAST_MC_HANDLE = 1 << 6,
  SWITCH_BD_ATTR_MROUTERS_MC_HANDLE = 1 << 7,
  SWITCH_BD_ATTR_IPV4_UNICAST = 1 << 8,
  SWITCH_BD_ATTR_IPV6_UNICAST = 1 << 9,
  SWITCH_BD_ATTR_IPV4_MULTICAST = 1 << 10,
  SWITCH_BD_ATTR_IPV6_MULTICAST = 1 << 11,
  SWITCH_BD_ATTR_LEARNING = 1 << 12,
  SWITCH_BD_ATTR_UUC_FLOODING_ENABLED = 1 << 13,
  SWITCH_BD_ATTR_UMC_FLOODING_ENABLED = 1 << 14,
  SWITCH_BD_ATTR_BCAST_FLOODING_ENABLED = 1 << 15,
  SWITCH_BD_ATTR_MROUTERS_FLOODING_ENABLED = 1 << 16,
  SWITCH_BD_ATTR_IGMP_SNOOPING = 1 << 17,
  SWITCH_BD_ATTR_MLD_SNOOPING = 1 << 18,
  SWITCH_BD_ATTR_AGING_INTERVAL = 1 << 19,
  SWITCH_BD_ATTR_IPV4_URPF_MODE = 1 << 20,
  SWITCH_BD_ATTR_IPV6_URPF_MODE = 1 << 21,
  SWITCH_BD_ATTR_MRPF_GROUP = 1 << 22,
  SWITCH_BD_ATTR_MTU_HANDLE = 1 << 23,
  SWITCH_BD_ATTR_NAT_MODE = 1 << 24,
  SWITCH_BD_ATTR_INGRESS_LABEL = 1 << 25,
  SWITCH_BD_ATTR_TYPE = 1 << 26,
  SWITCH_BD_ATTR_EGRESS_LABEL = 1 << 27,
} switch_bd_flags_t;

typedef struct switch_bd_info_s {
  switch_uint64_t bd_flags;
  switch_bd_type_t bd_type;

  switch_handle_t vrf_handle;
  switch_handle_t stp_handle;
  switch_handle_t rmac_handle;
  switch_handle_t handle;

  switch_handle_t flood_handle;
  switch_handle_t mrouters_mc_handle;

  switch_vni_t tunnel_vni;
  switch_vlan_t vlan;

  bool ipv4_unicast;
  bool ipv6_unicast;
  bool ipv4_multicast;
  bool ipv6_multicast;
  bool igmp_snooping;
  bool mld_snooping;
  bool stats_enabled;
  bool learning;
  bool flooding_enabled;

  switch_int32_t aging_interval;

  switch_urpf_mode_t ipv4_urpf_mode;
  switch_urpf_mode_t ipv6_urpf_mode;

  switch_mrpf_group_t mrpf_group;

  switch_xid_t xid;
  switch_rid_t rid;
  switch_id_t smac_index;
  switch_id_t tunnel_smac_index;
  switch_list_t members;

  /** ingress acl bd label */
  switch_bd_label_t ingress_bd_label;

  /** ingress acl bd label */
  switch_bd_label_t egress_bd_label;

  switch_bd_stats_t *bd_stats;
  switch_pd_mbr_hdl_t bd_entry;
  switch_pd_hdl_t egress_bd_entry;
  switch_pd_hdl_t egress_outer_bd_entry;
  switch_pd_hdl_t uuc_entry;
  switch_pd_hdl_t umc_entry;
  switch_pd_hdl_t bcast_entry;
  switch_pd_hdl_t mrouters_entry;
  switch_pd_hdl_t cpu_entry;
  switch_pd_hdl_t cpu_tx_entry;
  switch_nat_mode_t nat_mode;

  switch_array_t mac_array;

  switch_handle_t mtu_handle;

  switch_uint64_t hw_flags;

  /** ingress acl group handle */
  switch_handle_t ingress_acl_group_handle;

  /** egress acl group handle */
  switch_handle_t egress_acl_group_handle;

} switch_bd_info_t;

typedef struct switch_bd_context_ {
  switch_id_allocator_t *xid_allocator;
  switch_id_allocator_t *stats_id_allocator;
  switch_handle_t vlan_bd_handle[SWITCH_MAX_VLANS + 1];
} switch_bd_context_t;

#define SWITCH_BD_STATS_START_INDEX(ln) \
  (ln->bd_stats != NULL) ? ln->bd_stats->stats_id[0] : 0

switch_status_t switch_bd_init(switch_device_t device);

switch_status_t switch_bd_free(switch_device_t device);

switch_status_t switch_bd_default_entries_add(switch_device_t device);

switch_status_t switch_bd_default_entries_delete(switch_device_t device);

switch_status_t switch_bd_create(switch_device_t device,
                                 switch_uint64_t bd_flags,
                                 switch_bd_info_t *bd_info,
                                 switch_handle_t *bd_handle);

switch_status_t switch_bd_update(switch_device_t device,
                                 switch_handle_t bd_handle,
                                 switch_uint64_t bd_flags,
                                 switch_bd_info_t *bd_info);

switch_status_t switch_bd_attribute_set(switch_device_t device,
                                        switch_handle_t bd_handle,
                                        switch_uint64_t bd_flags,
                                        switch_bd_info_t *bd_info);

switch_status_t switch_bd_attribute_get(switch_device_t device,
                                        switch_handle_t bd_handle,
                                        switch_uint64_t bd_flags,
                                        switch_bd_info_t *bd_info);

switch_status_t switch_bd_delete(switch_device_t device,
                                 switch_handle_t bd_handle);

switch_status_t switch_bd_member_add(switch_device_t device,
                                     switch_handle_t bd_handle,
                                     switch_handle_t *member_handle);

switch_status_t switch_bd_member_delete(switch_device_t device,
                                        switch_handle_t bd_handle,
                                        switch_handle_t member_handle);

switch_status_t switch_bd_member_find(switch_device_t device,
                                      switch_handle_t bd_handle,
                                      switch_handle_t intf_handle,
                                      switch_bd_member_t **member);

switch_status_t switch_bd_learning_set(switch_device_t device,
                                       switch_handle_t bd_handle,
                                       bool enable);

switch_status_t switch_bd_learning_get(switch_device_t device,
                                       switch_handle_t bd_handle,
                                       bool *enable);

switch_status_t switch_bd_igmp_snooping_set(switch_device_t device,
                                            switch_handle_t vlan_handle,
                                            bool enable);

switch_status_t switch_bd_igmp_snooping_get(switch_device_t device,
                                            switch_handle_t vlan_handle,
                                            bool *enable);

switch_status_t switch_bd_mld_snooping_set(switch_device_t device,
                                           switch_handle_t vlan_handle,
                                           bool enable);

switch_status_t switch_bd_mld_snooping_get(switch_device_t device,
                                           switch_handle_t vlan_handle,
                                           bool *enable);

switch_status_t switch_bd_aging_interval_set(switch_device_t device,
                                             switch_handle_t bd_handle,
                                             switch_int32_t aging_interval);

switch_status_t switch_bd_aging_interval_get(switch_device_t device,
                                             switch_handle_t bd_handle,
                                             switch_int32_t *aging_interval);
;

switch_status_t switch_bd_rewrite_smac_index_get(switch_device_t device,
                                                 switch_handle_t bd_handle,
                                                 switch_id_t *smac_index);

switch_status_t switch_bd_handle_dump(const switch_device_t device,
                                      const switch_handle_t bd_handle,
                                      const void *cli_ctx);

switch_status_t switch_bd_member_handle_dump(
    const switch_device_t device,
    const switch_handle_t member_handle,
    const void *cli_ctx);

switch_status_t switch_bd_stats_get(const switch_device_t device,
                                    const switch_handle_t bd_handle,
                                    const switch_uint8_t count,
                                    const switch_bd_counter_id_t *counter_ids,
                                    switch_counter_t *counters);

switch_status_t switch_bd_stats_enable(switch_device_t device,
                                       switch_handle_t bd_handle);

switch_status_t switch_bd_stats_disable(switch_device_t device,
                                        switch_handle_t bd_handle);

switch_status_t switch_bd_stp_handle_set(switch_device_t device,
                                         switch_handle_t bd_handle,
                                         switch_handle_t stp_handle);

switch_status_t switch_bd_stp_handle_get(switch_device_t device,
                                         switch_handle_t bd_handle,
                                         switch_handle_t *stp_handle);

switch_status_t switch_bd_mrpf_group_set(switch_device_t device,
                                         switch_handle_t bd_handle,
                                         switch_mrpf_group_t mrpf_group);

switch_status_t switch_bd_mrpf_group_get(switch_device_t device,
                                         switch_handle_t bd_handle,
                                         switch_mrpf_group_t *mrpf_group);

switch_status_t switch_bd_member_stp_state_set(switch_device_t device,
                                               switch_handle_t network_handle,
                                               switch_status_t intf_handle,
                                               switch_stp_state_t stp_state);

switch_status_t switch_bd_mrouters_handle_set(switch_device_t device,
                                              switch_handle_t bd_handle,
                                              switch_handle_t mrouters_handle);

switch_status_t switch_bd_mrouters_handle_get(switch_device_t device,
                                              switch_handle_t bd_handle,
                                              switch_handle_t *mrouters_handle);

switch_status_t switch_bd_rmac_handle_set(switch_device_t device,
                                          switch_handle_t bd_handle,
                                          switch_handle_t rmac_handle);

switch_status_t switch_bd_rmac_handle_get(switch_device_t device,
                                          switch_handle_t bd_handle,
                                          switch_handle_t *rmac_handle);

switch_status_t switch_bd_vrf_handle_set(switch_device_t device,
                                         switch_handle_t bd_handle,
                                         switch_handle_t vrf_handle);

switch_status_t switch_bd_vrf_handle_get(switch_device_t device,
                                         switch_handle_t bd_handle,
                                         switch_handle_t *vrf_handle);

switch_status_t switch_bd_stats_clear(switch_device_t device,
                                      switch_handle_t bd_handle);

#ifdef __cplusplus
}
#endif

#endif
