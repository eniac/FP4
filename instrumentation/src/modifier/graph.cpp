

// A class to represent a graph object
class Graph
{
public:
    map<string, vector<string>> adjList;
    map<string, Node*> nameToNode;
    map<string, int> headerStacks;
    // map<tuple<string, string>, tuple<string, string>> edgeConditions;
    Graph() {
    }

    void printGraph() {
        cout << "Start printGraph" << endl;
        for(auto it = adjList.cbegin(); it != adjList.cend(); ++it)
        {
            std::cout << it->first << ":";
            for (int i = 0; i < it->second.size(); i++) {
                std::cout << (it->second)[i] << ",";
            }
            std::cout << "\n";
        }
        cout << "End printGraph" << endl;
    }
    void addEdge(string src, string dst) {
        addNode(src);
        addNode(dst);
        // cout << "adding edge " << src << " to " << dst << endl;
        adjList[src].push_back(dst);
        // edgeConditions[make_tuple(src,dst)] = make_tuple(condition, value);
    }

    void addToStack(string headerName, int value) {
        headerStacks[headerName] = value;
    }

    void addNode(string state) {
        if (nameToNode.count(state) == 0) {
            nameToNode[state] = new Node();
            // cout << "adding node " << state << endl;
            nameToNode[state] -> state = state;
        }
    }
    void addExtract(string state, string header) {
        addNode(state);
        Node* curNode = nameToNode[state];
        // cout << "adding extract " << header << " to " << state << endl;
        curNode -> extract.push_back(header);
    }

    // This function assumes DAG
    vector<vector<string>> getExtractSequence() {
        // Start state of DAG
        string startState = "start";

        // End state of DAG
        string endState = "ingress";

        // Output of extract sequence
        vector<vector <string>> extractSeq;

        // Path list of states
        vector<vector <string>*> pathList;

        // Stack to maintain incomplete paths
        vector< vector<string>* > stack;

        // Current path
        vector<string>* curPath;

        // printGraph();        
        int visitedStates = 0;

        // Add edges (incomplete paths) from startState
        for(string i : adjList[startState]) {
            curPath = new vector<string>;
            curPath -> push_back(startState);
            curPath -> push_back(i);
            stack.push_back(curPath);

        }

        // While we have have incomplete path
        while (!stack.empty()) {
            // Take latest path in stack
            curPath = stack.front();

            // Remove it from stack
            // stack.pop_back();
            stack.erase(stack.begin());

            // cout << "Current Path: ";
            // for (string stateName: *curPath) {
            //     cout << stateName << ","; 
            // }
            // cout << endl;


            // If the last element of current path is end state, it means it is a complete path
            if (curPath -> back() == endState) {
                pathList.push_back(curPath);
                // cout << "Path is reaching end of state" << endl;
                continue;
            }

            // Else create new path by adding neighbour
            for(string i : adjList[curPath -> back()]) {
                vector<string>* newPath = new vector<string>(*curPath);
                // if (std::find(newPath -> begin(), newPath -> end(), i) != newPath -> end()) {
                //     continue;
                // }
                if (visitedStates > 100) {
                    continue;
                } else {
                    visitedStates += 1;
                }
                newPath -> push_back(i);
                // cout << "Adding new path: "; 
                // for (string stateName: *newPath) {
                //     cout << stateName << ","; 
                // }
                // cout << endl;
                stack.push_back(newPath);
            }
        }
        // for (vector<string>* path: pathList) {
        //     cout << "Path: ";
        //     for (string stateName: *path) {
        //         cout << stateName << ","; 
        //     }
        //     cout << endl;
        // }

        // int index = 0;
        // We have paths of state, we need the extract sequence now.
        for (vector<string>* path: pathList) {
            vector<string> newExtract;
            for (string stateName: *path) {
                for (string headerName: nameToNode[stateName] -> extract) {
                    if (headerStacks.find(headerName) != headerStacks.end()) {
                        newExtract.push_back(headerName + "*");
                    } else {
                        newExtract.push_back(headerName);    
                    }
                }
            }

            // for (int i = 0; i < (path -> size() - 1); i++) {
            //     condition[index].push_back(edgeConditions[make_tuple((*path)[i],(*path)[i+1])]);
            // }

            extractSeq.push_back(newExtract);
        }

        string prev = "";
        int count = 1;
        for (int i = 0; i < extractSeq.size(); i++) {
            for (auto it = extractSeq[i].begin(); it != extractSeq[i].end();) {
                if ((*it).back() == '*' && prev != (*it)) {
                    prev = (*it);
                    *it = removeLastChar(*it) + "*1*";
                    count = 2;
                    it++;
                } else if ((*it).back() == '*') {
                    vector<string> nameAndCount = split(*it, '*');
                    *(it-1) = nameAndCount[0] + "*" + to_string(count) + "*";
                    count += 1;
                    it = extractSeq[i].erase(it);
                } else {
                    prev = (*it);
                    it++;
                }
                
            }
        }
        // *conditionVector = condition;

        std::sort(extractSeq.begin(), extractSeq.end());
        extractSeq.erase(std::unique(extractSeq.begin(), extractSeq.end()), extractSeq.end());
        for (int i = 0; i < extractSeq.size(); ) {
            if (extractSeq[i].size() == 0) {
                extractSeq.erase(extractSeq.begin() + i);
            } else {
                ++i;
            }
        }

        bool remove = false;
        for (int i = 0; i < extractSeq.size(); ) {
            remove = false;
            for (int j = 0; j < extractSeq[i].size(); j++) {
                if (extractSeq[i][j].back() == '*') {
                    vector<string> nameAndCount = split(extractSeq[i][j], '*');
                    if (stoi(nameAndCount[1]) > headerStacks[nameAndCount[0]]) {
                        remove = true;
                        break;
                    }
                }
            }
            if (remove) {
                extractSeq.erase(extractSeq.begin() + i);
            } else {
                ++i;
            }
        }

        for (vector<string> path: extractSeq) {
            cout << "Extract Path: ";
            for (string stateName: path) {
                cout << stateName << ","; 
            }
            cout << endl;
        }

        return extractSeq;
    }
};