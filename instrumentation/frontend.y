// C declarations
%{
#define YYDEBUG 1
#include <string>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <typeinfo>

#include "include/ast_nodes.hpp"
#include "include/ast_nodes_p4.hpp"
#include "include/helper.hpp"
#include "include/modifier.h"
#include "include/modifier_ut.h"
#include "include/modifier_dt.h"

// Only support tofino target
int with_tofino=1;
int done_init=0;

// Bison stuff to read parsed tokens from file
extern int yylex();
extern int yyparse();
extern FILE* yyin;
extern int yylineno;
extern char* yytext;
extern void yyrestart(FILE * input_file);
void yyerror(const char* s);

using namespace std;

char* in_fn = NULL;
char* out_fn_base = NULL;
char* target = NULL;
std::string p4_out_ut, p4_out_dt;
FILE* in_file;
char* rules = NULL;
std::string p4_out_fn, ut_rules_out_fn;
int num_assertions;

// From: https://stackoverflow.com/questions/865668/how-to-parse-command-line-arguments-in-c
char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
    return std::find(begin, end, option) != end;
}

void parseArgs(int argc, char* argv[]){ 
    in_fn = getCmdOption(argv, argv+argc, "-i");
    out_fn_base = getCmdOption(argv, argv+argc, "-o");
    target = getCmdOption(argv, argv+argc, "-t");
    rules = getCmdOption(argv, argv+argc, "-r"); 
    if ((in_fn == NULL) || (out_fn_base == NULL) || (target == NULL)){
        cout << "expected arguments: "
             << argv[0]
             << " -i <input P4 filename> -o <output filename base> -t <target hw/sim>"
             << endl;
        exit(0);
    }

    in_file = fopen(in_fn, "r");
    if (in_file == 0) {
        // Exit bison program
        PANIC("Input P4R file not found");
    }
    // Name of output p4 file
    p4_out_ut = string(string(out_fn_base) + string("_ut_") + string(target) + string(".p4"));
    p4_out_dt = string(string(out_fn_base) + string("_dt_") + string(target) + string(".p4"));
    ut_rules_out_fn = string(string(out_fn_base) + "_ut_" + string(target) + "_rules.txt");
}

std::vector<AstNode*> node_array;
AstNode* root;
%}

// Types of tokens. Union is the bridge b/w lex and y code.
%union {
    AstNode* aval;
    std::string* pval;
    char* sval;
}

// One could extend tofino-specifc P4 syntax to support other variants by branching
%token START_TOFINO START_BMV2

// Parsed tokens
%token L_BRACE "{"
%token R_BRACE "}"
%token L_PAREN "("
%token R_PAREN ")"
%token SEMICOLON ";"
%token COLON ":"
%token COMMA ","
%token PERIOD "."
%token DOLLAR "$"
%token L_BRACKET "["
%token R_BRACKET "]"
%token SLASH "/"
%token PLUS "+"
%token MINUS "-"
%token GTE ">="
%token GT ">"
%token LT "<"
%token EQU "=="
%token NEQU "!="
%token AND "&&"

// Parsed keywords - some are coming from the lexer
%token TABLE
%token VALUE
%token FIELD
%token WIDTH
%token INIT
%token ALTS
%token READS
%token EXACT
%token TERNARY
%token LPM
%token RANGE
%token ACTIONS
%token HEADER_TYPE
%token HEADER
%token METADATA
%token FIELDS
%token ACTION
%token RETURNS
%token SELECT
%token SET_METADATA
%token NEXT
%token CURRENT
%token VALID
%token APPLY
%token IF
%token ELSE
%token DEFAULT
%token ACTION_SELECTOR
%token DYNAMIC_ACTION_SELECTION
%token ACTION_PROFILE
//%token ALGORITHM
%token SELECTION_KEY
%token SELECTION_MODE
%token SELECTION_TYPE

%token BLACKBOX

%token P4R_INIT_BLOCK

%token P4R_REACTION
%token REGISTER
%token PARSER
%token CONTROL
%token EXTRACT
%token REACTION_ARG_REG
%token REACTION_ARG_ING
%token REACTION_ARG_EGR
//%nonassoc STR
%left R_PAREN
%left OR
%left AND
%left EQU NEQU 
%left GTE GT LT LTE
%left PLUS MINUS
%right VALID
%left L_PAREN

// Parsed words
// Declaring the type of token
%token <sval> INCLUDE
%token <sval> PRAGMA
//%token <sval> PRAGMA_NO_DEAD_CODE
//%token <sval> PRAGMA_PLACEMENT
//%token <sval> PRAGMA_METADATA_OVERLAY
%token <sval> IDENTIFIER
%token <sval> STRING
//%token <sval> STR
%token <sval> INTEGER
%token <sval> HEX
%token <sval> ASSERT_KEY


%type <aval> rootTofino;
%type <aval> rootBmv2;

// Nonterminals
%type <aval> inputTofino
%type <aval> inputBmv2
%type <aval> p4ExprTofino
%type <aval> p4ExprBmv2
//%type <aval> p4rExpr
%type <aval> include
%type <aval> nameList
%type <aval> keyWord
//%type <aval> pragmaStatement
//%type <aval> pragmaKeyword
%type <aval> body
%type <aval> opts
%type <aval> field
%type <aval> bodyWord

%type <aval> registerDecl
%type <aval> parserDecl

%type <aval> pragmas 

%type <aval> tableDecl
%type <aval> tableReads
%type <aval> tableReadStmt
%type <aval> tableReadStmts
%type <aval> tableActions
%type <aval> tableActionStmt
%type <aval> tableActionStmts

%type <aval> aluBlock
%type <aval> aluStatement
%type <aval> aluStatementList

%type <aval> headerTypeDeclaration
%type <aval> headerDecBody
%type <aval> headerInstance
%type <aval> metadataInstance
%type <aval> fieldDec
%type <aval> fieldDecList

// Parser additions
%type <aval> parserFunctionBody
%type <aval> extractOrSetStatements
%type <aval> extractOrSetStatement
%type <aval> extractStatement
%type <aval> setStatement
%type <aval> returnStatement
%type <aval> caseEntryList
%type <aval> caseEntry
%type <aval> selectExpList
%type <aval> returnValue
%type <aval> fieldOrDataRef
%type <aval> headerExtractRef
%type <aval> expr

