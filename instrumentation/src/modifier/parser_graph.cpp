#include "../../include/parser_graph.h"
#include "../../util/util.cpp"

using namespace std;

ParserGraph::ParserGraph() {
    nameToState["ingress"] = new State("ingress");
    visitedStates = 0;
}

void ParserGraph::addState(string name) {
    if (nameToState.count(name) == 0) {
        nameToState[name] = new State(name);
        // cout << "Adding state: " << name << endl;
    }
}

void ParserGraph::addDefaultTransition(string _fromState, string _toState) {
    State* curState = nameToState[_fromState];
    curState -> defaultTransition = _toState;
}
void ParserGraph::addMatchField(string stateName, string headerField) {
    State* curState = nameToState[stateName];
    curState -> addMatchHeader(headerField);    
}

void ParserGraph::addMatchCurrent(string stateName, int _start, int _end) {
    State* curState = nameToState[stateName];
    curState -> addMatchCurrent(_start, _end);
}

void ParserGraph::addConditionTransition(string _fromState, string _toState, string value) {
    State* curState = nameToState[_fromState];
    Transition* transition = new Transition(_toState, value);
    curState -> addTransition(transition);
}

void ParserGraph::addConditionMaskTransition(string _fromState, string _toState, string value, string mask) {
    State* curState = nameToState[_fromState];
    Transition* transition = new Transition(_toState, value, mask);
    curState -> addTransition(transition);
}

void ParserGraph::addExtract(string stateName, string headerField) {
    State* curState = nameToState[stateName];
    curState -> addHeader(headerField);
}

void ParserGraph::addToStack(string headerName, int value) {
    headerStacks[headerName] = value;
    // cout << "Adding header to stack " << headerName << " : " << value << endl;
}

void ParserGraph::addMetadata(string stateName, string leftSide, ExprNode* rightSide) {
    State* curState = nameToState[stateName];
    if (curState -> extractedHeaders.size() > 0) {
        string latest_header = curState -> extractedHeaders.back();
        if (leftSide.rfind("latest.", 0) == 0) {
            leftSide.replace(0,7, latest_header + ".");
        }
    }
    curState -> setMetadata[leftSide] = rightSide;
}

vector<Path* > ParserGraph::addPathToStack(vector<Path* > stack, State* latestState, Path* curPath, map<string, HeaderTypeDeclarationNode*> headerInformation) {
    Path* newPath;
    vector<string> headerField;
    string headerToMatch;
    string fieldToMatch;
    // cout << "latest state: " << latestState -> name << endl;
    // cout << "latest state: " << visitedStates << endl;
    for(Transition* i : latestState -> transitions) {
        if (visitedStates > 100) {
            continue;
        } else {
            visitedStates += 1;
        }
        newPath = new Path(*curPath);
        newPath -> push_back(i -> toState);
        // cout << "New Path: ";
        // for (string stateName: newPath -> states) {
        //     cout << stateName << ","; 
        // }
        // cout << endl;
        // cout << endl;
        if (latestState -> matchWith.matchType == 1) {
            // cout << "here 3" << endl;
            newPath -> futureValue = i -> value;
            // cout << "setting future value: " << i -> value << endl;
            // cout << "here 4" << endl;
        } else {
            // cout << "here 5" << endl;
            headerField = split(latestState -> matchWith.headerField, '.');
            headerToMatch = headerField[0];
            fieldToMatch = headerField[1];
            // cout << "headerToMatch" << headerToMatch << endl;
            if (headerToMatch == "latest") {
                headerToMatch = newPath -> extractedHeaders.back();
                if (headerStacks.count(headerToMatch)) {
                    headerToMatch += "[" + to_string((curPath -> headerStacks[headerToMatch]) - 1) + "]";
                }
            } ;

            newPath -> fixedValues[headerToMatch + "." + fieldToMatch] = i -> value;
        }
        if (!(newPath -> futureValue.empty()) && nameToState[i -> toState] -> extractedHeaders.size() > 0) {
            headerToMatch = nameToState[i -> toState] -> extractedHeaders[0];
            HeaderTypeDeclarationNode* headerNode = headerInformation[headerToMatch];
            fieldToMatch = headerInformation[headerToMatch] -> field_decs_ -> list_ -> at(0) -> name_ -> toString();
            newPath -> fixedValues[headerToMatch + "." + fieldToMatch] = newPath -> futureValue;
            newPath -> futureValue = "";
        // } else {
            // cout << "condition failed" << endl;
        }
        stack.push_back(newPath);
    }
    if (visitedStates <= 100) {
        newPath = new Path(*curPath);
        newPath -> push_back(latestState -> defaultTransition);
        newPath -> futureValue = "";
        stack.push_back(newPath);
        visitedStates += 1;
    }

    return stack;
}

