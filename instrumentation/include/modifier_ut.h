#ifndef P4_MODIFIER_UT_H
#define P4_MODIFIER_UT_H

#include "modifier.h"

const std::string kTiPortCorrection {"ti_port_correction"};

class UTModifier : public P4Modifier {
public:
    ostringstream assert_rules_;
	int max_keys_ = 0;
	const char* rules_in_ = NULL;
    const char* rules_out_ = NULL;

	map<string, string> tbl_2_state_action_;
	map<string, string> state_action_2_tbl_;
	map<string, vector<string> > tbl_2_read_fields_ {};
	vector<string> popular_actions_;

	UTModifier(AstNode* head, char* target, char* plan, const char* rules_in, const char* rules_out, int num_assertions, char* out_fn_base);

    string addAssertExpression(ExprNode* expr, int assertNum);
    void addAssertTable(AssertNode* tableStmt);
	void addModifyTables(AstNode* root);
	void addModifyTablesRecurse(AstNode* root, P4ControlBlock* controlBlock);
	void addStatefulTblActions(AstNode* root);
	void addVisitedType(AstNode* head);
    void correctPort(AstNode* root);
    void createStatefulTables(AstNode* root);
    ControlStatement* createTableCall(string tableName);
    void distinguishDefaultPopularActions(AstNode* root);
    void distinguishPopularActions(AstNode* root);
    void distinguishPopularActionsForControl(AstNode* root);
    void findStatefulActions(AstNode* root);
    void instrumentRules();
    void iterateInputNodes(AstNode* head);
    void markActionVisited(AstNode* head);
	void redirectDroppedPacket(AstNode* root);
};

#endif