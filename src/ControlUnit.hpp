#ifndef CONTROLUNIT_H
#define CONTROLUNIT_H

#include <string>
#include "Graph.hpp"
#include "Reductions.hpp"
#include "Alg.hpp"

class ControlUnit {
public:
    ControlUnit(const std::string &inputFile) : graph(inputFile), reductions(graph, mis), alg(graph, mis) {}
    void run();

private:
    Graph graph;
    std::vector<uint32_t> mis;
    Reductions reductions;
    Alg alg;
};

#endif
