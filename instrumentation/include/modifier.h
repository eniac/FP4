#ifndef P4_MODIFIER_H
#define P4_MODIFIER_H

#include <set>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>

#include "json.hpp"
#include "ast_nodes_p4.hpp"
// #include "node_object_v2.hpp"

using json = nlohmann::ordered_json;
#define RESERVED_PORT 0

const std::string kAiNoAction {"ai_NoAction"};

class P4Modifier {

	// All these variables, stl and functions are used by both UT and DT.
public:
	// Variables
	int num_action_ = 0;
	int num_assertions_ = 0;
	int num_hdr_instance_ = 0;
	int num_tables_ = 0;
	int num_tbl_action_stmt_ = 0;
	std::string program_name_;
	AstNode* root_ = NULL;
	std::string sig_ = "fp4";
	char* target_ = NULL;

	// STL
    map<string, int> action_2_bitmap_;
    map<string, AstNode*> prev_action_2_nodes_ {};
	map<string, vector<string> > prev_action_2_tbls_;
	map<string, AstNode*> prev_hdrinstance_2_nodes_ {};
	vector<string> tbl_action_stmt_;
    vector<UnanchoredNode*> unanchored_nodes_;
    set<AstNode*> work_set_;

	map<string, string> action_2_encoding_field_;
	map<string, string> action_2_encoding_incr_;

	// Functions
	P4Modifier(AstNode* head, char* target, char* plan, int num_assertions, char* out_fn_base);

	virtual void AddFieldToJson(string fieldName, int numBit);
	void AddVisitedHdr();
	void AssignAction2Bitmap();
	// std::string CombineHeader(std::vector<std::string> vec);
	HeaderTypeDeclarationNode* DeclVisitedHdr();
	virtual void DistinguishPopularActions(AstNode* root);
	void ExtractVisitedHdr(AstNode* root);
	P4ControlNode* FindControlNode(AstNode* root, string inputControlName);
	void GetAction2Nodes(AstNode* head);
	void GetAction2Tbls(AstNode* head);
	// void GetHdrInstance2Nodes(AstNode* head);
	string GetUnanchoredNodesStr();
	void RemoveNodes(AstNode* root);
	bool TypeContains(AstNode* n, const char* type);


};

#endif
