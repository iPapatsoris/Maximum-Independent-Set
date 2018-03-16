#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "ExactAlg.hpp"

using namespace std;

void ExactAlg::run() {
    cout << " \n\nExactAlg\n\n";
    reduce();
    graph.print(false);
    //graph.printEdgeCounts();
    //graph.printWithGraphTraversal(true);
    cout << "Mis: " << endl;
    for (uint32_t i = 0 ; i < mis.size() ; i++) {
        cout << mis[i] << endl;
    }
}

void ExactAlg::reduce() {
    Graph::ReduceInfo reduceInfo;
    removeLineGraphs(6, reduceInfo);
    removeLineGraphs(7, reduceInfo);
    removeLineGraphs(8, reduceInfo);
    graph.rebuild(reduceInfo);
}

void ExactAlg::removeLineGraphs(const uint32_t &degree, Graph::ReduceInfo &reduceInfo) {
    vector<Graph::GraphTraversal> clique;
    clique.reserve(degree+1);
    while (true) {
        Graph subgraph;
        graph.buildNDegreeSubgraph(degree, subgraph);
        cout << "Subgraph size " << subgraph.getNodeCount() << endl;
        if (!subgraph.getNodeCount()) {
            break;
        }
        //cout << "Subgraph is" << endl;
        //subgraph.print(true);
        if (!findClique(clique, degree+1, subgraph)) {
            break;
        }
        graph.remove(clique, reduceInfo);
        mis.push_back(clique[0].curNode);
    }
}

bool ExactAlg::findClique(vector<Graph::GraphTraversal> &clique, const uint32_t &cliqueSize, const Graph &graph) {
    clique.clear();
    Graph::GraphTraversal graphTraversal(graph);
    clique.push_back(graphTraversal);
    while (clique.size() < cliqueSize) {
        //if (clique[0].curNode == 11) {
        //cout << "Cur clique: \n";
        //for (uint32_t i = 0 ; i < clique.size() ; i++) {
        //cout << clique[i].curNode << endl;
        //}
        //}
        uint32_t neighbor = graph.edgeBuffer[graphTraversal.curEdgeOffset];
        //cout << "node " << graphTraversal.curNode << " neighbor " << neighbor << endl;
        if (graph.idToPos->find(neighbor) != graph.idToPos->end() && !find(neighbor, clique) && isSubsetOfNeighbors(clique, neighbor, graph)) {
            //cout << "going to " << neighbor << endl;
            //cout << "commonNode is " << commonNode << endl;
            graph.goToNode(neighbor, graphTraversal);
            clique.push_back(graphTraversal);
        } else {
            if (!advance(clique, graphTraversal, graph)) {
                return false;
            }
        }
    }
    cout << "\nClique: " << endl;
    for (uint32_t i = 0 ; i < clique.size() ; i++) {
        cout << clique[i].curNode << endl;
    }
    return true;
}
