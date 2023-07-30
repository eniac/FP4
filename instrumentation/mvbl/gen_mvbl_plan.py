import argparse

import pydotplus
import pydot
import json
import networkx as nx
import copy
import re
import sys


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

import json
def pretty_print_dict(dictionary):
    print(json.dumps(dictionary, indent=4, sort_keys=True))

def visualize_digraph(graph, name):
    print("\n====== Visualize {} ======".format(name))
    print("--- nodelist ---")
    for node in graph.nodes:
        print(node)
    print("--- edgelist ---")
    for line in nx.generate_edgelist(graph, delimiter='$', data=False):
        u, v = line.split('$')
        print("{0} --> {1}".format(u, v))

UNIQUE_ACTION_SIG = "_pfuzz_"
JSON_OUTPUT_KEY_NUM_VARS = "num_vars"
JSON_OUTPUT_KEY_NUM_BITS = "num_bits"
JSON_OUTPUT_KEY_EDGE_DST_INCREMENT_DICT = "edge_dst_to_increment"
JSON_OUTPUT_KEY_EDGE_DST_EDGE_DICT = "edge_dst_to_edge"
JSON_OUTPUT_KEY_NON_ACTION_INCREMENT_DICT = "non_action_to_increment"
JSON_OUTPUT_KEY_FINAL_EDGE_DST_EDGE_DICT = "final_edge_dst_to_increment"
JSON_OUTPUT_KEY_FINAL_EDGE_INCREMENT_DICT = "final_edge_dst_to_edge"
JSON_OUTPUT_KEY_FINAL_NON_ACTION_INCREMENT_DICT = "final_non_action_to_increment"
JSON_OUTPUT_KEY_FINAL_NON_ACTION_INCREMENT_ROOTWORD_DICT = "final_non_action_to_increment_rootword"
JSON_OUTPUT_KEY_ENCODING_TO_PATH_DICT = "encoding_to_path"
JSON_OUTPUT_KEY_NUM_PATHS = "num_paths"
JSON_OUtPUT_KEY_NODES = "nodes"
JSON_OUTPUT_EDGES = "edges"
JSON_OUTPUT_KEY_TABLE_TO_ACTIONS_DICT = "table2actions_dict"
JSON_OUTPUT_KEY_STAGE_TO_TABLES_DICT_ORIGINAL = "stage2tables_dict"
JSON_OUTPUT_KEY_STAGE_TO_TABLES_DICT_SANITIZED = "stage2tables_dict_sanitized"
JSON_OUTPUT_KEY_SUM_NUM_PATHS = "sum_num_paths"
JSON_OUTPUT_KEY_GLOBAL_NUM_PATHS = "global_num_paths"
JSON_OUTPUT_KEY_GLOBAL_PATHS = "global_paths"


context_to_dot_label_mapping = {
    "(ingress_metadata.on_miss_ipv4_fib == 1)": "\"meta.ingress_metadata.on_miss_ipv4_fib == 1;\"",
    "(ipv4.$valid == 1)": "\"hdr.ipv4.isValid();\"",
    "(eg_intr_md.egress_port == 40)": "\"eg_intr_md.egress_port == 40;\"",
    "(meta.current_reading == 0)": "\"meta.meta.current_reading == 0;\"",
    "(meta.available != 0)": "\"meta.meta.available != 0;\"",    
    "(nc_hdr.$valid == 1)": "\"hdr.nc_hdr.isValid();\"",
    "(nc_hdr.op == 0)": "\"hdr.nc_hdr.op == 0;\"",
    "(nc_hdr.op == 1)": "\"hdr.nc_hdr.op == 1;\"",
    "(meta.direction == 0)": "\"meta.meta.direction == 0;\"",
    "(tcp.syn == 1)": "\"hdr.tcp.syn == 1;\"",
    "(meta.check_ports_hit == 1)": "\"meta.meta.check_ports_hit == 1;\"",
    "(distance_vec.$valid == 1)": "\"hdr.distance_vec.isValid();\"",
    "(arp.$valid == 1)": "\"hdr.arp.isValid();\"",
    "(arp.oper == 1)": "\"hdr.arp.oper == 1;\"",
    "(arp.$valid == 1 && arp.oper == 1)": "\"hdr.arp.isValid() && hdr.arp.oper == 1;\"",
    "(ethernet.$valid == 1)": "\"hdr.ethernet.isValid();\"",
    "(arp.$valid == 1 && arp.oper == 2)": "\"hdr.arp.isValid() && hdr.arp.oper == 2;\"",
    "(cis553_metadata.forMe == 0)": "\"meta.cis553_metadata.forMe == 0;\"",
}


