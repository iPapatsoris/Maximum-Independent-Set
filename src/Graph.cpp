#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "Graph.hpp"

#define MAXLINE 100

using namespace std;

void Graph::print(bool direction) const {
    cout << "Nodes: " << nodeIndex.size() << " Edges: " << edgeBuffer.size() / 2 << "\n";
    for (uint32_t pos = 0 ; pos < nodeIndex.size() ; pos++) {
        for (uint32_t offset = nodeIndex[pos].getOffset() ; offset < nodeIndex[pos].offset + nodeIndex[pos].edges ; offset++) {
            uint32_t node = (!mapping ? pos : (*posToId)[pos]);
            if (direction || !direction && node < edgeBuffer[offset]) {
                cout << node << "\t" << edgeBuffer[offset] << "\n";
            }
        }
    }
}

void Graph::printWithGraphTraversal(bool direction) const {
    GraphTraversal graphTraversal(*this);
    while (graphTraversal.curNode != NONE) {
        while (graphTraversal.curEdgeOffset != NONE) {
            if (direction || !direction && graphTraversal.curNode < this->edgeBuffer[graphTraversal.curEdgeOffset]) {
                cout << graphTraversal.curNode << "\t" << this->edgeBuffer[graphTraversal.curEdgeOffset] << "\n";
            }
            this->getNextEdge(graphTraversal);
        }
        this->getNextNode(graphTraversal);
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
            uint32_t node;
            for (int i = 0 ; ;) {
                /* i==0: Add source node's smaller neighbors
                 * i>0 : Add smaller neighbors of nodes in-between previous node and source node (aka missing source nodes) */
                if (!i) {
                    node = previousNode;
                } else if (i == 1) {
                    node = nodeIndex.size();
                }
                if (i && node >= sourceNode) {
                    break;
                }
                uint32_t previousEdges = straightEdges + reverseEdges[node].size();
                nodeIndex.push_back(Graph::NodeInfo(offset, previousEdges));
                offset += previousEdges;
                edgeBuffer.insert(edgeBuffer.end(), reverseEdges[(!i ? sourceNode : node)].begin(), reverseEdges[(!i ? sourceNode : node)].end());
                straightEdges = 0;
                if (i) {
                    //cout << "in between missing node " << node << endl;
                    node++;
                }
                if (i < 2) {
                    i++;
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
