#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "ControlUnit.hpp"

#define MAXLINE 100

using namespace std;


void ControlUnit::run() {
    graph.print(0);
}

ControlUnit::ControlUnit(const string &inputFile) {
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
        graph.nodeIndex.reserve(nodes);
    }
    catch (const length_error &le) {
	    cerr << "NodeIndex length error: " << le.what() << endl;
        exit(EXIT_FAILURE);
    }

    try {
        graph.edgeBuffer.reserve(edges * 2);
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
        if (previousNode != sourceNode && previousNode != NONE) {
            /* Add reverse direction edges */
            uint32_t node;
            for (int i = 0 ; ;) {
                /* i==0: Add node's reverse direction edges
                 * i>0 : Add reverse direction edges of in-between nodes that don't appear as source nodes */
                if (!i) {
                    node = previousNode;
                } else if (i == 1) {
                    node = graph.nodeIndex.size();
                }
                if (i && node >= sourceNode) {
                    break;
                }
                uint32_t previousEdges = straightEdges + reverseEdges[node].size();
                graph.nodeIndex.push_back(Graph::NodeInfo(offset, previousEdges));
                offset += previousEdges;
                graph.edgeBuffer.insert(graph.edgeBuffer.end(), reverseEdges[(!i ? sourceNode : node)].begin(), reverseEdges[(!i ? sourceNode : node)].end());
                straightEdges = 0;
                if (i) {
                    node++;
                }
                if (i < 2) {
                    i++;
                }
            }
        }
        /* Add straight direction edges */
        graph.edgeBuffer.push_back(targetNode);
        straightEdges++;
        reverseEdges[targetNode].push_back(sourceNode);
        previousNode = sourceNode;
    }
    /* Add final node's reverse direction edges*/
    uint32_t previousEdges = straightEdges + reverseEdges[previousNode].size();
    graph.nodeIndex.push_back(Graph::NodeInfo(offset, previousEdges));
    offset += previousEdges;
    /* Add reverse direction edges of the nodes left that don't appear as source nodes */
    for (uint32_t missingNode = graph.nodeIndex.size() ; missingNode < nodes ; missingNode++) {
        previousEdges = reverseEdges[missingNode].size();
        graph.nodeIndex.push_back(Graph::NodeInfo(offset, previousEdges));
        offset += previousEdges;
        graph.edgeBuffer.insert(graph.edgeBuffer.end(), reverseEdges[missingNode].begin(), reverseEdges[missingNode].end());
    }
    fclose(f);
}

void ControlUnit::parseNodeIDs(char *buf, uint32_t *sourceNode, uint32_t *targetNode) {
    uint32_t *node = sourceNode;
    for (int i = 0 ; i < 2 ; i++) {
        uint32_t id = 0;
        for (; *buf != ' ' && *buf != '\t' && *buf != '\n' ; buf++) {
            id = id * 10 + (*buf - '0');
        }
        *node = id;
        node = targetNode;
        buf++;
    }
}