void ParserGraph::generateExtractSequence(map<string, HeaderTypeDeclarationNode*> headerInformation) {
    // Start state of DAG
    string startState = "start";

    // End state of DAG
    string endState = "ingress";

    // Path list of states
    vector<Path*> pathList;

    // Stack to maintain incomplete paths
    vector<Path* > stack;

    // Current path
    Path* curPath = new Path();
    curPath -> push_back(startState);

    // New path
    Path* newPath;

    // Current State
    State* latestState = nameToState[startState];

    // printGraph();        
    // int visitedStates = 0;

    bool invalidPath = false;

    stack.push_back(curPath);

    // While we have have incomplete path
    while (!stack.empty()) {
        // Take latest path in stack
        curPath = stack.front();

        // Remove it from stack
        // stack.pop_back();
        stack.erase(stack.begin());

        // cout << "Current Path: ";
        // for (string stateName: curPath -> states) {
        //     cout << stateName << ","; 
        // }
        // cout << endl;
        // cout << "end of current path" << endl;

        // If the last element of current path is end state, it means it is a complete path
        if (curPath -> back() == endState) {
            pathList.push_back(curPath);
            // cout << "Path is reaching end of state" << endl;
            continue;
        }

        // cout << "finding latest state" << endl; 
        latestState = nameToState[curPath -> back()];
        // cout << "current state: " << latestState -> name << endl;

        invalidPath = false;
        // TODO: Iterate metadata info of state first, if latest, then replace and add, else only add
        for (auto const& x : latestState -> setMetadata) {
            string temp = x.first;
            if (x.first.rfind("latest.", 0) == 0) {
                const string replacement = curPath -> extractedHeaders.back() + ".";
                temp.replace(0,7, replacement);
            }
            // curPath -> metadataInfo["latest"] = curPath -> extractedHeaders.back();
            // ExprReturnType output;
            // if (curPath -> extractedHeaders.size() > 0) {
            //     output = x.second -> evaluate(curPath -> extractedHeaders, curPath -> extractedHeaders.back());
            // } else {
            //     output = x.second -> evaluate(curPath -> extractedHeaders);
            // }
            vector<string> field_list;
            field_list = x.second -> GetFields(field_list);
            for (string field : field_list) {
                if (field.rfind("latest.", 0) == 0) {
                    string replacement = curPath -> extractedHeaders.back() + ".";
                    field.replace(0,7, replacement);
                }
                curPath -> metadataDependency[x.first].push_back(field);
                // cout << "Adding to metadataDependency: " << x.first << " with field: " << field << endl;
            }
            
            // if (output.mode == ExprReturnMode::ARITH) {
            //     curPath -> metadataInfo[temp] = output.arithValue;
            // }
            
        }
        // cout << "after metadata loop" << endl;

        for (string headerName: latestState -> extractedHeaders) {
            if (headerStacks.find(headerName) != headerStacks.end()) {
                // cout << "header is a stack: " << headerName << endl;
                if (curPath -> headerStacks.find(headerName) == curPath -> headerStacks.end()) {
                    // cout << "currently not in path" << endl;
                    curPath -> headerStacks[headerName] = 0;
                }
                curPath -> headerStacks[headerName] += 1;
                if (curPath -> headerStacks[headerName] > headerStacks[headerName]) {
                    invalidPath = true;
                    break;
                }
            // } else {
            //     cout << "header is not a stack: " << headerName << endl;
            }
            curPath -> extractedHeaders.push_back(headerName); 
        }
        // cout << "After extractedHeaders loop" << endl;
        if (invalidPath) {
            // cout << "path is invalid" << endl;
            continue;
        }
        // cout << "Adding addPathToStack" << endl;
        stack = addPathToStack(stack, latestState, curPath,headerInformation);
        // cout << "Done Adding addPathToStack" << endl;

    }
    // cout << "out of while loop" << endl;

    // We have paths of state, we need to mark header stacks
    for (Path* path: pathList) {
        for (int i = 0; i < path -> extractedHeaders.size(); ++i)
        {
            if (headerStacks.find(path -> extractedHeaders[i]) != headerStacks.end()) {
                path -> extractedHeaders[i] += "*";
            } 
        }
    }

    // Merging header stacks
    string prev = "";
    int count = 1;
    for (int i = 0; i < pathList.size(); i++) {
        for (auto it = pathList[i] -> extractedHeaders.begin(); it != pathList[i] -> extractedHeaders.end();) {
            if ((*it).back() == '*' && prev != (*it)) {
                prev = (*it);
                *it = removeLastChar(*it) + "*1*";
                count = 2;
                it++;
            } else if ((*it).back() == '*') {
                vector<string> nameAndCount = split(*it, '*');
                *(it-1) = nameAndCount[0] + "*" + to_string(count) + "*";
                count += 1;
                it = pathList[i] -> extractedHeaders.erase(it);
            } else {
                prev = (*it);
                it++;
            } 
        }
    }


    // Remove repeated paths
    std::sort(pathList.begin(), pathList.end());
    pathList.erase(std::unique(pathList.begin(), pathList.end()), pathList.end());
    bool remove = false;
    for (int i = 0; i < pathList.size(); ) {
        remove = false;
        for (int j = 0; j < pathList[i] -> extractedHeaders.size(); j++) {
            if (pathList[i] -> extractedHeaders[j].back() == '*') {
                vector<string> nameAndCount = split(pathList[i] -> extractedHeaders[j], '*');
                if (stoi(nameAndCount[1]) > headerStacks[nameAndCount[0]]) {
                    remove = true;
                    break;
                }
            }
        }
        if (remove) {
            pathList.erase(pathList.begin() + i);
        } else {
            ++i;
        }
    }

    // Move in extracted sequences
    for (Path* path : pathList) {
        map<string, int> currentPathHeaders;
        for (string header : path -> extractedHeaders) {
            if (currentPathHeaders.count(header) == 0) {
                currentPathHeaders[header] = 0;
            }
            currentPathHeaders[header] += 1;
        }
        validHeaders.push_back(currentPathHeaders);
        extractSequence.push_back(path -> extractedHeaders);
        // metadataInfo.push_back(path -> metadataInfo);
        for (auto const& x : path -> metadataDependency) {
            RemoveDuplicatesKeepOrder(x.second);
        }
        // cout << "size of path metadataDependency: " << path -> metadataDependency.size() << endl;
        metadataDependency.push_back(path -> metadataDependency);
    }

    // for (vector<string> path: extractSequence) {
    //     cout << "Extract Path: ";
    //     for (string stateName: path) {
    //         cout << stateName << ","; 
    //     }
    //     cout << endl;
    // }

    for (int i = 0; i < pathList.size(); i++) {
        string name = CombineHeader(pathList[i] -> extractedHeaders);
        // setValues[]
        for (auto const& x : pathList[i] -> fixedValues) {
            setValues[name][x.first] = x.second;
        }
    }

    // cout << "Fixed Values dict" << endl;
    // for (auto const& x : setValues) {
    //     std::cout << x.first 
    //               << " ==> ";
    //     for (auto const& y : x.second) {
    //         cout << y.first << ": " << y.second << ", ";
    //     }
    //     cout << std::endl;
    // }
    // exit(0);

}