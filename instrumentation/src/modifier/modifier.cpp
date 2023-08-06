#include "../../include/modifier.h"
#include "../../include/helper.hpp"
#include "../../util/util.cpp"
#include <regex>

using namespace std;

P4Modifier::P4Modifier(AstNode* root, char* target, char* ingress_plan, char* egress_plan, int num_assertions, char* out_fn_base) {
	PRINT_INFO("================= Modifier =================\n");
    target_ = target;
    root_ = root;
    num_assertions_ = num_assertions;
    program_name_ = string(out_fn_base);
    std::vector<string> fileName = split(program_name_, '/');
    program_name_ = fileName.back();

    std::ifstream fi(ingress_plan);
    ingress_plan_json_ = nlohmann::json::parse(fi);
    std::ifstream fe(egress_plan);
    egress_plan_json_ = nlohmann::json::parse(fe);
}


P4ControlNode* P4Modifier::FindControlNode(AstNode* root, string inputControlName) {
    AstNode* curr = root;
    while (curr != NULL && TypeContains(curr, "InputNode")) {
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "P4ControlNode")) {
            P4ControlNode* controlNode = dynamic_cast<P4ControlNode*>(dynamic_cast<InputNode*>(curr)->expression_);
                string controlName = controlNode -> name_ -> toString();
                controlName =  std::regex_replace(controlName, std::regex("^ +| +$|( ) +"), "$1");
                if (controlName == inputControlName) {
                    return controlNode;
                }
        }
        curr = dynamic_cast<InputNode*>(curr) -> next_;
    }
    return NULL;
}

void P4Modifier::GetAction2Tbls(AstNode* head) {
    if (head == NULL) {
        return;
    }
    if (work_set_.find(head) != work_set_.end()) {
        return;
    }
    work_set_.insert(head);
    if (TypeContains(head, "TableActionStmtNode")) {
        string key = (dynamic_cast<TableActionStmtNode*>(head)) -> name_->toString();
        string tblName = (dynamic_cast<TableNode*>(head -> parent_ -> parent_) -> name_) -> toString();
        PRINT_VERBOSE("Found TableActionStmt key: %s\n", key.c_str());
        PRINT_VERBOSE("Push action key to prev_action_2_tbls_: %s\n", key.c_str());
        prev_action_2_tbls_[key].push_back(tblName);
        num_tbl_action_stmt_ += 1;
    }

    if (TypeContains(head, "InputNode")) {
        if (dynamic_cast<InputNode*>(head) -> next_) {
            GetAction2Tbls(dynamic_cast<InputNode*>(head) -> next_);
        }
        GetAction2Tbls(dynamic_cast<InputNode*>(head) -> expression_);
    } 
    else if (TypeContains(head, "TableNode")) {
        if (dynamic_cast<TableNode*>(head) -> name_) {
            GetAction2Tbls(dynamic_cast<TableNode*>(head) -> name_);
        }
        if (dynamic_cast<TableNode*>(head) -> reads_) {
            GetAction2Tbls(dynamic_cast<TableNode*>(head) -> reads_);
        }
        if (dynamic_cast<TableNode*>(head) -> actions_) {
            GetAction2Tbls(dynamic_cast<TableNode*>(head) -> actions_);
        }
    } 
    else if (TypeContains(head, "TableActionStmtsNode")) {
        TableActionStmtsNode* allActions = dynamic_cast<TableActionStmtsNode*>(head);
        if (allActions -> list_) {
            for (auto item: *(allActions->list_)) {
                GetAction2Tbls(item);
            }
        }
    } 
}

void P4Modifier::GetAction2Nodes(AstNode* head) {
    if (head == NULL) {
        return;
    }
    if (work_set_.find(head) != work_set_.end()) {
        return;
    }
    work_set_.insert(head);
    if (TypeContains(head, "ActionNode")) {
        string key = (dynamic_cast<ActionNode*>(head)) -> name_->toString();
        PRINT_VERBOSE("Found ActionNode: %s\n", key.c_str());
        PRINT_VERBOSE("Push ActionNode to action2Node_: %s\n", key.c_str());
        prev_action_2_nodes_[key] = head;
        num_action_ += 1;
    }
    if (TypeContains(head, "InputNode")) {
        if (dynamic_cast<InputNode*>(head) -> next_) {
            GetAction2Nodes(dynamic_cast<InputNode*>(head) -> next_);
        }
        GetAction2Nodes(dynamic_cast<InputNode*>(head) -> expression_);
    } 
}

