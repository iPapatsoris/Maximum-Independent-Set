#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "ExactAlg.hpp"

using namespace std;

void ExactAlg::run() {
    cout << " \nExactAlg\n";
    reduce();
    //graph.print(false);
    //graph.printEdgeCounts();
    //graph.printWithGraphTraversal(true);
    cout << "Mis size: " << mis.size() << "\n";
    cout << "Zero degree nodes: " << graph.zeroDegreeNodes.size() << "\n";
    //cout << "Mis: " << endl;
    //for (uint32_t i = 0 ; i < mis.size() ; i++) {
    //    cout << mis[i] << endl;
    //}
}

void ExactAlg::reduce() {
    Graph::ReduceInfo reduceInfo;
    removeLineGraphs(reduceInfo);
    removeUnconfinedNodes(reduceInfo);
    graph.rebuild(reduceInfo);
}

void ExactAlg::removeUnconfinedNodes(Graph::ReduceInfo &reduceInfo) {
    cout << "\n**Performing unconfined nodes reduction**" << endl;
    removeUnconfinedNodes2(reduceInfo);
    Graph::ReduceInfo old;
    do {
        cout << "Nodes removed " << reduceInfo.nodesRemoved << ", edges removed " << reduceInfo.edgesRemoved << endl;
        old = reduceInfo;
        removeUnconfinedNodes2(reduceInfo);
        //assert(old.nodesRemoved == reduceInfo.nodesRemoved);
    } while (old.nodesRemoved != reduceInfo.nodesRemoved);
}

void ExactAlg::removeUnconfinedNodes2(Graph::ReduceInfo &reduceInfo) {
    Graph::GraphTraversal graphTraversal(graph);
    while (graphTraversal.curNode != NONE) {
        //cout << "node " << graphTraversal.curNode << "\n";
        bool isUnconfined = false;
        vector<uint32_t> extendedGrandchildren;
        while (graphTraversal.curEdgeOffset != NONE) {
            uint32_t neighbor = graph.edgeBuffer[graphTraversal.curEdgeOffset];
            uint32_t outerNeighbor;
            bool exactlyOne;
            graph.getOuterNeighbor(graphTraversal.curNode, neighbor, outerNeighbor, exactlyOne);
            if (outerNeighbor == NONE) {
                //cout << "none\n";
                isUnconfined = true;
                break;
            } else if (exactlyOne) {
                //cout << "exactly one\n";
                extendedGrandchildren.push_back(outerNeighbor);
            }
            graph.getNextEdge(graphTraversal);
        }
        if (isUnconfined || !graph.isIndependentSet(extendedGrandchildren)) {
            //cout << "Unconfined node " << graphTraversal.curNode << "\n";
            graph.remove(graphTraversal.curNode, reduceInfo);
        }
        graph.getNextNode(graphTraversal);
    }
}

void ExactAlg::removeLineGraphs(Graph::ReduceInfo &reduceInfo) {
    cout << "\n**Perfoming line graph reduction**" << endl;
    removeLineGraphs(6, reduceInfo);
    removeLineGraphs(7, reduceInfo);
    removeLineGraphs(8, reduceInfo);
    cout << "Nodes removed " << reduceInfo.nodesRemoved << ", edges removed " << reduceInfo.edgesRemoved << "\n";
}

void ExactAlg::removeLineGraphs(const uint32_t &degree, Graph::ReduceInfo &reduceInfo) {
    vector<Graph::GraphTraversal> clique;
    clique.reserve(degree+1);
    while (true) {
        Graph subgraph;
        graph.buildNDegreeSubgraph(degree, subgraph);
        cout << degree << "-degree subgraph size: " << subgraph.getNodeCount() << endl;
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
    if (graphTraversal.curNode == NONE) {
        return false;
    }
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
    cout << "Clique: " << endl;
    for (uint32_t i = 0 ; i < clique.size() ; i++) {
        cout << clique[i].curNode << endl;
    }
    return true;
}
