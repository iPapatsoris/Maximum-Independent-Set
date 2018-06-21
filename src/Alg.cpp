#include "Alg.hpp"
#include "unordered_set"

using namespace std;

Alg::Alg(const string &inputFile, const bool &checkIndependentSet) {
    SearchNode *root = new SearchNode(inputFile, checkIndependentSet);
    root->mis.setMisOutputFile(inputFile + ".mis");
    searchTree.push_back(root);
}

Alg::~Alg() {
    for (auto &searchNode: searchTree) {
        delete searchNode;
    }
}

Alg::SearchNode::SearchNode(const SearchNode &searchNode, const uint32_t &parentNode) {
    id = NONE;
    theta = searchNode.theta;
    branchingRule = BranchingRule();
    graph = searchNode.graph;
    mis = searchNode.mis;
    reductions = new Reductions(graph, mis);
    parent = parentNode;
    leftChild = NONE;
    rightChild = NONE;
    finalMis = NULL;
    hasCut = false;
    cutIsDone = false;
    cut = searchNode.cut;
    c1 = searchNode.c1;
    c2 = searchNode.c2;
    actualComponent1 = searchNode.actualComponent1;
}

Alg::SearchNode::~SearchNode() {
    delete reductions;
}

uint32_t searchNodeID = 0;

void Alg::run() {
    /*unordered_set<uint32_t> cut;
    vector<uint32_t> component1, component2;
    bool actualComponent1;
    searchTree[0]->graph.getSeparatingPairs(cut, component1, component2, actualComponent1);
    exit(0);*/

    //searchTree[0]->graph.print(true);
    //searchTree[0]->mis.print(searchTree[0]->graph.zeroDegreeNodes);
    uint32_t searchNodes = 1;
    uint32_t minCompletedSearchNode = NONE;
    bool down = true;
    uint32_t i = 0;
    while(true) {
        if (down) {
            if (searchTree[i]->theta == 5 && !searchTree[i]->hasCut && searchTree[i]->handleCuts()) {
                ;
            } else {
                searchTree[i]->reductions->run(searchTree[i]->theta);
                //cout << "search node " << searchNodes ;
                searchTree[i]->branchingRule.choose(searchTree[i]->graph, *(searchTree[i]->reductions), searchTree[i]->theta, searchTree[i]);
            }
        } else if (searchTree[i]->rightChild == NONE) {
            down = true;
            assert(searchTree[i]->branchingRule.type != BranchingRule::Type::DONE);
        }
        if (down && searchTree[i]->branchingRule.type == BranchingRule::Type::DONE) {
            searchTree[i]->id = searchNodeID++;
            searchTree[i]->finalMis = new vector<uint32_t>();
            searchTree[i]->mis.unfoldHypernodes(searchTree[i]->graph.zeroDegreeNodes, *searchTree[i]->finalMis);
            i = searchTree[i]->parent;
            if (i == NONE) {
                break;
            }
            down = false;
            continue;
        }
        uint32_t *nextChild;
        if (searchTree[i]->leftChild == NONE) {
            nextChild = &searchTree[i]->leftChild;
        } else if (searchTree[i]->rightChild == NONE) {
            nextChild = &searchTree[i]->rightChild;
        } else if (searchTree[i]->hasCut && !searchTree[i]->cutIsDone) {
            chooseCutBranch(searchTree[i]);
            nextChild = &searchTree[i]->rightChild;
            down = true;
        } else {
            if (i < minCompletedSearchNode) {
                minCompletedSearchNode = i;
                cout << i << endl;
            }
            SearchNode *parent = searchTree[i];
            if (parent->hasCut) {
                assert(parent->cutIsDone);
                concatMis(parent);
                //cout << "concating mis" << endl;
            } else {
                chooseMaxMis(parent);
            }
            i = parent->parent;
            if (searchTree.size() == 1) {
                break;
            }
            continue;
        }
        //cout << "Copying graph" << endl;
        SearchNode *searchNode = new SearchNode(*searchTree[i], i);
        searchNodes++;
        //cout << "Done" << endl;
        searchTree.push_back(searchNode);
        *nextChild = searchTree.size() - 1;
        if (nextChild == &searchTree[i]->leftChild) {
            branchLeft(searchTree[i]->branchingRule, searchNode, searchTree[i]->mis);
        }
        else if (nextChild == &searchTree[i]->rightChild) {
            branchRight(searchTree[i]->branchingRule, searchNode, searchTree[i]->mis);
        }
        else {
            assert(false);
        }
        i = *nextChild;
    }
    cout << searchNodes << " search nodes\n";
    //print();
    searchTree[0]->graph.collectZeroDegreeNodes();
    cout << "Final id " << searchTree[0]->id;
    Mis::print(*searchTree[0]->finalMis);
    delete searchTree[0]->finalMis;
}

bool Alg::SearchNode::handleCuts() {
    bool connected;
    if (graph.getArticulationPoints(cut, c1, c2, actualComponent1, connected) || connected && graph.getSeparatingPairs(cut, c1, c2, actualComponent1)) {
        branchingRule.type = BranchingRule::Type::CUT;
        hasCut = true;
        return true;
    } else {
        return false;
    }
}

void Alg::print() const {
    //searchTree[searchTree.size()-1]->graph.print(true);
    cout << searchTree.size();
    cout << "\n";
    return;
    for (uint32_t i = 0 ; i < searchTree.size() ; i++) {
        cout << "Search Node " << i << "\n---\n";
        searchTree[i]->print();
        cout << "\n";
    }
}

void Alg::SearchNode::print() const {
    cout << "Nodes: " << graph.getNodeCount() <<
    "\nParent: " << (parent == NONE ? "NONE" : to_string(parent)) << "\nLeft: " << (leftChild == NONE ? "NONE" : to_string(leftChild)) <<
    "\nRight: " << (rightChild == NONE ? "NONE" : to_string(rightChild));
    if (finalMis != NULL) {
        cout << "\nMis: " << finalMis->size();
    }
    cout << "\n";
}
