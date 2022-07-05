#ifndef P4_MODIFIER_DT_H
#define P4_MODIFIER_DT_H

#include "modifier.h"
#include "../src/static_analysis/static_analysis.cpp"
#include "../src/modifier/parser_graph.cpp"

#define BLOOM_FILTER_ENTRIES 4096
#define BLOOM_FILTER_BIT_WIDTH 1


const std::string kVisitedHeaderType {"fp4_visited_t"};
const std::string kVisitedMetadata {"fp4_visited"};
// From UT
const std::string kTiGetRegPosSim = "ti_get_reg_pos";
const std::string kAiGetRegPosSim = "ai_get_reg_pos";
const std::string kTiReadBloomFilter1Sim = "ti_read_values_1";
const std::string kAiReadBloomFilter1Sim = "ai_read_values_1";
const std::string kTiReadBloomFilter2Sim = "ti_read_values_2";
const std::string kAiReadBloomFilter2Sim = "ai_read_values_2";
const std::string kTiPathAssertionSim = "ti_path_assertion";
const std::string kRiBloomFilter1Sim = "ri_bloom_filter_1";
const std::string kRiBloomFilter2Sim = "ri_bloom_filter_2";
const std::string kFiBloomFilterHashFieldsSim = "fi_bf_hash_fields";
const std::string kAiSendToControlSim = "ai_send_to_control_plane";
const std::string kAiRecyclePktSim = "ai_recycle_packet";
// Seed
const std::string kTiGetSeed {"ti_get_random_seed"};
const std::string kAiGetSeed {"ai_get_random_seed"};
const std::string kTiTurnMutation {"ti_turn_on_mutation"};
const std::string kAiTurnMutation {"ai_turn_on_mutation"};
const std::string kTiSetSeed {"ti_set_seed_num"};
const std::string kAiSetSeed {"ai_set_seed_num"};
const std::string kTiSetResubmit {"ti_set_resubmit"};
const std::string kAiSetResubmit {"ai_set_resubmit"};
const std::string kTiSetFixed {"ti_set_fixed_header"};
const std::string kAiSetFixed {"ai_set_fixed_header"};
const std::string kTiCreatePkt {"ti_create_packet"};
const std::string kTiAddFields {"ti_add_fields"};
// CPU
const std::string kTiSetPort {"ti_set_port"};
const std::string kAiSetPort {"ai_set_port"};
// Mutate
const std::string kTeGetTblSeed {"te_get_table_seed"};
const std::string kAeGetTblSeed {"ae_get_table_seed"};
const std::string kTeSetTblSeed {"te_set_table_seed"};
const std::string kAeSetTblSeed {"ae_set_table_seed"};
const std::string kTeMoveFields {"te_move_fields"};
const std::string kTeApplyMutations {"te_apply_mutations"};
const std::string kAeApplyMutations {"ae_apply_mutations"};
const std::string kTeMoveBackFields {"te_move_back_fields"};
const std::string kTeDoResubmit {"te_do_resubmit"};
const std::string kAeDoResubmit {"ae_resubmit"};
const std::string kFeResubmit {"fe_resub_fields"};
// Correct port
// Clone
const std::string kTiAddClones {"ti_add_clones"};
const std::string kHdrInstanceCloneSuffix {"_clone"};
const std::string kParserCloneAll {"parse_clone_all"};

const std::string kAiDrop {"ai_drop_packet"};

const std::string kGlobalHeaderType {"fp4_metadata_t"};
const std::string kGlobalMetadata {"fp4_metadata"};

const std::string kTeUpdateCount {"te_update_count"};
const std::string kAeUpdateCount {"ae_update_count"};
const std::string kReUpdateCount {"forward_count_register"};

class DTModifier : public P4Modifier {
public:
    int num_metadata_instance_ = 0;
    int num_hdrtypedeclr_ = 0;
    int num_tbl_nodes_ = 0;
    int max_tblreads_field_num_ = 0;
    ParserGraph* pGraph;

    json controlPlane;
    json header_field;
    json parser_state_machine;

    vector <vector<string> > extseq_ {};
    vector <int> max_tblreads_field_width_ {};
    vector<string> prev_hdrinstance_combinations_ {};
    vector<vector<string>> prev_hdrinstance_combinations_vec_ {};
    map<string, AstNode*> prev_hdrtypedecl_2_nodes_ {};
    map<string, AstNode*> prev_metadatainstance_2_nodes_ {};
    map<string, AstNode*> prev_name_2_tablenodes_ {};
    map<string, vector<string> > tbl2readfields_ {};
    map<string, HeaderTypeDeclarationNode*> instance_2_header_type_ {};

    DTModifier(AstNode* head, char* target, int num_assertions, char* out_fn_base);
    
    set<string> AddControlBlock(AstNode* root, DAG* dag, P4ControlBlock* controlBlock, set<string> parents, bool evaluationResult);
    void AddFieldToJson(string fieldName, int numBit);
    void AddGlobalMetadata(AstNode* root);
    set<string> AddIfBlock(AstNode* root, DAG* dag, IfElseStatement* ifStmt, set<string> parents, bool evaluationResult);
    set<string> AddIfCondition(ExprNode* condition, DAG* dag, set<string> parents, bool evaluationResult);
    void AddParserDependencies(AstNode* root, DAG* dag);
    void AddVisitedType(AstNode* head);
    void AppendEgressCount(AstNode* head);
    void AppendIngressAddClones(AstNode* head);
    void AppendIngressProcessCPU(AstNode* head);
    void AppendIngressProcessSeed(AstNode* head);
    void AppendIngressProcessUT(AstNode* head);
    void ClearIngressEgress(AstNode* head);
    void Clone(AstNode* head);
    void CloneAllHdrInstances(AstNode* head);
    void CloneParser(AstNode* head);
    void CorrectPort(AstNode* head);
    void CountTables();
    void CreateHeaderJson(AstNode* head);
    HeaderTypeDeclarationNode* DeclGlobalHdr();
    void DistinguishPopularActions(AstNode* root);
    void GenerateParserJson(AstNode* head);
    vector<vector <string> > GetExtractSequence(AstNode* root);
    int GetFieldLength(string headerFieldName);
    map<string, map<string, string> > GetHeaderValues();
    void GetHdrInstance2Nodes(AstNode* head);
    void GetHdrInstanceCombinations(AstNode* head);
    void GetHdrTypeDecl2Nodes(AstNode* head);
    void GetMaxFieldWidth(AstNode* head);
    void GetMaxTblReadsFieldNum(map<string, vector<string> > tableReadFields);
    void GetMaxTblReadsFieldWidth(AstNode* head);
    void GetMetadataInstance2Nodes(AstNode* head);
    map<string, vector<string> > GetMoveFieldsMap(AstNode* root);
    void GetName2TableNodes(AstNode* head);
    void IterateInputNodes(AstNode* head);
    void MarkActionVisited(AstNode* head);
    void Mutate(AstNode* head);
    void ProcessCPU(AstNode* head);
    void ProcessSeed(AstNode* head);
    void ProcessUTSim(AstNode* head);
    void RedirectDroppedPacket(AstNode* root);
    void RemoveTblAction(AstNode* head);
    void SaveExtractPaths(vector <vector<string> > extseq);
    void SetNodeValues(AstNode* root, DAG* dag);
    void UpdateCountEgress(AstNode* head);
    void UpdateParserDAG(AstNode* head);
    void WriteHeaderJson(AstNode* head);
    void WriteJsonFile();

};

#endif