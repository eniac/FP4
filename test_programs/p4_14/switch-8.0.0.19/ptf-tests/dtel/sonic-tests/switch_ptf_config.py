###############################################################################
# BAREFOOT NETWORKS CONFIDENTIAL & PROPRIETARY
#
# Copyright (c) 2015-2018 Barefoot Networks, Inc.

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


this_dir = os.path.dirname(os.path.abspath(__file__))
sys.path.append(os.path.join(this_dir, '../../base'))
from common.utils import *
sys.path.append(os.path.join(this_dir, '../../base/api-tests'))
import api_base_tests
sys.path.append(os.path.join(this_dir, '..'))
from dtel_utils import *

sys.path.append(os.path.join(this_dir, '../../../../../tools/sonic/ultrasonic'))
from sonic import sonic_switch
import collections
from dtel.infra import *

set_int_l45_dscp(value=0x01, mask=0x01)

mac_all_zeros = '00:00:00:00:00:00'

# change the following params per your target test switch perspective

# aasw38 - aah44
swports = [2, 3, 52]
devports = [188, 189, 136]
fpports = ['1/2', '1/3', '14/0']
switch_ip = '10.12.11.38'
mac_self = '00:90:fb:5c:e1:8a'
ipaddr_nbr = ['10.144.1.2', '10.144.2.2', '10.144.3.2']
mac_nbr = ['00:00:10:44:01:02', '00:00:10:44:02:02', '00:00:10:44:03:02']

# aasw37 - aah42
'''
swports = [2, 3, 48]
devports = [130, 131, 36]
fpports = ['1/2', '1/3', '12/0']
switch_ip = '10.12.11.37'
mac_self = '00:90:fb:5e:48:a2'
ipaddr_nbr = ['10.132.1.2', '10.132.2.2', '10.132.3.2']
mac_nbr = ['00:00:10:32:01:02', '00:00:10:32:02:02', '00:00:10:32:03:02']
'''

report_ports = [2]
report_src = switch_ip
report_dst = ['10.144.3.2']
report_udp_port = UDP_PORT_DTEL_REPORT
report_truncate_size = 512
switch_id = 1
high_latency_sensitivity = MAX_QUANTIZATION
low_latency_sensitivity = 0
