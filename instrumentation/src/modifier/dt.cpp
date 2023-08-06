#include "../../include/ast_nodes_p4.hpp"
#include "../../include/modifier_dt.h"
#include "../../include/helper.hpp"
#include "../../util/util.cpp"

using namespace std;
DTModifier::DTModifier(AstNode* root, char* target, char* ingress_plan, char* egress_plan, int num_assertions, char* out_fn_base) : P4Modifier(root, target, ingress_plan, egress_plan, num_assertions, out_fn_base) {

    int num_mvbl_vars_ingress = ingress_plan_json_["num_vars"];
    PRINT_INFO("num_mvbl_vars_ingress: %d\n", num_mvbl_vars_ingress);
    for (int var_idx = 0; var_idx < num_mvbl_vars_ingress; var_idx++) {
        PRINT_INFO("--- var_idx: %d ---\n", var_idx);

        int num_bits = ingress_plan_json_[std::to_string(var_idx)]["num_bits"];
        encoding_field_2_bitlength_.insert({"encoding_i"+std::to_string(var_idx),
                                            num_bits});

        std::map<std::string, std::string> non_action_name_rootword_map;
        for (nlohmann::json::iterator it = ingress_plan_json_[std::to_string(var_idx)]["final_non_action_to_increment_rootword"].begin(); it != ingress_plan_json_[std::to_string(var_idx)]["final_non_action_to_increment_rootword"].end(); ++it) {
            std::string nonaction_name = it.key();
            std::string rootword = it.value();
            non_action_name_rootword_map[nonaction_name] = rootword;
            PRINT_INFO("%s: %s\n", nonaction_name.c_str(), rootword.c_str());
        }

        for (nlohmann::json::iterator it = ingress_plan_json_[std::to_string(var_idx)]["final_edge_dst_to_increment"].begin(); it != ingress_plan_json_[std::to_string(var_idx)]["final_edge_dst_to_increment"].end(); ++it) {
            std::string node_name = it.key();
            int incre = it.value();
            if (non_action_name_rootword_map.find(node_name) != non_action_name_rootword_map.end()) {
            } else {
                // Action
                action_2_encoding_field_.insert({node_name, "_i"+std::to_string(var_idx)});
                action_2_encoding_incr_.insert({node_name, std::to_string(incre)});
                PRINT_INFO("action_2_encoding_field_.insert(%s, %s)", node_name.c_str(), std::to_string(var_idx).c_str());
                PRINT_INFO("action_2_encoding_incr_.insert(%s, %s)", node_name.c_str(), std::to_string(incre).c_str());
            }
        }
    }

    int num_mvbl_vars_egress = egress_plan_json_["num_vars"];
    PRINT_INFO("\nnum_mvbl_vars_egress: %d\n", num_mvbl_vars_egress);
    for (int var_idx = 0; var_idx < num_mvbl_vars_egress; var_idx++) {
        PRINT_INFO("--- var_idx: %d ---\n", var_idx);

        int num_bits = egress_plan_json_[std::to_string(var_idx)]["num_bits"];
        encoding_field_2_bitlength_.insert({"encoding_e"+std::to_string(var_idx),
                                            num_bits});

        // Get the list of non actions
        std::map<std::string, std::string> non_action_name_rootword_map;
        for (nlohmann::json::iterator it = egress_plan_json_[std::to_string(var_idx)]["final_non_action_to_increment_rootword"].begin(); it != egress_plan_json_[std::to_string(var_idx)]["final_non_action_to_increment_rootword"].end(); ++it) {
            std::string nonaction_name = it.key();
            std::string rootword = it.value();
            non_action_name_rootword_map[nonaction_name] = rootword;
            PRINT_INFO("%s: %s\n", nonaction_name.c_str(), rootword.c_str());
        }

        for (nlohmann::json::iterator it = egress_plan_json_[std::to_string(var_idx)]["final_edge_dst_to_increment"].begin(); it != egress_plan_json_[std::to_string(var_idx)]["final_edge_dst_to_increment"].end(); ++it) {
            std::string node_name = it.key();
            int incre = it.value();
            if (non_action_name_rootword_map.find(node_name) != non_action_name_rootword_map.end()) {
            } else {
                // Action
                action_2_encoding_field_.insert({node_name, "_e"+std::to_string(var_idx)});
                action_2_encoding_incr_.insert({node_name, std::to_string(incre)});
                PRINT_INFO("action_2_encoding_field_.insert(%s, %s)", node_name.c_str(), std::to_string(var_idx).c_str());
                PRINT_INFO("action_2_encoding_incr_.insert(%s, %s)", node_name.c_str(), std::to_string(incre).c_str());
            }
        }
    }


    PRINT_INFO("================= DT =================\n");
    // Remove user table and action nodes
    work_set_.clear();
    RemoveTblAction(root);

    // Traverse TableActionStmt
    work_set_.clear();
    GetAction2Tbls(root);

    PRINT_VERBOSE("===prev_action_2_tbls_===\n");
    // fieldModificationFile.open("out/fieldModificationFile.json", ios::out | ios::trunc);
    for (map<string, vector<string> >::iterator ii=prev_action_2_tbls_.begin(); ii!=prev_action_2_tbls_.end(); ++ii) {
        PRINT_VERBOSE("--- %s ---\n", (*ii).first.c_str());
        vector <string> inVect = (*ii).second;
        for (unsigned j=0; j<inVect.size(); j++){
            PRINT_VERBOSE("%s\n", inVect[j].c_str());
        }
    }
    
    CreateHeaderJson(root);
    extseq_ = GetExtractSequence(root);
    PRINT_VERBOSE("Done GetExtractSequence\n");
    tbl2readfields_ = GetMoveFieldsMap(root);
    PRINT_VERBOSE("======== Print tbl2readfields_ =========\n");
    // cout << "out of GetMoveFieldsMap" << endl;
    for (const auto& x : tbl2readfields_) {
        PRINT_VERBOSE("--- %s ---\n", x.first.c_str());
        for (string key : x.second) {
            PRINT_VERBOSE("%s\n", key.c_str());
        }
    }

    CountTables();
    PRINT_VERBOSE("Done GetMoveFieldsMap\n");
    PRINT_VERBOSE("=================\n");
    work_set_.clear();

    GetAction2Nodes(root);
    work_set_.clear();

    GetHdrInstance2Nodes(root);
    work_set_.clear();
    GetMetadataInstance2Nodes(root);    
    GetHdrInstanceCombinations(root);

    work_set_.clear();
    GetHdrTypeDecl2Nodes(root);

    work_set_.clear();
    GetName2TableNodes(root);
    GetMaxTblReadsFieldNum(tbl2readfields_);
    GetMaxTblReadsFieldWidth(root);

    PRINT_INFO("Number of TableNode: %d, max table reads field width: %d, max table reads field len: %d, \
        TblActionStmt: %d, number of ActionNode: %d, number of header instance %d, number of metadata instance %d\n", 
        num_tbl_nodes_, max_tblreads_field_width_, max_tblreads_field_num_, num_tbl_action_stmt_, num_action_,
        num_hdr_instance_, num_metadata_instance_); 

    work_set_.clear();
    DistinguishPopularActions(root);

    work_set_.clear();
    RemoveNodes(root);
    AssignAction2Bitmap();

    AddVisitedHdr();

    AddGlobalMetadata(root);

    ExtractVisitedHdr(root);
    PRINT_VERBOSE("Done ExtractVisitedHdr\n");

    work_set_.clear();
    // MarkActionVisited(root);

    
    
    CloneParser(root);
    SaveExtractPaths(extseq_);
    UpdateParserDAG(root);
    PRINT_VERBOSE("Done UpdateParserDAG\n");

    ClearIngressEgress(root);
    PRINT_VERBOSE("Done ClearIngressEgress\n");

    CloneAllHdrInstances(root);
    PRINT_VERBOSE("Done CloneAllHdrInstances\n");

    // Process packets from UT (visited.type == 1) 
    // Check new actions explored or any assertion failure
    // Either send to control-plane or re-use packet
    AppendIngressProcessUT(root);
    ProcessUTSim(root);
    // Select seed packet (visited.type == 0) or randomize it -- Send to UT
    AppendIngressProcessSeed(root);
    ProcessSeed(root);
    PRINT_VERBOSE("Out of ProcessSeed\n");
    // Send packet from control plane (visited.type==2) to switch under test without modification
    AppendIngressProcessCPU(root);
    PRINT_VERBOSE("Out of AppendIngressProcessCPU\n");
    // Branch to override packet generation and send without modificaiton. Used to send packet from control-plane
    ProcessCPU(root);
    PRINT_VERBOSE("Out of ProcessCPU\n");
    // Apply mutation to packet – maybe resubmit packet for additional mutation if meta.apply_mutations==1
    // AppendIngressMutate(root);
    Mutate(root);
    PRINT_VERBOSE("Out of Mutate\n");
    // Send out packet to correct port or drop
    CorrectPort(root);
    PRINT_VERBOSE("Out of CorrectPort\n");
    // Clone the packet headers as they allow us to update control-plane part of fuzzer later
    AppendIngressAddClones(root);
    PRINT_VERBOSE("Out of AppendIngressAddClones\n");
    Clone(root);
    PRINT_VERBOSE("Out of Clone\n");
    AppendEgressCount(root);
    PRINT_VERBOSE("Out of AppendEgressCount\n");
    UpdateCountEgress(root);
    PRINT_VERBOSE("Out of UpdateCountEgress\n");
    GenerateParserJson(root);
    PRINT_VERBOSE("Out of GenerateParserJson\n");

    WriteHeaderJson(root);

    WriteJsonFile();
    cout << "[Done P4Modifier DT]\n";
}


void DTModifier::DistinguishPopularActions(AstNode* head) {
    if (head == NULL) {
        return;
    }
    if (work_set_.find(head) != work_set_.end()) {
        return;
    }
    work_set_.insert(head);
    // Note that some actions can be present in multiple tables
    if (TypeContains(head, "TableActionStmtNode")) {
        string key = (dynamic_cast<TableActionStmtNode*>(head)) -> name_->toString();
        string tblName = (dynamic_cast<TableNode*>(head -> parent_ -> parent_) -> name_) -> toString();
        PRINT_VERBOSE("TableActionStmt key: %s\n", key.c_str());
        if (prev_action_2_tbls_.count(key) != 0) {
            // if (prev_action_2_tbls_[key].size() > 1) {
                PRINT_VERBOSE("Duplicate TableActionStmt key: %s\n", key.c_str());
                string* keyDistinct = new string(key+"_"+sig_+"_"+tblName);
                dynamic_cast<TableActionStmtNode*>(head) -> name_ -> rename(keyDistinct);
                PRINT_VERBOSE("Coin distinct name: %s\n", (*keyDistinct).c_str());
                tbl_action_stmt_.push_back(*keyDistinct);

                PRINT_VERBOSE("Deep copy ActionNode and rename \n");
                InputNode* iterator = dynamic_cast<InputNode*>(root_);
                while (TypeContains(iterator -> expression_, "IncludeNode")) {
                    iterator = iterator -> next_;
                }
                ActionNode* newAction = dynamic_cast<ActionNode*>(prev_action_2_nodes_[key])->duplicateAction(*keyDistinct);
                newAction->removed_ = true;
                InputNode* newInputNode = new InputNode(iterator -> next_, newAction);
                iterator -> next_ = newInputNode;
            // } else {
            //     tbl_action_stmt_.push_back(key);
            // }
        }
    }

    if (TypeContains(head, "InputNode")) {
        if (dynamic_cast<InputNode*>(head) -> next_) {
            DistinguishPopularActions(dynamic_cast<InputNode*>(head) -> next_);
        }
        DistinguishPopularActions(dynamic_cast<InputNode*>(head) -> expression_);
    } 
    else if (TypeContains(head, "TableNode")) {
        if (dynamic_cast<TableNode*>(head) -> name_) {
            DistinguishPopularActions(dynamic_cast<TableNode*>(head) -> name_);
        }
        if (dynamic_cast<TableNode*>(head) -> reads_) {
            DistinguishPopularActions(dynamic_cast<TableNode*>(head) -> reads_);
        }
        if (dynamic_cast<TableNode*>(head) -> actions_) {
            DistinguishPopularActions(dynamic_cast<TableNode*>(head) -> actions_);
        }
    } 
    else if (TypeContains(head, "TableActionStmtsNode")) {
        TableActionStmtsNode* allActions = dynamic_cast<TableActionStmtsNode*>(head);
        if (allActions -> list_) {
            for (auto item: *(allActions->list_)) {
                DistinguishPopularActions(item);
            }
        }
    } 
}

void DTModifier::GetHdrInstance2Nodes(AstNode* head) {
  if (head == NULL) {
    return;
    }
    if (work_set_.find(head) != work_set_.end()) {
        return;
    }
    work_set_.insert(head);
    if (TypeContains(head, "HeaderInstanceNode")) {
        string key = (dynamic_cast<HeaderInstanceNode*>(head)) -> name_->toString();
        PRINT_VERBOSE("Found HeaderInstanceNode: %s\n", key.c_str());
        prev_hdrinstance_2_nodes_[key] = head;
        num_hdr_instance_ += 1;
    }
    if (TypeContains(head, "InputNode")) {
        if (dynamic_cast<InputNode*>(head) -> next_) {
          GetHdrInstance2Nodes(dynamic_cast<InputNode*>(head) -> next_);
      }
      GetHdrInstance2Nodes(dynamic_cast<InputNode*>(head) -> expression_);
    } 
}


void DTModifier::CountTables() {
    for (const auto& x : tbl2readfields_) {
        for (string key : x.second) {
            controlPlane["table"][x.first].push_back(key);
        }
    }
}


void DTModifier::SaveExtractPaths(vector <vector<string> > extseq) {
    controlPlane["number_of_extract_paths"] = extseq.size();
    map<string, map<string, string> > header2values = GetHeaderValues();
    int pathIndex = 0;
    for (vector<string> path: extseq) {
        for (string header: path) {
            controlPlane["extract"][pathIndex].push_back(header);    
        }
        for (auto iteri = header2values.begin(); iteri!=header2values.end(); iteri++) {
            if (CombineHeader(path).compare(iteri->first)==0) {
                for (auto iterj = iteri->second.begin(); iterj != iteri->second.end(); iterj++) {
                    vector<string> headerFieldVec = split(iterj->first, '.');
                    if (instance_2_header_type_.count(headerFieldVec[0])) {
                        controlPlane["fixed_fields"][pathIndex][iterj->first] = iterj->second;
                    }
                }
            }
        }
        pathIndex += 1;
    }
}

void DTModifier::RemoveTblAction(AstNode* head) {
    if (head == NULL) {
        return;
    }
    if (work_set_.find(head) != work_set_.end()) {
        return;
    }
    work_set_.insert(head);
    if (TypeContains(head, "TableNode")) {
        string tblName = (dynamic_cast<TableNode*>(head) -> name_) -> toString();
        head -> removed_ = true;
        PRINT_VERBOSE("Remove user table node: %s\n", tblName.c_str());
    } else if (TypeContains(head, "ActionNode")) {
        // Remove!!
        string actName = (dynamic_cast<ActionNode*>(head) -> name_) -> toString();
        head -> removed_ = true;
        PRINT_VERBOSE("Remove user action node: %s\n", actName.c_str());
    }
    if (TypeContains(head, "InputNode")) {
        if (dynamic_cast<InputNode*>(head) -> next_) {
            RemoveTblAction(dynamic_cast<InputNode*>(head) -> next_);
        }
        RemoveTblAction(dynamic_cast<InputNode*>(head) -> expression_);
    } 
}


