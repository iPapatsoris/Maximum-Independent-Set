#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "Reductions.hpp"

using namespace std;

void Reductions::run() {
    cout << " \nReductions\n";
    reduce();
    //graph.printEdgeCounts();
    //graph.printWithGraphTraversal(true);
    //cout << "Mis: " << endl;
    //for (uint32_t i = 0 ; i < mis.size() ; i++) {
    //    cout << mis[i] << endl;
    //}
    //graph.print(true);
}

void Reductions::reduce() {
    removeLineGraphs();
    removeUnconfinedNodes();
    unordered_set<uint32_t> nodesWithoutSortedNeighbors;
    foldCompleteKIndependentSets(nodesWithoutSortedNeighbors);
    graph.rebuild(nodesWithoutSortedNeighbors, reduceInfo);
}

void Reductions::foldCompleteKIndependentSets(unordered_set<uint32_t> &nodesWithoutSortedNeighbors) {
    cout << "\n**Performing K-Independent set folding reduction**" << endl;
    ReduceInfo old = reduceInfo;
    foldCompleteKIndependentSets2(nodesWithoutSortedNeighbors);
    do {
        reduceInfo.print(&old);
        old = reduceInfo;
        foldCompleteKIndependentSets2(nodesWithoutSortedNeighbors);
    } while (old.nodesRemoved != reduceInfo.nodesRemoved);
}


void Reductions::removeUnconfinedNodes() {
    cout << "\n**Performing unconfined nodes reduction**" << endl;
    ReduceInfo old = reduceInfo;
    removeUnconfinedNodes2();
    do {
        reduceInfo.print(&old);
        old = reduceInfo;
        removeUnconfinedNodes2();
        //assert(old.nodesRemoved == reduceInfo.nodesRemoved);
    } while (old.nodesRemoved != reduceInfo.nodesRemoved);
}

/*void Reductions::foldCompleteKIndependentSets2(unordered_set<uint32_t> &nodesWithoutSortedNeighbors) {
    foldCompleteKIndependentSets(1, nodesWithoutSortedNeighbors);
    foldCompleteKIndependentSets(2, nodesWithoutSortedNeighbors);
}*/

void Reductions::foldCompleteKIndependentSets2(unordered_set<uint32_t> &nodesWithoutSortedNeighbors) {
    Graph::GraphTraversal graphTraversal(graph);
    uint32_t bound = 25000;
    while (graphTraversal.curNode != NONE) {
        for (uint32_t k = 1 ; k <= 2 ; k++) {
            if (graph.getNodeDegree(graphTraversal.curNode) == k+1) {
                if (graphTraversal.curNode >= bound) {
                    cout << "Node " << graphTraversal.curNode << "\n";
                    bound += 25000;
                }
                vector<uint32_t> nodes;
                vector<uint32_t> neighbors;
                nodes.push_back(graphTraversal.curNode);
                graph.gatherNeighbors(graphTraversal.curNode, neighbors);
                if (k == 2) {
                    uint32_t secondNode = graph.getNextNodeWithIdenticalNeighbors(graphTraversal.curNode, neighbors);
                    if (secondNode != NONE) {
                        nodes.push_back(secondNode);
                    }
                }
                if (k == 1 || k == 2 && nodes.size() == 2) {
                    vector<uint32_t> &mis = this->mis.getMis();
                    if (graph.isIndependentSet(neighbors)) {
                        uint32_t newNode = graph.contractToSingleNode(nodes, neighbors, nodesWithoutSortedNeighbors, reduceInfo);
                        this->mis.markHypernode(newNode, nodes, neighbors);
                    } else {
                        mis.insert(mis.end(), nodes.begin(), nodes.end());
                    }
                    neighbors.insert(neighbors.end(), nodes.begin(), nodes.end());
                    graph.remove(neighbors, reduceInfo);
                    //graph.print(true);
                }
                break;
            }
        }
        graph.getNextNode(graphTraversal);
    }
}

void Reductions::removeUnconfinedNodes2() {
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

void Reductions::removeLineGraphs() {
    cout << "\n**Perfoming line graph reduction**" << endl;
    removeLineGraphs(6);
    removeLineGraphs(7);
    removeLineGraphs(8);
    reduceInfo.print();
}

void Reductions::removeLineGraphs(const uint32_t &degree) {
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
        mis.getMis().push_back(clique[0].curNode);
    }
}

bool Reductions::findClique(vector<Graph::GraphTraversal> &clique, const uint32_t &cliqueSize, const Graph &graph) {
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
