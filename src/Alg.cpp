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

void Alg::run() {
    //searchTree[0]->mis.print(searchTree[0]->graph.zeroDegreeNodes);
    uint32_t searchNodes = 1;
    uint32_t minCompletedSearchNode = NONE;
    bool down = true;
    BranchingRule branchingRule;
    uint32_t i = 0;
    uint32_t theta = 8;
    while(true) {
        if (down) {
            searchTree[i]->reductions->run();
            branchingRule.choose(searchTree[i]->graph, theta);
        } else if (searchTree[i]->rightChild == NONE) {
            down = true;
            branchingRule.choose(searchTree[i]->graph, theta);
            assert(branchingRule.type != BranchingRule::Type::DONE);
        }
        if (down && branchingRule.type == BranchingRule::Type::DONE) {
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
        if (down) {
            //cout << "node " << branchingRule.node1 << "\n";
        }
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
            branchLeft(branchingRule, searchNode);
        }
        else if (nextChild == &searchTree[i]->rightChild) {
            branchRight(branchingRule, searchNode);
        }
        else {
            assert(false);
        }
        i = *nextChild;
    }
    cout << searchNodes << " search nodes\n";
    //print();
    Mis::print(*searchTree[0]->finalMis);
    delete searchTree[0]->finalMis;
}

void Alg::BranchingRule::findOptimalShortEdge(const Graph &graph) {
    uint32_t node1 = NONE;
    uint32_t node2 = NONE;
    //unor
    Graph::GraphTraversal graphTraversal(graph);


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
