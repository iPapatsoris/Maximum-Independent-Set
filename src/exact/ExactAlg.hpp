#ifndef EXACTALG_H
#define EXACTALG_H

#include "../Graph.hpp"

class ExactAlg {
public:
    ExactAlg(Graph &graph) : graph(graph) {}
    void run();

private:
    static bool isSubsetOfNeighbors(const std::vector<Graph::GraphTraversal> &subset, const uint32_t &node, const Graph &graph) {
        uint32_t pos = (!graph.mapping ? node : graph.idToPos->at(node));
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
    void removeLineGraphs(const uint32_t &degree, Graph::ReduceInfo &reduceInfo);
    bool findClique(std::vector<Graph::GraphTraversal> &clique, const uint32_t &cliqueSize, const Graph &graph);
    Graph &graph;
    std::vector<uint32_t> mis;
};

#endif
