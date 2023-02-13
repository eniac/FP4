#ifndef _SWITCH_PD_BFD_H_
#define _SWITCH_PD_BFD_H_

#include "switch_bfd_int.h"
#include "switch_pd_types.h"

switch_status_t switch_pd_bfd_init(switch_device_t device);

switch_status_t switch_pd_bfd_session_update(switch_device_t device,
                                             switch_bfd_info_t *bfd_info);

switch_status_t switch_pd_bfd_session_delete(switch_device_t device,
                                             switch_bfd_info_t *bfd_info);

#endif  // _SWITCH_PD_BFD_H_
