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

    static bool advance(std::vector<Graph::GraphTraversal> &clique, Graph::GraphTraversal &graphTraversal, const Graph &graph, uint32_t *commonNode = NULL, std::vector<Graph::GraphTraversal> *previousClique = NULL) {
        bool validNeighbor = false;
        while(!validNeighbor) {
            graphTraversal = clique.back();
            graph.getNextEdge(graphTraversal);
            clique.back() = graphTraversal;
            if (graphTraversal.curEdgeOffset != NONE) {
                validNeighbor = true;
            } else {
                graphTraversal = clique.back();
                if (commonNode != NULL && graphTraversal.curNode == *commonNode) {
                    *commonNode = NONE;
                }
                clique.pop_back();
                if (clique.empty()) {
                    graph.getNextNode(graphTraversal);
                    if (graphTraversal.curNode == NONE) {
                        std::cout << "No clique" << std::endl;
                        return false;
                    }
                    clique.push_back(graphTraversal);
                    if (previousClique != NULL && find(*previousClique, graphTraversal.curNode)) {
                        *commonNode = graphTraversal.curNode;
                    }
                }
            }
        }
        return true;
    }

    void reduce(const int &degree, const int &cliqueSize);
    bool findCliques(const uint32_t &cliqueSize, const Graph &graph, std::vector<Graph::GraphTraversal> &clique, Graph::GraphTraversal &graphTraversal, std::vector<Graph::GraphTraversal> *previousClique = NULL);
    Graph &graph;
};

#endif
