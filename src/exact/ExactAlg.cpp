#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "ExactAlg.hpp"

using namespace std;

void ExactAlg::run() {
    cout << " \n\nExactAlg\n\n";
    reduce();
    graph.print(true);
    //graph.printEdgeCounts();
    //graph.printWithGraphTraversal(true);
}

void ExactAlg::reduce() {
    Graph::ReduceInfo reduceInfo;
    removeLineGraphs(6, 4, 4, reduceInfo);
    removeLineGraphs(7, 4, 5, reduceInfo);
    removeLineGraphs(8, 5, 5, reduceInfo);
    graph.rebuild(reduceInfo);
}

void ExactAlg::removeLineGraphs(const uint32_t &degree, const uint32_t &clique1Size, const uint32_t &clique2Size, Graph::ReduceInfo &reduceInfo) {
    vector<Graph::GraphTraversal> clique1;
    vector<Graph::GraphTraversal> clique2;
    clique1.reserve(clique1Size);
    clique2.reserve(clique2Size);
    while (true) {
        Graph subgraph;
        buildSubgraph(degree, subgraph);
        //cout << "Subgraph is" << endl;
        //subgraph.print(true);
        if (!findCliques(clique1, clique2, clique1Size, clique2Size, subgraph)) {
            break;
        }
        graph.remove(clique1, reduceInfo);
        graph.remove(clique2, reduceInfo);
    }
}

void ExactAlg::buildSubgraph(const uint32_t &degree, Graph &subgraph) {
    unordered_map<uint32_t, uint32_t> *idToPos = new unordered_map<uint32_t, uint32_t>();
    vector<uint32_t> *posToId = new vector<uint32_t>();
    vector<Graph::NodeInfo> &newNodeIndex = subgraph.getNodeIndex();
    vector<uint32_t> &newEdgeBuffer = subgraph.getEdgeBuffer();
    vector<Graph::NodeInfo> &nodeIndex = graph.getNodeIndex();
    vector<uint32_t> &edgeBuffer = graph.getEdgeBuffer();
    for (uint32_t node = 0 ; node < nodeIndex.size() ; node++) {
        if (1 || degree == NONE || nodeIndex[node].getEdges() == degree) {
            uint32_t newOffset = newEdgeBuffer.size();
            uint32_t newEdges = 0;
            uint32_t nextNodeOffset = (node == nodeIndex.size()-1 ? edgeBuffer.size() : nodeIndex[node+1].offset);
            for (uint32_t offset = nodeIndex[node].offset ; offset < nextNodeOffset ; offset++) {
                if (!nodeIndex[edgeBuffer[offset]].removed && (1 || degree == NONE || nodeIndex[edgeBuffer[offset]].getEdges() == degree)) {
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
    subgraph.setMapping(true);
    subgraph.setIdToPos(idToPos);
    subgraph.setPosToId(posToId);
}

bool ExactAlg::findCliques(vector<Graph::GraphTraversal> &clique1, vector<Graph::GraphTraversal> &clique2, const uint32_t &clique1Size, const uint32_t &clique2Size, const Graph &graph) {
    clique1.clear();
    Graph::GraphTraversal graphTraversal1(graph);
    clique1.push_back(graphTraversal1);
    while (findClique(clique1Size, graph, clique1, graphTraversal1)) {
        cout << "\nClique 1: " << endl;
        for (uint32_t i = 0 ; i < clique1.size() ; i++) {
            cout << clique1[i].curNode << endl;
        }

        clique2.clear();
        Graph::GraphTraversal graphTraversal2(graph);
        clique2.push_back(graphTraversal2);
        while (findClique(clique2Size, graph, clique2, graphTraversal2, &clique1)) {
            cout << "\nClique 2: " << endl;
            for (uint32_t i = 0 ; i < clique2.size() ; i++) {
                cout << clique2[i].curNode << endl;
            }
            return true;
        }

        graphTraversal1 = clique1.back();
        clique1.pop_back();
        if (clique1.empty()) {
            graph.getNextNode(graphTraversal1);
            if (graphTraversal1.curNode == NONE) {
                cout << "No other clique1" << endl;
                return false;
            }
            clique1.push_back(graphTraversal1);
        }
        advance(clique1, graphTraversal1, graph);
        //cout << "it's " << graphTraversal1.curNode << " and " << graphTraversal1.curEdgeOffset << endl;
    }
    return false;
}

bool ExactAlg::findClique(const uint32_t &cliqueSize, const Graph &graph, vector<Graph::GraphTraversal> &clique, Graph::GraphTraversal &graphTraversal, vector<Graph::GraphTraversal> *previousClique) {
    uint32_t commonNode = NONE; // Used for excluding cliques with a common edge
    if (previousClique != NULL && find(*previousClique, graphTraversal.curNode)) {
        commonNode = graphTraversal.curNode;
    }
    while (clique.size() < cliqueSize) {
        //if (clique[0].curNode == 11) {
        //cout << "Cur clique: \n";
        //for (uint32_t i = 0 ; i < clique.size() ; i++) {
        //    cout << clique[i].curNode << endl;
        //}
        //}
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
