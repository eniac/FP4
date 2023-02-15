#include "../../include/ast_nodes_p4.hpp"
#include "../../include/modifier_ut.h"
#include "../../include/helper.hpp"
#include <regex>

using namespace std;

UTModifier::UTModifier(AstNode* root, char* target, char* plan, const char* rules_in, const char* rules_out, int num_assertions, char* out_fn_base) : P4Modifier(root, target, plan, num_assertions, out_fn_base) {
	PRINT_INFO("============== UT ==================\n");
	rules_in_ = rules_in;
    rules_out_ = rules_out;

    GetAction2Tbls(root);
    PRINT_VERBOSE("===prev_action_2_tbls_===\n");
    
    for (map<string, vector<string> >::iterator ii=prev_action_2_tbls_.begin(); ii!=prev_action_2_tbls_.end(); ++ii) {
        PRINT_VERBOSE("%s : ", (*ii).first.c_str());
        vector <string> inVect = (*ii).second;
        for (unsigned j=0; j<inVect.size(); j++){
            PRINT_VERBOSE("%s ", inVect[j].c_str());
        }
        PRINT_VERBOSE("\n");
    }
    PRINT_VERBOSE("=================\n");
    work_set_.clear();
   
    findStatefulActions(root);
    createStatefulTables(root);
    GetAction2Nodes(root);

    PRINT_INFO("Number of TblActionStmt: %d, number of ActionNode: %d\n", num_tbl_action_stmt_, num_action_); 

    work_set_.clear();

    // GetHdrInstance2Nodes(root);

    // work_set_.clear();

    distinguishPopularActions(root);
    distinguishPopularActionsForControl(root);
    distinguishDefaultPopularActions(root);
    // exit(0);

    work_set_.clear();
    RemoveNodes(root);

	AssignAction2Bitmap();
	// addStatefulTblActions(root);  // Mark stateful actions inline
	addModifyTables(root);
	AddVisitedHdr();

	ExtractVisitedHdr(root);

	work_set_.clear();

	markActionVisited(root);
	addVisitedType(root);
	redirectDroppedPacket(root);
	PRINT_VERBOSE("Done redirectDroppedPacket\n");
	// correctPort(root);

    // createAssertTables(root);

	if (rules_in_ != NULL && rules_out_ != NULL) {
    	instrumentRules();
    }
    PRINT_VERBOSE("Done instrument rules.txt\n");

    cout << "[Done UTModifier]\n";
}


void UTModifier::addStatefulTblActions(AstNode* root) {
    ostringstream oss;
    for (auto const& tableAction : tbl_2_state_action_) {
        oss << "action astate_" << tableAction.first << "() {\n"
            << "    modify_field(" << sig_ << "_visited." << tableAction.second
            << ", 1);\n"
            << "}\n";
        unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string("astate_" + tableAction.first), new string("astate_" + tableAction.first)));
        oss.str("");

        oss << "table tstate_" <<  tableAction.first << " {\n"
            << "    actions {\n"
            << "        astate_" << tableAction.first << ";\n"
            << "    }\n"
            << "    default_action : astate_" << tableAction.first << "();\n"
            << "    size: 0;\n"
            << "}\n";
        unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string("tstate_" + tableAction.first), new string("tstate_" + tableAction.first)));
        oss.str("");
    }
}

void UTModifier::correctPort(AstNode* root) {
    ostringstream oss;
    oss << "action ai_port_correction(outPort) {\n";
    if (strcmp(target_, "sim")==0) {
        oss << "    modify_field(standard_metadata.egress_spec, outPort);\n";
    } else if (strcmp(target_, "hw")==0) {
        oss << "    modify_field(ig_intr_md_for_tm.ucast_egress_port, outPort);\n";
    } else {
        PANIC("Unknown target!\n");
        exit;
    }
    oss << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string("ai_port_correction"), new string("ai_port_correction")));
    oss.str("");

    oss << "action " << kAiNoAction << "() {\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kAiNoAction), new string(kAiNoAction)));
    oss.str("");

    oss << "table ti_port_correction {\n"
    << "    reads {\n";
    if (strcmp(target_, "sim")==0) {
        oss << "        standard_metadata.egress_spec: exact;\n";
    } else if (strcmp(target_, "hw")==0) {
        oss << "        ig_intr_md_for_tm.ucast_egress_port: exact;\n";
    } else { 
        PANIC("Unknown target!\n");
    } 
    oss << "  }\n"
    << "    actions {\n"
    << "        ai_port_correction;\n"
    << "        " << kAiNoAction << ";\n"
    << "    }\n"
    << "    default_action : " << kAiNoAction << "();\n"
    << "    size: 1;\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string(kTiPortCorrection), new string(kTiPortCorrection)));
    oss.str("");
}

