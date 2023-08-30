import math
import json
from itertools import product
import struct
import sys
from collections import defaultdict, OrderedDict
from header import Header
# from headerSynthesizer import HeaderSynthesizer
import random
from datetime import datetime

class CoverageDetector(object):
    def __init__(self, baseName, simulation=True, rulesFile=None, mode='normal'):
        print("====== CoverageDetector ======")

        ingress_plan_json = None
        egress_plan_json = None

        # A path is uniquely defined by a tuple of encoding values
        self.paths_seen = []
        self.field2index_intra_path_seen = {}
        self.total_num_paths = 1
        self.total_num_actions = 0
        self.field2encoding2path = {}
        self.total_num_vars = 0
        self.all_actions = []
        print("--- Read ingress plan ---")
        with open("/home/leoyu/FP4/instrumentation/mvbl/plan/"+baseName+"_ingress.json") as f:
            ingress_plan_json = json.load(f)
        self.ingress_num_vars = ingress_plan_json["num_vars"]
        self.total_num_vars += self.ingress_num_vars
        print("ingress_num_vars: {}".format(self.ingress_num_vars))
        intra_index = 0
        for var in range(self.ingress_num_vars):
            num_paths = ingress_plan_json[str(var)]["num_paths"]
            print("var {0}, num_paths: {1}".format(var, num_paths))
            self.total_num_paths *= num_paths
            self.field2encoding2path["encoding_i"+str(var)] = ingress_plan_json[str(var)]["encoding_to_path"]
            self.field2index_intra_path_seen["encoding_i"+str(var)] = intra_index
            intra_index += 1
        # Get the total number of actions
        if self.ingress_num_vars != 0:
            for actions in ingress_plan_json["table2actions_dict"].values():
                self.total_num_actions += len(actions)
                self.all_actions.extend(actions)
            

        print("--- Read egress plan ---")
        with open("/home/leoyu/FP4/instrumentation/mvbl/plan/"+baseName+"_egress.json") as f:
            egress_plan_json = json.load(f)
        self.egress_num_vars = egress_plan_json["num_vars"]
        self.total_num_vars += self.egress_num_vars
        print("egress_num_vars: {}".format(self.egress_num_vars))
        for var in range(self.egress_num_vars):
            num_paths = egress_plan_json[str(var)]["num_paths"]
            print("var {0}, num_paths: {1}".format(var, num_paths))
            self.total_num_paths *= num_paths
            self.field2encoding2path["encoding_e"+str(var)] = egress_plan_json[str(var)]["encoding_to_path"]
            self.field2index_intra_path_seen["encoding_e"+str(var)] = intra_index
            intra_index += 1
        if self.egress_num_vars != 0:
            for actions in egress_plan_json["table2actions_dict"].values():
                self.total_num_actions += len(actions)
                self.all_actions.extend(actions)
        

        print("self.total_num_paths: {}".format(self.total_num_paths))
        print("self.total_num_actions: {}".format(self.total_num_actions))
        print("self.all_actions: {}".format(self.all_actions))
        print("self.field2encoding2path: {}".format(self.field2encoding2path))
        
        # Action name is the key, value is the number of times seen
        self.actionToCount = dict()
        self.totalUniqueActions = 0

        # (field,bit) is the key, action name is the value
        # self.bitToAction = dict()
        self.actionList = []
        self.numSeeds = 0

        # Key is the field (field0, field1 ...), and value is the number of bits in the field
        self.fieldToLen = dict()
        self.maxKeys = 0
        self.numAssertions = 0
        # self.numTables = 0
        self.numParserPaths = 0

        # Each index contains a list of headers to extract
        self.parserPaths = []

        # Each index contains a dictionary. Index signifies a index of the parser path.
        # Key of the dictionary is the header.field to set and value is the value to set.
        self.fixedFields = []

        # Key is the table name, value is the list of keys of the table (keys of the table are fields)
        self.tableToKeys = dict()

        # Header order in table ti_add_clones
        self.headerOrder = OrderedDict()
        self.numHeaders = 0

        self.totalPacketReceived = 0
        self.seenActions = set()

        self.seedToTable = defaultdict(list)
        self.original_keys = OrderedDict()

        self.fixedRulesFields = dict()

        self.mode = mode

        self.baseName = baseName

        actionsFile = baseName + "_coverageAndRules.json"
        self.initialize_variables(actionsFile, rulesFile)
        self.simulation = simulation

        self.packets_forwarded = 0
        self.start_time = None
        self.action_coverage = 0
        self.path_coverage = 0

        self.set_out_file()
        print("header written")

    def set_out_file(self):
        self.outfile = open("cpdigest_sdt_"+self.baseName+"_" + str(self.mode) +".txt", 'w')
        self.outfile.write("time,number_of_seeds,packets_forwarded,action_coverage,path_coverage,total_packets_received,action_seen\n")

    def write_output(self, time):
        self.outfile.write(str(time) + "," + str(self.numSeeds) \
         + "," + str(self.packets_forwarded) + "," + str(self.action_coverage) + "," + str(self.path_coverage) \
         + "," + str(self.totalPacketReceived) + "," + str(self.seenActions) + "\n")
        self.outfile.flush()

    def set_packets_forwarded(self, numPackets):
        self.packets_forwarded = numPackets
        #if self.start_time is None:
         #   self.write_output(0)


    @staticmethod
    def breakdown_rules(rule_as_list):
        index = 4
        outDict = dict()
        while index < len(rule_as_list):
            if rule_as_list[index].endswith("_start"):
                outDict[rule_as_list[index][:-6]] = rule_as_list[index + 1]
            else:
                outDict[rule_as_list[index]] = rule_as_list[index + 1]
            index += 2

        return outDict


    def add_single_rule(self, rule):
        if "add_entry" not in rule:
            return
        if "ti_port_correction" in rule:
            return

        currentRule = rule.split()
        table_name = currentRule[1]
        action_name = currentRule[3]
        key_value_dict = CoverageDetector.breakdown_rules(currentRule)
        for key, value in key_value_dict.items():
            if key not in self.fixedRulesFields:
                self.fixedRulesFields[key] = []
            
            self.fixedRulesFields[key].append(value)


    def add_new_rules(self, ruleList):
        for rule in ruleList:
            self.add_single_rule(rule)

    def analyze_rules(self, rulesFile):
        if rulesFile is None:
            return
        table_rules = open(rulesFile, 'r')
        for line in table_rules:
            self.add_single_rule(line)

    def initialize_variables(self, filePath, rulesFile):
        data = None
        print("file path", filePath)
        with open(filePath) as f:
            data = json.load(f, object_pairs_hook=OrderedDict)

        for extractList in data["extract"]:
            self.numParserPaths += 1
            self.numSeeds += 1
            self.parserPaths.append(extractList)

        print(self.parserPaths)

        if "fixed_fields" in data:
            self.fixedFields = data["fixed_fields"]
        else:
            self.fixedFields = None

        # toRemove = []
        # for idx, header_value_dict in enumerate(self.fixedFields):
        #     if header_value_dict is None:
        #         continue
        #     for header, value in header_value_dict.items():
        #         if header == "ig_intr_md.ingress_port":
        #             toRemove.append(idx)

        # for index in sorted(toRemove, reverse=True):
        #     del self.fixedFields[index]
        #     del self.parserPaths[index]


        # self.maxKeys = data["max_keys"]
        self.numAssertions = data["num_assertions"]
        # self.numTables = data["num_tables"]

        # Keys from UT
        if "table" in data:
            self.tableToKeys = data["table"]
        else:
            self.tableToKeys = None

        # DTKeys
        self.dtKeys = data["table_keys"]
        for table_name, field_list in self.dtKeys.items():
            for i in range(len(field_list)):
                field_list[i] = field_list[i].replace("[", "_")
                field_list[i] = field_list[i].replace("]", "_")

        # DTActionParam
        self.dtActionParams = data["action_params"]

        counter = 0
        
        for fieldBitsDict in data["visited"]:
            for fieldName, bits in fieldBitsDict.items():
                # print("fieldName:", fieldName, "bits:", bits)
                self.fieldToLen[fieldName] = bits

                if fieldName == "__pad":
                    continue
                if fieldName == "temp_port":
                    continue
                elif "assertion" in fieldName:
                    continue
            # print("length: ", self.fieldToLen[fieldName])
            # actions_dict = data["visited"][fieldName]["actions"]
            # for action_name, bitPos in actions_dict.items():
                # print("action_name", action_name, "bitPos", bitPos)
                self.totalUniqueActions += 1
                # print("position", self.fieldToLen[fieldName] - math.log(bitPos,2) - 1)
                # self.bitToAction[(fieldName, self.fieldToLen[fieldName] - math.log(bitPos,2) - 1)] = action_name
                self.actionList.append(fieldName) 

                self.actionToCount[fieldName] = 0
        print("self.actionList: {}".format(self.actionList))
        for header_list in data["headers"]:
            for headerName, numHeaders in header_list.items():
                self.headerOrder[headerName] =  numHeaders
                if int(numHeaders) > 0:
                    self.numHeaders += int(numHeaders)
                else:
                    self.numHeaders += 1
                
        # key is the table_name, value is the dict
        # In inner dict: key is the field, value is the match type
        self.original_keys = data["original_keys"]

        if "analysis" not in self.mode:
            if "fixed_values" in data:
                for field, value in data["fixed_values"].items():
                    field = field.replace(".", "_")
                    self.fixedRulesFields[field] = value

            self.analyze_rules(rulesFile)

        # print("parserPaths:", self.parserPaths)
        # print("numParserPaths:",  self.numParserPaths)


        # for action_name in self.actionList:
        #     print(action_name)
        # print("bitToAction:", self.bitToAction)


    def add_initial_seed_packets(self):
        print("====== add_initial_seed_packets prologue ======")
        ruleList = []

        # Test only
        # ruleList.append("pd ti_port2count add_entry ai_port2count ipv4_dstAddr 10.0.0.1")
        # ruleList.append("pd te_port2count add_entry ae_port2count ipv4_dstAddr 10.0.0.1")        

        # ruleList.append("table_add ti_port_correction ai_drop_packet 0 =>")
        if self.simulation:
            for i in range(0,16):
                ruleList.append("table_add ti_set_port ai_set_port " + str(i) + " => " + str(i*4))
        else:
            for i in range(0,16):
                ruleList.append("pd ti_set_port add_entry ai_set_port pfuzz_metadata_temp_port " + str(i) + " action_outPort " + str(i*4))


        for matchRule in product(range(2), repeat=2+self.numAssertions):
            print(matchRule)
            if len(matchRule) == 2 and matchRule[0] == 1 and matchRule[1] == 1:
                continue
            elif len(matchRule) > 2 and matchRule[0] == 1 and matchRule[1] == 1 and (not any(matchRule[2:])):
                continue 
            else:
                if self.simulation:
                    ruleList.append("table_add ti_path_assertion ai_send_to_control_plane " + " ".join([str(x) for x in matchRule]) + " => ")
                else:
                    ruleList.append("pd ti_path_assertion add_entry ai_send_to_control_plane " + " ".join([val for pair in zip(self.dtKeys["ti_path_assertion"], map(str,matchRule)) for val in pair]))


        ruleList.extend(self.add_ternary_rules("ti_set_seed_num", "ai_set_seed_num", self.numSeeds, 255))
        ruleList.extend(self.add_ternary_rules("ti_set_resubmit", "ai_set_resubmit", 1, 255))
        # ruleList.extend(self.add_ternary_rules("ti_set_fixed_header", "ai_set_fixed_header", 1, 255))
        ruleList.append("pd ti_set_fixed_header add_entry ai_set_fixed_header pfuzz_metadata_make_clone_start 2 pfuzz_metadata_make_clone_end 255 pfuzz_metadata_fixed_header_seed 0 pfuzz_metadata_fixed_header_seed_mask 0 priority 255 action_real_fixed_header 0")


        for i in range(self.numParserPaths):
            if self.simulation:
                ruleList.append("table_add ti_create_packet ai_add_" + CoverageDetector.combine_header(self.parserPaths[i]) + " " + str(i) + " => ")
            else:
                ruleList.append("pd ti_create_packet add_entry ai_add_" + CoverageDetector.combine_header(self.parserPaths[i]) + " pfuzz_metadata_seed_num " + str(i))
            actionsParameters = []
            for header in self.parserPaths[i]:
                if header[-1] == "*":
                    nameAndCount = header.split("*")
                    for field, numBits in Header.fields[nameAndCount[0]]:
                        for j in range(int(nameAndCount[1])):
                            if ((self.fixedFields is not None) and (self.fixedFields[i] is not None) and ((header + "[" + str(j) + "]." + field) in (self.fixedFields[i]))):
                                actionsParameters.append(CoverageDetector.add_fixed(self.fixedFields[i][header + "[" + str(j) + "]." + field]))
                            else:
                                actionsParameters.append(str(CoverageDetector.generate_param(numBits)))
                else:
                    for field, numBits in Header.fields[header]:
                        if ((self.fixedFields is not None) and (self.fixedFields[i] is not None) and ((header + "." + field) in (self.fixedFields[i]))):
                            actionsParameters.append(CoverageDetector.add_fixed(self.fixedFields[i][header + "." + field]))
                        else:

                            actionsParameters.append(str(CoverageDetector.generate_param(numBits)))

            # print("going in format_for_hardware:", CoverageDetector.combine_header(self.parserPaths[i]))
            # print("going in format_for_hardware:", self.parserPaths[i])
            if self.simulation:
                ruleList.append("table_add ti_add_fields ai_add_fixed_" + CoverageDetector.combine_header(self.parserPaths[i]) + " " + str(i) + " => " + " ".join(actionsParameters))
            else:
                ruleList.append("pd ti_add_fields add_entry ai_add_fixed_" + CoverageDetector.combine_header(self.parserPaths[i]) + " pfuzz_metadata_seed_num " + str(i) + " " + CoverageDetector.format_for_hardware(self.dtActionParams["ai_add_fixed_" + CoverageDetector.combine_header(self.parserPaths[i])] ,actionsParameters))



        for i in range(self.numParserPaths):
            headersPresent = ["0"] * self.numHeaders
            # print("numHeaders: ", self.numHeaders)
            # print("parserPaths:", self.parserPaths[i])

            for header in self.parserPaths[i]:
                if header[-1] == "*":
                    nameAndCount = header.split("*")
                    index = 0
                    for headerName, numHeaders in self.headerOrder.items():
                        if headerName == nameAndCount[0]:
                            for j in range(int(nameAndCount[1])):
                                headersPresent[index + j] = "1"
                            break
                        index += 1


                else:
                    index = 0
                    for headerName, numHeaders in self.headerOrder.items():
                        if headerName == header:
                            headersPresent[index] = "1"
                            break
                        index += 1

            if self.simulation:
                ruleList.append("table_add ti_add_clones ai_add_clone_" + CoverageDetector.combine_header(self.parserPaths[i]) + " " + " ".join(headersPresent) + " => ")
            else:
                ruleList.append("pd ti_add_clones add_entry ai_add_clone_" + CoverageDetector.combine_header(self.parserPaths[i]) + " " + " ".join([val for pair in zip(self.dtKeys["ti_add_clones"], headersPresent) for val in pair]))
            # print(ruleList[-1])

        # for i in range(self.maxKeys):
        #     if self.simulation:
        #         ruleList.append("table_add te_apply_mutations" + str(i) + " ae_apply_mutations" + \
        #         str(i) + " => ")
        #     else:
        #         ruleList.append("pd te_apply_mutations" + str(i) + " add_entry ")

        for idx, path in enumerate(self.parserPaths):
            self.add_in_seed_to_table(idx, path)
            
        for seedNum, table_list in self.seedToTable.items():
            ruleList.extend(self.add_mutation_rules(seedNum, table_list))
            ruleList.extend(self.add_fixed_rules(seedNum, table_list))

        if self.simulation:
            ruleList.append("table_add te_do_resubmit ae_resubmit 1 => ")
        else:
            ruleList.append("pd te_do_resubmit add_entry ae_resubmit pfuzz_metadata_make_clone 1")

        for rule in ruleList:
            print(rule)
        print("====== add_initial_seed_packets epilog ======")
        return ruleList

    @staticmethod
    def combine_header(header_list):
        ret = ""
        for i in range(len(header_list)):
            if (i == 0):
                if (header_list[i][-1] == "*"):
                    nameAndCount = header_list[i].split("*")
                    ret += (nameAndCount[0] + nameAndCount[1])
                else:
                    ret += header_list[0]
            else:
                if (header_list[i][-1] == "*"):
                    nameAndCount = header_list[i].split("*")
                    ret += ("_" + nameAndCount[0] + nameAndCount[1])
                else:
                    ret += ("_" + header_list[i])


        return ret


    @staticmethod
    def format_for_hardware(action_params, action_values):
        action_params = ['action_' + x for x in action_params]
        out = " ".join([val for pair in zip(action_params, action_values) for val in pair])
        return out


    @staticmethod
    def add_fixed(strInput):
        if strInput.isdigit():
            return strInput
        elif strInput.startswith('0x'):
            return str(int(strInput, 16))

    @staticmethod
    def generate_param(numBits):
        value = random.randrange(0, 2**(numBits))
        out = ""
        if numBits > 32:
            out = hex(value)
        else:
            out = str(value)
        if out[-1] == "L":
            return out[:-1]
        return out

    @staticmethod
    def find_num_bits(action_param):
        for headerName, field_list in Header.fields.items():
            for field, numBits in field_list:
                if ((headerName + "_" + field) == action_param):
                    return numBits

        print("Bug present - exiting - Unknown action param:", action_param)
        exit()
        return None

    def add_fixed_rules(self, seedNum, table_list):
        if (len(self.fixedRulesFields) == 0) or self.simulation or 'analysis' in self.mode:
            return []
        ruleList = []

        for table_num, table_name in enumerate(table_list):
            paramsPresent = []
            for action_param in self.dtActionParams["ae_move_back_fix_" + table_name]:
                if action_param in self.fixedRulesFields:
                    paramsPresent.append(self.fixedRulesFields[action_param])
                else:
                    paramsPresent.append(['CoverageDetector.generate_param(CoverageDetector.find_num_bits(action_param))'])

            counter = 0
            for result in product(*paramsPresent):
                rule = "pd te_move_back_fields add_entry ae_move_back_fix_" + table_name + " pfuzz_metadata_seed_num " + str(seedNum) + " pfuzz_metadata_table_seed " + str(table_num) + " pfuzz_metadata_fixed_header_seed " + str(counter+1) + " pfuzz_metadata_fixed_header_seed_mask " + str(CoverageDetector.shift_bit_length(counter+1)) +" priority " + str(255 - counter - 1)

                for idx, action_param in enumerate(self.dtActionParams["ae_move_back_fix_" + table_name]):
                    rule += (" action_" + action_param + " ")
                    if action_param in self.fixedRulesFields:
                        rule += result[idx]
                    else:
                        rule += eval(result[idx])

                counter += 1
                ruleList.append(rule)

        return ruleList



    def add_mutation_rules(self, seedNum, table_list):
        ruleList = []
        for i in range(len(table_list)):
            if self.simulation:
                ruleList.append("table_add te_set_table_seed ae_set_table_seed " + str(seedNum) + " " + str(i) + "&&&" + str(i) + " => " + str(i) + " " + str(15 - i))
            else:
                # ruleList.append("pd te_set_table_seed add_entry ae_set_table_seed pfuzz_metadata_seed_num " + str(seedNum) + " pfuzz_metadata_table_seed " + str(i) + " pfuzz_metadata_table_seed_mask " + str(i) + " priority " + str(15 - i) + " action_real_table_seed " + str(i))
                ruleList.append("pd te_set_table_seed add_entry ae_set_table_seed pfuzz_metadata_seed_num " + str(seedNum) + " pfuzz_metadata_table_seed " + str(i) + " pfuzz_metadata_table_seed_mask " + str(CoverageDetector.shift_bit_length(i+1)) + " priority " + str(15 - i) + " action_real_table_seed " + str(i))                

        for table_num, table_name in enumerate(table_list):
            if self.simulation:
                ruleList.append("table_add te_move_fields ae_move_" + table_name + \
                    " " + str(seedNum) + " " + str(table_num) +  " => ")
                ruleList.append("table_add te_move_back_fields ae_move_back_" + table_name + \
                    " " + str(seedNum) + " " + str(table_num) +  " 0 => ")

            else:
                ruleList.append("pd te_move_fields add_entry ae_move_" + table_name + " pfuzz_metadata_seed_num " + str(seedNum) + " pfuzz_metadata_table_seed " + str(table_num))
                ruleList.append("pd te_move_back_fields add_entry ae_move_back_" + table_name + " pfuzz_metadata_seed_num " + str(seedNum) + " pfuzz_metadata_table_seed " + str(table_num) + " pfuzz_metadata_fixed_header_seed 0 pfuzz_metadata_fixed_header_seed_mask 0 priority 255")


        return ruleList

    def add_in_seed_to_table(self, idx, path):
        if self.tableToKeys is None:
            return
        for table_name, header_fields in self.tableToKeys.items():
            include = True
            for field in header_fields:
                field_breakdown = field.split(".")[0]
                if field_breakdown not in path:
                    include = False
                    break
            if include:
                self.seedToTable[idx].append(table_name)

    # def add_in_seed_to_table(self, idx, path):
    #     if self.tableToKeys is None:
    #         return
    #     print("path", path)
    #     for table_name, header_fields in self.tableToKeys.items():
    #         print("table_name", table_name, "header_fields", header_fields)
    #         for field in header_fields:
    #             field_breakdown = field.split(".")[0]
    #             if field_breakdown in path:
    #                 self.seedToTable[idx].append(table_name)
    #                 print("included")
    #                 break

    def check_coverage_and_add_seed(self, packet, addSeed=False):
        print("====== check_coverage_and_add_seed prologue ======")

        # Update coverage
        self.totalPacketReceived += 1
        # counter = 0
        # fieldName = "field" + str(counter)
        flag = False
        # print("fieldName", fieldName)
        # print(packet.headers)
        path_seen = [-1 for _ in range(self.total_num_vars)]
        for field_name in packet.headers["pfuzz_visited"]:
            print("--- field_name: {} ---".format(field_name))
            if field_name not in self.actionList:
                print("Not in self.actionList {}".format(self.actionList))
                continue

            bitList = packet.headers["pfuzz_visited"][field_name]
            encoding_val = 0
            for bit in bitList:
                encoding_val = (encoding_val << 1) | bit
            print("bitList: {0}, encoding_val: {1}".format(bitList, encoding_val))
            try:
                path_seen[self.field2index_intra_path_seen[field_name]] = encoding_val
            # if len(bitList) > 1:
                # print("It cannot be more than one")
                # exit()

            # bitValue = bitList[0]
            # if bitValue == 0:
                # continue
            # Update the action related statistics
                print("Encoded path: {}".format(self.field2encoding2path[field_name][str(encoding_val)]))
                for node in self.field2encoding2path[field_name][str(encoding_val)]:
                    if "_pfuzz_" in node:
                        # self.actionToCount[node] += 1
                        self.seenActions.add(node)
                    # if self.actionToCount[field_name] > 1:
                    #     continue
                        flag = True
            except Exception as e:
                print("[ERROR] {}".format(e))
            # counter += 1
            # fieldName = "field" + str(counter)
        print("--- path_seen {} ---".format(path_seen))
        self.paths_seen.append(path_seen)
        self.action_coverage = 1.0*len(self.seenActions)/self.total_num_actions
        self.path_coverage = 1.0*len(self.paths_seen)/self.total_num_paths
        
        if self.start_time is None:
            self.start_time = datetime.now()
            self.write_output(0)
        else:
            self.write_output((datetime.now() - self.start_time).total_seconds())        

        print("--- Path coverage: {0}/{1}={2} ---".format(len(self.paths_seen), self.total_num_paths, self.path_coverage))
        print("self.paths_seen: {}".format(self.paths_seen))
        print("--- Action coverage: {0}/{1}={2}, number_of_seeds: {3}, time: {4}".format(len(self.seenActions), self.total_num_actions, self.action_coverage, self.numSeeds, datetime.now().time()))
        print("self.seenActions: {}".format(self.seenActions))
        print("--- self.all_actions - self.seenActions ---")
        print(list(set(self.all_actions) - set(self.seenActions)))
        sys.stdout.flush()
        # print("Coverage: ", len(self.seenActions)/(self.totalUniqueActions*1.0 ))
        #self.outfile.write("coverage: " + str(len(self.seenActions)/(self.totalUniqueActions*1.0 )) + ' actions seen: ' + str(self.seenActions)+"\n")

        if ((datetime.now() - self.start_time).total_seconds() > 300):
            print("totalUniqueActions: ", self.totalUniqueActions)
            print("actionToCount", self.actionToCount)
            print("Coverage complete", self.action_coverage)
            print("Exiting control plane")
            print("Endtime time", datetime.now().time())
            exit()

        if not flag or 'seed' in self.mode:
            print("No actions seen, not adding seed packet OR seeds are set to off")
            rules = []
        elif CoverageDetector.shifting(packet.headers["pfuzz_visited"]["pkt_type"]) == 3:
            print("Packet type is 3 so will not add packet")
            rules = []
        elif addSeed:
            rules = self.add_seed_packet(packet)
        print("--- check_coverage_and_add_seed epilog ---")
        return rules

    def add_seed_packet(self, packet):
        print("--- add_seed_packet prologue ---")
        ruleList = []
        
        # ruleList.append("table_set_default ti_get_random_seed ai_get_random_seed " + str(self.numSeeds) + " 100 1")



        validHeaders = []
        for headerName, headerValues in packet.headers.items():
            # print("headerName", headerName)
            if "clone" not in headerName:
                continue

            validHeaders.append(headerName[:-6])

        self.parserPaths.append(validHeaders)

        # print("paths")
        # for path in self.parserPaths:
        #     print(path)

        if self.simulation:
            ruleList.append("table_add ti_create_packet ai_add_" + CoverageDetector.combine_header(self.parserPaths[self.numSeeds]) + " " + str(self.numSeeds) + " => ")
        else:
            ruleList.append("pd ti_create_packet add_entry ai_add_" + CoverageDetector.combine_header(self.parserPaths[self.numSeeds]) + " pfuzz_metadata_seed_num " + str(self.numSeeds))

        valueList = []
        for headerName in validHeaders:
            for fieldName, fieldValue in packet.headers[headerName + "_clone"].items():
                # print(fieldName, ":", fieldValue)
                if 'Addr' in fieldName and 'ethernet' in headerName and self.simulation:
                    valueList.append(CoverageDetector.convertToMac(fieldValue))
                elif 'Addr' in fieldName and 'ipv4' in headerName:
                    valueList.append(CoverageDetector.convertToIP(fieldValue))
                else:
                    valueList.append(str(CoverageDetector.shifting(fieldValue)))
                # print(valueList[-1])

            # valueList.extend(packet.headers[headerName + "_clone"].values())

        # print("valueList:", valueList)
        
        if self.simulation:
            ruleList.append("table_add ti_add_fields ai_add_fixed_" + CoverageDetector.combine_header(self.parserPaths[self.numSeeds]) + " " + str(self.numSeeds) + " => "  + " ".join(valueList))
        else:
            ruleList.append("pd ti_add_fields add_entry ai_add_fixed_" + CoverageDetector.combine_header(self.parserPaths[self.numSeeds]) + " pfuzz_metadata_seed_num " + str(self.numSeeds) + " " + CoverageDetector.format_for_hardware(self.dtActionParams["ai_add_fixed_" + "_".join(self.parserPaths[self.numSeeds])] ,valueList))


        self.add_in_seed_to_table(self.numSeeds, self.parserPaths[self.numSeeds])

        ruleList.extend(self.add_mutation_rules(self.numSeeds, self.seedToTable[self.numSeeds]))
        ruleList.extend(self.add_fixed_rules(self.numSeeds, self.seedToTable[self.numSeeds]))

        ruleList.extend(self.add_single_ternary_rules("ti_set_seed_num", "ai_set_seed_num", self.numSeeds, 255))

        self.numSeeds += 1
        # print("Seed rules")
        # for rule in ruleList:
        #     print(rule)

        print("--- add_seed_packet epilog ---")
        return ruleList

    # https://stackoverflow.com/a/14267825
    @staticmethod
    def shift_bit_length(x):
        if x == 0:
            return 1
        else:
            a = x.bit_length()
            return (1 << a) - 1

    def add_ternary_rules(self, table_name, action_name, num_entries, max_entries):
        rules = []
        for i in range(num_entries):
            if self.simulation:
                rules.append("table_add " + table_name + " " + action_name + " " \
                    + str(i) + "&&&" + str(CoverageDetector.shift_bit_length(i)) + " => " + str(i) + " " + str(max_entries - i))
            else:
                rules.append("pd " + table_name + " add_entry " + action_name + " " + self.dtKeys[table_name][0] + " " + str(i) + " " + self.dtKeys[table_name][0] + "_mask " + str(CoverageDetector.shift_bit_length(i)) + " priority " + str(max_entries - i) + " action_" + self.dtActionParams[action_name][0] + " " + str(i))

        return rules


    def add_single_ternary_rules(self, table_name, action_name, num_entry, max_entries):
        rules = []
        if self.simulation:
            rules.append("table_add " + table_name + " " + action_name + " " \
                    + str(num_entry) + "&&&" + str(shift_bit_length(num_entry)) + " => "\
                    + str(max_entries - num_entry))
        else:
            rules.append("pd " + table_name + " add_entry " + action_name + " " + self.dtKeys[table_name][0] + " " + str(num_entry) + " " + self.dtKeys[table_name][0] + "_mask " + str(CoverageDetector.shift_bit_length(num_entry)) + " priority " + str(max_entries - num_entry) + " action_" + self.dtActionParams[action_name][0] + " " + str(num_entry))

        return rules

    @staticmethod
    def convertToIP(bitList):
        bitList = [str(x) for x in bitList]
        strList = []
        for i in range(4):
            strList.append(chr(int(''.join(bitList[i*8:i*8 + 8]), 2)))
        ipNums = struct.unpack("BBBB", ''.join(strList))
        ipNums = [str(x) for x in ipNums]
        return ".".join(ipNums)

    @staticmethod
    def convertToMac(bitList):
        bitList = [str(x) for x in bitList]
        strList = []
        for i in range(6):
            strList.append(chr(int(''.join(bitList[i*8:i*8 + 8]), 2)))
        macNums = struct.unpack("BBBBBB", ''.join(strList))
        macNums = ['%02x' % x for x in macNums]
        return ":".join(macNums)


    @staticmethod
    def covertToStr(valueList):
        outStr = []

        for valueBits in valueList:
            outStr.append(CoverageDetector.shifting(valueBits))

        return " ".join([str(x) for x in outStr])

    @staticmethod
    def shifting(bitList):
        out = 0
        # print("in shifting:", bitList)
        for bit in bitList:
            out = (out << 1) | bit
        return out

# if __name__ == '__main__':
#     HeaderSynthesizer("headerFile.json")
#     from header import Header
#     covDet = CoverageDetector("coverageAndRules.json", False)
#     ruleList = covDet.add_initial_seed_packets()
#     for rule in ruleList:
#         print(rule)

