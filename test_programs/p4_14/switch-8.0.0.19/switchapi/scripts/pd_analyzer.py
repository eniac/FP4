################################################################################
# BAREFOOT NETWORKS CONFIDENTIAL & PROPRIETARY
#
# Copyright (c) 2017-2018 Barefoot Networks, Inc.

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
This script helps identify all the delete pd calls.
"""
import argparse
import os
import sys

from clang.cindex import CursorKind
from clang.cindex import Index
from clang.cindex import TypeKind


def parse_file(idx, filename):
    counter = 2
    changed = False
    tu = idx.parse(filename)
    cursor = tu.cursor
    lines = read_file(filename)
    for child_cursor in cursor.get_children():
        if not child_cursor.location.file or get_file_name(
                child_cursor) != filename:
            continue
        if child_cursor.kind == CursorKind.FUNCTION_DECL:
            is_device_parameter = False

            for argument in child_cursor.get_arguments():
                location = argument.extent.start.line
                if argument.spelling == "device":
                    is_device_parameter = True
            if "delete" in child_cursor.spelling and is_device_parameter:
                changed = True
                lines[location + counter] = "  SWITCH_FAST_RECONFIG(device)\n"
    if changed:
        write_file(filename, lines, "w")


def get_file_name(cursor):
    return cursor.location.file.name


def include_file_type(filename):
    return "switch_pd" in filename and filename.endswith(".c")


def read_file(filename):
    fp = open(filename)
    lines = fp.readlines()
    fp.close()
    return lines


def write_file(filename, lines, mode):
    fp = open(filename, mode)
    fp.writelines(lines)
    fp.close()


def parse_directory(idx, directory):
    for root, directories, names in os.walk(directory):
        for filename in names:
            fullpath = directory + filename
            if include_file_type(fullpath):
                parse_file(idx, fullpath)


if __name__ == "__main__":
    idx = Index.create(True)
    src_dir = "../src/"
    parse_directory(idx, src_dir)
