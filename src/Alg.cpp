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
}

Alg::SearchNode::~SearchNode() {
    delete reductions;
}

uint32_t searchNodeID = 0;

void Alg::run() {
    searchTree[0]->graph.getArticulationPoints();
    return;
    //searchTree[0]->graph.print(true);
    //searchTree[0]->mis.print(searchTree[0]->graph.zeroDegreeNodes);
    uint32_t searchNodes = 1;
    uint32_t minCompletedSearchNode = NONE;
    bool down = true;
    uint32_t i = 0;
    while(true) {
        if (down) {
            searchTree[i]->reductions->run(searchTree[i]->theta);
            //cout << "search node " << searchNodes ;
            searchTree[i]->branchingRule.choose(searchTree[i]->graph, *(searchTree[i]->reductions), searchTree[i]->theta);
        } else if (searchTree[i]->rightChild == NONE) {
            down = true;
            assert(searchTree[i]->branchingRule.type != BranchingRule::Type::DONE);
        }
        if (down && searchTree[i]->branchingRule.type == BranchingRule::Type::DONE) {
            searchTree[i]->id = searchNodeID++;
            //cout << "Id " << searchTree[i]->id;
            searchTree[i]->finalMis = new vector<uint32_t>();
            searchTree[i]->mis.unfoldHypernodes(searchTree[i]->graph.zeroDegreeNodes, *searchTree[i]->finalMis);
            i = searchTree[i]->parent;
            if (i == NONE) {
                break;
            }
            down = false;
            continue;
        }
        //print();
        /*if (down && minCompletedSearchNode == 42) {
            cout << "node " << searchTree[i]->branchingRule.node1 << "\n";
        }*/
        uint32_t *nextChild;
        if (searchTree[i]->leftChild == NONE) {
            nextChild = &searchTree[i]->leftChild;
        } else if (searchTree[i]->rightChild == NONE) {
            nextChild = &searchTree[i]->rightChild;
        } else {
            if (i < minCompletedSearchNode) {
                minCompletedSearchNode = i;
                cout << i << endl;
            }
            SearchNode *parent = searchTree[i];
            chooseMaxMis(parent);
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
            branchLeft(searchTree[i]->branchingRule, searchNode);
        }
        else if (nextChild == &searchTree[i]->rightChild) {
            branchRight(searchTree[i]->branchingRule, searchNode);
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
