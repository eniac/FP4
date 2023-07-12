import pydotplus
import pydot
import json
import networkx as nx
import copy
import re
from utils import has_numbers

class GraphParser(object):
    def __init__(self, dotfile, jsonfile, input_type='bmv2'):

        # Step 1
        nodes_dict = self.get_nodes(dotfile)
        nodes = nodes_dict.values()
        tables_and_conditions = nodes

        # Step 2
        edges = self.get_edges(dotfile, nodes_dict)

        # Step 3
        edges_tuples = []
        for e in edges:
            edges_tuples.append((e['src'], e['dst'], 0))
        edges_tuples = list(set(edges_tuples))

        table_graph = nx.DiGraph()
        table_graph.add_weighted_edges_from(edges_tuples)

        if input_type == 'bmv2':
            conditions_to_nextstep = self.extract_conditionals(jsonfile)

            actions = self.extract_actions_bmv2(jsonfile)

            table_actions = self.extract_tables_actions_bmv2(jsonfile, actions)

        elif input_type == 'p414':
            # Step 4
            stage_to_tables_dict, table_actions = self.extract_stages_p4_14(jsonfile, nodes)
            actions = []
            for action_list in table_actions.values():
                actions.extend(action_list)
            actions = list(set(actions))

        # Step 5 - filter out tables for different pipeline
        for stage, table_list in stage_to_tables_dict.items():
            stage_to_tables_dict[stage] = [x for x in table_list if x in table_graph.nodes]


        if input_type == 'bmv2':
            stage_to_tables_dict = self.estimate_stages(table_graph)


        # Step 6 - Find leaves
        leaf_nodes = [v for v, d in table_graph.out_degree() if d == 0]

        # Step 7 - Add edges from tables to actions
        updates_edges = self.append_missing_edges(table_actions, edges, leaf_nodes)

        edges_to_remove = []


        # Step 8 - Remove table to table edge
        for e in updates_edges:
            if e['src'] in table_actions and e['dst'] in table_actions:
                edges_to_remove.append(e)
            elif e['src'] in table_actions and e['dst'] in tables_and_conditions:
                edges_to_remove.append(e)
            elif e['dst'] in table_actions and e['src'] in tables_and_conditions:
                edges_to_remove.append(e)

        for edge in edges_to_remove:
            if edge in updates_edges:
                updates_edges.remove(edge)

        # Step 9 - create graph with 0 weight
        for e in updates_edges:
            edges_tuples.append((e['src'], e['dst'], 0))
        edges_tuples = list(set(edges_tuples))

        full_graph = nx.DiGraph()
        full_graph.add_weighted_edges_from(edges_tuples)

        # Step 10 - rename nodes
        new_node_mapping = self.get_new_node_mapping(full_graph)

        table_graph = nx.relabel_nodes(table_graph, new_node_mapping)
        full_graph =  nx.relabel_nodes(full_graph, new_node_mapping)


        for old_name, new_name in new_node_mapping.items():
            if old_name in table_actions:
                table_actions[new_name] = table_actions.pop(old_name)


        for stage, table_list in stage_to_tables_dict.items():
            for i, n in enumerate(table_list):
                if n in new_node_mapping:
                    table_list[i] = new_node_mapping[n]

        # Step 11 - check for cycles
        cycles = list(nx.simple_cycles(full_graph))
        if cycles:
            print("****** cycles found ********")
            for cycle in cycles:
                print(cycle)
            print("****** end cycles found ********")
            print("****** It should be a DAG ********")
            exit()

        # Step 12
        from pulpSolver import PulpSolver
        print("******  Going in PulpSolver  **********")
        pulpSolver = PulpSolver(table_graph, full_graph, stage_to_tables_dict, table_actions)
        print("******   Out of PulpSolver   **********")


        # Step 13 - construct the subgraphs
        new_graph_edges = []
        for _ in range(len(pulpSolver.var_to_bits)):
            new_graph_edges.append([])

        
        for graph_number, number_of_bits in enumerate(pulpSolver.var_to_bits):
            for edge in full_graph.edges:
                src = edge[0]
                dst = edge[1]

                if '__' in src:
                    table_src = src.split('__')[0]
                else:
                    table_src = src

                if table_src in pulpSolver.subgraph_to_tables[graph_number]:
                    new_src = src
                else:
                    new_src = table_src

                if '__' in dst:
                    table_dst = dst.split('__')[0]
                else:
                    table_dst = dst

                if table_dst in pulpSolver.subgraph_to_tables[graph_number]:
                    new_dst = dst
                else:
                    new_dst = table_dst

                if new_src == new_dst:
                    continue

                new_graph_edges[graph_number].append((new_src, new_dst, 0))

        new_graphs = []
        for graph_edges in new_graph_edges:
            new_graphs.append(nx.DiGraph())
            new_graphs[-1].add_weighted_edges_from(graph_edges)

        
        # Step 14
        graph_with_weights = []
        for idx, graph in enumerate(new_graphs):
            print("----- Variable {0} additions------".format(idx))
            edges_with_weights = self.ball_larus(graph)
            graph_with_weights.append(nx.DiGraph())
            edges = []
            for e in edges_with_weights:
                edges.append((e['src'], e['dst'], e['weight']))
            graph_with_weights[-1].add_weighted_edges_from(edges)


    def get_new_node_mapping(self, graph):
        new_node_mapping = {}

        for node in graph.nodes:
            new_node = re.sub(r'\W+', '', node)
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
        stage_to_tables_dict = dict()
        table_to_stage_dict = dict()

        # print(len(graph.nodes))

        while (len(graph.nodes) > 0):
            current_nodes = [v for v, d in graph.in_degree() if d == 0]
            stage_to_tables_dict[latest_stage] = current_nodes
            for node in current_nodes:
                graph.remove_node(node)

            latest_stage += 1

        return stage_to_tables_dict


    def append_missing_edges(self, table_actions, edges, leaf_nodes):
        new_edges = []
        ed_to_del = []
        for e in edges:
            if e['src'] in table_actions.keys():
                ed_to_del.append(e)
                for ac in table_actions[e['src']]:
                    new_edges.append({'src':e['src'], 'dst':ac, 'label':''})
                    new_edges.append({'src':ac, 'dst':e['dst'], 'label':''})
            elif e['dst'] in leaf_nodes and e['dst'] in table_actions.keys():
                for ac_1 in table_actions[e['dst']]:
                    new_edges.append({'src':e['dst'], 'dst':ac_1, 'label':''})

        edges = edges + new_edges
        for de in ed_to_del:
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


    def get_condition_stage(self, stage_to_tables_dict, table_information, nodes):
        # breakout = False
        if 'cond' in table_information['name']:
            # print('table_information:', table_information['condition'])
            # exit()
            for node in nodes:
                if table_information['condition'][1:-1] in node:
                    for stage_information in table_information['stage_tables']:
                        stage_number = stage_information['stage_number']
                        if stage_number not in stage_to_tables_dict:
                            stage_to_tables_dict[stage_number] = []
                        stage_to_tables_dict[stage_number].append(node)
                        # stage_to_tables_dict[stage_number].append(table_information['name'])
                        # breakout = True

        if 'cond' in table_information['name'] and 'valid' in table_information['condition']:

            header_name, valid_field = table_information['condition'][1:-1].split('$')

            for node in nodes:
                if header_name in node and (valid_field in node or ('1' in valid_field and 'isValid' in node and ('!' not in node))):
                    for stage_information in table_information['stage_tables']:
                        stage_number = stage_information['stage_number']
                        if stage_number not in stage_to_tables_dict:
                            stage_to_tables_dict[stage_number] = []
                        stage_to_tables_dict[stage_number].append(node)

        return stage_to_tables_dict
            

    def extract_stages_p4_14(self, contextFile, nodes):
        
        data = None
        with open(contextFile, 'r') as f:
            data = json.load(f)

        stage_to_tables_dict = {}
        table_actions = {}
        
        for table_information in data['tables']:
            if 'match_attributes' not in table_information:
                stage_to_tables_dict = self.get_condition_stage(stage_to_tables_dict, table_information, nodes)
                continue

            table_name = table_information['name']
            

            if table_name not in table_actions:
                table_actions[table_name] = []

            for action in table_information['actions']:
                table_actions[table_name].append(table_name + "__" + action['name'])
                
            for stage_information in table_information['match_attributes']['stage_tables']:
                stage_number = stage_information['stage_number']
                if stage_number not in stage_to_tables_dict:
                    stage_to_tables_dict[stage_number] = []

                stage_to_tables_dict[stage_number].append(table_information['name'])
                
        return stage_to_tables_dict, table_actions

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


    def get_edges(self, dotfile, nodes):
        """ 
        This method takes .dot file path and returns a dictionary having Edge Source Node Name and
        Edge details like destination node name, edge label. Extracting edge information.
        It is restricted to conditions and tables only.
        """
        edge_src_details = {}
        edge_src_names = {}
        edges = []
        dot_graph = pydotplus.graphviz.graph_from_dot_file(dotfile) 
        count = 0   
        for subGraph in dot_graph.get_subgraphs():
            for e in subGraph.get_edge_list():
                edge_src_details[count] = {"src":e.get_source(),"dst":e.get_destination(), "label":e.obj_dict["attributes"]['label']}
                edge_src_names[count] = {"src":e.get_source(),"dst":e.get_destination(), "label":e.obj_dict["attributes"]['label']}
                count+=1

        for e in edge_src_details.keys():
            if(edge_src_details[e]['src'] in nodes.keys() and edge_src_details[e]['dst'] in nodes.keys()):
                edges.append({'src':nodes[edge_src_details[e]['src']], 'dst':nodes[edge_src_details[e]['dst']], 'label': edge_src_details[e]['label'].replace('"','')})
        return edges

    def get_nodes(self, dotfile, cnt_blk='MyIngress'):
        """
        This method takes .dot file path and returns a dictionary having Node Name and Node Label.
        Extracting node information.
        It is restricted to conditions and tables only.
        """
        node_name_label = {}
        dot_graph = pydotplus.graphviz.graph_from_dot_file(dotfile) 
        subgraphs = dot_graph.get_subgraphs()  
        for subG in subgraphs:
            for n in subG.get_node_list():
                if not has_numbers(n.get_name()):
                    continue
                name = n.obj_dict["attributes"]['label']
                if name not in ['__START__',"", '__EXIT__']:
                    if ';' in name:
                        name = name.replace(';','')
                    if '"' in name:
                        name = name.replace('"','')
                    if "()" in name and "." not in name:
                        name = name.replace("()",'')
                        name = cnt_blk+'.'+name
                    if name not in node_name_label.values():
                        node_name_label[n.get_name()] = name
                    else:
                        cnt = 1
                        key = list(node_name_label.keys())
                        values = list(node_name_label.values())
                        ind = key[values.index(name)]
                        while(1):
                            if name+"##"+str(cnt) in node_name_label:
                                cnt += 1
                            else:
                                name = name+"##"+str(cnt)
                                break
                        node_name_label[n.get_name()] = name
                        # if "##" in node_name_label[ind]:
                        #   cnt = int(node_name_label[ind].split("##")[1])

        # print("node_name_label", node_name_label)
        # exit()
        return node_name_label
        
if __name__ == '__main__':
    GraphParser('examples/Firewall_ingress.dot', 'examples/Firewall.json')
