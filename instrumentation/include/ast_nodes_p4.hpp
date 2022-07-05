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

#ifndef AST_NODES_P4_H
#define AST_NODES_P4_H

#include "ast_nodes.hpp"
#include<map>
#include<set>

template<class T>
class ListNode : public AstNode {
public:
    ListNode() {
        list_ = new std::vector<T*>();
    }
    void push_back(T* node) {
        list_->push_back(node);
        node->parent_ = this;
    }

    void insert(int index, T* node) {
        list_ -> insert((list_ -> begin()) +  index, node);
        node -> parent_ = this;
    }
    std::vector<T*>* list_;
};

// An identifier, word, int, or special char
class BodyWordNode : public AstNode {
public:
    enum WordType { VARREF, NAME, FIELD, STRING, INTEGER, SPECIAL, HEX };

    BodyWordNode(WordType wordType, AstNode* contents);
    BodyWordNode* deepCopy();
    std::string toString();

    const WordType wordType_;
    AstNode* contents_;
};

class BodyNode : public AstNode {
public:
    BodyNode(AstNode* bodyOuter, AstNode* bodyInner, AstNode* str);
    std::string toString();

    BodyNode* bodyOuter_;
    BodyNode* bodyInner_;
    BodyWordNode* str_;
};

class HeaderExtractNode : public AstNode {
public:
    HeaderExtractNode(AstNode* _name, AstNode* _index, bool _next);
    NameNode* name_;
    IntegerNode* index_;
    bool next_;
    HeaderExtractNode* MakeCopy();
    std::string toString();
};

class ExtractStatementNode : public AstNode {
public:
    ExtractStatementNode(AstNode* headerName);
    HeaderExtractNode* headerExtract_;
    ExtractStatementNode* MakeCopy();
    std::string toString();
};

class FieldNode : public AstNode {
public: 
    FieldNode(AstNode* headerName, AstNode* fieldName, AstNode* index);
    std::string toString();
    FieldNode* MakeCopy();
    NameNode* headerName_;
    NameNode* fieldName_;
    IntegerNode* index_;
};

typedef enum {FALSE, TRUE, UNKNOWN, ARITH} ExprReturnMode;

struct ExprReturnType
{
    ExprReturnMode mode;
    uint64_t arithValue;
};

class ExprNode : public AstNode {
public:
    ExprNode(AstNode* first, AstNode* second, string* operatorIn, bool valid);
    ExprNode(AstNode* first, AstNode* second, string* operatorIn);
    // ExprReturnType evaluate();
    AstNode* first_;
    AstNode* second_;
    string* operator_;
    ExprNode* MakeCopy();
    bool valid_;
    vector<string> GetFields(vector<string> currentFields);
    std::map<string, vector<string> > GetFixedValues(std::map<string, vector<string> > fixedValuesInCondtion);
    ExprReturnType evaluate(map<string, int> validHeaders, string latest = "");
    std::string toString();
};

class AssertNode : public AstNode {
public:
    AssertNode(AstNode* expr, int assertNum);
    // void setAssertNum(int assertNum);
    std::string toString();
    ExprNode* expr_;
    int assertNum_;
};


class SetStatementNode : public AstNode {
public:
    SetStatementNode(AstNode* metadata_field, AstNode* packet_field);
    SetStatementNode* MakeCopy();
    FieldNode* metadata_field_;
    ExprNode* packet_field_;
    std::string toString();
};


class ReturnValueNode : public AstNode {
public:
    ReturnValueNode(AstNode* retValue);
    NameNode* retValue_;
    ReturnValueNode* MakeCopy();
    std::string toString();
};

class CaseEntryNode : public AstNode {
public:
    CaseEntryNode(AstNode* matchValue, AstNode* returnValue);
    BodyWordNode* matchValue_;
    NameNode* retValue_;
    CaseEntryNode* MakeCopy();
    std::string toString();
};

class CaseEntryList : public ListNode<CaseEntryNode> {
public:
    CaseEntryList();
    CaseEntryList* MakeCopy();
    std::string toString();
};

class CurrentNode : public AstNode {
public:
    CurrentNode(AstNode* _start, AstNode* _end);
    IntegerNode* start_;
    IntegerNode* end_;
    CurrentNode* MakeCopy();
    std::string toString();
};

