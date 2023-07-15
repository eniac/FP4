import pulp as plp
import math


class PulpSolver(object):
    def __init__(self, graph_wo_actions, graph_with_actions, stage_to_tables_dict, table_actions):
        
        cfgModel = plp.LpProblem("CFG Partition Problem", plp.LpMinimize)

        # K in section 4 of the Tracking P4 Program Execution in the Data Plane. It is the number of sub-dags.
        numk = max([len(x) for x in stage_to_tables_dict.values()])
        variables = [i for i in range(numk)]

        # Number of variables
        var_size = plp.LpVariable.dicts(name='metaVariables', indices=variables, lowBound=0)

        # Objective function
        cfgModel += (plp.lpSum(var_size[v] for v in var_size), 'minimize the total number of bits')

        # Possible assignment for each table
        assignment_keys = [(v, t) for v in variables for t in graph_wo_actions.nodes]

        assignment = plp.LpVariable.dicts(name="assignmentVar", indices=assignment_keys, cat='Binary')

        for var in variables:
            # First constraint in the paper. Maximum bits can be 32 for each variable.
            cfgModel += var_size[var] <= 32

            # Constraint 4
            cfgModel += plp.lpSum(self.log_outgoing_edges(graph_wo_actions, table_actions, table_name) * assignment[(var, table_name)] for table_name in graph_wo_actions.nodes) <= var_size[var]

            for stage, table_list in stage_to_tables_dict.items():
                # Constraint For each stage S and sub-DAG j (variable v), only one table from Ts is assigned to sub-DAG j (variable v). - Number 2
                cfgModel += plp.lpSum(assignment[var, t] for t in table_list) <= 1

        for table_name in graph_wo_actions.nodes:
            # Each table t is assigned to a single sub-DAG j (variable v). - Constraint Number 3
            # print('table_name', table_name)
            cfgModel += plp.lpSum(assignment[(var, table_name)] for var in var_size) == 1


        cfgModel.solve()
        
        # Dictionary - subgraph to table name
        self.subgraph_to_tables = dict()

        # Variable to Number of bits needed
        self.var_to_bits = [0 for _ in range(numk)]

        # Get the output in the required format
        for variable in cfgModel.variables():
            if 'assignmentVar' in variable.name and float(variable.varValue) == 1.0:
                subgraph_number = int(variable.name.split('_')[1][1:-1])
                if subgraph_number not in self.subgraph_to_tables:
                    self.subgraph_to_tables[subgraph_number] = set()

                self.subgraph_to_tables[subgraph_number].add(variable.name.split('\'')[1])
            elif "metaVariables" in variable.name:
                self.var_to_bits[int(variable.name.split('_')[1])] = int(variable.varValue)


        print("subgraph to tables:\n", self.subgraph_to_tables)
        print("variable to variable size. Index is variable, value is the number of bits:\n", self.var_to_bits)



    
    # def find_number_of_paths(self, graph_with_actions):
        

    def summarize_log_outgoing_edges(self, graph_wo_actions, table_actions):
        table_edge_values = dict()
        for table_name in graph_wo_actions.nodes:
            table_edge_values[table_name] = self.log_outgoing_edges(graph_wo_actions, table_actions, table_name)

        return table_edge_values

    def log_outgoing_edges(self, graph_wo_actions, table_actions, table_name):
        next_nodes = len(graph_wo_actions.out_edges(table_name))
        if next_nodes == 0:
            next_nodes = 1
        if table_name in table_actions:
            next_nodes = next_nodes * len(table_actions[table_name])

        # print(next_nodes)
        return max( int(math.ceil(math.log(next_nodes))), 1)