void UTModifier::findStatefulActions(AstNode* root) {
    AstNode* curr = root;
    while (curr != NULL && TypeContains(curr, "InputNode")) {
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "ActionNode")) {
            ActionNode* actionNode = dynamic_cast<ActionNode*>(dynamic_cast<InputNode*>(curr)->expression_);
            if (actionNode -> stmts_ -> list_ -> size() == 1) {
                ActionStmtNode* actStatement = dynamic_cast<ActionStmtNode*>(actionNode -> stmts_ -> list_ -> at(0));
                if (actStatement -> type_ == ActionStmtNode::PROG_EXEC) {
                    state_action_2_tbl_[actionNode -> name_ -> toString()] = "";
                }
            }
        }
        curr = dynamic_cast<InputNode*>(curr) -> next_;
    }
    PRINT_VERBOSE("Done\n");
}

void UTModifier::createStatefulTables(AstNode* root) {
    AstNode* curr = root;
    while (curr != NULL && TypeContains(curr, "InputNode")) {
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "TableNode")) {
            TableNode* tblNode = dynamic_cast<TableNode*>(dynamic_cast<InputNode*>(curr)->expression_);
            if (tblNode->actions_ != NULL && tblNode -> actions_ -> list_ -> size() == 1) {
                TableActionStmtNode* tblActStatement = dynamic_cast<TableActionStmtNode*>(tblNode -> actions_  -> list_ -> at(0));
                if (state_action_2_tbl_.find(tblActStatement -> name_ -> toString()) != state_action_2_tbl_.end()) {
                    state_action_2_tbl_[tblActStatement -> name_ -> toString()] = tblNode -> name_ -> toString();
                    tbl_2_state_action_[tblNode -> name_ -> toString()] = tblActStatement -> name_ -> toString();
                } 
            }
        }
        curr = dynamic_cast<InputNode*>(curr) -> next_;
    } 
    PRINT_VERBOSE("Done\n");
}

void UTModifier::addModifyTables(AstNode* root) {
    P4ControlNode* curr = FindControlNode(root, "ingress");
    addModifyTablesRecurse(root, curr -> controlBlock_);
    curr = FindControlNode(root, "egress");
    if (curr != NULL) {
        addModifyTablesRecurse(root, curr -> controlBlock_);
    }
}

ControlStatement* UTModifier::createTableCall(string tableName) {
    // String* tableNamePointer = new string(tableName);
    ApplyTableCall* tableCall = new ApplyTableCall(new NameNode(new string(tableName)));
    return new ControlStatement(tableCall, ControlStatement::TABLECALL);
}