void P4Modifier::ExtractVisitedHdr(AstNode* root) {
    AstNode* curr = root;
  // Iterate InputNode
    while (curr != NULL && TypeContains(curr, "InputNode")) {
        if (TypeContains(dynamic_cast<InputNode*>(curr)->expression_, "ParserNode")) {
            P4ParserNode* parserNode = dynamic_cast<P4ParserNode*>(dynamic_cast<InputNode*>(curr)->expression_);
            string parserName = parserNode->name_->toString();
            if (parserName.find("start") != string::npos) {
                parserNode->prefixStr_ = "    extract("+sig_+"_visited);\n";
                PRINT_VERBOSE("Added extract(%s_visited);\n", sig_.c_str());
            } else {
                PRINT_VERBOSE("parser start not found\n");
            }
        } 
        curr = dynamic_cast<InputNode*>(curr) -> next_;
    }
}

void P4Modifier::AssignAction2Bitmap() {
    // TODO(liangcheng) Currently, # distinct action = # bits, i.e., 32b field can handle 32 action references.
    // May want to optimize given that actions references by the same table are mutually exclusive for a packet thread.
    sort(tbl_action_stmt_.begin(), tbl_action_stmt_.end());
    PRINT_VERBOSE("===tbl_action_stmt_ (sorted)===\n");
    for (int i = 0; i < tbl_action_stmt_.size(); i++) {
        action_2_bitmap_[tbl_action_stmt_[i]] = i;
        PRINT_VERBOSE("%s: %d\n", tbl_action_stmt_[i].c_str(), i);
    }
    PRINT_VERBOSE("=============================\n");
}


void P4Modifier::RemoveNodes(AstNode* head) {
    if (head == NULL) {
        return;
    }
    if (work_set_.find(head) != work_set_.end()) {
        return;
    }
    work_set_.insert(head);
    if (TypeContains(head, "ActionNode")) {
        string key = (dynamic_cast<ActionNode*>(head)) -> name_->toString();
        if (prev_action_2_tbls_[key].size()>1) {
            PRINT_VERBOSE("Remove ActionNode: %s\n", key.c_str());
            prev_action_2_nodes_[key]->removed_ = true;
        }
    }
    if (TypeContains(head, "InputNode")) {
        if (dynamic_cast<InputNode*>(head) -> next_) {
            RemoveNodes(dynamic_cast<InputNode*>(head) -> next_);
        }
        RemoveNodes(dynamic_cast<InputNode*>(head) -> expression_);
    } 
}

void P4Modifier::DistinguishPopularActions(AstNode* head) {
    return;
}

void P4Modifier::AddVisitedHdr() {
    InputNode* iterator = dynamic_cast<InputNode*>(root_);
    while (TypeContains(iterator -> expression_, "IncludeNode")) {
        iterator = iterator -> next_;
    }
    PRINT_VERBOSE("Locate point after IncludeNode\n");

    HeaderTypeDeclarationNode* newHeader = DeclVisitedHdr();
    InputNode* newInputNode = new InputNode(iterator -> next_, newHeader);

    newInputNode = new InputNode(newInputNode, 
        new HeaderInstanceNode(
            new NameNode(new string(sig_+"_visited_t")), 
            new NameNode(new string(sig_+"_visited")),
            NULL)
        );

    iterator -> next_ = newInputNode;
}

void P4Modifier::AddFieldToJson(string fieldName, int numBit) {
    return;
}

