#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <assert.h>
#include "Util.hpp"

#define MAXLINE 100

class Graph {

friend class ControlUnit;
friend class Reductions;
friend class Alg;
class NodeInfo;

public:
    struct GraphTraversal;
    Graph(const std::string &inputFile, const bool &checkIndependentSet);
    Graph() : mapping(false), idToPos(NULL), posToId(NULL) {}
    Graph(const Graph &graph);
    Graph& operator=(const Graph &graph);

    uint32_t getNodeCount() const;

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

    uint32_t getNodeDegree(const uint32_t &node) const {
        uint32_t pos = (!mapping ? node : idToPos->at(node));
        assert(!nodeIndex[pos].removed);
        return nodeIndex[pos].edges;
    }

    void collectZeroDegreeNodes();
    void addEdges(const uint32_t node, const std::vector<uint32_t> &nodes);
    void getCommonNeighbors(const uint32_t &node1, const uint32_t &node2, std::vector<uint32_t> &container) const;
    void getOptimalShortEdge(const uint32_t &degree, uint32_t &finalNode1, uint32_t &finalNode2, std::vector<uint32_t> &finalSet) const;
    void getExtendedGrandchildren(Graph::GraphTraversal &graphTraversal, std::unordered_set<uint32_t> &extendedGrandchildren, bool *isUnconfined = NULL) const;
    void getMaxNodeDegree(uint32_t &node, uint32_t &maxDegree) const;
    void remove(const uint32_t &node, ReduceInfo &reduceInfo);
    void rebuild(const ReduceInfo &reduceInfo);
    void buildNDegreeSubgraph(const uint32_t &degree, Graph &subgraph);
    uint32_t contractToSingleNode(const std::vector<uint32_t> &nodes, const std::vector<uint32_t> &neighbors, ReduceInfo &reduceInfo);
    uint32_t getNextNodeWithIdenticalNeighbors(const uint32_t &previousNode, const std::vector<uint32_t> &neighbors) const;
    void replaceNeighbor(const uint32_t &node, const uint32_t &oldNeighbor, const uint32_t &newNeighbor);
    void print(bool direction) const;
    void printWithGraphTraversal(bool direction) const;
    void printEdgeCounts() const;

    /* Mark selected nodes as removed and reduce their neighbors' neighbor count.
     * fullComponent should be set to true when the nodes to be removed belong in the same component, and
     * that component has no other nodes (e.g. when removing line graphs). */
    template <typename Container>
    void remove(const Container &nodes, ReduceInfo &reduceInfo, const bool &fullComponent = false, std::unordered_set<uint32_t> *candidateNodes = NULL) {
        for (auto it = nodes.begin() ; it != nodes.end() ; it++) {
            //std::cout << "removing " << *it << std::endl;
            uint32_t pos = (!mapping ? *it : idToPos->at(*it));
            if (!nodeIndex[pos].removed) {
                reduceInfo.nodesRemoved++;
                if (!fullComponent) {
                    uint32_t nextNodeOffset = (pos == nodeIndex.size()-1 ? edgeBuffer->size() : nodeIndex[pos+1].offset);
                    for (uint32_t offset = nodeIndex[pos].offset ; offset < nextNodeOffset ; offset++) {
                        uint32_t neighbor = (*edgeBuffer)[offset];
                        uint32_t nPos = (!mapping ? neighbor : idToPos->at(neighbor));
                        if (!nodeIndex[nPos].removed) {
                            nodeIndex[nPos].edges--;
                            reduceInfo.edgesRemoved++;
                            if (find(std::next(it, 1), nodes.end(), neighbor) == nodes.end() &&
                            candidateNodes != NULL && (nodeIndex[nPos].edges == 2 || nodeIndex[nPos].edges == 3) && nPos < pos) {
                                candidateNodes->insert(neighbor);
                            }
                        }
                    }
                }
                //nodeIndex[pos].edges = 0;
                nodeIndex[pos].removed = true;
            }
        }
    }

    template <typename Container, typename ContainerRemoved = std::vector<uint32_t> >
    void gatherNeighbors(const uint32_t &node, Container &neighbors, ContainerRemoved *removedNeighbors = NULL) const {
        uint32_t pos = (!mapping ? node : idToPos->at(node));
        uint32_t neighborCount = nodeIndex[pos].edges;
        uint32_t nextNodeOffset = (pos == nodeIndex.size()-1 ? edgeBuffer->size() : nodeIndex[pos+1].offset);
        for (uint32_t offset = nodeIndex[pos].offset ; offset  < nextNodeOffset && neighborCount; offset++) {
            uint32_t nPos = (!mapping ? (*edgeBuffer)[offset] : idToPos->at((*edgeBuffer)[offset]));
            if (!nodeIndex[nPos].removed) {
                neighbors.insert(neighbors.end(), (*edgeBuffer)[offset]);
                neighborCount--;
            } else if (removedNeighbors != NULL) {
                removedNeighbors->insert(removedNeighbors->end(), (*edgeBuffer)[offset]);
            }
        }
    }