void UTModifier::addModifyTablesRecurse(AstNode* root, P4ControlBlock* controlBlock) {
    int listSize = controlBlock -> controlStatements_ -> list_ -> size();
    ControlStatement* controlStmt;
    for (int i = 0; i < listSize; ++i) {
        controlStmt = controlBlock -> controlStatements_ -> list_ -> at(i);
        if (controlStmt -> stmtType_ == ControlStatement::TABLECALL) {
            // Avoid creating new tables to mark stateful actions
            // string tableName = dynamic_cast<ApplyTableCall*>(controlStmt -> stmt_) -> name_ -> toString();
            // if (tbl_2_state_action_.find(tableName) != tbl_2_state_action_.end()) {
            //     controlBlock -> controlStatements_  -> insert(i+1, createTableCall("tstate_" + tableName));
            //     listSize += 1;
            // }
        } else if (controlStmt -> stmtType_ == ControlStatement::CONTROL) {
            P4ControlNode* controlNode = FindControlNode(root, dynamic_cast<NameNode*>(controlStmt -> stmt_) -> toString());
            addModifyTablesRecurse(root, controlNode -> controlBlock_);
        } else if (controlStmt -> stmtType_ == ControlStatement::IFELSE) {
            IfElseStatement* ifStmt = dynamic_cast<IfElseStatement*>(controlStmt -> stmt_);
            addModifyTablesRecurse(root, ifStmt -> controlBlock_);
            if (ifStmt -> elseBlock_ != NULL) {
                if (ifStmt -> elseBlock_ -> controlBlock_ != NULL) {
                    addModifyTablesRecurse(root, ifStmt -> elseBlock_ -> controlBlock_);
                } else if (ifStmt -> elseBlock_ -> ifElseStatement_ != NULL) {
                    addModifyTablesRecurse(root, ifStmt -> elseBlock_ -> ifElseStatement_ -> controlBlock_);
                }
            }
        } else if (controlStmt -> stmtType_ == ControlStatement::TABLESELECT) {
            ApplyAndSelectBlock* tableStmt = dynamic_cast<ApplyAndSelectBlock*>(controlStmt -> stmt_); 
            if (tableStmt -> caseList_ != NULL) {
                if (tableStmt -> caseList_ -> actionCases_ != NULL) {
                    for (ActionCase* actionCase : *(tableStmt -> caseList_ -> actionCases_ -> list_)) {
                        addModifyTablesRecurse(root, actionCase -> controlBlock_);
                    }
                } else if (tableStmt -> caseList_ -> hitOrMissCases_ != NULL) {
                    for (HitOrMissCase* hitMissCase : *(tableStmt -> caseList_ -> hitOrMissCases_ -> list_)) {
                        addModifyTablesRecurse(root, hitMissCase -> controlBlock_);
                    }
                }
            } 
        } else if (controlStmt -> stmtType_ == ControlStatement::ASSERT) {
            // cout << "It's an assert statement" << endl;
            AssertNode* tableStmt = dynamic_cast<AssertNode*>(controlStmt -> stmt_); 
            addAssertTable(tableStmt);
        } else {
            cout << "Unknown control statement" << endl;
        }
    }
}

string UTModifier::addAssertExpression(ExprNode* expr, int assertNum) {
    string field;
    int value;
    if (dynamic_cast<FieldNode*>(dynamic_cast<ExprNode*>(expr -> first_) -> first_))  {
        field = expr -> first_ -> toString();
    } else {
        cout << "Assert statement is currently not supported 1: " << endl;
        cout << "Please write in form assert(header.field <operator> value)" << endl;
        cout << "Currently: " << expr -> toString() << endl;
        cout << "First: " << expr -> first_ -> toString() << endl;
        exit(0);
    }
    if (dynamic_cast<IntegerNode*>(dynamic_cast<ExprNode*>(expr -> second_) -> first_))  {
        value = stoi(expr -> second_ -> toString());
    } else {
        cout << "Assert statement is currently not supported 2" << endl;
        cout << "Please write in form assert(header.field <operator> value)" << endl;
        exit(0);
    }
    size_t pos = field.find(".");
    field = field.replace(pos, 1, "_");
    if (*(expr -> operator_) == "==") {
        assert_rules_ << "pd assert_table" << assertNum << " add_entry assert_action" << assertNum << " " << field << " " << value << "\n";
        return field + " : exact";
    } else if (*(expr -> operator_) == "<=")  {
        assert_rules_ << "pd assert_table" << assertNum << " add_entry assert_action" << assertNum << " " << field << "_start " << to_string(value + 1) << " " << field << "_end 65535 priority 2" << "\n";
        return field + " : range";
    } else if (*(expr -> operator_) == ">=")  {
        assert_rules_ << "pd assert_table" << assertNum << " add_entry assert_action" << assertNum << " " << field << "_start 0 " << field << "_end " << to_string(value - 1) << " priority 2" << "\n";
        return field + " : range";
    } else {
        cout << "Currently only supported operators in assert are ==, <= and >= " << endl;
        exit(0);
    }
    return "";
}

