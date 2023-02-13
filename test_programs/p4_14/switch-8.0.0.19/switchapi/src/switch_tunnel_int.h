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

#ifndef __SWITCH_TUNNEL_INT_H__
#define __SWITCH_TUNNEL_INT_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** tunnel mapper handle wrappers */
#define switch_tunnel_mapper_handle_create(_device)      \
  switch_handle_create(_device,                          \
                       SWITCH_HANDLE_TYPE_TUNNEL_MAPPER, \
                       sizeof(switch_tunnel_mapper_info_t))

#define switch_tunnel_mapper_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_TUNNEL_MAPPER, _handle)

#define switch_tunnel_mapper_get(_device, _handle, _info) \
  switch_handle_get(                                      \
      _device, SWITCH_HANDLE_TYPE_TUNNEL_MAPPER, _handle, (void **)_info)

/** tunnel mapper entry handle wrappers */
#define switch_tunnel_mapper_entry_handle_create(_device)      \
  switch_handle_create(_device,                                \
                       SWITCH_HANDLE_TYPE_TUNNEL_MAPPER_ENTRY, \
                       sizeof(switch_tunnel_mapper_entry_info_t))

#define switch_tunnel_mapper_entry_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_TUNNEL_MAPPER_ENTRY, _handle)

#define switch_tunnel_mapper_entry_get(_device, _handle, _info) \
  switch_handle_get(_device,                                    \
                    SWITCH_HANDLE_TYPE_TUNNEL_MAPPER_ENTRY,     \
                    _handle,                                    \
                    (void **)_info)

/** tunnel handle wrappers */
#define switch_tunnel_handle_create(_device) \
  switch_handle_create(                      \
      _device, SWITCH_HANDLE_TYPE_TUNNEL, sizeof(switch_tunnel_info_t))

#define switch_tunnel_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_TUNNEL, _handle)

#define switch_tunnel_get(_device, _handle, _info) \
  switch_handle_get(_device, SWITCH_HANDLE_TYPE_TUNNEL, _handle, (void **)_info)

/** tunnel term handle wrappers */
#define switch_tunnel_term_handle_create(_device)      \
  switch_handle_create(_device,                        \
                       SWITCH_HANDLE_TYPE_TUNNEL_TERM, \
                       sizeof(switch_tunnel_term_info_t))

#define switch_tunnel_term_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_TUNNEL_TERM, _handle)

#define switch_tunnel_term_get(_device, _handle, _info) \
  switch_handle_get(                                    \
      _device, SWITCH_HANDLE_TYPE_TUNNEL_TERM, _handle, (void **)_info)

/** tunnel encap handle wrappers */
#define switch_tunnel_encap_handle_create(_device)      \
  switch_handle_create(_device,                         \
                       SWITCH_HANDLE_TYPE_TUNNEL_ENCAP, \
                       sizeof(switch_tunnel_encap_info_t))

#define switch_tunnel_encap_handle_delete(_device, _handle) \
  switch_handle_delete(_device, SWITCH_HANDLE_TYPE_TUNNEL_ENCAP, _handle)

#define switch_tunnel_encap_get(_device, _handle, _info) \
  switch_handle_get(                                     \
      _device, SWITCH_HANDLE_TYPE_TUNNEL_ENCAP, _handle, (void **)_info)

/** tunnel ip hash key size */
#define SWITCH_TUNNEL_IP_HASH_KEY_SIZE sizeof(switch_tunnel_ip_key_t)

/** tunnel ip hash random seed */
#define SWITCH_TUNNEL_IP_HASH_SEED 0x12345678

/** tunnel vtep hash key size */
#define SWITCH_TUNNEL_VTEP_HASH_KEY_SIZE sizeof(switch_tunnel_vtep_key_t)

/** tunnel vtep hash random seed */
#define SWITCH_TUNNEL_VTEP_HASH_SEED 0x97568901

/** tunnel vni allocator size */
#define SWITCH_TUNNEL_VNI_ALLOCATOR_SIZE (1 << 16)