    template <typename Container>
    void gatherNeighbors(Container &nodes, Container &neighbors) const {
        for (auto &node: nodes) {
            gatherNeighbors(node, neighbors);
        }
    }

    bool edgeExists(const uint32_t &node, const uint32_t &neighbor) const {
        return (findEdgeOffset(node, neighbor) != NONE);
    }

    /* Check whether a particular edge exists with binary search,
     * return neighbor's offset in edge buffer */
    uint32_t findEdgeOffset(const uint32_t &node, const uint32_t &neighbor) const {
        uint32_t pos = (!mapping ? node : idToPos->at(node));
        uint32_t nPos = (!mapping ? neighbor : idToPos->at(neighbor));
        /*if (nodeIndex[pos].removed || nodeIndex[nPos].removed) {
            std::cout << "testing edge " << node << " " << neighbor << std::endl;
        }*/
        assert(!nodeIndex[pos].removed && !nodeIndex[nPos].removed);
        uint32_t offset = nodeIndex[pos].offset;
        uint32_t endOffset = (pos == nodeIndex.size()-1 ? edgeBuffer->size() - 1 : nodeIndex[pos+1].offset - 1);
        if (offset == endOffset+1) {
            return NONE;
        }
        //std::cout <<"hey" << std::endl;
        uint32_t startIndex = 0;
        uint32_t endIndex = endOffset - offset;
        uint32_t index = (endIndex - startIndex) / 2;
        while (startIndex != endIndex) {
            //std::cout << "endIndex " << endIndex << ", startIndex " << startIndex << ", index " << index << std::endl;
            //std::cout << "size is " << edgeBuffer->size() << " vs " << offset+startIndex+index << std::endl;
            if ((*edgeBuffer)[offset + startIndex + index] == neighbor) {
                return offset + startIndex + index;
            } else if ((*edgeBuffer)[offset + startIndex + index] < neighbor) {
                startIndex += index + 1;
            } else {
                if (!index) {
                    return NONE;
                }
                endIndex = startIndex + index - 1;
            }
            index = (endIndex - startIndex) / 2;
        }
        return ((*edgeBuffer)[offset + startIndex + index] == neighbor ? offset + startIndex + index : NONE);
    }

    /* Return the first "outer neighbor of 'neighbor' at 'node'", and a flag of whether its the only one */
    void getOuterNeighbor(const uint32_t &node, const uint32_t &neighbor, uint32_t &outerNeighbor, bool &exactlyOne) const {
        outerNeighbor = NONE;
        exactlyOne = false;
        bool found = false;
        GraphTraversal graphTraversal(*this, neighbor);
        while (graphTraversal.curEdgeOffset != NONE) {
            uint32_t extendedGrandchild = (*edgeBuffer)[graphTraversal.curEdgeOffset];
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

    template <typename Container>
    bool isIndependentSet(const Container &set, uint32_t *node1 = NULL, uint32_t *node2 = NULL) const {
        for (auto i = set.begin() ; i != set.end() ; i++) {
            for (auto j = std::next(i, 1) ; j != set.end() ; j++) {
                if (edgeExists(*i, *j)) {
                    if (node1 != NULL && node2 != NULL) {
                        *node1 = *i;
                        *node2 = *j;
                    }
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
        assert(graphTraversal.curNode != NONE && graphTraversal.curEdgeOffset != NONE);
        uint32_t pos = (!mapping ? graphTraversal.curNode : idToPos->at(graphTraversal.curNode));
        uint32_t nextNodeOffset = (pos == nodeIndex.size()-1 ? edgeBuffer->size() : nodeIndex[pos+1].offset);
        graphTraversal.curEdgeOffset++;
        bool validNeighbor = false;
        for ( ; graphTraversal.curEdgeOffset  < nextNodeOffset ; graphTraversal.curEdgeOffset++) {
            uint32_t nPos = (!mapping ? (*edgeBuffer)[graphTraversal.curEdgeOffset] : idToPos->at((*edgeBuffer)[graphTraversal.curEdgeOffset]));
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
    void fill(const uint32_t &size, const bool &checkIndependentSet);

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
        uint32_t nextNodeOffset = (pos == nodeIndex.size()-1 ? edgeBuffer->size() : nodeIndex[pos+1].offset);
        for ( ; offset < nextNodeOffset ; offset++) {
            uint32_t nPos = (!mapping ? (*edgeBuffer)[offset] : idToPos->at((*edgeBuffer)[offset]));
            if (!nodeIndex[nPos].removed) {
                validNeighbor = true;
                break;
            }
        }
        return (validNeighbor ? offset : NONE);
    }

    std::vector<NodeInfo> nodeIndex;
    std::vector<uint32_t> *edgeBuffer; // Pointer to avoid copying at Graph::rebuild
    std::vector<uint32_t> zeroDegreeNodes;
    uint32_t nextUnusedId;

    /* Optional mappers, when ids are not equal to the equivalent vector index position */
    bool mapping;
    std::unordered_map<uint32_t, uint32_t> *idToPos;
    std::vector<uint32_t> *posToId;

    //std::unordered_map<uint32_t, std::set<uint32_t> > extraEdges;
};

#endif