void UTModifier::addAssertTable(AssertNode* tableStmt) {
    ostringstream oss;
    oss << "action assert_action" << tableStmt -> assertNum_ << "() {\n"
    << "    modify_field(" << sig_ << "_visited.assertion" << tableStmt -> assertNum_  << ", 1);\n"
    << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string("assert_action" + to_string(tableStmt -> assertNum_ )), new string("assert_action" + to_string(tableStmt -> assertNum_ ))));
    oss.str("");
    
    // Add table
    oss << "table assert_table" << tableStmt -> assertNum_ << " {\n"
        << "    reads {\n";
    oss << "        " << addAssertExpression(tableStmt -> expr_, tableStmt -> assertNum_) << ";\n";
    // vector<string> fieldList;
    // fieldList = tableStmt -> expr_ -> GetFields(fieldList);
    // for (string field : fieldList) {
    //     oss << "        " << field << " : range;\n";
    // }
    oss << "    }\n"
        << "    actions { assert_action" << tableStmt -> assertNum_  << "; }\n"
        << "}\n";
    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string("assert_table" + to_string(tableStmt -> assertNum_ )), new string("assert_table" + to_string(tableStmt -> assertNum_ ))));
    oss.str("");
}

void UTModifier::distinguishDefaultPopularActions(AstNode* root) {
    PRINT_INFO("Enter distinguishDefaultPopularActions\n");
    AstNode* curr = root;
    while (curr != NULL && TypeContains(curr, "InputNode")) {
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "TableNode")) {
            TableNode* tblNode = dynamic_cast<TableNode*>(dynamic_cast<InputNode*>(curr)->expression_);
            PRINT_VERBOSE("------ %s ------\n", tblNode->name_->toString().c_str());
            for (int i = 0; i < tblNode->options_.size(); i++) {
                PRINT_VERBOSE("%s\n", tblNode->options_[i].c_str());
                if (tblNode->options_[i].find("default_action:") != std::string::npos) {
                    PRINT_VERBOSE("Identify default_action statement\n");
                    for (std::string action : popular_actions_) {
                        if (tblNode->options_[i].find("(") != std::string::npos) {
                            std::regex pat ("default_action:\\s*("+action+")\\(([\\d]*)\\)");
                            tblNode->options_[i] = std::regex_replace (tblNode->options_[i], pat, "default_action:$1_fp4_"+tblNode->name_->toString()+"($2)");   
                        } else {
                            std::regex pat ("default_action:\\s*("+action+")");
                            tblNode->options_[i] = std::regex_replace (tblNode->options_[i], pat, "default_action:$1_fp4_"+tblNode->name_->toString());      
                        }                  
                    }
                }
            }
        }
        curr = dynamic_cast<InputNode*>(curr) -> next_;
    }     
}

