#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <assert.h>
#include "Util.hpp"

class Graph {

friend class ControlUnit;
friend class Reductions;
friend class Alg;
class NodeInfo;

public:
    struct GraphTraversal;
    Graph(const std::string &inputFile);
    Graph() : mapping(false), idToPos(NULL), posToId(NULL) {}

    uint32_t getNodeCount() const {
        return nodeIndex.size();
    }

    uint32_t getPos(const uint32_t &node) const {
        return (!mapping ? node : idToPos->at(node));
    }

    ~Graph();

    void setIdToPos(std::unordered_map<uint32_t, uint32_t> *idToPos) {
        if (this->idToPos != NULL) {
            delete this->idToPos;
        }
        this->idToPos = idToPos;
    }

    void setPosToId(std::vector<uint32_t> *posToId) {
        if (this->posToId != NULL) {
            delete this->posToId;
        }
        this->posToId = posToId;
    }

    void setMapping(const bool &mapping) {
        this->mapping = mapping;
    }

    uint32_t getNodeDegree(const uint32_t &node) {
        uint32_t pos = (!mapping ? node : idToPos->at(node));
        assert(!nodeIndex[pos].removed);
        return nodeIndex[pos].edges;
    }

    void remove(const std::vector<Graph::GraphTraversal> &nodes, ReduceInfo &reduceInfo);
    void remove(const std::vector<uint32_t> &nodes, ReduceInfo &reduceInfo);
    void remove(const uint32_t &node, ReduceInfo &reduceInfo);
    void rebuild(const std::unordered_set<uint32_t> &nodesWithoutSortedNeighbors, const ReduceInfo &reduceInfo);
    void buildNDegreeSubgraph(const uint32_t &degree, Graph &subgraph);
    uint32_t contractToSingleNode(const std::vector<uint32_t> &nodes, const std::vector<uint32_t> &neighbors, std::unordered_set<uint32_t> &nodesWithoutSortedNeighbors);
    void gatherNeighbors(const uint32_t &node, std::vector<uint32_t> &neighbors) const;
    uint32_t getNextNodeWithIdenticalNeighbors(const uint32_t &previousNode, const std::vector<uint32_t> &neighbors) const;
    void replaceNeighbor(const uint32_t &node, const uint32_t &oldNeighbor, const uint32_t &newNeighbor, const std::unordered_set<uint32_t> &nodesWithoutSortedNeighbors);
    void print(bool direction) const;
    void printWithGraphTraversal(bool direction) const;
    void printEdgeCounts() const;

    bool edgeExists(const uint32_t &node, const uint32_t &neighbor, const bool &binarySearch = true) const {
        return (findEdgeOffset(node, neighbor, binarySearch) != NONE);
    }

    /* Check whether a particular edge exists with binary search,
     * return neighbor's offset in edge buffer */
    uint32_t findEdgeOffset(const uint32_t &node, const uint32_t &neighbor, const bool &binarySearch = true) const {
        //std::cout << "testing edge " << node << " " << neighbor << std::endl;
        uint32_t pos = (!mapping ? node : idToPos->at(node));
        uint32_t nPos = (!mapping ? neighbor : idToPos->at(neighbor));
        assert(!nodeIndex[pos].removed && !nodeIndex[nPos].removed);
        uint32_t offset = nodeIndex[pos].offset;
        uint32_t endOffset = offset + nodeIndex[pos].edges - 1;
        if (offset == endOffset) {
            return NONE;
        }
        uint32_t startIndex = 0;
        uint32_t endIndex = nodeIndex[pos].edges - 1;
        uint32_t index = (endIndex - startIndex) / 2;
        while (startIndex != endIndex) {
            if (edgeBuffer[offset + startIndex + index] == neighbor) {
                return offset + startIndex + index;
            } else if (edgeBuffer[offset + startIndex + index] < neighbor) {
                startIndex += index + 1;
            } else {
                if (!index) {
                    return NONE;
                }
                endIndex = startIndex + index - 1;
            }
            index = (endIndex - startIndex) / 2;
        }
        return (edgeBuffer[offset + startIndex + index] == neighbor ? offset + startIndex + index : NONE);
    }

    /* Return the first "outer neighbor of 'neighbor' at 'node'", and a flag of whether its the only one */
    void getOuterNeighbor(const uint32_t &node, const uint32_t &neighbor, uint32_t &outerNeighbor, bool &exactlyOne) const {
        outerNeighbor = NONE;
        exactlyOne = false;
        bool found = false;
        GraphTraversal graphTraversal(*this, neighbor);
        while (graphTraversal.curEdgeOffset != NONE) {
            uint32_t extendedGrandchild = edgeBuffer[graphTraversal.curEdgeOffset];
            if (extendedGrandchild != node && !edgeExists(extendedGrandchild, node)) {
                //std::cout << "edge " << node << " " << extendedGrandchild << " does not exist\n";
                if (!found) {
                    found = true;
                    outerNeighbor = extendedGrandchild;
                } else {
                    return;
                }
            }
            getNextEdge(graphTraversal);
        }
        if (found) {
            exactlyOne = true;
        }
    }