%type <aval> actionFunctionDeclaration
%type <aval> actionParamList
%type <aval> actionParam
%type <aval> actionStatements
%type <aval> actionStatement
%type <aval> actionProfileSpecification
%type <aval> actionProfileDecl
%type <aval> actionSelector
%type <aval> actionSelectorDecl
%type <aval> selectionKey
%type <aval> selectionMode
%type <aval> selectionType
%type <aval> assertDecl
%type <aval> argList
%type <aval> arg

// Control additions
%type <aval> controlDecl
%type <aval> controlBlock
%type <aval> controlStatements
%type <aval> controlStatement
%type <aval> applyTableCall
%type <aval> applyAndSelectBlock
%type <aval> caseList
%type <aval> actionCases
%type <aval> hitOrMissCases
%type <aval> ifElseStatement
%type <aval> elseBlock

// Leaves
%type <aval> name
%type <aval> specialChar
%type <aval> integer
%type <aval> hex

// Define start state
%start root


%%
/*=====================================
=               GRAMMAR               =
=====================================*/

root:
    START_TOFINO rootTofino
    | START_BMV2 rootBmv2
    ;


rootTofino :
    inputTofino {
        root = $1;
    }
;

rootBmv2 :
    inputBmv2 {
        root = $1;
    }   
;

inputTofino :
    /* epsilon */ {
        $$=NULL;
    }
    | inputTofino include {
        AstNode* rv = new InputNode($1, $2);
        node_array.push_back(rv);
        $$=rv;
    }
    | inputTofino p4ExprTofino  {
        AstNode* rv = new InputNode($1, $2);
        node_array.push_back(rv);
        $$=rv;
        PRINT_VERBOSE("----- parsed P4 Expr ------ \n");
        PRINT_VERBOSE("%s\n", $2 -> toString().c_str());
        PRINT_VERBOSE("---------------------------\n");
    }
;

inputBmv2 :
    /* epsilon */ {
        $$=NULL;
    }
    | inputBmv2 include {
        AstNode* rv = new InputNode($1, $2);
        node_array.push_back(rv);
        $$=rv;
    }
    | inputBmv2 p4ExprBmv2  {
        AstNode* rv = new InputNode($1, $2);
        node_array.push_back(rv);
        $$=rv;
        PRINT_VERBOSE("----- parsed P4 Expr ------ \n");
        PRINT_VERBOSE("%s\n", $2 -> toString().c_str());
        PRINT_VERBOSE("---------------------------\n");
    }
;

// Include statements
include :
    INCLUDE {
        string* strVal = new string(string($1));
        AstNode* rv = new IncludeNode(strVal, IncludeNode::P4);
        node_array.push_back(rv);
        $$=rv;
        free($1);
    }
;


/*=====================================
=            P4 expressions           =
=====================================*/

registerDecl :
    REGISTER name "{" body "}" {
        BodyNode* bv = dynamic_cast<BodyNode*>($4);
        bv -> indent_ = (bv -> indent_) + 1;
        AstNode* rv = new P4RegisterNode($2, bv);
        node_array.push_back(rv);
        $$=rv;    
    }
;

parserDecl :
    PARSER name "{" parserFunctionBody "}" {
        //cout << "Reaching here" << endl;
        AstNode* rv = new P4ParserNode($2, $4);
        //cout << "Reaching here2" << endl;
        node_array.push_back(rv);
        //cout << "Reaching here3" << endl;
        $$=rv;
        //cout << "Reaching here3.5" << endl;
    }
;

parserFunctionBody :
    extractOrSetStatements returnStatement {
        //cout << "Reaching here 4" << endl;
        AstNode* rv = new P4ParserFunctionBody($1, $2);
        //cout << "Reaching here 5" << endl;
        $$=rv;
        //cout << "Reaching here 5.5" << endl;
    }
;

extractOrSetStatements :
    /* empty */ {
        //cout << "Reaching here 8" << endl;
        $$ = new ExtractOrSetList();
        //cout << "Reaching here 8.5" << endl;
    }
    | extractOrSetStatements extractOrSetStatement {
        //cout << "Reaching here 9" << endl;
        ExtractOrSetList* rv = dynamic_cast<ExtractOrSetList*>($1);
        //cout << "Reaching here 9.1" << endl;
        ExtractOrSetNode* trs = dynamic_cast<ExtractOrSetNode*>($2);
        //cout << "Reaching here 9.2" << endl;
        rv -> push_back(trs);
        //cout << "Reaching here 9.3" << endl;
        $$ = rv;
        //cout << "Reaching here 9.4" << endl;
    }
;

extractOrSetStatement : 
    extractStatement {
        //cout << "Reaching here 9.5" << endl;
        $$ = new ExtractOrSetNode($1, NULL);
        //cout << "Reaching here 9.6" << endl;
    }
    | setStatement {
        //cout << "Reaching here 9.7" << endl;
        $$ = new ExtractOrSetNode(NULL, $1);
        //cout << "Reaching here 9.8" << endl;
    }
;

extractStatement :
    EXTRACT L_PAREN headerExtractRef R_PAREN SEMICOLON {
        $$ = new ExtractStatementNode($3);
    }
;

headerExtractRef :
    name {
        $$ = new HeaderExtractNode($1, NULL, false);
    }
    | name L_BRACKET NEXT R_BRACKET {
        $$ = new HeaderExtractNode($1, NULL, true);
    }
    | name L_BRACKET integer R_BRACKET {
        $$ = new HeaderExtractNode($1, $3, false);
    }
;


//headerExtractIndex :
//    const_value {

//    }
//    | next {

//    }
//;

setStatement :
    SET_METADATA L_PAREN field COMMA expr R_PAREN SEMICOLON {
        $$ = new SetStatementNode($3, $5);
    }
;


expr :
    expr PLUS expr {
        $$ = new ExprNode($1, $3, new string("+"));
    }
    | expr MINUS expr {
        $$ = new ExprNode($1, $3, new string("-"));
    }
    | expr GTE expr {
        $$ = new ExprNode($1, $3, new string(">="));
    }
    | expr LT expr {
        $$ = new ExprNode($1, $3, new string("<"));
    }
    | expr LTE expr {
        $$ = new ExprNode($1, $3, new string("<="));
    }
    | expr GT expr {
        $$ = new ExprNode($1, $3, new string(">"));
    }
    | expr EQU expr {
        $$ = new ExprNode($1, $3, new string("=="));
    }
    | expr NEQU expr {
        $$ = new ExprNode($1, $3, new string("!="));
    }
    | expr AND expr {
        $$ = new ExprNode($1, $3, new string("&&"));
    }
    | expr OR expr {
        $$ = new ExprNode($1, $3, new string("||"));
    }
    | expr expr {
        $$ = new ExprNode($1, $2, new string(" "));
    }
    | field {
        //cout << "field " << $1 -> toString() << endl;
        $$ = new ExprNode($1, NULL, NULL);
    }
    | integer {
        //cout << "integer " << $1 -> toString() << endl;
        $$ = new ExprNode($1, NULL, NULL);    
    }
    | hex {
        //cout << "hex " << $1 -> toString() << endl;
        $$ = new ExprNode($1, NULL, NULL);    
    }
    | name {
        //cout << "name " << $1 -> toString() << endl;
        $$ = new ExprNode($1, NULL, NULL);
    }
    | VALID L_PAREN name R_PAREN {
        //cout << "valid " << $3 -> toString() << endl;
        $$ = new ExprNode($3, NULL, NULL, true);
    }