map<string, map<string, string> > DTModifier::GetHeaderValues() {
    return pGraph -> setValues;
}


void DTModifier::SetNodeValues(AstNode* root, DAG* dag) {
    // First adding header types
    AstNode* curr = root;
    while (curr != NULL && TypeContains(curr, "InputNode")) {
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "HeaderTypeDeclarationNode")) {
            HeaderTypeDeclarationNode* headerType = dynamic_cast<HeaderTypeDeclarationNode*>(dynamic_cast<InputNode*>(curr)->expression_);
            string headerTypeName = headerType -> name_ -> toString();
            dag -> addHeaderType(headerTypeName);
            for (FieldDecNode* fieldName : *(headerType -> field_decs_ -> list_)) {
                dag -> addFieldToType(headerTypeName, fieldName -> name_ -> toString());
            }
        }
        curr = dynamic_cast<InputNode*>(curr) -> next_;
    }

    // Adding header instances
    curr = root;
    while (curr != NULL && TypeContains(curr, "InputNode")) {
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "HeaderInstanceNode")) {
            HeaderInstanceNode* headerInstance = dynamic_cast<HeaderInstanceNode*>(dynamic_cast<InputNode*>(curr)->expression_);
            dag -> addHeader(headerInstance -> type_ -> toString(), headerInstance -> name_ -> toString());
        }
        curr = dynamic_cast<InputNode*>(curr) -> next_;
    }
    // cout << "Adding blackboxes" << endl;
    curr = root;
    while (curr != NULL && TypeContains(curr, "InputNode")) {
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "BlackboxNode")) {
            BlackboxNode* blackbox = dynamic_cast<BlackboxNode*>(dynamic_cast<InputNode*>(curr)->expression_);
            string bbName = blackbox -> name_ -> toString();
            dag -> addBlackbox(bbName);
            for (ALUStatementNode* curStatement : *(blackbox -> alu_statements_ -> list_)) {
                if ((curStatement -> command_ -> toString()) == "output_dst") {
                    dag -> addBlackboxOutput(bbName, curStatement -> expr_ -> toString());
                } else {
                    vector<string> startVec;
                    for (string fieldName : curStatement -> expr_ -> GetFields(startVec)) {
                        dag -> addBlackboxModifer(bbName, fieldName);
                    }
                }
            }
        }
        curr = dynamic_cast<InputNode*>(curr) -> next_;
    }
    // cout << "Adding actions" << endl;
    // Adding actions
    curr = root;
    while (curr != NULL && TypeContains(curr, "InputNode")) {
        // cout << "Finding next action node" << endl;
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "ActionNode")) {
            // cout << "it is an action node" << endl;
            ActionNode* actionNode = dynamic_cast<ActionNode*>(dynamic_cast<InputNode*>(curr)->expression_);
            string actionName = actionNode -> name_ -> toString();
            // cout << "Adding action: " << actionName << endl;
            dag -> addAction(actionName, actionNode);
            for (ActionStmtNode* curStatement : *(actionNode -> stmts_ -> list_)) {
                // cout << "in ActionStmtNode list iteration" << endl;
                if (curStatement -> type_ == ActionStmtNode::NAME_ARGLIST && curStatement -> args_ != NULL && curStatement -> args_ -> list_ -> size() > 1) {
                    // cout << "if first if" << endl;
                    ArgsNode* argList = curStatement -> args_;
                    BodyWordNode* firstArg = argList -> list_ -> at(0);
                    BodyWordNode* secArg;
                    for (int i = 1; i < argList -> list_ -> size(); i++) {
                        secArg = argList -> list_ -> at(i);
                        // cout << "second arg" << secArg -> toString() << endl;
                        if (secArg -> wordType_ == BodyWordNode::FIELD) {
                            // cout << "in middle if" << endl;
                            dag -> addFieldDependencyAction(actionName, firstArg -> toString(), secArg -> toString());    
                        } else {
                            // cout << "in middle if - else" << endl;
                            dag -> addFeildModified(actionName, firstArg -> toString());
                        }
                        // cout << "after second arg" << endl;
                    }
                    
                }
                else if (curStatement -> type_ == ActionStmtNode::NAME_ARGLIST && curStatement -> args_ != NULL && curStatement -> args_ -> list_ -> size() > 0) {
                    // cout << "if second if" << endl;
                    // This is for add_header commands
                    string actionCommand = curStatement -> name1_ -> toString();
                    BodyWordNode* firstArg = curStatement -> args_ -> list_ -> at(0);
                    if (actionCommand == "add_header") {
                        // validHeaders[actionCommand] = 1;
                        dag -> addHeaderInAction(actionName, firstArg -> toString());
                    } else if (actionCommand == "remove_header") {
                        // validHeaders[actionCommand] = 0;
                        dag -> removeHeaderInAction(actionName, firstArg -> toString());
                    } else if (actionCommand == "pop") {
                        // validHeaders[actionCommand] -= stoi(args_ -> list_ -> at(1) -> toString());
                        dag -> removeHeaderInAction(actionName, firstArg -> toString());
                    } else if (actionCommand == "push") {
                        // validHeaders[actionCommand] += stoi(args_ -> list_ -> at(1) -> toString());
                        dag -> addHeaderInAction(actionName, firstArg -> toString());
                    }
                    // ArgsNode* argList = curStatement -> args_;
                    // BodyWordNode* firstArg = argList -> list_ -> at(0);
                    // dag -> addHeaderInAction(actionName, firstArg -> toString());
                } else if (curStatement -> type_ == ActionStmtNode::NAME_ARGLIST && curStatement -> args_ != NULL && curStatement -> args_ -> list_ -> size() == 0) {
                    // cout << "if second if" << endl;
                    // This is for add_header commands
                    string actionCommand = curStatement -> name1_ -> toString();
                    if (actionCommand == "drop") {
                        // validHeaders[actionCommand] = 1;
                        dag -> addDropinAction(actionName);
                    } 
                    // ArgsNode* argList = curStatement -> args_;
                    // BodyWordNode* firstArg = argList -> list_ -> at(0);
                    // dag -> addHeaderInAction(actionName, firstArg -> toString());
                } else if (curStatement -> type_ == ActionStmtNode::PROG_EXEC) {    
                    // cout << "if third if" << endl;
                    dag -> addBlackboxModifer(curStatement -> name1_ -> toString(), curStatement -> args_ -> list_ -> at(0) -> toString());
                    // cout << "adding action blackbox" << endl;
                    dag -> addActionBlackbox(actionName, curStatement -> name1_ -> toString());
                    // cout << "going to resolve dependencies" << endl;
                    dag -> resolveBlackboxDependencies(actionName);
                    // cout << "dependencies resolved" << endl;
                }
                // cout << "last line of ActionStmtNode list iteration" << endl;
            }
            // cout << "out of ActionStmtNode list iteration" << endl;
        }
        // cout << "going to next" << endl;
        curr = dynamic_cast<InputNode*>(curr) -> next_;
    }

    map <string, vector<string>> profileNameToAction;
    curr = root;
    while (curr != NULL && TypeContains(curr, "InputNode")) {
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "ActionProfile")) {
            ActionProfile* actionProfile = dynamic_cast<ActionProfile*>(dynamic_cast<InputNode*>(curr)->expression_);
            for (TableActionStmtNode* action : *(actionProfile -> tableActions_ -> list_)) {
                profileNameToAction[actionProfile -> name_ -> toString()].push_back(action -> name_ -> toString());
            }

        }
        curr = dynamic_cast<InputNode*>(curr) -> next_;
    }

    // cout << "adding tables" << endl;
    // Adding tables
    curr = root;
    while (curr != NULL && TypeContains(curr, "InputNode")) {
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "TableNode")) {
            TableNode* table = dynamic_cast<TableNode*>(dynamic_cast<InputNode*>(curr)->expression_);
            string tableName = table -> name_ -> toString();
            dag -> addTable(tableName);
            if (table -> reads_ != NULL) {
                for (TableReadStmtNode* key : *(table -> reads_ -> list_)) {

                    // cout << "Adding key: " << key -> field_ -> toString() << endl;
                    if (key -> matchType_ == TableReadStmtNode::VALID) {
                        controlPlane["original_keys"][tableName][key -> field_ -> toString()] = "valid";
                        continue;
                    } else {
                        // cout << "tableName: " << tableName << " key: " <<  key -> field_ -> toString() << endl;
                        if (!(endsWith(key -> field_ -> toString(), ".valid"))) {
                            dag -> addTableKey(tableName, key -> field_ -> toString());    
                        // } else {
                        //     cout << "Checking for validity" << key -> field_ -> toString() << endl;
                        }
                        if (key -> matchType_ == TableReadStmtNode::EXACT) {
                            controlPlane["original_keys"][tableName][key -> field_ -> toString()] = "exact";
                        } else if (key -> matchType_ == TableReadStmtNode::TERNARY) {
                            controlPlane["original_keys"][tableName][key -> field_ -> toString()] = "ternary";
                        } else if (key -> matchType_ == TableReadStmtNode::LPM) {
                            controlPlane["original_keys"][tableName][key -> field_ -> toString()] = "lpm";
                        } else if (key -> matchType_ == TableReadStmtNode::RANGE) {
                            controlPlane["original_keys"][tableName][key -> field_ -> toString()] = "range";
                        }
                    }
                }
            }
            if (table -> actions_ != NULL) {
                for (TableActionStmtNode* action : *(table -> actions_ -> list_)) {
                    dag -> addTableAction(tableName, action -> name_ -> toString());
                }
            } else if (table -> actionprofile_ -> name_ != NULL) {
                // cout << "If it crashes here, it means we have not properly either added action profile or not searching properly" << endl;
                for (string actionName : profileNameToAction[table -> actionprofile_ -> name_ -> toString()]) {
                    dag -> addTableAction(tableName, actionName);
                }
                // cout << "did not crash" << endl;
            }
        }
        curr = dynamic_cast<InputNode*>(curr) -> next_;
    }
}

void DTModifier::AddParserDependencies(AstNode* root, DAG* dag) {
    AstNode* curr = root;
    string latest_header = "";
    string leftSide = "";
    string rightSide = "";
    while (curr != NULL && TypeContains(curr, "InputNode")) {
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "P4ParserNode")) {
            P4ParserNode* parserNode = dynamic_cast<P4ParserNode*>(dynamic_cast<InputNode*>(curr)->expression_);
            if (parserNode -> body_ -> extSetList_ != NULL) {
                for (ExtractOrSetNode* extSetStatement : *(parserNode -> body_ -> extSetList_ -> list_)) {
                    if (extSetStatement -> extStatement_ != NULL) {
                        latest_header = extSetStatement -> extStatement_ -> headerExtract_ -> name_ -> toString();
                    }
                    if (extSetStatement -> setStatement_ != NULL) {
                        leftSide = extSetStatement -> setStatement_ -> metadata_field_ -> toString();
                        rightSide = extSetStatement -> setStatement_ -> packet_field_ -> toString();
                        if (leftSide.rfind("latest.", 0) == 0) {
                            leftSide.replace(0,7, latest_header + ".");
                        }
                        if (rightSide.rfind("latest.", 0) == 0) {
                            rightSide.replace(0,7, latest_header + ".");
                        }
                        dag -> addFieldDependencyParser(leftSide, rightSide);
                    }
                }
            }
        }
        curr = dynamic_cast<InputNode*>(curr) -> next_;
    }
}

map<string, vector<string> > DTModifier::GetMoveFieldsMap(AstNode* root) {
    DAG* dag = new DAG();
    map<string, vector<string> > out;
    // cout << "going in set node values" << endl;
    PRINT_VERBOSE("Setting NodeValues in GetMoveFieldsMap\n");
    SetNodeValues(root, dag);
    // cout << "out of set node values" << endl;
    PRINT_VERBOSE("Adding ParserDependencies in GetMoveFieldsMap\n");
    AddParserDependencies(root, dag);
    P4ControlNode* curr = FindControlNode(root, "ingress");
    set<string> parents;
    parents.insert("parser");
    // cout << "going in AddControlBlock ingress" << endl;
    parents = AddControlBlock(root, dag, curr -> controlBlock_, parents, true);
    // cout << "going in finding egress, parents: ";
    // for (string par: parents) {
    //     cout << par << ",";
    // }
    // cout << endl;
    curr = FindControlNode(root, "egress");
    if (curr != NULL) {
        // cout << "going in AddControlBlock egress" << endl;
        parents = AddControlBlock(root, dag, curr -> controlBlock_, parents, true);
    }
    dag -> addParserPaths(pGraph -> validHeaders);
    // dag -> metadataFromParser = pGraph -> metadataInfo;
    dag -> metadataDependency = pGraph -> metadataDependency;
    // dag -> parserPaths = pGraph -> validHeaders;
    // cout << "getting fields now" << endl;
    // dag -> printTableToActions();
    PRINT_VERBOSE("Getting GetMoveFieldsMap map\n");
    return dag -> getFields();
}

set<string> DTModifier::AddIfCondition(ExprNode* condition, DAG* dag, set<string> parents, bool evaluationResult) {
    vector<string> fieldsInCondition;
    fieldsInCondition = condition -> GetFields(fieldsInCondition);
    // cout << "fieldsInCondition: " ;
    // for (string field: fieldsInCondition) {
    //     cout << field << " , ";
    // }
    // cout << endl;
    set<string> ifCondParents = dag -> addIfCondition(fieldsInCondition, parents, condition, evaluationResult);
    map<string, vector<string> > fixedValuesInCondtion;
    fixedValuesInCondtion = condition -> GetFixedValues(fixedValuesInCondtion);
    for (auto const& x : fixedValuesInCondtion)
    {
        vector<string> headerFieldVec = split(x.first, '.');
        if (instance_2_header_type_.count(headerFieldVec[0])) {
            for (string fieldName : x.second) {
                controlPlane["fixed_values"][x.first].push_back(fieldName);    
            }
        }
    }
    return ifCondParents;
}

set<string> DTModifier::AddIfBlock(AstNode* root, DAG* dag, IfElseStatement* ifStmt, set<string> parents, bool evaluationResult) {
    set<string> ifCondParents = AddIfCondition(ifStmt -> condition_, dag, parents, evaluationResult);
    set<string> ifParents =  AddControlBlock(root, dag, ifStmt -> controlBlock_, ifCondParents, true);
    set<string> tempParents;
    if (ifStmt -> elseBlock_ != NULL) {
        if (ifStmt -> elseBlock_ -> controlBlock_ != NULL) {
            tempParents = AddControlBlock(root, dag, ifStmt -> elseBlock_ -> controlBlock_, ifCondParents, false);
            ifParents.insert(tempParents.begin(), tempParents.end());
        } else if (ifStmt -> elseBlock_ -> ifElseStatement_ != NULL) {
            tempParents = AddIfBlock(root, dag, ifStmt -> elseBlock_ -> ifElseStatement_, ifCondParents, false);
            ifParents.insert(tempParents.begin(), tempParents.end());
        }
    }
    parents = ifParents;
    // cout << "Returning from if statement, parents: ";
    // for (string par: parents) {
    //     cout << par << ", ";
    // }
    // cout << endl;
    return parents;
}

