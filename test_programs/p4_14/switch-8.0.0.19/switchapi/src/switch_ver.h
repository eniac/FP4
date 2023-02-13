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
#ifndef SWITCH_VER_H
#define SWITCH_VER_H

#include "switch_bld_ver.h"

#define SWITCH_REL_VER "8.0.0"
#define SWITCH_VER SWITCH_REL_VER "-" SWITCH_BLD_VER

#define SWITCH_INTERNAL_VER SWITCH_VER "(" SWITCH_GIT_VER ")"

#endif /* BF_DRV_VER_H */