class FieldOrDataNode : public AstNode {
public:
    FieldOrDataNode(AstNode* _field, AstNode* _current);
    FieldNode* field_;
    CurrentNode* current_;
    FieldOrDataNode* MakeCopy();
    std:: string toString();
};

class SelectExpList : public ListNode<FieldOrDataNode> {
public:
    SelectExpList();
    SelectExpList* MakeCopy();
    std::string toString();
};


class ExtractOrSetNode : public AstNode {
public:
    ExtractOrSetNode(AstNode* extStatement, AstNode* setStatement);
    ExtractStatementNode* extStatement_;
    SetStatementNode* setStatement_;
    ExtractOrSetNode* MakeCopy();
    std::string toString();
};

class ExtractOrSetList : public ListNode<ExtractOrSetNode> {
public:
    ExtractOrSetList();
    ExtractOrSetList* MakeCopy();
    std::string toString();
};

class ReturnStatementNode : public AstNode {
public:
    ReturnStatementNode(AstNode* _single, AstNode* _selectList, AstNode* _caseList);
    ReturnValueNode* single;
    SelectExpList* selectList;
    CaseEntryList* caseList;
    ReturnStatementNode* MakeCopy();
    std::string toString();
};


class P4ParserFunctionBody : public AstNode {
public:
    P4ParserFunctionBody(AstNode* extSetStatements, AstNode* retStatement);
    ExtractOrSetList* extSetList_;
    ReturnStatementNode* retStatement_;
    P4ParserFunctionBody* MakeCopy();
    std::string toString();
};

class P4ParserNode: public AstNode {
public:
    P4ParserNode(AstNode* name, AstNode* body);
    std::string toString();
    std::string prefixStr_ = "";
    bool redirect_to_clone_ = false;
    std::string parser_clone_all_ {""};
    void Clone(std::string parser_clone);
    AstNode* name_;
    P4ParserNode* MakeCopy();
    P4ParserFunctionBody* body_;
};


class KeywordNode : public AstNode {
public: 
    KeywordNode(std::string* word);
    std::string toString();

    std::string* word_;
};

class P4RegisterNode : public AstNode {
public:
    P4RegisterNode(AstNode* name, AstNode* body);
    std::string toString();
    
    AstNode* name_;
    BodyNode* body_;
    
    int width_;
    int instanceCount_;    
};

class ControlStatement : public AstNode {
public:
    enum StmtType { TABLECALL, TABLESELECT, IFELSE, CONTROL, ASSERT};
    ControlStatement(AstNode* stmt, StmtType stmtType);
    std::string toString();
    AstNode* stmt_;
    const StmtType stmtType_;
};

class ControlStatements : public ListNode<ControlStatement> {
public:
    ControlStatements();
    std::string toString();
};

class P4ControlBlock: public AstNode {
public:
    P4ControlBlock(AstNode* controlStatements);
    ControlStatements* controlStatements_;
    std::string suffixStr_ = "";
    std::string toString();
    void append(std::string str);
    bool clear_ = false;
    void clear();
};

class P4ControlNode: public AstNode {
public:
    P4ControlNode(AstNode* name, AstNode* controlBlock);
    std::string toString();
    // void clear();
    void RenamePopularActions(std::vector<std::string> popularactions);
    // void append(std::string str);
    std::string prefixStr_ = "";
    // std::string suffixStr_ = "";
    AstNode* name_;
    P4ControlBlock* controlBlock_;
    // bool clear_ = false;
    bool rename_popular_actions_ = false;
    std::vector<std::string> popular_actions_;
    std::string normalPrint();
};


class ApplyTableCall : public AstNode {
public:
    ApplyTableCall(AstNode* name);
    NameNode* name_;
    std::string toString();
};

class HitOrMissCase : public AstNode {
public:
    enum StmtType { HIT, MISS };
    HitOrMissCase(AstNode* controlBlock, StmtType stmtType);
    std::string toString();
    P4ControlBlock* controlBlock_;
    const StmtType stmtType_;
};

class HitOrMissCases : public ListNode<HitOrMissCase> {
public:
    HitOrMissCases();
    std::string toString();
};


class ActionCase : public AstNode {
public:
    ActionCase(AstNode* name, AstNode* controlBlock);
    NameNode* name_;
    P4ControlBlock* controlBlock_;
    std::string toString();
};