set<string> DTModifier::AddControlBlock(AstNode* root, DAG* dag, P4ControlBlock* controlBlock, set<string> parents, bool evaluationResult) {
    for (ControlStatement* controlStmt : *(controlBlock -> controlStatements_ -> list_)) {

        if (controlStmt -> stmtType_ == ControlStatement::TABLECALL) {
            ApplyTableCall* tableCall = dynamic_cast<ApplyTableCall*>(controlStmt -> stmt_);
            dag -> setTableParents(tableCall -> name_ -> toString(), parents, evaluationResult);
            parents.clear();
            parents.insert(tableCall -> name_ -> toString());
        } else if (controlStmt -> stmtType_ == ControlStatement::CONTROL) {
            P4ControlNode* controlNode = FindControlNode(root, dynamic_cast<NameNode*>(controlStmt -> stmt_) -> toString());
            parents = AddControlBlock(root, dag, controlNode -> controlBlock_, parents, evaluationResult);
        } else if (controlStmt -> stmtType_ == ControlStatement::IFELSE) {
            IfElseStatement* ifStmt = dynamic_cast<IfElseStatement*>(controlStmt -> stmt_);            
            parents = AddIfBlock(root, dag, ifStmt, parents, evaluationResult);
        } else if (controlStmt -> stmtType_ == ControlStatement::TABLESELECT) {
            ApplyAndSelectBlock* tableStmt = dynamic_cast<ApplyAndSelectBlock*>(controlStmt -> stmt_);  
            dag -> setTableParents(tableStmt -> name_ -> toString(), parents, evaluationResult);
            parents.clear();
            parents.insert(tableStmt -> name_ -> toString());
            set<string> result;
            set<string> tempParents;

            if (tableStmt -> caseList_ != NULL) {
                if (tableStmt -> caseList_ -> actionCases_ != NULL) {
                    for (ActionCase* actionCase : *(tableStmt -> caseList_ -> actionCases_ -> list_)) {
                        tempParents = AddControlBlock(root, dag, actionCase -> controlBlock_, parents, evaluationResult);
                        result.insert(tempParents.begin(), tempParents.end());
                        tempParents.clear();
                    }
                    parents = result;
                } else if (tableStmt -> caseList_ -> hitOrMissCases_ != NULL) {
                    for (HitOrMissCase* hitMissCase : *(tableStmt -> caseList_ -> hitOrMissCases_ -> list_)) {
                        tempParents = AddControlBlock(root, dag, hitMissCase -> controlBlock_, parents, evaluationResult);
                        result.insert(tempParents.begin(), tempParents.end());
                        tempParents.clear();
                    }
                    parents = result;
                }
            }
        } else if (controlStmt -> stmtType_ == ControlStatement::ASSERT) {
            // cout << "It's an assert statement" << endl;
            // AssertNode* tableStmt = dynamic_cast<AssertNode*>(controlStmt -> stmt_); 
            // addAssertTable(tableStmt);
        } else {
            cout << "Unknown control statement" << endl;
        }
    }
    return parents;
}

vector<vector <string> > DTModifier::GetExtractSequence(AstNode* root) {
    AstNode* curr = root;
    pGraph = new ParserGraph();
    while (curr != NULL && TypeContains(curr, "InputNode")) {
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "ParserNode")) {
            P4ParserNode* parserNode =  dynamic_cast<P4ParserNode*>(dynamic_cast<InputNode*>(curr)->expression_);
            pGraph -> addState(parserNode -> name_ -> toString());
            ExtractOrSetList* extSetList = dynamic_cast<ExtractOrSetList*>(parserNode -> body_ -> extSetList_);

            if (extSetList != NULL and extSetList -> list_ != NULL) {
                for (auto item: *(extSetList-> list_)) {
                    // TODO: Add metadata information here - done
                    if (item -> setStatement_ != NULL) {
                        // ExprNode* packet_field_ = item -> setStatement_ -> packet_field_;
                        // vector<string> field_list;
                        // field_list = packet_field_ -> GetFields(field_list);
                        // for (string field : field_list) {
                        pGraph -> addMetadata(parserNode -> name_ -> toString(), item -> setStatement_ -> metadata_field_ -> toString(), item -> setStatement_ -> packet_field_);
                        // }
                    }
                    if (item -> extStatement_ != NULL) {
                        pGraph -> addExtract(parserNode -> name_ -> toString(), item -> extStatement_ -> headerExtract_ -> name_ -> toString());
                    }
                }
            }
            ReturnStatementNode* retStatement = dynamic_cast<ReturnStatementNode*>(parserNode -> body_ -> retStatement_);
            if (retStatement -> single != NULL) {
                pGraph -> addDefaultTransition(parserNode -> name_ -> toString(), retStatement -> single -> retValue_ -> toString());
            } else {
                for (auto item: *(retStatement->selectList -> list_)) {
                    if (item -> field_ != NULL) {
                        pGraph -> addMatchField(parserNode -> name_ -> toString(), item -> field_ -> toString());
                    } else {
                        pGraph -> addMatchCurrent(parserNode -> name_ -> toString(), stoi(item -> current_ -> start_ -> toString()), stoi(item -> current_ -> end_ -> toString()));
                    }
                }
                for (auto item: *(retStatement->caseList -> list_)) {
                    if (item -> matchValue_ == NULL) {
                        pGraph -> addDefaultTransition(parserNode -> name_ -> toString(), item -> retValue_ -> toString());
                    } else {
                        if (dynamic_cast<HexNode*>(item -> matchValue_)) {
                            HexNode* currentMatch = dynamic_cast<HexNode*>(item -> matchValue_);
                            if (currentMatch -> mask_ == NULL) {
                                pGraph -> addConditionTransition(parserNode -> name_ -> toString(), item -> retValue_ -> toString(), currentMatch -> toString());    
                            } else {
                                pGraph -> addConditionMaskTransition(parserNode -> name_ -> toString(), item -> retValue_ -> toString(), *(currentMatch -> word_), *(currentMatch -> mask_)); 
                            }
                        } else {
                            pGraph -> addConditionTransition(parserNode -> name_ -> toString(), item -> retValue_ -> toString(), item -> matchValue_ -> toString());
                        }
                    }
                    
                }
            }

        }
        curr = dynamic_cast<InputNode*>(curr) -> next_;
    }
    curr = root;
    while (curr != NULL && TypeContains(curr, "InputNode")) {
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "HeaderInstanceNode")) {
            HeaderInstanceNode* headerInstance = dynamic_cast<HeaderInstanceNode*>(dynamic_cast<InputNode*>(curr)->expression_);
            if (headerInstance -> numHeaders_ != NULL) {
                pGraph -> addToStack(headerInstance -> name_ -> toString(), stoi(headerInstance -> numHeaders_ -> toString()));
            }
        }
        curr = dynamic_cast<InputNode*>(curr) -> next_;
    }
    // cout << "Now generation extractSequence" << endl;
    // for (auto const& x : instance_2_header_type_) {
    //     cout << "header: " << x.first << endl;
    // }
    pGraph -> generateExtractSequence(instance_2_header_type_);
    // cout << "done generating extractSequence" << endl;
    return pGraph -> extractSequence;
}

void DTModifier::GenerateParserJson(AstNode* root) {
    AstNode* curr = root;
    parser_state_machine["initial_state"] = "start";
    parser_state_machine["final_state"] = "ingress";
    // cout << "in GenerateParserJson before loop" << endl;
    while (curr != NULL && TypeContains(curr, "InputNode")) {
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "ParserNode")) {
            P4ParserNode* parserNode =  dynamic_cast<P4ParserNode*>(dynamic_cast<InputNode*>(curr)->expression_);
            parser_state_machine["states"].push_back(parserNode -> name_ -> toString());
            ExtractOrSetList* extSetList = dynamic_cast<ExtractOrSetList*>(parserNode -> body_ -> extSetList_);
            if (extSetList != NULL and extSetList -> list_ != NULL) {
                for (auto item: *(extSetList-> list_)) {
                    if (item -> extStatement_ != NULL) {
                        parser_state_machine["extract"][parserNode -> name_ -> toString()].push_back(item -> extStatement_ -> headerExtract_ -> toString());
                    }
                }
            }

            ReturnStatementNode* retStatement = dynamic_cast<ReturnStatementNode*>(parserNode -> body_ -> retStatement_);
            if (retStatement -> single != NULL) {
                json transitionJson;
                transitionJson["from"] = parserNode -> name_ -> toString();
                transitionJson["to"] = retStatement -> single -> retValue_ -> toString();
                transitionJson["condition"] = 0;
                parser_state_machine["transitions"].push_back(transitionJson);
            } else {
                for (auto item: *(retStatement->caseList -> list_)) {
                    // cout << "In loop" << endl;
                    if ((retStatement -> selectList -> toString() == "ig_intr_md.ingress_port") && (item -> matchValue_ != NULL)) {
                        // cout << "first condition is okay" << endl;
                        continue;
                    } else if (retStatement -> selectList -> toString() == "ig_intr_md.ingress_port") {
                        // cout << "in second condition" << endl;
                        json transitionJson;
                        transitionJson["from"] = parserNode -> name_ -> toString();
                        transitionJson["to"] = item -> retValue_ -> toString();
                        transitionJson["condition"] = 0;
                        parser_state_machine["transitions"].push_back(transitionJson);
                        // continue;
                    } else {
                        json transitionJson;
                        transitionJson["condition"] = 1;

                        transitionJson["header"] = retStatement -> selectList -> toString();
                        transitionJson["from"] = parserNode -> name_ -> toString();
                        transitionJson["to"] = item -> retValue_ -> toString();
                        if (item -> matchValue_ == NULL) {
                            transitionJson["value"] = "default";
                        } else {
                            transitionJson["value"] = item -> matchValue_ -> toString();
                        }
                        parser_state_machine["transitions"].push_back(transitionJson);
                    }
                }
            }

        }
        curr = dynamic_cast<InputNode*>(curr) -> next_;
    }

    parser_state_machine["extract"]["start"].push_back("pfuzz_visited");
    ofstream parserFile("out/" + program_name_ + "_parserFile.json", ios::out | ios::trunc);
    parserFile << parser_state_machine.dump(4) << endl;
    parserFile.close();
}

void DTModifier::WriteHeaderJson(AstNode* root) {
    AstNode* curr = root;
    map <string, HeaderTypeDeclarationNode*> type_to_declaration;
    while (curr != NULL && TypeContains(curr, "InputNode")) {
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "HeaderTypeDeclarationNode")) {
            HeaderTypeDeclarationNode* headerDecl =  dynamic_cast<HeaderTypeDeclarationNode*>(dynamic_cast<InputNode*>(curr)->expression_);
            string headerName = headerDecl -> name_ -> toString();
            if (headerName.find("pfuzz_visited") != std::string::npos) {
                for(FieldDecNode* field: *(headerDecl->field_decs_->list_)) {
                    json fieldBit;
                    fieldBit[field -> name_ -> toString()] = field -> size_ -> toString();
                    header_field[headerDecl -> name_ -> toString()].push_back(fieldBit);
                }
            }
        }
        curr = dynamic_cast<InputNode*>(curr) -> next_;
    }

    curr = root;

    while (curr != NULL && TypeContains(curr, "InputNode")) {
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "HeaderInstanceNode")) {
            HeaderInstanceNode* headerInst = dynamic_cast<HeaderInstanceNode*>(dynamic_cast<InputNode*>(curr)->expression_);
            string headerName = headerInst -> name_ -> toString();
            if (headerName.find("pfuzz_visited") != std::string::npos) {
                header_field["type_to_instance"][headerInst -> type_ -> toString()] = headerInst -> name_ -> toString();
            }
        }
        curr = dynamic_cast<InputNode*>(curr) -> next_;
    }

    ofstream headerFile("out/" + program_name_ + "_headerFile.json", ios::out | ios::trunc);
    headerFile << header_field.dump(4) << endl;
    headerFile.close();
}

void DTModifier::CreateHeaderJson(AstNode* root) {
    AstNode* curr = root;
    map <string, HeaderTypeDeclarationNode*> type_to_declaration;
    while (curr != NULL && TypeContains(curr, "InputNode")) {
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "HeaderTypeDeclarationNode")) {
            HeaderTypeDeclarationNode* headerDecl =  dynamic_cast<HeaderTypeDeclarationNode*>(dynamic_cast<InputNode*>(curr)->expression_);
            type_to_declaration[headerDecl -> name_ -> toString()] = headerDecl;
            for(FieldDecNode* field: *(headerDecl->field_decs_->list_)) {
                json fieldBit;
                fieldBit[field -> name_ -> toString()] = field -> size_ -> toString();
                header_field[headerDecl -> name_ -> toString()].push_back(fieldBit);
            }
        }
        curr = dynamic_cast<InputNode*>(curr) -> next_;
    }

    curr = root;

    while (curr != NULL && TypeContains(curr, "InputNode")) {
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "HeaderInstanceNode")) {
            HeaderInstanceNode* headerInst = dynamic_cast<HeaderInstanceNode*>(dynamic_cast<InputNode*>(curr)->expression_);
            header_field["type_to_instance"][headerInst -> type_ -> toString()] = headerInst -> name_ -> toString();
            // cout << "Adding: " << headerInst -> name_ -> toString() << " with val: " << type_to_declaration[headerInst -> type_ -> toString()] -> name_ -> toString() << endl;
            instance_2_header_type_[headerInst -> name_ -> toString()] = type_to_declaration[headerInst -> type_ -> toString()];
        }
        curr = dynamic_cast<InputNode*>(curr) -> next_;
    }
}

void DTModifier::CloneAllHdrInstances(AstNode* root) {
    ostringstream oss;
    for (auto iter=prev_hdrinstance_2_nodes_.begin(); iter!=prev_hdrinstance_2_nodes_.end(); ++iter) {
        PRINT_VERBOSE("Clone %s\n", iter->first.c_str());
        HeaderInstanceNode* hdrinstance_node = dynamic_cast<HeaderInstanceNode*>(iter->second);
        oss << "header " << hdrinstance_node->type_->toString() << " " << iter->first << kHdrInstanceCloneSuffix;
        if (hdrinstance_node -> numHeaders_ != NULL) {
            oss << "[" << hdrinstance_node -> numHeaders_ -> toString() << "]";
        }
        oss << ";\n";
        PRINT_VERBOSE("222end\n");
    }
    PRINT_VERBOSE("333\n");
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string("hdrinstanceclones"), new string("hdrinstanceclones")));
    oss.str("");
}

