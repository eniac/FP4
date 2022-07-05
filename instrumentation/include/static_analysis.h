#ifndef NODE_OBJ_CPP
#define NODE_OBJ_CPP

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <regex>
#include <queue>
#include <unordered_set>
#include "helper.hpp"
#include "../util/util.cpp"
#include "ast_nodes_p4.hpp"

using namespace std;

class NodeObject {
public:
    string name;
    virtual ~NodeObject() = default;
    virtual string toString() {
        return "";
    }
};

class HeaderType : public NodeObject {
public:
    HeaderType(string inputName) {
        name = inputName;
    }
    void addField(string fieldName) {
        fields.insert(fieldName);
    }
    set<string> fields;
    string toString() {
        ostringstream oss;
        oss << "HeaderType: " << name << " fields: ";
        for (string f: fields) {
            oss << f << ", ";
        }
        oss << endl;
        return oss.str();
    }
};

class Header : public NodeObject {
public:
    Header(HeaderType* headerTypePointer, string headerName) {
        headerType = headerTypePointer;
        name = headerName;
    }
    string toString() {
        ostringstream oss;
        oss << "Header: " << name << " type: " << headerType -> name << endl;
        return oss.str();
    }
    HeaderType* headerType;
};

class Blackbox : public NodeObject {
public:
    Blackbox(string bbName) {
        name = bbName;
        outputSet = false;
    }
    void addOutput(string outputField) {
        output = outputField;
        outputSet = true;
    }

    void addModifier(string rightSide) {
        modifiers.insert(rightSide);
    }

    string toString() {
        ostringstream oss;
        oss << "Blackbox: " << name << " outputSet: " << outputSet << endl;
        if (outputSet) {
            oss << "output: " << output << endl;
        }
        oss << "modifiers: ";
        for (string fieldName : modifiers) {
            oss << fieldName << " , ";
        }
        oss << endl;
        return oss.str();
    }

    string output;
    set<string> modifiers;
    bool outputSet;
};

class Action : public NodeObject {
public:
    Action(string actionName) {
        name = actionName;
        statefulAlu = NULL;
        processing = false;
        drop = false;
        // rawAction = rawAction_;
    }

    void addDrop() {
        processing = true;
        drop = true;
    }
    void addDependency(string leftSide, string rightSide) {
        processing = true;
        modifiedPairs.insert(make_pair(leftSide, rightSide));
    }
    void addModified(string leftSide) {
        processing = true;
        modified.insert(leftSide);
    }
    void addHeader(Header* headerPointer) {
        processing = true;
        addedHeaders.insert(headerPointer);
    }

    void removeHeader(Header* headerPointer) {
        processing = true;
        removedHeaders.insert(headerPointer);
    }

    void addBlackbox(Blackbox* bbPointer) {
        processing = true;
        statefulAlu = bbPointer;
    }

    void evaluate(std::map<string, int>  validHeaders) {
        for (Header* headerPointer : addedHeaders) {
            validHeaders[headerPointer ->  name] = 1;
        }
        for (Header* headerPointer : removedHeaders) {
            validHeaders[headerPointer -> name] = 0;
        }
    }

    void resolveBlackboxDependencies() {
        if (statefulAlu == NULL or statefulAlu -> outputSet == false) {
            return;
        }
        // cout << "in resolveBlackboxDependencies" << endl;
        if (statefulAlu -> modifiers.size() == 0) {
            addModified(statefulAlu -> output);
        } else {
            for (string fieldName : statefulAlu -> modifiers) {
                addDependency(statefulAlu -> output, fieldName);
            }
        }
        // printInformation();
    }

    string toString() {
        ostringstream oss;
        oss << "Action: " << name << " modifiedPairs: ";
        for (auto f: modifiedPairs) {
            oss << "(" << f.first << ", " <<  f.second << ")" << ", ";
        }
        oss << " modified: ";
        for (auto f: modified) {
            oss << f << ", ";
        }
        oss << " added headers: ";
        for (auto f: addedHeaders) {
            oss << f -> name << ", ";
        }
        oss << endl;
        return oss.str();
    }
    set< pair<string, string> > modifiedPairs;
    set<string> modified;
    set<Header*> addedHeaders;
    set<Header*> removedHeaders;
    Blackbox* statefulAlu;
    bool processing;
    bool drop;
    // ActionNode* rawAction;
};