class ActionCases : public ListNode<ActionCase> {
public:
    ActionCases();
    std::string toString();
};

class CaseList : public AstNode {
public:
    CaseList(AstNode* actionCases, AstNode* hitOrMissCases);
    ActionCases* actionCases_;
    HitOrMissCases* hitOrMissCases_;
    std::string toString();
};

class ApplyAndSelectBlock : public AstNode {
public:
    ApplyAndSelectBlock(AstNode* name, AstNode* caseList);
    NameNode* name_;
    CaseList* caseList_;
    std::string toString();
};

class IfElseStatement;

class ElseBlock : public AstNode {
public:
    ElseBlock(AstNode* controlBlock, AstNode* ifElseStatement);
    P4ControlBlock* controlBlock_;
    IfElseStatement* ifElseStatement_;
    std::string toString();
};

class IfElseStatement : public AstNode {
public:
    IfElseStatement(AstNode* body, AstNode* controlBlock, AstNode* elseBlock);
    ExprNode* condition_;
    P4ControlBlock* controlBlock_;
    ElseBlock* elseBlock_;
    std::string toString();
};

class P4ExprNode : public AstNode {
public:
    P4ExprNode(AstNode* keyword, AstNode* name1, AstNode* name2,
               AstNode* opts, AstNode* body);
    std::string toString();

    KeywordNode* keyword_;
    NameNode* name1_;
    AstNode* name2_;
    AstNode* opts_;
    BodyNode* body_;
};

class P4PragmaNode : public AstNode {
public:
    P4PragmaNode(string* word);
    std::string toString();
    string* word_;
};

class OptsNode : public AstNode {
public:
    OptsNode(AstNode* nameList);
    std::string toString();

    AstNode* nameList_;
};

class NameListNode : public AstNode {
public: 
    NameListNode(AstNode* nameList, AstNode* name);
    std::string toString();

    AstNode *nameList_, *name_;
};

class TableReadStmtNode : public AstNode {
public:
    enum MatchType { EXACT, TERNARY, LPM, RANGE, VALID };

    TableReadStmtNode(MatchType matchType, AstNode* field);
    std::string toString();

    MatchType matchType_;
    AstNode* field_;
};

class TableReadStmtsNode : public ListNode<TableReadStmtNode> {
public:
    TableReadStmtsNode();
    std::string toString();
};

class PragmaNode : public AstNode {
public:
    PragmaNode(std::string name);
    std::string toString();

    std::string name_;
};

class PragmasNode: public ListNode<PragmaNode> {
public:
    PragmasNode();
    std::string toString();
};

class TableActionStmtNode : public AstNode {
public:
    TableActionStmtNode(AstNode* name);
    std::string toString();

    NameNode* name_;
};

class TableActionStmtsNode : public ListNode<TableActionStmtNode> {
public:
    TableActionStmtsNode();
    std::string toString();
};

class ActionProfileSpecificationNode : public AstNode {
public:
    ActionProfileSpecificationNode(AstNode* name);
    std::string toString();

    NameNode* name_;
};

// A P4 table
class TableNode : public AstNode {
public:
    TableNode(AstNode* name, AstNode* reads, AstNode* actions,
              std::string options, AstNode* pragma, AstNode* actionprofile);
    std::string toString();
//    void transformPragma();

    NameNode* name_;
    TableReadStmtsNode* reads_;
    ActionProfileSpecificationNode* actionprofile_;
    TableActionStmtsNode* actions_;
    // size, default action, etc.
    vector<std::string> options_;

    PragmasNode* pragma_;
    bool pragmaTransformed_;

    // for malleable table, parser gives TableNode as well 
    // need to keep this meta data to indicate if the table node is variable
    bool isMalleable_;
};

class ALUStatementNode : public AstNode {
public:
    ALUStatementNode(AstNode* command, AstNode* expr);
    std::string toString();

    NameNode* command_;
    ExprNode* expr_;
};


class ALUStatmentList : public ListNode<ALUStatementNode> {
public:
    ALUStatmentList();
    std::string toString();
};


class BlackboxNode : public AstNode {
public:
    BlackboxNode(AstNode* name, AstNode* alu_statements);
    std::string toString();
    NameNode* name_;
    ALUStatmentList* alu_statements_;
};