void DTModifier::CloneParser(AstNode* root) {
    PRINT_VERBOSE("Start CloneParser\n");

    std::set<std::string> metadata_instances_in_parser {};

    // Call MakeCopy() for all P4ParsersNode
    AstNode* curr = root;
    while (curr != NULL && TypeContains(curr, "InputNode")) {
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "ParserNode")) {
            P4ParserNode* parserNode = dynamic_cast<P4ParserNode*>(dynamic_cast<InputNode*>(curr)->expression_);
            PRINT_VERBOSE("Call MakeCopy for P4ParserNode: %s\n", parserNode->toString().c_str());
            P4ParserNode* new_parsernode = parserNode->MakeCopy();
            PRINT_VERBOSE("Done parserNode->MakeCopy\n");
            // PRINT_VERBOSE("new_parsernode->toString(): %s\n", new_parsernode->toString().c_str());
            PRINT_VERBOSE("Create new InputNode\n");
            // Add to output tree rather than unanchorednodes
            InputNode* newInputNode = new InputNode(dynamic_cast<InputNode*>(curr)->next_, new_parsernode);
            PRINT_VERBOSE("newInputNode: %s\n", newInputNode->toString().c_str());
            PRINT_VERBOSE("Link deep copied node to the root tree\n");
            dynamic_cast<InputNode*>(curr)-> next_ = newInputNode;

            // Identify set of metadata instances, suffice to focus on search over set_metadata statements
            if (parserNode->body_->extSetList_ != NULL) {
                PRINT_VERBOSE("parserNode->body_->extSetList_ != NULL\n");
                for (auto it : *(parserNode->body_->extSetList_->list_)) {
                    PRINT_VERBOSE("it->toString(): %s\n", it->toString().c_str());
                    if (it->setStatement_ != NULL) {
                        PRINT_VERBOSE("it->setStatement_ != NULL\n");
                        std::string name = it->setStatement_->metadata_field_->headerName_->toString();
                        PRINT_VERBOSE("Identify metadata instance: %s\n", name.c_str());
                        metadata_instances_in_parser.insert(name);
                    } else {
                        PRINT_VERBOSE("it->setStatement_ == NULL\n");
                    }
                }
            } else {
                PRINT_VERBOSE("parserNode->body_->extSetList_ == NULL\n");
            }

            // Skip the pointer (twice in total)
            curr = dynamic_cast<InputNode*>(curr) -> next_;
        } 
        curr = dynamic_cast<InputNode*>(curr) -> next_;
    }
    PRINT_VERBOSE("Print metadata_instances_in_parser and add declaration of cloned metadata instances\n");
    PRINT_VERBOSE("Add declaration of cloned metadata instances");
    for (auto it=metadata_instances_in_parser.begin(); it!=metadata_instances_in_parser.end(); ++it) {
        PRINT_VERBOSE("Declare cloned instance for %s\n", (*it).c_str());
        auto got = prev_metadatainstance_2_nodes_.find(*it);
        if (got == prev_metadatainstance_2_nodes_.end()) {
            PRINT_VERBOSE("ERROR: Not found in prev_metadatainstance_2_nodes_\n");
        } else {
            PRINT_VERBOSE("Found in prev_metadatainstance_2_nodes_\n");
            MetadataInstanceNode* mdinstance_node = dynamic_cast<MetadataInstanceNode*>(prev_metadatainstance_2_nodes_[*it]);
            PRINT_VERBOSE("Add unanchored nodes declaring cloned metadata instance\n");
            // Adding to root tree MetadataInstanceNode is OK as well, but we may need to be careful in future traversals to skip the cloned statement
            unanchored_nodes_.push_back(new UnanchoredNode(new string(mdinstance_node->MakeCopyToString()), new string(*it), new string(*it)));
        }
    }
    PRINT_VERBOSE("Done CloneParser\n");
}

void DTModifier::UpdateParserDAG(AstNode* root) {
    AstNode* curr = root;
    string replacementString = "start_clone";
    string findString = "ingress";
    P4ParserNode* parserNode;
  // Iterate InputNode
    while (curr != NULL && TypeContains(curr, "InputNode")) {
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "ParserNode")) {
            parserNode = dynamic_cast<P4ParserNode*>(dynamic_cast<InputNode*>(curr)->expression_);
            if ((parserNode -> name_ -> toString()).find("_clone") != string::npos) {
                // Do nothing in clone parser
            } else {
                ReturnStatementNode* retStatement = parserNode -> body_ -> retStatement_;

                if (retStatement -> single != NULL) {
                    retStatement -> single -> retValue_ -> findAndReplace(findString, replacementString);
                }
                if (retStatement -> caseList != NULL) {
                    for (auto item: *(retStatement->caseList -> list_)) {
                        item -> retValue_  -> findAndReplace(findString, replacementString);
                    }
                }
            }

        } 
        curr = dynamic_cast<InputNode*>(curr) -> next_;
    }
}

int DTModifier::GetFieldLength(string headerFieldName) {
    // cout << "in GetFieldLength 0 " << endl;
    vector<string> headerFieldVec = split(headerFieldName, '.');
    // cout << "in GetFieldLength 10 " << endl;
    HeaderTypeDeclarationNode* hdrtypedecl_node =  instance_2_header_type_[headerFieldVec[0]];
    // cout << "header name: " << headerFieldVec[0] << endl;
    // cout << hdrtypedecl_node -> name_ -> toString() << endl;
    // cout << "in GetFieldLength 20 " << endl;
    for(FieldDecNode* field: *(hdrtypedecl_node->field_decs_->list_)) {
        // cout << "in GetFieldLength 30 " << endl;
        if (field -> name_ -> toString() == headerFieldVec[1]) {
            // cout << "in GetFieldLength 40 " << endl;
            return stoi(field -> size_ -> toString());
        }
        // cout << "in GetFieldLength 50 " << endl;
    }
    // cout << "in GetFieldLength 60 " << endl;
    return 0;
}

// Scan both Metadata instance and header instances
void DTModifier::GetMaxTblReadsFieldWidth(AstNode* root) {
    PRINT_VERBOSE("Enter GetMaxTblReadsFieldWidth\n");
    for (int i = 0; i < max_tblreads_field_num_; ++i) {
        max_tblreads_field_width_.push_back(0);
    }

    // Sort vectors in tbl2readfields_
    // for (const auto& x : tbl2readfields_) {
    //     tbl2readfields_[]
    // }
    for (auto it = tbl2readfields_.cbegin(); it != tbl2readfields_.cend(); ++it) {
        vector<string> fieldVector = it -> second;
        // sort(fieldVector.begin(), fieldVector.end());
        // cout << "Key: " << it -> first << " values: ";
        // for (string field : fieldVector) {
        //     cout << field << ",";
        // }
        // cout << endl;
        sort(fieldVector.begin(), 
            fieldVector.end(), 
            [&](const string& a, const string& b) {
                return GetFieldLength(a) > GetFieldLength(b); 
            });

        // cout << "After sort -> Key: " << it -> first << " values: ";
        // for (string field : fieldVector) {
        //     cout << field << ",";
        // }
        // cout << endl;
        tbl2readfields_[it->first] = fieldVector;
        //  end(arr), 
        //  [](int a, int b) {return a > b; });
    }


    for (int i = 0; i < max_tblreads_field_num_; ++i) {
        for (const auto& x : tbl2readfields_) {
            if (i < x.second.size()) {
                max_tblreads_field_width_[i] = max(max_tblreads_field_width_[i], GetFieldLength(x.second[i]));
                max_tblreads_field_width_[i] = min(32, max_tblreads_field_width_[i]);
            }
        }
    }
    // for (int i = 0; i < max_tblreads_field_num_; ++i) {
    //     if (max_tblreads_field_width_[i] > 32) {
    //         max_tblreads_field_width_[i] = 32;
    //     }
    // }
}


void DTModifier::GetMaxTblReadsFieldNum(map<string, vector<string> > tableReadFields) {
    for (auto it = tableReadFields.cbegin(); it != tableReadFields.cend(); ++it) {
        if (it->second.size() > max_tblreads_field_num_) {
            max_tblreads_field_num_ = it->second.size();    
        }
        
    }
}

void DTModifier::AddGlobalMetadata(AstNode* root) {
    InputNode* iterator = dynamic_cast<InputNode*>(root_);
    while (TypeContains(iterator -> expression_, "IncludeNode")) {
        iterator = iterator -> next_;
    }
    PRINT_VERBOSE("Locate point after IncludeNode\n");

    HeaderTypeDeclarationNode* newHeader = DeclGlobalHdr();
    InputNode* newInputNode = new InputNode(iterator -> next_, newHeader);

  // Not HeaderInstanceNode
    newInputNode = new InputNode(newInputNode, new MetadataInstanceNode(new NameNode(new string(kGlobalHeaderType)), new NameNode(new string(kGlobalMetadata))));

    iterator -> next_ = newInputNode;
}

HeaderTypeDeclarationNode* DTModifier::DeclGlobalHdr() {
    EmptyNode* other_stmts = new EmptyNode();
    FieldDecsNode* field_list = new FieldDecsNode();

    NameNode* curr_namenode;
    IntegerNode* curr_intnode;
    FieldDecNode* curr_fieldnode;

    curr_namenode = new NameNode(new string("reg_pos_one"));
    curr_intnode = new IntegerNode(new string(to_string(12)));
    curr_fieldnode = new FieldDecNode(curr_namenode, curr_intnode, NULL);
    field_list -> push_back(curr_fieldnode);

    curr_namenode = new NameNode(new string("reg_pos_two"));
    curr_intnode = new IntegerNode(new string(to_string(12)));
    curr_fieldnode = new FieldDecNode(curr_namenode, curr_intnode, NULL);
    field_list -> push_back(curr_fieldnode);

    curr_namenode = new NameNode(new string("reg_val_one"));
    curr_intnode = new IntegerNode(new string(to_string(1)));
    curr_fieldnode = new FieldDecNode(curr_namenode, curr_intnode, NULL);
    field_list -> push_back(curr_fieldnode);

    curr_namenode = new NameNode(new string("reg_val_two"));
    curr_intnode = new IntegerNode(new string(to_string(1)));
    curr_fieldnode = new FieldDecNode(curr_namenode, curr_intnode, NULL);
    field_list -> push_back(curr_fieldnode);

    curr_namenode = new NameNode(new string("apply_mutations"));
    curr_intnode = new IntegerNode(new string(to_string(2)));
    curr_fieldnode = new FieldDecNode(curr_namenode, curr_intnode, NULL);
    field_list -> push_back(curr_fieldnode);

    curr_namenode = new NameNode(new string("temp_port"));
    curr_intnode = new IntegerNode(new string(to_string(4)));
    curr_fieldnode = new FieldDecNode(curr_namenode, curr_intnode, NULL);
    field_list -> push_back(curr_fieldnode);

    curr_namenode = new NameNode(new string("make_clone"));
    curr_intnode = new IntegerNode(new string(to_string(8)));
    curr_fieldnode = new FieldDecNode(curr_namenode, curr_intnode, NULL);
    field_list -> push_back(curr_fieldnode);

    curr_namenode = new NameNode(new string("seed_num"));
    curr_intnode = new IntegerNode(new string(to_string(8)));
    curr_fieldnode = new FieldDecNode(curr_namenode, curr_intnode, NULL);
    field_list -> push_back(curr_fieldnode);

    curr_namenode = new NameNode(new string("fixed_header_seed"));
    curr_intnode = new IntegerNode(new string(to_string(8)));
    curr_fieldnode = new FieldDecNode(curr_namenode, curr_intnode, NULL);
    field_list -> push_back(curr_fieldnode);

    curr_namenode = new NameNode(new string("table_seed"));
    curr_intnode = new IntegerNode(new string(to_string(4)));
    curr_fieldnode = new FieldDecNode(curr_namenode, curr_intnode, NULL);
    field_list -> push_back(curr_fieldnode);

    curr_namenode = new NameNode(new string("__pad1"));
    curr_intnode = new IntegerNode(new string(to_string(4)));
    curr_fieldnode = new FieldDecNode(curr_namenode, curr_intnode, NULL);
    field_list -> push_back(curr_fieldnode);

    curr_namenode = new NameNode(new string("temp_data"));
    curr_intnode = new IntegerNode(new string(to_string(32)));
    curr_fieldnode = new FieldDecNode(curr_namenode, curr_intnode, NULL);
    field_list -> push_back(curr_fieldnode);


    for (int i = 0; i < max_tblreads_field_num_; i++) {
        curr_namenode = new NameNode(new string("max_bits_field"+to_string(i)));
        curr_intnode = new IntegerNode(new string(to_string(max_tblreads_field_width_[i])));
        curr_fieldnode = new FieldDecNode(curr_namenode, curr_intnode, NULL);
        field_list -> push_back(curr_fieldnode);
    }

    curr_namenode = new NameNode(new string(kGlobalHeaderType));

    HeaderTypeDeclarationNode* new_header = new HeaderTypeDeclarationNode(curr_namenode, field_list, other_stmts);

    return new_header;
}

void DTModifier::UpdateCountEgress(AstNode* root) {
    ostringstream oss;
    oss << "register " << kReUpdateCount << " {\n"
        << "  width: 32;\n"
        << "  instance_count: 1;\n"
        << "}\n";

    if (strcmp(target_, "hw") == 0 ) {    
        oss << "blackbox stateful_alu " << kReUpdateCount << "_alu {\n"
            << "    reg: " << kReUpdateCount << ";\n"
            << "    output_value: register_lo;\n"
            << "    output_dst: " << kGlobalMetadata << ".temp_data;\n"
            << "    update_lo_1_value: register_lo + 1;\n"
            << "}\n";
    }

    oss << "action " << kAeUpdateCount << "() {\n";

    if (strcmp(target_, "hw")==0) {
        oss << "    " << kReUpdateCount << "_alu.execute_stateful_alu(0);\n";
    } else {
        oss << "  register_read(" << kGlobalMetadata << ".temp_data, " << kReUpdateCount << ", 0);\n"
            << "  add_to_field(" << kGlobalMetadata << ".temp_data, 1);\n"
            << "  register_write(" << kReUpdateCount << ", 0, " << kGlobalMetadata << ".temp_data);\n";
    }
        
    oss << "}\n";
    oss << "table " << kTeUpdateCount << "{\n"
        << "  actions {\n"
        << "    " << kAeUpdateCount << ";\n"
        << "  }\n"
        << "  default_action: " << kAeUpdateCount << "();\n"
        << "  size: 0;\n"
        << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kTiGetRegPosSim), new string(kTiGetRegPosSim)));
    oss.str("");          
}