class IfCondition : public NodeObject {
public:
    IfCondition(int id, ExprNode* rawCondition_) {
        name = "IF_CONDITION_" + to_string(id);
        parseRoot = NULL;
        rawCondition = rawCondition_;
    }
    void addField(string fieldName) {
        fieldsList.insert(fieldName);
    }
    void addParent(NodeObject* par) {
        parents.insert(par);
    }
    void addTrueChild(NodeObject* child) {
        trueChildren.insert(child);
    }
    void addFalseChild(NodeObject* child) {
        falseChildren.insert(child);
    }
    void setRoot(NodeObject* parser) {
        parseRoot = parser;
    }

    // bool evaluate(std::map<string, int>  validHeaders) {
    //     if (rawCondition == NULL) {
    //         cout << "Condition is null" << endl;
    //         return true;
    //     }
    //     cout << "Going in evaluate " << endl;
    //     ExprReturnType evOut = rawCondition -> evaluate(validHeaders);
    //     if (evOut.mode == ExprReturnMode::FALSE) {
    //         return false;
    //     } else {
    //         return true;
    //     }
    // }

    string toString() {
        ostringstream oss;
        oss << "IfCondition: " << name << " fields: ";
        for (auto f: fieldsList) {
            oss << f << ", ";
        }
        if (parseRoot != NULL) {
            oss << "root: " << parseRoot -> name << ", ";
        }
        oss << " True children: ";
        for (auto t: trueChildren) {
            oss << t-> name << ", ";
        }
        oss << " False children: ";
        for (auto t: falseChildren) {
            oss << t-> name << ", ";
        }
        oss << " parents: ";
        for (auto t: parents) {
            oss << t-> name << ", ";
        }
        oss << endl;
        return oss.str();
    }
    ExprNode* rawCondition;
    NodeObject* parseRoot;
    set<NodeObject*> trueChildren;
    set<NodeObject*> falseChildren;
    set<NodeObject*> parents;
    set<string> fieldsList;
};

class Table : public NodeObject {
public:
    Table(string tableName) {
        name = tableName;
        // stageNo = 0;
        parseRoot = NULL;
    }
    void addKey(string keyName) {
        keys.insert(keyName);
    }

    void addAction(Action* action) {
        actionList.insert(action);
    }
    void addParent(NodeObject* par) {
        parents.insert(par);
    }
    void addChild(NodeObject* child) {
        children.insert(child);
    }
    void setRoot(NodeObject* parser) {
        parseRoot = parser;
    }

    string toString() {
        ostringstream oss;
        oss << "Table: " << name << " actions: ";
        for (auto f: actionList) {
            oss << f -> name << ", ";
        }
        oss << " keys: ";
        for (auto f: keys) {
            oss << f << ", ";
        }
        if (parseRoot != NULL) {
            oss << "root: " << parseRoot -> name << ", ";
        }
        oss << " children: ";
        for (auto t: children) {
            oss << t-> name << ", ";
        }
        oss << " parents: ";
        for (auto t: parents) {
            oss << t-> name << ", ";
        }
        oss << endl;
        return oss.str();
    }
    set<string> keys;
    set<Action*> actionList;
    set<NodeObject*> children;
    set<NodeObject*> parents;
    NodeObject* parseRoot;
};

class ParserNode : public NodeObject {
public:
    ParserNode() {
        name = "parser";
    }
    void addDependency(string leftSide, string rightSide) {
        modifiedPairs.insert(make_pair(leftSide, rightSide));
    }
    void addChild(NodeObject* child) {
        children.insert(child);
    }
    string toString() {
        ostringstream oss;
        oss << "Parser: " << " modifiedPairs: ";
        for (auto f: modifiedPairs) {
            oss << "(" << f.first << ", " <<  f.second << ")" << ", ";
        }
        oss << " children: ";
        for (auto t: children) {
            oss << t-> name << ", ";
        }
        oss << endl;
        return oss.str();
    }
    set<NodeObject*> children;
    set< pair<string, string> > modifiedPairs;
};

class PathInformation {
public:
    PathInformation() {}
    PathInformation(const PathInformation &p1) {
        fieldsDict = std::map<string,  set<string> > (p1.fieldsDict);
        validHeaders = std::map<string, int> (p1.validHeaders);
        // valuesSet = std::map<string, uint64_t> (p1.valuesSet);
        nextToExecute = p1.nextToExecute;
        out = std::map<string, vector<string> > (p1.out);
        actionPath = vector<string> (p1.actionPath);
    }
    std::map<string, set<string> > fieldsDict;
    std::map<string, int>  validHeaders;
    // Use valuesSet if decide to go to full symbolic execution
    // std::map<string, uint64_t> valuesSet;
    // current table/ifcond/parser
    NodeObject* nextToExecute;

