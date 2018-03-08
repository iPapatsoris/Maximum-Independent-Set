#ifndef EXACTALG_H
#define EXACTALG_H

#include "../Graph.hpp"

class ExactAlg {
public:
    ExactAlg(Graph &graph) : graph(graph) {}
    void run();

private:
    static bool isSubsetOfNeighbors(const std::vector<Graph::GraphTraversal> &subset, const uint32_t &node, const Graph &graph) {
        uint32_t pos = (!graph.mapping ? node : (*graph.idToPos)[node]);
        uint32_t count = subset.size();
        for (uint32_t offset = graph.nodeIndex[pos].getOffset() ; count && offset < graph.nodeIndex[pos].getOffset() + graph.nodeIndex[pos].getEdges() ; offset++) {
            if (find(subset, graph.edgeBuffer[offset])) {
                count--;
            }
        }
        return !count;
    }
    
    static bool find(const std::vector<Graph::GraphTraversal> &subset, const uint32_t &node) {
        for (uint32_t i = 0 ; i < subset.size() ; i++) {
            if (subset[i].curNode == node) {
                return true;
            }
        }
        return false;
    }
    void reduce(const int &degree, const int &cliqueSize);
    Graph &graph;
};

#endif