void UTModifier::instrumentRules() {
    PRINT_INFO("Target: %s, Rules input: %s, rules output: %s\n", target_, rules_in_, rules_out_);
    fstream ifs;
    ifs.open(rules_in_, ios_base::in);
    if (!ifs) PANIC("Couldn't open %s for R\n", rules_in_);
    ofstream ofs(rules_out_); // ‘‘o’’ for ‘‘output’’ implying ios::out
    if (!ofs) PANIC("Couldn't open %s for W\n", rules_out_);

    std::string temp;
    while(std::getline(ifs, temp)) {
        // Rename popular actions
        for (std::string action : popular_actions_) {
            if (strcmp(target_, "sim")==0) {
                std::regex pat1 ("table_set_default\\s*([a-z_\\d]*)\\s*("+action+")\\s*");
                temp = std::regex_replace (temp, pat1, "table_set_default $1 $2_fp4_$1");

                std::regex pat2 ("table_add\\s*([a-z_\\d]*)\\s*("+action+")\\s*");
                temp = std::regex_replace (temp, pat2, "table_add $1 $2_fp4_$1 ");            
            } else if (strcmp(target_, "hw")==0) {
                std::regex pat1 ("pd\\s*([a-zA-Z_\\d]*)\\s*set_default_action\\s*("+action+")\\s*");
                temp = std::regex_replace (temp, pat1, "pd $1 set_default_action $2_fp4_$1");

                std::regex pat2 ("pd\\s*([a-zA-Z_\\d]*)\\s*add_entry\\s*("+action+")\\s*");
                temp = std::regex_replace (temp, pat2, "pd $1 add_entry $2_fp4_$1 ");
            } else {
                PANIC("Unknown target!\n");
            } 
        }        
        ofs << temp << endl;
    }
    // if (strcmp(target_, "sim")==0) {
    //     ofs << "table_add ti_port_correction ai_port_correction 0 => 1" << endl;
    // } else {
    //     ofs << "pd ti_port_correction add_entry ai_port_correction ig_intr_md_for_tm_ucast_egress_port 0 action_outPort 0" << endl;
    // }
    ofs << assert_rules_.str();

    // for (int i = 0; i < 16; ++i)
    // {
    //     if (strcmp(target_, "sim")==0) {
    //         ofs << "table_add ti_port_correction ai_port_correction " << i << " => " << (i+1) << endl;
    //     } else {
    //         ofs << "pd ti_port_correction add_entry ai_port_correction ig_intr_md_for_tm_ucast_egress_port " << i << " action_outPort " << (i*4) << endl;
    //     }
    // }
}

void UTModifier::markActionVisited(AstNode* head) {
    if (head == NULL) {
        return;
    }
    if (work_set_.find(head) != work_set_.end()) {
        return;
    }
    work_set_.insert(head);
    if (TypeContains(head, "InputNode")) {
        if (dynamic_cast<InputNode*>(head) -> next_) {
            markActionVisited(dynamic_cast<InputNode*>(head) -> next_);
        }
        markActionVisited(dynamic_cast<InputNode*>(head) -> expression_);
    } else if (TypeContains(head, "ActionNode")) {
        markActionVisited(dynamic_cast<ActionNode*>(head) -> stmts_);
    } else if (TypeContains(head, "ActionStmtsNode")) {
        string key = (dynamic_cast<ActionNode*>(head->parent_)) -> name_->toString();

        if (prev_action_2_tbls_[key].size()>1) {
            PRINT_VERBOSE("Skip marking for removed action %s\n", key.c_str());
            return;  
        }
        
        // Don't skip stateful actions, mark them inline
        // if (state_action_2_tbl_.find(key) != state_action_2_tbl_.end()) {
        //     return;
        // }

        if (action_2_encoding_field_.find(key) == action_2_encoding_field_.end()) {
            cout << "ERR: encoding for action not found " << key << std::endl;
        }

        ActionStmtsNode* allStatements = dynamic_cast<ActionStmtsNode*>(head);
        NameNode* funcName = new NameNode(new string("add"));
        ArgsNode* argList = new ArgsNode();

        string* fieldName = new string("encoding"+action_2_encoding_field_[key]);
        BodyWordNode* updateField = new BodyWordNode(BodyWordNode::FIELD, 
            new FieldNode(
                new NameNode(new string(sig_+"_visited")), 
                new NameNode(fieldName),
                NULL)
            );

        string* fieldName1 = new string("encoding"+action_2_encoding_field_[key]);
        BodyWordNode* updateField1 = new BodyWordNode(BodyWordNode::FIELD, 
            new FieldNode(
                new NameNode(new string(sig_+"_visited")), 
                new NameNode(fieldName1),
                NULL)
            );

        string* numToAdd = new string(action_2_encoding_incr_[key]);
        BodyWordNode* bitToFlip = new BodyWordNode(BodyWordNode::INTEGER, new IntegerNode(numToAdd));

        argList -> push_back(updateField);
        argList -> push_back(updateField1);
        argList -> push_back(bitToFlip);

        ActionStmtNode* newStatement = new ActionStmtNode(funcName, argList, ActionStmtNode::NAME_ARGLIST, NULL, NULL);

        allStatements -> push_back(newStatement);
        PRINT_VERBOSE("%s, %s, %s\n", dynamic_cast<ActionNode*>(allStatements -> parent_) -> name_ -> toString().c_str(), (*fieldName).c_str(), (*numToAdd).c_str());

        // ActionStmtsNode* allStatements = dynamic_cast<ActionStmtsNode*>(head);
        // NameNode* funcName = new NameNode(new string("modify_field"));
        // ArgsNode* argList = new ArgsNode();
        // string* fieldName = new string(key);
        // BodyWordNode* updateField = new BodyWordNode(BodyWordNode::FIELD, 
        //     new FieldNode(
        //         new NameNode(new string(sig_+"_visited")), 
        //         new NameNode(fieldName),
        //         NULL)
        //     );

        // string* numToAdd = new string(to_string(1));
        // BodyWordNode* bitToFlip = new BodyWordNode(BodyWordNode::INTEGER, new IntegerNode(numToAdd));

        // argList -> push_back(updateField);
        // argList -> push_back(bitToFlip);

        // ActionStmtNode* newStatement = new ActionStmtNode(funcName, argList, ActionStmtNode::NAME_ARGLIST, NULL, NULL);

        // allStatements -> push_back(newStatement);
        // PRINT_VERBOSE("%s, %s, %s\n", dynamic_cast<ActionNode*>(allStatements -> parent_) -> name_ -> toString().c_str(), (*fieldName).c_str(), (*numToAdd).c_str());
    }
}

