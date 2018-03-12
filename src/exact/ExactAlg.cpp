#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "ExactAlg.hpp"

using namespace std;

void ExactAlg::run() {
    this->reduce(6, 4);
}

void ExactAlg::reduce(const int &degree, const int &cliqueSize) {
    Graph newGraph;
    unordered_map<uint32_t, uint32_t> *idToPos = new unordered_map<uint32_t, uint32_t>();
    vector<uint32_t> *posToId = new vector<uint32_t>();
    vector<Graph::NodeInfo> &newNodeIndex = newGraph.getNodeIndex();
    vector<uint32_t> &newEdgeBuffer = newGraph.getEdgeBuffer();
    vector<Graph::NodeInfo> &nodeIndex = graph.getNodeIndex();
    vector<uint32_t> &edgeBuffer = graph.getEdgeBuffer();
    for (uint32_t node = 0 ; node < nodeIndex.size() ; node++) {
        if (1 || nodeIndex[node].getEdges() == degree) {
            uint32_t newOffset = newEdgeBuffer.size();
            uint32_t newEdges = 0;
            for (uint32_t offset = nodeIndex[node].getOffset() ; offset < nodeIndex[node].getOffset() + nodeIndex[node].getEdges() ; offset++) {
                if (1 || nodeIndex[edgeBuffer[offset]].getEdges() == degree) {
                    newEdgeBuffer.push_back(edgeBuffer[offset]);
                    newEdges++;
                }
            }
            if (newEdges) {
                idToPos->insert({node, newNodeIndex.size()});
                posToId->push_back(node);
                newNodeIndex.push_back(Graph::NodeInfo(newOffset, newEdges));
            }
        }
    }
    newGraph.setMapping(true);
    newGraph.setIdToPos(idToPos);
    newGraph.setPosToId(posToId);
    cout << " \n\nExactAlg\n\n";
    //graph.printEdgeCounts();
    //newGraph.print(true); return;
    //newGraph.printWithGraphTraversal(true); return;

    /* Clique detection */
    vector<Graph::GraphTraversal> clique1;
    vector<Graph::GraphTraversal> clique2;
    Graph::GraphTraversal graphTraversal1(newGraph);
    clique1.reserve(cliqueSize);
    clique2.reserve(cliqueSize);
    clique1.push_back(graphTraversal1);

    while (findCliques(cliqueSize, newGraph, clique1, graphTraversal1)) {
        cout << "\nClique 1: " << endl;
        for (uint32_t i = 0 ; i < clique1.size() ; i++) {
            cout << clique1[i].curNode << endl;
        }

        clique2.clear();
        Graph::GraphTraversal graphTraversal2(newGraph);
        clique2.push_back(graphTraversal2);
        while (findCliques(cliqueSize, newGraph, clique2, graphTraversal2, &clique1)) {
            cout << "\nClique 2: " << endl;
            for (uint32_t i = 0 ; i < clique2.size() ; i++) {
                cout << clique2[i].curNode << endl;
            }
            break;
        }

        graphTraversal1 = clique1.back();
        clique1.pop_back();
        if (clique1.empty()) {
            newGraph.getNextNode(graphTraversal1);
            if (graphTraversal1.curNode == NONE) {
                cout << "No second clique" << endl;
                return;
            }
            clique1.push_back(graphTraversal1);
        }
        uint32_t dummy = NONE;
        advance(clique1, graphTraversal1, newGraph);
        //cout << "it's " << graphTraversal1.curNode << " and " << graphTraversal1.curEdgeOffset << endl;
    }

    /* Clique detection *
    unordered_set<uint32_t> clique1;
    for (uint32_t pos1 = 0 ; pos1 < newNodeIndex.size() ; pos1++) {
        uint32_t node = (*posToId)[pos1];
        clique1.insert(node);
        cout << "insert " << node << endl;
        for (uint32_t offset1 = newNodeIndex[pos1].getOffset() ; offset1 < newNodeIndex[pos1].getOffset() + newNodeIndex[pos1].getEdges() ; offset1++) {
            uint32_t neighbor1 = newEdgeBuffer[offset1];
            if (clique1.find(neighbor1) != clique1.end()) {
                continue;
            }
            if (isSubsetOfNeighbors(clique1, idToPos[neighbor1], graph)) {
                clique1.insert(neighbor1);
                cout << "insert " << neighbor1 << endl;
                uint32_t pos2 = idToPos[neighbor1];
                for (uint32_t offset2 = newNodeIndex[pos2].getOffset() ; offset2 < newNodeIndex[pos2].getOffset() + newNodeIndex[pos2].getEdges() ; offset2++) {
                    uint32_t neighbor2 = newEdgeBuffer[offset2];
                    if (clique1.find(neighbor2) != clique1.end()) {
                        continue;
                    }
                    if (isSubsetOfNeighbors(clique1, idToPos[neighbor2], graph)) {
                        clique1.insert(neighbor2);
                        cout << "insert " << neighbor2 << endl;
                        uint32_t pos3 = idToPos[neighbor2];
                        for (uint32_t offset3 = newNodeIndex[pos3].getOffset() ; offset3 < newNodeIndex[pos3].getOffset() + newNodeIndex[pos3].getEdges() ; offset3++) {
                            uint32_t neighbor3 = newEdgeBuffer[offset3];
                            if (clique1.find(neighbor3) != clique1.end()) {
                                continue;
                            }
                            if (isSubsetOfNeighbors(clique1, idToPos[neighbor3], graph)) {
                                clique1.insert(neighbor3);
                                cout << "insert " << neighbor3 << endl;
                                cout << "Clique: " << endl;
                                for (auto it = clique1.begin() ; it != clique1.end() ; it++) {
                                    cout << *it << endl;
                                }
                                return;
                            }
                        }
                        clique1.erase(neighbor2);
                        cout << "remove " << neighbor2 << endl;
                    }
                }
                clique1.erase(neighbor1);
                cout << "remove " << neighbor1 << endl;
            }
        }
        clique1.erase(node);
        cout << "remove " << node << endl;
    }*/
}

