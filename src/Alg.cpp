#include "Alg.hpp"

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
    value = NONE;
}

Alg::SearchNode::~SearchNode() {
    delete reductions;
}

void Alg::print() const {
    for (uint32_t i = 0 ; i < searchTree.size() ; i++) {
        cout << "Search Node " << i << "\n---\n";
        searchTree[i]->print();
        cout << "\n";
    }
}

void Alg::SearchNode::print() const {
    cout << "Nodes: " << graph.getNodeCount() <<
    "\nParent: " << parent << "\nLeft: " << leftChild <<
    "\nRight: " << rightChild <<
    "\nValue: " << value << "\n";
}

void Alg::run() {
    searchTree[0]->reductions->run();
    searchTree[0]->mis.print(searchTree[0]->graph.zeroDegreeNodes);

    print();
}