class GraphParser(object):
    def __init__(self, prog_name, dotfile_ing, jsonfile, input_type='p414', direction='ingress'):
        print("\n====== program_name: {3}, dotfile_ing: {0}, jsonfile: {1}, direction: {2} ======".format(dotfile_ing, jsonfile, direction, prog_name))
        json_output_dict = {}

        print("\n====== Extract node name to label mapping, filtering START, EXIT, tbl_act ======")
        node2label_dict, ignored_node2label_dict = self.get_raw_nodes_from_dot(dotfile_ing)

        if len(node2label_dict) == 0:
            json_output_dict[JSON_OUTPUT_KEY_NUM_VARS] = 0
            with open("plan/"+prog_name+"_"+direction+".json", 'w') as f:
                json.dump(json_output_dict, f, indent=4, sort_keys=True)
            print("====== Completed ======")
            return
            # raise Exception("len(node2label_dict) == 0!")

        print("\n====== Extract edge dicts, renamed src, dst with the labels, skipped START, EXIT ======")
        renamed_edges = self.get_edges_from_dot(dotfile_ing, node2label_dict, ignored_node2label_dict)

        print("\n====== Create a table graph with 0 weights ======")
        table_graph_edges_tuples, table_graph = self.get_table_graph(node2label_dict, renamed_edges)

        visualize_digraph(table_graph, "table_graph")

        print("\n====== extract_stages_p4_14 ======")
        stage2tables_dict, table2actions_dict = self.extract_stages_p4_14(jsonfile, node2label_dict.values(), direction)
        # if input_type == 'bmv2':
            # conditions_to_nextstep = self.extract_conditionals(jsonfile)
            # actions = self.extract_actions_bmv2(jsonfile)
            # table2actions_dict = self.extract_tables_actions_bmv2(jsonfile, actions)
        stage2tables_dict_original = copy.deepcopy(stage2tables_dict)
        json_output_dict[JSON_OUTPUT_KEY_STAGE_TO_TABLES_DICT_ORIGINAL] = stage2tables_dict_original

        print("====== Print all_actions ======")
        all_actions = []
        for action_list in table2actions_dict.values():
            all_actions.extend(action_list)
        all_actions = list(set(all_actions))
        print(all_actions)

        print("\n====== Check the consistency of the outputs from context.json and .dot ======")
        for stage, table_list in stage2tables_dict.items():
            for table in table_list:
                if table not in table_graph.nodes:
                    print("table: {} NOT in table_graph.nodes!".format(table))
                    raise Exception("ERR!")

        # if input_type == 'bmv2':
        #     stage2tables_dict = self.estimate_stages(table_graph)

        print("\n====== Print roots and leaf_nodes ======")
        root_nodes = [v for v, d in table_graph.in_degree() if d == 0]
        leaf_nodes = [v for v, d in table_graph.out_degree() if d == 0]
        print("--- root_nodes ---")
        print(root_nodes)
        if len(root_nodes) != 1:
            raise Exception("len(root_nodes) != 1")
        global_root_node = root_nodes[0]
        print("--- leaf_nodes ---")
        print(leaf_nodes)

        print("\n====== Add edges from tables to actions, actions to the next table ======")
        updated_nodes, updated_edges = self.append_table_action_edges(table2actions_dict, renamed_edges, leaf_nodes, root_nodes, node2label_dict)

        print("\n====== Create full graph ======")
        print("--- updated_nodes ---")
        print(updated_nodes)
        print("--- updated_edges ---")
        print(updated_edges)
        full_graph_edges_tuples = []  # Reset
        for e in updated_edges:
            full_graph_edges_tuples.append((e['src'], e['dst'], 0))
        full_graph_edges_tuples = list(set(full_graph_edges_tuples))
        full_graph = nx.DiGraph()
        full_graph.add_nodes_from(updated_nodes)
        full_graph.add_weighted_edges_from(full_graph_edges_tuples)

        visualize_digraph(full_graph, "full_graph")

        print("\n====== Sanitize special chars ([^a-zA-Z0-9_], e.g., \`.\`,\(,\),) for all nodes (actually for conditionals) for pulp ======")
        new_node_mapping, reverse_new_node_mapping = self.sanitize_node_name(full_graph)

        print("\n====== Create a sanitized graph ======")
        table_graph = nx.relabel_nodes(table_graph, new_node_mapping)
        full_graph =  nx.relabel_nodes(full_graph, new_node_mapping)

        print("\n====== Visialuze sanitized table_graph ======")
        for line in nx.generate_edgelist(full_graph, delimiter='$', data=False):
            # print(line)
            u, v = line.split('$')
            print("{0} --> {1}".format(u, v))

        print("\n====== Visialuze sanitized full_graph ======")
        for line in nx.generate_edgelist(full_graph, delimiter='$', data=False):
            # print(line)
            u, v = line.split('$')
            print("{0} --> {1}".format(u, v))

        print("\n=== Update table names in table2actions_dict ===")
        for old_name, new_name in new_node_mapping.items():
            if old_name in table2actions_dict:
                if old_name != new_name:
                    print("Unexpected rename of table old_name: {0} -> new_name: {1}".format(old_name, new_name))
                    table2actions_dict[new_name] = table2actions_dict.pop(old_name)
        json_output_dict[JSON_OUTPUT_KEY_TABLE_TO_ACTIONS_DICT] = table2actions_dict

        print("\n=== Update table names in stage2tables_dict ===")
        print("--- stage2tables_dict original ---")
        pretty_print_dict(stage2tables_dict)
        for stage, table_list in stage2tables_dict.items():
            for i, table_or_condtional in enumerate(table_list):
                if table_or_condtional in new_node_mapping:
                    if table_or_condtional != new_node_mapping[table_or_condtional]:
                        print("Must be a rename of conditional old_name: {0} -> new_name: {1}".format(table_or_condtional, new_node_mapping[table_or_condtional]))
                        table_list[i] = new_node_mapping[table_or_condtional]
                else:
                    print("table_or_condtional {} not found in new_node_mapping!".format(table_or_condtional))
                    raise Exception("ERR!")
        print("--- stage2tables_dict after ---")
        pretty_print_dict(stage2tables_dict)

        json_output_dict[JSON_OUTPUT_KEY_STAGE_TO_TABLES_DICT_SANITIZED] = stage2tables_dict

        print("\n====== Check for cycles... ======")
        cycles = list(nx.simple_cycles(full_graph))
        if cycles:
            print("****** cycles found! ********")
            for cycle in cycles:
                print(cycle)
            print("****** end cycles found ********")
            print("****** It should be a DAG ********")
            raise Exception("ERR!")

        from pulp_solver import PulpSolver
        print("\n====== Running PulpSolver ======")
        pulpSolver = PulpSolver(table_graph, stage2tables_dict, table2actions_dict)

        json_output_dict[JSON_OUTPUT_KEY_NUM_VARS] = len(pulpSolver.var_to_bits)
        # Based on the solver result, construct the sub-DAGs with only the tables and edges involved
        # Note that dummy edges should be handled properly
        new_subgraphs = []
        for graph_number, number_of_bits in enumerate(pulpSolver.var_to_bits):
            json_output_dict[str(graph_number)] = {
                JSON_OUTPUT_KEY_NUM_BITS : number_of_bits
            }
            print("\n====== Constructing subgraph {0} for MVBL based on table_graph and pulp result ======".format(graph_number))
            print("--- List of (table/conditonal) nodes included from pulp ---")
            included_table_conditional = pulpSolver.subgraph_to_tables[graph_number]
            print(included_table_conditional)
            included_table_conditional_action = []
            for node in full_graph.nodes:
                if node in included_table_conditional:
                    included_table_conditional_action.append(node)
                elif UNIQUE_ACTION_SIG in node and node.split(UNIQUE_ACTION_SIG)[1] in included_table_conditional:
                    included_table_conditional_action.append(node)
            print("--- included_table_conditional_action ---")
            print(included_table_conditional_action)

            # Construct the new graph using included_table_conditional_action
            new_subgraph = nx.DiGraph()
            new_subgraph.add_nodes_from(included_table_conditional_action)  # Always create from node first
            # Now consider whether to add an edge between the two nodes
            new_subgraph_edges = []
            for node_src in included_table_conditional_action:
                for node_dst in included_table_conditional_action:
                    if node_src != node_dst:
                        all_simple_paths = nx.all_simple_paths(full_graph, node_src, node_dst)
                        for path in all_simple_paths:
                            if set(path[1:-1]).isdisjoint(included_table_conditional_action):
                                new_subgraph_edges.append((node_src, node_dst, 0))
                                break
            print("--- new_subgraph_edges ---")
            print(new_subgraph_edges)
            new_subgraph.add_weighted_edges_from(new_subgraph_edges)
            visualize_digraph(new_subgraph, "new_subgraph")

            print("--- Check if each subgraph is a DAG and weakly connected ---")
            is_weak, cycles = check_dag_connected(new_subgraph)
            if not is_weak or len(cycles) != 0:
                raise Exception("Not weakly connected or with loops!")
                sys.exit()
            # LC_TODO: So there are randomness in the ILP plan, if one of the plan works, just cache it, otherwise, handle it by connecting the global_root to the roots here...

            new_subgraphs.append(new_subgraph)
            json_output_dict[str(graph_number)][JSON_OUtPUT_KEY_NODES] = sorted(list(new_subgraph.nodes), key=lambda e: e)
            json_output_dict[str(graph_number)][JSON_OUTPUT_EDGES] = sorted(list(nx.generate_edgelist(new_subgraph, delimiter=' -> ', data=False)), key=lambda e: e)

        graphs_with_weights = []
        for idx, graph in enumerate(new_subgraphs):
            print("\n====== Running BL for subgraph {0} ======".format(idx))
            print("----- Get BL plan for variable {0} additions------".format(idx))
            json_output_dict[str(idx)][JSON_OUTPUT_KEY_EDGE_DST_INCREMENT_DICT] = {}
            graph_with_weights, json_output_edge_dst_increment_dict, json_output_edge_dst_edge_dict, json_output_non_action_increment_dict, json_output_final_edge_dst_increment_dict, json_output_final_edge_dst_edge_dict, json_output_final_non_action_increment_dict, json_output_final_non_action_increment_rootword_dict = self.ball_larus(graph, table2actions_dict, idx)
            json_output_dict[str(idx)][JSON_OUTPUT_KEY_EDGE_DST_INCREMENT_DICT] = json_output_edge_dst_increment_dict
            json_output_dict[str(idx)][JSON_OUTPUT_KEY_EDGE_DST_EDGE_DICT] = json_output_edge_dst_edge_dict
            json_output_dict[str(idx)][JSON_OUTPUT_KEY_NON_ACTION_INCREMENT_DICT] = json_output_non_action_increment_dict
            json_output_dict[str(idx)][JSON_OUTPUT_KEY_FINAL_EDGE_DST_EDGE_DICT] = json_output_final_edge_dst_increment_dict
            json_output_dict[str(idx)][JSON_OUTPUT_KEY_FINAL_EDGE_INCREMENT_DICT] = json_output_final_edge_dst_edge_dict
            json_output_dict[str(idx)][JSON_OUTPUT_KEY_FINAL_NON_ACTION_INCREMENT_DICT] = json_output_final_non_action_increment_dict
            json_output_dict[str(idx)][JSON_OUTPUT_KEY_FINAL_NON_ACTION_INCREMENT_ROOTWORD_DICT] = json_output_final_non_action_increment_rootword_dict
            graphs_with_weights.append(graph_with_weights)
        
        sum_num_paths = 0
        for idx, graph in enumerate(graphs_with_weights):
            json_output_dict[str(idx)][JSON_OUTPUT_KEY_ENCODING_TO_PATH_DICT] = {}
            print("\n====== Printing path to encoding for sub-DAG {0} ======".format(idx))
            subdag_root_nodes = [v for v, d in graph.in_degree() if d == 0]
            subdag_leaf_nodes = [v for v, d in graph.out_degree() if d == 0]
            print("--- subdag_root_nodes ---")
            print(subdag_root_nodes)
            print("--- subdag_leaf_nodes ---")
            print(subdag_leaf_nodes)
            all_paths_weights = []
            for root in subdag_root_nodes:
                for leaf in subdag_leaf_nodes:
                    # This sub-dag must be a single (conditional) node
                    if root == leaf:
                        if len(subdag_root_nodes) != 1 or len(subdag_leaf_nodes) != 1:
                            raise Exception("len(subdag_root_nodes) != 1 or len(subdag_leaf_nodes) != 1!")
                            sys.exit()
                        all_paths_weights.append(([root], 0))
                    else:
                        paths = list(nx.all_simple_paths(graph, root, leaf))
                        for path in paths:
                            weight = sum(graph.get_edge_data(path[i], path[i + 1])['weight'] for i in range(len(path) - 1))
                            all_paths_weights.append((path, weight))
            all_paths_weights.sort(key=lambda x: x[1])
            json_output_dict[str(idx)][JSON_OUTPUT_KEY_NUM_PATHS] = len(all_paths_weights)
            sum_num_paths += len(all_paths_weights)
            print("--- Expecting all_path_weights with weight from 0 to {} ---".format(len(all_paths_weights)))
            expected_weight = 0
            for path, weight in all_paths_weights:
                print("Path: {0}, Total Weight: {1}".format(path, weight))
                json_output_dict[str(idx)][JSON_OUTPUT_KEY_ENCODING_TO_PATH_DICT][str(weight)] = path
                if weight != expected_weight:
                    raise Exception("Unexpected weight! Possibly mis-assigned weights")
                expected_weight += 1
        json_output_dict[JSON_OUTPUT_KEY_SUM_NUM_PATHS] = sum_num_paths

        print("\n====== Get the number of paths for the full_graph ===")
        full_root_nodes = [v for v, d in full_graph.in_degree() if d == 0]
        full_leaf_nodes = [v for v, d in full_graph.out_degree() if d == 0]
        print("--- full_root_nodes ---")
        print(full_root_nodes)
        print("--- full_leaf_nodes ---")
        print(full_leaf_nodes)
        global_paths = []
        for root in full_root_nodes:
            for leaf in full_leaf_nodes:
                if root == leaf:
                    if len(full_root_nodes) != 1 or len(full_leaf_nodes) != 1:
                        raise Exception("len(full_root_nodes) != 1 or len(full_leaf_nodes) != 1!")
                        sys.exit()
                    global_paths.append(root)
                else:
                    paths = list(nx.all_simple_paths(full_graph, root, leaf))
                    for path in paths:
                        global_paths.append(path)
        print("--- len(global_paths): {} ---".format(len(global_paths)))
        json_output_dict[JSON_OUTPUT_KEY_GLOBAL_NUM_PATHS] = len(global_paths)
        json_output_dict[JSON_OUTPUT_KEY_GLOBAL_PATHS] = global_paths

        with open("plan/"+prog_name+"_"+direction+".json", 'w') as f:
            json.dump(json_output_dict, f, indent=4, sort_keys=True)
        
        print("====== Completed ======")

    def get_raw_nodes_from_dot(self, dotfile, cnt_blk='MyIngress'):
        node_name_label = {}
        ignored_node_name_label = {}
        dot_graph = pydotplus.graphviz.graph_from_dot_file(dotfile) 
        subgraphs = dot_graph.get_subgraphs()  
        for subG in subgraphs:
            for n in subG.get_node_list():
                label = n.obj_dict["attributes"]['label']
                print("--- name: {0}, label {1} ---".format(n.get_name(), label))
                if has_numbers(n.get_name()) and label not in ['__START__',"", '__EXIT__', 'tbl_act']:
                    # Do one function at a time
                    # label = remove_special_chars(label, cnt_blk)
                    if label not in node_name_label.values():
                        print("Write: {0}:{1}".format(n.get_name(), label))
                        node_name_label[n.get_name()] = label
                    else:
                        print("TODO: Duplicate label, exit!!!")
                        raise Exception("ERR!")
                        cnt = 1
                        key = list(node_name_label.keys())
                        values = list(node_name_label.values())
                        ind = key[values.index(label)]
                        while(1):
                            if label+"##"+str(cnt) in node_name_label:
                                cnt += 1
                            else:
                                label = label+"##"+str(cnt)
                                break
                        print("Write to node_name_label: {0}:{1}".format(n.get_name(), label))
                        node_name_label[n.get_name()] = label
                        # if "##" in node_name_label[ind]:
                        #   cnt = int(node_name_label[ind].split("##")[1])
                else:
                    ignored_node_name_label[n.get_name()] = label
        print("--- node_name_label ---")
        pretty_print_dict(node_name_label)
        print("--- ignored_node_name_label ---")
        pretty_print_dict(ignored_node_name_label)
        return node_name_label, ignored_node_name_label

    def get_table_graph(self, node2label_dict, renamed_edges):
        print("--- get_table_graph ---")
        edges_tuples = []
        for e in renamed_edges:
            edges_tuples.append((e['src'], e['dst'], 0))
        edges_tuples = list(set(edges_tuples))
        table_graph = nx.DiGraph()
        print("add_nodes_from: {}".format(list(node2label_dict.values())))
        table_graph.add_nodes_from(list(node2label_dict.values()))
        table_graph.add_weighted_edges_from(edges_tuples)
        return edges_tuples, table_graph
 
    def extract_stages_p4_14(self, contextFile, node_labels, target_direction):
        context_data = None
        with open(contextFile, 'r') as f:
            context_data = json.load(f)

        stage2tables_dict = {}
        table2actions_dict = {}

        # Goal: every label should be matched
        label2matched = {}
        for label in node_labels:
            label2matched[label] = 0
        
        for table_information in context_data['tables']:
            table_name = table_information['name']
            table_type = table_information['table_type']
            direction = table_information["direction"]
            print("--- name: {0}, table_type: {1}, direction: {2} ---".format(table_name, table_type, direction))
            is_matched = False
            if target_direction != direction:
                print("Skipped")
                continue
            if table_type == "condition":
                # Rename the condition before matching
                # We don't know the naming schema in context.json, also need to handle multi-conditonals
                # renamed_condition = table_information['condition'][1:-1]
                # if '$valid' in table_information['condition']:
                #     renamed_condition = renamed_condition.replace("$valid", "isValid()")
                #     if ' == 1' in table_information['condition']:
                #         renamed_condition = renamed_condition.replace(" == 1", "")
                # Safer to do in a dictionary manner
                renamed_condition = table_information['condition']
                if renamed_condition in context_to_dot_label_mapping:
                    renamed_condition = context_to_dot_label_mapping[renamed_condition]
                print("raw condition: {0}, renamed: {1}".format(table_information['condition'], renamed_condition))
                for node_label in node_labels:
                    # print("Try matching to node_label: {}".format(node_label))
                    if renamed_condition == node_label:
                        # print("[INFO] Fuzzy matched to node_label: {}".format(node_label))
                        print("[INFO] Exact matched to node_label: {}".format(node_label))
                        is_matched = True
                        if label2matched[node_label] == 0:
                            label2matched[node_label] = 1
                        else:
                            raise Exception("[ERROR] Double matched!")
                        for stage_information in table_information['stage_tables']:
                            stage_number = stage_information['stage_number']
                            if stage_number not in stage2tables_dict:
                                stage2tables_dict[stage_number] = []
                            stage2tables_dict[stage_number].append(node_label)
                        break
            # Which is similar to a branch point
            elif table_type == "match":
                if table_name == "tbl_act":
                    print("[INFO] Skipped")
                for node_label in node_labels:
                    if table_name == node_label:
                        print("Exact matched to node_label: {}".format(node_label))
                        is_matched = True
                        if label2matched[node_label] == 0:
                            label2matched[node_label] = 1
                        else:
                            raise Exception("[ERROR] Double matched!")
                        if table_name not in table2actions_dict:
                            table2actions_dict[table_name] = []
                        for action in table_information['actions']:
                            # Each action is renamed to table_name__action_name s.t. actions are unique to a table
                            table2actions_dict[table_name].append(action['name'] + UNIQUE_ACTION_SIG + table_name)
                            if action['name'] == "NoAction":
                                print("[ERROR] NoAction! Check if it is from the user or the compiler!")
                                raise Exception("ERR!")
                        for stage_information in table_information['match_attributes']['stage_tables']:
                            stage_number = stage_information['stage_number']
                            if stage_number not in stage2tables_dict:
                                stage2tables_dict[stage_number] = []
                            stage2tables_dict[stage_number].append(table_information['name'])
            elif table_type == "stateful":
                print("[INFO] Skipped")
            elif table_type == "action":
                print("[INFO] Skipped")
            else:
                print("Unknown table_type!")
                raise Exception("ERR!")
            if not is_matched:
                print("[WARNING] Unmatched!")

        print("--- Check if every nodes from dot are matched ---")
        for label in node_labels:
            if label2matched[label] == 0:
                raise Exception("{} unmatched!".format(label))

        print("--- stage2tables_dict ---")
        pretty_print_dict(stage2tables_dict)
        print("--- table2actions_dict ---")
        pretty_print_dict(table2actions_dict)
        return stage2tables_dict, table2actions_dict

    def sanitize_node_name(self, graph):
        new_node_mapping = {}
        reverse_new_node_mapping = {}
        for node in graph.nodes:
            new_node = re.sub(r'\W+', '', node)
            print("--- {0} mapped to {1} ---".format(node, new_node))
            new_node_mapping[node] = new_node
            reverse_new_node_mapping[new_node] = node
        return new_node_mapping, reverse_new_node_mapping

    def ball_larus(self, graph, table2actions_dict, graph_idx):
        weighted_edges = []
        for e in graph.edges:
            # print(e)
            weighted_edges.append({'src':e[0], 'dst':e[1], 'weight':0})

        num_path = {}
        rev_topological_order = list(reversed(list(nx.topological_sort(graph))))
        leaf_vertex = [v for v, d in graph.out_degree() if d == 0]
        for v in rev_topological_order:
            if v in leaf_vertex:
                num_path[v] = 1
            else:
                num_path[v] = 0
                out_edges = graph.out_edges(v)
                # Sort the edges in rev_topological_order
                print("--- out_edges for {} ---".format(v))
                print(out_edges)
                # To avoid randomness in the generated out_edges, include the alphabetical sort in the last key
                # rev_topological sort only (in the sub-DAG) will still be random
                # sorted_out_edges = sorted(out_edges, key=lambda e: (rev_topological_order.index(e[1]), e))
                sorted_out_edges = sorted(out_edges, key=lambda e: e)
                print("--- sorted_out_edges for {} ---".format(v))
                print(sorted_out_edges)
                if list(sorted_out_edges) != list(out_edges):
                    print("[WARNING] sorted_out_edges != out_edges")
                if set(out_edges) != set(sorted_out_edges):
                    raise Exception("set(out_edges) != set(sorted_out_edges)")
                for e in sorted_out_edges:
                    ind = weighted_edges.index({'src':e[0], 'dst':e[1],'weight': 0})
                    weighted_edges[ind]['weight'] = num_path[v]
                    num_path[v] = num_path[v] + num_path[e[1]]

        graph_with_weights = nx.DiGraph()
        graph_with_weights.add_nodes_from(graph.nodes)
        edges = []
        for e in weighted_edges:
            edges.append((e['src'], e['dst'], e['weight']))
        graph_with_weights.add_weighted_edges_from(edges)

        # root_nodes = [v for v, d in graph_with_weights.in_degree() if d == 0]
        # leaf_nodes = [v for v, d in graph_with_weights.out_degree() if d == 0]

        print("--- printing edges with non-0 weights from total {} edges ---".format(len(weighted_edges)))

        json_output_edge_dst_increment_dict = {}  # Original
        json_output_edge_dst_edge_dict = {}  # Original
        json_output_non_action_increment_dict = {}  # Original
        json_output_final_edge_dst_increment_dict = {}
        json_output_final_edge_dst_edge_dict = {}
        json_output_final_non_action_increment_dict = {}
        json_output_final_non_action_increment_rootword_dict = {}
        # Get the original increment plan
        for edge in weighted_edges:
            if edge['weight'] == 0:
                continue
            else:
                json_output_edge_dst_increment_dict[edge['dst']] = edge['weight']
                json_output_edge_dst_edge_dict[edge['dst']] = (edge['src']+" -> "+edge['dst'])
                print(edge)
                if UNIQUE_ACTION_SIG not in edge['dst']:
                    print("[WARNING] Target instrumentation point {} is NOT an action!".format(edge['dst']))
                    json_output_non_action_increment_dict[edge['dst']] = edge['weight']
                    # If the edge destination is a table, try to relocate it...
                    if edge['dst'] in table2actions_dict:
                        if graph_with_weights.in_degree()[edge['dst']] == 1:
                            print("[INFO] For a table, OK to merge the weights of the edge its actions as its in_degree == 1")
                            if graph_with_weights.out_degree()[edge['dst']] == len(table2actions_dict[edge['dst']]):
                                print("Merge the weights to the edges to its actions...")
                                out_edges_to_merge = graph_with_weights.out_edges(edge['dst'], data=True)
                                for out_edge_to_merge in out_edges_to_merge:
                                    print("Increment weight of {0} by {1}".format(out_edge_to_merge, edge['weight']))
                                    graph_with_weights[out_edge_to_merge[0]][out_edge_to_merge[1]]['weight'] += edge['weight']
                                graph_with_weights[edge['src']][edge['dst']]['weight'] = 0
                            else:
                                raise Exception("out_degree() {0} != {1} actions".format(graph_with_weights.out_degree()[edge['dst']], len(table2actions_dict[edge['dst']])))
                        else:
                            print("[WARNING] Can't instrument this edge for this dst table!")
                            # raise Exception("Can't instrument this edge for this dst table!")
                    else:
                        print("[WARNING] Can't relocate this dst conditonal!")
                        # raise Exception("Can't relocate this dst conditonal!")
                    # raise Exception("ERR! Target instrumentation point {} is NOT an action!".format(edge['dst']))
        # Get final plan
        for edge in graph_with_weights.edges(data=True):
            if edge[2]['weight'] != 0:
                json_output_final_edge_dst_increment_dict[edge[1]] = edge[2]['weight']
                json_output_final_edge_dst_edge_dict[edge[1]] = (edge[0]+" -> "+edge[1])
                if UNIQUE_ACTION_SIG not in edge[1]:
                    json_output_final_non_action_increment_dict[edge[1]] = edge[2]['weight']
                    json_output_final_non_action_increment_rootword_dict[edge[1]] = "mvbl_"+str(graph_idx)+"_"+edge[0]+"_"+edge[1]

        return graph_with_weights, json_output_edge_dst_increment_dict, json_output_edge_dst_edge_dict, json_output_non_action_increment_dict, json_output_final_edge_dst_increment_dict, json_output_final_edge_dst_edge_dict, json_output_final_non_action_increment_dict, json_output_final_non_action_increment_rootword_dict

    def estimate_stages(self, original_graph):
        graph = copy.deepcopy(original_graph)
        
        latest_stage = 0
        stage2tables_dict = dict()
        table_to_stage_dict = dict()

        # print(len(graph.nodes))

        while (len(graph.nodes) > 0):
            current_nodes = [v for v, d in graph.in_degree() if d == 0]
            stage2tables_dict[latest_stage] = current_nodes
            for node in current_nodes:
                graph.remove_node(node)

            latest_stage += 1

        return stage2tables_dict

    def append_table_action_edges(self, table2actions_dict, edges, leaf_nodes, root_nodes, node2label_dict):
        print("--- append_table_action_edges ---")

        full_graph_nodes = list(node2label_dict.values())
        print("--- full_graph_nodes inherited from the original table_graph ---") 
        print(full_graph_nodes)

        # LC_TODO: Assuming no table predication logic for now, i.e., nodes are tables or conditionals, and edges are T/F.
        # One could extend it later to match the edge label to the (renamed) action name and add action nodes selectively
        edges_to_del = []
        edges_to_add = []
        for e in edges:
            print("--- {} ---".format(e))
            # dst can be a conditional or table
            if e['src'] in table2actions_dict.keys():
                print("Mark del")
                edges_to_del.append(e)
                for action in table2actions_dict[e['src']]:
                    edges_to_add.append({'src': e['src'], 'dst': action, 'label': ''})
                    edges_to_add.append({'src': action, 'dst': e['dst'], 'label': ''})
                    print("Append edge {}".format({'src': e['src'], 'dst': action, 'label': ''}))
                    print("Append edge {}".format({'src': action, 'dst': e['dst'], 'label': ''}))
                    full_graph_nodes.append(action)
                    print("Append node: {}".format(action))
                # Note the edge case when the dst if also a leaf table
                if e['dst'] in leaf_nodes and e['dst'] in table2actions_dict.keys():
                    for action in table2actions_dict[e['dst']]:
                        edges_to_add.append({'src': e['dst'], 'dst': action, 'label': ''})
                        print("Append edge {}".format({'src': e['dst'], 'dst': action, 'label': ''}))
                        full_graph_nodes.append(action)
                        print("Append node: {}".format(action))                        
            # Otherwise, only if the destination is a leaf table
            elif e['dst'] in leaf_nodes and e['dst'] in table2actions_dict.keys():
                for action in table2actions_dict[e['dst']]:
                    edges_to_add.append({'src': e['dst'], 'dst': action, 'label': ''})
                    print("Append edge {}".format({'src': e['dst'], 'dst': action, 'label': ''}))
                    full_graph_nodes.append(action)
                    print("Append node: {}".format(action)) 

        print("--- Consider the edge case when a node is isolated ---")
        for node in full_graph_nodes:
            if node in leaf_nodes and node in root_nodes:
                print("[INFO] Identified isolated node: {}".format(node))
                if node in table2actions_dict.keys():
                    for action in table2actions_dict[node]:
                        edges_to_add.append({'src': node, 'dst': action, 'label': ''})
                        print("Append edge {}".format({'src': node, 'dst': action, 'label': ''}))
                        full_graph_nodes.append(action)
                        print("Append node: {}".format(action)) 
                else:
                    raise Exception("node {} not in table2actions_dict.keys()!".format(node))

        full_graph_edges = edges + edges_to_add
        for de in edges_to_del:
            print("de in edges_to_del: {}".format(de))
            if de in edges:
                print("edges.remove: {}".format(de))
                full_graph_edges.remove(de)
        print("--- full_graph_nodes ---")
        print(full_graph_nodes)
        print("--- full_graph_edges ---")
        print(full_graph_edges)

        return full_graph_nodes, full_graph_edges

    def eliminate_edge(self, edge, edges, nodes,edge_to_del):
        if edge['dst'] in nodes:
            return edge['dst']
        else:
            for e in edges:
                if(e['src'] == edge['dst']):
                    nxt_node = self.eliminate_edge(e, edges, nodes,edge_to_del)
                    edge_to_del.append(e)
                    break
                else:
                    # print("returning -1", e)
                    nxt_node = -1
            return nxt_node


    def extract_tables_actions_bmv2(self, filename, actions, cnt_blk= 'MyIngress'):
        data = None
        name_to_action = {}
        tbl_to_action = {}
        tbl_to_table = {}
        with open(filename, 'r') as f:
            data = json.load(f)

        for name in data['pipelines']:
            for table in name["tables"]:
                if cnt_blk in table["name"]:
                    name_to_action[str(table["name"]) if(cnt_blk in table["name"]) else table["name"]] =  [str(x) for x in table["actions"]]
                else:
                    tbl_to_action[str(table["name"])] =  [str(x) for x in table["actions"]]
                    tbl_to_table[str(table["name"])] = [str(x) for x in table["next_tables"].values()]
        return name_to_action

    def extract_stages_bmv2(self, contextFile):
        data = None
        with open(contextFile, 'r') as f:
            data = json.load(f)

        for pipeline in data['pipelines']:
            for table_information in pipeline['tables']:
                for key, item in table_information.items():
                    print("key", key)
                    print("item", item)
        exit()

    def extract_conditionals(self, filename):
        """This function creates dictionary of conditionals with its 
        corresponding next actions based on the condition 
        evaluation "True or False"."""
        data = None
        with open(filename, 'r') as f:
            data = json.load(f)

        #If any duplicate Conditions present then append "##1", "##2" etc. at the end of the conditionals.
        node_to_condition = {}
        for name in data['pipelines']:
            for condition in name['conditionals']:
                if 'source_info' in condition.keys():
                    if str(condition['source_info']['source_fragment']) not in node_to_condition.values():
                        node_to_condition[str(condition['name'])] = str(condition['source_info']['source_fragment'])
                    else:
                        cnt = 1
                        key = node_to_condition.keys()
                        values = node_to_condition.values()
                        # ind = key[values.index(str(condition['source_info']['source_fragment']))]
                        while(1):
                            if str(condition['source_info']['source_fragment'])+"##"+str(cnt) in node_to_condition:
                                cnt += 1
                            else:
                                node_to_condition[str(condition['name'])] = str(condition['source_info']['source_fragment']) + "##" + str(cnt)
                                break

                else:
                    if str(condition['true_next']) != 'None' and str(condition['true_next']) not in node_to_condition.values():
                        node_to_condition[str(condition['name'])] = str(condition['true_next'])
                    else:
                        cnt = 1
                        key = node_to_condition.keys()
                        values = node_to_condition.values()
                        # ind = key[values.index(name)]
                        while(1):
                            if str(condition['true_next'])+"##"+str(cnt) in node_to_condition.values():
                                cnt += 1
                            else:
                                node_to_condition[str(condition['name'])] = str(condition['true_next']) + "##" + str(cnt)
                                break
                    if str(condition['false_next']) != 'None' and str(condition['false_next']) not in node_to_condition.values():
                        node_to_condition[str(condition['name'])] = str(condition['false_next'])
                    else:
                        cnt = 1
                        key = node_to_condition.keys()
                        values = node_to_condition.values()
                        # ind = key[values.index(name)]
                        while(1):
                            if str(condition['false_next'])+"##"+str(cnt) in node_to_condition.values():
                                cnt += 1
                            else:
                                node_to_condition[str(condition['name'])] = str(condition['false_next']) + "##" + str(cnt)
                                break

        conditions_to_nextstep = {}
        for name in data['pipelines']:
            for condition in name['conditionals']:
                if 'source_info' in condition.keys() and str(condition['source_info']['source_fragment']) not in conditions_to_nextstep.keys():
                    conditions_to_nextstep[str(condition['source_info']['source_fragment'])] = {'true_next':str(condition['true_next']), 
                                                                                    'false_next':str(condition['false_next'])}
                elif 'source_info' in condition.keys() and str(condition['source_info']['source_fragment']) in conditions_to_nextstep.keys():
                    cnt = 1
                    while(1):
                        if str(condition['source_info']['source_fragment'])+"##"+str(cnt) in conditions_to_nextstep.keys():
                            cnt += 1
                        else:
                            conditions_to_nextstep[str(condition['source_info']['source_fragment'])+ "##" + str(cnt)] = {'true_next':str(condition['true_next']), 
                                                                                    'false_next':str(condition['false_next'])}
                            break

        to_delete = []
        for node in node_to_condition:
            if node_to_condition[node] in node_to_condition.keys():
                to_delete.append(node)

        if len(to_delete) > 0:
            for n in to_delete:
                del node_to_condition[n]
        return conditions_to_nextstep



    def extract_actions_bmv2(self, filename, cnt_blk='MyIngress'):
        data = None
        action_list = []
        with open(filename, 'r') as f:
            data = json.load(f)

        for action in data['actions']:
            if cnt_blk in str(action):
                action_list.append(str(action['name']))
        
        action_list = list(set(action_list))
        return action_list


    def get_edges_from_dot(self, dotfile, nodes, ignored_nodes):
        print("--- get_edges_from_dot ---")
        original_src_dst_label = []
        dot_graph = pydotplus.graphviz.graph_from_dot_file(dotfile) 
        for subGraph in dot_graph.get_subgraphs():
            for e in subGraph.get_edge_list():
                # print("--- src: {0}, dst: {1}, label: {2} ---".format(e.get_source(), e.get_destination(), e.obj_dict["attributes"]['label']))
                original_src_dst_label.append({"src":e.get_source(),"dst":e.get_destination(), "label":e.obj_dict["attributes"]['label']})

        renamed_src_dst_label = []
        for e in original_src_dst_label:
            print("--- src: {0}, dst: {1}, label: {2} ---".format(e['src'], e['dst'], e['label']))
            if e['src'] in nodes.keys() and e['dst'] in nodes.keys():
                print("Append to renamed_src_dst_label: {}".format({'src':nodes[e['src']], 'dst':nodes[e['dst']], 'label': e['label']}))
                renamed_src_dst_label.append({'src':nodes[e['src']], 'dst':nodes[e['dst']], 'label': e['label']})
            elif e['src'] in nodes.keys() and e['dst'] not in nodes.keys():
                print("[WARNING] Skipped {}".format({'src':nodes[e['src']], 'dst':ignored_nodes[e['dst']], 'label': e['label']}))
                if ignored_nodes[e['dst']] == "__EXIT__" or ignored_nodes[e['dst']] == "tbl_act":
                    continue
                else:
                    print("[ERROR] Unexpected edge!")
                    raise Exception("ERR!")
            elif e['src'] not in nodes.keys() and e['dst'] in nodes.keys():
                print("[WARNING] Skipped {}".format({'src':ignored_nodes[e['src']], 'dst':nodes[e['dst']], 'label': e['label']}))
                if ignored_nodes[e['src']] == "__START__":
                    continue
                else:
                    # Can't be ignored_nodes[e['src']] == "tbl_act"!
                    print("[ERROR] Unexpected edge!")
                    raise Exception("ERR!")
            else:
                print("[WARNING] Skipped {}".format({'src':ignored_nodes[e['src']], 'dst':ignored_nodes[e['dst']], 'label': e['label']}))
                if (ignored_nodes[e['src']] == "tbl_act" and ignored_nodes[e['dst']] == "__EXIT__") or (ignored_nodes[e['src']] == "__START__" and ignored_nodes[e['dst']] == "__EXIT__"):
                    continue
                else:
                    print("[ERROR] Unexpected edge!")
                    raise Exception("ERR!")
        print("--- renamed_src_dst_label ---")
        pretty_print_dict(renamed_src_dst_label)
        return renamed_src_dst_label


def parse_arguments():
    parser = argparse.ArgumentParser(description='Ball_larus split')
    parser.add_argument('-p','--prog', help='Program name', required=True)
    parser.add_argument('-d','--dotfile', help='Dot file of the program', required=True)
    parser.add_argument('-g','--gress', help='Ingress or egress', required=True)
    parser.add_argument('-c','--contextfile', help='Context json file of the program', required=True)
    parser.add_argument('-t','--target', help='Pick one from bmv2, p414, p416', choices=['bmv2', 'p414', 'p416'], default='p414')
    return parser.parse_args()

def main():
    args = parse_arguments()
    GraphParser(args.prog, args.dotfile, args.contextfile, args.target, args.gress)


if __name__ == '__main__':
    main()