;


returnStatement :
    returnValue SEMICOLON {
        //cout << "Reaching here 11 " << endl;
        $$ = new ReturnStatementNode($1, NULL, NULL);
        //cout << "Reaching here 11.5" << endl;
    }
    | RETURNS SELECT L_PAREN selectExpList R_PAREN L_BRACE caseEntryList R_BRACE {
        //cout << "Reaching here 12" << endl;
        $$ = new ReturnStatementNode(NULL, $4, $7);
        //cout << "Reaching here 12.5" << endl;
    } 
;

returnValue :
    RETURNS name {
        //cout << "Reaching here 13" << endl;
        $$ = new ReturnValueNode($2);
        //cout << "Reaching here 14" << endl;
    }
;

caseEntryList :
    /* empty */ {
        $$ = new CaseEntryList();
    }
    | caseEntryList caseEntry {
        CaseEntryList* rv = dynamic_cast<CaseEntryList*>($1);
        CaseEntryNode* trs = dynamic_cast<CaseEntryNode*>($2);
        rv -> push_back(trs);
        $$ = rv;
    }
;


caseEntry :
    arg COLON name SEMICOLON {
        $$ = new CaseEntryNode($1, $3);
    }
    | DEFAULT COLON name SEMICOLON {
        $$ = new CaseEntryNode(NULL, $3);
    }
;


selectExpList :
    /* empty */ {
        $$ = new SelectExpList();
    }
    | selectExpList COMMA fieldOrDataRef {
        SelectExpList* rv = dynamic_cast<SelectExpList*>($1);
        FieldOrDataNode* trs = dynamic_cast<FieldOrDataNode*>($3);
        rv->push_back(trs);
        $$=rv;
    }
    | selectExpList fieldOrDataRef {
        SelectExpList* rv = dynamic_cast<SelectExpList*>($1);
        FieldOrDataNode* trs = dynamic_cast<FieldOrDataNode*>($2);
        rv->push_back(trs);
        $$=rv;
    }
;

fieldOrDataRef : 
    field {
        FieldOrDataNode* rv = new FieldOrDataNode($1, NULL);
        $$=rv; 
    }
    | CURRENT L_PAREN integer COMMA integer R_PAREN {
        CurrentNode* currentNode = new CurrentNode($3, $5);
        FieldOrDataNode* rv = new FieldOrDataNode(NULL, currentNode);
        $$=rv;
    }
;

aluBlock :
    // keyWord name name "{" body "}"
    BLACKBOX name name "{" aluStatementList "}" {
        AstNode* rv = new BlackboxNode($3, $5);
        //cout << "Blackbox created" << endl;
        node_array.push_back(rv);
        $$=rv;
    }
;

aluStatementList :
    /* empty */ {
        $$ = new ALUStatmentList();
    }
    | aluStatementList aluStatement {
        ALUStatmentList* rv = dynamic_cast<ALUStatmentList*>($1);
        ALUStatementNode* trs = dynamic_cast<ALUStatementNode*>($2);
        rv->push_back(trs);
        $$=rv;
    }
;

aluStatement :
    name ":" expr ";" {
        AstNode* rv = new ALUStatementNode($1, $3);
        $$ = rv;
    }
;

controlDecl :
    CONTROL name controlBlock {
        AstNode* rv = new P4ControlNode($2, $3);
        node_array.push_back(rv);
        $$=rv;
    }
;

controlBlock :
    L_BRACE controlStatements R_BRACE {
        AstNode* rv = new P4ControlBlock($2);
        $$=rv;
    }
;

controlStatements :
    /* empty */ {
        $$ = new ControlStatements();
    }
    | controlStatements controlStatement {
        ControlStatements* rv = dynamic_cast<ControlStatements*>($1);
        ControlStatement* trs = dynamic_cast<ControlStatement*>($2);
        rv->push_back(trs);
        $$=rv;
    } 
;

controlStatement :
    applyTableCall {
        ControlStatement* rv = new ControlStatement($1, ControlStatement::TABLECALL);
        $$=rv;
    }
    | applyAndSelectBlock {
        ControlStatement* rv = new ControlStatement($1, ControlStatement::TABLESELECT);
        $$=rv;
    } 
    | ifElseStatement {
        ControlStatement* rv = new ControlStatement($1, ControlStatement::IFELSE);
        $$=rv;
    }
    | name L_PAREN R_PAREN ";" {
        ControlStatement* rv = new ControlStatement($1, ControlStatement::CONTROL);
        $$=rv;
    }
    | assertDecl {
        ControlStatement* rv = new ControlStatement($1, ControlStatement::ASSERT);
        //cout << "set to assert" << endl;
        $$=rv;
    }
;

applyTableCall : 
    APPLY L_PAREN name R_PAREN ";" {
        ApplyTableCall* rv = new ApplyTableCall($3);
        $$ = rv;
    }
;

applyAndSelectBlock :
    APPLY L_PAREN name R_PAREN L_BRACE caseList R_BRACE {
        ApplyAndSelectBlock* rv = new ApplyAndSelectBlock($3, $6);
        $$=rv;
    }
    | APPLY L_PAREN name R_PAREN L_BRACE R_BRACE {
        ApplyAndSelectBlock* rv = new ApplyAndSelectBlock($3, NULL);
        $$=rv;
    }
;

caseList :
    actionCases {
        CaseList* rv = new CaseList($1, NULL);
        $$ = rv;
    }
    | hitOrMissCases {
        CaseList* rv = new CaseList(NULL, $1);
        $$ = rv;
    }
;