void DTModifier::ProcessUTSim(AstNode* root) {
  // Add action ai_get_reg_pos table ti_get_reg_pos
  // Get bloom filter (BFs) positions. BFs to check if we have visited this particular path before
    ostringstream oss;
    oss << "action " << kAiGetRegPosSim << "() {\n"
    << "  modify_field_with_hash_based_offset(" << kGlobalMetadata << ".reg_pos_one, 0, bloom_filter_hash_16, " << BLOOM_FILTER_ENTRIES << ");\n"
    << "  modify_field_with_hash_based_offset(" << kGlobalMetadata << ".reg_pos_two, 0, bloom_filter_hash_32, " << BLOOM_FILTER_ENTRIES << ");\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kTeUpdateCount), new string(kTeUpdateCount)));
    oss.str("");

    oss << "table " << kTiGetRegPosSim << " {\n"
    << "  actions {\n"
    << "    " << kAiGetRegPosSim << ";\n"
    << "  }\n"
    << "  default_action : " << kAiGetRegPosSim << "();\n"
    << "  size: 0;\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kTiGetRegPosSim), new string(kTiGetRegPosSim)));
    oss.str("");  

  // Add bloom_filter_hash_16 and bloom_filter_hash_32 field_list_calculation
    oss << "field_list_calculation bloom_filter_hash_16" << "{\n"
    << "  input { " << kFiBloomFilterHashFieldsSim << "_16; }\n"
    << "  algorithm: crc16;\n"
    << "  output_width : 16;\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string("bloom_filter_hash_16"), new string("bloom_filter_hash_16")));
    oss.str("");
    oss << "field_list_calculation bloom_filter_hash_32" << "{\n"
    << "  input { " << kFiBloomFilterHashFieldsSim << "_32; }\n"
    << "  algorithm: crc32;\n"
    << "  output_width : 16;\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string("bloom_filter_hash_32"), new string("bloom_filter_hash_32")));
    oss.str("");

  // Add field_list with action bitmaps to hash
    // int bitsToAdd = roundUp(num_tbl_action_stmt_, 8);
    // int counter = 0;
    oss << "field_list " << kFiBloomFilterHashFieldsSim << "_16 {\n";
    std::vector<std::string> added_16 {};
    for (auto it = action_2_encoding_field_.begin(); it != action_2_encoding_field_.end(); it++) {
        if (std::find(added_16.begin(), added_16.end(), it->second) != added_16.end()) {
            continue;
        } else {
            added_16.push_back(it->second);
            oss << "  " << kVisitedMetadata << ".encoding" << it->second << ";\n";
        }
    }
    // for (int i = 0; i < tbl_action_stmt_.size(); i++) {
    //     oss << "  " << kVisitedMetadata << "." << tbl_action_stmt_[i] << ";\n";
    // }

    // while (bitsToAdd >= 32) {
    //     oss << "  " << kVisitedMetadata << ".field" << to_string(counter) << ";\n";
    //     bitsToAdd = bitsToAdd - 32;
    //     counter += 1;
    // }
    // if (bitsToAdd > 0) {
    //     oss << "  " << kVisitedMetadata << ".field" << to_string(counter) << ";\n";
    // }
    oss << "}\n\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kFiBloomFilterHashFieldsSim+"_16"), new string(kFiBloomFilterHashFieldsSim+"_16")));
    oss.str("");

    // bitsToAdd = roundUp(num_tbl_action_stmt_, 8);
    // counter = 0;
    oss << "field_list " << kFiBloomFilterHashFieldsSim << "_32 {\n";
    std::vector<std::string> added_32 {};
    for (auto it = action_2_encoding_field_.begin(); it != action_2_encoding_field_.end(); it++) {
        if (std::find(added_32.begin(), added_32.end(), it->second) != added_32.end()) {
            continue;
        } else {
            added_32.push_back(it->second);
            oss << "  " << kVisitedMetadata << ".encoding" << it->second << ";\n";
        }
    }
    // for (int i = 0; i < tbl_action_stmt_.size(); i++) {
    //     oss << "  " << kVisitedMetadata << "." << tbl_action_stmt_[i] << ";\n";
    // }

    // while (bitsToAdd >= 32) {
    //     oss << "  " << kVisitedMetadata << ".field" << to_string(counter) << ";\n";
    //     bitsToAdd = bitsToAdd - 32;
    //     counter += 1;
    // }
    // if (bitsToAdd > 0) {
    //     oss << "  " << kVisitedMetadata << ".field" << to_string(counter) << ";\n";
    // }
    oss << "}\n\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kFiBloomFilterHashFieldsSim+"_32"), new string(kFiBloomFilterHashFieldsSim+"_32")));
    oss.str("");

  // Add registers for bloom filter
    oss << "register " << kRiBloomFilter1Sim << " {\n"
    << "  width: " << BLOOM_FILTER_BIT_WIDTH << ";\n"
    << "  instance_count : " << BLOOM_FILTER_ENTRIES << ";\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kRiBloomFilter1Sim), new string(kRiBloomFilter1Sim)));
    oss.str("");
    oss << "register " << kRiBloomFilter2Sim << " {\n"
    << "  width: " << BLOOM_FILTER_BIT_WIDTH << ";\n"
    << "  instance_count : " << BLOOM_FILTER_ENTRIES << ";\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kRiBloomFilter2Sim), new string(kRiBloomFilter2Sim)));
    oss.str("");

  // Add ti_read_values and ai_read_values

    if (strcmp(target_, "hw")==0) {
        oss << "blackbox stateful_alu " << kRiBloomFilter1Sim << "_alu_update {\n"
            << "    reg: " << kRiBloomFilter1Sim << ";\n"
            << "    output_value: alu_lo;\n"
            << "    output_dst: " << kGlobalMetadata << ".reg_val_one;\n"
            << "    update_lo_1_value: set_bit;\n"
            << "}\n";
        oss << "blackbox stateful_alu " << kRiBloomFilter2Sim << "_alu_update {\n"
            << "    reg: " << kRiBloomFilter2Sim << ";\n"
            << "    output_value: alu_lo;\n"
            << "    output_dst: " << kGlobalMetadata << ".reg_val_two;\n"
            << "    update_lo_1_value: set_bit;\n"
            << "}\n";
    }

    oss << "action " << kAiReadBloomFilter1Sim << "() {\n";
    if (strcmp(target_, "hw")==0) {
        oss << "    " << kRiBloomFilter1Sim << "_alu_update.execute_stateful_alu(" << kGlobalMetadata << ".reg_pos_one);\n";
    } else {
        oss << "  register_read(" << kGlobalMetadata << ".reg_val_one, " << kRiBloomFilter1Sim << ", " << kGlobalMetadata << ".reg_pos_one" << ");\n";
        oss << "  register_write(" << kRiBloomFilter1Sim << ", " << kGlobalMetadata << ".reg_pos_one, 1);\n";

    }
    oss << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kAiReadBloomFilter1Sim), new string(kAiReadBloomFilter1Sim)));
    oss.str("");

    oss << "action " << kAiReadBloomFilter2Sim << "() {\n";
    if (strcmp(target_, "hw")==0) {
        oss << "    " << kRiBloomFilter2Sim << "_alu_update.execute_stateful_alu(" << kGlobalMetadata << ".reg_pos_two);\n";
    } else {
        oss << "  register_read(" << kGlobalMetadata << ".reg_val_two, " << kRiBloomFilter2Sim << ", " << kGlobalMetadata << ".reg_pos_two" << ");\n";
        oss << "  register_write(" << kRiBloomFilter2Sim << ", " << kGlobalMetadata << ".reg_pos_two, 1);\n";

    }
    oss << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kAiReadBloomFilter2Sim), new string(kAiReadBloomFilter2Sim)));
    oss.str("");

    oss << "table " << kTiReadBloomFilter1Sim << " {\n"
    << "  actions {\n"
    << "    " << kAiReadBloomFilter1Sim << ";\n"
    << "  }\n"
    << "  default_action : " << kAiReadBloomFilter1Sim << "();\n"
    << "  size: 0;\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kTiReadBloomFilter1Sim), new string(kTiReadBloomFilter1Sim)));
    oss.str("");

    oss << "table " << kTiReadBloomFilter2Sim << " {\n"
    << "  actions {\n"
    << "    " << kAiReadBloomFilter2Sim << ";\n"
    << "  }\n"
    << "  default_action : " << kAiReadBloomFilter2Sim << "();\n"
    << "  size: 0;\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kTiReadBloomFilter2Sim), new string(kTiReadBloomFilter2Sim)));
    oss.str("");

  // ti_path_assertion
  // If new paths or any assertion failures, send the packet to control-plane; otherwise, re-use the packet
  // TODO(liangcheng) Add visited assertion related bits to reads
    controlPlane["table_keys"][kTiPathAssertionSim].push_back(kGlobalMetadata + "_reg_val_one");
    controlPlane["table_keys"][kTiPathAssertionSim].push_back(kGlobalMetadata + "_reg_val_two");
    oss << "table " << kTiPathAssertionSim << " {\n"
        << "  reads {\n"
        << "    " << kGlobalMetadata << ".reg_val_one : exact;\n"
        << "    " << kGlobalMetadata << ".reg_val_two : exact;\n";
    for (int i = 0; i < num_assertions_; i++) {
        oss << "    " << kGlobalMetadata << ".assertion" << i << " : exact;\n";
    }
    oss << "  }\n"
        << "  actions {\n"
        << "    " << kAiSendToControlSim << ";\n"
        << "    " << kAiRecyclePktSim << ";\n"
        << "  }\n"
        << "  default_action : " << kAiRecyclePktSim << "();\n"
        << "  size: " << to_string((int) pow(2,2 + num_assertions_)) << ";\n"
        << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kTiPathAssertionSim), new string(kTiPathAssertionSim)));
    oss.str(""); 

  // ai_send_to_control_plane
    oss << "action " << kAiSendToControlSim << "() {\n";
    if (strcmp(target_, "hw")==0) {
        oss << "  modify_field(ig_intr_md_for_tm.ucast_egress_port, " << 192 << ");\n";
        oss << "  modify_field(pfuzz_visited.preamble, 14593470);\n";
        oss << "  exit();\n";
    } else {
        oss << "  modify_field(standard_metadata.egress_spec, " << 31 << ");\n";
        oss << "  modify_field(pfuzz_visited.preamble, 14593470);\n";
        oss << "  exit();\n";
    }
    
    oss << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kAiSendToControlSim), new string(kAiSendToControlSim)));
    oss.str("");  

  // ai_recycle_packet
    oss << "action " << kAiRecyclePktSim << "() {\n";
    // TODO: Remove header needs to fixed
    for (map<string, AstNode*>::iterator iter=prev_hdrinstance_2_nodes_.begin(); iter!=prev_hdrinstance_2_nodes_.end(); ++iter) {
        PRINT_VERBOSE("Headerinstance: %s\n", (*iter).first.c_str());
        std::string hdrinstance_name = (*iter).first;
        HeaderInstanceNode* hdrinstance_node = dynamic_cast<HeaderInstanceNode*>(prev_hdrinstance_2_nodes_[hdrinstance_name]);
        if (hdrinstance_node -> numHeaders_ == NULL) {
            oss << "  remove_header(" << hdrinstance_name << ");\n"
            << "  remove_header(" << hdrinstance_name << kHdrInstanceCloneSuffix << ");\n";
        } else {
            oss << "  pop(" << hdrinstance_name << "," << hdrinstance_node -> numHeaders_ -> toString() << ");\n"
            << "  pop(" << hdrinstance_name << kHdrInstanceCloneSuffix << "," << hdrinstance_node -> numHeaders_ -> toString() << ");\n";
        }
    }  
    oss << "  modify_field(" << kVisitedMetadata << ".pkt_type, 0);\n";
    std::vector<std::string> tmp {};
    for (auto it = action_2_encoding_field_.begin(); it != action_2_encoding_field_.end(); it++) {
        if (std::find(tmp.begin(), tmp.end(), it->second) != tmp.end()) {
            continue;
        } else {
            tmp.push_back(it->second);
            oss << "  modify_field(" << kVisitedMetadata << ".encoding" << it->second << ", 0);\n";
        }
    }
    // for (int i = 0; i < tbl_action_stmt_.size(); i++ ) {
    //     oss << "  modify_field(" << kVisitedMetadata << "." << tbl_action_stmt_[i] << ", 0);\n";
    // }

    // TODO: Currently, field0 is hardcoded, replace it with a loop later
        // << "  modify_field(" << kVisitedMetadata << ".field0, 0);\n"
    oss << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kAiRecyclePktSim), new string(kAiRecyclePktSim)));
    oss.str("");
    PRINT_VERBOSE("Quit ProcessUTSim\n");
}

// std::string DTModifier::CombineHeader(std::vector<std::string> vec) {
//     std::string ret = "";
//     if (vec.size() != 0) {
//         for (int i=0; i<vec.size(); i++) {
//             if (i==0) {
//                 if (vec[i].back() == '*') {
//                     vector<string> nameAndCount = split(vec[i], '*');
//                     ret += (nameAndCount[0]+nameAndCount[1]);
//                 } else {
//                     ret += vec[0];    
//                 }
//             } else {
//                 if (vec[i].back() == '*') {
//                     vector<string> nameAndCount = split(vec[i], '*');
//                     ret += ("_"+nameAndCount[0]+nameAndCount[1]);
//                 } else {
//                     ret += ("_"+vec[i]);
//                 }
//             }
//         }
//     }
//     return ret;
// }

