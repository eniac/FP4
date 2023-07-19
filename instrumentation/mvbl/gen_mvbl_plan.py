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

import json
def pretty_print_dict(dictionary):
    print(json.dumps(dictionary, indent=4, sort_keys=True))


class GraphParser(object):
    def __init__(self, dotfile_ing, jsonfile, input_type='p414', direction='ingress'):

        print("====== Extract node name to label mapping, filtering START, EXIT ======")
        node2label_dict = self.get_raw_nodes_from_dot(dotfile_ing)

        print("====== Extract edge dicts, renamed src, dst with the labels, skipped START, EXIT ======")
        renamed_edges = self.get_edges_from_dot(dotfile_ing, node2label_dict)

        print("====== Create a table graph with 0 weights ======")
        edges_tuples, table_graph = self.get_table_graph(renamed_edges)

        print("====== extract_stages_p4_14 ======")
        stage2tables_dict, table2actions_dict = self.extract_stages_p4_14(jsonfile, node2label_dict.values(), direction)
        # if input_type == 'bmv2':
            # conditions_to_nextstep = self.extract_conditionals(jsonfile)
            # actions = self.extract_actions_bmv2(jsonfile)
            # table2actions_dict = self.extract_tables_actions_bmv2(jsonfile, actions)

        print("====== Print all_actions ======")
        all_actions = []
        for action_list in table2actions_dict.values():
            all_actions.extend(action_list)
        all_actions = list(set(all_actions))
        print(all_actions)

        print("====== Check the consistency of the outputs from context.json and .dot ======")
        for stage, table_list in stage2tables_dict.items():
            for table in table_list:
                if table not in table_graph.nodes:
                    print("table: {} NOT in table_graph.nodes!".format(table))
                    sys.exit()

        # if input_type == 'bmv2':
        #     stage2tables_dict = self.estimate_stages(table_graph)

        print("====== Print leaf_nodes ======")
        leaf_nodes = [v for v, d in table_graph.out_degree() if d == 0]
        print(leaf_nodes)

        print("====== Add edges from tables to actions, actions to the next table ======")
        updated_edges = self.append_table_action_edges(table2actions_dict, renamed_edges, leaf_nodes)

        print("====== Create full graph ======")
        for e in updated_edges:
            edges_tuples.append((e['src'], e['dst'], 0))
        edges_tuples = list(set(edges_tuples))
        full_graph = nx.DiGraph()
        full_graph.add_weighted_edges_from(edges_tuples)

        print("====== Sanitize special chars ([^a-zA-Z0-9_], e.g., \`.\`,\(,\),) for all nodes (actually for conditionals) for pulp ======")
        new_node_mapping = self.sanitize_node_name(full_graph)
        # LC_TODO: get reverse mapping as well and validate it

        print("====== Create a sanitized graph ======")
        table_graph = nx.relabel_nodes(table_graph, new_node_mapping)
        full_graph =  nx.relabel_nodes(full_graph, new_node_mapping)

        print("=== Update table names in table2actions_dict ===")
        for old_name, new_name in new_node_mapping.items():
            if old_name in table2actions_dict:
                if old_name != new_name:
                    print("Unexpected rename of table old_name: {0} -> new_name: {1}".format(old_name, new_name))
                    table2actions_dict[new_name] = table2actions_dict.pop(old_name)
        print("=== Update table names in stage2tables_dict ===")
        for stage, table_list in stage2tables_dict.items():
            for i, table_or_condtional in enumerate(table_list):
                if table_or_condtional in new_node_mapping:
                    if table_or_condtional != new_node_mapping[table_or_condtional]:
                        print("Must be a rename of conditional old_name: {0} -> new_name: {1}".format(table_or_condtional, new_node_mapping[table_or_condtional]))
                        table_list[i] = new_node_mapping[table]
                else:
                    print("table_or_condtional {} not found in new_node_mapping!".format(table_or_condtional))
                    sys.exit()

        print("====== Check for cycles... ======")
        cycles = list(nx.simple_cycles(full_graph))
        if cycles:
            print("****** cycles found! ********")
            for cycle in cycles:
                print(cycle)
            print("****** end cycles found ********")
            print("****** It should be a DAG ********")
            sys.exit()

        from pulp_solver import PulpSolver
        print("====== Running PulpSolver ======")
        pulpSolver = PulpSolver(table_graph, stage2tables_dict, table2actions_dict)

        # LC_TODO: maybe need to traverse the graph itself rather than edges to deal with dummy nodes (bypass them and direct connect)
        # Maybe directly use table_graph rather than full_graph (actually full_graph is useless even in pulp... as it is easy to go from a table_graph to full_graph)
        # The current implementation doesn't make sense!
        new_graph_edges = []
        for _ in range(len(pulpSolver.var_to_bits)):
            new_graph_edges.append([])
        for graph_number, number_of_bits in enumerate(pulpSolver.var_to_bits):
            print("====== Constructing subgraph {0} for MVBL ======".format(graph_number))
            for edge in full_graph.edges:
                print("--- {0} ---".format(edge))
                src = edge[0]
                dst = edge[1]

                node_src = src
                if '__' in src:
                    node_src = src.split('__')[0]
                if node_src in pulpSolver.subgraph_to_tables[graph_number]:
                    new_src = src
                else:
                    new_src = node_src

                node_dst = dst
                if '__' in dst:
                    node_dst = dst.split('__')[0]
                if node_dst in pulpSolver.subgraph_to_tables[graph_number]:
                    new_dst = dst
                else:
                    new_dst = node_dst

                if new_src == new_dst:
                    print("new_src {0} == new_dst {1}!!!".format(new_src, new_dst))
                    continue
                new_graph_edge = (new_src, new_dst, 0)
                print("new_graph_edge: {0}".format(new_graph_edge))
                new_graph_edges[graph_number].append(new_graph_edge)

        new_graphs = []
        for graph_edges in new_graph_edges:
            new_graph = nx.DiGraph()
            new_graph.add_weighted_edges_from(graph_edges)
            new_graphs.append(new_graph)

        print("====== Running BL for each subgraph ======")
        graphs_with_weights = []
        for idx, graph in enumerate(new_graphs):
            print("----- Variable {0} additions------".format(idx))
            edges_with_weights = self.ball_larus(graph)
            graph_with_weights = nx.DiGraph()
            edges = []
            for e in edges_with_weights:
                edges.append((e['src'], e['dst'], e['weight']))
            graph_with_weights.add_weighted_edges_from(edges)
            graphs_with_weights.append(graph_with_weights)
            # LC_TODO: print the mapping of values to paths using networkx

    def get_raw_nodes_from_dot(self, dotfile, cnt_blk='MyIngress'):
        node_name_label = {}
        dot_graph = pydotplus.graphviz.graph_from_dot_file(dotfile) 
        subgraphs = dot_graph.get_subgraphs()  
        for subG in subgraphs:
            for n in subG.get_node_list():
                label = n.obj_dict["attributes"]['label']
                print("--- name: {0}, label {1} ---".format(n.get_name(), label))
                if has_numbers(n.get_name()) and label not in ['__START__',"", '__EXIT__']:
                    # Do one function at a time
                    # label = remove_special_chars(label, cnt_blk)
                    if label not in node_name_label.values():
                        print("Write: {0}:{1}".format(n.get_name(), label))
                        node_name_label[n.get_name()] = label
                    else:
                        print("TODO: Duplicate label, exit!!!")
                        sys.exit()
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
        pretty_print_dict(node_name_label)
        return node_name_label

    def get_table_graph(self, renamed_edges):
        edges_tuples = []
        for e in renamed_edges:
            edges_tuples.append((e['src'], e['dst'], 0))
        edges_tuples = list(set(edges_tuples))
        table_graph = nx.DiGraph()
        table_graph.add_weighted_edges_from(edges_tuples)
        return edges_tuples, table_graph
 
    # Bug when there are multiple conditions in the if statement
    def extract_stages_p4_14(self, contextFile, node_labels, target_direction):
        context_data = None
        with open(contextFile, 'r') as f:
            context_data = json.load(f)

        stage2tables_dict = {}
        table2actions_dict = {}
        
        for table_information in context_data['tables']:
            table_name = table_information['name']
            table_type = table_information['table_type']
            direction = table_information["direction"]
            print("--- table_name: {0}, table_type: {1}, direction: {2} ---".format(table_name, table_type, direction))
            if target_direction != direction:
                print("Skipped")
                continue
            matched_to_node = False
            if table_type == "condition":
                # Rename the condition before matching
                renamed_condition = table_information['condition'][1:-1]
                if '$valid' in table_information['condition']:
                    renamed_condition = renamed_condition.replace("$valid", "isValid()")
                if ' == 1' in table_information['condition']:
                    renamed_condition = renamed_condition.replace(" == 1", "")
                print("raw condition: {0}, renamed: {1}".format(table_information['condition'], renamed_condition))
                for node_label in node_labels:
                    # print("Try matching to node_label: {}".format(node_label))
                    if renamed_condition in node_label:
                        print("Fuzzy matched to node_label: {}".format(node_label))
                        matched_to_node = True
                        for stage_information in table_information['stage_tables']:
                            stage_number = stage_information['stage_number']
                            if stage_number not in stage2tables_dict:
                                stage2tables_dict[stage_number] = []
                            stage2tables_dict[stage_number].append(node_label)
            elif table_type == "match":
                for node_label in node_labels:
                    if table_name == node_label:
                        print("Exact matched to node_label: {}".format(node_label))
                        matched_to_node = True
                        if table_name not in table2actions_dict:
                            table2actions_dict[table_name] = []
                        for action in table_information['actions']:
                            # Each action is renamed to table_name__action_name s.t. actions are unique to a table
                            table2actions_dict[table_name].append(table_name + "__" + action['name'])
                        for stage_information in table_information['match_attributes']['stage_tables']:
                            stage_number = stage_information['stage_number']
                            if stage_number not in stage2tables_dict:
                                stage2tables_dict[stage_number] = []
                            stage2tables_dict[stage_number].append(table_information['name'])
            elif table_type == "action":
                matched_to_node = True
                print("Skipped")
            else:
                print("Unknown table_type!")
                sys.exit()
            if not matched_to_node:
                print("Unmatched table! Exit")
                sys.exit()
        print("--- stage2tables_dict ---")
        pretty_print_dict(stage2tables_dict)
        print("--- table2actions_dict ---")
        pretty_print_dict(table2actions_dict)
        return stage2tables_dict, table2actions_dict

    def sanitize_node_name(self, graph):
        new_node_mapping = {}
        for node in graph.nodes:
            new_node = re.sub(r'\W+', '', node)
            print("--- {0} mapped to {1} ---".format(node, new_node))
            new_node_mapping[node] = new_node
        return new_node_mapping

    def ball_larus(self, graph):
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
                for e in graph.out_edges(v):
                    ind = weighted_edges.index({'src':e[0], 'dst':e[1],'weight': 0})
                    weighted_edges[ind]['weight'] = num_path[v]
                    num_path[v] = num_path[v] + num_path[e[1]]

        # print('------------')
        for edge in weighted_edges:
            if edge['weight'] == 0:
                continue
            print(edge)

        return weighted_edges

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


    def append_table_action_edges(self, table2actions_dict, edges, leaf_nodes):
        # LC_TODO: Assuming no table predication logic for now, i.e., nodes are tables or conditionals, and edges are T/F.
        # One could extend it later to match the edge label to the (renamed) action name and add action nodes selectively
        edges_to_del = []
        edges_to_add = []
        for e in edges:
            print("--- {} ---".format(e))
            # dst can be a conditional or table
            if e['src'] in table2actions_dict.keys():
                print("Del")
                edges_to_del.append(e)
                for action in table2actions_dict[e['src']]:
                    edges_to_add.append({'src': e['src'], 'dst': action, 'label': ''})
                    edges_to_add.append({'src': action, 'dst': e['dst'], 'label': ''})
                    print("Append {}".format({'src': e['src'], 'dst': action, 'label': ''}))
                    print("Append {}".format({'src': action, 'dst': e['dst'], 'label': ''}))
            # Otherwise, only if the destination is a leaf table
            elif e['dst'] in leaf_nodes and e['dst'] in table2actions_dict.keys():
                for action in table2actions_dict[e['dst']]:
                    edges_to_add.append({'src': e['dst'], 'dst': action, 'label': ''})
                    print("Append {}".format({'src': e['dst'], 'dst': action, 'label': ''}))

        edges = edges + edges_to_add
        for de in edges_to_del:
            if de in edges:
                edges.remove(de)
        return edges

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


    def get_edges_from_dot(self, dotfile, nodes):
        original_src_dst_label = []
        dot_graph = pydotplus.graphviz.graph_from_dot_file(dotfile) 
        for subGraph in dot_graph.get_subgraphs():
            for e in subGraph.get_edge_list():
                # print("--- src: {0}, dst: {1}, label: {2} ---".format(e.get_source(), e.get_destination(), e.obj_dict["attributes"]['label']))
                original_src_dst_label.append({"src":e.get_source(),"dst":e.get_destination(), "label":e.obj_dict["attributes"]['label']})

        renamed_src_dst_label = []
        for e in original_src_dst_label:
            print("--- src: {0}, dst: {1}, label: {2} ---".format(e['src'], e['dst'], e['label']))
            if(e['src'] in nodes.keys() and e['dst'] in nodes.keys()):
                # renamed_src_dst_label.append({'src':nodes[e['src']], 'dst':nodes[e['dst']], 'label': e['label'].replace('"','')})
                renamed_src_dst_label.append({'src':nodes[e['src']], 'dst':nodes[e['dst']], 'label': e['label']})
            else:
                print("[WARNING] Skipped due to unidentified node ID (typically START or EXIT))!")
                # sys.exit()
        pretty_print_dict(renamed_src_dst_label)
        return renamed_src_dst_label


def parse_arguments():
    parser = argparse.ArgumentParser(description='Ball_larus split')
    parser.add_argument('-d','--dotfile', help='Dot file of the program', required=True)
    parser.add_argument('-c','--contextfile', help='Context json file of the program', required=True)
    parser.add_argument('-t','--target', help='Pick one from bmv2, p414, p416', choices=['bmv2', 'p414', 'p416'], default='p414')
    return parser.parse_args()

def main():
    args = parse_arguments()
    GraphParser(args.dotfile, args.contextfile, args.target)


if __name__ == '__main__':
    main()