actionCases :
    name controlBlock {
        ActionCases* rv = new ActionCases();
        ActionCase* trs = new ActionCase($1, $2);
        rv -> push_back(trs);
        $$ = rv;
    }
    | DEFAULT controlBlock {
        ActionCases* rv = new ActionCases();
        ActionCase* trs = new ActionCase(NULL, $2);
        rv -> push_back(trs);
        $$ = rv;
    }
    | actionCases name controlBlock {
        ActionCases* rv = dynamic_cast<ActionCases*>($1);
        ActionCase* trs = new ActionCase($2, $3);
        rv -> push_back(trs);
        $$=rv;
    }
    | actionCases DEFAULT controlBlock {
        ActionCases* rv = dynamic_cast<ActionCases*>($1);
        ActionCase* trs = new ActionCase(NULL, $3);
        rv -> push_back(trs);
        $$=rv;
    }
;

hitOrMissCases :
    "hit" controlBlock {
        HitOrMissCases* rv = new HitOrMissCases();
        HitOrMissCase* trs = new HitOrMissCase($2, HitOrMissCase::HIT);
        rv -> push_back(trs);
        $$ = rv;
    } 
    | "miss" controlBlock {
        HitOrMissCases* rv = new HitOrMissCases();
        HitOrMissCase* trs = new HitOrMissCase($2, HitOrMissCase::MISS);
        rv -> push_back(trs);
        $$ = rv;
    }
    | hitOrMissCases "hit" controlBlock {
        HitOrMissCases* rv = dynamic_cast<HitOrMissCases*>($1);
        HitOrMissCase* trs = new HitOrMissCase($3, HitOrMissCase::HIT);
        rv -> push_back(trs);
        $$=rv;
    }
    | hitOrMissCases "miss" controlBlock {
        HitOrMissCases* rv = dynamic_cast<HitOrMissCases*>($1);
        HitOrMissCase* trs = new HitOrMissCase($3, HitOrMissCase::MISS);
        rv -> push_back(trs);
        $$=rv;
    }
;

ifElseStatement :
    IF L_PAREN expr R_PAREN controlBlock {
        IfElseStatement* rv = new IfElseStatement($3,$5,NULL);
        $$=rv;
    }
    | IF L_PAREN expr R_PAREN controlBlock elseBlock {
        IfElseStatement* rv = new IfElseStatement($3,$5,$6);
        $$=rv;
    }
;

elseBlock :
    ELSE controlBlock {
        ElseBlock* rv = new ElseBlock($2, NULL);
        $$=rv;
    }
    | ELSE ifElseStatement {
        ElseBlock* rv = new ElseBlock(NULL, $2);
        $$=rv;
    }
;


actionProfileSpecification :
    /* empty */ {
        ActionProfileSpecificationNode* rv = new ActionProfileSpecificationNode(NULL);
        $$=rv;
    }
    | ACTION_PROFILE ":" name ";" {
        ActionProfileSpecificationNode* rv = new ActionProfileSpecificationNode($3);
        $$=rv;
    }

tableDecl : 
    TABLE name "{" tableReads tableActions actionProfileSpecification body "}" {
        AstNode* rv;
        BodyNode* bv;
        if (bv = dynamic_cast<BodyNode*>($7)) {
            bv -> indent_ = (bv -> indent_) + 1;
            rv = new TableNode($2, $4, $5,
                                bv->toString(), NULL, $6);
        } else {
            rv = new TableNode($2, $4, $5,
                                $7->toString(), NULL, $6);
        }
        node_array.push_back(rv);
        $$=rv;
    }
    | TABLE name "{" tableActions actionProfileSpecification body "}" {
        // Reads are optional
        AstNode* rv;
        BodyNode* bv;
        if (bv = dynamic_cast<BodyNode*>($6)) {
            bv -> indent_ = (bv -> indent_) + 1;
            rv = new TableNode($2, NULL, $4,
                                bv->toString(), NULL, $5);
        } else {
            rv = new TableNode($2, NULL, $4,
                                $6->toString(), NULL, $5);
        }

        //BodyNode* bv = dynamic_cast<BodyNode*>($5);
        //bv -> indent_ = (bv -> indent_) + 1;
        //auto rv = new TableNode($2, NULL, $4,
        //                        $5->toString(), "");
        node_array.push_back(rv);
        $$=rv;
    }
    | pragmas TABLE name "{" tableReads tableActions actionProfileSpecification body "}" {
        AstNode* rv;
        BodyNode* bv;
        if (bv = dynamic_cast<BodyNode*>($8)) {
            bv -> indent_ = (bv -> indent_) + 1;
            rv = new TableNode($3, $5, $6,
                                bv->toString(), $1, $7);
        } else {
            rv = new TableNode($3, $5, $6,
                                $8->toString(), $1, $7);
        }
        //BodyNode* bv = dynamic_cast<BodyNode*>($7);
        //bv -> indent_ = (bv -> indent_) + 1;
        //auto rv = new TableNode($3, $5, $6,
                                //$7->toString(), $1);
        node_array.push_back(rv);
        $$=rv;
    }
    | pragmas TABLE name "{" tableActions actionProfileSpecification body "}" {
        AstNode* rv;
        BodyNode* bv;
        if (bv = dynamic_cast<BodyNode*>($7)) {
            bv -> indent_ = (bv -> indent_) + 1;
            rv = new TableNode($3, NULL, $5,
                                bv->toString(), $1, $6);
        } else {
            rv = new TableNode($3, NULL, $5,
                                $7->toString(), $1, $6);
        }
        //BodyNode* bv = dynamic_cast<BodyNode*>($6);
        //bv -> indent_ = (bv -> indent_) + 1;
        //auto rv = new TableNode($3, NULL, $5,
        //                        $6->toString(), $1);
        node_array.push_back(rv);
        $$=rv;
    }    
    | TABLE name "{" tableReads actionProfileSpecification body "}" {
        AstNode* rv;
        BodyNode* bv;
        if (bv = dynamic_cast<BodyNode*>($6)) {
            bv -> indent_ = (bv -> indent_) + 1;
            rv = new TableNode($2, $4, NULL,
                                bv->toString(), NULL, $5);
        } else {
            rv = new TableNode($2, $4, NULL,
                                $6->toString(), NULL, $5);
        }
        node_array.push_back(rv);
        $$=rv;
    }    
    | pragmas TABLE name "{" tableReads actionProfileSpecification body "}" {
        AstNode* rv;
        BodyNode* bv;
        if (bv = dynamic_cast<BodyNode*>($7)) {
            bv -> indent_ = (bv -> indent_) + 1;
            rv = new TableNode($3, $5, NULL,
                                bv->toString(), $1, $6);
        } else {
            rv = new TableNode($3, $5, NULL,
                                $7->toString(), $1, $6);
        }
        node_array.push_back(rv);
        $$=rv;
    }    
