/* Copyright 2020-present University of Pennsylvania
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../../include/ast_nodes_p4.hpp"
#include "../../include/helper.hpp"

#include <regex>

using namespace std;

static string operator*(std::string str, int count)
{
    string ret;
    for (auto i = 0; i < count; i++) {
        ret = ret + str;
    }
    return ret;
}

BodyWordNode::BodyWordNode(WordType wordType, AstNode* contents)
    : wordType_(wordType) {

    nodeType_ = typeid(*this).name();
    contents_ = contents;
    contents_->parent_ = this;
}

BodyWordNode* BodyWordNode::deepCopy() {
    AstNode* newContents = contents_;
    // if (wordType_ == WordType::VARREF) {
    //     newContents = dynamic_cast<MblRefNode*>(contents_)->deepCopy();
    // }

    auto newNode = new BodyWordNode(wordType_, newContents);
    return newNode;
}

string BodyWordNode::toString() {
    ostringstream oss;
    if (wordType_ == WordType::INTEGER || wordType_ == WordType::HEX) {
        oss << contents_->toString();
    } else if (wordType_ == WordType::SPECIAL) {
        string output = contents_->toString();
        if (output == "if") {
            oss << " ";
        }
        oss << output;
    } else if (dynamic_cast<BodyNode*>(parent_) != NULL && dynamic_cast<BodyNode*>(parent_)->bodyOuter_ != NULL &&
               dynamic_cast<BodyNode*>(parent_)->bodyOuter_->str_ != NULL && dynamic_cast<BodyNode*>(parent_)->bodyOuter_->str_->wordType_ == WordType::SPECIAL &&
               dynamic_cast<BodyNode*>(parent_)->bodyOuter_->str_->contents_->toString() != ";" ) {
        oss << contents_->toString();
    } else if (dynamic_cast<BodyNode*>(parent_) != NULL && dynamic_cast<BodyNode*>(parent_)->bodyOuter_ != NULL &&
               dynamic_cast<BodyNode*>(parent_)->bodyOuter_->str_ != NULL && dynamic_cast<BodyNode*>(parent_)->bodyOuter_->str_->wordType_ == WordType::INTEGER) {
        oss << contents_->toString();
    } else if (dynamic_cast<BodyNode*>(parent_) != NULL && dynamic_cast<BodyNode*>(parent_)->bodyOuter_ != NULL &&
               dynamic_cast<BodyNode*>(parent_)->bodyOuter_->str_ != NULL && dynamic_cast<BodyNode*>(parent_)->bodyOuter_->str_->wordType_ == WordType::HEX) {
        oss << contents_->toString();
    } else {
        oss << string("    ")*indent_ << contents_->toString();
    }
    if (contents_->toString() == ";") {
        oss << "\n";
    }
    return oss.str();
}

BodyNode::BodyNode(AstNode* bodyOuter, AstNode* bodyInner, AstNode* str) {

    nodeType_ = typeid(*this).name();
    bodyOuter_ = dynamic_cast<BodyNode*>(bodyOuter);
    if (bodyOuter_) bodyOuter_->parent_ = this;
    bodyInner_ = dynamic_cast<BodyNode*>(bodyInner);
    if (bodyInner_) {
        bodyInner_->parent_ = this;
    }
    str_ = dynamic_cast<BodyWordNode*>(str);
    if (str_) {
        str_->parent_ = this;
    }
}

string BodyNode::toString() {
    ostringstream oss;
    // Called from the root BodyNode
    if (bodyOuter_) {
        bodyOuter_->indent_ = indent_;
        oss << bodyOuter_->toString();
    }
    if (str_) {
        str_->indent_ = indent_;
        oss << str_->toString();
        return oss.str();
    } else if (bodyInner_) {
        bodyInner_->indent_ = indent_ + 1;
        oss << string("    ")*indent_ << "{\n"
            << bodyInner_->toString()
            << string("    ")*indent_ << "}\n";
        return oss.str();
    } else {
        assert(false);
    }
}

SelectionType::SelectionType(AstNode* name) {
    nodeType_ = typeid(*this).name();
    if (name != NULL) {
        name_ = dynamic_cast<NameNode*>(name);
        name_ -> parent_ = this;
    } else {
        name_ = NULL;
    }
}

string SelectionType::toString() {
    if (name_ == NULL) {
        return "";
    }
    ostringstream oss;
    oss << "selection_type : "
        << name_ -> toString()
        << ";\n";
    return oss.str();
}

SelectionMode::SelectionMode(AstNode* name) {
    nodeType_ = typeid(*this).name();
    if (name != NULL) {
        name_ = dynamic_cast<NameNode*>(name);
        name_ -> parent_ = this;
    } else {
        name_ = NULL;
    }
}

string SelectionMode::toString() {
    if (name_ == NULL) {
        return "";
    }
    ostringstream oss;
    oss << "selection_mode : "
        << name_ -> toString()
        << ";\n";
    return oss.str();
}

SelectionKey::SelectionKey(AstNode* name) {
    name_ = dynamic_cast<NameNode*>(name);
    name_ -> parent_ = this;
}


string SelectionKey::toString() {
    ostringstream oss;
    oss << "selection_key : "
        << name_ -> toString()
        << ";\n";
    return oss.str();
}

ActionSelector::ActionSelector(AstNode* name) {
    nodeType_ = typeid(*this).name();
    if (name != NULL) {
        name_ = dynamic_cast<NameNode*>(name);
        name_ -> parent_ = this;
    } else {
        name_ = NULL;
    }
}

ActionProfileSpecificationNode::ActionProfileSpecificationNode(AstNode* name) {
    nodeType_ = typeid(*this).name();
    if (name != NULL) {
        name_ = dynamic_cast<NameNode*>(name);
        name_ -> parent_ = this;
    } else {
        name_ = NULL;
    }
}

string ActionProfileSpecificationNode::toString() {
    if (name_ == NULL) {
        return "";
    }
    ostringstream oss;
    oss << "action_profile: "
        << name_ -> toString()
        << ";\n";
    return oss.str();
}

ActionSelectorDecl::ActionSelectorDecl(AstNode* name, AstNode* selectionKey, AstNode* selectionMode, AstNode* selectionType) {
    nodeType_ = typeid(*this).name();
    name_ = dynamic_cast<NameNode*>(name);
    name_ -> parent_ = this;
    selectionKey_ = dynamic_cast<SelectionKey*>(selectionKey);
    selectionKey_ -> parent_ = this;
    selectionMode_ = dynamic_cast<SelectionMode*>(selectionMode);
    selectionMode_ -> parent_ = this;
    selectionType_ = dynamic_cast<SelectionType*>(selectionType);
    selectionType_ -> parent_ = this;
}

string ActionSelectorDecl::toString() {
    ostringstream oss;
    oss << "action_selector " 
        << name_ -> toString() << "{\n"
        << selectionKey_ -> toString()
        << selectionMode_ -> toString()
        << selectionType_ -> toString()
        << "}\n";

    return oss.str();
}

string ActionSelector::toString() {
    if (name_ == NULL) {
        return "";
    }
    ostringstream oss;
    oss << "dynamic_action_selection : "
        << name_ -> toString()
        << ";\n";
    return oss.str();
}

ActionProfile::ActionProfile(AstNode* name, AstNode* tableActions, AstNode* body, AstNode* actionSelector) {
    nodeType_ = typeid(*this).name();
    name_ = dynamic_cast<NameNode*>(name);
    name_ -> parent_ = this;
    tableActions_ = dynamic_cast<TableActionStmtsNode*>(tableActions);
    tableActions_ -> parent_ = this;
    body_ = dynamic_cast<BodyNode*>(body);
    body_ -> parent_ = this;
    actionSelector_ = dynamic_cast<ActionSelector*>(actionSelector);
    actionSelector_ -> parent_ = this;
}

string ActionProfile::toString() {
    ostringstream oss;
    oss << "action_profile "
        << name_ -> toString() << " {\n"
	<< "  actions{\n"
        << tableActions_ -> toString()
	<< "  }\n"
        << body_ -> toString()
        << actionSelector_ -> toString()
        << "}\n";

    return oss.str();
}

P4RegisterNode::P4RegisterNode(AstNode* name, AstNode* body) {
    nodeType_ = typeid(*this).name();
    name_ = dynamic_cast<NameNode*>(name);
    if (body->nodeType_.find("EmptyNode") != string::npos) {
        body_ = new BodyNode(NULL, NULL, new BodyWordNode(BodyWordNode::STRING, new StrNode(new string(""))));
        width_ = -1;
        instanceCount_ = -1;
    } else {
        body_ = dynamic_cast<BodyNode*>(body);
        std::stringstream ss(body_->toString());
        std::string item;
        while (std::getline(ss, item, ';'))
        {
            std::stringstream ss_(item);
            std::string item_;
            while (std::getline(ss_, item_, ':')) {
                boost::algorithm::trim(item_);
                if (item_.compare("width") == 0) {
                    std::getline(ss_, item_, ':');
                    boost::algorithm::trim(item_);
                    width_ = stoi(item_);
                } else if (item_.compare("instance_count") == 0) {
                    std::getline(ss_, item_, ':');
                    boost::algorithm::trim(item_);
                    instanceCount_ = stoi(item_);
                }
            }
        }
    }
    if (body_) body_->parent_ = this;
}

string P4RegisterNode::toString() {
    ostringstream oss;
    oss << "register "
        << name_->toString()
        << " {\n"
        << "    " << body_->toString()
        << "}\n\n";
    return oss.str();
}

P4ParserFunctionBody::P4ParserFunctionBody(AstNode* extSetStatements, AstNode* retStatement) {
    nodeType_ = typeid(*this).name();
    if (extSetStatements != NULL) {
        extSetList_ = dynamic_cast<ExtractOrSetList*>(extSetStatements);
        extSetList_ -> parent_ = this;
    } else {
        extSetList_ = NULL;
    }
    retStatement_ = dynamic_cast<ReturnStatementNode*>(retStatement);
    // cout << retStatement_ -> toString() << endl;
    retStatement_ -> parent_ = this;

}

P4ParserFunctionBody* P4ParserFunctionBody::MakeCopy() {
    PRINT_VERBOSE("Start P4ParserFunctionBody::MakeCopy\n");
    ExtractOrSetList* newList = NULL;
    ReturnStatementNode* newRet = retStatement_ -> MakeCopy();
    PRINT_VERBOSE("Done retStatement_ -> MakeCopy(): %s\n", newRet->toString().c_str());
    if (extSetList_ != NULL) {
        PRINT_VERBOSE("Enter branch extSetList_ != NULL\n");
        PRINT_VERBOSE("extSetList_->toString(): %s\n", extSetList_->toString().c_str());
        newList = extSetList_ -> MakeCopy();
        PRINT_VERBOSE("Done extSetList_ -> MakeCopy()\n");
        PRINT_VERBOSE("newList->toString(): %s\n", newList->toString().c_str());
        PRINT_VERBOSE("Done branch extSetList_ != NULL\n");
    }
    PRINT_VERBOSE("Return new P4ParserFunctionBody(newList, newRet);\n");
    return new P4ParserFunctionBody(newList, newRet);
}

string P4ParserFunctionBody::toString() {
    ostringstream oss;
    if (extSetList_ != NULL) {
        oss << extSetList_ -> toString()
            << "\n";
    }
    oss << retStatement_ -> toString()
        << "\n";

    return oss.str();
}

string ExtractOrSetNode::toString() {
    ostringstream oss;
    if (extStatement_ != NULL) {
        oss << extStatement_ -> toString();
    } else {
        oss << setStatement_ -> toString();
    }
    return oss.str();
}

string ExtractStatementNode::toString() {
    ostringstream oss;
    oss << "extract("
        << headerExtract_ -> toString()
        << ");\n";
    return oss.str();
}

string ExprNode::toString() {
    ostringstream oss;
    if (valid_) {
        oss << "valid(" << first_ -> toString() << ")";
    } else {
        oss << first_ -> toString();    
    }
    if (operator_ != NULL) {
        oss << " " << *operator_ << " " << second_ -> toString();
    }
    return oss.str();
}

string AssertNode::toString() {
    ostringstream oss;
    oss << string("    ")*indent_;
    oss << "apply(assert_table" << assertNum_ << ");\n";
    return oss.str(); 
}

string SetStatementNode::toString() {
    ostringstream oss;
    oss << "    set_metadata("
        << metadata_field_ -> toString()
        << ","
        << packet_field_ -> toString()
        << ");\n";
    return oss.str();
}

string ReturnValueNode::toString() {
    ostringstream oss;
    oss << "    return " << retValue_ -> toString();
    return oss.str();
}

string CaseEntryNode::toString() {
    ostringstream oss;
    if (matchValue_ != NULL) {
        oss << "        " << matchValue_ -> toString();
    } else {
        oss << "        default ";
    }

    oss << " : " << retValue_ -> toString()
        << ";\n";

    return oss.str();
}

string CaseEntryList::toString() {
    ostringstream oss;
    for (auto rsn : *list_) {
        oss << rsn->toString();
    }

    return oss.str();
}

string SelectExpList::toString() {
    ostringstream oss;
    for (auto rsn : *list_) {
        oss << rsn->toString() << ",";
    }
    // Remove last Comma
    string out = oss.str();
    out.pop_back();
    // oss << '\0';
    return out;
}

string ExtractOrSetList::toString() {
    PRINT_VERBOSE("Start ExtractOrSetList::toString()\n");
    ostringstream oss;
    for (auto rsn : *list_) {
        // PRINT_VERBOSE("rsn->toString() begin: %s\n", rsn->toString().c_str());
        oss << rsn->toString();
        // PRINT_VERBOSE("rsn->toString() end: %s\n", rsn->toString().c_str());
    }
    PRINT_VERBOSE("Done ExtractOrSetList::toString()\n");
    return oss.str();
}

string ReturnStatementNode::toString() {
    ostringstream oss;
    if (single != NULL) {
        oss << single -> toString() << ";\n";
    } else {
        oss << "    return select(" << selectList ->toString()
            << ") { \n" << caseList -> toString()
            << "    }\n";
    }
    return oss.str();
}


P4ParserNode::P4ParserNode(AstNode* name, AstNode* functionBody) {
    nodeType_ = typeid(*this).name();
    name_ = dynamic_cast<NameNode*>(name);
    body_ = dynamic_cast<P4ParserFunctionBody*>(functionBody);
    body_ -> parent_ = this;
}


P4ParserNode* P4ParserNode::MakeCopy() {
    NameNode* newName;
    if (name_ ->toString() == "ig_intr_md.ingress_port") {
        newName = new NameNode(new string(name_ -> toString()));
    } else {
        newName = new NameNode(new string(name_ -> toString() + "_clone"));
    }
    PRINT_VERBOSE("Created newName: %s\n", newName->toString().c_str());
    P4ParserFunctionBody* newFunc = body_ -> MakeCopy();
    PRINT_VERBOSE("Created newFunc\n");
    return new P4ParserNode(newName, newFunc);
}

ExtractStatementNode::ExtractStatementNode(AstNode* headerName) {
    nodeType_ = typeid(*this).name();
    headerExtract_ = dynamic_cast<HeaderExtractNode*>(headerName);
    headerExtract_ -> parent_ = this;
}

ExtractStatementNode* ExtractStatementNode::MakeCopy() {
    ExtractStatementNode* newNode = new ExtractStatementNode(headerExtract_ -> MakeCopy());
    return newNode;
}

ExprNode::ExprNode(AstNode* first, AstNode* second, string* operatorIn) {
    nodeType_ = typeid(*this).name();
    first_ = first;
    first_ -> parent_ = this;
    if (second != NULL) {
        second_ = second;
        operator_ = operatorIn;
        second_ -> parent_ = this;
    } else {
        second_ = NULL;
        operator_ = NULL;
    }
    valid_ = false;
}

ExprNode::ExprNode(AstNode* first, AstNode* second, string* operatorIn, bool valid) {
    nodeType_ = typeid(*this).name();
    first_ = first;
    first_ -> parent_ = this;
    if (second != NULL) {
        second_ = second;
        operator_ = operatorIn;
        second_ -> parent_ = this;
    } else {
        second_ = NULL;
        operator_ = NULL;
    }
    valid_ = valid;
}

ExprReturnType convertToBool(ExprReturnType input) {
    ExprReturnType output;
    if (input.mode == ExprReturnMode::TRUE) {
        output.mode = ExprReturnMode::TRUE;
    } else if (input.mode == ExprReturnMode::FALSE) {
        output.mode = ExprReturnMode::FALSE;
    } else if (input.mode == ExprReturnMode::ARITH && input.arithValue > 0) {
        output.mode = ExprReturnMode::TRUE;
    } else if (input.mode == ExprReturnMode::ARITH && input.arithValue == 0) {
        output.mode = ExprReturnMode::FALSE;
    } else {
        output.mode = ExprReturnMode::UNKNOWN;
    }
    return output;
}

ExprReturnType convertToArith(ExprReturnType input) {
    if (input.mode == ExprReturnMode::TRUE) {
        input.mode = ExprReturnMode::ARITH;
        input.arithValue = 1;
    } else if (input.mode == ExprReturnMode::FALSE) {
        input.mode = ExprReturnMode::ARITH;
        input.arithValue = 0;
    } 
    return input;
}

ExprReturnType resolveArith(ExprReturnType left, ExprReturnType right, string operator_) {
    left = convertToArith(left);
    right = convertToArith(right);
    ExprReturnType output;
    if (left.mode == ExprReturnMode::ARITH && right.mode == ExprReturnMode::ARITH) {
        if (operator_ == "==") {
            output.arithValue = (left.arithValue == right.arithValue);
        } else if (operator_ == "!=") {
            output.arithValue = (left.arithValue != right.arithValue);
        } else if (operator_ == ">") {
            output.arithValue = (left.arithValue > right.arithValue);
        } else if (operator_ == "<") {
            output.arithValue = (left.arithValue < right.arithValue);
        } else if (operator_ == ">=") {
            output.arithValue = (left.arithValue >= right.arithValue);
        } else if (operator_ == "<=") {
            output.arithValue = (left.arithValue <= right.arithValue);
        } else if (operator_ == "+") {
            output.arithValue = (left.arithValue + right.arithValue);
        } else if (operator_ == "-") {
            output.arithValue = (left.arithValue - right.arithValue);
        } else {
            cout << "------- fix this ----------" << endl;
            cout << "operator: " << operator_ << endl;
        }
        if (output.arithValue) {
            output.mode == ExprReturnMode::TRUE;
        } else {
            output.mode == ExprReturnMode::FALSE;
        }
    } else {
        output.mode = ExprReturnMode::UNKNOWN;
    }
    return output;
}

ExprReturnType ExprNode::evaluate(map<string, int> validHeaders, string latest) {
    ExprReturnType output;
    if (valid_) {
        // cout << "In valid!!" << first_ -> toString() << "!!" << endl;
        if (validHeaders.find(first_ -> toString()) != validHeaders.end()) {
            output.mode = TRUE;
        } else {
            output.mode = FALSE;
        }
        return output;
    } else if (second_ == NULL) {
        output.mode = ExprReturnMode::UNKNOWN;
        return output;   
    }

    ExprReturnType left = (dynamic_cast<ExprNode*>(first_)) -> evaluate(validHeaders);
    ExprReturnType right = (dynamic_cast<ExprNode*>(second_)) -> evaluate(validHeaders);
    if ((*operator_) == "||") {
        left = convertToBool(left);
        right = convertToBool(right);
        if (left.mode == ExprReturnMode::TRUE || right.mode == ExprReturnMode::TRUE) {
            output.mode = ExprReturnMode::TRUE;
        } else if (left.mode == ExprReturnMode::FALSE && right.mode == ExprReturnMode::FALSE) {
            output.mode = ExprReturnMode::FALSE;
        } else {
            output.mode = ExprReturnMode::UNKNOWN;
        }
        return output;
    } else if ((*operator_) == "&&") {
        left = convertToBool(left);
        right = convertToBool(right);
        if (left.mode == ExprReturnMode::TRUE && right.mode == ExprReturnMode::TRUE) {
            output.mode = ExprReturnMode::TRUE;
        } else if (left.mode == ExprReturnMode::FALSE || right.mode == ExprReturnMode::FALSE) {
            output.mode = ExprReturnMode::FALSE;
        } else {
            output.mode = ExprReturnMode::UNKNOWN;
        }
        return output;
    } else {
        output = resolveArith(left, right, *operator_);
    }
    return output;

}

vector<string> ExprNode::GetFields(vector<string> currentFields) {
    if (dynamic_cast<FieldNode*>(first_)) {
        currentFields.push_back(first_ -> toString());
        return currentFields;
    } else if (dynamic_cast<IntegerNode*>(this -> first_)) {
        return currentFields;
    } else if (dynamic_cast<HexNode*>(this -> first_)) {
        return currentFields;
    } else if (dynamic_cast<NameNode*>(this -> first_)) {
        return currentFields;
    } 
    currentFields = (dynamic_cast<ExprNode*>(first_)) -> GetFields(currentFields);
    currentFields = (dynamic_cast<ExprNode*>(second_)) -> GetFields(currentFields);
    return currentFields;
}

map<string, vector<string> > ExprNode::GetFixedValues(map<string, vector<string> > fixedValuesInCondtion) {
    // cout << this ->toString() << endl;
    if (dynamic_cast<FieldNode*>(first_)) {
        return fixedValuesInCondtion;
    } else if (dynamic_cast<IntegerNode*>(this -> first_)) {
        return fixedValuesInCondtion;
    } else if (dynamic_cast<HexNode*>(this -> first_)) {
        return fixedValuesInCondtion;
    } else if (dynamic_cast<NameNode*>(this -> first_)) {
        return fixedValuesInCondtion;
    } 
    map<string, vector<string> > temp = (dynamic_cast<ExprNode*>(first_)) -> GetFixedValues(fixedValuesInCondtion);
    for (auto const& x : temp)
    {
        for (string fieldName : x.second) {
            fixedValuesInCondtion[x.first].push_back(fieldName);
        }
        
    }
    temp = (dynamic_cast<ExprNode*>(second_)) -> GetFixedValues(fixedValuesInCondtion);
    for (auto const& x : temp)
    {
        for (string fieldName : x.second) {
            fixedValuesInCondtion[x.first].push_back(fieldName);   
        }
        
    }

    if ((*operator_) == "==") {
        fixedValuesInCondtion[first_ -> toString()].push_back(second_ -> toString());
    }
    return fixedValuesInCondtion;
}

ExprNode* ExprNode::MakeCopy() {
    PRINT_VERBOSE("Start ExprNode::MakeCopy\n");
    PRINT_VERBOSE("this->toString(): %s\n", this->toString().c_str());
    AstNode* firstExpr;
    ExprNode* newNode;
    if (dynamic_cast<FieldNode*>(this -> first_)) {
        PRINT_VERBOSE("dynamic_cast<FieldNode*>(this -> first_)\n");
        firstExpr = dynamic_cast<FieldNode*>(this -> first_) -> MakeCopy();
        newNode = new ExprNode(firstExpr, NULL, NULL);
        PRINT_VERBOSE("Done ExprNode::MakeCopy\n");
        return newNode;
    } else if (dynamic_cast<IntegerNode*>(this -> first_)) {
        PRINT_VERBOSE("dynamic_cast<IntegerNode*>(this -> first_)\n");
        firstExpr = new IntegerNode(new string(this -> first_ -> toString()));
        newNode = new ExprNode(firstExpr, NULL, NULL);
        PRINT_VERBOSE("Done ExprNode::MakeCopy\n");
        return newNode;
    } else if (dynamic_cast<HexNode*>(this -> first_)) {
        PRINT_VERBOSE("dynamic_cast<IntegerNode*>(this -> first_)\n");
        firstExpr = new HexNode(new string(this -> first_ -> toString()));
        newNode = new ExprNode(firstExpr, NULL, NULL);
        PRINT_VERBOSE("Done ExprNode::MakeCopy\n");
        return newNode;
    } else if (dynamic_cast<NameNode*>(this -> first_)) {
        firstExpr = new NameNode(new string(this -> first_ -> toString()));
        newNode = new ExprNode(firstExpr, NULL, NULL);
        return newNode;
    }
    PRINT_VERBOSE("Else: expr PLUS expr or expr MINUS expr\n");
    // Don't forget to pass value to firstExpr
    firstExpr = dynamic_cast<ExprNode*>(this -> first_) -> MakeCopy();
    ExprNode* secExpr = dynamic_cast<ExprNode*>(this -> second_) -> MakeCopy();
    string* newOp = new string(*(this -> operator_));
    newNode = new ExprNode(firstExpr, secExpr, newOp);
    PRINT_VERBOSE("newNode->toString(): %s\n", newNode->toString().c_str());
    PRINT_VERBOSE("Done ExprNode::MakeCopy\n");
    return newNode;
}

AssertNode::AssertNode(AstNode* expr, int assertNum) {
    nodeType_ = typeid(*this).name();
    expr_ = dynamic_cast<ExprNode*>(expr);
    expr_  -> parent_ = this;
    assertNum_ = assertNum;
    // cout << "Making assert statement" << endl;
}

// void AssertNode::setAssertNum(int assertNum) {
//     assertNum_ = assertNum;
// }

SetStatementNode::SetStatementNode(AstNode* metadataField, AstNode* packetField) {
    nodeType_ = typeid(*this).name();
    metadata_field_ = dynamic_cast<FieldNode*>(metadataField);
    packet_field_ = dynamic_cast<ExprNode*>(packetField);
    metadata_field_ -> parent_ = this;
    packet_field_ -> parent_ = this;
}

SetStatementNode* SetStatementNode::MakeCopy() {
    PRINT_VERBOSE("Start SetStatementNode::MakeCopy\n");
    PRINT_VERBOSE("Original metadata_field_->toString(): %s\n", metadata_field_->toString().c_str());
    FieldNode* newMetadata = this -> metadata_field_ -> MakeCopy();
    PRINT_VERBOSE("newMetadata->toString(): %s\n", newMetadata->toString().c_str());

    PRINT_VERBOSE("Original packet_field_->toString(): %s\n", this -> packet_field_->toString().c_str());
    ExprNode* newPacket = this -> packet_field_ -> MakeCopy();
    PRINT_VERBOSE("newPacket->toString(): %s\n", newPacket->toString().c_str());

    SetStatementNode* newNode = new SetStatementNode(newMetadata, newPacket);
    PRINT_VERBOSE("Done SetStatementNode::MakeCopy\n");
    return newNode;
}

ExtractOrSetNode::ExtractOrSetNode(AstNode* extStatement, AstNode* setStatement) {
    nodeType_ = typeid(*this).name();
    if (extStatement != NULL) {
        extStatement_ = dynamic_cast<ExtractStatementNode*>(extStatement);
        extStatement_ -> parent_ = this;
        setStatement_ = NULL;
    } else {
        setStatement_ = dynamic_cast<SetStatementNode*>(setStatement);
        setStatement_ -> parent_ = this;
        extStatement_ = NULL;
    }
}

ExtractOrSetNode* ExtractOrSetNode::MakeCopy() {
    PRINT_VERBOSE("ExtractOrSetNode::MakeCopy: %s\n", this->toString().c_str());
    if (extStatement_ != NULL) {
        PRINT_VERBOSE("extStatement_ != NULL, call extStatement_ -> MakeCopy()\n");
        ExtractStatementNode* new_stmt = extStatement_ -> MakeCopy();
        PRINT_VERBOSE("new_stmt->toString(): %s\n", new_stmt->toString().c_str());
        PRINT_VERBOSE("Done extStatement_ -> MakeCopy() call\n");
        return new ExtractOrSetNode(new_stmt, NULL);
    } else {
        PRINT_VERBOSE("extStatement_ == NULL, call setStatement_ -> MakeCopy()\n");
        SetStatementNode* new_stmt = setStatement_ -> MakeCopy();
        PRINT_VERBOSE("new_stmt->toString(): %s\n", new_stmt->toString().c_str());
        PRINT_VERBOSE("Done setStatement_ -> MakeCopy() call\n");
        return new ExtractOrSetNode(NULL, new_stmt);
    }
}

ExtractOrSetList::ExtractOrSetList() {
    nodeType_ = typeid(*this).name();
}

ExtractOrSetList* ExtractOrSetList::MakeCopy() {
    PRINT_VERBOSE("Start ExtractOrSetList::MakeCopy\n");
    ExtractOrSetList* newList = new ExtractOrSetList();
    for (auto it : *list_) {
        PRINT_VERBOSE("Original it->toString(): %s\n", it->toString().c_str());
        ExtractOrSetNode* new_copy = it -> MakeCopy();
        PRINT_VERBOSE("new_copy->toString(): %s\n", new_copy->toString().c_str());
        newList -> push_back(new_copy);
    }
    PRINT_VERBOSE("Done list_ iteration\n");
    PRINT_VERBOSE("newList->toString(): %s\n", newList->toString().c_str());
    PRINT_VERBOSE("Done ExtractOrSetList::MakeCopy\n");
    return newList;
}

ReturnValueNode::ReturnValueNode(AstNode* retValue) {
    nodeType_ = typeid(*this).name();
    retValue_ = dynamic_cast<NameNode*>(retValue);
}

ReturnValueNode* ReturnValueNode::MakeCopy() {
    NameNode* newName;
    if ((retValue_ -> toString()) == "ingress") {
        newName = new NameNode(new string(retValue_ -> toString()));
    } else {
        newName = new NameNode(new string(retValue_ -> toString() + "_clone"));
    }
    return new ReturnValueNode(newName);
}

CaseEntryNode::CaseEntryNode(AstNode* matchValue, AstNode* returnValue) {
    nodeType_ = typeid(*this).name();
    PRINT_VERBOSE("Start CaseEntryNode constructor\n");
    if (matchValue != NULL) {
        matchValue_ = dynamic_cast<BodyWordNode*>(matchValue);
        matchValue_ -> parent_ = this;
    } else {
        matchValue_ = NULL;
    }
    PRINT_VERBOSE("returnValue: %s\n", returnValue -> toString().c_str());
    retValue_ = dynamic_cast<NameNode*>(returnValue);
    retValue_ -> parent_ = this;
    PRINT_VERBOSE("Done CaseEntryNode constructor\n");
    PRINT_VERBOSE("%s\n", toString().c_str());
}

CaseEntryNode* CaseEntryNode::MakeCopy() {
    PRINT_VERBOSE("Start CaseEntryNode::MakeCopy: %s\n", toString().c_str());
    NameNode* newName;
    if ((retValue_ -> toString()) == "ingress") {
        newName = new NameNode(new string(retValue_ -> toString()));
    } else {
        newName = new NameNode(new string(retValue_ -> toString() + "_clone"));
    }
    PRINT_VERBOSE("newName->toString(): %s\n", newName->toString().c_str());
    BodyWordNode* newMatch = NULL;
    if (matchValue_ != NULL) {
        PRINT_VERBOSE("matchValue_ != NULL\n");
        newMatch = new BodyWordNode(matchValue_ -> wordType_,
            new NameNode(new string(matchValue_ -> contents_ -> toString())));
        PRINT_VERBOSE("newMatch->toString(): %s\n", newMatch->toString().c_str());
    }
    PRINT_VERBOSE("Done CaseEntryNode::MakeCopy\n");
    return new CaseEntryNode(newMatch, newName);
}


CaseEntryList::CaseEntryList() {
    nodeType_ = typeid(*this).name();
}

CaseEntryList* CaseEntryList::MakeCopy() {
    PRINT_VERBOSE("Start CaseEntryList::MakeCopy\n");
    CaseEntryList* newList = new CaseEntryList();
    for (auto it : *list_) {
        PRINT_VERBOSE("Original it->toString(): %s\n", it->toString().c_str());
        CaseEntryNode* new_copy = it -> MakeCopy();
        PRINT_VERBOSE("new_copy->toString(): %s\n", new_copy->toString().c_str());
        newList -> push_back(new_copy);
    }
    PRINT_VERBOSE("Done list_ iteration\n");
    PRINT_VERBOSE("newList->toString(): %s\n", newList->toString().c_str());
    PRINT_VERBOSE("Done CaseEntryList::MakeCopy\n");
    return newList;
}

SelectExpList::SelectExpList() {
    nodeType_ = typeid(*this).name();
}

SelectExpList* SelectExpList::MakeCopy() {
    SelectExpList* newList = new SelectExpList();
    for (auto it : *list_) {
        newList -> push_back(it -> MakeCopy());
    }
    return newList;
}

HeaderExtractNode::HeaderExtractNode(AstNode* _name, AstNode* _index, bool _next) {
    nodeType_ = typeid(*this).name();

    name_ = dynamic_cast<NameNode*>(_name);
    name_ -> parent_ = this;
    next_ = _next;

    if (_index != NULL) {
        index_ = dynamic_cast<IntegerNode*>(_index);
        index_ -> parent_ = this;
    } else {
        index_ = NULL;
    }
}


HeaderExtractNode* HeaderExtractNode::MakeCopy() {
    NameNode* newName = new NameNode(new string(name_ -> toString() + "_clone"));
    IntegerNode* newIndex;

    if (this -> index_ == NULL) {
        newIndex = NULL;
    } else {
        newIndex = new IntegerNode(new string(index_ -> toString()));
    }
    HeaderExtractNode* newNode = new HeaderExtractNode(newName, newIndex, next_);
    return newNode;
}


string HeaderExtractNode::toString() {
    ostringstream oss;
    oss << name_ -> toString();
    if (next_) {
        oss << "[next]";
    } else if (index_ != NULL) {
        oss << "[" << index_ -> toString() << "]";
    }
    return oss.str();
}


CurrentNode::CurrentNode(AstNode* _start, AstNode* _end) {
    nodeType_ = typeid(*this).name();
    start_ = dynamic_cast<IntegerNode*>(_start);
    end_ = dynamic_cast<IntegerNode*>(_end);
    start_ -> parent_ = this;
    end_ -> parent_ = this;
}

CurrentNode* CurrentNode::MakeCopy() {
    IntegerNode* newStart = new IntegerNode(new string(start_ -> toString()));
    IntegerNode* newEnd = new IntegerNode(new string(end_ -> toString()));
    return new CurrentNode(newStart, newEnd);
}

string CurrentNode::toString() {
    ostringstream oss;
    oss << "current("
        << start_ -> toString()
        << ","
        << end_ -> toString()
        << ")";
    return oss.str();
}

FieldOrDataNode::FieldOrDataNode(AstNode* _field, AstNode* _current) {
    nodeType_ = typeid(*this).name();
    if (_field != NULL) {
        field_ = dynamic_cast<FieldNode*>(_field);
        field_ -> parent_ = this;
    } else {
        field_ = NULL;
    }
    if (_current != NULL) {
        current_ = dynamic_cast<CurrentNode*>(_current);
        current_ -> parent_ = this;
    } else {
        current_ = NULL;
    }
}

FieldOrDataNode* FieldOrDataNode::MakeCopy() {
    FieldNode* newField = NULL;
    CurrentNode* newCurrent = NULL;
    if (field_ != NULL) {
        newField = field_ -> MakeCopy();
    }
    if (current_ != NULL) {
        newCurrent = current_ -> MakeCopy();
    }
    return new FieldOrDataNode(newField, newCurrent);
}

string FieldOrDataNode::toString() {
    ostringstream oss;
    if (field_ != NULL) {
        oss << field_ -> toString();
    }
    if (current_ != NULL) {
        oss << current_ -> toString();
    }
    return oss.str();
}

ReturnStatementNode::ReturnStatementNode(AstNode* _single, AstNode* _selectList, AstNode* _caseList) {
    nodeType_ = typeid(*this).name();
    PRINT_VERBOSE("Start ReturnStatementNode constructor\n");   
    if (_single != NULL) {
        single = dynamic_cast<ReturnValueNode*>(_single);
        single -> parent_ = this;
    } else {
        single = NULL;
    }
    if (_selectList != NULL) {
        selectList = dynamic_cast<SelectExpList*>(_selectList);
        selectList -> parent_ = this;
    } else {
        selectList = NULL;
    }

    if (_caseList != NULL) {
        caseList = dynamic_cast<CaseEntryList*>(_caseList);
        caseList -> parent_ = this;
        PRINT_VERBOSE("CaseList is not NULL");
        PRINT_VERBOSE("%s\n", caseList -> toString().c_str());
    } else {
        caseList = NULL;
    }
    PRINT_VERBOSE("DONE ReturnStatementNode constructor\n");   
    // cout << this -> toString() << endl;
    // cout << "going out of ReturnStatementNode constructor" << endl;
}

ReturnStatementNode* ReturnStatementNode::MakeCopy() {
    PRINT_VERBOSE("Start ReturnStatementNode::MakeCopy\n");
    ReturnValueNode* newRet = NULL;
    SelectExpList* newSelect = NULL;
    CaseEntryList* newCase = NULL;
    PRINT_VERBOSE("Original ReturnStatementNode: %s\n", toString().c_str());

    if (single != NULL) {
        newRet = single -> MakeCopy();
    }
    if (selectList != NULL) {
        newSelect = selectList -> MakeCopy();
    }
    if (caseList != NULL) {
        newCase = caseList -> MakeCopy();
    }
    PRINT_VERBOSE("DONE ReturnStatementNode::MakeCopy\n");
    return new ReturnStatementNode(newRet, newSelect, newCase);
}



string P4ParserNode::toString() {
    ostringstream oss;
    oss << "parser "
        << name_->toString()
        << " {\n";
    if (prefixStr_.length() != 0) {
        oss << prefixStr_;
    }
    std::string final_body = body_->toString();

    if (redirect_to_clone_) {
        // Search return ingress and default: ingress, replace them with parse_clone_all
        std::regex e_val_return ("return\\s*ingress");  // \s doesn't work, try \\s
        final_body = std::regex_replace (final_body, e_val_return, "return " + parser_clone_all_);
        std::regex e_val_default ("default\\s*:\\s*ingress");
        final_body = std::regex_replace (final_body, e_val_default, "default: " + parser_clone_all_);
    }
    oss << "    " << final_body
        << "}\n\n";
    return oss.str();
}


void P4ParserNode::Clone(std::string parser_clone) {
    redirect_to_clone_ = true;
    parser_clone_all_ = parser_clone;
}

P4ControlNode::P4ControlNode(AstNode* name, AstNode* controlBlock) {
    nodeType_ = typeid(*this).name();
    name_ = dynamic_cast<NameNode*>(name);
    name_ -> parent_ = this;
    if (controlBlock != NULL) {
        controlBlock_ = dynamic_cast<P4ControlBlock*>(controlBlock);
        controlBlock_ -> parent_ = this;
    } else {
        controlBlock_ = NULL;
    }
}

string P4ControlNode::toString() {
    ostringstream oss;
    oss << "control " << name_ -> toString();
    if (controlBlock_ != NULL) {
        if (rename_popular_actions_) {
            std::string tmp_str = controlBlock_->toString();
            for (std::string action : popular_actions_) {
                std::regex e_val ("apply\\s*\\(\\s*([a-z_\\d]*)\\s*\\)\\s*\\{\\n\\s*(" + action + ")\\s*\\{");
                tmp_str = std::regex_replace (tmp_str, e_val, "apply($1) {\n $2_fp4_$1 {");
            }
            oss << "    " << tmp_str;
        } else {
            oss << controlBlock_ -> toString();    
        }
        
    }
    return oss.str();
}

void P4ControlBlock::clear() {
    clear_ = true;
}

void P4ControlNode::RenamePopularActions(std::vector<std::string> popularactions) {
    rename_popular_actions_ = true;
    for (std::string action : popularactions) {
        popular_actions_.push_back(action);
    }
    // cout << "Number of popular actions " << popular_actions_.size() << endl;
}

// void P4ControlNode::append(std::string str) {
//     suffixStr_ += str;
// }


void P4ControlBlock::append(std::string str) {
    suffixStr_ += str;
}

P4ControlBlock::P4ControlBlock(AstNode* controlStatements) {
    nodeType_ = typeid(*this).name();
    controlStatements_ = dynamic_cast<ControlStatements*>(controlStatements);
    controlStatements_ -> parent_ = this;
}

string P4ControlBlock::toString() {
    ostringstream oss;
    oss << " {\n";
    if (!clear_) {
        controlStatements_ -> indent_ = indent_ + 1;
        oss << controlStatements_ -> toString() << "\n";
    }
    if (suffixStr_.length() != 0) {
        oss << suffixStr_;
    }
    oss << string("    ")*indent_;
    oss << "}\n";
    return oss.str();
}

ControlStatements::ControlStatements() {
    nodeType_ = typeid(*this).name();
}


string ControlStatements::toString() {
    ostringstream oss;
    for (auto rsn : *list_) {
        rsn -> indent_ = indent_;
        oss << rsn->toString();
    }
    return oss.str();
}

ControlStatement::ControlStatement(AstNode* stmt, StmtType stmtType) : stmtType_(stmtType) {
    nodeType_ = typeid(*this).name();
    stmt_ = stmt;
    stmt_ -> parent_ = this;
}

string ControlStatement::toString() {
    ostringstream oss;
    stmt_ -> indent_ = indent_;
    oss << stmt_ -> toString();
    if (stmtType_ == StmtType::CONTROL) {
        oss << "();\n";
    } else {
        oss << "\n";
    }
    return oss.str();
}

ApplyTableCall::ApplyTableCall(AstNode* name) {
    nodeType_ = typeid(*this).name();
    name_ = dynamic_cast<NameNode*>(name);
    name_ -> parent_ = this;
}

string ApplyTableCall::toString() {
    ostringstream oss;
    oss << string("    ")*indent_;
    oss << "apply(" << name_ -> toString()
        << ");\n";
    return oss.str();
}

ApplyAndSelectBlock::ApplyAndSelectBlock(AstNode* name, AstNode* caseList) {
    nodeType_ = typeid(*this).name();
    name_ = dynamic_cast<NameNode*>(name);
    name_ -> parent_ = this;

    if (caseList != NULL) {
        caseList_ = dynamic_cast<CaseList*>(caseList);
        caseList_ -> parent_ = this;
    } else {
        caseList_ = NULL;
    }
}

string ApplyAndSelectBlock::toString() {
    ostringstream oss;
    oss << string("    ")*indent_;
    oss << "apply(" << name_ -> toString()
        << ")\n{\n";
    if (caseList_ != NULL) {
        // caseList_ -> indent_ = indent_ + 1;
        oss << caseList_ -> toString();
    }
    oss << string("    ")*indent_;
    oss << "}\n";
    return oss.str();
}

CaseList::CaseList(AstNode* actionCases, AstNode* hitOrMissCases) {
    nodeType_ = typeid(*this).name();
    if (actionCases != NULL) {
        actionCases_ = dynamic_cast<ActionCases*>(actionCases);
        actionCases_ -> parent_ = this;
    } else {
        actionCases_ = NULL;
    }
    if (hitOrMissCases != NULL) {
        hitOrMissCases_ = dynamic_cast<HitOrMissCases*>(hitOrMissCases);
        hitOrMissCases_ -> parent_ = this;
    } else {
        hitOrMissCases_ = NULL;
    }
}

string CaseList::toString() {
    ostringstream oss;
    if (actionCases_ != NULL) {
        actionCases_ -> indent_ = indent_;
        oss << actionCases_ -> toString();
    }
    if (hitOrMissCases_ != NULL) {
        hitOrMissCases_ -> indent_ = indent_;
        oss << hitOrMissCases_ -> toString();
    }
    return oss.str();
}

ActionCases::ActionCases() {
    nodeType_ = typeid(*this).name();
}


string ActionCases::toString() {
    ostringstream oss;
    for (auto rsn : *list_) {
        rsn -> indent_ = indent_;
        oss << rsn->toString();
    }
    return oss.str();
}

HitOrMissCases::HitOrMissCases() {
    nodeType_ = typeid(*this).name();
}


string HitOrMissCases::toString() {
    ostringstream oss;
    for (auto rsn : *list_) {
        rsn -> indent_ = indent_;
        oss << rsn->toString();
    }
    return oss.str();
}


ActionCase::ActionCase(AstNode* name, AstNode* controlBlock) {
    nodeType_ = typeid(*this).name();
    if (name != NULL) {
        name_ = dynamic_cast<NameNode*>(name);
        name_ -> parent_ = this;
    } else {
        name_ = NULL;
    }
    controlBlock_ = dynamic_cast<P4ControlBlock*>(controlBlock);
    controlBlock_ -> parent_ = this;
}

string ActionCase::toString() {
    ostringstream oss;
    oss << string("    ")*indent_;
    if (name_ != NULL) {
        oss << name_ -> toString() << " ";
    } else {
        oss << "default ";
    }
    // controlBlock_ -> indent_ = indent_ + 1;
    oss << controlBlock_ -> toString() << " ";
    return oss.str();
}

HitOrMissCase::HitOrMissCase(AstNode* controlBlock, StmtType stmtType) : stmtType_(stmtType) {
    nodeType_ = typeid(*this).name();
    controlBlock_ = dynamic_cast<P4ControlBlock*>(controlBlock);
    controlBlock_ -> parent_ = this;
}

string HitOrMissCase::toString() {
    ostringstream oss;
    oss << string("    ")*indent_;
    if (stmtType_ == StmtType::HIT) {
        oss << "hit";
    } else if (stmtType_ == StmtType::MISS) {
        oss << "miss";
    } else {
        cout << "Unknown type, bug present in ast_nodes_p4.cpp, class HitOrMissCases function toString" << endl;
    }
    // controlBlock_ -> indent_ = indent_ + 1;
    oss << controlBlock_ -> toString();
    return oss.str();
}

IfElseStatement::IfElseStatement(AstNode* body, AstNode* controlBlock, AstNode* elseBlock) {
    nodeType_ = typeid(*this).name();
    condition_ = dynamic_cast<ExprNode*>(body);
    controlBlock_ = dynamic_cast<P4ControlBlock*>(controlBlock);
    condition_ -> parent_ = this;
    controlBlock_ -> parent_ = this;
    if (elseBlock != NULL) {
        elseBlock_ = dynamic_cast<ElseBlock*>(elseBlock);
        elseBlock_ -> parent_ = this;
    } else {
        elseBlock_ = NULL;
    }
}

string IfElseStatement::toString() {
    ostringstream oss;
    oss << string("    ")*indent_;
    controlBlock_ -> indent_ = indent_;
    oss << "if ("
        << condition_ -> toString()
        << ") "
        << controlBlock_ -> toString();
    if (elseBlock_ != NULL) {
        elseBlock_ -> indent_ = indent_;
        oss << elseBlock_ -> toString();
    }
    return oss.str();
}


string ElseBlock::toString() {
    ostringstream oss;
    oss << string("    ")*indent_;
    oss << "else ";
    if (controlBlock_ != NULL) {
        controlBlock_ -> indent_ = indent_;
        oss << controlBlock_ -> toString();
    }
    if (ifElseStatement_ != NULL) {
        ifElseStatement_ -> indent_ = indent_;
        oss << ifElseStatement_ -> toString();
    }
    return oss.str();
}

ElseBlock::ElseBlock(AstNode* controlBlock, AstNode* ifElseStatement) {
    nodeType_ = typeid(*this).name();
    if (controlBlock != NULL) {
        controlBlock_ = dynamic_cast<P4ControlBlock*>(controlBlock);
        controlBlock_ -> parent_ = this;
    } else {
        controlBlock_ = NULL;
    }
    if (ifElseStatement != NULL) {
        ifElseStatement_ = dynamic_cast<IfElseStatement*>(ifElseStatement);
        ifElseStatement_ -> parent_ = this;
    } else {
        ifElseStatement_ = NULL;
    }
}



P4ExprNode::P4ExprNode(AstNode* keyword, AstNode* name1, AstNode* name2,
                       AstNode* opts, AstNode* body) {
    nodeType_ = typeid(*this).name();
    keyword_ = dynamic_cast<KeywordNode*>(keyword);
    name1_ = dynamic_cast<NameNode*>(name1);
    if (name1_) name1_->parent_ = this;
    name2_ = name2;
    if (name2_) name2_->parent_ = this;
    opts_ = opts;
    if (opts_) opts_->parent_ = this;
    if (body->nodeType_.find("EmptyNode") != string::npos) {
        body_ = new BodyNode(NULL, NULL, new BodyWordNode(BodyWordNode::STRING, new StrNode(new string(""))));
    } else {
        body_ = dynamic_cast<BodyNode*>(body);
    }
    if (body_) body_->parent_ = this;
}

// P4PragmaNode::P4PragmaNode(AstNode* name1, AstNode* name2, AstNode* name3) {
P4PragmaNode::P4PragmaNode(string* word) {
    nodeType_ = typeid(*this).name();
    word_ = word;
    // word_ -> parent_ = this;
    // name1_ = name1;
    // name2_ = name2;
    // name3_ = name3;
    // if (name1_) name1_->parent_ = this;
    // if (name2_) name2_->parent_ = this;
    // if (name3_) name2_->parent_ = this;
}

string P4PragmaNode::toString() {
    ostringstream oss;
    oss << *word_;
    // oss << "@pragma " << name1_->toString();
    // if (name2_)
    //     oss << " " << name2_->toString();
    // if (name3_)
    //     oss << " " << name3_->toString();        
    return oss.str();
}

string P4ExprNode::toString() {
    ostringstream oss;
    oss << keyword_->toString() << " "
        << name1_->toString();

    // Append name2 if its present
    if (name2_) {
        if (keyword_->toString() == "calculated_field") {
            oss << "."
                << name2_->toString();
        } else {
            oss << " " << name2_->toString();
        }
    } else {
        oss << " ";
    }

    // Append opts if they're present
    if (opts_) {
        oss << "(" << opts_->toString() << ") ";
    }

    // Append body if its present.
    if (body_) {
        string res = body_->toString();
        oss << "{\n" << res << "}\n";
    } else {
        // If there's no body, its a statement that
        // ends with a semicolon
        oss << ";";
    }
    return oss.str();
}

KeywordNode::KeywordNode(string* word) {
    nodeType_ = typeid(*this).name();
    word_ = word;
}

string KeywordNode::toString() {
    return *word_;
}

OptsNode::OptsNode(AstNode* nameList) {
    nodeType_ = typeid(*this).name();
    nameList_ = nameList;
}

string OptsNode::toString() {
    if (nameList_) {
        return nameList_->toString();
    }
    return "";
}

NameListNode::NameListNode(AstNode* nameList, AstNode* name) {
    nodeType_ = typeid(*this).name();
    nameList_ = nameList;
    name_ = name;
}

string NameListNode::toString() {
    if (nameList_) {
        ostringstream oss;
        oss << nameList_->toString()
            << ", "
            << name_->toString();
        return oss.str();
    } else {
        return name_->toString();
    }

}

TableReadStmtNode::TableReadStmtNode(MatchType matchType, AstNode* field) {
    nodeType_ = typeid(*this).name();
    matchType_ = matchType;
    field_ = field;
    field_->parent_ = this;
}

string TableReadStmtNode::toString() {
    ostringstream oss;
    oss << "        " << field_->toString() << " : ";
    switch (matchType_) {
    case EXACT: oss << "exact"; break;
    case TERNARY: oss << "ternary"; break;
    case LPM: oss << "lpm"; break;
    case VALID: oss << "valid"; break;
    case RANGE: oss << "range"; break;
    }
    oss << ";\n";
    return oss.str();
}

TableReadStmtsNode::TableReadStmtsNode() {
    nodeType_ = typeid(*this).name();
}

string TableReadStmtsNode::toString() {
    ostringstream oss;
    for (auto rsn : *list_) {
        oss << rsn->toString();
    }
    return oss.str();
}

PragmaNode::PragmaNode(std::string name) {
    nodeType_ = typeid(*this).name();
    name_ = name;
}

string PragmaNode::toString() {
    ostringstream oss;
    oss << name_ << "\n";
    return oss.str();
}

PragmasNode::PragmasNode() {
    nodeType_ = typeid(*this).name();
}

string PragmasNode::toString() {
    ostringstream oss;
    for (auto asn : *list_) {
        oss << asn->toString();
    }
    return oss.str();
}


TableActionStmtNode::TableActionStmtNode(AstNode* name) {
    nodeType_ = typeid(*this).name();
    name_ = dynamic_cast<NameNode*>(name);
}

string TableActionStmtNode::toString() {
    ostringstream oss;
    oss << "        " << name_->toString() << ";\n";
    return oss.str();
}

TableActionStmtsNode::TableActionStmtsNode() {
    nodeType_ = typeid(*this).name();
}

string TableActionStmtsNode::toString() {
    ostringstream oss;
    for (auto asn : *list_) {
        oss << asn->toString();
    }
    return oss.str();
}

TableNode::TableNode(AstNode* name, AstNode* reads, AstNode* actions,
                     string options, AstNode* pragma, AstNode* actionprofile) {
    nodeType_ = typeid(*this).name();
    name_ = dynamic_cast<NameNode*>(name);
    name_->parent_ = this;
    reads_ = dynamic_cast<TableReadStmtsNode*>(reads);
    if (reads_) reads_->parent_ = this;
    actions_ = dynamic_cast<TableActionStmtsNode*>(actions);
    if (actions_) actions_->parent_ = this;
    
    actionprofile_ = dynamic_cast<ActionProfileSpecificationNode*>(actionprofile);
    if (actionprofile_) actionprofile_->parent_ = this;
    
    pragma_ = dynamic_cast<PragmasNode*>(pragma);
    if (pragma_) pragma_->parent_ = this;

    boost::algorithm::trim_right_if(options, boost::is_any_of("; \t\n"));
    if (options != "") {
        boost::split(options_, options, boost::is_any_of(";"));
    }
    for (int i = 0; i < options_.size(); i++) {
        boost::algorithm::trim(options_[i]);
    }
    isMalleable_ = false;
    pragmaTransformed_ = false;
}

string TableNode::toString() {
    if (removed_) {
        return "";
    }
    ostringstream oss;
    if (pragma_ != NULL) {
        oss << pragma_->toString();
    }
    oss << "table " << name_->toString() << " {\n";
    if (reads_ && reads_->list_->size() > 0) {
        oss << "    reads {\n"
            << reads_->toString()
            << "    }\n";
    }
    if (actions_) {
      oss << "    actions {\n"
        << actions_->toString()
        << "    }\n";
    }
    if (actionprofile_) {
      oss << actionprofile_->toString();
    }
    for (auto str : options_) {
        oss << "    " << str << ";\n";
    }
    oss << "}\n\n";
    return oss.str();
}

//void TableNode::transformPragma() {
//    if (pragma_->toString().compare("") != 0 && !pragmaTransformed_) {
//        std::vector<string> tmp_v;
//        boost::algorithm::split(tmp_v, pragma_->toString(), boost::algorithm::is_space());
//        pragma_ = "";
//        for (int i = 0; i < tmp_v.size() - 1; i++) {
//            pragma_ += tmp_v[i];
 //           pragma_ += " ";
 //       }
 //       int stage_num = stoi(tmp_v[tmp_v.size() - 1]);
  //      stage_num ++;
   //     pragma_ += std::to_string(stage_num);

     //   pragmaTransformed_ = true;
   // }
//}

FieldDecNode::FieldDecNode(AstNode* name, AstNode* size, AstNode* information) {
    nodeType_ = typeid(*this).name();
    name_ = dynamic_cast<NameNode*>(name);
    size_ = dynamic_cast<IntegerNode*>(size);
    if (information != NULL) {
        information_ = dynamic_cast<BodyNode*>(information);
    } else {
        information_ = NULL;
    }
}

ALUStatementNode::ALUStatementNode(AstNode* command, AstNode* expr) {
    nodeType_ = typeid(*this).name();
    command_ = dynamic_cast<NameNode*>(command);
    expr_ = dynamic_cast<ExprNode*>(expr);
    expr_->parent_ = this;
    command_->parent_ = this;
}

string ALUStatementNode::toString() {
    ostringstream oss;
    oss << "    " << command_ -> toString() << " : " << expr_ -> toString() << ";\n";
    return oss.str();
}

ALUStatmentList::ALUStatmentList() {
    nodeType_ = typeid(*this).name();
}

string ALUStatmentList::toString() {
    ostringstream oss;
    for (auto alus : *list_) {
        oss << alus->toString() << endl;
    }
    return oss.str();
}

BlackboxNode::BlackboxNode(AstNode* name, AstNode* alu_statements) {
    nodeType_ = typeid(*this).name();
    name_ = dynamic_cast<NameNode*>(name);
    alu_statements_ = dynamic_cast<ALUStatmentList*>(alu_statements);
    alu_statements_->parent_ = this;
    name_->parent_ = this;
}

string BlackboxNode::toString() {
    ostringstream oss;
    oss << "blackbox stateful_alu " << name_ -> toString() << " {\n"
        << alu_statements_ -> toString()
        << "\n}\n";
    return oss.str();
}

string FieldDecNode::toString() {
    ostringstream oss;
    oss << "        " << name_->toString() << " : " << size_->toString();
    if (information_ != NULL) {
        oss << " (" << information_ -> toString() << ")";
    }
    oss << ";";
    return oss.str();
}

FieldDecsNode::FieldDecsNode() {
    nodeType_ = typeid(*this).name();
}

string FieldDecsNode::toString() {
    ostringstream oss;
    for (auto fdn : *list_) {
        oss << fdn->toString() << endl;
    }
    return oss.str();
}

HeaderTypeDeclarationNode::HeaderTypeDeclarationNode(AstNode* name,
        AstNode* field_decs,
        AstNode* other_stmts) {
    nodeType_ = typeid(*this).name();
    name_ = dynamic_cast<NameNode*>(name);
    field_decs_ = dynamic_cast<FieldDecsNode*>(field_decs);
    other_stmts_ = other_stmts;
}

string HeaderTypeDeclarationNode::toString() {
    ostringstream oss;
    oss << "header_type " << name_->toString() << " {\n";
    oss << "    fields {\n";
    oss << field_decs_->toString();
    oss << "    }\n";
    oss << other_stmts_->toString();
    oss << "}\n\n";
    return oss.str();
}

HeaderInstanceNode::HeaderInstanceNode(AstNode* type, AstNode* name, AstNode* numHeaders) {
    nodeType_ = typeid(*this).name();
    type_ = dynamic_cast<NameNode*>(type);
    name_ = dynamic_cast<NameNode*>(name);
    if (numHeaders != NULL) {
        numHeaders_ = dynamic_cast<IntegerNode*>(numHeaders);
    } else {
        numHeaders_ = NULL;
    }
}

string HeaderInstanceNode::toString() {
    ostringstream oss;
    oss << "header " << type_->toString() << " " << name_->toString();
    if (numHeaders_ != NULL) {
        oss << "[" << numHeaders_ -> toString() << "]";
    }
    oss << ";\n\n";
    return oss.str();
}

MetadataInstanceNode::MetadataInstanceNode(AstNode* type, AstNode* name) {
    nodeType_ = typeid(*this).name();
    type_ = dynamic_cast<NameNode*>(type);
    name_ = dynamic_cast<NameNode*>(name);
}

string MetadataInstanceNode::toString() {
    ostringstream oss;
    oss << "metadata " << type_->toString() << " " << name_->toString() << ";\n\n";
    return oss.str();
}

std::string MetadataInstanceNode::MakeCopyToString() {
    ostringstream oss;
    oss << "metadata " << type_->toString() << " " << name_->toString() << "_clone;\n\n";
    return oss.str();
}

ArgsNode::ArgsNode() {
    nodeType_ = typeid(*this).name();
}

ArgsNode* ArgsNode::deepCopy() {
    auto newNode = new ArgsNode();
    for (auto bw : *list_) {
        auto newArg = bw->deepCopy();
        newNode->push_back(newArg);
    }

    return newNode;
}

string ArgsNode::toString() {
    bool first = true;
    ostringstream oss;
    for (auto bw : *list_) {
        if (first) {
            first = false;
        } else {
            oss << ", ";
        }
        oss << bw->toString();
    }
    return oss.str();
}

ActionParamNode::ActionParamNode(AstNode* param) {
    nodeType_ = typeid(*this).name();
    param_ = param;
    param->parent_ = this;
}

ActionParamNode* ActionParamNode::deepCopy() {
    AstNode* newParam = param_;
    // if (param_->nodeType_.find("MblRefNode") != string::npos) {
    //     newParam = dynamic_cast<MblRefNode*>(param_)->deepCopy();
    // }
    auto newNode = new ActionParamNode(newParam);
    return newNode;
}

string ActionParamNode::toString() {
    return param_->toString();
}

ActionParamsNode::ActionParamsNode() {
    nodeType_ = typeid(*this).name();
}

ActionParamsNode* ActionParamsNode::deepCopy() {
    auto newNode = new ActionParamsNode();
    for (auto ap : *list_) {
        auto newParam = ap->deepCopy();
        newNode->push_back(newParam);
    }

    return newNode;
}

string ActionParamsNode::toString() {
    bool first = true;
    ostringstream oss;
    for (auto ap : *list_) {
        if (first) {
            first = false;
        } else {
            oss << ", ";
        }
        oss << ap->toString();
    }
    return oss.str();
}

ActionStmtNode::ActionStmtNode(AstNode* name1, AstNode* args, ActionStmtType type, AstNode* name2, AstNode* index) {
    nodeType_ = typeid(*this).name();
    name1_ = dynamic_cast<NameNode*>(name1);
    name1_ -> parent_ = this;
    if (name2 != NULL) {
        name2_ = dynamic_cast<NameNode*>(name2);    
        name2_ -> parent_ = this;
    } else {
        name2_ = NULL;
    }
    
    if (args) {
        args_ = dynamic_cast<ArgsNode*>(args);
        args_->parent_ = this;
    }

    index_ = dynamic_cast<IntegerNode*>(index);
    type_ = type;
}

ActionStmtNode* ActionStmtNode::deepCopy() {
    return new ActionStmtNode(name1_, args_->deepCopy(), type_, name2_, index_);
}

void ActionStmtNode::evaluate(map<string, int> validHeaders) {
// void ActionStmtNode::evaluate(map<string, uint64_t> metadataInfo) {
    if (type_ == ActionStmtType::PROG_EXEC) {
        return;
    }
    string actionCommand = name1_ -> toString();
    if (actionCommand == "add_header") {
        validHeaders[actionCommand] = 1;
    } else if (actionCommand == "remove_header") {
        validHeaders[actionCommand] = 0;
    } else if (actionCommand == "pop") {
        validHeaders[actionCommand] -= stoi(args_ -> list_ -> at(1) -> toString());
    } else if (actionCommand == "push") {
        validHeaders[actionCommand] += stoi(args_ -> list_ -> at(1) -> toString());
    }

}

string ActionStmtNode::toString() {
    ostringstream oss;
    // cout << "In ActionStmtNode toString" << endl;
    if (removed_) {
        return "";
    }
    // cout << "not removed" << endl;
    if (type_ == ActionStmtType::NAME_ARGLIST) {
        // cout << "in first if" << endl;
        oss << "    "
            << name1_->toString()
            << "("
            << args_->toString()
            << ");\n";
    } else if (type_ == ActionStmtType::PROG_EXEC) {
        // cout << "in second if" << endl;
        oss << "    "
            << name1_->toString()
            << " . "
            << name2_->toString()
            << " ( "
            << args_->toString()
            << " );\n";
    } else {
        // cout << "in else" << endl;
        oss << "parsing error!\n";
    }
    // cout << "in return" << endl;
    return oss.str();
}

ActionStmtsNode::ActionStmtsNode() {
    nodeType_ = typeid(*this).name();
}

ActionStmtsNode* ActionStmtsNode::deepCopy() {
    auto newNode = new ActionStmtsNode();
    for (auto as : *list_) {
        auto newStmt = as->deepCopy();
        newNode->push_back(newStmt);
    }

    return newNode;
}

void ActionStmtsNode::evaluate(map<string, int> validHeaders) {
// void ActionStmtsNode::evaluate(map<string, uint64_t> metadataInfo) {
    for (auto as : *list_) {
        as -> evaluate(validHeaders);
    }
}

string ActionStmtsNode::toString() {
    ostringstream oss;
    for (auto as : *list_) {
        oss << as->toString();
    }
    return oss.str();
}

ActionNode::ActionNode(AstNode* name, AstNode* params, AstNode* stmts) {
    nodeType_ = typeid(*this).name();
    name_ = dynamic_cast<NameNode*>(name);
    name_->parent_ = this;
    params_ = dynamic_cast<ActionParamsNode*>(params);
    params_->parent_ = this;
    stmts_ = dynamic_cast<ActionStmtsNode*>(stmts);
    stmts_->parent_ = this;
}

ActionNode* ActionNode::duplicateAction(const string& name) {
    auto newNode = new ActionNode(new NameNode(new string(name)),
                                  params_->deepCopy(), stmts_->deepCopy());
    return newNode;
}

void ActionNode::evaluate(map<string, int> validHeaders) {
// void ActionNode::evaluate(map<string, uint64_t> metadataInfo) {
    stmts_ -> evaluate(validHeaders);
}

string ActionNode::toString() {
    if (removed_) {
        return "";
    }
    ostringstream oss;
    oss << "action " << name_->toString()
        << "(" << params_->toString() << ") {\n"
        << stmts_->toString()
        << "}\n";
    return oss.str();
}


FieldNode::FieldNode(AstNode* headerName, AstNode* fieldName, AstNode* index) {
    nodeType_ = typeid(*this).name();

    headerName_ = dynamic_cast<NameNode*>(headerName);
    headerName_ -> parent_ = this;

    fieldName_ = dynamic_cast<NameNode*>(fieldName);
    fieldName_ -> parent_ = this;

    if (index != NULL) {
        index_ = dynamic_cast<IntegerNode*>(index);
        index_ -> parent_ = NULL;
    } else {
        index_ = NULL;
    }
}

FieldNode* FieldNode::MakeCopy() {
    PRINT_VERBOSE("Start FieldNode::MakeCopy\n");
    NameNode* newHeaderName;
    if (((headerName_ -> toString()) == "latest") || ((headerName_ -> toString()) == "ig_intr_md")) {
        newHeaderName = new NameNode(new string(headerName_ -> toString()));
    } else {
        // cout << "Fieldnode: " << headerName_ -> toString() << endl;
        newHeaderName = new NameNode(new string(headerName_ -> toString() + "_clone"));
    }
    PRINT_VERBOSE("newHeaderName->toString(): %s\n", newHeaderName->toString().c_str());
    PRINT_VERBOSE("Original fieldName_->toString(): %s\n", fieldName_->toString().c_str());
    NameNode* newFieldName = new NameNode(new string(fieldName_ -> toString()));
    PRINT_VERBOSE("newFieldName->toString(): %s\n", newFieldName->toString().c_str());
    IntegerNode* newIndex;

    if (this -> index_ == NULL) {
        newIndex = NULL;
    } else {
        newIndex = new IntegerNode(new string(this -> index_ -> toString()));
    }
    FieldNode* newNode = new FieldNode(newHeaderName, newFieldName, newIndex);
    PRINT_VERBOSE("DONE FieldNode::MakeCopy\n");
    return newNode;
}

string FieldNode::toString() {
    ostringstream oss;
    oss << headerName_->toString();
    if (index_ != NULL) {
        oss << "[" << index_ -> toString() << "]";
    }
    oss << "."
        << fieldName_->toString();
    return oss.str();
}
