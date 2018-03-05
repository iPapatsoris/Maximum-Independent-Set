#ifndef EXACTALG_H
#define EXACTALG_H

#include "../Graph.hpp"

class ExactAlg {
public:
    ExactAlg(Graph &graph) : graph(graph) {}
    void run();

private:
    void reduce(const int &degree, const int &cliqueSize);
    Graph &graph;
};

#endif