void DTModifier::ProcessSeed(AstNode* root) {
    ostringstream oss;
    // ti_get_random_seed
    // cout << "in ProcessSeed" << endl;
    oss << "table " << kTiGetSeed << " {\n"
    << "  actions {\n"
    << "    " << kAiGetSeed << ";\n"
    << "  }\n"
    << "  default_action : " << kAiGetSeed << "();\n"
    << "  size: 0;\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kTiGetSeed), new string(kTiGetSeed)));
    oss.str("");
    // cout << "in ProcessSeed10" << endl;
  // ai_get_random_seed
    oss << "action " << kAiGetSeed << "() {\n"
    << "  modify_field_rng_uniform(" << kGlobalMetadata << ".seed_num, 0 , 255);\n"
    << "  modify_field_rng_uniform(" << kGlobalMetadata << ".make_clone, 0, 255);\n"
    << "  modify_field_rng_uniform(" << kGlobalMetadata << ".fixed_header_seed, 0, 255);\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kAiGetSeed), new string(kAiGetSeed)));
    oss.str("");


    // ti_turn_on_mutation
    oss << "table " << kTiTurnMutation << " {\n"
    << "  actions {\n"
    << "    " << kAiTurnMutation << ";\n"
    << "  }\n"
    << "  default_action : " << kAiTurnMutation << "();\n"
    << "  size: 0;\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kTiTurnMutation), new string(kTiTurnMutation)));
    oss.str("");
    // cout << "in ProcessSeed10" << endl;
  // ai_get_random_seed
    oss << "action " << kAiTurnMutation << "() {\n"
    << "  modify_field(" << kGlobalMetadata << ".apply_mutations, 1);\n"
    << "  modify_field_rng_uniform(" << kGlobalMetadata << ".temp_port, 0, 15);\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kAiTurnMutation), new string(kAiTurnMutation)));
    oss.str("");

    controlPlane["action_params"][kAiSetSeed].push_back("real_seed_num");

    // ai_set_seed_num
    oss << "action " << kAiSetSeed << "(real_seed_num) {\n"
    << "  modify_field(" << kGlobalMetadata << ".seed_num, real_seed_num);\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kAiSetSeed), new string(kAiSetSeed)));
    oss.str("");

    // oss << "action " << kAiSetSeed << "(real_seed_num) {\n"
    // << "  bit_and(" << kGlobalMetadata << ".seed_num, " << kGlobalMetadata <<  ".seed_num, real_seed_num);\n"
    // << "}\n";
    // unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kAiSetSeed), new string(kAiSetSeed)));
    // oss.str("");
    controlPlane["table_keys"][kTiSetSeed].push_back(kGlobalMetadata + "_seed_num");
    // ti_set_seed_num
    oss << "table " << kTiSetSeed << " {\n"
    << "  reads {\n"
    << "    " << kGlobalMetadata << ".seed_num : ternary;\n"
    << "  }\n"
    << "  actions {\n"
    << "    " << kAiSetSeed << ";\n"
    << "    " << kAiNoAction << ";\n"
    << "  }\n"
    << "  default_action : " << kAiNoAction << "();\n"
    // << "  default_action : " << kAiNoAction << "(1);\n"
    << "  size: 64;\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kTiSetSeed), new string(kTiSetSeed)));
    oss.str("");
    
    controlPlane["action_params"][kAiSetResubmit].push_back("real_resubmit");
    // ai_set_resubmit
    oss << "action " << kAiSetResubmit << "(real_resubmit) {\n"
    << "  modify_field(" << kGlobalMetadata << ".make_clone, real_resubmit);\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kAiSetResubmit), new string(kAiSetResubmit)));
    oss.str("");

    // ti_set_resubmit
    controlPlane["table_keys"][kTiSetResubmit].push_back(kGlobalMetadata + "_make_clone");
    // ai_set_resubmit
    oss << "table " << kTiSetResubmit << " {\n"
    << "  reads {\n"
    << "    " << kGlobalMetadata << ".make_clone : ternary;\n"
    << "  }\n"
    << "  actions {\n"
    << "    " << kAiSetResubmit << ";\n"
    << "    " << kAiNoAction << ";\n"
    << "  }\n"
    << "  default_action : " << kAiNoAction << "();\n"
    << "  size: 1;\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kTiSetResubmit), new string(kTiSetResubmit)));
    oss.str("");

    // ti_set_fixed_header
    controlPlane["table_keys"][kTiSetFixed].push_back(kGlobalMetadata + "_fixed_header_seed");
    oss << "table " << kTiSetFixed << " {\n"
    << "  reads {\n"
    << "    " << kGlobalMetadata << ".make_clone : range;\n"
    << "    " << kGlobalMetadata << ".fixed_header_seed : ternary;\n"
    << "  }\n"
    << "  actions {\n"
    << "    " << kAiSetFixed << ";\n"
    << "    " << kAiNoAction << ";\n"
    << "  }\n"
    << "  default_action : " << kAiNoAction << "();\n"
    << "  size: 1;\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kTiSetFixed), new string(kTiSetFixed)));
    oss.str("");

    controlPlane["action_params"][kAiSetFixed].push_back("real_fixed_header");
    oss << "action " << kAiSetFixed << "(real_fixed_header) {\n"
    << "  modify_field(" << kGlobalMetadata << ".fixed_header_seed, real_fixed_header);\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kAiSetFixed), new string(kAiSetFixed)));
    oss.str("");

    controlPlane["table_keys"][kTiCreatePkt].push_back(kGlobalMetadata + "_seed_num");
  // ti_create_packet
    oss << "table " << kTiCreatePkt << " {\n"
    << "  reads {\n"
    << "    " << kGlobalMetadata << ".seed_num: exact;\n"
    << "  }\n"
    << "  actions {\n"
    << "    " << kAiDrop << ";\n";
    for (int i = 0; i < extseq_.size(); i++) {
        oss << "    ai_add_" << CombineHeader(extseq_[i]) << ";\n";
    }
    oss << "  }\n"
    << "  default_action : " << kAiDrop << "();\n"
    << "  size: 64;\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kTiCreatePkt), new string(kTiCreatePkt)));
    oss.str("");

    // ai_add_* (all combinations of header instances for now)
    map<string, map<string, string> > header2values = GetHeaderValues();
    for (int i = 0; i < extseq_.size(); i++) {
        oss << "action ai_add_" << CombineHeader(extseq_[i]) << "() {\n";
        for (int j = 0; j < extseq_[i].size(); j++) {
            if (extseq_[i][j].back() == '*') {
            // if (isdigit(extseq_[i][j].back())) {
                vector<string> nameAndCount = split(extseq_[i][j], '*');
                oss << "  push(" << nameAndCount[0] << "," << nameAndCount[1] << ");\n";
                oss << "  push(" << nameAndCount[0] << "_clone," << nameAndCount[1] << ");\n";
            } else {
                oss << "  add_header(" << extseq_[i][j] << ");\n";
                oss << "  add_header(" << extseq_[i][j] << "_clone);\n";    
            }
            
        }
        for (auto iteri = header2values.begin(); iteri!=header2values.end(); iteri++) {
            if (CombineHeader(extseq_[i]).compare(iteri->first)==0) {
                for (auto iterj = iteri->second.begin(); iterj != iteri->second.end(); iterj++) {
                    vector<string> headerFieldVec = split(iterj->first, '.');
                    if (instance_2_header_type_.count(headerFieldVec[0])) {
                        oss << "  modify_field(" << iterj->first << ", " << iterj->second << ");\n";
                    }
                }
            }
        }        
        oss << "}\n";
        unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kAiGetSeed), new string(kAiGetSeed)));
        oss.str("");    
    }
    // cout << "in ProcessSeed40" << endl;

    controlPlane["table_keys"][kTiAddFields].push_back(kGlobalMetadata + "_seed_num");
  // ti_add_fields
  // Seed packets could contain both fixed fields or random fields
    oss << "table " << kTiAddFields << " {\n"
    << "  reads {\n"
    << "    " << kGlobalMetadata << ".seed_num: exact;\n"
    << "  }\n"
    << "  actions {\n"
    << "    " << kAiNoAction << ";\n";
    for (int i = 0; i < extseq_.size(); i++) {
        // oss << "    ai_add_random_" << CombineHeader(extseq_[i]) << ";\n";
        oss << "    ai_add_fixed_" << CombineHeader(extseq_[i]) << ";\n";
    }      
    oss << "  }\n"
    << "  default_action : " << kAiNoAction << "();\n"
    << "  size: 64;\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kTiAddFields), new string(kTiAddFields)));
    oss.str("");
    // cout << "in ProcessSeed50" << endl;

    PRINT_VERBOSE("ai_add_{fixed, random}_* (different combinations of header instances)\n");
    // cout << "in ProcessSeed60" << endl;
    string headerInstanceName;
    HeaderInstanceNode* hdrinstance_node;
    HeaderTypeDeclarationNode* hdrtypedecl_node;
    int stopPoint = 0;
    for (int i = 0; i < extseq_.size(); i++) {
        PRINT_VERBOSE("------ Synthesizing %dth ai_add_fixed_* action: %s ------\n", i, CombineHeader(extseq_[i]).c_str());
        oss << "action ai_add_fixed_" << CombineHeader(extseq_[i]) << "(";
    // Add all related fileds as action args
    // TODO(liangcheng) What if fields reach limit of action num? (Set worst case limit of action arg num and replicate)
        bool is_first = true;
        for (int j = 0; j < extseq_[i].size(); j++) {
            PRINT_VERBOSE("Process %dth instance: %s\n", j, extseq_[i][j].c_str());
            // if (isdigit(extseq_[i][j].back())) {
            if (extseq_[i][j].back() == '*') {
                vector<string> nameAndCount = split(extseq_[i][j], '*');
                PRINT_VERBOSE("%c satisfy isdigit\n", nameAndCount[1]);
                headerInstanceName = nameAndCount[0];
                PRINT_VERBOSE("Get headerInstanceName after remove LastChar: %s\n", headerInstanceName.c_str());
                if (TypeContains(prev_hdrinstance_2_nodes_[headerInstanceName], "HeaderInstanceNode")) {
                    hdrinstance_node = dynamic_cast<HeaderInstanceNode*>(prev_hdrinstance_2_nodes_[headerInstanceName]);
                    if (hdrinstance_node != NULL) {
                        hdrtypedecl_node = dynamic_cast<HeaderTypeDeclarationNode*>(prev_hdrtypedecl_2_nodes_[hdrinstance_node->type_->toString()]);
                        PRINT_VERBOSE("Get hdrtypedecl_node: %s\n", hdrtypedecl_node->toString().c_str());
                        stopPoint = stoi(nameAndCount[1]);
                        PRINT_VERBOSE("stopPoint: %d\n", stopPoint);
                        for (int k = 0; k < stopPoint; k++) {
                            PRINT_VERBOSE("k: %d\n", k);
                            for(FieldDecNode* field: *(hdrtypedecl_node->field_decs_->list_)) {
                                if (!is_first) oss << ", ";
                                controlPlane["action_params"]["ai_add_fixed_" + CombineHeader(extseq_[i])].push_back(headerInstanceName + to_string(k) + field->name_->toString());
                                oss << headerInstanceName << k << field->name_->toString();
                                // cout << "added to carg list" << endl;
                                PRINT_VERBOSE("Add to arg list: %s%d%s\n", headerInstanceName.c_str(), k, field->name_->toString().c_str());
                                is_first = false;
                           }
                        }
                    }
                } else {
                    PANIC("%s NOT HeaderInstanceNode node!\n", headerInstanceName.c_str());
                }
            } else {
                PRINT_VERBOSE("%c doesn't satisfy isdigit\n", extseq_[i][j].back());
                headerInstanceName = extseq_[i][j];
                PRINT_VERBOSE("Get headerInstanceName: %s\n", headerInstanceName.c_str());
                if (TypeContains(prev_hdrinstance_2_nodes_[headerInstanceName], "HeaderInstanceNode")) {
                    HeaderInstanceNode* hdrinstance_node = dynamic_cast<HeaderInstanceNode*>(prev_hdrinstance_2_nodes_[headerInstanceName]);
                    if (hdrinstance_node != NULL) {
                        hdrtypedecl_node = dynamic_cast<HeaderTypeDeclarationNode*>(prev_hdrtypedecl_2_nodes_[hdrinstance_node->type_->toString()]);
                        PRINT_VERBOSE("Get hdrtypedecl_node: %s\n", hdrtypedecl_node->toString().c_str());
                        for(FieldDecNode* field: *(hdrtypedecl_node->field_decs_->list_)) {
                            if (!is_first) oss << ", ";
                            controlPlane["action_params"]["ai_add_fixed_" + CombineHeader(extseq_[i])].push_back(headerInstanceName + field->name_->toString());
                            oss << headerInstanceName << field->name_->toString();
                            PRINT_VERBOSE("Add to arg list: %s%s\n", headerInstanceName.c_str(), field->name_->toString().c_str());
                            is_first = false;
                        }
                    } else {
                        PANIC("%s NOT HeaderInstanceNode node!\n", headerInstanceName.c_str());
                    }
                }
            }
            // cout << "next j" << endl;
        }    
        oss << ") {\n";
        PRINT_VERBOSE("Done action argument, start action body\n");
        for (int j = 0; j < extseq_[i].size(); j++) {
            PRINT_VERBOSE("Process %dth instance: %s\n", j, extseq_[i][j].c_str());
            // if (isdigit(extseq_[i][j].back())) {
            if (extseq_[i][j].back() == '*') {
                vector<string> nameAndCount = split(extseq_[i][j], '*');
                PRINT_VERBOSE("%c satisfy isdigit\n", nameAndCount[1]);
                headerInstanceName = nameAndCount[0];
                PRINT_VERBOSE("Get headerInstanceName after remove LastChar: %s\n", headerInstanceName.c_str());
                if (TypeContains(prev_hdrinstance_2_nodes_[headerInstanceName], "HeaderInstanceNode")) {
                    hdrinstance_node = dynamic_cast<HeaderInstanceNode*>(prev_hdrinstance_2_nodes_[headerInstanceName]);
                    if (hdrinstance_node != NULL) {
                        hdrtypedecl_node = dynamic_cast<HeaderTypeDeclarationNode*>(prev_hdrtypedecl_2_nodes_[hdrinstance_node->type_->toString()]);
                        PRINT_VERBOSE("Get hdrtypedecl_node: %s\n", hdrtypedecl_node->toString().c_str());
                        stopPoint = stoi(nameAndCount[1]);
                        for (int k = 0; k < stopPoint; k++) {
                            PRINT_VERBOSE("k: %d\n", k);
                            for(FieldDecNode* field: *(hdrtypedecl_node->field_decs_->list_)) {
                                oss << "  modify_field(" << headerInstanceName << "[" << k << "]." << field->name_->toString() << ", " << headerInstanceName << k << field->name_->toString() << ");\n";
                                PRINT_VERBOSE("Add action body statement: modify_field(%s[%d].%s, %s%d%s);\n", headerInstanceName.c_str(), k, field->name_->toString().c_str(), headerInstanceName.c_str(), k, field->name_->toString().c_str());
                            }
                        }
                    } else {
                        PANIC("%s NOT HeaderInstanceNode node!\n", headerInstanceName.c_str());
                    }
                }
            } else {
                PRINT_VERBOSE("%c doesn't satisfy isdigit\n", extseq_[i][j].back());
                headerInstanceName = extseq_[i][j];
                PRINT_VERBOSE("Get headerInstanceName: %s\n", headerInstanceName.c_str());
                if (TypeContains(prev_hdrinstance_2_nodes_[headerInstanceName], "HeaderInstanceNode")) {
                    hdrinstance_node = dynamic_cast<HeaderInstanceNode*>(prev_hdrinstance_2_nodes_[headerInstanceName]);
                    if (hdrinstance_node != NULL) {
                        hdrtypedecl_node = dynamic_cast<HeaderTypeDeclarationNode*>(prev_hdrtypedecl_2_nodes_[hdrinstance_node->type_->toString()]);
                        PRINT_VERBOSE("Get hdrtypedecl_node: %s\n", hdrtypedecl_node->toString().c_str());
                        for(FieldDecNode* field: *(hdrtypedecl_node->field_decs_->list_)) {
                            oss << "  modify_field(" << headerInstanceName << "." << field->name_->toString() << ", " << extseq_[i][j] << field->name_->toString() << ");\n";
                            PRINT_VERBOSE("Add action body statement: modify_field(%s.%s, %s%s);\n", headerInstanceName.c_str(), field->name_->toString().c_str(), headerInstanceName.c_str(), field->name_->toString().c_str());
                        }
                    } else {
                        PANIC("%s NOT HeaderInstanceNode node!\n", headerInstanceName.c_str());
                    }
                }
            }
        }
        PRINT_VERBOSE("------ Done action body ------\n");
        oss << "}\n";
        unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string("ai_add_fixed_"+CombineHeader(extseq_[i])), new string("ai_add_fixed_"+CombineHeader(extseq_[i]))));
        oss.str("");
        
    }
    // cout << "in ProcessSeed100" << endl;
    PRINT_VERBOSE("Done Seed\n");
}

void DTModifier::Clone(AstNode* root) {
    ostringstream oss;
    HeaderInstanceNode* hdrinstance_node;
  // ti_add_clones
    oss << "table " << kTiAddClones << " {\n"
    << "  reads {\n";
  
    std::map<string, int> countArrayNum;
    for (int i = 0; i < extseq_.size(); i++) {
        for (int j = 0; j < extseq_[i].size(); j++) {
            if (extseq_[i][j].back() == '*') {
                vector<string> nameAndCount = split(extseq_[i][j], '*');
                if (countArrayNum.count(nameAndCount[0])) {
                    countArrayNum[nameAndCount[0]] = max(countArrayNum[nameAndCount[0]], stoi(nameAndCount[1]));
                } else {
                    countArrayNum[nameAndCount[0]] = stoi(nameAndCount[1]);
                }
            }
        }
    }

    // Iterate every header instance
    for (auto iter=prev_hdrinstance_2_nodes_.begin(); iter!=prev_hdrinstance_2_nodes_.end(); ++iter) {
        hdrinstance_node = dynamic_cast<HeaderInstanceNode*>(iter->second);
        if (hdrinstance_node -> numHeaders_ == NULL) {
            oss << "    " << iter->first << ".valid: exact;\n";   
            controlPlane["table_keys"][kTiAddClones].push_back(iter->first + "_valid") ;
        } else {
            for (int i = 0; i < countArrayNum[iter -> first]; i++) {
                oss << "    " << iter->first << "[" << i << "].valid: exact;\n";    
                controlPlane["table_keys"][kTiAddClones].push_back(iter->first + "[" + to_string(i) + "]_valid") ;
            }
        }
    }

    oss << "  }\n"
    << "  // I think the default action should be ai_NoAction\n"
    << "  actions {\n"
    << "    " << kAiNoAction << ";\n";
    for (int i = 0; i < extseq_.size(); i++) {
        oss << "    ai_add_clone_" << CombineHeader(extseq_[i]) << ";\n";
    }      
    oss << "  }\n"
    << "  default_action : " << kAiNoAction << "();\n"
    << "  size: 16;\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kTiAddClones), new string(kTiAddClones)));
    oss.str("");
  // ai_add_clone_* (hdr instance combinations)
  // TODO(liangcheng) Again, may need to refer to parser state machine DAG
    
    HeaderTypeDeclarationNode* hdrtypedecl_node;
    string headerInstanceName;
    int stopPoint = 0;
    for (int i = 0; i < extseq_.size(); i++) {
        oss << "action ai_add_clone_" << CombineHeader(extseq_[i]) << "() {\n";
        for (int j = 0; j < extseq_[i].size(); j++) {
            // if (isdigit(extseq_[i][j].back())) {
            if (extseq_[i][j].back() == '*') {
                vector<string> nameAndCount = split(extseq_[i][j], '*');
                // headerInstanceName = removeLastChar(extseq_[i][j]);
                headerInstanceName = nameAndCount[0];
                if (TypeContains(prev_hdrinstance_2_nodes_[headerInstanceName], "HeaderInstanceNode")) {
                    hdrinstance_node = dynamic_cast<HeaderInstanceNode*>(prev_hdrinstance_2_nodes_[headerInstanceName]);
                    if (hdrinstance_node != NULL) {
                        hdrtypedecl_node = dynamic_cast<HeaderTypeDeclarationNode*>(prev_hdrtypedecl_2_nodes_[hdrinstance_node->type_->toString()]);
                        stopPoint = stoi(nameAndCount[1]);
                        for (int k = 0; k < stopPoint; k++) {
                            for(FieldDecNode* field: *(hdrtypedecl_node->field_decs_->list_)) {
                                oss << "  modify_field(" << headerInstanceName << "_clone[" << k << "]." << field->name_->toString() << ", " << headerInstanceName << "[" << k << "]." << field->name_->toString() << ");\n";
                            }
                        }
                    } else {
                        PANIC("%s NOT HeaderInstanceNode node!\n", extseq_[i][j].c_str());
                    }
                }

            } else {
                if (TypeContains(prev_hdrinstance_2_nodes_[extseq_[i][j]], "HeaderInstanceNode")) {
                    hdrinstance_node = dynamic_cast<HeaderInstanceNode*>(prev_hdrinstance_2_nodes_[extseq_[i][j]]);
                    if (hdrinstance_node != NULL) {
                        hdrtypedecl_node = dynamic_cast<HeaderTypeDeclarationNode*>(prev_hdrtypedecl_2_nodes_[hdrinstance_node->type_->toString()]);
                        for(FieldDecNode* field: *(hdrtypedecl_node->field_decs_->list_)) {
                            oss << "  modify_field(" << extseq_[i][j] << "_clone." << field->name_->toString() << ", " << extseq_[i][j] << "." << field->name_->toString() << ");\n";
                        }
                    } else {
                        PANIC("%s NOT HeaderInstanceNode node!\n", extseq_[i][j].c_str());
                    }
                }    
            }
            
        }       
        oss << "}\n";
        unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string("ai_add_clone_"+CombineHeader(extseq_[i])), new string("ai_add_clone"+CombineHeader(extseq_[i]))));
        oss.str("");
    }
    PRINT_VERBOSE("Done Clone\n");
}

