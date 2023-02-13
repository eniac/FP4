#!/user/bin/env python
###############################################################################
# BAREFOOT NETWORKS CONFIDENTIAL & PROPRIETARY
#
# Copyright (c) 2015-2017 Barefoot Networks, Inc.

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
##############################################################################
"""
Switchapi graph generator

INSTALLATION:
+ Setup on Ubuntu
    - step 1. $ sudo get install graphviz
    - step 2. $ git clone https://github.com/pygraphviz/pygraphviz.git
    - step 3. $ cd pygraphviz/
    - step 4. $ find / -name graphviz
              (find where graphviz 'include' and 'lib' to be used in step 5)
    - step 5. $ sudo pip install pygraphviz \
               --install-option="--include-path=/usr/share/graphviz" \
               --install-option="--library-path=/usr/lib/graphviz/"
    - step 6. $ sudo dot -c

+ Setup on MAC
    - step 1. $ brew install graphviz
    - step 2. $ git clone https://github.com/pygraphviz/pygraphviz.git
    - step 3. $ cd pygraphviz/
    - step 4. $ find / -name graphviz
              (find where graphviz 'include' and 'lib' to be used in step 5)
    - step 5. $ sudo pip install pygraphviz \
               --install-option="--include-path=/usr/local/include/graphviz" \
               --install-option="--library-path=/usr/local/lib/graphviz/"
    - step 6. $ sudo dot -c


RUN:
    $ python switch_graph.py

GRAPH OUTPUT:
    switchapi.dot
    switchapi.svg
"""
import os
import pygraphviz
import re
import sys


# ----------------------------------------------------------------------------
# GLOBAL VARIABLES
SRC_PATH1 = '../include/switchapi'
SRC_PATH2 = '../src'


# ----------------------------------------------------------------------------
def parse_file(fname):
    ''' Parse only what are needed '''
    retlines = []
    with open(fname, "r") as infile:
        oftype, is_first = '', False
        for currline in infile:
            rawline = currline
            currline = currline.strip()
            currline = re.sub('/\*.*\*/', '', currline)
            # ---
            if oftype == 'statement':
                saveline += ' ' + currline
                if ';' in currline:
                    retlines.append(saveline)
                    oftype, saveline = '', ''
            if re.search('typedef\s+enum ', rawline):
                oftype = 'statement'
                saveline = currline
            if rawline.startswith('switch_'):
                temp = rawline.split('(', 1)[0]
                if len(temp.split()) == 2:
                    if oftype: continue
                    oftype = 'statement'
                    saveline = currline
            # ---
            if oftype == 'define':
                if currline.endswith('\\'):
                    currline = currline[:-1].strip()
                    saveline += ' ' + currline
                else:
                    currline = currline.strip()
                    saveline += ' ' + currline
                    retlines.append(saveline)
                    oftype, saveline = '', ''
            if rawline.startswith('#define'):
                oftype = 'define'
                if currline.endswith('\\'):
                    currline = currline[:-1].strip()
                    saveline = currline
                else:
                    retlines.append(currline)
                    oftype, saveline = '', ''
            # ---
            if oftype == 'struct':
                saveline += ' ' + currline
                if '}' in currline:
                    retlines.append(saveline)
                    oftype, saveline = '', ''
            if re.search('typedef\s+struct ', rawline):
                oftype = 'struct'
                saveline = currline
    return retlines


def get_nodes(path):
    ''' Get nodes from switch_handle.h '''
    lines = parse_file('%s/switch_handle.h' % path)
    for line in lines:
        line = re.sub(r'\s+', ' ', line)
        if 'typedef' in line and 'SWITCH_HANDLE_TYPE_' in line:
            return re.findall('SWITCH_HANDLE_TYPE_(\w+)', line)
    return None


def get_node_mappings(path, NODES):
    ''' Read switch_handle_int.h and find handle type ints '''
    lines = parse_file('%s/switch_handle_int.h' % path)
    mappings = []     # key: alias, value: node
    conditions = {}
    for line in lines:
        if re.match('#define\s+SWITCH_', line):
            keys = re.findall('SWITCH_(\w+)_HANDLE', line)
            if not keys: continue
            if 1 < len(keys):
                if 2 == len(keys):
                    if keys[0] != keys[1]:
                        mappings.append((keys[0], keys[1]))
                        continue
                conditions[keys[0]] = keys[1:]
                continue
            alias = keys[0]
            values = re.findall('SWITCH_HANDLE_TYPE_(\w+)', line)
            if not values: continue
            node = values[0]
            if alias != node:
                mappings.append((alias, node))
    for alias, nodes in conditions.items():
        if 1 < nodes:
            for idx, node in enumerate(nodes):
                if node in NODES:
                    mappings.append((alias, node))
                    continue
                if node in dict(mappings):
                    mappings.append((alias, dict(mappings).get(node)))
        else:
            node = nodes
            if node in NODES:
                mappings.append((alias, node))
                continue
            if node in mappings:
                mappings.append((alias, dict(mappings).get(node)))
    return mappings


