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
#ifndef _RMT_PRE_H
#define _RMT_PRE_H

#define PRE_MGID_MAX 16384
#define PRE_PORTS_MAX 288
#define PRE_YID_MAX 512
#define PRE_LAG_MAX 256
#define PRE_L1_NODE_MAX 16384
#define PRE_L1_ECMP_NODE_MAX 16384
#define PRE_L2_NODE_MAX 4096

typedef enum l1_node_type_ {
  L1_NODE_TYPE_RID = 1,
  L1_NODE_TYPE_RID_XID = 2,
  L1_NODE_TYPE_RID_NO_NEXT = 3,
  L1_NODE_TYPE_ECMP_VECTOR = 4,
  L1_NODE_TYPE_ECMP = 5,
  L1_NODE_TYPE_ECMP_XID = 6
} l1_node_type_t;

typedef uint16_t mgrp_id_t;
typedef uint16_t mgrp_rid_t;
typedef uint16_t mgrp_xid_t;
typedef uint16_t mgrp_yid_t;
typedef uint16_t mgrp_lag_id_t;
typedef uint16_t mgrp_port_id_t;

#endif /* _RMT_PRE_H */
