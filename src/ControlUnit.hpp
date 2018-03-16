#ifndef CONTROLUNIT_H
#define CONTROLUNIT_H

#include <string>
#include "Graph.hpp"

class ControlUnit {
public:
    ControlUnit(const std::string &inputFile) : graph(inputFile) {}
    void run();

private:
    Graph graph;
};

#endif
