#ifndef CONTROLUNIT_H
#define CONTROLUNIT_H

#include <string>
#include "Graph.hpp"
#include "Reductions.hpp"
#include "Alg.hpp"
#include "Mis.hpp"

class ControlUnit {
public:
    ControlUnit(const std::string &inputFile) : graph(inputFile), reductions(graph, mis), alg(graph, mis) {}
    void run();

private:
    Graph graph;
    Mis mis;
    Reductions reductions;
    Alg alg;
};

#endif