/** tunnel vni offset */
#define SWITCH_TUNNEL_VNI_OFFSET 4096

/** tunnel table ingress hash key size */
#define SWITCH_TUNNEL_INGRESS_VNI_HASH_KEY_SIZE \
  sizeof(switch_tunnel_vni_ingress_key_t)

/** tunnel table ingress vni hash random seed */
#define SWITCH_TUNNEL_INGRESS_VNI_HASH_SEED 0x12345678

/** tunnel table egress hash key size */
#define SWITCH_TUNNEL_EGRESS_VNI_HASH_KEY_SIZE \
  sizeof(switch_tunnel_vni_egress_key_t)

/** tunnel table egress vni hash random seed */
#define SWITCH_TUNNEL_EGRESS_VNI_HASH_SEED 0x12345678

/** tunnel encap handle size */
#define SWITCH_TUNNEL_ENCAP_HANDLE_SIZE 16384

static inline char *switch_tunnel_type_to_string(switch_tunnel_type_t type) {
  switch (type) {
    case SWITCH_TUNNEL_TYPE_QINQ:
      return "qinq";
    case SWITCH_TUNNEL_TYPE_VXLAN:
      return "vxlan";
    case SWITCH_TUNNEL_TYPE_GRE:
      return "gre";
    case SWITCH_TUNNEL_TYPE_NVGRE:
      return "nvgre";
    case SWITCH_TUNNEL_TYPE_GENEVE:
      return "geneve";
    case SWITCH_TUNNEL_TYPE_ERSPAN_T3:
      return "erspan t3";
    case SWITCH_TUNNEL_TYPE_IPIP:
      return "ipinip";
    case SWITCH_TUNNEL_TYPE_SRV6:
      return "srv6";
    default:
      return "none";
  }
}

static inline char *switch_tunnel_map_type_to_string(
    switch_tunnel_map_type_t tunnel_map_type) {
  switch (tunnel_map_type) {
    case SWITCH_TUNNEL_MAP_TYPE_VNI_TO_VLAN_HANDLE:
      return "vni to vlan";
    case SWITCH_TUNNEL_MAP_TYPE_VLAN_HANDLE_TO_VNI:
      return "vlan to vni";
    case SWITCH_TUNNEL_MAP_TYPE_VNI_TO_LN_HANDLE:
      return "vni to ln";
    case SWITCH_TUNNEL_MAP_TYPE_LN_HANDLE_TO_VNI:
      return "ln to vni";
    case SWITCH_TUNNEL_MAP_TYPE_VRF_HANDLE_TO_VNI:
      return "vrf to vni";
    case SWITCH_TUNNEL_MAP_TYPE_VNI_TO_VRF_HANDLE:
      return "vni to vrf";
    default:
      return "unknown";
  }
}

typedef enum switch_tunnel_pd_type_s {
  SWITCH_TUNNEL_PD_TYPE_IP = 0x0,
  SWITCH_TUNNEL_PD_TYPE_NON_IP = 0x1,
} switch_tunnel_pd_type_t;

/** hardware tunnel ingress type */
typedef enum switch_tunnel_type_ingress_s {
  SWITCH_TUNNEL_TYPE_INGRESS_NONE = 0,
  SWITCH_TUNNEL_TYPE_INGRESS_VXLAN = 1,
  SWITCH_TUNNEL_TYPE_INGRESS_GRE = 2,
  SWITCH_TUNNEL_TYPE_INGRESS_IPIP = 3,
  SWITCH_TUNNEL_TYPE_INGRESS_GENEVE = 4,
  SWITCH_TUNNEL_TYPE_INGRESS_NVGRE = 5,
  SWITCH_TUNNEL_TYPE_INGRESS_VXLAN_GPE = 12,
  SWITCH_TUNNEL_TYPE_INGRESS_SRV6 = 14,
} switch_tunnel_type_ingress_t;

