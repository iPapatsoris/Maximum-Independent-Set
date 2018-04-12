#ifndef REDUCTIONS_H
#define REDUCTIONS_H

#include "Graph.hpp"
#include "Mis.hpp"

class Reductions {
public:
    Reductions(Graph &graph, Mis &mis);
    ~Reductions();
    void run();

private:
    /* Check whether subset is a subset of node's neighbors */
    static bool isSubsetOfNeighbors(const std::vector<Graph::GraphTraversal> &subset, const uint32_t &node, const Graph &graph) {
        uint32_t pos = graph.getPos(node);
        uint32_t count = subset.size();
        uint32_t nextNodeOffset = (pos == graph.nodeIndex.size()-1 ? graph.edgeBuffer.size() : graph.nodeIndex[pos+1].offset);
        for (uint32_t offset = graph.nodeIndex[pos].offset ; count && offset < nextNodeOffset ; offset++) {
            if (find(graph.edgeBuffer[offset], subset)) {
                count--;
            }
        }
        return !count;
    }

    static bool find(const uint32_t &node, const std::vector<Graph::GraphTraversal> &set) {
        for (uint32_t i = 0 ; i < set.size() ; i++) {
            if (set[i].curNode == node) {
                return true;
            }
        }
        return false;
    }

    /* Get current node's next valid neighbor. If none, advance to next node */
    static bool advance(std::vector<Graph::GraphTraversal> &clique, Graph::GraphTraversal &graphTraversal, const Graph &graph) {
        bool validNeighbor = false;
        while(!validNeighbor) {
            graphTraversal = clique.back();
            graph.getNextEdge(graphTraversal);
            clique.back() = graphTraversal;
            if (graphTraversal.curEdgeOffset != NONE && graph.idToPos->find(graph.edgeBuffer[graphTraversal.curEdgeOffset]) != graph.idToPos->end()) {
                validNeighbor = true;
            } else {
                graphTraversal = clique.back();
                clique.pop_back();
                if (clique.empty()) {
                    graph.getNextNode(graphTraversal);
                    if (graphTraversal.curNode == NONE) {
                        return false;
                    }
                    clique.push_back(graphTraversal);
                }
            }
        }
        return true;
    }

    void reduce();
    void removeUnconfinedNodes();
    void removeUnconfinedNodes2();
    void removeLineGraphs();
    void foldCompleteKIndependentSets(std::unordered_set<uint32_t> &nodesWithoutSortedNeighbors);
    void foldCompleteKIndependentSets2(std::unordered_set<uint32_t> &nodesWithoutSortedNeighbors);
    void foldCompleteKIndependentSets(const uint32_t &k, std::unordered_set<uint32_t> &nodesWithoutSortedNeighbors);
    void removeLineGraphs(const uint32_t &degree);
    bool findClique(std::vector<Graph::GraphTraversal> &clique, const uint32_t &cliqueSize, const Graph &graph);
    void printCC() const;
    void printCCSizes() const;

    Graph &graph;
    Mis &mis;
    ReduceInfo reduceInfo;
    std::unordered_map<uint32_t, uint32_t> nodeToCC;
    std::vector<std::vector<uint32_t>* > ccToNodes;
};

#endif