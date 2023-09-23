
import pydotplus
from utils import visualize_digraph, parse_and_transform
import networkx as nx
import json
import copy
import re
from utils import has_numbers, pretty_print_dict
from constants import *
"""
For each table - attach conditions to it
For each condition - write 
    true - > next table
    false -> next table

For each table and action -> find next table. If condition is attached to that table, it means next is the condition, not the table. This is also available in graph file so not really needed
if a condition goes to condition, it means it's an elseif
"""

class GraphParser(object):
    def __init__(self, prog_name, dotfile_ing, jsonfile, input_type='p414', direction='ingress'):
        print("\n====== program_name: {3}, dotfile_ing: {0}, jsonfile: {1}, direction: {2} ======".format(dotfile_ing, jsonfile, direction, prog_name))
        json_output_dict = {}

        print("\n====== Extract node name to label mapping, filtering START, EXIT, tbl_act ======")
        # OK to ignore start as there is always a real start node
        # Need to be careful with ignoring exit and tbl_act, we need to remember those paths
        node2label_dict, ignored_node2label_dict = self.get_raw_nodes_from_dot(dotfile_ing)

        if len(node2label_dict) == 0:
            json_output_dict[JSON_OUTPUT_KEY_NUM_VARS] = 0
            with open("plan/"+prog_name+"_"+direction+".json", 'w') as f:
                json.dump(json_output_dict, f, indent=4, sort_keys=True)
            print("====== Completed ======")
            return

        print("\n====== Extract edge dicts, renamed src, dst with the labels, skipped START, EXIT ======")
        # We should remember the table/conditional with path to exit to avoid false encoding
        renamed_edges, table_conditional_to_exit, if_conditions_list = self.get_edges_from_dot(dotfile_ing, node2label_dict, ignored_node2label_dict)

        print("\n====== table_conditional_to_exit ======")
        print(table_conditional_to_exit)

        print("\n====== Create a table graph with 0 weights ======")
        table_graph_edges_tuples, table_graph = self.get_table_graph(node2label_dict, renamed_edges)

        visualize_digraph(table_graph, "table_graph")

        print("\n====== extract_stages_p4_14 ======")
        stage2tables_dict, table2actions_dict, actions2Table2nextTable_dict = self.extract_stages_p4_14(jsonfile, node2label_dict.values(), direction)

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

        print("\n====== Print roots and leaf_nodes ======")
        table_graph_root_nodes = [v for v, d in table_graph.in_degree() if d == 0]
        table_graph_leaf_nodes = [v for v, d in table_graph.out_degree() if d == 0]
        print("--- table_graph_root_nodes ---")
        print(table_graph_root_nodes)
        if len(table_graph_root_nodes) != 1:
            raise Exception("len(table_graph_root_nodes) != 1")
        global_root_node = table_graph_root_nodes[0]
        print("--- table_graph_leaf_nodes ---")
        print(table_graph_leaf_nodes)

        print("\n====== Add edges from tables to actions, actions to the next table ======")
        updated_nodes, updated_edges = self.append_table_action_edges(table2actions_dict, renamed_edges, table_graph_leaf_nodes, table_graph_root_nodes, node2label_dict, actions2Table2nextTable_dict)

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

        full_graph_root_nodes = [v for v, d in full_graph.in_degree() if d == 0]
        if len(full_graph_root_nodes) != 1:
            raise Exception("len(full_graph_root_nodes) != 1")
        if full_graph_root_nodes[0] != global_root_node:
            raise Exception("full_graph_root_nodes[0] != global_root_node")
        full_graph_leaf_nodes = [v for v, d in full_graph.out_degree() if d == 0]

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

        print("\n=== Sanitize node names in table_conditional_to_exit ===")
        for i, node in enumerate(table_conditional_to_exit):
            if node in new_node_mapping:
                table_conditional_to_exit[i] = new_node_mapping[node]
        print("--- table_conditional_to_exit after sanitization ---")
        print(table_conditional_to_exit)

        print("\n====== Sanitize global_root_node ======")
        if global_root_node not in new_node_mapping:
            raise Exception("global_root_node not in new_node_mapping")
        else:
            global_root_node = new_node_mapping[global_root_node]

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

        print("\n====== Update full_graph with exit ======")
        from ball_larus_extension import ExtendBallLarus
        ball_larus = ExtendBallLarus(pulpSolver, full_graph, table2actions_dict, table_conditional_to_exit, global_root_node, json_output_dict)
    
    def sanitize_node_name(self, graph):
        new_node_mapping = {}
        reverse_new_node_mapping = {}
        for node in graph.nodes:
            new_node = re.sub(r'\W+', '', node)
            print("--- {0} mapped to {1} ---".format(node, new_node))
            new_node_mapping[node] = new_node
            reverse_new_node_mapping[new_node] = node
        return new_node_mapping, reverse_new_node_mapping

    def append_table_action_edges(self, table2actions_dict, edges, leaf_nodes, root_nodes, node2label_dict, actions2Table2nextTable_dict):
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
                    if action in actions2Table2nextTable_dict and e['src'] in actions2Table2nextTable_dict[action] and e['dst'] in actions2Table2nextTable_dict[action][e['src']]:
                        edges_to_add.append({'src': action, 'dst': e['dst'], 'label': ''})
                    elif "isValid" in e['dst'] or "==" in e['dst']:
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

    def get_raw_nodes_from_dot(self, dotfile, cnt_blk='MyIngress') -> tuple[dict[str,str], dict[str,str]]:
        node_name_label = {}
        ignored_node_name_label = {}
        dot_graph = pydotplus.graphviz.graph_from_dot_file(dotfile) 
        subgraphs = dot_graph.get_subgraphs()  
        for subG in subgraphs:
            for n in subG.get_node_list():
                if 'label' not in n.obj_dict["attributes"]:
                    continue
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

    def get_edges_from_dot(self, dotfile, nodes, ignored_nodes) -> tuple[list[str], list[str]]:
        nodes_to_exit = []
        if_conditions_list = []
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
                if e['label'] == "FALSE":
                    found = False
                    for current_list in if_conditions_list:
                        if nodes[e['src']] == current_list[-1]:
                            found = True
                            current_list.append(nodes[e['dst']])
                        if found:
                            break
                    if not found:
                        if_conditions_list.append([nodes[e['src']], nodes[e['dst']]])
                    

            elif e['src'] in nodes.keys() and e['dst'] not in nodes.keys():
                print("[WARNING] Skipped {}".format({'src':nodes[e['src']], 'dst':ignored_nodes[e['dst']], 'label': e['label']}))
                if ignored_nodes[e['dst']] == "__EXIT__" or ignored_nodes[e['dst']] == "tbl_act":
                    # Remember the from nodes to exit
                    nodes_to_exit.append(nodes[e['src']])
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
                    # tbl_act must be equivalent to EXIT
                    continue
                else:
                    print("[ERROR] Unexpected edge!")
                    raise Exception("ERR!")
        if_conditions_list = self.fix_if_conditions_list(if_conditions_list)
        print("--- renamed_src_dst_label ---")
        pretty_print_dict(renamed_src_dst_label)
        print("--- if_conditions_list ---")
        pretty_print_dict(if_conditions_list)
        return renamed_src_dst_label, nodes_to_exit, if_conditions_list

    def fix_if_conditions_list(self, if_conditions_list):
        if_conditions_list[:] = [x[:-1] for x in if_conditions_list if len(x) > 2]
        return if_conditions_list

    def get_table_graph(self, node2label_dict, renamed_edges) -> tuple[tuple[str,str,int], nx.DiGraph]:
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
        actions2Table2nextTable_dict = {}

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
                renamed_condition = parse_and_transform(table_information['condition'])
                print("raw condition: {0}, renamed: {1}".format(table_information['condition'], renamed_condition))
                for node_label in node_labels:
                    # print("Try matching to node_label: {}".format(node_label))
                    if renamed_condition == node_label:
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
                            print("table_name", table_name)
                            if 'action_format' in stage_information:
                                actions2Table2nextTable_dict = self.fill_table_action_dict(table_name, actions2Table2nextTable_dict, stage_information)
                            if 'ternary_indirection_stage_table' in stage_information:
                                actions2Table2nextTable_dict = self.fill_table_action_dict(table_name, actions2Table2nextTable_dict, stage_information['ternary_indirection_stage_table'])

                            # if table_name == "tiHandleIpv4":
                            #     print("************")
                            #     pretty_print_dict(actions2Table2nextTable_dict)
                            #     exit()



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

        print("--- actions2Table2nextTable_dict ---")
        pretty_print_dict(actions2Table2nextTable_dict)
        return stage2tables_dict, table2actions_dict, actions2Table2nextTable_dict

    def fill_table_action_dict(self, table_name, actions2Table2nextTable_dict, information):
        for action_information in information['action_format']:
            action_name = action_information["action_name"] + UNIQUE_ACTION_SIG + table_name
            if action_name not in actions2Table2nextTable_dict:
                actions2Table2nextTable_dict[action_name] = {}
            if table_name not in actions2Table2nextTable_dict[action_name]:
                actions2Table2nextTable_dict[action_name][table_name] = set()
            if "next_tables" not in action_information:
                continue
            for next_tables in action_information["next_tables"]:
                actions2Table2nextTable_dict[action_name][table_name].add(next_tables["next_table_name"])

        return actions2Table2nextTable_dict