/** hardware tunnel egress type */
typedef enum switch_tunnel_type_egress_s {
  SWITCH_TUNNEL_TYPE_EGRESS_NONE = 0,
  SWITCH_TUNNEL_TYPE_EGRESS_IPV4_VXLAN = 1,
  SWITCH_TUNNEL_TYPE_EGRESS_IPV6_VXLAN = 2,
  SWITCH_TUNNEL_TYPE_EGRESS_IPV4_GENEVE = 3,
  SWITCH_TUNNEL_TYPE_EGRESS_IPV6_GENEVE = 4,
  SWITCH_TUNNEL_TYPE_EGRESS_IPV4_NVGRE = 5,
  SWITCH_TUNNEL_TYPE_EGRESS_IPV6_NVGRE = 6,
  SWITCH_TUNNEL_TYPE_EGRESS_IPV4_ERSPAN_T3 = 7,
  SWITCH_TUNNEL_TYPE_EGRESS_IPV6_ERSPAN_T3 = 8,
  SWITCH_TUNNEL_TYPE_EGRESS_IPV4_GRE = 9,
  SWITCH_TUNNEL_TYPE_EGRESS_IPV6_GRE = 10,
  SWITCH_TUNNEL_TYPE_EGRESS_IPV4_IP = 11,
  SWITCH_TUNNEL_TYPE_EGRESS_IPV6_IP = 12,
  SWITCH_TUNNEL_TYPE_EGRESS_FABRIC = 15,
  SWITCH_TUNNEL_TYPE_EGRESS_CPU = 16,
  SWITCH_TUNNEL_TYPE_EGRESS_IPV4_VXLAN_GPE = 17,
  SWITCH_TUNNEL_TYPE_EGRESS_IPV4_DTEL_REPORT = 18,
  SWITCH_TUNNEL_TYPE_EGRESS_IPV6_DTEL_REPORT = 19,
  SWITCH_TUNNEL_TYPE_EGRESS_SRV6 = 24,
} switch_tunnel_type_egress_t;

static inline char *switch_tunnel_ingress_type_to_string(
    switch_tunnel_type_ingress_t tunnel_type) {
  switch (tunnel_type) {
    case SWITCH_TUNNEL_TYPE_INGRESS_NONE:
      return "none";
    case SWITCH_TUNNEL_TYPE_INGRESS_VXLAN:
      return "vxlan";
    case SWITCH_TUNNEL_TYPE_INGRESS_GRE:
      return "gre";
    case SWITCH_TUNNEL_TYPE_INGRESS_IPIP:
      return "ipip";
    case SWITCH_TUNNEL_TYPE_INGRESS_GENEVE:
      return "geneve";
    case SWITCH_TUNNEL_TYPE_INGRESS_NVGRE:
      return "nvgre";
    case SWITCH_TUNNEL_TYPE_INGRESS_VXLAN_GPE:
      return "vxlan gpe";
    case SWITCH_TUNNEL_TYPE_INGRESS_SRV6:
      return "srv6";
    default:
      return "unknown";
  }
}