    // Path specific output
    std::map<string, vector<string> > out;
    // For debugging only
    vector<string> actionPath;
};


class DAG {
public:
    map <string, NodeObject*> nameToNode;
    map <string, vector<string> > tableToMove;
    vector < std::map<string, int>  > parserPaths;

    // Uncomment metadataFromParser if decide to go to full symbolic execution
    // vector<map<string, uint64_t> > metadataFromParser;
    vector<map<string, vector<string> > > metadataDependency;
    ParserNode* root;
    int ifConditionID;

    DAG() {
        root = new ParserNode();
        ifConditionID = 0;
    }

    void addTable(string tableName) {
        tableName =  std::regex_replace(tableName, std::regex("^ +| +$|( ) +"), "$1");
        
        if (nameToNode.count(tableName) != 0) {
            return;
        }
        nameToNode[tableName] = new Table(tableName);
        // printInformation();
    }

    void addTableKey(string tableName, string fieldName) {
        tableName =  std::regex_replace(tableName, std::regex("^ +| +$|( ) +"), "$1");
        fieldName =  std::regex_replace(fieldName, std::regex("^ +| +$|( ) +"), "$1");

        Table* table = dynamic_cast<Table*>(nameToNode[tableName]);
        table -> addKey(fieldName);
        // printInformation();
    }

    void addTableAction(string tableName, string actionName) {
        tableName =  std::regex_replace(tableName, std::regex("^ +| +$|( ) +"), "$1");
        actionName =  std::regex_replace(actionName, std::regex("^ +| +$|( ) +"), "$1");
        Table* table = dynamic_cast<Table*>(nameToNode[tableName]);
        Action* action = dynamic_cast<Action*>(nameToNode[actionName]);
        if (!(action -> processing)) {
            // cout << "action wo processing: " << action -> name << endl;
            return;
        }
        // if (actionName == "nop") {
        //     return;
        // }
        // cout << "Adding action: " << action -> name << " to table: " << table -> name << endl;
        table -> addAction(action);
        // printInformation();
    }

    void resolveBlackboxDependencies(string actionName) {
        actionName =  std::regex_replace(actionName, std::regex("^ +| +$|( ) +"), "$1");
        Action* action = dynamic_cast<Action*>(nameToNode[actionName]);
        action -> resolveBlackboxDependencies();
        // printInformation();
    }

    void addAction(string actionName, ActionNode* rawAction) {
        actionName =  std::regex_replace(actionName, std::regex("^ +| +$|( ) +"), "$1");
        if (nameToNode.count(actionName) != 0) {
            return;
        }
        // if (actionName == "nop") {
        //     return;
        // }
        // cout << "in adding action: " << actionName << endl;
        // nameToNode[actionName] = new Action(actionName, rawAction);
        nameToNode[actionName] = new Action(actionName);
        // printInformation();
    }

    void addActionBlackbox(string actionName, string bbName) {
        actionName =  std::regex_replace(actionName, std::regex("^ +| +$|( ) +"), "$1");
        bbName = std::regex_replace(bbName, std::regex("^ +| +$|( ) +"), "$1");
        Action* action = dynamic_cast<Action*>(nameToNode[actionName]);
        Blackbox* blackbox = dynamic_cast<Blackbox*>(nameToNode[bbName]);

        action -> addBlackbox(blackbox);
        // cout << " in addActionBlackbox" << endl;
        // printInformation();
    }

    void addBlackboxModifer(string bbName, string fieldName) {
        if (!checkField(fieldName)) {
            return;
        }
        bbName = std::regex_replace(bbName, std::regex("^ +| +$|( ) +"), "$1");
        // cout << "bbname: " << bbName << " fieldName: " << fieldName << endl;
        Blackbox* blackbox = dynamic_cast<Blackbox*>(nameToNode[bbName]);
        // cout << "found blackbox: " << blackbox -> toString() << endl;
        blackbox -> addModifier(fieldName);
        // cout << "return from addBlackboxModifer " << endl;
        // printInformation();
    }

    void addBlackboxOutput(string bbName, string outputField) {
        bbName = std::regex_replace(bbName, std::regex("^ +| +$|( ) +"), "$1");
        Blackbox* blackbox = dynamic_cast<Blackbox*>(nameToNode[bbName]);
        blackbox -> addOutput(outputField);
        // printInformation();
    }

