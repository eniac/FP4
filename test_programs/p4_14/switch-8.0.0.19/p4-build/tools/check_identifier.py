#!/usr/bin/env python

import re
import sys

if re.match("^[_A-Za-z][_a-zA-Z0-9]*$", sys.argv[1]):
    sys.exit(0)
else:
    sys.exit(1)
