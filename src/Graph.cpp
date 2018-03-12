#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "Graph.hpp"

#define MAXLINE 100

using namespace std;

Graph::GraphTraversal::GraphTraversal(const Graph &graph) {
    curNode = NONE;
    curEdgeOffset = NONE;
    graph.getNextNode(*this);
}

Graph::~Graph() {
    if (mapping) {
        delete idToPos;
        delete posToId;
    }
}

void Graph::remove(const std::vector<Graph::GraphTraversal> &nodes) {
    for (auto it = nodes.begin() ; it != nodes.end() ; it++) {
        uint32_t pos = (!mapping ? it->curNode : (*idToPos)[it->curNode]);
        if (!nodeIndex[pos].removed) {
            uint32_t nextNodeOffset = (pos == nodeIndex.size()-1 ? edgeBuffer.size() : nodeIndex[pos+1].offset);
            for (uint32_t offset = nodeIndex[pos].offset ; offset < nextNodeOffset ; offset++) {
                uint32_t nPos = (!mapping ? edgeBuffer[offset] : (*idToPos)[edgeBuffer[offset]]);
                nodeIndex[nPos].edges--;
            }
        }
        nodeIndex[pos].edges = 0;
        nodeIndex[pos].removed = true;
    }
}


uint32_t Graph::getNextBiggerNeighborOffset(const uint32_t &node) const {
    uint32_t pos = (!mapping ? node : (*idToPos)[node]);
    uint32_t offset = nodeIndex[pos].offset;
    uint32_t endOffset = offset + nodeIndex[pos].edges - 1;
    if (offset == endOffset) {
        return NONE;
    }
    uint32_t startIndex = 0;
    uint32_t endIndex = nodeIndex[pos].edges - 1;
    uint32_t index = (endIndex - startIndex) / 2;
    while (startIndex != endIndex) {
        //cout << "val " << node << "\n";
        //cout << "start " << startIndex << " end " << endIndex << "\n";
        //cout << edgeBuffer[offset + startIndex + index] << "\n";
        if (edgeBuffer[offset + startIndex + index] == node) {
            return (offset + startIndex + index + 1 <= endOffset ? offset + startIndex + index + 1 : NONE);
        } else if (edgeBuffer[offset + startIndex + index] < node) {
            startIndex += index + 1;
        } else {
            if (!index) {
                return offset + startIndex + index;
            }
            endIndex = startIndex + index - 1;
        }
        index = (endIndex - startIndex) / 2;
    }
    if (edgeBuffer[offset + startIndex + index] <= node) {
        //cout << "it's " << offset + startIndex + index + 1 << " vs " << endOffset << endl;
        return (offset + startIndex + index + 1 <= endOffset ? offset + startIndex + index + 1 : NONE);
    } else {
        return offset + startIndex + index;
    }
}

void Graph::print(bool direction) const {
    cout << "Nodes: " << nodeIndex.size() << " Edges: " << edgeBuffer.size() / 2 << "\n";
    for (uint32_t pos = 0 ; pos < nodeIndex.size() ; pos++) {
        if (!nodeIndex[pos].removed) {
            uint32_t nextNodeOffset = (pos == nodeIndex.size()-1 ? edgeBuffer.size() : nodeIndex[pos+1].offset);
            for (uint32_t offset = nodeIndex[pos].offset ; offset < nextNodeOffset ; offset++) {
                uint32_t node = (!mapping ? pos : (*posToId)[pos]);
                uint32_t nPos = (!mapping ? edgeBuffer[offset] : (*idToPos)[edgeBuffer[offset]]);
                if (!nodeIndex[nPos].removed && (direction || !direction && node < edgeBuffer[offset])) {
                    cout << node << "\t" << edgeBuffer[offset] << "\n";
                }
            }
        }
    }
}

void Graph::printWithGraphTraversal(bool direction) const {
    GraphTraversal graphTraversal(*this);
    while (graphTraversal.curNode != NONE) {
        while (graphTraversal.curEdgeOffset != NONE) {
            if (direction || !direction && graphTraversal.curNode < edgeBuffer[graphTraversal.curEdgeOffset]) {
                cout << graphTraversal.curNode << "\t" << edgeBuffer[graphTraversal.curEdgeOffset] << "\n";
            }
            getNextEdge(graphTraversal);
        }
        getNextNode(graphTraversal);
    }
}

void Graph::printEdgeCounts() const {
    cout << "Nodes: " << nodeIndex.size() << " Edges: " << edgeBuffer.size() / 2 << "\n";
    for (uint32_t node = 0 ; node < nodeIndex.size() ; node++) {
        cout << "Node " << node << " has " << nodeIndex[node].getEdges() << " edges\n";
    }
}