def define_handles(nodes, nodes_alias):
    handles = [(name, name.lower() + '_handle') for name in nodes ]
    for alias, node in nodes_alias:
        handles.append((node, alias.lower() + '_handle'))

    # Special cases
    handles.append(('INTERFACE', 'intf_handle'))
    return handles


def get_edges(path, nodes, handles):
    ''' Get edges '''
    edges = set()
    sorted_nodes = sorted(nodes, key=lambda i: i.count('_'), reverse=True)
    for file in os.listdir(path):
        if file.endswith('.h'):
            for line in parse_file('%s/%s' % (path, file)):
                if re.match('typedef\s+struct', line):
                    name = line.split()[2]
                    child_node = ''
                    for node in sorted_nodes:
                        if '_' + node.lower() + '_' in name:
                            child_node = node
                            break
                    if child_node:
                        value = re.findall('{(.*)}', line)[0]
                        for node, handle in handles:
                            if handle in value:
                                if node == 'QUEUE' or child_node == 'QUEUE':
                                    print "========="
                                    print file
                                    print "========="
                                print (node, child_node)
                                edges.add((node, child_node))
                if re.match('switch_', line) and '(' in line:
                    name = line.split('(', 1)[0].split()[1]
                    child_node = ''
                    for node in sorted_nodes:
                        if '_' + node.lower() + '_' in name:
                            child_node = node
                            break
                    if child_node:
                        value = re.findall('\((.*)\)', line)[0]
                        for node, handle in handles:
                            if handle in value:
                                if node == 'QUEUE' or child_node == 'QUEUE':
                                    print "========="
                                    print file
                                    print "========="
                                print (node, child_node)
                                edges.add((node, child_node))
    return edges


# ----------------------------------------------------------------------------
def generate_graph(nodes, edges):

    graph = pygraphviz.AGraph(directed=True, landscape='true', ranksep='0.1')

    # default node parameters
    graph.node_attr['style'] = 'filled'
    graph.node_attr['shape'] = 'ellipse'
    #graph.node_attr['fixedsize'] = 'true'
    graph.node_attr['fillcolor'] = '#e6ffff'
    graph.node_attr['fontcolor'] = '#000000'

    for node in nodes:
        if 'MEMBER' in node:
            graph.add_node(node, color='#808000', fillcolor='#ffff80', shape='rectangle')
        elif 'ENTRY' in node:
            graph.add_node(node, color='#6600cc', fillcolor='#d9b3ff', shape='rectangle')
        elif 'GROUP' in node:
            graph.add_node(node, color='#e600e6', fillcolor='#ffccff', shape='rectangle')
        elif 'TABLE' in node:
            graph.add_node(node, color='#00b3b3', fillcolor='#b3ffff', shape='rectangle')
        else:
            graph.add_node(node, color='#00802b', fillcolor='#ccffdd', shape='oval')

    for parent, child in edges:
        graph.add_edge(parent, child, style='dotted', color='#00802b')

    graph.graph_attr['epsilon']='0.001'
    graph.graph_attr['orientation']='portrait'
    graph.graph_attr['ratio'] = 'fill'
    # graph.graph_attr['resolution'] = 100
    graph.graph_attr['autosize'] = False
    graph.graph_attr['size'] = "4, 2"
    graph.draw('switchapi.svg', prog="dot") # neato|dot|twopi|circo|fdp|nop
    graph.draw('switchapi.dot', prog="dot")


# ----------------------------------------------------------------------------
if __name__ == "__main__":
    collection = []
    nodes = get_nodes(SRC_PATH1)
    nodes_alias = get_node_mappings(SRC_PATH2, nodes)
    handles = define_handles(nodes, nodes_alias)

    edges1 = get_edges(SRC_PATH1, nodes, handles)
    edges2 = get_edges(SRC_PATH2, nodes, handles)
    edges = edges1.union(edges2)
    generate_graph(nodes, edges)


# ----------------------------------------------------------------------------