void DTModifier::ProcessCPU(AstNode* root) {
    ostringstream oss;
  // ti_set_port
    oss << "table " << kTiSetPort << " {\n"
        << "  reads { \n"
        << "    " << kGlobalMetadata << ".temp_port : exact;\n";
    oss << "  }\n"
        << "  "
        << "  actions {\n"
        << "    " << kAiSetPort << ";\n"
        << "  }\n"
        << "  default_action : " << kAiSetPort << "(0);\n"
        << "  size: 16;\n"
        << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kTiSetPort), new string(kTiSetPort)));
    oss.str("");
  // ai_set_port
    oss << "action " << kAiSetPort << "(outPort) {\n";
    if (strcmp(target_, "sim")==0) {
        oss << "  modify_field(standard_metadata.egress_spec, outPort);\n";
    } else if (strcmp(target_, "hw")==0) {
        oss << "  modify_field(ig_intr_md_for_tm.ucast_egress_port, outPort);\n";
    }
    oss << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kAiSetPort), new string(kAiSetPort)));
    oss.str("");
}

void DTModifier::CorrectPort(AstNode* root) {
    ostringstream oss;
    oss << "action " << kAiDrop << "() {\n"
    << "  drop();\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kAiDrop), new string(kAiDrop)));
    oss.str("");

    oss << "action " << kAiNoAction << "() {\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kAiNoAction), new string(kAiNoAction)));
    oss.str("");

}

void DTModifier::WriteJsonFile() {
    controlPlane["num_assertions"] = num_assertions_;
    for (auto iter=prev_hdrinstance_2_nodes_.begin(); iter!=prev_hdrinstance_2_nodes_.end(); ++iter) {
        HeaderInstanceNode* hdrinstance_node = dynamic_cast<HeaderInstanceNode*>(iter->second);
        json headerJson;
        if ((hdrinstance_node -> numHeaders_) == NULL) {
            headerJson[iter -> first] = "0";
        } else {
            headerJson[iter -> first] = hdrinstance_node -> numHeaders_ -> toString();

        }
        controlPlane["headers"].push_back(headerJson);
        
    }
    ofstream fieldModificationFile("out/" + program_name_ + "_coverageAndRules.json", ios::out | ios::trunc);
    fieldModificationFile << controlPlane.dump(4) << endl;
    fieldModificationFile.close();
}


void DTModifier::AddFieldToJson(string fieldName, int numBit) {
    json fieldJson;
    fieldJson[fieldName] = numBit;
    controlPlane["visited"].push_back(fieldJson);
}


void DTModifier::GetHdrTypeDecl2Nodes(AstNode* head) {
    if (head == NULL) {
        return;
    }
    if (work_set_.find(head) != work_set_.end()) {
        return;
    }
    work_set_.insert(head);
    if (TypeContains(head, "HeaderTypeDeclarationNode")) {
        string key = (dynamic_cast<HeaderTypeDeclarationNode*>(head)) -> name_->toString();
        PRINT_VERBOSE("Found HeaderTypeDeclarationNode: %s\n", key.c_str());
        prev_hdrtypedecl_2_nodes_[key] = head;
        num_hdrtypedeclr_ += 1;
    }
    if (TypeContains(head, "InputNode")) {
        if (dynamic_cast<InputNode*>(head) -> next_) {
            GetHdrTypeDecl2Nodes(dynamic_cast<InputNode*>(head) -> next_);
        }
        GetHdrTypeDecl2Nodes(dynamic_cast<InputNode*>(head) -> expression_);
    } 
}

void DTModifier::GetName2TableNodes(AstNode* head) {
    if (head == NULL) {
        return;
    }
    if (work_set_.find(head) != work_set_.end()) {
        return;
    }
    work_set_.insert(head);
    if (TypeContains(head, "TableNode")) {
        string key = (dynamic_cast<TableNode*>(head)) -> name_->toString();
        PRINT_VERBOSE("Found TableNode: %s\n", key.c_str());
        prev_name_2_tablenodes_[key] = head;
        num_tbl_nodes_ += 1;
    }
    if (TypeContains(head, "InputNode")) {
        if (dynamic_cast<InputNode*>(head) -> next_) {
            GetName2TableNodes(dynamic_cast<InputNode*>(head) -> next_);
        }
        GetName2TableNodes(dynamic_cast<InputNode*>(head) -> expression_);
    } 
}

void DTModifier::GetMetadataInstance2Nodes(AstNode* head) {
    if (head == NULL) {
        return;
    }
    if (work_set_.find(head) != work_set_.end()) {
        return;
    }
    work_set_.insert(head);
    if (TypeContains(head, "MetadataInstanceNode")) {
        string key = (dynamic_cast<MetadataInstanceNode*>(head)) -> name_->toString();
        PRINT_VERBOSE("Found MetadataInstanceNode: %s\n", key.c_str());
        prev_metadatainstance_2_nodes_[key] = head;
        num_metadata_instance_ += 1;
    }
    if (TypeContains(head, "InputNode")) {
        if (dynamic_cast<InputNode*>(head) -> next_) {
            GetMetadataInstance2Nodes(dynamic_cast<InputNode*>(head) -> next_);
        }
        GetMetadataInstance2Nodes(dynamic_cast<InputNode*>(head) -> expression_);
    } 
}

void DTModifier::GetHdrInstanceCombinations(AstNode* head) {
    vector<std::string> hdrinstance_name_vec;
    for (map<string, AstNode*>::iterator iter=prev_hdrinstance_2_nodes_.begin(); iter!=prev_hdrinstance_2_nodes_.end(); ++iter) {
        PRINT_VERBOSE("Header instance: %s\n", (*iter).first.c_str());
        std::string hdrinstance_name = (*iter).first;
        hdrinstance_name_vec.push_back(hdrinstance_name);
    }
    PRINT_VERBOSE("hdrinstance_name_vec length %d\n", hdrinstance_name_vec.size());
  // Need to sort first as the algorithm determines whether it is finished based on whether the elements are sorted or not
  // std::sort (hdrinstance_name_vec.begin(), hdrinstance_name_vec.begin()+hdrinstance_name_vec.size());
    for (int i = 1; i < pow(2, hdrinstance_name_vec.size()); i++) {
        int t = i;
        vector<string> tmp_vec {};
        for (int j = 0; j < hdrinstance_name_vec.size(); j++) {
            if (t & 1)
                tmp_vec.push_back(hdrinstance_name_vec[j]);
            t >>= 1;
        }
        prev_hdrinstance_combinations_vec_.push_back(tmp_vec);
        std::string combination = "";
        for (int k = 0; k<tmp_vec.size(); k++) {
            if (k==0) { combination += tmp_vec[k];}
            else {
                combination += ("_"+tmp_vec[k]);
            }
        }
        prev_hdrinstance_combinations_.push_back(combination);
    }
    PRINT_VERBOSE("prev_hdrinstance_combinations_ length %d\n", prev_hdrinstance_combinations_.size());
    for (int i =0; i<prev_hdrinstance_combinations_.size(); i++) {
        PRINT_VERBOSE("Combination %d: %s\n", i, prev_hdrinstance_combinations_[i].c_str());
    }
}

void DTModifier::AddVisitedType(AstNode* root) {
    // Add ai_set_visited_type to set type = 1 to DT indicating that the current packet has traversed UT
    ostringstream oss;
    oss << "action ai_set_visited_type() {\n"
    << "  modify_field(" << kVisitedMetadata << ".type, 1);\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string("ai_set_visited_type"), new string("ai_set_visited_type")));
    oss.str("");
    
    // Add table
    oss << "table ti_set_visited_type {\n"
    << "  actions { ai_set_visited_type; }\n"
    << "  default_action: ai_set_visited_type();\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string("ti_set_visited_type"), new string("ti_set_visited_type")));
    oss.str("");
    
    // Apply in control ingress
    AstNode* curr = root;
    bool foundIngress = false;
    P4ControlNode* controlNode = FindControlNode(root, "ingress");
    if (controlNode != NULL) {
        foundIngress = true;
        controlNode -> controlBlock_ -> suffixStr_ = "  apply(ti_set_visited_type);\n";
    }
    if (!foundIngress) {
        PANIC("control ingress not found\n");  
    }
}

void DTModifier::ClearIngressEgress(AstNode* root) {
    AstNode* curr = root;
    bool foundIngress = false;
    bool foundEgress = false;

    while (curr != NULL && TypeContains(curr, "InputNode")) {
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "P4ControlNode")) {
            P4ControlNode* controlNode = dynamic_cast<P4ControlNode*>(dynamic_cast<InputNode*>(curr)->expression_);
            string controlName = controlNode->name_->toString();
            if (controlName.find("ingress") != string::npos) {
                controlNode->controlBlock_-> clear(); 
                foundIngress = true;
            }
            if (controlName.find("egress") != string::npos) {
                controlNode->controlBlock_->clear(); 
                foundEgress = true;
            }
        } 
        curr = dynamic_cast<InputNode*>(curr) -> next_;
    }
    if (!foundIngress) PANIC("control ingress not found\n");  
    if (!foundEgress) PRINT_INFO("control egress not found\n");  
}

void DTModifier::AppendIngressProcessUT(AstNode* root) {
    AstNode* curr = root;
    bool foundIngress = false;
    // if (strcmp(target_, "hw")==0) {
    //     PANIC("TODO(liangcheng): support hw (bloom filter registers) for appendIngressProcessUT\n");
    // } else if (strcmp(target_, "sim")==0) {
    P4ControlNode* controlNode = FindControlNode(root, "ingress");
    if (controlNode != NULL) {
        foundIngress = true;
        ostringstream oss;
        oss << "  if (" << kVisitedMetadata << ".pkt_type == 1 || " << kVisitedMetadata << ".pkt_type == 3) {\n"
        << "    apply(" << kTiGetRegPosSim << ");\n"
        << "    apply(" << kTiReadBloomFilter1Sim << ");\n"
        << "    apply(" << kTiReadBloomFilter2Sim << ");\n"
        << "    apply(" << kTiPathAssertionSim << ");\n"
        << "  }\n";
        controlNode -> controlBlock_ -> append(std::string(oss.str()));
        oss.str("");
    }
    if (!foundIngress) PANIC("control ingress not found\n");  
}

void DTModifier::AppendIngressProcessSeed(AstNode* root) {
    AstNode* curr = root;
    bool foundIngress = false;
    P4ControlNode* controlNode = FindControlNode(root, "ingress");
    if (controlNode != NULL) {
        foundIngress = true;
        ostringstream oss;
        oss << "  if (" << kVisitedMetadata << ".pkt_type == 0) {\n"
            << "    apply(" << kTiGetSeed << ");\n"
            << "    apply(" << kTiTurnMutation << ");\n"
            << "    apply(" << kTiSetSeed << ");\n"
            << "    apply(" << kTiSetFixed << ");\n"
            << "    apply(" << kTiSetResubmit << ");\n"
            << "    apply(" << kTiCreatePkt << ");\n"
            << "    apply(" << kTiAddFields << ");\n"
            << "  }\n";
        controlNode -> controlBlock_ -> append(std::string(oss.str()));
        oss.str("");
    }
    // while (curr != NULL && TypeContains(curr, "InputNode")) {
    //     if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "P4ControlNode")) {
    //         P4ControlNode* controlNode = dynamic_cast<P4ControlNode*>(dynamic_cast<InputNode*>(curr)->expression_);
    //         string controlName = controlNode->name_->toString();
    //         if (controlName.find("ingress") != string::npos) {
    //             ostringstream oss;
    //             oss << "  if (" << kVisitedMetadata << ".type == 0) {\n"
    //             << "    apply(" << kTiGetSeed << ");\n"
    //             << "    apply(" << kTiCreatePkt << ");\n"
    //             << "    apply(" << kTiAddFields << ");\n"
    //             << "  }\n";
    //             controlNode->append(std::string(oss.str()));
    //             oss.str("");
    //             foundIngress = true;
    //         }
    //     } 
    //     curr = dynamic_cast<InputNode*>(curr) -> next_;
    // }
    if (!foundIngress) PANIC("control ingress not found\n");
    PRINT_VERBOSE("Done AppendIngressProcessSeed\n");
}

void DTModifier::AppendIngressProcessCPU(AstNode* root) {
    AstNode* curr = root;
    bool foundIngress = false;
    P4ControlNode* controlNode = FindControlNode(root, "ingress");
    if (controlNode != NULL) {
        foundIngress = true;
        ostringstream oss;
        oss << "  apply(" << kTiSetPort << ");\n";
        controlNode -> controlBlock_ -> append(std::string(oss.str()));
        oss.str("");
    }
    if (!foundIngress) PANIC("control ingress not found\n");  
}