void UTModifier::distinguishPopularActionsForControl(AstNode* head) {
    AstNode* curr = head;
    while (curr != NULL && TypeContains(curr, "InputNode")) {
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "P4ControlNode")) {
            P4ControlNode* controlNode = dynamic_cast<P4ControlNode*>(dynamic_cast<InputNode*>(curr)->expression_);
            string controlName = controlNode->name_->toString();
            controlNode->RenamePopularActions(popular_actions_);
        } 
        curr = dynamic_cast<InputNode*>(curr) -> next_;
    }
}


void UTModifier::distinguishPopularActions(AstNode* head) {
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
            if (prev_action_2_tbls_[key].size() > 1) {
                if (std::find(popular_actions_.begin(), popular_actions_.end(), key) == popular_actions_.end()) {
                    popular_actions_.push_back(key);
                }
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
                InputNode* newInputNode = new InputNode(iterator -> next_, newAction);
                iterator -> next_ = newInputNode;
            } else {
                tbl_action_stmt_.push_back(key);
            }
        }
    }

    if (TypeContains(head, "InputNode")) {
        if (dynamic_cast<InputNode*>(head) -> next_) {
            distinguishPopularActions(dynamic_cast<InputNode*>(head) -> next_);
        }
        distinguishPopularActions(dynamic_cast<InputNode*>(head) -> expression_);
    } 
    else if (TypeContains(head, "TableNode")) {
        if (dynamic_cast<TableNode*>(head) -> name_) {
            distinguishPopularActions(dynamic_cast<TableNode*>(head) -> name_);
        }
        if (dynamic_cast<TableNode*>(head) -> reads_) {
            distinguishPopularActions(dynamic_cast<TableNode*>(head) -> reads_);
        }
        if (dynamic_cast<TableNode*>(head) -> actions_) {
            distinguishPopularActions(dynamic_cast<TableNode*>(head) -> actions_);
        }
    } 
    else if (TypeContains(head, "TableActionStmtsNode")) {
        TableActionStmtsNode* allActions = dynamic_cast<TableActionStmtsNode*>(head);
        if (allActions -> list_) {
            for (auto item: *(allActions->list_)) {
                distinguishPopularActions(item);
            }
        }
    } 
}

