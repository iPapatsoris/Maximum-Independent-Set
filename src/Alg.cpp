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
    //searchTree[searchTree.size()-1]->graph.print(true);
    cout << "\n";
    for (uint32_t i = 0 ; i < searchTree.size() ; i++) {
        cout << "Search Node " << i << "\n---\n";
        searchTree[i]->print();
        cout << "\n";
    }
}

void Alg::SearchNode::print() const {
    cout << "Nodes: " << graph.getNodeCount() <<
    "\nParent: " << (parent == NONE ? "NONE" : to_string(parent)) << "\nLeft: " << (leftChild == NONE ? "NONE" : to_string(leftChild)) <<
    "\nRight: " << (rightChild == NONE ? "NONE" : to_string(rightChild)) <<
    "\nValue: " << (value == NONE ? "NONE" : to_string(value)) << "\n";
}

void Alg::run() {
    //searchTree[0]->mis.print(searchTree[0]->graph.zeroDegreeNodes);
    bool up = false;
    uint32_t i = 0;
    while(1) {
        searchTree[i]->reductions->run();
        print();
        uint32_t *nextChild;
        if (searchTree[i]->leftChild == NONE) {
            nextChild = &searchTree[i]->leftChild;
        } else if (searchTree[i]->rightChild == NONE) {
            nextChild = &searchTree[i]->rightChild;
        } else {
            cout << "no next child" << endl;
        }
        SearchNode *searchNode = new SearchNode(*searchTree[i], i);
        searchTree.push_back(searchNode);
        *nextChild = searchTree.size() - 1;

        Graph::GraphTraversal graphTraversal(searchNode->graph);
        if (graphTraversal.curNode == NONE) {

        }
        cout << "node " << graphTraversal.curNode << "\n";
        if (nextChild == &searchTree[i]->leftChild) {
            //cout << "left\n";
            searchNode->mis.getMis().push_back(graphTraversal.curNode);
            vector<uint32_t> neighbors;
            neighbors.push_back(graphTraversal.curNode);
            searchNode->graph.gatherNeighbors(graphTraversal.curNode, neighbors);
            searchNode->graph.remove(neighbors, searchNode->reductions->getReduceInfo());
        }
        else if (nextChild == &searchTree[i]->rightChild) {
            //cout << "right\n";
            searchNode->graph.remove(graphTraversal.curNode, searchNode->reductions->getReduceInfo());
        }
        else {
            assert(false);
        }
        i = *nextChild;

    }
    print();
}
