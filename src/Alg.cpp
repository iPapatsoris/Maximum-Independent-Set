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
    finalMis = NULL;
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
    "\nRight: " << (rightChild == NONE ? "NONE" : to_string(rightChild));
    if (finalMis != NULL) {
        cout << "\nMis: " << finalMis->size();
    }
    cout << "\n";
}

void Alg::run() {
    //searchTree[0]->mis.print(searchTree[0]->graph.zeroDegreeNodes);
    bool down = true;
    Graph::GraphTraversal graphTraversal(searchTree[0]->graph);
    uint32_t i = 0;
    while(true) {
        if (down) {
            searchTree[i]->reductions->run();
            graphTraversal = Graph::GraphTraversal(searchTree[i]->graph);
        } else if (searchTree[i]->rightChild == NONE) {
            down = true;
            graphTraversal = Graph::GraphTraversal(searchTree[i]->graph);
            assert(graphTraversal.curNode != NONE);
        }
        if (down && graphTraversal.curNode == NONE) {
            searchTree[i]->finalMis = new vector<uint32_t>();
            searchTree[i]->mis.unfoldHypernodes(searchTree[i]->graph.zeroDegreeNodes, *searchTree[i]->finalMis);
            i = searchTree[i]->parent;
            down = false;
            continue;
        }
        print();
        if (down) {
            cout << "node " << graphTraversal.curNode << "\n";
        }
        uint32_t *nextChild;
        if (searchTree[i]->leftChild == NONE) {
            nextChild = &searchTree[i]->leftChild;
        } else if (searchTree[i]->rightChild == NONE) {
            nextChild = &searchTree[i]->rightChild;
        } else {
            cout << "no next child" << endl;
            SearchNode *parent = searchTree[i];
            SearchNode *leftChild = searchTree[parent->leftChild];
            SearchNode *rightChild = searchTree[parent->rightChild];
            assert(parent->rightChild == searchTree.size() - 1 && parent->leftChild == searchTree.size() - 2);
            parent->finalMis = leftChild->finalMis;
            vector<uint32_t> *min = rightChild->finalMis;
            if (rightChild->finalMis->size() > parent->finalMis->size()) {
                parent->finalMis = rightChild->finalMis;
                min = leftChild->finalMis;
            }
            delete min;
            i = parent->parent;
            searchTree.pop_back();
            searchTree.pop_back();
            if (searchTree.size() == 1) {
                break;
            }
            continue;
        }
        SearchNode *searchNode = new SearchNode(*searchTree[i], i);
        searchTree.push_back(searchNode);
        *nextChild = searchTree.size() - 1;
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
    Mis::print(*searchTree[0]->finalMis);
    delete searchTree[0]->finalMis;
}
