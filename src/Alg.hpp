#ifndef ALG_H
#define ALG_H

#include "Graph.hpp"
#include "Mis.hpp"


class Alg {
public:
    Alg(Graph &graph, Mis &mis) : graph(graph), mis(mis) {}


private:
    Graph &graph;
    Mis &mis;
};

#endif