    void addBlackbox(string bbName) {
        bbName = std::regex_replace(bbName, std::regex("^ +| +$|( ) +"), "$1");
        if (nameToNode.count(bbName) != 0) {
            return;
        }
        // cout << "bbname: " << bbName << endl;
        nameToNode[bbName] = new Blackbox(bbName);
        // printInformation();
    }

    bool isNumber(const string& str) {
        for (char const &c : str) {
            if (std::isdigit(c) == 0) return false;
        }
        return true;
    }

    void addDropinAction(string actionName) {
        actionName =  std::regex_replace(actionName, std::regex("^ +| +$|( ) +"), "$1");
        Action* action = dynamic_cast<Action*>(nameToNode[actionName]);
        // cout << "drop in action" << actionName << endl;
        action -> addDrop();
    }

    void addHeaderInAction(string actionName, string headerName) {
        actionName =  std::regex_replace(actionName, std::regex("^ +| +$|( ) +"), "$1");
        headerName =  std::regex_replace(headerName, std::regex("^ +| +$|( ) +"), "$1");
        if (isNumber(headerName)) {
            return;
        }
        Action* action = dynamic_cast<Action*>(nameToNode[actionName]);
        Header* header = dynamic_cast<Header*>(nameToNode[headerName]);
        // cout << "action name: " << actionName << endl;
        // cout << "header name: " << headerName << endl;

        action -> addHeader(header);
        // cout << "in addHeaderInAction: "  << endl;
        // printInformation();
        // cout << "in addHeaderInAction out: "  << endl;
    }

    void removeHeaderInAction(string actionName, string headerName) {
        actionName =  std::regex_replace(actionName, std::regex("^ +| +$|( ) +"), "$1");
        headerName =  std::regex_replace(headerName, std::regex("^ +| +$|( ) +"), "$1");
        if (isNumber(headerName)) {
            return;
        }
        Action* action = dynamic_cast<Action*>(nameToNode[actionName]);
        Header* header = dynamic_cast<Header*>(nameToNode[headerName]);
        // cout << "action name: " << actionName << endl;
        // cout << "header name: " << headerName << endl;

        action -> removeHeader(header);
        // cout << "in addHeaderInAction: "  << endl;
        // printInformation();
        // cout << "in addHeaderInAction out: "  << endl;
    }
    
    bool checkField(std::string text) {
        std::string line;
        std::vector<std::string> vec;
        std::stringstream ss(text);
        while(std::getline(ss, line, '.')) {
            vec.push_back(line);
        }
        if (vec.size() == 2) {
            return true;
        }
        return false;
    }

    void addFieldDependencyAction(string actionName, string leftSide, string rightSide) {
        actionName =  std::regex_replace(actionName, std::regex("^ +| +$|( ) +"), "$1");
        Action* action = dynamic_cast<Action*>(nameToNode[actionName]);
        leftSide = std::regex_replace(leftSide, std::regex("^ +| +$|( ) +"), "$1");
        rightSide = std::regex_replace(rightSide, std::regex("^ +| +$|( ) +"), "$1");
        // cout << "in addFieldDependencyAction" << endl;
        if (checkField(rightSide)) {
            action -> addDependency(leftSide, rightSide);    
        } else {
            action -> addModified(leftSide);
        }
        // printInformation();
    }

    void addFeildModified(string actionName, string leftSide)  {
        actionName =  std::regex_replace(actionName, std::regex("^ +| +$|( ) +"), "$1");
        Action* action = dynamic_cast<Action*>(nameToNode[actionName]);
        leftSide = std::regex_replace(leftSide, std::regex("^ +| +$|( ) +"), "$1");
        action -> addModified(leftSide);
        // cout << "in addFeildModified" << endl;
        // printInformation();
    }

    void addHeader(string headerTypeName, string headerName) {
        headerName =  std::regex_replace(headerName, std::regex("^ +| +$|( ) +"), "$1");
        if (nameToNode.count(headerName) != 0) {
            return;
        }
        headerTypeName =  std::regex_replace(headerTypeName, std::regex("^ +| +$|( ) +"), "$1");
        HeaderType* headerType = dynamic_cast<HeaderType*>(nameToNode[headerTypeName]);
        // cout << "Creating header: " << headerName << endl;
        nameToNode[headerName] = new Header(headerType, headerName);
        // printInformation();
    }