;

tableReads :
    READS "{" tableReadStmts "}" {
        $$=$3;
    }
;

tableReadStmts :
    /* empty */ {
        $$=new TableReadStmtsNode();
    }
    | tableReadStmts tableReadStmt {
        TableReadStmtsNode* rv = dynamic_cast<TableReadStmtsNode*>($1);
        TableReadStmtNode* trs = dynamic_cast<TableReadStmtNode*>($2);
        rv->push_back(trs);
        node_array.push_back(rv);
        $$=rv;        
    }
;

tableReadStmt :
    field ":" EXACT ";" {
        AstNode* rv = new TableReadStmtNode(TableReadStmtNode::EXACT, $1);
        node_array.push_back(rv);
        $$=rv;
    }
    | field ":" TERNARY ";" {
        AstNode* rv = new TableReadStmtNode(TableReadStmtNode::TERNARY, $1);
        node_array.push_back(rv);
        $$=rv;
    }
    | field ":" LPM ";" {
        AstNode* rv = new TableReadStmtNode(TableReadStmtNode::LPM, $1);
        node_array.push_back(rv);
        $$=rv;
    }
    | field ":" RANGE ";" {
        AstNode* rv = new TableReadStmtNode(TableReadStmtNode::RANGE, $1);
        node_array.push_back(rv);
        $$=rv;    
    }    
    | headerExtractRef ":" VALID ";" {
        AstNode* rv = new TableReadStmtNode(TableReadStmtNode::VALID, $1);
        node_array.push_back(rv);
        $$=rv;    
    }
;

pragmas :
    /* empty */ {
        $$=new PragmasNode();
    }
    | pragmas PRAGMA {
      PragmasNode* rv = dynamic_cast<PragmasNode*>($1);
      PragmaNode* p = new PragmaNode($2);
      rv->push_back(p);
      node_array.push_back(rv);
      $$ = rv;
    }
;

tableActions :
    ACTIONS "{" tableActionStmts "}" {
        $$=$3;
    }
;

tableActionStmts :
    /* empty */ {
        $$=new TableActionStmtsNode();
    }
    | tableActionStmts tableActionStmt {
        TableActionStmtsNode* rv = dynamic_cast<TableActionStmtsNode*>($1);
        TableActionStmtNode* tas = dynamic_cast<TableActionStmtNode*>($2);
        rv->push_back(tas);
        node_array.push_back(rv);
        $$=rv;        
    }
;

tableActionStmt :
    name ";" {
        AstNode* rv = new TableActionStmtNode($1);
        node_array.push_back(rv);
        $$=rv;
    }
;

headerTypeDeclaration :
    HEADER_TYPE name "{" headerDecBody body "}" {
        if (dynamic_cast<BodyNode*>($5)) {
            BodyNode* bv = dynamic_cast<BodyNode*>($5);
            bv -> indent_ = (bv -> indent_) + 1;    
        }
        //BodyNode* bv = dynamic_cast<BodyNode*>($5);
        //bv -> indent_ = (bv -> indent_) + 1;
        AstNode* rv = new HeaderTypeDeclarationNode($2, $4, $5);
        node_array.push_back(rv);
        $$=rv;
    }
;

headerDecBody :
    FIELDS "{" fieldDecList fieldDec "}" {
        FieldDecsNode* rv = dynamic_cast<FieldDecsNode*>($3);
        FieldDecNode* fd = dynamic_cast<FieldDecNode*>($4);
        rv->push_back(fd);
        $$=rv;
    }
;

fieldDec :
    name ":" integer ";" {
        AstNode* rv = new FieldDecNode($1, $3, NULL);
        $$=rv;
    }
    | name ":" integer L_PAREN body R_PAREN ";" {
        AstNode* rv = new FieldDecNode($1, $3, $5);
        $$=rv;
    }
;

fieldDecList :
    /* empty */ {
        $$=new FieldDecsNode();
    }
    | fieldDecList fieldDec {
        FieldDecsNode* rv = dynamic_cast<FieldDecsNode*>($1);
        FieldDecNode* fd = dynamic_cast<FieldDecNode*>($2);
        rv->push_back(fd);
        $$=rv;
    }
;

headerInstance :
    HEADER name name SEMICOLON {
        HeaderInstanceNode* rv = new HeaderInstanceNode($2, $3, NULL);
        node_array.push_back(rv);
        $$=rv;
    }
    | HEADER name name L_BRACKET integer R_BRACKET SEMICOLON {
        HeaderInstanceNode* rv = new HeaderInstanceNode($2, $3, $5);
        node_array.push_back(rv);
        $$=rv;    
    }
;

metadataInstance :
    METADATA name name ";" {
        MetadataInstanceNode* rv = new MetadataInstanceNode($2, $3);
        node_array.push_back(rv);
        $$=rv;
    }
;    

actionFunctionDeclaration :
    ACTION name "(" actionParamList ")" "{" actionStatements "}" {
        ActionNode* rv = new ActionNode($2, $4, $7);
        node_array.push_back(rv);
        $$=rv;
    }
;

actionParamList :
    /* empty */ {
        $$=new ActionParamsNode();
    }
    | actionParam {
        ActionParamsNode* rv = new ActionParamsNode();
        ActionParamNode* ap = dynamic_cast<ActionParamNode*>($1);
        rv->push_back(ap);
        $$=rv;
    }
    | actionParamList "," actionParam {
        ActionParamsNode* rv = dynamic_cast<ActionParamsNode*>($1);
        ActionParamNode* ap = dynamic_cast<ActionParamNode*>($3);
        rv->push_back(ap);
        $$=rv;
    }
;

actionParam :
    field {
        AstNode* rv = new ActionParamNode($1);
        node_array.push_back(rv);
        $$=rv;
    }
    | name {
        AstNode* rv = new ActionParamNode($1);
        node_array.push_back(rv);
        $$=rv;
    }
;

actionStatements :
    /* empty */ {
        AstNode* rv = new ActionStmtsNode();
        node_array.push_back(rv);
        $$=rv;
    }
    | actionStatements actionStatement {
        ActionStmtsNode* rv = dynamic_cast<ActionStmtsNode*>($1);
        ActionStmtNode* as = dynamic_cast<ActionStmtNode*>($2);
        rv->push_back(as);
        $$=rv;
    }
