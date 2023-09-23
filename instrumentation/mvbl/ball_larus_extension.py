import networkx as nx
from utils import visualize_digraph
from constants import *

class ExtendBallLarus(object):
    def __init__(self, pulpSolver, full_graph, table2actions_dict, table_conditional_to_exit, global_root_node, json_output_dict):
        print("\n====== Update full_graph with exit ======")
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

        print("--- full_graph_new_edges ---")
        # full_graph.add_node(virtual_node_exit)
        print(full_graph_new_edges)
        for edge in full_graph_new_edges:
            full_graph.add_edge(edge[0], edge[1])
        visualize_digraph(full_graph, "full_graph with exit, start")

        full_graph_with_exit_leaf_nodes = [v for v, d in full_graph.out_degree() if d == 0]
        print("--- full_graph_with_exit_leaf_nodes ---")
        print(full_graph_with_exit_leaf_nodes)
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

            # Need to be extend the prior sub-DAG with virtual nodes/edges
            virtual_nodes = []
            virtual_edges = []
            # BL requires a single START, which may not be true if the sub-DAG has isolated nodes (not weakly connected)
            print("--- Always add a virtual start node, no harm if it is redundant---")
            new_subgraph_root_nodes = [v for v, d in new_subgraph.in_degree() if d == 0]
            new_subgraph_leaf_nodes = [v for v, d in new_subgraph.out_degree() if d == 0]
            virtual_nodes.append(virtual_start_node)
            for new_subgraph_root_node in new_subgraph_root_nodes:
                virtual_edges.append([virtual_start_node, new_subgraph_root_node])
            
            print("--- Check if needed to add a virtual node from start node, to avoid false encoding of path 0 ---")
            add_virtual_node_from_start = False
            all_paths = nx.all_simple_paths(full_graph, virtual_start_node, virtual_exit_node)
            for path in all_paths:
                if set(path[1:-1]).isdisjoint(included_table_conditional_action):
                    print("path {0}'s component {1} isdisjoint from {2}".format(path, path[1:-1], included_table_conditional_action))
                    add_virtual_node_from_start = True
                    break
            if add_virtual_node_from_start:
                print("add_virtual_node_from_start!")
                virtual_node = virtual_node_prefix+virtual_start_node
                virtual_nodes.append(virtual_node)
                virtual_edges.append([virtual_start_node, virtual_node])
            else:
                print("NOT add_virtual_node_from_start!")

            # 3. Similarly, check every node in the sub-DAG if there is a need to add a branching virtual node/edge, to avoid false encoding
            for node_to_branch in included_table_conditional_action:
                # If the node is the leaf node, don't care as it will not lead to false encoding
                if node_to_branch in new_subgraph_leaf_nodes:
                    continue
                all_paths = nx.all_simple_paths(full_graph, node_to_branch, virtual_node_exit)
                for path in all_paths:
                    if set(path[1:-1]).isdisjoint(included_table_conditional_action):
                        virtual_node = virtual_node_prefix+node_to_branch
                        virtual_nodes.append(virtual_node)
                        virtual_edges.append([node_to_branch, virtual_node])
                        break
            print("--- virtual_nodes ---")
            print(virtual_nodes)
            print("--- virtual_edges ---")
            print(virtual_edges)
            for node_to_add in virtual_nodes:
                new_subgraph.add_node(node_to_add)
            for edge_to_add in virtual_edges:
                new_subgraph.add_edge(edge_to_add[0], edge_to_add[1])
            visualize_digraph(new_subgraph, "new_subgraph after virtual nodes")

            print("--- Check if each subgraph is a DAG and weakly connected ---")
            is_weak, cycles = check_dag_connected(new_subgraph)
            if not is_weak or len(cycles) != 0:
                raise Exception("Not weakly connected or with loops!")
                sys.exit()

            new_subgraphs.append(new_subgraph)
            json_output_dict[str(graph_number)][JSON_OUTPUT_KEY_NODES] = sorted(list(new_subgraph.nodes), key=lambda e: e)
            json_output_dict[str(graph_number)][JSON_OUTPUT_EDGES] = sorted(list(nx.generate_edgelist(new_subgraph, delimiter=' -> ', data=False)), key=lambda e: e)

        graphs_with_weights = []
        for idx, graph in enumerate(new_subgraphs):
            print("\n====== Running BL for subgraph {0} ======".format(idx))
            print("----- Get BL plan for variable {0} additions------".format(idx))

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

            for edge_dst_to_increment in json_output_dict[str(idx)][JSON_OUTPUT_KEY_FINAL_EDGE_INCREMENT_DICT]:
                if virtual_node_prefix in edge_dst_to_increment:
                    raise Exception("virtual_node_prefix in {}".format(edge_dst_to_increment))

        sum_num_paths = 0
        for idx, graph in enumerate(graphs_with_weights):
            json_output_dict[str(idx)][JSON_OUTPUT_KEY_ENCODING_TO_PATH_DICT] = {}
            print("\n====== Printing path to encoding for sub-DAG {0} ======".format(idx))
            subdag_root_nodes = [v for v, d in graph.in_degree() if d == 0]
            subdag_leaf_nodes = [v for v, d in graph.out_degree() if d == 0]
            print("--- subdag_root_nodes ---")
            print(subdag_root_nodes)
            if len(subdag_root_nodes) != 1:
                raise Exception("subdag_root_nodes must have exactly 1 START_VIRTUAL!")
            print("--- subdag_leaf_nodes ---")
            print(subdag_leaf_nodes)
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

            print("--- Validate # of paths vs bit num ---")
            if len(all_paths_weights) > 2**json_output_dict[str(idx)][JSON_OUTPUT_KEY_NUM_BITS]:
                raise Exception("len(all_paths_weights) {0} > 2**{1}!".format(len(all_paths_weights), json_output_dict[str(idx)][JSON_OUTPUT_KEY_NUM_BITS]))
            else:
                print("# of paths {0} <= 2**{1}".format(len(all_paths_weights), json_output_dict[str(idx)][JSON_OUTPUT_KEY_NUM_BITS]))
            
            sum_num_paths += len(all_paths_weights)
            print("--- Expecting all_paths_weights with weight from 0 to {} ---".format(len(all_paths_weights)))
            expected_weight = 0
            # LC_TODO: check every path is unique
            for path, weight in all_paths_weights:
                print("Path: {0}, Total Weight: {1}".format(path, weight))
                if path[0] != virtual_start_node:
                    raise Exception("First node must be START_VIRTUAL!")
                else:
                    clean_plan = [u for u in path[1:] if virtual_node_prefix not in u]
                    json_output_dict[str(idx)][JSON_OUTPUT_KEY_ENCODING_TO_PATH_DICT][str(weight)] = clean_plan
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