static inline char *switch_tunnel_egress_type_to_string(
    switch_tunnel_type_egress_t tunnel_type) {
  switch (tunnel_type) {
    case SWITCH_TUNNEL_TYPE_EGRESS_NONE:
      return "none";
    case SWITCH_TUNNEL_TYPE_EGRESS_IPV4_VXLAN:
      return "ipv4 vxlan";
    case SWITCH_TUNNEL_TYPE_EGRESS_IPV6_VXLAN:
      return "ipv6 vxlan";
    case SWITCH_TUNNEL_TYPE_EGRESS_IPV4_GENEVE:
      return "ipv4 geneve";
    case SWITCH_TUNNEL_TYPE_EGRESS_IPV6_GENEVE:
      return "ipv6 geneve";
    case SWITCH_TUNNEL_TYPE_EGRESS_IPV4_NVGRE:
      return "ipv4 nvgre";
    case SWITCH_TUNNEL_TYPE_EGRESS_IPV6_NVGRE:
      return "ipv6 nvgre";
    case SWITCH_TUNNEL_TYPE_EGRESS_IPV4_ERSPAN_T3:
      return "ipv4 erspan t3";
    case SWITCH_TUNNEL_TYPE_EGRESS_IPV6_ERSPAN_T3:
      return "ipv6 erspan t3";
    case SWITCH_TUNNEL_TYPE_EGRESS_IPV4_GRE:
      return "ipv4 gre";
    case SWITCH_TUNNEL_TYPE_EGRESS_IPV6_GRE:
      return "ipv6 gre";
    case SWITCH_TUNNEL_TYPE_EGRESS_IPV4_IP:
      return "ipv4 ip";
    case SWITCH_TUNNEL_TYPE_EGRESS_IPV6_IP:
      return "ipv6 ip";
    case SWITCH_TUNNEL_TYPE_EGRESS_FABRIC:
      return "fabric";
    case SWITCH_TUNNEL_TYPE_EGRESS_CPU:
      return "cpu";
    case SWITCH_TUNNEL_TYPE_EGRESS_IPV4_VXLAN_GPE:
      return "ipv4 vxlan gpe";
    case SWITCH_TUNNEL_TYPE_EGRESS_IPV4_DTEL_REPORT:
      return "ipv4 telemetry report";
    case SWITCH_TUNNEL_TYPE_EGRESS_IPV6_DTEL_REPORT:
      return "ipv6 telemetry report";
    case SWITCH_TUNNEL_TYPE_EGRESS_SRV6:
      return "srv6";
    default:
      return "unknown";
  }
}

/** tunnel mapper info */
typedef struct switch_tunnel_mapper_entry_info_s {
  /** tunnel mapper to map logical network to vni */
  switch_api_tunnel_mapper_entry_t api_tunnel_mapper_entry;

  /** reference count */
  switch_uint16_t ref_count;

} switch_tunnel_mapper_entry_info_t;

/** tunnel mapper list identified by tunnel mapper handle */
typedef struct switch_tunnel_mapper_info_s {
  /** array of tunnel mappers */
  switch_array_t mapper_array;

  /** tunnel map type */
  switch_tunnel_map_type_t tunnel_map_type;

} switch_tunnel_mapper_info_t;

typedef struct switch_tunnel_info_s {
  /** array of tunnel term objects */
  switch_array_t tunnel_term_array;

  /** api tunnel info */
  switch_api_tunnel_info_t api_tunnel_info;

  /** underlay vrf handle */
  switch_handle_t underlay_vrf_handle;

  /** tunnel type */
  switch_tunnel_type_t tunnel_type;

  /** tunnel source ip rewrite index */
  switch_id_t sip_index;

  /** tunnel vni for IPinIP tunnel */
  switch_vni_t tunnel_vni;

  /** interface handle */
  switch_handle_t intf_handle;

  /** encap source ip pd handle */
  switch_pd_hdl_t src_ip_rewrite_pd_hdl;

  /** ingress tunnel type */
  switch_tunnel_type_ingress_t ingress_tunnel_type;

  /** egress tunnel type */
  switch_tunnel_type_egress_t egress_tunnel_type;

  /** tunnel table hw entry - for ipip/gre */
  switch_pd_hdl_t ingress_tunnel_hw_entry[3];

} switch_tunnel_info_t;

/** tunnel termination object */
typedef struct switch_tunnel_term_info_s {
  /** api tunnel term info */
  switch_api_tunnel_term_info_t api_tunnel_term_info;

  /** dst_vtep pd_handle */
  switch_pd_hdl_t dst_vtep_pd_hdl;

  /** src_vtep pd_handle */
  switch_pd_hdl_t src_vtep_pd_hdl;
} switch_tunnel_term_info_t;

/** tunnel mgid info */
typedef struct switch_tunnel_mgid_info_s {
  /** mgid state for the tunnel */
  switch_mgid_state_t mgid_state;

  /** tunnel handle - self reference */
  switch_handle_t tunnel_encap_handle;

  /** nexthop handle for the tunnel destination */
  switch_handle_t nhop_handle;

  /** route handle for the tunnel destination */
  switch_handle_t route_handle;
} switch_tunnel_mgid_info_t;