    void addHeaderType(string headerTypeName) {
        headerTypeName =  std::regex_replace(headerTypeName, std::regex("^ +| +$|( ) +"), "$1");
        if (nameToNode.count(headerTypeName) != 0) {
            return;
        }
        // cout << "Creating headerType: " << headerTypeName << endl;
        nameToNode[headerTypeName] = new HeaderType(headerTypeName);
        // printInformation();
    }

    void addFieldToType(string headerTypeName, string field) {
        headerTypeName =  std::regex_replace(headerTypeName, std::regex("^ +| +$|( ) +"), "$1");
        field =  std::regex_replace(field, std::regex("^ +| +$|( ) +"), "$1");
        HeaderType* headerType = dynamic_cast<HeaderType*>(nameToNode[headerTypeName]);
        headerType -> addField(field);
        // printInformation();
    }

    void addFieldDependencyParser(string leftSide, string rightSide) {
        leftSide = std::regex_replace(leftSide, std::regex("^ +| +$|( ) +"), "$1");
        rightSide = std::regex_replace(rightSide, std::regex("^ +| +$|( ) +"), "$1");
        root -> addDependency(leftSide, rightSide);
        // printInformation();
    }

    set<string> addIfCondition(vector<string> fieldsList, set<string> parents, ExprNode* rawCondition, bool evaluationResult) {
        IfCondition* newCond = new IfCondition(ifConditionID, rawCondition);
        // cout << "Adding ifCondition number" << ifConditionID << ": " ;
        // for (string field: fieldsList) {
        //     cout << field << " , ";
        // }
        // cout << endl;
        ifConditionID += 1;
        nameToNode[newCond -> name] = newCond;
        NodeObject* parent;
        for (string parentName : parents) {
            parentName = std::regex_replace(parentName, std::regex("^ +| +$|( ) +"), "$1");
            // cout << "parent: " << parentName << endl;
            if (parentName == "parser") {
                newCond -> setRoot(root);
                newCond -> parseRoot = root;
                root -> addChild(newCond);
            } else {
                parent = nameToNode[parentName];
                if (dynamic_cast<Table*>(parent)) {
                    newCond -> addParent(parent);    
                    dynamic_cast<Table*>(parent) -> addChild(newCond);
                } else if (dynamic_cast<IfCondition*>(parent)) {
                    newCond -> addParent(parent);
                    if (evaluationResult) {
                        dynamic_cast<IfCondition*>(parent) -> addTrueChild(newCond);
                    } else {
                        dynamic_cast<IfCondition*>(parent) -> addFalseChild(newCond);
                    }
                    for (string field : dynamic_cast<IfCondition*>(parent) -> fieldsList ) {
                        newCond -> addField(field);
                    }
                }
            }
        }
        for (string field : fieldsList) {
            newCond -> addField(field);
        }
        set<string> output;
        output.insert(newCond -> name);
        // cout << "Done adding if condition" << endl;
        // printInformation();
        return output;
    }

    void setTableParents(string tableName, set<string> parents, bool evaluationResult) {
        tableName =  std::regex_replace(tableName, std::regex("^ +| +$|( ) +"), "$1");
        // cout << "Table name: " << tableName << endl;
        Table* table = dynamic_cast<Table*>(nameToNode[tableName]);
        // if (table == NULL) {
        //     cout << "table does not exist" << endl;
        // } else {
        //     cout << "Found table" << endl;
        // }
        NodeObject* parent;
        for (string parentName : parents) {
            parentName = std::regex_replace(parentName, std::regex("^ +| +$|( ) +"), "$1");
            // cout << "parent: " << parentName << endl;
            if (parentName == "parser") {
                table -> setRoot(root);
                table -> parseRoot = root;
                root -> addChild(table);
            } else {
                parent = nameToNode[parentName];
                if (dynamic_cast<Table*>(parent)) {
                    table -> addParent(parent);    
                    dynamic_cast<Table*>(parent) -> addChild(table);
                } else if (dynamic_cast<IfCondition*>(parent)) {
                    table -> addParent(parent);
                    if (evaluationResult) {
                        dynamic_cast<IfCondition*>(parent) -> addTrueChild(table);
                    } else {
                        dynamic_cast<IfCondition*>(parent) -> addFalseChild(table);
                    }
                }
            }
        }
        // printInformation();
    }