class FieldDecNode : public AstNode {
public:
    FieldDecNode(AstNode* name, AstNode* size, AstNode* information);
    std::string toString();

    NameNode* name_;
    IntegerNode* size_;
    BodyNode* information_;
};

class FieldDecsNode : public ListNode<FieldDecNode> {
public:
    FieldDecsNode();
    std::string toString();
};

class HeaderTypeDeclarationNode : public AstNode {
public:
    HeaderTypeDeclarationNode(AstNode* name, AstNode* field_decs,
                              AstNode* other_stmts);
    std::string toString();

    NameNode* name_;
    FieldDecsNode* field_decs_;
    AstNode* other_stmts_;
};

class HeaderInstanceNode : public AstNode {
public:
    HeaderInstanceNode(AstNode* type, AstNode* name, AstNode* numHeaders);
    std::string toString();

    NameNode* type_;
    NameNode* name_;
    IntegerNode* numHeaders_;
};

class SelectionType : public AstNode {
public:
    SelectionType(AstNode* name);
    std::string toString();

    NameNode* name_;
};


class SelectionMode : public AstNode {
public:
    SelectionMode(AstNode* name);
    std::string toString();

    NameNode* name_;
};


class SelectionKey : public AstNode {
public:
    SelectionKey(AstNode* name);
    std::string toString();

    NameNode* name_;
};

class ActionSelectorDecl : public AstNode {
public:
    ActionSelectorDecl(AstNode* name, AstNode* selectionKey, AstNode* selectionMode, AstNode* selectionType);
    std::string toString();

    AstNode* name_;
    SelectionKey* selectionKey_;
    SelectionMode* selectionMode_;
    SelectionType* selectionType_;
};

class ActionSelector : public AstNode {
public:
    ActionSelector(AstNode* name);
    std::string toString();

    NameNode* name_;
};

class ActionProfile : public AstNode {
public:
    ActionProfile(AstNode* name, AstNode* tableActions, AstNode* body, AstNode* actionSelector);
    std::string toString();

    NameNode* name_;
    TableActionStmtsNode* tableActions_;
    BodyNode* body_;
    ActionSelector* actionSelector_;
};

class MetadataInstanceNode : public AstNode {
public:
    MetadataInstanceNode(AstNode* type, AstNode* name);
    std::string toString();

    std::string MakeCopyToString();    

    NameNode* type_;
    NameNode* name_;
};

class ArgsNode : public ListNode<BodyWordNode> {
public:
    ArgsNode();
    ArgsNode* deepCopy();
    std::string toString();
};

class ActionParamNode : public AstNode {
public:
    ActionParamNode(AstNode* param);
    ActionParamNode* deepCopy();
    std::string toString();

    AstNode* param_;
};

class ActionParamsNode : public ListNode<ActionParamNode> {
public:
    ActionParamsNode();
    ActionParamsNode* deepCopy();
    std::string toString();
};

class ActionStmtNode : public AstNode {
public:
    enum ActionStmtType { NAME_ARGLIST, PROG_EXEC };

    ActionStmtNode(AstNode* name1, AstNode* args, ActionStmtType type, AstNode* name2, AstNode* index);
    ActionStmtNode* deepCopy();
    std::string toString();

    ActionStmtType type_;
    // void evaluate(map<string, uint64_t> metadataInfo);
    void evaluate(map<string, int> validHeaders);
    NameNode* name1_;
    NameNode* name2_;
    IntegerNode* index_;
    ArgsNode* args_;
};

class ActionStmtsNode : public ListNode<ActionStmtNode> {
public:
    ActionStmtsNode();
    ActionStmtsNode* deepCopy();
    std::string toString();
    // void evaluate(map<string, uint64_t> metadataInfo);
    void evaluate(map<string, int> validHeaders);
};

class ActionNode : public AstNode {
public:
    ActionNode(AstNode* name, AstNode* params, AstNode* stmts);
    ActionNode* duplicateAction(const std::string& name);
    std::string toString();
    // void evaluate(map<string, uint64_t> metadataInfo);
    void evaluate(map<string, int> validHeaders);
    NameNode* name_;
    ActionParamsNode* params_;
    ActionStmtsNode* stmts_;
};

class FieldsNode : public ListNode<FieldNode> {
public :
    FieldsNode();
    std::string toString();
};

#endif