/** tunnel encapsulation object */
typedef struct switch_tunnel_encap_info_s {
  /** nexthop handle */
  switch_handle_t nhop_handle;

  /** tunnel handle */
  switch_handle_t tunnel_handle;

  /** underlay vrf handle */
  switch_handle_t vrf_handle;

  /** tunnel mgid info */
  switch_tunnel_mgid_info_t mgid_info;

  /** tunnel dst ip */
  switch_ip_addr_t dst_ip;

  /** list of mirror objects */
  Pvoid_t PJLarr_mirrors;

  /** tunnel destination ip rewrite index */
  switch_id_t tunnel_dip_index;

  switch_uint32_t num_mirrors;

  switch_pd_hdl_t tunnel_mgid_hw_entry;

} switch_tunnel_encap_info_t;

#define SWITCH_TUNNEL_VNI_SIZE 24

#define SWITCH_TUNNEL_VNI_VALID(_vni) \
  (_vni < ((1 << SWITCH_TUNNEL_VNI_SIZE) - 1))

#define SWITCH_TUNNEL_TYPE_IP(_t_info)                     \
  (_t_info->tunnel_type == SWITCH_TUNNEL_TYPE_IPIP ||      \
   _t_info->tunnel_type == SWITCH_TUNNEL_TYPE_SRV6 ||      \
   _t_info->tunnel_type == SWITCH_TUNNEL_TYPE_GRE ||       \
   _t_info->tunnel_type == SWITCH_TUNNEL_TYPE_NVGRE ||     \
   _t_info->tunnel_type == SWITCH_TUNNEL_TYPE_GENEVE ||    \
   _t_info->tunnel_type == SWITCH_TUNNEL_TYPE_VXLAN ||     \
   _t_info->tunnel_type == SWITCH_TUNNEL_TYPE_ERSPAN_T3 || \
   _t_info->tunnel_type == SWITCH_TUNNEL_TYPE_DTEL_REPORT)

#define SWITCH_TUNNEL_USING_MGID(_t_info)                  \
  (_t_info->tunnel_type == SWITCH_TUNNEL_TYPE_IPIP ||      \
   _t_info->tunnel_type == SWITCH_TUNNEL_TYPE_GRE ||       \
   _t_info->tunnel_type == SWITCH_TUNNEL_TYPE_NVGRE ||     \
   _t_info->tunnel_type == SWITCH_TUNNEL_TYPE_GENEVE ||    \
   _t_info->tunnel_type == SWITCH_TUNNEL_TYPE_VXLAN ||     \
   _t_info->tunnel_type == SWITCH_TUNNEL_TYPE_SRV6 ||      \
   _t_info->tunnel_type == SWITCH_TUNNEL_TYPE_ERSPAN_T3 || \
   _t_info->tunnel_type == SWITCH_TUNNEL_TYPE_DTEL_REPORT)

#define SWITCH_TUNNEL_MAPPER_HANDLE_SIZE 4096

#define SWITCH_TUNNEL_HANDLE_SIZE 16384

/** tunnel vtep type - source/destination */
typedef enum switch_tunnel_ip_type_s {
  SWITCH_TUNNEL_IP_TYPE_SRC = 0,
  SWITCH_TUNNEL_IP_TYPE_DST = 1
} switch_tunnel_ip_type_t;

/** tunnel vtep hashtable entry */
typedef struct __attribute__((__packed__)) switch_tunnel_ip_key_s {
  /** tunnel vtep type */
  switch_tunnel_ip_type_t ip_type;

  /** vtep ip address */
  switch_ip_addr_t ip_addr;

} switch_tunnel_ip_key_t;

/** vtep hashtable entry */
typedef struct switch_tunnel_ip_entry_s {
  /**
   * vtep table key
   * ip key struct should be on top
   * for vtep hashtable
   */
  switch_tunnel_ip_key_t ip_key;

  /** hashtable node */
  switch_hashnode_t node;

  /** source/destination ip rewrite index */
  switch_id_t ip_id;

  /** reference count */
  switch_uint16_t ref_count;

  /** rewrite hardware handle */
  switch_pd_hdl_t rw_hw_entry;

} switch_tunnel_ip_entry_t;

