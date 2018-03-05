#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <string>
#include <vector>
#include "Util.hpp"

class Graph {

friend class ControlUnit;
friend class ExactAlg;
class NodeInfo;

public:
    Graph(const std::string &inputFile);
    Graph() : posToId(NULL) {}
    std::vector<NodeInfo> &getNodeIndex() {
        return nodeIndex;
    }

    std::vector<uint32_t> &getEdgeBuffer() {
        return edgeBuffer;
    }

    std::vector<uint32_t> *getPosToId() {
        return posToId;
    }
    ~Graph();

    void setPosToId(std::vector<uint32_t> *posToId) {this->posToId = posToId;}
    void print(bool direction) const;
    void printEdgeCounts() const;


private:
    void static parseNodeIDs(char *buf, uint32_t *sourceNode, uint32_t *targetNode);

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
    std::vector<uint32_t> *posToId; // mapper
};

#endif
