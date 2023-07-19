import pulp
import math


class PulpSolver(object):
    def __init__(self, graph_wo_actions, stage_to_tables_dict, table_actions):
        
        cfgModel = pulp.LpProblem("CFG_Partition_Problem", pulp.LpMinimize)

        print("--- stage_to_tables_dict ---")
        print(stage_to_tables_dict)

        # K: assume the number of sub-graphs to be the max number of tables for a stage
        numk = max([len(x) for x in stage_to_tables_dict.values()])
        print("--- K as max number of tables for a stage: {} ---".format(numk))
        variables = [i for i in range(numk)]

        # Decision variable: number of bits per sub-DAG
        var_size = pulp.LpVariable.dicts(name='metaVariables', indices=variables, lowBound=0)
        print("--- {} ---".format(var_size))

        # Objective function
        cfgModel += (pulp.lpSum(var_size[v] for v in var_size), 'Minimize_the_total_number_of_bits')

        # Decision variable: per sub-DAG, per table/conditional assignment tuple
        assignment_keys = [(v, t) for v in variables for t in graph_wo_actions.nodes]
        print("--- assignment_keys ---")
        for assignment_key in assignment_keys:
            print(assignment_key)

        assignment = pulp.LpVariable.dicts(name="assignmentVar", indices=assignment_keys, cat='Binary')

        for var in variables:
            # Constraint 1: maximum bits can be 32 for each variable.
            cfgModel += var_size[var] <= 32

            # Constraint 4: sum of paths (in log) should be smaller than the var bit size
            cfgModel += pulp.lpSum(self.log_outgoing_edges(graph_wo_actions, table_actions, table_name) * assignment[(var, table_name)] for table_name in graph_wo_actions.nodes) <= var_size[var]

            constraint4_str = ""
            for table_name in graph_wo_actions.nodes:
                constraint4_str += (str(self.log_outgoing_edges(graph_wo_actions, table_actions, table_name)) + "*" + str(assignment[(var, table_name)]) + " + ")
            constraint4_str += ("<=" + str(var_size[var]))
            print("--- constraint4_str ---")
            print(constraint4_str)

            for stage, table_list in stage_to_tables_dict.items():
                # Constraint 2: For each stage S and sub-DAG j (variable v), only one table from Ts is assigned to sub-DAG j (variable v)
                # LC_TODO: one won't instrument the conditional node, maybe the conditional node can be ommited in this constraint?
                cfgModel += pulp.lpSum(assignment[var, t] for t in table_list) <= 1

                constraint2_str = ""
                for t in table_list:
                    constraint2_str += (str(assignment[var, t])+" + ")
                constraint2_str += " <= 1"
                print("--- constraint2_str ---")
                print(constraint2_str)

        for table_name in graph_wo_actions.nodes:
            # Constraint 3: Each table t is assigned to a single sub-DAG j (variable v)
            # print('table_name', table_name)
            cfgModel += pulp.lpSum(assignment[(var, table_name)] for var in var_size) == 1

        cfgModel.solve()
        
        # Dictionary - subgraph to table name
        self.subgraph_to_tables = dict()

        # Variable to Number of bits needed
        self.var_to_bits = [0 for _ in range(numk)]

        print("--- Iterate solver variables ---")
        for variable in cfgModel.variables():
            print("--- name: {0}, value: {1} ---".format(variable.name, variable.varValue))
            if 'assignmentVar' in variable.name and float(variable.varValue) == 1.0:
                subgraph_number = int(variable.name.split('_')[1][1:-1])
                node_name = variable.name.split('\'')[1]
                if subgraph_number not in self.subgraph_to_tables:
                    self.subgraph_to_tables[subgraph_number] = set()
                self.subgraph_to_tables[subgraph_number].add(node_name)
                print("Add node {0} to sub-DAG {1}".format(node_name, subgraph_number))
            elif "metaVariables" in variable.name:
                subgraph_number = int(variable.name.split('_')[1])
                self.var_to_bits[subgraph_number] = int(variable.varValue)
                print("Assign subgraph {0} to {1}b".format(subgraph_number, int(variable.varValue)))

        print("\n--- subgraph_to_tables ---")
        print(self.subgraph_to_tables)

        print("\n--- var_to_bits ---")
        print(self.var_to_bits)

    def summarize_log_outgoing_edges(self, graph_wo_actions, table_actions):
        table_edge_values = dict()
        for table_name in graph_wo_actions.nodes:
            table_edge_values[table_name] = self.log_outgoing_edges(graph_wo_actions, table_actions, table_name)
        return table_edge_values

    def log_outgoing_edges(self, graph_wo_actions, table_actions, table_name):
        print("--- log_outgoing_edges {0} ---".format(table_name))
        next_nodes = len(graph_wo_actions.out_edges(table_name))
        print("# of out_edge in graph_wo_actions: {}".format(next_nodes))
        if next_nodes == 0:
            next_nodes = 1
        if table_name in table_actions:
            next_nodes = next_nodes * len(table_actions[table_name])
            print("table_actions[{0}]: {1}".format(table_name, table_actions[table_name]))
        print("# of out edges (w actions): {}".format(next_nodes))
        # LC_TODO: Don't need to take ceil? maybe subject to over estimation.
        # It is OK to be 0 (rather than 1)?
        log_next_nodes = max( int(math.ceil(math.log(next_nodes))), 1)
        print("log of # of out edges (w actions): {}".format(log_next_nodes))
        return log_next_nodes
