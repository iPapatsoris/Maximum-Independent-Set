#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <assert.h>
#include "Util.hpp"

class Graph {

friend class ControlUnit;
friend class ExactAlg;
class NodeInfo;

public:
    struct GraphTraversal;
    struct ReduceInfo;
    Graph(const std::string &inputFile);
    Graph() : mapping(false), idToPos(NULL), posToId(NULL) {}
    std::vector<NodeInfo> &getNodeIndex() {
        return nodeIndex;
    }

    std::vector<uint32_t> &getEdgeBuffer() {
        return edgeBuffer;
    }

    std::unordered_map<uint32_t, uint32_t> *getIdToPos() {
        return idToPos;
    }

    std::vector<uint32_t> *getPosToId() {
        return posToId;
    }

    uint32_t getNodeCount() const {
        return nodeIndex.size();
    }

    bool hasMapping() const {
        return mapping;
    }
    ~Graph();

    void setIdToPos(std::unordered_map<uint32_t, uint32_t> *idToPos) {
        this->idToPos = idToPos;
    }
    void setPosToId(std::vector<uint32_t> *posToId) {
        this->posToId = posToId;
    }
    void setMapping(const bool &mapping) {
        this->mapping = mapping;
    }

    void remove(const std::vector<Graph::GraphTraversal> &nodes, ReduceInfo &reduceInfo);
    void rebuild(const ReduceInfo &reduceInfo);
    void print(bool direction) const;
    void printWithGraphTraversal(bool direction) const;
    void printEdgeCounts() const;

    /* For graph traversal through functions */
    struct GraphTraversal {
    public:
        GraphTraversal(const Graph &graph);

        uint32_t curNode;
        uint32_t curEdgeOffset;
    };

    void getNextNode(GraphTraversal &graphTraversal) const {
        bool firstTime = true;
        uint32_t pos;
        if (graphTraversal.curNode == NONE) {
            pos = NONE;
        } else {
            pos = (!mapping ? graphTraversal.curNode : (*idToPos)[graphTraversal.curNode]);
        }
        while (firstTime || pos == NONE || !nodeIndex[pos].edges) { // Ignore nodes with no edges
            if (pos == NONE && nodeIndex.size() || pos < nodeIndex.size() - 1) {
                pos = (pos == NONE ? 0 : pos+1);
                if (nodeIndex[pos].removed) {
                    continue;
                }
                graphTraversal.curNode = (!mapping ? pos : (*posToId)[pos]);
                bool validNeighbor = false;
                uint32_t offset = nodeIndex[pos].offset;
                uint32_t nextNodeOffset = (pos == nodeIndex.size()-1 ? edgeBuffer.size() : nodeIndex[pos+1].offset);
                for ( ; offset < nextNodeOffset ; offset++) {
                    uint32_t nPos = (!mapping ? edgeBuffer[offset] : (*idToPos)[edgeBuffer[offset]]);
                    if (!nodeIndex[nPos].removed) {
                        validNeighbor = true;
                        break;
                    }
                }
                if ((graphTraversal.curEdgeOffset = (validNeighbor ? offset : NONE)) != NONE) {
                    firstTime = false;
                }
            }
            else {
                graphTraversal.curNode = NONE;
                graphTraversal.curEdgeOffset = NONE;
                return;
            }
        }
    }

    void getNextEdge(GraphTraversal &graphTraversal) const {
        if (graphTraversal.curNode == NONE) {
            graphTraversal.curEdgeOffset = NONE;
            return;
        }
        uint32_t pos = (!mapping ? graphTraversal.curNode : (*idToPos)[graphTraversal.curNode]);
        uint32_t nextNodeOffset = (pos == nodeIndex.size()-1 ? edgeBuffer.size() : nodeIndex[pos+1].offset);
        if (graphTraversal.curEdgeOffset == NONE) {
            graphTraversal.curEdgeOffset = (nodeIndex[pos].edges ? nodeIndex[pos].offset : NONE);
        } else {
            graphTraversal.curEdgeOffset = (graphTraversal.curEdgeOffset + 1 < nextNodeOffset ? graphTraversal.curEdgeOffset + 1 : NONE);
        }
        if (graphTraversal.curEdgeOffset == NONE) {
            return;
        }
        bool validNeighbor = false;
        for ( ; graphTraversal.curEdgeOffset  < nextNodeOffset ; graphTraversal.curEdgeOffset++) {
            uint32_t nPos = (!mapping ? edgeBuffer[graphTraversal.curEdgeOffset] : (*idToPos)[edgeBuffer[graphTraversal.curEdgeOffset]]);
            if (!nodeIndex[nPos].removed) {
                validNeighbor = true;
                break;
            }
        }
        if (!validNeighbor) {
            graphTraversal.curEdgeOffset = NONE;
        }
    }

    void goToNode(const uint32_t &node, GraphTraversal &graphTraversal) const {
        graphTraversal.curNode = node;
        uint32_t pos = (!mapping ? node : (*idToPos)[node]);
        graphTraversal.curEdgeOffset = (nodeIndex[pos].edges ? nodeIndex[pos].offset : NONE);
        assert(graphTraversal.curEdgeOffset == NONE || !nodeIndex[(!mapping ? graphTraversal.curEdgeOffset : (*idToPos)[graphTraversal.curEdgeOffset])].removed);
    }

    struct ReduceInfo {
    public:
        ReduceInfo() : nodesRemoved(0), edgesRemoved(0) {}
        uint32_t nodesRemoved;
        uint32_t edgesRemoved;
    };

private:
    void static parseNodeIDs(char *buf, uint32_t *sourceNode, uint32_t *targetNode);
    void fill(const uint32_t &size);
    uint32_t getNextBiggerNeighborOffset(const uint32_t &node) const;
    void rebuild(uint32_t nodesRemoved);

    struct NodeInfo {
    public:
        NodeInfo(const uint32_t &offset, const uint32_t &edges) : offset(offset), edges(edges), removed(false) {}
        uint32_t getOffset() const {
            return offset;
        }
        uint32_t getEdges() const {
            return edges;
        }

        uint32_t offset; // Offset of neighbors in edgeBuffer
        uint32_t edges;
        bool removed;
    };

    std::vector<NodeInfo> nodeIndex;
    std::vector<uint32_t> edgeBuffer;

    /* Optional mappers, when ids are not equal to the equivalent vector index position */
    bool mapping;
    std::unordered_map<uint32_t, uint32_t> *idToPos;
    std::vector<uint32_t> *posToId;
};

#endif
