#include "../../include/static_analysis.h"
#include "../../util/util.cpp"

void  DAG::printTableToActions() {
    Table* currentTable;
    for (auto const& x : nameToNode) {
        if (currentTable = dynamic_cast<Table*>(x.second)) {
            cout << "Table: " << currentTable -> name;
            cout << " Actions: ";
            for (Action* action : currentTable -> actionList) {
                cout << action -> name << ",";
            }
            cout << endl;
        }
    }
}

std::map<string, vector<string> > DAG::getFields() {
    queue<PathInformation*> pathQueue;
    PathInformation* currentPath;
    PathInformation* newPath;
    Table* currentTable;
    IfCondition* ifcond;
    vector<PathInformation*> finishedPaths;
    std::map<string, vector<string> > out;

    for (int i = 0; i < parserPaths.size(); i++) {
        // vector<string> parserPath = parserPaths[i];
        currentPath = new PathInformation();
        // cout << "Adding parser number: " << i << endl;
        currentPath -> actionPath.push_back("parser_" + to_string(i));
        // cout << "TODO: In getFields function of class DAG. File: static_analysis.cpp" << endl;
        // TODO: There is a bit of an issue here with no ordering of metadatainfo. To resolve it, change the type of metadatainfo to ordered_map (https://github.com/Tessil/ordered-map) that preserves the insertion order. The type also needs to be changed in parser_graph.cpp. Would not have had this problem if STL map in C++ was more like python ¯\_(ツ)_/¯.
        // cout << "Path number: " << i << endl;
        for (auto const& x : metadataDependency[i]) {
            // vector<string> field_list;
            // field_list = x.second -> GetFields(field_list);
            // cout << "here 10" << endl;
            // cout << "size: " << metadataDependency[i].size() << endl;
            // cout << "x.first: " << x.first << endl;
            for (string field : x.second) {
                // cout << "here 20" << endl;
                if (currentPath -> fieldsDict.count(field) == 0) {
                    // cout << "here 30" << endl;
                    currentPath -> fieldsDict[x.first].insert(field);
                    // cout << "here 40" << endl;
                } else {
                    // cout << "here 50" << endl;
                    currentPath -> fieldsDict[x.first].insert(currentPath -> fieldsDict[field].begin(), currentPath -> fieldsDict[field].end());
                    // cout << "here 60" << endl;
                }
                // cout << "here 70" << endl;
            }
            // cout << "here 80" << endl;
        }

        // cout << "After first for loop" << endl;
        currentPath -> validHeaders = parserPaths[i];
        // for (string header : parserPath) {
        //     cout << header << ",";

            // currentPath -> validHeaders.insert(header);
            // currentPath -> valuesSet[header + ".valid"] = 1;
        // }
        // cout << "\nAfter second for loop" << endl;

        // for (auto const& x : metadataFromParser[i]) {
        //     currentPath -> valuesSet[x.first] = x.second;
        // }
        // cout << "After third for loop" << endl;

        for (NodeObject* child: root -> children) {
            newPath = new PathInformation(*currentPath);
            newPath -> nextToExecute = child;
            // cout << "Adding to queue" << endl;
            pathQueue.push(newPath);
        }
        // cout << "After fourth for loop" << endl;
    }
    // cout << "going in while loop now" << endl;
    while (!pathQueue.empty()) {
        // cout << "in path queue" << endl;
        currentPath = pathQueue.front();
        pathQueue.pop();
        // cout << "next to nextToExecute: " << currentPath -> nextToExecute -> name << endl;
        if (currentTable = dynamic_cast<Table*>(currentPath -> nextToExecute)) {
            // Create output before table execution
            // cout << "currentTable: " << currentTable -> name << endl;
            // if (currentTable -> name == "tiHandleIpv4") {
                // cout << "current path: ";
                // for (string action : currentPath -> actionPath) {
                //     cout << action << ",";
                // }
                // cout << endl;
                // printDict(currentPath -> fieldsDict);
                // exit(0);
            // }
            for (string key : currentTable -> keys) {
                currentPath -> out[currentTable -> name].push_back(key);
                if (currentPath -> fieldsDict.count(key) > 0) {
                    // cout << "Key does not exist" << endl;
                    for (string dependent : currentPath -> fieldsDict[key]) {
                        // cout << dependent << ",";
                        currentPath -> out[currentTable -> name].push_back(dependent);
                    }
                }
            }
            for (Action* action: currentTable -> actionList) {
                // Execution of an action
                newPath = new PathInformation(*currentPath);
                for (auto addedPair : action -> modifiedPairs) {
                    newPath -> fieldsDict[addedPair.first].insert(addedPair.second);
                    if (newPath -> fieldsDict.count(addedPair.second) > 0) {
                        newPath -> fieldsDict[addedPair.first].insert(newPath -> fieldsDict[addedPair.second].begin(), newPath -> fieldsDict[addedPair.second].end());
                    }
                    for (string key : currentTable -> keys) {
                        newPath -> fieldsDict[addedPair.first].insert(key);
                        if (newPath -> fieldsDict.count(key) > 0) {
                            newPath -> fieldsDict[addedPair.first].insert(newPath -> fieldsDict[key].begin(), newPath -> fieldsDict[key].end());
                        }
                    }
                }
                for (string field : action -> modified) {
                    for (string key : currentTable -> keys) {
                        newPath -> fieldsDict[field].insert(key);
                        if (newPath -> fieldsDict.count(key) > 0) {
                            // cout << "Adding dependency to: " << field << " in table: " << table -> name << " from key: " << key << endl; 
                            
                            newPath -> fieldsDict[field].insert(newPath -> fieldsDict[key].begin(),newPath -> fieldsDict[key].end());
                        }
                    }
                }
                
                // action -> rawAction -> evaluate(newPath -> valuesSet);
                action -> evaluate(newPath -> validHeaders);
                newPath -> actionPath.push_back(action -> name);
                // if (action -> name == "aiForMe") {
                //     printDict(newPath -> fieldsDict);
                //     exit(0);
                // }
                if (action -> drop || currentTable -> children.size() == 0) {
                    finishedPaths.push_back(newPath);
                    // cout << "Not adding children" << endl;
                    // cout << "number of children: " << currentTable -> children.size() << endl;
                    // cout << "actionName: " << action -> name << endl;
                } else {
                    for (NodeObject* child: currentTable -> children) {
                        newPath = new PathInformation(*newPath);
                        newPath -> nextToExecute = child;
                        // cout << "Adding child " << child -> name << endl;
                        pathQueue.push(newPath);
                    }
                    // cout << "Done adding child" << endl;
                }
                // if (currentTable -> children.size() == 0) {
                //     finishedPaths.push_back(newPath);
                // }
            }
        } else if (ifcond = dynamic_cast<IfCondition*>(currentPath -> nextToExecute)) {
            // if (ifcond -> name == "IF_CONDITION_1") {
            //     printDict(currentPath -> fieldsDict);
                // exit(0);
            // }
            ExprReturnType output = ifcond -> rawCondition -> evaluate(currentPath -> validHeaders);
            // cout << "In ifCondition number" << ifcond -> name << ": " ;
            // for (string field: ifcond -> fieldsList) {
            //     cout << field << " , ";
            // }
            // cout << endl;
            for (string key : ifcond -> fieldsList) {
                currentPath -> out[ifcond -> name].push_back(key);
                if (currentPath -> fieldsDict.count(key) > 0) { 
                    // cout << "key in fieldsDict: " << key << endl;
                    for (string dependent : currentPath -> fieldsDict[key]) {
                        currentPath -> out[ifcond -> name].push_back(dependent);
                    }
                // } else {
                //     cout << "key not in fieldsDict, key: " << key << endl;
                }
            }

            newPath -> actionPath.push_back(ifcond -> name);
            

            if (output.mode == ExprReturnMode::FALSE) {
                for (NodeObject* child : ifcond -> falseChildren) {
                    newPath = new PathInformation(*currentPath);
                    newPath -> nextToExecute = child;
                    pathQueue.push(newPath);
                }
                if (ifcond -> falseChildren.size() == 0) {
                    finishedPaths.push_back(currentPath);
                }

            } else if (output.mode == ExprReturnMode::TRUE) {
                for (NodeObject* child : ifcond -> trueChildren) {
                    newPath = new PathInformation(*currentPath);
                    newPath -> nextToExecute = child;
                    pathQueue.push(newPath);
                }
                if (ifcond -> trueChildren.size() == 0) {
                    finishedPaths.push_back(currentPath);
                }

            } else {
                for (NodeObject* child : ifcond -> trueChildren) {
                    newPath = new PathInformation(*currentPath);
                    newPath -> nextToExecute = child;
                    pathQueue.push(newPath);
                }
                for (NodeObject* child : ifcond -> falseChildren) {
                    newPath = new PathInformation(*currentPath);
                    newPath -> nextToExecute = child;
                    pathQueue.push(newPath);
                }
                if (ifcond -> trueChildren.size() == 0 && ifcond -> falseChildren.size() == 0) {
                    finishedPaths.push_back(currentPath);
                }
            }
        }
    }
    // cout << "Out of while loop" << endl;

    for (PathInformation* path : finishedPaths) {
        // cout << "Path: ";
        // for (string action : path -> actionPath) {
        //     cout << action << ",";
        // }
        // cout << endl;
        path -> out = removeMetadata(path -> out, path -> validHeaders);
        path -> out = removeEmpty(path -> out);
        path -> out = removeDuplicates(path -> out);
        path -> out = splitLarge(path -> out);
        path -> out = removeVectorDuplicates(path -> out);
        if (path -> out.size() > 1) {
            for (auto const& x : path -> out) {
                for (string field : x.second) {
                    out[x.first + "_" + (path -> actionPath[0])].push_back(field);
                }
            }
        } else {
            for (auto const& x : path -> out) {
                for (string field : x.second) {
                    out[x.first].push_back(field);
                }
            }
        }
    }
    // printMap(out);
    // cout << "Removing metadata" << endl;
    // out = removeMetadata(out);
    // printMap(out);
    // cout << "Removing empty" << endl;
    out = removeEmpty(out);
    // cout << "Removing duplicates" << endl;
    out = removeDuplicates(out);
    // printMap(out);
    // cout << "Splitting large" << endl;
    out = splitLarge(out);
    // printMap(out);
    // cout << "Removing vector duplicates" << endl;
    out = removeVectorDuplicates(out);
    // printMap(out);

    return out;
}

std::map<string, vector<string> > DAG::removeMetadata(std::map<string, vector<string> > out, std::map<string, int>  validHeaders) {
    for (auto& x : out) {
        vector< string >::iterator it = x.second.begin();
        while(it != x.second.end()) {
            vector<string> headerField = split(*it, '.');
            bool found = false;

            for (auto const& header : validHeaders) {
                if (headerField[0] == header.first && header.second > 0) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                it = x.second.erase(it);
            } else {
                it++;
            }
        }
    }
    return out;
}