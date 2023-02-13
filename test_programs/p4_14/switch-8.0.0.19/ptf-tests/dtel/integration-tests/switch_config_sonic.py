###############################################################################
# BAREFOOT NETWORKS CONFIDENTIAL & PROPRIETARY
#
# Copyright (c) 2015-2016 Barefoot Networks, Inc.

# All Rights Reserved.
#
# NOTICE: All information contained herein is, and remains the property of
# Barefoot Networks, Inc. and its suppliers, if any. The intellectual and
# technical concepts contained herein are proprietary to Barefoot Networks,
# Inc.
# and its suppliers and may be covered by U.S. and Foreign Patents, patents in
# process, and are protected by trade secret or copyright law.
# Dissemination of this information or reproduction of this material is
# strictly forbidden unless prior written permission is obtained from
# Barefoot Networks, Inc.
#
# No warranty, explicit or implicit is provided, unless granted under a
# written agreement with Barefoot Networks, Inc.
#
# $Id: $
#
###############################################################################

"""
Switch configuration
"""
import os, sys, pdb, collections

# Import ultrasonic
this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../../../../tools/sonic/ultrasonic'))
from sonic import sonic_switch
from dtel.infra import *

from constants import *

# Import parameters
import aasw35
import aasw36
import aasw37
import aasw38
import aah42
import aah44


TYPE_POSTCARD    = 0
TYPE_INT_EP      = 1
TYPE_INT_TRANSIT = 2
TYPE_QUEUEREPORT = 3

def config_watchlist(switch, postcard_watch=False, mod_watch=False,
        int_watch=False, int_session_id=1, suppress=True, percent=100):
    # Configure watchlist based on telemetry type
    if postcard_watch:
      wl = switch.create_dtel_watchlist('flow')
      # Add entries to the watchlist
      wl.create_entry(priority=10,
                     src_ip='10.131.0.0',
                     src_ip_mask=11,
                     dst_ip='10.131.0.0',
                     dst_ip_mask=11,
                     dtel_sample_percent=percent,
                     dtel_report_all=not suppress)
    if int_watch:
      wl = switch.create_dtel_watchlist('flow')
      # Add entries to the watchlist
      wl.create_entry(priority=10,
                     src_ip='10.131.0.0',
                     src_ip_mask=11,
                     dst_ip='10.131.0.0',
                     dst_ip_mask=11,
                     dtel_int_session=int_session_id,
                     dtel_sample_percent=percent,
                     dtel_report_all=not suppress)
    if mod_watch:
      wl = switch.create_dtel_watchlist('drop')
      # Add entries to the watchlist
      wl.create_entry(priority=10,
                     src_ip='10.131.0.0',
                     src_ip_mask=11,
                     dst_ip='10.131.0.0',
                     dst_ip_mask=11)

def main():
  while True:
    sid = int(raw_input(
      "Which switch to configure (35, 36, 37, 38)? "))
    if not sid in [35, 36, 37, 38]:
      print "invalid switch"
      return
    config_switch(sid)

def config_switch(sid):
    # Import variables from corresponding file
    sw = globals()["aasw%d"%sid]
    print "Configure aasw%d (Mavericks %s)"%(sid, sw.management_ip)
    switch_type = int(raw_input(
        'Enter switch type: 0: Postcard, 1: INT endpoint, 2: INT transit, 3: Queue endpoint: '))
    mod_enabled = True
    quantization_shift = 17

    if switch_type == TYPE_POSTCARD:
        switch = sonic_switch.SONiCSwitch(dtel_switch_id=sw.switch_id,
                                      management_ip=sw.management_ip,
                                      dtel_monitoring_type='postcard')
        switch.dtel_postcard_enable = True
        config_watchlist(switch, postcard_watch=True, mod_watch=mod_enabled)
    elif switch_type == TYPE_INT_TRANSIT:
        switch = sonic_switch.SONiCSwitch(dtel_switch_id=sw.switch_id,
                                      management_ip=sw.management_ip,
                                      dtel_monitoring_type='int_transit')
        switch.dtel_int_l4_dscp = {'value': 1, 'mask': 1}
        switch.dtel_int_transit_enable = True
        config_watchlist(switch, int_watch=False, mod_watch=mod_enabled, suppress=True)
    elif switch_type == TYPE_INT_EP:
        switch = sonic_switch.SONiCSwitch(dtel_switch_id=sw.switch_id,
                                      management_ip=sw.management_ip,
                                      dtel_monitoring_type='int_endpoint')
        switch.dtel_int_l4_dscp = {'value': 1, 'mask': 1}
        switch.dtel_int_endpoint_enable = True
        # Add sink ports
        switch.dtel_int_sink_port_list = ['1/0', '1/1', '1/2', '1/3']
        # INT EP reqiuires INT session configuration
        int_s=switch.create_dtel_int_session(max_hop_count=8)
        config_watchlist(switch, int_watch=True, int_session_id=int_s,
            mod_watch=mod_enabled, suppress=True)
    elif switch_type == TYPE_QUEUEREPORT:
        # dtel_monitoring_type value is not relevant if we are only configuring Queue
        # reports
        switch = sonic_switch.SONiCSwitch(dtel_switch_id=sw.switch_id,
                                      management_ip=sw.management_ip,
                                      dtel_monitoring_type='postcard')

        threshold = int(raw_input('Queue threshold? '))
        quota = int(raw_input('Queue quota? '))
        dod = int(raw_input('Queue dod (0: False, 1: True)? '))
        quantization_shift = int(raw_input('Latency quantization? '))
        # Create a queue report on queue 0 of port 1/1
        qr = switch.create_dtel_queue_report('1/1',
                                             0,
                                             hex_to_i16(threshold),
                                             hex_to_i32(0xffffffff),
                                             hex_to_i16(quota),
                                             dod!=0)
    else:
        print "invalid switch type"
        return
    # Create report session
    rs = switch.create_dtel_report_session([aah44.ipaddr_inf[1]])
    # Set other switch-wide parameters
    switch.dtel_drop_report_enable = mod_enabled
    switch.dtel_latency_sensitivity = quantization_shift
    switch.dtel_flow_state_clear_cycle = 1


if __name__ == "__main__":
    main()