Graph::Graph(const string &inputFile) : mapping(false), idToPos(NULL), posToId(NULL) {
    /* Open graph input file */
    FILE *f;
    f = fopen(inputFile.c_str(), "r");
    if (f == NULL) {
        cerr << "Error in opening input file " << inputFile << endl;
        exit(EXIT_FAILURE);
    }
    char buf[MAXLINE];

    /* Skip info lines */
    for (int i = 0 ; i < 3 ; i++) {
        if (fgets(buf, MAXLINE, f) == NULL) {
            cerr << "Error in parsing input file " << inputFile << endl;
            exit(EXIT_FAILURE);
        }
    }

    /* Get total node and edge numbers */
    char *ptr = strstr(buf, "Nodes:");
    uint32_t nodes = atoi(ptr + 7);
    ptr = strstr(buf, "Edges:");
    uint32_t edges = atoi(ptr + 7);
    if (fgets(buf, MAXLINE, f) == NULL) {
        cerr << "Error in parsing input file " << inputFile << endl;
        exit(EXIT_FAILURE);
    }

    try {
        nodeIndex.reserve(nodes);
    }
    catch (const length_error &le) {
        cerr << "NodeIndex length error: " << le.what() << endl;
        exit(EXIT_FAILURE);
    }

    try {
        edgeBuffer.reserve(edges * 2);
    }
    catch (const length_error &le) {
        cerr << "EdgeBuffer length error: " << le.what() << endl;
        exit(EXIT_FAILURE);
    }

    /* Hold reverse direction edges temporarily */
    vector<vector<uint32_t> > reverseEdges(nodes);

    /* Build graph with both edge directions, keep them sorted */
    uint32_t sourceNode, targetNode, previousNode, straightEdges, offset;
    previousNode = NONE;
    straightEdges = 0;
    offset = 0;
    while(fgets(buf, MAXLINE, f) != NULL) {
        parseNodeIDs(buf, &sourceNode, &targetNode);
        if (previousNode == NONE && sourceNode) {
            this->fill(sourceNode);
        }
        if (previousNode != sourceNode && previousNode != NONE) {
            for (uint32_t node = previousNode + 1 ; node <= sourceNode ; node++) {
                /* i==0: Add source node's smaller neighbors
                 * i>0 : Add smaller neighbors of nodes in-between previous node and source node (aka missing source nodes) */
                uint32_t previousEdges = straightEdges + reverseEdges[node - 1].size();
                nodeIndex.push_back(Graph::NodeInfo(offset, previousEdges));
                offset += previousEdges;
                edgeBuffer.insert(edgeBuffer.end(), reverseEdges[node].begin(), reverseEdges[node].end());
                straightEdges = 0;
                if (node < sourceNode) {
                    //cout << "in between missing node " << node << endl;
                }
            }
        }
        /* Add source node's bigger neighbors */
        edgeBuffer.push_back(targetNode);
        straightEdges++;
        reverseEdges[targetNode].push_back(sourceNode);
        previousNode = sourceNode;
    }
    /* Add final source node's info */
    uint32_t previousEdges = straightEdges + reverseEdges[previousNode].size();
    nodeIndex.push_back(Graph::NodeInfo(offset, previousEdges));
    offset += previousEdges;
    /* Add smaller neighbors of the nodes left that don't appear as source nodes */
    for (uint32_t missingNode = nodeIndex.size() ; missingNode < nodes ; missingNode++) {
        //cout << "end missing node " << missingNode << endl;
        previousEdges = reverseEdges[missingNode].size();
        nodeIndex.push_back(Graph::NodeInfo(offset, previousEdges));
        offset += previousEdges;
        edgeBuffer.insert(edgeBuffer.end(), reverseEdges[missingNode].begin(), reverseEdges[missingNode].end());
    }
    fclose(f);
}

void Graph::parseNodeIDs(char *buf, uint32_t *sourceNode, uint32_t *targetNode) {
    uint32_t *node = sourceNode;
    for (int i = 0 ; i < 2 ; i++) {
        uint32_t id = 0;
        for (; *buf != ' ' && *buf != '\t' && *buf != '\n' ; buf++) {
            id = id * 10 + (*buf - '0');
        }
        *node = id;
        node = targetNode;
        for (; *buf == ' ' || *buf == '\t' ; buf++);
    }
}

void Graph::fill(const uint32_t &size) {
    //cout << "start missing nodes from " << nodeIndex.size() << " to " << size << endl;
    while (nodeIndex.size() < size) {
        nodeIndex.push_back(NodeInfo(edgeBuffer.size(), 0));
    }
}