    void printInformation() {
        cout << "-------------" << endl;
        cout << root -> toString() << endl;
        for(auto mapIt = begin(nameToNode); mapIt != end(nameToNode); ++mapIt)
        {
            cout << mapIt -> first << " : " << flush;
            cout << mapIt->second->toString() << endl;
        }
        cout << "-------------" << endl;
    }

    void printDict(std::map<string, set<string> > fieldsDict) {
        cout << "*******START*******" << endl;
        for (const auto &p : fieldsDict) {
            cout << "-----------" << endl;
            cout << "key: " << p.first << " ";
            // for (const auto &tableDepend : p.second) {
            //     cout << "\ntable name: " << tableDepend.first << " dependencies: ";
            for (string dependent : p.second) {
                cout << dependent << ", " << endl;
            }
            // }
            cout << endl;
        }
        cout << "******END********" << endl;
    }

    void printMap(map<string, vector<string> > out) {
        cout << "Printing outmap \n";
        for(auto it = out.cbegin(); it != out.cend(); ++it)
        {
            std::cout << it->first << ": ";
            for (string value : (it->second)) {
                cout << value << ", ";
            }
            cout << endl;
        }
        cout << "Done Printing outmap" << endl;
    }

    std::map<string, vector<string> > removeVectorDuplicates(std::map<string, vector<string> > out) {
        vector<string> toDelete;
        int counter = 0;
        for (auto it = out.cbegin(); it != out.cend();) {
            auto jt = out.cbegin();
            jt = next(jt, counter + 1);
            while (jt != out.cend()) {
                if (isEqual(it-> second, jt->second)) {
                    toDelete.push_back(it->first);
                }
                jt++;
            }
            it++;
            counter += 1;
        }

        for (string key : toDelete) {
            out.erase(key);
        }
        return out;
    } 

    std::map<string, vector<string> > removeEmpty(std::map<string, vector<string> > out) {
        for (auto it = out.cbegin(); it != out.cend();) {
            if ((it -> second).size() == 0) {
                out.erase(it++);
            } else{
                it++;
            }
        }
        return out;
    }



    std::map<string, vector<string> > mergeConditions(std::map<string, vector<string> > out) {
        set<string> conditionSet;
        string pre = "IF_CONDITION_";
        for (auto it = out.cbegin(); it != out.cend();) {
            if ((it ->first).compare(0, pre.size(), pre) == 0) {
                for (string field : it -> second) {
                    conditionSet.insert(field);
                }
            }
            it++;
        }

        for (auto it = out.cbegin(); it != out.cend();) {
            if ((it ->first).compare(0, pre.size(), pre) == 0) {
                out.erase(it++);
            } else{
                it++;
            }
        }

        vector<string> conditionVector(conditionSet.begin(), conditionSet.end());
        if (conditionVector.size() > 0) {
            out[pre] = conditionVector;    
        }
        
        return out;
    }

    std::map<string, vector<string> > removeDuplicates(std::map<string, vector<string> > out) {
        for (auto& x : out) {
            sort( x.second.begin(), x.second.end() );
            x.second.erase( unique( x.second.begin(), x.second.end() ), x.second.end() );
        }
        return out;
    }

    std::map<string, vector<string> > splitLarge(std::map<string, vector<string> > out) {
        map<string, vector<string> >  newAdditions;
        // cout << "in splitLarge" << endl;
        for (auto it = out.cbegin(); it != out.cend();) {
            // cout << "See if any vector is large" << endl;
            if ((it->second).size() > 3) {
                // cout << "Going to break now" << it->first << endl;
                vector<vector<string> > chunks = split_vector((it->second), 3);
                // cout << "Broken up" << endl;
                for (int i = 0; i < chunks.size(); i++) {
                    newAdditions[(it -> first) + to_string(i)] = chunks[i];
                }
                out.erase(it++);
            } else {
                it++;    
            }
            
        }
        // cout << "Works till here 5" << endl;
        for (auto it = newAdditions.cbegin(); it != newAdditions.cend();) {
            out[it -> first] = it -> second;
            it++;
        }
        return out;
    }

    void addParserPaths(vector < std::map<string, int>  >_parserPaths) {
        // cout << "number of paths added: " << _parserPaths.size() << endl;
        parserPaths = _parserPaths;
    }

    std::map<string, vector<string> > getFields();
    std::map<string, vector<string> > removeMetadata(std::map<string, vector<string> > out, std::map<string, int>  validHeaders);
    void printTableToActions();
    
};
#endif