bool ExactAlg::findCliques(const uint32_t &cliqueSize, const Graph &graph, vector<Graph::GraphTraversal> &clique, Graph::GraphTraversal &graphTraversal, vector<Graph::GraphTraversal> *previousClique) {
    uint32_t commonNode = NONE; // Used for excluding cliques with a common edge
    if (previousClique != NULL && find(*previousClique, graphTraversal.curNode)) {
        commonNode = graphTraversal.curNode;
    }
    while (clique.size() < cliqueSize) {
        /*cout << "Cur clique: \n";
        for (uint32_t i = 0 ; i < clique.size() ; i++) {
            cout << clique[i].curNode << endl;
        }*/
        //cout << "ni " << graphTraversal.curEdgeOffset << endl;
        uint32_t neighbor = graph.edgeBuffer[graphTraversal.curEdgeOffset];
        bool existsInPreviousClique = (previousClique == NULL ? false : find(*previousClique, neighbor));
        if ((previousClique == NULL || previousClique != NULL && (commonNode == NONE || commonNode != NONE && !existsInPreviousClique)) &&
           !find(clique, neighbor) && isSubsetOfNeighbors(clique, neighbor, graph)) {
            //cout << "going to " << neighbor << endl;
            //cout << "commonNode is " << commonNode << endl;
            if (existsInPreviousClique) {
                commonNode = neighbor;
            }
            graph.goToNode(neighbor, graphTraversal);
            clique.push_back(graphTraversal);
        } else {
            if (!advance(clique, graphTraversal, graph, &commonNode, previousClique)) {
                return false;
            }
        }
    }
    return true;
}

/*
clique2 = clique1;
graphTraversal = clique2.back();
clique2.pop_back();
if (clique2.empty()) {
    graph.getNextNode(graphTraversal);
    if (graphTraversal.curNode == NONE) {
        cout << "No second clique" << endl;
        return;
    }
    clique2.push_back(graphTraversal);
}
advance(clique2, graphTraversal, graph);
filledClique = &clique1;
fillingClique = &clique2;*/