;

actionStatement :
    name "(" argList ")" ";" {
        ActionStmtNode* rv = new ActionStmtNode($1, $3, ActionStmtNode::NAME_ARGLIST, NULL, NULL);
        node_array.push_back(rv);
        $$=rv;
    }
    /* e.g., bi.execute_stateful_alu(eg_intr_md.egress_port) or index */
    | name "." name "(" argList ")" ";" {
        ActionStmtNode* rv = new ActionStmtNode($1, $5, ActionStmtNode::PROG_EXEC, $3, NULL);
        node_array.push_back(rv);
        $$=rv;    
    }
;


actionProfileDecl :
    ACTION_PROFILE name L_BRACE tableActions body actionSelector R_BRACE {
        BodyNode* bv = dynamic_cast<BodyNode*>($5);
        bv -> indent_ = (bv -> indent_) + 1;
        ActionProfile* rv = new ActionProfile($2, $4, bv, $6);
        node_array.push_back(rv);
        $$=rv;
    }
;

actionSelector :
    /* empty */ {
        ActionSelector* rv = new ActionSelector(NULL);
        $$=rv;
    }
    | DYNAMIC_ACTION_SELECTION ":" name ";" {
        ActionSelector* rv = new ActionSelector($3);
        $$=rv;
    }
;

actionSelectorDecl :
    ACTION_SELECTOR name L_BRACE selectionKey selectionMode selectionType R_BRACE {
        ActionSelectorDecl* rv = new ActionSelectorDecl($2, $4, $5, $6);
        $$=rv;
    }
;

selectionKey :
    SELECTION_KEY ":" name ";" {
        SelectionKey* rv = new SelectionKey($3);
        $$=rv;
    }
;

selectionMode :
    /* empty */ {
        SelectionMode* rv = new SelectionMode(NULL);
        $$=rv;
    }
    | SELECTION_MODE ":" name ";" {
        SelectionMode* rv = new SelectionMode($3);
        $$=rv;
    }
;

selectionType :
    /* empty */ {
        SelectionType* rv = new SelectionType(NULL);
        $$=rv;
    }
    | SELECTION_TYPE ":" name ";" {
        SelectionType* rv = new SelectionType($3);
        $$=rv;
    }
;

assertDecl :
    ASSERT_KEY L_PAREN expr R_PAREN ";" {
        AssertNode* rv = new AssertNode($3, num_assertions);
        num_assertions += 1;
        $$=rv;
    }
;


argList :
    /* empty */ {
        ArgsNode* rv = new ArgsNode();
        node_array.push_back(rv);
        $$=rv;
    }
    | arg {
        ArgsNode* rv = new ArgsNode();
        BodyWordNode* bw = dynamic_cast<BodyWordNode*>($1);
        rv->push_back(bw);
        node_array.push_back(rv);
        $$=rv;
    }
    | argList "," arg {
        ArgsNode* rv = dynamic_cast<ArgsNode*>($1);
        BodyWordNode* bw = dynamic_cast<BodyWordNode*>($3);
        rv->push_back(bw);
        $$=rv;
    }
;

arg :
    name {
        AstNode* rv = new BodyWordNode(BodyWordNode::NAME, $1);
        node_array.push_back(rv);
        $$=rv;
    }
    | field {
        AstNode* rv = new BodyWordNode(BodyWordNode::FIELD, $1);
        node_array.push_back(rv);
        $$=rv;
    }
    | integer {
        AstNode* rv = new BodyWordNode(BodyWordNode::INTEGER, $1);
        node_array.push_back(rv);
        $$=rv;
    }
    | hex {
        AstNode* rv = new BodyWordNode(BodyWordNode::HEX, $1);
        node_array.push_back(rv);
        $$=rv;
    }
;

/* 
General form of P4-14 declarations: 
keyWord name [ name ] ["(" opts ")"] [ [";"] | "{" body "}"]
*/

p4ExprTofino :
    // Parsed statements.
    headerTypeDeclaration {
        node_array.push_back($1);
        $$=$1;
    }
    | headerInstance {
        node_array.push_back($1);
        $$=$1;
    } 
    | metadataInstance {
        node_array.push_back($1);
        $$=$1;
    }
    | parserDecl {
        //cout << "Reaching here 21" << endl;
        node_array.push_back($1);
        //cout << "Reaching here 22" << endl;
        $$=$1;
        //cout << "Reaching here 23" << endl;
    } 
    | registerDecl {
        // necessary for mirroring measurement register
        node_array.push_back($1);
        $$=$1;
    } 
    | actionFunctionDeclaration {
        node_array.push_back($1);
        $$=$1;
    }
    | tableDecl {
        node_array.push_back($1);
        $$=$1;
    }
    | controlDecl {
        node_array.push_back($1);
        $$=$1;
    }
    | actionSelectorDecl {
        node_array.push_back($1);
        $$=$1;
    }
    | actionProfileDecl {
        node_array.push_back($1);
        $$=$1;
    }
    | aluBlock {
        node_array.push_back($1);
        $$=$1;
    }
    // Generic statements.
    | keyWord name ";" {
        AstNode* rv = new P4ExprNode($1, $2, NULL, NULL, NULL);
        node_array.push_back(rv);
        $$=rv;
    }
    | keyWord name "{" body "}" {
        BodyNode* bv = dynamic_cast<BodyNode*>($4);
        bv -> indent_ = (bv -> indent_) + 1;
        AstNode* rv = new P4ExprNode($1, $2, NULL, NULL, bv);
        node_array.push_back(rv);
        $$=rv;
    }
    /* name2, no opts */
    | keyWord name name ";" {
        AstNode* rv = new P4ExprNode($1, $2, $3, NULL, NULL);
        node_array.push_back(rv);
        $$=rv;
    }
    /* To parse e.g., "calculated_field ipv4.hdrChecksum" */
    | keyWord name "." name "{" body "}" {  
        BodyNode* bv = dynamic_cast<BodyNode*>($6);
        bv -> indent_ = (bv -> indent_) + 1;  
        AstNode* rv = new P4ExprNode($1, $2, $4, NULL, bv);
        node_array.push_back(rv);
        $$=rv;
    }
    /* no name2, opts */
    | keyWord name "(" opts ")" ";" {
        AstNode* rv = new P4ExprNode($1, $2, NULL, $4, NULL);
        node_array.push_back(rv);
        $$=rv;        
    }
    | keyWord name "(" opts ")" "{" body "}" {
        BodyNode* bv = dynamic_cast<BodyNode*>($7);
        bv -> indent_ = (bv -> indent_) + 1;
        AstNode* rv = new P4ExprNode($1, $2, NULL, $4, bv);
        node_array.push_back(rv);
        $$=rv;                
    }        
    /* name2, opts */
    | keyWord name name "(" opts ")" ";" {
        AstNode* rv = new P4ExprNode($1, $2, $3, $5, NULL);
        node_array.push_back(rv);
        $$=rv;
    }
    | keyWord name name "(" opts ")" "{" body "}" {
        BodyNode* bv = dynamic_cast<BodyNode*>($8);
        bv -> indent_ = (bv -> indent_) + 1;
        AstNode* rv = new P4ExprNode($1, $2, $3, $5, bv);
        node_array.push_back(rv);
        $$=rv;
    }
