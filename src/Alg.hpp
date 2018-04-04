#ifndef ALG_H
#define ALG_H

#include "Graph.hpp"


class Alg {
public:
    Alg(Graph &graph, std::vector<uint32_t> &mis) : graph(graph), mis(mis) {}


private:
    Graph &graph;
    std::vector<uint32_t> &mis;
};

#endif
