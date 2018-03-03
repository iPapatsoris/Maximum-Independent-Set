#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <vector>
#include "Util.hpp"

class Graph {

friend class ControlUnit;
class NodeInfo;

public:
    std::vector<NodeInfo> &getNodeIndex() {
        return nodeIndex;
    }
    std::vector<uint32_t> &getEdgeBuffer() {
        return edgeBuffer;
    }
    void print(bool direction) const;


private:

    class NodeInfo {
    public:
        NodeInfo(const uint32_t &offset, const uint32_t &edges) : offset(offset), edges(edges) {}
        uint32_t getOffset() const {return offset;}
        uint32_t getEdges() const {return edges;}

    private:
        uint32_t offset; // offset of neighbors in edgeBuffer
        uint32_t edges;
    };

    std::vector<NodeInfo> nodeIndex;
    std::vector<uint32_t> edgeBuffer;
};

#endif