;

p4ExprBmv2 :
    // Parsed statements.
    tableDecl {
        node_array.push_back($1);
        $$=$1;
    }
    | headerTypeDeclaration {
        node_array.push_back($1);
        $$=$1;
    }
    | headerInstance {
        node_array.push_back($1);
        $$=$1;
    }
    | actionFunctionDeclaration {
        node_array.push_back($1);
        $$=$1;
    }
    // Generic statements.
    | keyWord name ";" {
        AstNode* rv = new P4ExprNode($1, $2, NULL, NULL, NULL);
        node_array.push_back(rv);
        $$=rv;
    }
    | keyWord name "{" body "}" {
        AstNode* rv = new P4ExprNode($1, $2, NULL, NULL, $4);
        dynamic_cast<P4ExprNode*>(rv) -> body_ -> indent_ = (dynamic_cast<P4ExprNode*>(rv) -> body_ -> indent_) + 1;
        node_array.push_back(rv);
        $$=rv;
    }
    /* name2, no opts */
    | keyWord name name ";" {
        AstNode* rv = new P4ExprNode($1, $2, $3, NULL, NULL);
        node_array.push_back(rv);
        $$=rv;
    }
    | keyWord name name "{" body "}" {
        AstNode* rv = new P4ExprNode($1, $2, $3, NULL, $5);
        dynamic_cast<P4ExprNode*>(rv) -> body_ -> indent_ = (dynamic_cast<P4ExprNode*>(rv) -> body_ -> indent_) + 1;
        node_array.push_back(rv);
        $$=rv;
    }
    /* no name2, opts */
    | keyWord name "(" opts ")" ";" {
        AstNode* rv = new P4ExprNode($1, $2, NULL, $4, NULL);
        node_array.push_back(rv);
        $$=rv;        
    }
    | keyWord name "(" opts ")" "{" body "}" {
        AstNode* rv = new P4ExprNode($1, $2, NULL, $4, $7);
        dynamic_cast<P4ExprNode*>(rv) -> body_ -> indent_ = (dynamic_cast<P4ExprNode*>(rv) -> body_ -> indent_) + 1;
        node_array.push_back(rv);
        $$=rv;                
    }        
    /* name2, opts */
    | keyWord name name "(" opts ")" ";" {
        AstNode* rv = new P4ExprNode($1, $2, $3, $5, NULL);
        node_array.push_back(rv);
        $$=rv;
    }
    | keyWord name name "(" opts ")" "{" body "}" {
        AstNode* rv = new P4ExprNode($1, $2, $3, $5, $8);
        dynamic_cast<P4ExprNode*>(rv) -> body_ -> indent_ = (dynamic_cast<P4ExprNode*>(rv) -> body_ -> indent_) + 1;
        node_array.push_back(rv);
        $$=rv;
    }
;


// KeyWord should be a list of P4 keyWords.
// For now its a generic identifier.
keyWord :
    IDENTIFIER {
        std::string* newStr = new string(string($1));
        AstNode* rv = new KeywordNode(newStr);
        node_array.push_back(rv);
        $$=rv;
    }
;

// A name is a valid P4 identifier
name : 
    IDENTIFIER {
        std::string* newStr = new string(string($1));
        AstNode* rv = new NameNode(newStr);
        node_array.push_back(rv);
        $$=rv;
    }
;

// Options are empty or a list of names
opts :
    /* empty */ {
        AstNode* rv = new OptsNode(NULL);
        node_array.push_back(rv);
        $$=rv;
    }
    | nameList {
        AstNode* rv = new OptsNode($1);
        node_array.push_back(rv);
        $$=rv;
    }
;

// A name list is a comma-separated list of names
nameList : 
    name {
        AstNode* rv = new NameListNode(NULL, $1);
        node_array.push_back(rv);
        $$=rv;
    }
    | nameList "," name {
        AstNode* rv = new NameListNode($1, $3);
        node_array.push_back(rv);
        $$=rv;
    }
;


// A body is an unparsed block of code in any whitespace-insensitive language
// e.g., P4, C, C++
body :
    /* empty */ {
        AstNode* empty = new EmptyNode();
        $$ = empty;
    }
    | body bodyWord {
        AstNode* rv = new BodyNode($1, NULL, $2);
        node_array.push_back(rv);
        $$=rv;
    }
    | body "{" body "}" {
        AstNode* rv = new BodyNode($1, $3, NULL);
        node_array.push_back(rv);
        $$=rv;
    }
;

// A string can be an identifier, word, or parsed special character.
bodyWord :
    name {
        AstNode* rv = new BodyWordNode(BodyWordNode::NAME, $1);
        node_array.push_back(rv);
        $$=rv;
    }
    | integer {
        AstNode* rv = new BodyWordNode(BodyWordNode::INTEGER, $1);
        node_array.push_back(rv);
        $$=rv;
    }
    | hex {
        AstNode* rv = new BodyWordNode(BodyWordNode::HEX, $1);
        node_array.push_back(rv);
        $$=rv;
    }       
    | specialChar {
        AstNode* rv = new BodyWordNode(BodyWordNode::SPECIAL, $1);
        node_array.push_back(rv);
        $$=rv;
    }       
    | STRING {
        AstNode* sv = new StrNode(new string($1));
        AstNode* rv = new BodyWordNode(BodyWordNode::STRING, sv);
        node_array.push_back(rv);
        $$=rv;
        free($1);
    }
    // Better to set up blackbox declaration itself
;

