import networkx as nx
import re
import logging
import json
logging.basicConfig(level=logging.INFO)

def count_total_conditions(table_or_cond):
    condition_hints = {'isValid', '==', '!=', '>', '<', '>=', '<='}
    total_conditions = 0
    for hint in condition_hints:
        total_conditions += table_or_cond.count(hint)
    return total_conditions

def draw_graph(graph):
    import networkx as nx
    import matplotlib.pyplot as plt
    G =graph.copy()
    for layer, nodes in enumerate(nx.topological_generations(G)):
        # `multipartite_layout` expects the layer as a node attribute, so add the
        # numeric layer value as a node attribute
        for node in nodes:
            G.nodes[node]["layer"] = layer

    # Compute the multipartite_layout using the "layer" node attribute
    pos = nx.multipartite_layout(G, subset_key="layer")

    fig, ax = plt.subplots()
    nx.draw_networkx(G, pos=pos, ax=ax)
    ax.set_title("DAG layout in topological order")
    fig.tight_layout()
    plt.show()

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
    logging.info("cycle num: {}".format(len(cycles)))
    is_strong = nx.is_strongly_connected(new_subgraph)
    logging.info('is strongly connected: {}'.format(is_strong))
    is_weak = nx.is_weakly_connected(new_subgraph)
    logging.info('is weakly connected: {}'.format(is_weak))
    # Each sub-DAG must be weakly connected DAG for BL to run
    return is_weak, cycles

def set_default(obj):
    if isinstance(obj, set):
        return list(obj)
    raise TypeError

def pretty_print_dict(dictionary):
    logger = logging.getLogger()
    if logger.getEffectiveLevel() <= logging.INFO:
        print(json.dumps(dictionary, indent=4, sort_keys=True, default=set_default))

def visualize_digraph(graph, name):
    logger = logging.getLogger()
    if logger.getEffectiveLevel() <= logging.INFO:
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