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
This script identifies missing internal method definitions for switchapi
calls. These changes were introduced for multithread safety for switchapi.
Once it identifies the missing methods, it either defines them if the m flag is
passed in. Otherwise, it just prints the missing methods to standard output. The
script analyzes methods whose headers are in the include directory of
switchapi. Methods and files can be excluded from this analysis if they are in
the corresponding excluded_methods.txt and excluded_files.txt files.
"""
import argparse
import os
import sys

from clang.cindex import CursorKind
from clang.cindex import Index
from clang.cindex import TypeKind


def parse_file(idx, filename, process_cursor):
    tu = idx.parse(filename)
    cursor = tu.cursor
    for child_cursor in cursor.get_children():
        if not child_cursor.location.file or get_file_name(
                child_cursor) != filename:
            continue
        if child_cursor.kind == CursorKind.FUNCTION_DECL and child_cursor.spelling not in excluded_methods:
            is_device_parameter = False
            for argument in child_cursor.get_arguments():
                if argument.spelling == "device":
                    is_device_parameter = True
            if is_device_parameter:
                process_cursor(child_cursor)


def generate_excluded_methods():
    lines = read_file("excluded_methods.txt")
    return set(map(lambda s: s.strip("\n"), lines))


def generate_excluded_files():
    lines = read_file("excluded_files.txt")
    return set(map(lambda s: s.strip("\n"), lines))


def get_file_name(cursor):
    return cursor.location.file.name


def include_file_type_headers(filename):
    return filename.endswith(".h")


def include_file_type_c(filename):
    return filename.endswith(".c")


def read_file(filename):
    fp = open(filename)
    lines = fp.readlines()
    fp.close()
    return lines


def write_file(cursor, lines, mode):
    filename = get_file_name(cursor)
    fp = open(filename, mode)
    fp.writelines(lines)
    fp.close()


def transform_first_line(line, spelling):
    return line.replace(spelling, spelling + "_internal")


def parse_directory(idx, directory, include_file_type, process_cursor):
    for root, directories, names in os.walk(directory):
        for filename in names:
            fullpath = directory + filename
            if include_file_type(fullpath) and filename not in excluded_files:
                parse_file(idx, fullpath, process_cursor)


def create_header_dic(cursor):
    header_dict[cursor.spelling] = cursor


def create_functions_dic(cursor):
    spelling = cursor.spelling
    if spelling in header_dict:
        c_dict[spelling] = (header_dict[spelling], cursor)
    elif spelling.endswith("_internal"):
        c_dict_modified[spelling] = True


def analyze_differences(handle_method_not_found):
    differences_exist = False
    for method in c_dict:
        new_name = method + "_internal"
        if new_name not in c_dict_modified:
            differences_exist = True
            cursor_tuple = c_dict[method]
            handle_method_not_found(cursor_tuple)
    return differences_exist


def generate_error_message(cursor_tuple):
    print "the internal method for", cursor_tuple[
        0].displayname, "was not found in", get_file_name(cursor_tuple[1])


def generate_function_call(cursor):
    function_call = cursor.spelling + "_internal("
    for argument in cursor.get_arguments():
        function_call += argument.spelling + ", "
    return "    SWITCH_MT_WRAP(" + function_call[:-2] + "))" + "\n"


def alter_existing_function(cursor):
    filename = get_file_name(cursor)
    lines = read_file(filename)
    original_spelling = cursor.spelling
    new_spelling = original_spelling + "_internal"
    line_number = cursor.extent.start.line - 1
    lines[line_number] = lines[line_number].replace(original_spelling,
                                                    new_spelling)
    write_file(cursor, lines, "w")


def add_new_function(header_cursor, c_cursor):
    start_line, end_line = create_string(header_cursor)
    filename = get_file_name(header_cursor)
    lines = read_file(filename)
    method_lines = ["\n"]
    method_lines.extend(lines[start_line - 1:end_line])
    method_lines[-1] = method_lines[-1][:-2] + " {\n"
    method_lines.append(generate_function_call(header_cursor))
    method_lines.append("}\n")
    write_file(c_cursor, method_lines, 'ab')


def fix_files(cursor_tuple):
    header_cursor, c_cursor = cursor_tuple
    add_new_function(header_cursor, c_cursor)
    alter_existing_function(c_cursor)


def create_string(cursor):
    source_range = cursor.extent
    start_location = source_range.start
    end_location = source_range.end
    start_line = start_location.line
    end_line = end_location.line
    return (start_line, end_line)


if __name__ == "__main__":
    idx = Index.create(True)
    header_dict = dict()
    c_dict = dict()
    c_dict_modified = dict()
    excluded_methods = generate_excluded_methods()
    excluded_files = generate_excluded_files()
    include_dir = "../include/switchapi/"
    src_dir = "../src/"
    parse_directory(idx, include_dir, include_file_type_headers,
                    create_header_dic)
    parse_directory(idx, src_dir, include_file_type_c, create_functions_dic)
    parser = argparse.ArgumentParser(description="format your parse for lock")
    parser.add_argument("-m", action="store_true", default=False)
    command_flags = parser.parse_args()
    if command_flags.m:
        differences_exist = analyze_differences(fix_files)
    else:
        differences_exist = analyze_differences(generate_error_message)
    if differences_exist:
        sys.exit(-1)