    bool isIndependentSet(const std::vector<uint32_t> &set) const {
        for (uint32_t i = 0 ; i < set.size() ; i++) {
            for (uint32_t j = i+1 ; j < set.size() ; j++) {
                if (edgeExists(set[i], set[j])) {
                    return false;
                }
            }
        }
        return true;
    }

    /* For graph traversal through functions */
    struct GraphTraversal {
    public:
        GraphTraversal(const Graph &graph);
        GraphTraversal(const Graph &graph, const uint32_t &node);
        GraphTraversal(const uint32_t &curNode, const uint32_t &curEdgeOffset) : curNode(curNode), curEdgeOffset(curEdgeOffset) {}

        uint32_t curNode;
        uint32_t curEdgeOffset;
    };

    /* Return the next node in a graph traversal, ignoring removed or zero-degree ones.
     * Initialise its first edge in the traversal as its first non-removed neighbor. */
    void getNextNode(GraphTraversal &graphTraversal) const {
        bool firstTime = true;
        uint32_t pos;
        if (graphTraversal.curNode == NONE) {
            pos = NONE;
        } else {
            pos = (!mapping ? graphTraversal.curNode : idToPos->at(graphTraversal.curNode));
        }
        while (firstTime || pos == NONE || !nodeIndex[pos].edges) {
            if (pos == NONE && nodeIndex.size() || pos != NONE && pos < nodeIndex.size() - 1) {
                pos = (pos == NONE ? 0 : pos+1);
                if (nodeIndex[pos].removed) {
                    continue;
                }
                graphTraversal.curNode = (!mapping ? pos : (*posToId)[pos]);
                uint32_t offset = getFirstValidNeighborOffset(pos);
                if ((graphTraversal.curEdgeOffset = offset) != NONE) {
                    firstTime = false;
                }
            }
            else {
                graphTraversal.curNode = NONE;
                graphTraversal.curEdgeOffset = NONE;
                break;
            }
        }
    }

    /* Return next edge in a graph traversal, ignoring removed nodes */
    void getNextEdge(GraphTraversal &graphTraversal) const {
        if (graphTraversal.curNode == NONE) {
            graphTraversal.curEdgeOffset = NONE;
            return;
        }
        uint32_t pos = (!mapping ? graphTraversal.curNode : idToPos->at(graphTraversal.curNode));
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
            if (mapping && idToPos->find(edgeBuffer[graphTraversal.curEdgeOffset]) == idToPos->end()) { // For subgraphs generated by buildNDegreeSubgraph
                continue;
            }
            uint32_t nPos = (!mapping ? edgeBuffer[graphTraversal.curEdgeOffset] : idToPos->at(edgeBuffer[graphTraversal.curEdgeOffset]));
            if (!nodeIndex[nPos].removed) {
                validNeighbor = true;
                break;
            }
        }
        if (!validNeighbor) {
            graphTraversal.curEdgeOffset = NONE;
        }
    }

    /* Jump to a node */
    void goToNode(const uint32_t &node, GraphTraversal &graphTraversal) const {
        graphTraversal.curNode = node;
        uint32_t pos = (!mapping ? node : idToPos->at(node));
        graphTraversal.curEdgeOffset = getFirstValidNeighborOffset(pos);
    }

private:
    void static parseNodeIDs(char *buf, uint32_t *sourceNode, uint32_t *targetNode);
    void fill(const uint32_t &size);

    struct NodeInfo {
    public:
        NodeInfo(const uint32_t &offset, const uint32_t &edges) : offset(offset), edges(edges), removed(false) {}

        uint32_t offset; // Offset of neighbors in edgeBuffer
        uint32_t edges;
        bool removed;
    };

    /* Rerturn edge buffer offset of first non-removed neighbor of node at pos */
    uint32_t getFirstValidNeighborOffset(const uint32_t &pos) const {
        bool validNeighbor = false;
        uint32_t offset = nodeIndex[pos].offset;
        uint32_t nextNodeOffset = (pos == nodeIndex.size()-1 ? edgeBuffer.size() : nodeIndex[pos+1].offset);
        for ( ; offset < nextNodeOffset ; offset++) {
            if (mapping && idToPos->find(edgeBuffer[offset]) == idToPos->end()) { // For subgraphs generated by buildNDegreeSubgraph
                continue;
            }
            uint32_t nPos = (!mapping ? edgeBuffer[offset] : idToPos->at(edgeBuffer[offset]));
            if (!nodeIndex[nPos].removed) {
                validNeighbor = true;
                break;
            }
        }
        return (validNeighbor ? offset : NONE);
    }

    std::vector<NodeInfo> nodeIndex;
    std::vector<uint32_t> edgeBuffer;
    std::vector<uint32_t> zeroDegreeNodes;

    /* Optional mappers, when ids are not equal to the equivalent vector index position */
    bool mapping;
    std::unordered_map<uint32_t, uint32_t> *idToPos;
    std::vector<uint32_t> *posToId;
};

#endif
