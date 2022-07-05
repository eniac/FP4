#ifndef PARSER_GRAPH_H
#define PARSER_GRAPH_H

#include <tuple>
#include <string>
#include <vector>
#include <map>

#include "ast_nodes_p4.hpp"

using namespace std;

struct matchFieldOrCurrent {
    int matchType;
    string headerField;
    int start;
    int end;
};

class Transition {
public:
    Transition(string _toState, string _value, string _mask = {}) {
        toState = _toState;
        // if (_value == "default" || _value.empty()) {
        //     conditionType = TransitionType::NOCONDITION;
        // } else if (_current) {
        //     rangeStart = _rangeStart;
        //     rangeEnd = _rangeEnd;
        //     conditionType = TransitionType::CURRENT;
        // } else {
        value = _value;
        mask = _mask;
        //     conditionType = TransitionType::CONDITION;
        // }
    }
    string toState;
    string value;
    string mask;
};

class Path {
public:
    Path() {}

    Path(const Path &p1) {
        fixedValues = map<string, string> (p1.fixedValues);
        futureValue = p1.futureValue;
        states = vector<string>(p1.states);
        extractedHeaders = vector<string>(p1.extractedHeaders);
        headerStacks = map<string, int> (p1.headerStacks);
    }

    void push_back(string stateName) {
        states.push_back(stateName);
    }
    bool operator < (const Path& p1) const {
        return (extractedHeaders < p1.extractedHeaders);
    }

    bool operator == (const Path& p1) const {
        return (extractedHeaders == p1.extractedHeaders);
    }

    string back() {
        return states.back();
    }
    map<string, string> fixedValues;
    string futureValue;
    vector<string> states;
    vector<string> extractedHeaders;
    std::map<string, int> headerStacks;
    // map<string, uint64_t> metadataInfo;
    map<string, vector<string> > metadataDependency;
};

class State {
public:
    State(string _name) {
        name = _name;
        // matchWith = {};
    }
    void addTransition(Transition* _transition) {
        transitions.push_back(_transition);
    }

    void addMatchHeader(string _headerField) {
        // matchFieldOrCurrent currentMatch;
        matchWith.matchType = 0;
        matchWith.headerField = _headerField;
        // matchWith.push_back(currentMatch);
    }

    void addMatchCurrent(int _start, int _end) {
        // matchFieldOrCurrent currentMatch;
        matchWith.matchType = 1;
        matchWith.start = _start;
        matchWith.end = _end;
        // matchWith.push_back(currentMatch);
    }

    void addHeader(string name) {
        extractedHeaders.push_back(name);
    }

    string name;
    vector<Transition*> transitions;
    vector<string> extractedHeaders;
    matchFieldOrCurrent matchWith;
    string defaultTransition;
    map<string, ExprNode*> setMetadata;
};

class ParserGraph {
public:
    map<string, State*> nameToState;
    map<string, int> headerStacks;
    int visitedStates;
    vector<vector<string>> extractSequence;
    map<string, map<string, string> >  setValues;
    vector<map<string, int> > validHeaders;
    vector<map<string, vector<string> > > metadataDependency;
    ParserGraph();
    void addConditionTransition(string _fromState, string _toState, string value);
    void addConditionMaskTransition(string _fromState, string _toState, string value, string mask);
    void addDefaultTransition(string _fromState, string _toState); 
    void addExtract(string stateName, string headerField);
    void addMatchCurrent(string stateName, int _start, int _end);
    void addMatchField(string stateName, string headerField);
    void addMetadata(string stateName, string leftSide, ExprNode* rightSide);
    vector<Path* > addPathToStack(vector<Path* > stack, State* latestState, Path* curPath,  map<string,HeaderTypeDeclarationNode*> headerInformation);
    void addState(string name);
    void addToStack(string headerName, int value);
    void generateExtractSequence(map<string, HeaderTypeDeclarationNode*> headerInformation);
};

#endif