specialChar:
    L_PAREN {
        string* newStr = new string("(");
        AstNode* rv = new SpecialCharNode(newStr);
        node_array.push_back(rv);
        $$=rv;}
    | R_PAREN {
        string* newStr = new string(")");
        AstNode* rv = new SpecialCharNode(newStr);
        node_array.push_back(rv);
        $$=rv;}
    | L_BRACKET {
        string* newStr = new string("[");
        AstNode* rv = new SpecialCharNode(newStr);
        node_array.push_back(rv);
        $$=rv;}
    | R_BRACKET {
        string* newStr = new string("]");
        AstNode* rv = new SpecialCharNode(newStr);
        node_array.push_back(rv);
        $$=rv;}
    | SEMICOLON {
        string* newStr = new string(";");
        AstNode* rv = new SpecialCharNode(newStr);
        node_array.push_back(rv);
        $$=rv;}
    | COLON {
        string* newStr = new string(":");
        AstNode* rv = new SpecialCharNode(newStr);
        node_array.push_back(rv);
        $$=rv;}
    | COMMA {
        string* newStr = new string(",");
        AstNode* rv = new SpecialCharNode(newStr);
        node_array.push_back(rv);
        $$=rv;}
    | PERIOD {
        string* newStr = new string(".");
        AstNode* rv = new SpecialCharNode(newStr);
        node_array.push_back(rv);
        $$=rv;
    }
    | PLUS {
        string* newStr = new string("+");
        AstNode* rv = new SpecialCharNode(newStr);
        node_array.push_back(rv);
        $$=rv;
    }
    | GTE {
        string* newStr = new string(">=");
        AstNode* rv = new SpecialCharNode(newStr);
        node_array.push_back(rv);
        $$=rv;
    }
    | LT {
        string* newStr = new string("<");
        AstNode* rv = new SpecialCharNode(newStr);
        node_array.push_back(rv);
        $$=rv;
    }
    | GT {
        string* newStr = new string(">");
        AstNode* rv = new SpecialCharNode(newStr);
        node_array.push_back(rv);
        $$=rv;
    }
    | EQU {
        string* newStr = new string("==");
        AstNode* rv = new SpecialCharNode(newStr);
        node_array.push_back(rv);
        $$=rv;
    }
    | NEQU {
        string* newStr = new string("!=");
        AstNode* rv = new SpecialCharNode(newStr);
        node_array.push_back(rv);
        $$=rv;
    }
    | AND {
        string* newStr = new string("&&");
        AstNode* rv = new SpecialCharNode(newStr);
        node_array.push_back(rv);
        $$=rv;
    }
    | MINUS {
        string* newStr = new string("-");
        AstNode* rv = new SpecialCharNode(newStr);
        node_array.push_back(rv);
        $$=rv;
    }
    | WIDTH {
        string* newStr = new string("width");
        AstNode* rv = new SpecialCharNode(newStr);
        node_array.push_back(rv);
        $$=rv;}
    | SLASH {
        string* newStr = new string("/");
        AstNode* rv = new SpecialCharNode(newStr);
        node_array.push_back(rv);
        $$=rv;}
    | VALID {
        string* newStr = new string("valid");
        AstNode* rv = new SpecialCharNode(newStr);
        node_array.push_back(rv);
        $$=rv;
    }
    | IF {
        string* newStr = new string("if");
        AstNode* rv = new SpecialCharNode(newStr);
        node_array.push_back(rv);
        $$=rv;
    }

;

integer :
    INTEGER {
        string* strVal = new string(string($1));
        AstNode* rv = new IntegerNode(strVal);
        node_array.push_back(rv);
        $$=rv;        
        free($1);    
    }
;

hex :
    HEX {
        string* strVal = new string(string($1));
        AstNode* rv = new HexNode(strVal);
        node_array.push_back(rv);
        $$=rv;        
        free($1);    
    }
    // For HEX mask HEX
    | HEX name HEX {
        string* strVal = new string(string($1));
        string* maskVal = new string(string($3));
        AstNode* rv = new HexNode(strVal, maskVal);
        node_array.push_back(rv);
        $$=rv;        
        free($1); 
        free($3); 
    }
;

/*=====  End of P4 expressions  ======*/


field :
    name "." name {
        AstNode* rv = new FieldNode($1, $3, NULL);
        node_array.push_back(rv);
        //cout << "Field: " << rv -> toString() << endl;
        $$=rv;
    }
    | name L_BRACKET integer R_BRACKET "." name {
        AstNode* rv = new FieldNode($1, $6, $3);
        node_array.push_back(rv);
        $$=rv;
    }
    | name "." VALID {
        string* newStr = new string("valid");
        AstNode* valid = new NameNode(newStr);
        AstNode* rv = new FieldNode($1, valid, NULL);
        node_array.push_back(rv);
        $$=rv;        
    }
;


/* END NEW GRAMMAR */

%%


void handler(int sig);

int main(int argc, char* argv[]) {
    signal(SIGSEGV, handler);

    parseArgs(argc, argv);  

    yyin = in_file;

    // Run this when you see weird issues. It helps with debugging.
    // int out;
    // while (true) {
    //     out = yylex();
    //     if (out > 0) {
    //         // cout << yylval.sval << endl;
    //         cout << "token type: " <<  out << endl;
    //         cout << "token string: " << yytext << endl;
    //         // cout << yylval.aval -> toString() << endl;
    //     } else {
    //         exit(0);
    //     }
    // }
    yyparse();
    fclose(in_file);

    PRINT_VERBOSE("Number of syntax tree nodes: %d\n", node_array.size());

    P4Modifier* modifier;
    ofstream os;

    os.open(p4_out_ut);
    modifier = new UTModifier(root, target, rules, ut_rules_out_fn.c_str(), num_assertions, out_fn_base);
    os << root->toString() << endl << endl;
    os << modifier->GetUnanchoredNodesStr(); 
    os.close();

    root = NULL;
    done_init=0;
    num_assertions = 0;

    yyrestart(yyin);

    yyin = fopen(in_fn, "r");
    yyparse();
    os.open(p4_out_dt);
    modifier = new DTModifier(root, target, num_assertions, out_fn_base);
    os << root->toString() << endl << endl;
    os << modifier->GetUnanchoredNodesStr(); 
    os.close();


    cout << "[Done main]" << endl;

    return 0;
}


void yyerror(const char* s) {
    printf("Line %d: %s when parsing '%s'\n", yylineno, s, yytext);
    exit(1);
}

// Crash dump handler: https://stackoverflow.com/questions/77005/how-to-automatically-generate-a-stacktrace-when-my-program-crashes
void handler(int sig) {
    void* array[10];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}