void DTModifier::Mutate(AstNode* root) {
    // te_get_table_seed
    // Used to select random table to modify based on seed packet
    // cout << "Mutate 0" << endl;
    ostringstream oss;
    oss << "table " << kTeGetTblSeed << " {\n"
    << "  actions {\n"
    << "    " << kAeGetTblSeed << ";\n"
    << "  }\n"
    << "  default_action : " << kAeGetTblSeed << "();\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kTeGetTblSeed), new string(kTeGetTblSeed)));
    oss.str("");
  // ai_get_table_seed
    oss << "action " << kAeGetTblSeed << "() {\n"
        << "  modify_field_rng_uniform(" << kGlobalMetadata << ".table_seed, 0, 15);\n"
        << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kAeGetTblSeed), new string(kAeGetTblSeed)));
    oss.str("");

    // cout << "Mutate 10" << endl;
    controlPlane["action_params"][kAeSetTblSeed].push_back("real_table_seed");
    oss << "action " << kAeSetTblSeed << "(real_table_seed) {\n"
    << "  modify_field(" << kGlobalMetadata << ".table_seed, real_table_seed);\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kAeSetTblSeed), new string(kAeSetTblSeed)));
    oss.str("");

    // oss << "action " << kAeSetTblSeed << "(real_table_seed) {\n"
    // << "  bit_and(" << kGlobalMetadata << ".table_seed, " << kGlobalMetadata <<  ".table_seed, real_table_seed);\n"
    // << "}\n";
    // unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kAeSetTblSeed), new string(kAeSetTblSeed)));
    // oss.str("");
    controlPlane["table_keys"][kTeSetTblSeed].push_back(kGlobalMetadata + "_seed_num");
    controlPlane["table_keys"][kTeSetTblSeed].push_back(kGlobalMetadata + "_table_seed");
    // te_set_table_seed
    oss << "table " << kTeSetTblSeed << " {\n"
    << "  reads {\n"
    << "    " << kGlobalMetadata << ".seed_num : exact;\n"
    << "    " << kGlobalMetadata << ".table_seed : ternary;\n"
    << "  }\n"
    << "  actions {\n"
    << "    " << kAeSetTblSeed << ";\n"
    << "    " << kAiNoAction << ";\n"
    << "  }\n"
    << "  default_action : " << kAiNoAction << "();\n"
    << "  size: 256;\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kTeSetTblSeed), new string(kTeSetTblSeed)));
    oss.str("");
    // cout << "Mutate 20" << endl;
    controlPlane["table_keys"][kTeMoveFields].push_back(kGlobalMetadata + "_seed_num");
    controlPlane["table_keys"][kTeMoveFields].push_back(kGlobalMetadata + "_table_seed");

  // te_move_fields
  // Move selected table keys (header fields) to separate metadata  
    oss << "table " << kTeMoveFields << " {\n"
    << "  reads {\n"
    << "    " << kGlobalMetadata << ".seed_num: exact;\n"
    << "    " << kGlobalMetadata << ".table_seed: exact;\n"
    << "  }\n"
    << "  actions {\n"
    << "    " << kAiNoAction << ";\n";
    // Add one action per table
    for (auto iter=tbl2readfields_.begin(); iter!=tbl2readfields_.end(); iter++) {
        oss << "    ae_move_" << iter->first << ";\n";
    }
    oss << "  }\n"
    << "  default_action : " << kAiNoAction << "();\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kTeMoveFields), new string(kTeMoveFields)));
    oss.str("");
    // cout << "Mutate 30 " << endl;
    // ae_move_*
    for (auto iter=tbl2readfields_.begin(); iter!=tbl2readfields_.end(); iter++) {
        oss << "action ae_move_" << iter->first << "(){\n";
        int reads_ind = 0;
        // Move every reads fields
        for (string read_field: iter->second) {
            oss << "  modify_field(" << kGlobalMetadata << ".max_bits_field" << reads_ind << ", " << read_field << ");\n";
            reads_ind += 1;
        }
        oss << "}\n";
    }    

    // cout << "Mutate 40" << endl;
  // te_apply_mutations
  // Apply mutation to metadata  
    // cout << "value:" << max_tblreads_field_num_ << endl;
    // for (int x : max_tblreads_field_width_) {
    //     cout << x << endl;
    // }
    // cout << "total nums: " << max_tblreads_field_num_ << endl;
    // cout << "Mutate 41" << endl;
    for (int i = 0; i < max_tblreads_field_num_; i++) {
        // cout << "Mutate 42" << endl;
        oss << "table " << kTeApplyMutations << i << " {\n"
        << "  actions {\n"
        << "    " << kAeApplyMutations << i << ";\n"
        << "  }\n"
        << "  default_action : " << kAeApplyMutations << i << "();\n"
        << "}\n";  
        unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kTeApplyMutations), new string(kTeApplyMutations))); 
        oss.str("");
        // cout << "Mutate 43" << endl;
      // ae_apply_mutations
        oss << "action " << kAeApplyMutations << i << "() {\n";
        oss << "  modify_field_rng_uniform(" << kGlobalMetadata << ".max_bits_field" << i << ", 0," << int_to_hex(max_tblreads_field_width_[i])  << ");\n";
        oss << "}\n";
        unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kAeApplyMutations + to_string(i)), new string(kAeApplyMutations + to_string(i))));
        oss.str("");
        // cout << "Mutate 44" << endl;
    }
    // cout << "Mutate 50" << endl;
    controlPlane["table_keys"][kTeMoveBackFields].push_back(kGlobalMetadata + "_seed_num");
    controlPlane["table_keys"][kTeMoveBackFields].push_back(kGlobalMetadata + "_table_seed");
    controlPlane["table_keys"][kTeMoveBackFields].push_back(kGlobalMetadata + "_fixed_header_seed");

    // te_move_back_fields
    // Either move the mutated metadata to header or set fixed values to header
    oss << "table " << kTeMoveBackFields << " {\n"
    << "  reads {\n"
    << "    " << kGlobalMetadata << ".seed_num: exact;\n"
    << "    " << kGlobalMetadata << ".table_seed: exact;\n"
    << "    " << kGlobalMetadata << ".fixed_header_seed: ternary;\n"
    << "  }\n"
    << "  actions {\n"
    << "    " << kAiNoAction << ";\n"
    << "    " << kAiDrop << ";\n";
    // Two actions per table
    for (auto iter=tbl2readfields_.begin(); iter!=tbl2readfields_.end(); iter++) {
        oss << "    ae_move_back_" << iter->first << ";\n";
        oss << "    ae_move_back_fix_" << iter->first << ";\n";
    }
    oss << "  }\n"
    << "  default_action : " << kAiNoAction << "();\n"
    << "  size: 2048;\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kTeMoveFields), new string(kTeMoveFields)));
    oss.str("");
    // cout << "Mutate 60" << endl;
    // ai_move_back_*
    for (auto iter=tbl2readfields_.begin(); iter!=tbl2readfields_.end(); iter++) {
        oss << "action ae_move_back_" << iter->first << "(){\n";
        int reads_ind = 0;
        // Moveback every reads fields
        for (string read_field: iter->second) {
            oss << "  modify_field(" << read_field << ", " << kGlobalMetadata << ".max_bits_field" << reads_ind << ");\n";
            vector<string> headerField = split(read_field, '.');
            oss << "  modify_field(" << headerField[0] << "_clone." << headerField[1] << ", " << kGlobalMetadata << ".max_bits_field" << reads_ind << ");\n";
            reads_ind += 1;
        }
        oss << "}\n";
    }
    // cout << "Mutate 70" << endl;
    // ai_move_back_fix_*
    for (auto iter=tbl2readfields_.begin(); iter!=tbl2readfields_.end(); iter++) {
        // cout << "table name: " << iter->first << endl;
        oss << "action ae_move_back_fix_" << iter->first << "(";
        std::regex pat {"\\."};
        // cout << "Mutate 71" << endl;
        for (int i = 0; i<iter->second.size(); i++) {
            string read_field_arg = regex_replace(iter->second[i], pat, "_");
            if (i==0) {
                oss << read_field_arg;
            } else {
                oss << ", " << read_field_arg;
            }
            // cout << "Mutate 73" << endl;
            controlPlane["action_params"]["ae_move_back_fix_" + iter->first].push_back(read_field_arg);
        }
        // cout << "Mutate 74" << endl;
        oss << "){\n";
        // cout << "Mutate 75" << endl;
        // Moveback every reads fields
        for (string read_field: iter->second) {
            // cout << "Mutate 76" << endl;
            string read_field_arg = regex_replace(read_field, pat, "_");
            // cout << "Mutate 76.1" << endl;
            oss << "  modify_field(" << read_field << ", " << read_field_arg << ");\n";
            // cout << "Mutate 76.2: " << read_field << endl;
            vector<string> headerField = split(read_field, '.');
            // cout << "Mutate 76.3" << endl;
            oss << "  modify_field(" << headerField[0] << "_clone." << headerField[1] << ", " << read_field_arg << ");\n";
            // cout << "Mutate 76.4" << endl;
        }
        // cout << "Mutate 76.9" << endl;
        oss << "}\n";
        // cout << "Mutate 77" << endl;
    }
    // cout << "Mutate 78" << endl;
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string("ai_move_back*"), new string("ai_move_back*")));
    // cout << "Mutate 79" << endl;
    oss.str("");
    // cout << "Mutate 80" << endl;

    // fi_resub_fields
    oss << "field_list " << kFeResubmit << " {\n"
    << "  " << kGlobalMetadata << ".apply_mutations;\n"
    << "  " << kGlobalMetadata << ".seed_num;\n";
    oss << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kFeResubmit), new string(kFeResubmit)));
    oss.str("");

    controlPlane["table_keys"][kTeDoResubmit].push_back(kGlobalMetadata + "_make_clone");
  // te_do_resubmit
  // Probabilistically resubmit packets for additional mutations
    oss << "table " << kTeDoResubmit << " {\n"
    << "  reads {\n"
    << "    " << kGlobalMetadata << ".make_clone: exact;\n"
    << "  }\n"
    << "  actions {\n"
    << "    " << kAeDoResubmit << ";\n"
    << "    " << kAiNoAction << ";\n"
    << "  }\n"
    << "  default_action : " << kAiNoAction << "();\n"
    << "  size: 256;\n"
    << "}\n";  
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kTeDoResubmit), new string(kTeDoResubmit))); 
    oss.str("");
  // ai_resubmit
    oss << "action " << kAeDoResubmit << "() {\n"
        << "  modify_field(" << kVisitedMetadata << ".pkt_type, 3);\n"
    // << "  resubmit(" << kFeResubmit << ");\n"
        << "  clone_egress_pkt_to_egress(99, " << kFeResubmit << ");\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kAeDoResubmit), new string(kAeDoResubmit)));
    oss.str("");
    // cout << "Mutate 90" << endl;
  // ai_send_out
    // oss << "action " << kAeSendOut << "() {\n";
  // TODO(liangcheng) Configurable params?
    // if (strcmp(target_, "sim")==0) {
    //     oss << "  modify_field_rng_uniform(standard_metadata.egress_spec, 2, 3);\n";
    // } else if(strcmp(target_, "hw")==0) {
    //     oss << "  modify_field_rng_uniform(ig_intr_md_for_tm.ucast_egress_port, 2, 3);\n";
    // } else {
    //     PANIC("Unknown target!\n");
    // }
    // oss << "}\n";
    // unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kAeDoResubmit), new string(kAeDoResubmit)));
    // oss.str("");  

}

void DTModifier::AppendIngressAddClones(AstNode* root) {
    AstNode* curr = root;
    bool foundIngress = false;
    P4ControlNode* controlNode = FindControlNode(root, "ingress");
    if (controlNode != NULL) {
        foundIngress = true;
        ostringstream oss;
        oss << "  apply(" << kTiAddClones << ");\n";
        controlNode -> controlBlock_ -> append(std::string(oss.str()));
        oss.str("");
    }
    if (!foundIngress) PANIC("control ingress not found\n");  
}

void DTModifier::AppendEgressCount(AstNode* root) {
    ostringstream oss;
    oss << "action ai_set_visited_type() {\n"
        << "    modify_field(" << sig_ << "_visited.pkt_type, 1);\n"
	<< "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string("ai_set_visited_type"), new string("ai_set_visited_type")));
    oss.str("");
    
    // Add table
    oss << "table ti_set_visited_type {\n"
        << "    actions { ai_set_visited_type; }\n"
        << "    default_action: ai_set_visited_type();\n"
        << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string("ti_set_visited_type"), new string("ti_set_visited_type")));
    oss.str("");    

    AstNode* curr = root;
    bool foundEgress = false;
    P4ControlNode* controlNode = FindControlNode(root, "egress");
    if (controlNode != NULL) {
        foundEgress = true;
        ostringstream oss;
        oss << "  if (" << kGlobalMetadata << ".apply_mutations == 1) {\n"
            << "    apply(" << kTeGetTblSeed << ");\n"
            << "    apply(" << kTeSetTblSeed << ");\n"
            << "    apply(" << kTeMoveFields << ");\n";
        for (int i = 0; i < max_tblreads_field_num_; i++) {
            oss << "    apply(" << kTeApplyMutations << i << ");\n";
        }
        oss << "    apply(" << kTeMoveBackFields << ");\n"
            << "    apply(" << kTeDoResubmit << ");\n"
            << "  }\n";
        oss << "    apply(ti_set_visited_type);\n";
	oss << "    apply(" << kTeUpdateCount << ");\n";
        controlNode -> controlBlock_ -> append(std::string(oss.str()));
        oss.str("");
    }

    if (!foundEgress) {
        ostringstream oss;
        oss << "control egress {\n"
            << "  if (" << kGlobalMetadata << ".apply_mutations == 1) {\n"
            << "    apply(" << kTeGetTblSeed << ");\n"
            << "    apply(" << kTeSetTblSeed << ");\n"
            << "    apply(" << kTeMoveFields << ");\n";
        for (int i = 0; i < max_tblreads_field_num_; i++) {
            oss << "    apply(" << kTeApplyMutations << i << ");\n";
        }
        oss << "    apply(" << kTeMoveBackFields << ");\n"
            << "    apply(" << kTeDoResubmit << ");\n"
            << "  }\n";
        oss << "    apply(ti_set_visited_type);\n";
        oss << "    apply(" << kTeUpdateCount << ");\n"
            << "}\n";
        unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string("egress"), new string("egress")));
    }  
}

void DTModifier::RedirectDroppedPacket(AstNode* root) {
    // Per Barefoot Tofino Compiler User Guide, drop() only marks packets to be dropped,
    // only when exit() is applied as well, we force an immediate drop after the action completes.
    // Hence, we only need to replace drop() call with reserved_port set.
    AstNode* curr = root;
    while (curr != NULL && TypeContains(curr, "InputNode")) {
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "ActionNode")) {
            ActionNode* actionNode = dynamic_cast<ActionNode*>(dynamic_cast<InputNode*>(curr)->expression_);
            string actionName = actionNode->name_->toString();
            ActionStmtsNode* stmts = actionNode->stmts_;
            for(ActionStmtNode* stmt : *(stmts->list_)) {
                string stmtName = stmt->name1_->toString();
                if (stmtName.compare("drop")==0) {
                    PRINT_VERBOSE("@action %s, stmt func %s: redirect the packet to drop to RESERVED_PORT\n", actionName.c_str(), stmtName.c_str());
                    stmt->removed_ = true;
                // modify_field(ig_intr_md_for_tm.ucast_egress_port, RESERVED_PORT); 
                    NameNode* funcName = new NameNode(new string("modify_field"));
                    ArgsNode* argList = new ArgsNode();
                    BodyWordNode* updateField = new BodyWordNode(BodyWordNode::FIELD, 
                        new FieldNode(
                            new NameNode(new string("ig_intr_md_for_tm")), 
                            new NameNode(new string("ucast_egress_port")),
                            NULL));
                    BodyWordNode* portNum = new BodyWordNode(BodyWordNode::INTEGER, new IntegerNode(new string(std::to_string(RESERVED_PORT))));

                    argList -> push_back(updateField);
                    argList -> push_back(portNum);

                    ActionStmtNode* redirectStmt = new ActionStmtNode(funcName, argList, ActionStmtNode::NAME_ARGLIST, NULL, NULL);
                    stmts->push_back(redirectStmt);
                }
            } 
        } 
        curr = dynamic_cast<InputNode*>(curr) -> next_;
    }
}

// bool DTModifier::TypeContains(AstNode* n, const char* type) {
//     if (n->nodeType_.find(string(type)) != string::npos) {
//         return true;    
//     } else {
//         return false;
//     }
// }
