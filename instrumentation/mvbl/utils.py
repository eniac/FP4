import networkx as nx
import re

def has_numbers(inputString):
    return any(char.isdigit() for char in inputString)

def remove_special_chars(label, cnt_blk):
    if ';' in label:
        label = label.replace(';','')
    if '"' in label:
        label = label.replace('"','')
    if "()" in label and "." not in label:
        label = label.replace("()",'')
        label = cnt_blk+'.'+label
    return label

def check_dag_connected(new_subgraph):
    cycles = list(nx.simple_cycles(new_subgraph))
    print("cycle num: {}".format(len(cycles)))
    is_strong = nx.is_strongly_connected(new_subgraph)
    print('is strongly connected: {}'.format(is_strong))
    is_weak = nx.is_weakly_connected(new_subgraph)
    print('is weakly connected: {}'.format(is_weak))
    # Each sub-DAG must be weakly connected DAG for BL to run
    return is_weak, cycles

def set_default(obj):
    if isinstance(obj, set):
        return list(obj)
    raise TypeError

import json
def pretty_print_dict(dictionary):
    print(json.dumps(dictionary, indent=4, sort_keys=True, default=set_default))

def visualize_digraph(graph, name):
    print("\n====== Visualize {} ======".format(name))
    print("--- nodelist ---")
    for node in graph.nodes:
        print(node)
    print("--- edgelist ---")
    for line in nx.generate_edgelist(graph, delimiter='$', data=False):
        u, v = line.split('$')
        print("{0} --> {1}".format(u, v))

def parse_and_transform(input_str):
    # print("input", input_str)
    set_of_headers = {r"^ ?ipv4\.", r"^ ?nc_hdr\.", r"^ ?tcp\.", r"^ ?arp\.", r"^ ?distance_vec\.", r"^ ?ethernet\."}
    set_of_metadata = {r"^ ?ingress_metadata\.", r"^ ?meta\.", r"^ ?cis553_metadata\.", r"^ ?cheetah_md\."}
    special_transformations = {"$valid == 1" : "isValid()"}

    output_builder = ['"']

    cleaned_input = input_str.strip('()')
    all_conditions = cleaned_input.split("&&")
    condition_builder = []
    for condition in all_conditions:
        for key, value in special_transformations.items():
            condition = condition.replace(key, value)
            
        expression_list = condition.split("==")
        expression_builder = []
        for expression in expression_list:
            expression = expression.strip()
            for header in set_of_headers:
                if re.match(header,expression):
                    # print("matching header", expression)
                    expression = expression.replace(expression,"hdr." +  expression)
                    # print("after update", expression)
                    break
            for metadata in set_of_metadata:
                if re.match(metadata,expression):
                    # expression = "meta." + expression
                    expression = expression.replace(expression,"meta." +  expression)
                    break
            expression_builder.append(expression)
        # print("expression_builder", expression_builder)
        condition_builder.append(" == ".join(expression_builder))
    # print("condition_builder", condition_builder)
    output_builder.append(" && ".join(condition_builder))

    output_builder.append(';"')
    output_str = "".join(output_builder)
    # print("output", output_str)
    return output_str