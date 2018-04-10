#ifndef CONTROLUNIT_H
#define CONTROLUNIT_H

#include <string>
#include "Graph.hpp"
#include "Reductions.hpp"
#include "Alg.hpp"
#include "Mis.hpp"

class ControlUnit {
public:
    ControlUnit(const std::string &inputFile, const bool &checkIndependentSet) : graph(inputFile, checkIndependentSet), mis(inputFile + ".mis"), reductions(graph, mis), alg(graph, mis) {}
    void run();
    void checkIndependentSet(const std::string &misInputFile) const;

private:
    Graph graph;
    Mis mis;
    Reductions reductions;
    Alg alg;
};

#endif