typedef struct __attribute__((__packed__)) switch_tunnel_vtep_key_s {
  /** tunnel ip type */
  switch_tunnel_ip_type_t ip_type;

  /** tunnel type */
  switch_tunnel_type_t tunnel_type;

  /** vrf handle */
  switch_handle_t vrf_handle;

  /** ip address */
  switch_ip_addr_t ip_addr;

} switch_tunnel_vtep_key_t;

typedef struct switch_tunnel_vtep_info_s {
  /** ref count */
  switch_uint16_t ref_count;

  /** hashtable node */
  switch_hashnode_t node;

} switch_tunnel_vtep_info_t;

/** ingress tunnel vni table hash key */
typedef struct __attribute__((__packed__)) switch_tunnel_vni_ingress_key_s {
  /** tunnel vni */
  switch_vni_t tunnel_vni;

} switch_tunnel_vni_ingress_key_t;

/** ingress tunnel vni hashtable entry */
typedef struct switch_tunnel_vni_ingress_entry_s {
  /**
   * ingress vni key
   * this struct should be on top for ingress vni hashtable
   */
  switch_tunnel_vni_ingress_key_t vni_key;

  /** reference count */
  switch_uint16_t ref_count;

  /** hashtable node */
  switch_hashnode_t node;

  /** bridge domain handle */
  switch_handle_t bd_handle;

  /** tunnel table hw entry */
  switch_pd_hdl_t tunnel_hw_entry[3];

} switch_tunnel_vni_ingress_entry_t;

/** egress tunnel vni table hash key */
typedef struct __attribute__((__packed__)) switch_tunnel_vni_egress_key_s {
  /** bridge domain handle */
  switch_handle_t bd_handle;

} switch_tunnel_vni_egress_key_t;

/** egress tunnel vni hashtable entry */
typedef struct switch_tunnel_vni_egress_entry_s {
  /**
   * egress vni key
   * this struct should be on top for egress vni hashtable
   */
  switch_tunnel_vni_egress_key_t vni_key;

  /** reference count */
  switch_uint16_t ref_count;

  /** hashtable node */
  switch_hashnode_t node;

  /** tunnel vni */
  switch_vni_t tunnel_vni;

  /** egress vni hw entry */
  switch_pd_hdl_t tunnel_hw_entry;

} switch_tunnel_vni_egress_entry_t;

/** tunnel device context */
typedef struct switch_tunnel_context_s {
  /** source ip rewrite hashtable */
  switch_hashtable_t src_ip_hashtable;

  /** destionation ip rewrite haashtable */
  switch_hashtable_t dst_ip_hashtable;

  /**
   * ingress tunnel vni hashtable
   * tunnel type and vni uniquely identifies a
   * bridge domain
   */
  switch_hashtable_t ingress_tunnel_vni_hashtable;

  /**
   * egress tunnel vni hashtable
   * tunnel type and bd uniquely identifies a vni
   */
  switch_hashtable_t egress_tunnel_vni_hashtable;

  /** source vtep hashtable */
  switch_hashtable_t src_vtep_hashtable;

  /** destination vtep hashtable */
  switch_hashtable_t dst_vtep_hashtable;

  /** source ip rewrite index allocator */
  switch_id_allocator_t *src_ip_id_allocator;

  /** destination ip rewrite index allocator */
  switch_id_allocator_t *dst_ip_id_allocator;

  /** tunnel vni allocator */
  switch_id_allocator_t *tunnel_vni_allocator;

  /** mpls transit array indexed by label */
  switch_array_t *mpls_transit_array;

  /** data structure to store tunnel destination IPs */
  Pvoid_t PJLarr_tunnel_dest;

} switch_tunnel_context_t;

switch_status_t switch_tunnel_init(switch_device_t device);

switch_status_t switch_tunnel_free(switch_device_t device);

switch_status_t switch_tunnel_default_entries_add(switch_device_t device);

switch_status_t switch_tunnel_default_entries_delete(switch_device_t device);