void UTModifier::addVisitedType(AstNode* root) {
    // Add ai_set_visited_type to set type = 1 to DT indicating that the current packet has traversed UT
//    ostringstream oss;
//    oss << "action ai_set_visited_type() {\n"
//    << "    modify_field(" << sig_ << "_visited.pkt_type, 1);\n"
//    << "}\n";
//    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string("ai_set_visited_type"), new string("ai_set_visited_type")));
//    oss.str("");
    
    // Add table
//    oss << "table ti_set_visited_type {\n"
//    << "    actions { ai_set_visited_type; }\n"
//    << "    default_action: ai_set_visited_type();\n"
//    << "}\n";
//    unanchored_nodes_.push_back(new UnanchoredNode(new string(oss.str()), new string("ti_set_visited_type"), new string("ti_set_visited_type")));
//    oss.str("");
    
    // AstNode* curr = root;
    // bool foundIngress = false;
    // P4ControlNode* controlNode = FindControlNode(root, "ingress");
    // if (controlNode != NULL) {
    //     foundIngress = true;
    //     controlNode -> controlBlock_ -> controlStatements_ -> push_back(createTableCall("ti_port_correction"));
    // }
//    controlNode = FindControlNode(root, "egress");
//    if (controlNode != NULL) {
//        controlNode -> controlBlock_ -> controlStatements_ -> push_back(createTableCall("ti_set_visited_type"));
//    }
//    if (!foundIngress) {
//        PANIC("control ingress not found\n");  
//    }
}

void UTModifier::redirectDroppedPacket(AstNode* root) {
    // Per Barefoot Tofino Compiler User Guide, drop() only marks packets to be dropped,
    // only when exit() is applied as well, we force an immediate drop after the action completes.
    // Hence, we only need to replace drop() call with reserved_port set.
    AstNode* curr = root;
    // cout << "In redirectDroppedPacket" << endl;
    while (curr != NULL && TypeContains(curr, "InputNode")) {
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "ActionNode")) {
            std::vector<ActionStmtNode*> tmpstmts {};
            ActionNode* actionNode = dynamic_cast<ActionNode*>(dynamic_cast<InputNode*>(curr)->expression_);
            string actionName = actionNode->name_->toString();
            ActionStmtsNode* stmts = actionNode->stmts_;
            PRINT_VERBOSE("--- @action %s ---\n", actionName.c_str());
            if (stmts == NULL) {
                PRINT_VERBOSE("@action %s, empty statements\n", actionName.c_str());
            }
            for(ActionStmtNode* stmt : *(stmts->list_)) {

                string stmtName = stmt->name1_->toString();
                PRINT_VERBOSE("stmt func %s\n", stmtName.c_str());
                if (stmtName.compare("drop")==0) {
                    PRINT_VERBOSE("redirect the packet to drop to RESERVED_PORT\n");
                    stmt->removed_ = true;
                    // modify_field(ig_intr_md_for_tm.ucast_egress_port, RESERVED_PORT); 
                    NameNode* funcName = new NameNode(new string("modify_field"));
                    ArgsNode* argList = new ArgsNode();
                    // By default, target HW (tofino)
                    BodyWordNode* updateField = new BodyWordNode(
                        BodyWordNode::FIELD, 
                        new FieldNode(
                        new NameNode(new string("ig_intr_md_for_tm")), 
                        new NameNode(new string("ucast_egress_port")),
                        NULL));
                    if (strcmp(target_, "sim") == 0) {
                        updateField = new BodyWordNode(
                            BodyWordNode::FIELD, 
                            new FieldNode(
                            new NameNode(new string("standard_metadata")), 
                            new NameNode(new string("egress_spec")),
                            NULL));
                    }
                BodyWordNode* portNum = new BodyWordNode(BodyWordNode::INTEGER, new IntegerNode(new string(std::to_string(RESERVED_PORT))));;

                argList -> push_back(updateField);
                argList -> push_back(portNum);

                ActionStmtNode* redirectStmt = new ActionStmtNode(funcName, argList, ActionStmtNode::NAME_ARGLIST, NULL, NULL);
                tmpstmts.push_back(redirectStmt);
                PRINT_VERBOSE("Done drop action\n");
            }
        }
        PRINT_VERBOSE("Copy new stmts\n");
        for (auto stmt : tmpstmts) {
            stmts->push_back(stmt);
        }   
    } 
    curr = dynamic_cast<InputNode*>(curr) -> next_;
    }
}
