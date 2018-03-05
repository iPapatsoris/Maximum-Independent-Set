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

    void parseNodeIDs(char *buf, uint32_t *sourceNode, uint32_t *targetNode);
};

#endif
