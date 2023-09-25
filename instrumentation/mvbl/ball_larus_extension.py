import json
from collections import defaultdict
import networkx as nx
from utils import visualize_digraph
from constants import *
from utils import check_dag_connected, draw_graph
import logging
logging.basicConfig(level=logging.WARNING)

class ExtendBallLarus(object):
    def __init__(self, pulpSolver, full_graph, table2actions_dict, table_conditional_to_exit, global_root_node, json_output_dict, prog_name, direction, if_conditions_list):
        logging.info("\n====== Update full_graph with exit ======")
        full_graph_new_edges = []
        virtual_start_node = "VIRTUAL_START"
        virtual_exit_node = "VIRTUAL_EXIT"
        
        for node in full_graph.nodes:
            # For action, check if its table is in edge
            if UNIQUE_ACTION_SIG in node:
                if node.split(UNIQUE_ACTION_SIG)[1] in table_conditional_to_exit:
                    full_graph_new_edges.append([node, virtual_exit_node])
            # If conditional
            elif node not in table2actions_dict:
                if node in table_conditional_to_exit:
                    full_graph_new_edges.append([node, virtual_exit_node])
        full_graph_new_edges.append([virtual_start_node, global_root_node])

        logging.info("--- full_graph_new_edges ---")
        # full_graph.add_node(virtual_node_exit)
        logging.info(full_graph_new_edges)
        for edge in full_graph_new_edges:
            full_graph.add_edge(edge[0], edge[1])
        visualize_digraph(full_graph, "full_graph with exit, start")

        full_graph_with_exit_leaf_nodes = [v for v, d in full_graph.out_degree() if d == 0]
        logging.info("--- full_graph_with_exit_leaf_nodes ---")
        logging.info(full_graph_with_exit_leaf_nodes)
        if len(full_graph_with_exit_leaf_nodes) != 1 or full_graph_with_exit_leaf_nodes[0] != virtual_exit_node:
            raise Exception("Must be a single EXIT for the new full graph!")

        json_output_dict[JSON_OUTPUT_KEY_NUM_VARS] = len(pulpSolver.var_to_bits)
        # Based on the solver result, construct the sub-DAGs with only the tables and edges involved
        # Note that dummy edges should be handled properly
        new_subgraphs = []
        for graph_number, number_of_bits in enumerate(pulpSolver.var_to_bits):
            json_output_dict[str(graph_number)] = {
                JSON_OUTPUT_KEY_NUM_BITS_ORIGINAL : number_of_bits
            }
            logging.info("\n====== Constructing subgraph {0} for MVBL based on table_graph and pulp result ======".format(graph_number))
            logging.info("--- List of (table/conditonal) nodes included from pulp ---")
            included_table_conditional = pulpSolver.subgraph_to_tables[graph_number]
            logging.info(included_table_conditional)
            included_table_conditional_action = []
            for node in full_graph.nodes:
                if node in included_table_conditional:
                    included_table_conditional_action.append(node)
                elif UNIQUE_ACTION_SIG in node and node.split(UNIQUE_ACTION_SIG)[1] in included_table_conditional:
                    included_table_conditional_action.append(node)
            logging.info("--- included_table_conditional_action ---")
            logging.info(included_table_conditional_action)

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
            logging.info("--- new_subgraph_edges ---")
            logging.info(new_subgraph_edges)
            new_subgraph.add_weighted_edges_from(new_subgraph_edges)
            visualize_digraph(new_subgraph, "new_subgraph")

            # Need to be extend the prior sub-DAG with virtual nodes/edges
            virtual_nodes = []
            virtual_edges = []
            # BL requires a single START, which may not be true if the sub-DAG has isolated nodes (not weakly connected)
            logging.info("--- Always add a virtual start node, no harm if it is redundant---")
            new_subgraph_root_nodes = [v for v, d in new_subgraph.in_degree() if d == 0]
            new_subgraph_leaf_nodes = [v for v, d in new_subgraph.out_degree() if d == 0]
            virtual_nodes.append(virtual_start_node)
            for new_subgraph_root_node in new_subgraph_root_nodes:
                virtual_edges.append([virtual_start_node, new_subgraph_root_node])

            virtual_nodes.append(virtual_exit_node)
            for new_subgraph_leaf_node in new_subgraph_leaf_nodes:
                virtual_edges.append([new_subgraph_leaf_node, virtual_exit_node])
            
            # logging.info("--- Check if needed to add a virtual node from start node, to avoid false encoding of path 0 ---")
            # add_virtual_node_from_start = False
            # all_paths = nx.all_simple_paths(full_graph, virtual_start_node, virtual_exit_node)
            # for path in all_paths:
            #     if set(path[1:-1]).isdisjoint(included_table_conditional_action):
            #         logging.info("path {0}'s component {1} isdisjoint from {2}".format(path, path[1:-1], included_table_conditional_action))
            #         add_virtual_node_from_start = True
            #         break
            # if add_virtual_node_from_start:
            #     logging.info("add_virtual_node_from_start!")
            #     virtual_node = virtual_node_prefix+virtual_start_node
            #     virtual_nodes.append(virtual_node)
            #     virtual_edges.append([virtual_start_node, virtual_node])
            # else:
            #     logging.info("NOT add_virtual_node_from_start!")

            # # 3. Similarly, check every node in the sub-DAG if there is a need to add a branching virtual node/edge, to avoid false encoding
            # for node_to_branch in included_table_conditional_action:
            #     # If the node is the leaf node, don't care as it will not lead to false encoding
            #     if node_to_branch in new_subgraph_leaf_nodes:
            #         continue
            #     all_paths = nx.all_simple_paths(full_graph, node_to_branch, virtual_node_exit)
            #     for path in all_paths:
            #         if set(path[1:-1]).isdisjoint(included_table_conditional_action):
            #             virtual_node = virtual_node_prefix+node_to_branch
            #             virtual_nodes.append(virtual_node)
            #             virtual_edges.append([node_to_branch, virtual_node])
            #             break
            logging.debug("--- virtual_nodes ---")
            logging.info(virtual_nodes)
            logging.info("--- virtual_edges ---")
            logging.info(virtual_edges)
            for node_to_add in virtual_nodes:
                new_subgraph.add_node(node_to_add)
            for edge_to_add in virtual_edges:
                new_subgraph.add_edge(edge_to_add[0], edge_to_add[1])
            visualize_digraph(new_subgraph, "new_subgraph after virtual nodes")
            # draw_graph(new_subgraph)


            logging.info("--- Check if each subgraph is a DAG and weakly connected ---")
            is_weak, cycles = check_dag_connected(new_subgraph)
            if not is_weak or len(cycles) != 0:
                raise Exception("Not weakly connected or with loops!")
                sys.exit()

            new_subgraphs.append(new_subgraph)
            json_output_dict[str(graph_number)][JSON_OUTPUT_KEY_NODES] = sorted(list(new_subgraph.nodes), key=lambda e: e)
            json_output_dict[str(graph_number)][JSON_OUTPUT_EDGES] = sorted(list(nx.generate_edgelist(new_subgraph, delimiter=' -> ', data=False)), key=lambda e: e)

        graphs_with_weights = []
        # print("old_num_graphs", len(new_subgraphs))
        new_subgraphs = self.merge_conditional_graphs(new_subgraphs, if_conditions_list)
        # print("new_num_graphs", len(new_subgraphs))
        for idx, graph in enumerate(new_subgraphs):
            logging.debug("\n====== Running BL for subgraph {0} ======".format(idx))
            logging.debug("----- Get BL plan for variable {0} additions------".format(idx))

            graph_with_weights, json_output_edge_dst_increment_dict, json_output_edge_dst_edge_dict, json_output_non_action_increment_dict, json_output_final_edge_dst_increment_dict, json_output_final_edge_dst_edge_dict, json_output_final_non_action_increment_dict, json_output_final_non_action_increment_rootword_dict, json_output_nonzero_edge_dict, json_output_final_nonzero_edge_dict = self.ball_larus(graph, table2actions_dict, idx, full_graph)

            json_output_dict[str(idx)][JSON_OUTPUT_KEY_EDGE_DST_INCREMENT_DICT] = json_output_edge_dst_increment_dict
            json_output_dict[str(idx)][JSON_OUTPUT_KEY_EDGE_DST_EDGE_DICT] = json_output_edge_dst_edge_dict
            json_output_dict[str(idx)][JSON_OUTPUT_KEY_NON_ACTION_INCREMENT_DICT] = json_output_non_action_increment_dict
            json_output_dict[str(idx)][JSON_OUTPUT_KEY_FINAL_EDGE_DST_EDGE_DICT] = json_output_final_edge_dst_increment_dict
            json_output_dict[str(idx)][JSON_OUTPUT_KEY_FINAL_EDGE_INCREMENT_DICT] = json_output_final_edge_dst_edge_dict
            json_output_dict[str(idx)][JSON_OUTPUT_KEY_FINAL_NON_ACTION_INCREMENT_DICT] = json_output_final_non_action_increment_dict
            json_output_dict[str(idx)][JSON_OUTPUT_KEY_FINAL_NON_ACTION_INCREMENT_ROOTWORD_DICT] = json_output_final_non_action_increment_rootword_dict
            json_output_dict[str(idx)][JSON_OUTPUT_KEY_NONZERO_EDGE_DICT] = json_output_nonzero_edge_dict
            json_output_dict[str(idx)][JSON_OUTPUT_KEY_FINAL_NONZERO_EDGE_DICT] = json_output_final_nonzero_edge_dict

            # Check if there is a node being edge dst multiple times
            if len(json_output_final_nonzero_edge_dict) != len(json_output_final_edge_dst_edge_dict):
                edge_dst_tmp = [edge.split(' -> ')[1] for edge in json_output_final_nonzero_edge_dict.keys()]
                counter = defaultdict(int)
                for s in edge_dst_tmp:
                    counter[s] += 1
                duplicates = [s for s, count in counter.items() if count > 1]
                json_output_dict[JSON_OUTPUT_KEY_FINAL_INCAST_EDGE_DST_LIST] = duplicates

            graphs_with_weights.append(graph_with_weights)

            # for edge_dst_to_increment in json_output_dict[str(idx)][JSON_OUTPUT_KEY_FINAL_EDGE_INCREMENT_DICT]:
            #     if virtual_node_prefix in edge_dst_to_increment:
            #         raise Exception("virtual_node_prefix in {}".format(edge_dst_to_increment))

        sum_num_paths = 0
        for idx, graph in enumerate(graphs_with_weights):
            json_output_dict[str(idx)][JSON_OUTPUT_KEY_ENCODING_TO_PATH_DICT] = {}
            logging.info("\n====== logging.infoing path to encoding for sub-DAG {0} ======".format(idx))
            subdag_root_nodes = [v for v, d in graph.in_degree() if d == 0]
            subdag_leaf_nodes = [v for v, d in graph.out_degree() if d == 0]
            logging.info("--- subdag_root_nodes ---")
            logging.info(subdag_root_nodes)
            if len(subdag_root_nodes) != 1:
                raise Exception("subdag_root_nodes must have exactly 1 START_VIRTUAL!")
            logging.info("--- subdag_leaf_nodes ---")
            logging.info(subdag_leaf_nodes)
            all_paths_weights = []
            for root in subdag_root_nodes:
                for leaf in subdag_leaf_nodes:
                    # This sub-dag must be a single (conditional) node
                    if root == leaf:
                        raise Exception("root == leaf! Should never happen!")
                        # if len(subdag_root_nodes) != 1 or len(subdag_leaf_nodes) != 1:
                        #     raise Exception("len(subdag_root_nodes) != 1 or len(subdag_leaf_nodes) != 1!")
                        #     sys.exit()
                        # all_paths_weights.append(([root], 0))
                    else:
                        paths = list(nx.all_simple_paths(graph, root, leaf))
                        for path in paths:
                            weight = sum(graph.get_edge_data(path[i], path[i + 1])['weight'] for i in range(len(path) - 1))
                            all_paths_weights.append((path, weight))
            all_paths_weights.sort(key=lambda x: x[1])
            json_output_dict[str(idx)][JSON_OUTPUT_KEY_NUM_PATHS] = len(all_paths_weights)

            # Because we add 1 path, typically it should be accomodated due to overestimation of paths, but still check and increment by 1b
            if len(all_paths_weights) > 2**json_output_dict[str(idx)][JSON_OUTPUT_KEY_NUM_BITS_ORIGINAL]:
                if json_output_dict[str(idx)][JSON_OUTPUT_KEY_NUM_BITS_ORIGINAL] >= 32: 
                    raise Exception("Original number of bits is already 32, can't accomodate the additional virtual path!")
                else:
                    json_output_dict[str(idx)][JSON_OUTPUT_KEY_NUM_BITS] = json_output_dict[str(idx)][JSON_OUTPUT_KEY_NUM_BITS_ORIGINAL] + 1
            else:
                json_output_dict[str(idx)][JSON_OUTPUT_KEY_NUM_BITS] = json_output_dict[str(idx)][JSON_OUTPUT_KEY_NUM_BITS_ORIGINAL]
            # Now round the bit number...
            json_output_dict[str(idx)][JSON_OUTPUT_KEY_NUM_BITS] = ((json_output_dict[str(idx)][JSON_OUTPUT_KEY_NUM_BITS] + 7) // 8) * 8

            logging.info("--- Validate # of paths vs bit num ---")
            if len(all_paths_weights) > 2**json_output_dict[str(idx)][JSON_OUTPUT_KEY_NUM_BITS]:
                raise Exception("len(all_paths_weights) {0} > 2**{1}!".format(len(all_paths_weights), json_output_dict[str(idx)][JSON_OUTPUT_KEY_NUM_BITS]))
            else:
                logging.info("# of paths {0} <= 2**{1}".format(len(all_paths_weights), json_output_dict[str(idx)][JSON_OUTPUT_KEY_NUM_BITS]))
            
            sum_num_paths += len(all_paths_weights)
            logging.info("--- Expecting all_paths_weights with weight from 0 to {} ---".format(len(all_paths_weights)))
            expected_weight = 0
            # LC_TODO: check every path is unique
            for path, weight in all_paths_weights:
                logging.info("Path: {0}, Total Weight: {1}".format(path, weight))
                if path[0] != virtual_start_node:
                    raise Exception("First node must be START_VIRTUAL!")
                else:
                    clean_plan = [u for u in path[1:] if virtual_exit_node not in u]
                    json_output_dict[str(idx)][JSON_OUTPUT_KEY_ENCODING_TO_PATH_DICT][str(weight)] = clean_plan
                if weight != expected_weight:
                    raise Exception("Unexpected weight! Possibly mis-assigned weights")
                expected_weight += 1
        json_output_dict[JSON_OUTPUT_KEY_SUM_NUM_PATHS] = sum_num_paths

        logging.info("\n====== Get the number of paths for the full_graph ===")
        full_root_nodes = [v for v, d in full_graph.in_degree() if d == 0]
        full_leaf_nodes = [v for v, d in full_graph.out_degree() if d == 0]
        logging.info("--- full_root_nodes ---")
        logging.info(full_root_nodes)
        logging.info("--- full_leaf_nodes ---")
        logging.info(full_leaf_nodes)
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
        logging.info("--- len(global_paths): {} ---".format(len(global_paths)))
        json_output_dict[JSON_OUTPUT_KEY_GLOBAL_NUM_PATHS] = len(global_paths)
        json_output_dict[JSON_OUTPUT_KEY_GLOBAL_PATHS] = global_paths

        with open("plan/"+prog_name+"_"+direction+".json", 'w') as f:
            json.dump(json_output_dict, f, indent=4, sort_keys=True)
        
        logging.info("====== Completed ======")

    def merge_conditional_graphs(self, new_subgraphs, if_conditions_list):
        merged_subgraphs = []
        condition_only_graphs = set()

        for index, graph in enumerate(new_subgraphs):
            is_graph_condition_only = True
            for node in graph.nodes:
                if node[:4] == 'meta' or node[:3] == 'hdr' or node == 'VIRTUAL_START' or node == 'VIRTUAL_EXIT':
                    continue
                else:
                    logging.info("------------")
                    logging.info(node)
                    logging.info("------------")
                    is_graph_condition_only = False
                    # exit()
                    break
            if is_graph_condition_only:
                condition_only_graphs.add(index)
            else:
                merged_subgraphs.append(graph)
            # visualize_digraph(graph, "new subgraph")

        logging.info("condition_only_graphs", condition_only_graphs)

        # Selected nodes to merge. Key is the index in if_conditions_list, value is the set of nodes in each list
        if_conditions_selected_nodes = dict()
        graphs_to_merge = set()
        for graph_index in condition_only_graphs:
            candidates_to_merge = dict()
            total_nodes = 0
            current_graph_potential_merge = set()
            for node in new_subgraphs[graph_index].nodes:

                # Check which conditions it can potentailly merge with
                
                logging.info("current_node", node)
                if node == 'VIRTUAL_START' or node == 'VIRTUAL_EXIT':
                    total_nodes += 1
                    continue
                for if_condition_list_index, if_conditions in enumerate(if_conditions_list):
                    for condition_index, condition in enumerate(if_conditions):
                        logging.info("condition", condition)
                        if condition == node:
                            total_nodes += 1
                            current_graph_potential_merge.add(if_condition_list_index)
                            if if_condition_list_index not in candidates_to_merge:
                                candidates_to_merge[if_condition_list_index] = set()
                            candidates_to_merge[if_condition_list_index].add(condition_index)

            logging.info("total_nodes", total_nodes, "expected_nodes", len(new_subgraphs[graph_index].nodes))
            if len(current_graph_potential_merge) == 1 and total_nodes == len(new_subgraphs[graph_index].nodes):
                logging.info("merging graph")
                for if_condition_list_index, condition_indices in candidates_to_merge.items():
                    if if_condition_list_index not in if_conditions_selected_nodes:
                        if_conditions_selected_nodes[if_condition_list_index] = set()
                    for condition_index in condition_indices:
                        logging.info(type(if_condition_list_index), type(if_conditions_selected_nodes), type(condition_index))
                        if_conditions_selected_nodes[if_condition_list_index].add(condition_index)
                graphs_to_merge.add(graph_index)
            else:
                logging.info("unable to merge graph")

        logging.info('graphs_to_merge', graphs_to_merge)
        graphs_that_could_not_be_merged = condition_only_graphs - graphs_to_merge
        logging.info('graphs_that_could_not_be_merged', graphs_that_could_not_be_merged)
        for graph_index in graphs_that_could_not_be_merged:
            merged_subgraphs.append(new_subgraphs[graph_index])

        for if_condition_list_index, items_in_list in if_conditions_selected_nodes.items():
            sorted_items_to_merge = sorted(items_in_list)
            new_subgraph = nx.DiGraph()
            new_subgraph.add_edge('VIRTUAL_START', if_conditions_list[if_condition_list_index][sorted_items_to_merge[0]])
            for condition_index in range(1, len(sorted_items_to_merge)):
                new_subgraph.add_edge(if_conditions_list[if_condition_list_index][sorted_items_to_merge[condition_index - 1]], if_conditions_list[if_condition_list_index][sorted_items_to_merge[condition_index]])

            new_subgraph.add_edge(if_conditions_list[if_condition_list_index][sorted_items_to_merge[-1]], 'VIRTUAL_EXIT')
            visualize_digraph(new_subgraph, 'new merged graph created')
            merged_subgraphs.append(new_subgraph)

        return merged_subgraphs


    def ball_larus(self, graph, table2actions_dict, graph_idx, full_graph):
        weighted_edges = []
        for e in graph.edges:
            # logging.info(e)
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
                logging.info("--- out_edges for {} ---".format(v))
                logging.info(out_edges)
                # To avoid randomness in the generated out_edges, include the alphabetical sort in the last key
                # rev_topological sort only (in the sub-DAG) will still be random
                # sorted_out_edges = sorted(out_edges, key=lambda e: (rev_topological_order.index(e[1]), e))
                sorted_out_edges = sorted(out_edges, key=lambda e: e)
                logging.info("--- sorted_out_edges for {} ---".format(v))
                logging.info(sorted_out_edges)
                if list(sorted_out_edges) != list(out_edges):
                    logging.info("[WARNING] sorted_out_edges != out_edges")
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

        logging.info("--- logging.infoing edges with non-0 weights from total {} edges ---".format(len(weighted_edges)))

        json_output_edge_dst_increment_dict = {}  # Original
        json_output_edge_dst_edge_dict = {}  # Original
        json_output_non_action_increment_dict = {}  # Original
        json_output_final_edge_dst_increment_dict = {}
        json_output_final_edge_dst_edge_dict = {}
        json_output_final_non_action_increment_dict = {}
        json_output_final_non_action_increment_rootword_dict = {}
        json_output_nonzero_edge_dict = {}
        json_output_final_nonzero_edge_dict = {}
        # Get the original increment plan
        tables_applied_batch_incre = []
        for edge in weighted_edges:
            if edge['weight'] == 0:
                continue
            else:
                json_output_edge_dst_increment_dict[edge['dst']] = edge['weight']
                json_output_edge_dst_edge_dict[edge['dst']] = (edge['src']+" -> "+edge['dst'])
                json_output_nonzero_edge_dict[edge['src']+" -> "+edge['dst']] = edge['weight']
                logging.info(edge)
                if UNIQUE_ACTION_SIG not in edge['dst']:
                    logging.info("[WARNING] Target instrumentation point {} is NOT an action!".format(edge['dst']))
                    json_output_non_action_increment_dict[edge['dst']] = edge['weight']
                    # If the edge destination is a table, try to relocate it...
                    if edge['dst'] in table2actions_dict:
                        # Case 1: If sug-DAG in_degree is 1 (regardless of full in_degree)
                        if graph_with_weights.in_degree()[edge['dst']] == 1:
                            # Edge case: IN-conditional -> OUT-table -> IN-table
                            # if full_graph.in_degree()[edge['dst']] != 1:
                            #     raise Exception("full_graph.in_degree()[edge['dst']] != 1 for {}".format(edge['dst']))
                            logging.info("[INFO] For a table, OK to merge the weights of the edge its actions as its sub-DAG in_degree == 1")
                            tables_applied_batch_incre.append(edge['dst'])
                            if graph_with_weights.out_degree()[edge['dst']] == len(table2actions_dict[edge['dst']]):
                                logging.info("Merge the weights to the edges to its actions...")
                                out_edges_to_merge = graph_with_weights.out_edges(edge['dst'], data=True)
                                for out_edge_to_merge in out_edges_to_merge:
                                    logging.info("Increment weight of {0} by {1}".format(out_edge_to_merge, edge['weight']))
                                    graph_with_weights[out_edge_to_merge[0]][out_edge_to_merge[1]]['weight'] += edge['weight']
                                graph_with_weights[edge['src']][edge['dst']]['weight'] = 0
                            else:
                                raise Exception("out_degree() {0} != {1} actions".format(graph_with_weights.out_degree()[edge['dst']], len(table2actions_dict[edge['dst']])))
                        # Case 2: If full in_degree is 1, regardless of sub-DAG
                        elif full_graph.in_degree()[edge['dst']] == 1:
                            logging.info("Need to double check if the weight is still non-zero")
                            if edge['dst'] in tables_applied_batch_incre:
                                logging.info("edge['dst'] in tables_applied_batch_incre")
                                graph_with_weights[edge['src']][edge['dst']]['weight'] = 0
                                continue
                            logging.info("We can merge if all the edge weights are the same in sub-DAG")
                            predecessor_nodes = list(graph_with_weights.predecessors(edge['dst']))
                            is_all_actions = True
                            # For now just merge if all of them are actions with equal weights
                            for predecessor_node in predecessor_nodes:
                                if UNIQUE_ACTION_SIG not in predecessor_node:
                                    is_all_actions = False
                            if is_all_actions:
                                logging.info("Check if equal!")
                                weight_to_check = graph_with_weights[predecessor_node][edge['dst']]['weight']
                                logging.info("Weight: {}".format(weight_to_check))
                                for predecessor_node in predecessor_nodes:
                                    if graph_with_weights[predecessor_node][edge['dst']]['weight'] != weight_to_check:
                                        logging.info("Not equal!")
                                        continue
                                logging.info("Is equal!")
                                tables_applied_batch_incre.append(edge['dst'])
                                out_edges_to_merge = graph_with_weights.out_edges(edge['dst'], data=True)
                                for out_edge_to_merge in out_edges_to_merge:
                                    graph_with_weights[out_edge_to_merge[0]][out_edge_to_merge[1]]['weight'] += edge['weight']
                                graph_with_weights[edge['src']][edge['dst']]['weight'] = 0                                
                            else:
                                raise Exception("Predecessors are not all actions!")
                        else:
                            logging.info("[WARNING] Can't instrument this edge for this dst table!")
                            # raise Exception("Can't instrument this edge for this dst table!")
                    else:
                        logging.info("[WARNING] Can't relocate this dst conditonal!")
                        # raise Exception("Can't relocate this dst conditonal!")
                    # raise Exception("ERR! Target instrumentation point {} is NOT an action!".format(edge['dst']))
        # Get final plan
        for edge in graph_with_weights.edges(data=True):
            if edge[2]['weight'] != 0:
                json_output_final_edge_dst_increment_dict[edge[1]] = edge[2]['weight']
                json_output_final_edge_dst_edge_dict[edge[1]] = (edge[0]+" -> "+edge[1])
                json_output_final_nonzero_edge_dict[edge[0]+" -> "+edge[1]] = edge[2]['weight']
                if UNIQUE_ACTION_SIG not in edge[1]:
                    json_output_final_non_action_increment_dict[edge[1]] = edge[2]['weight']
                    json_output_final_non_action_increment_rootword_dict[edge[1]] = "mvbl_"+str(graph_idx)+"_"+edge[0]+"_"+edge[1]

        return graph_with_weights, json_output_edge_dst_increment_dict, json_output_edge_dst_edge_dict, json_output_non_action_increment_dict, json_output_final_edge_dst_increment_dict, json_output_final_edge_dst_edge_dict, json_output_final_non_action_increment_dict, json_output_final_non_action_increment_rootword_dict, json_output_nonzero_edge_dict, json_output_final_nonzero_edge_dict
