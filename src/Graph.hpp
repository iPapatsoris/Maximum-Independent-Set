#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include "Util.hpp"

class Graph {

friend class ControlUnit;
friend class ExactAlg;
class NodeInfo;

public:
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
    void print(bool direction) const;
    void printEdgeCounts() const;

    struct GraphTraversal {
    public:
        GraphTraversal(const Graph &graph);

        uint32_t curNode;
        uint32_t curEdgeOffset;
    };

    void getNextNode(GraphTraversal &graphTraversal) const {
        bool checkEdgeNum = false;
        if (graphTraversal.curNode == NONE) {
            graphTraversal.curNode = (!mapping ? 0 : (*posToId)[0]);
            checkEdgeNum = true;
        }
        uint32_t pos = (!mapping ? graphTraversal.curNode : (*idToPos)[graphTraversal.curNode]);
        while (checkEdgeNum && !nodeIndex[pos].edges || !checkEdgeNum) {
            if (pos < nodeIndex.size() - 1) {
                graphTraversal.curNode = (!mapping ? 1 + pos : (*posToId)[1 + pos]);
                graphTraversal.curEdgeOffset = (nodeIndex[pos].edges ? nodeIndex[pos].offset : NONE);
                checkEdgeNum = true;
                pos = graphTraversal.curNode;
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
        if (graphTraversal.curEdgeOffset == NONE) {
            graphTraversal.curEdgeOffset = (nodeIndex[pos].edges ? nodeIndex[pos].offset : NONE);
        } else {
            graphTraversal.curEdgeOffset = (graphTraversal.curEdgeOffset + 1 < nodeIndex[pos].offset + nodeIndex[pos].edges ? graphTraversal.curEdgeOffset + 1 : NONE);
        }
    }

private:
    void static parseNodeIDs(char *buf, uint32_t *sourceNode, uint32_t *targetNode);
    void fill(const uint32_t &size);

    struct NodeInfo {
    public:
        NodeInfo(const uint32_t &offset, const uint32_t &edges) : offset(offset), edges(edges) {}
        uint32_t getOffset() const {
            return offset;
        }
        uint32_t getEdges() const {
            return edges;
        }

        uint32_t offset; // Offset of neighbors in edgeBuffer
        uint32_t edges;
    };

    std::vector<NodeInfo> nodeIndex;
    std::vector<uint32_t> edgeBuffer;

    /* Optional mappers, when ids are not equal to the equivalent vector index position */
    bool mapping;
    std::unordered_map<uint32_t, uint32_t> *idToPos;
    std::vector<uint32_t> *posToId;
};

#endif