HeaderTypeDeclarationNode* P4Modifier::DeclVisitedHdr() {
    EmptyNode* other_stmts_ = new EmptyNode();
    FieldDecsNode* field_list = new FieldDecsNode();

    NameNode* name_;
    IntegerNode* size_;
    FieldDecNode* currentField;
    string fieldName;

    // Add preamble
    name_ = new NameNode(new string("preamble"));
    size_ = new IntegerNode(new string(to_string(48)));
    currentField = new FieldDecNode(name_, size_, NULL);
    field_list -> push_back(currentField);

    // std::vector<std::string> added {};
    // for (auto it = action_2_encoding_field_.begin(); it != action_2_encoding_field_.end(); it++) {
    //     if (std::find(added.begin(), added.end(), it->second) != added.end()) {
    //         continue;
    //     } else {
    //         added.push_back(it->second);
    //         name_ = new NameNode(new string("encoding"+it->second));
    //         size_ = new IntegerNode(new string("32"));
    //         currentField = new FieldDecNode(name_, size_, NULL);
    //         field_list -> push_back(currentField);
    //         AddFieldToJson("encoding"+it->second, 32);
    //     }
    // }

    // Add 2b packet type: generate new packet, check for assertion, forward to UT without change
    name_ = new NameNode(new string("pkt_type"));
    size_ = new IntegerNode(new string(to_string(2)));
    currentField = new FieldDecNode(name_, size_, NULL);
    field_list -> push_back(currentField);

    for (auto it = encoding_field_2_bitlength_.begin(); it != encoding_field_2_bitlength_.end(); it++) {
        name_ = new NameNode(new string(it->first));
        size_ = new IntegerNode(new string(std::to_string(it->second)));
        currentField = new FieldDecNode(name_, size_, NULL);
        field_list -> push_back(currentField);
        AddFieldToJson(it->first, it->second);
    }

    // cout << "Modifier.cpp line 214 num_assertions_: " << num_assertions_ << endl;
    for (int i = 0; i < num_assertions_; i++) {
        name_ = new NameNode(new string("assertion" + to_string(i)));
        size_ = new IntegerNode(new string(to_string(1)));
        currentField = new FieldDecNode(name_, size_, NULL);
        field_list -> push_back(currentField);
        AddFieldToJson("assertion" + to_string(i), 1);
    }

    // int leftoverBits = ((num_tbl_action_stmt_ + num_assertions_ + 50) % 8);
    int leftoverBits = ((num_assertions_ + 50) % 8);
    if (leftoverBits > 0) {
        name_ = new NameNode(new string("__pad"));
        size_ = new IntegerNode(new string(to_string(8 - leftoverBits)));
        currentField = new FieldDecNode(name_, size_, NULL);
        field_list -> push_back(currentField);    
        AddFieldToJson("__pad", 8 - leftoverBits);
    } 

  // Name of new header
    name_ = new NameNode(new string(sig_+"_visited_t"));

    HeaderTypeDeclarationNode* newHeader = new HeaderTypeDeclarationNode(name_, field_list, other_stmts_);

    return newHeader;
}


// void P4Modifier::GetHdrInstance2Nodes(AstNode* head) {
//   if (head == NULL) {
//     return;
//     }
//     if (work_set_.find(head) != work_set_.end()) {
//         return;
//     }
//     work_set_.insert(head);
//     if (TypeContains(head, "HeaderInstanceNode")) {
//         string key = (dynamic_cast<HeaderInstanceNode*>(head)) -> name_->toString();
//         PRINT_VERBOSE("Found HeaderInstanceNode: %s\n", key.c_str());
//         prev_hdrinstance_2_nodes_[key] = head;
//         num_hdr_instance_ += 1;
//     }
//     if (TypeContains(head, "InputNode")) {
//         if (dynamic_cast<InputNode*>(head) -> next_) {
//           GetHdrInstance2Nodes(dynamic_cast<InputNode*>(head) -> next_);
//       }
//       GetHdrInstance2Nodes(dynamic_cast<InputNode*>(head) -> expression_);
//     } 
// }

string P4Modifier::GetUnanchoredNodesStr() {
    string ret = "";
    for (auto n : unanchored_nodes_) {
        ret += n->toString();
        ret += "\n";
    }
    return ret;
}

bool P4Modifier::TypeContains(AstNode* n, const char* type) {
    if (n->nodeType_.find(string(type)) != string::npos) {
        return true;    
    } else {
        return false;
    }
}