switch_status_t switch_tunnel_ip_index_get(switch_device_t device,
                                           switch_tunnel_ip_key_t *ip_key,
                                           switch_id_t *ip_id);

switch_status_t switch_tunnel_member_add(switch_device_t device,
                                         switch_direction_t direction,
                                         switch_handle_t bd_handle,
                                         switch_handle_t intf_handle,
                                         switch_uint64_t flags);

switch_status_t switch_tunnel_member_delete(switch_device_t device,
                                            switch_direction_t direction,
                                            switch_handle_t bd_handle,
                                            switch_handle_t intf_handle,
                                            switch_uint64_t flags);

switch_status_t switch_api_tunnel_mgid_state_dont_handle(
    switch_device_t device,
    void *info,
    switch_tunnel_mgid_events_t event,
    void *event_arg);

switch_status_t switch_api_tunnel_mgid_state_init(
    switch_device_t device,
    void *info,
    switch_tunnel_mgid_events_t event,
    void *event_arg);

switch_status_t switch_api_tunnel_mgid_state_no_mgid(
    switch_device_t device,
    void *info,
    switch_tunnel_mgid_events_t event,
    void *event_arg);

switch_status_t switch_api_tunnel_mgid_state_mgid_associated(
    switch_device_t device,
    void *info,
    switch_tunnel_mgid_events_t event,
    void *event_arg);

switch_status_t switch_api_tunnel_send_mgid_event(
    switch_device_t device,
    switch_handle_t tunnel_handle,
    switch_tunnel_mgid_events_t event,
    void *event_arg);

switch_status_t switch_api_tunnel_dest_list_get(
    switch_device_t device,
    switch_handle_t **tunnel_handle_list,
    switch_uint32_t *tunnel_handle_count,
    switch_handle_t route_handle);

switch_status_t switch_api_tunnel_handle_dump(
    const switch_device_t device,
    const switch_handle_t tunnel_handle,
    const void *cli_ctx);

switch_status_t switch_api_tunnel_term_handle_dump(
    const switch_device_t device,
    const switch_handle_t tunnel_handle,
    const void *cli_ctx);

switch_status_t switch_api_tunnel_mapper_handle_dump(
    const switch_device_t device,
    const switch_handle_t tunnel_handle,
    const void *cli_ctx);

switch_status_t switch_api_tunnel_mapper_entry_handle_dump(
    const switch_device_t device,
    const switch_handle_t tunnel_handle,
    const void *cli_ctx);

switch_status_t switch_api_tunnel_encap_handle_dump(
    const switch_device_t device,
    const switch_handle_t tunnel_encap_handle,
    const void *cli_ctx);

switch_status_t switch_api_tunnel_encap_create(
    switch_device_t device,
    switch_handle_t nhop_handle,
    switch_handle_t *tunnel_encap_handle);

switch_status_t switch_api_tunnel_encap_delete(
    switch_device_t device, switch_handle_t tunnel_encap_handle);

switch_status_t switch_api_tunnel_mirror_list_add(
    switch_device_t device,
    switch_handle_t tunnel_encap_handle,
    switch_handle_t mirror_handle);

switch_status_t switch_api_tunnel_mirror_list_remove(
    switch_device_t device,
    switch_handle_t tunnel_encap_handle,
    switch_handle_t mirror_handle);

switch_status_t switch_tunnel_underlay_vrf_handle_get(
    switch_device_t device,
    switch_handle_t tunnel_handle,
    switch_handle_t *vrf_handle);

switch_status_t switch_tunnel_type_egress_get(
    switch_tunnel_type_t tunnel_type,
    bool ipv4,
    switch_tunnel_type_egress_t *egress_tunnel_type);

switch_status_t switch_tunnel_type_ingress_get(
    switch_tunnel_type_t tunnel_type,
    bool ipv4,
    switch_tunnel_type_ingress_t *ingress_tunnel_type);

switch_status_t switch_api_tunnel_context_dump(const switch_device_t device,
                                               const void *cli_ctx);
#ifdef __cplusplus
}
#endif

#endif /* __SWITCH_TUNNEL_INT_H__ */
