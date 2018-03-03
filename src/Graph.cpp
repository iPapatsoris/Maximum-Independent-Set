#include <iostream>
#include "Graph.hpp"

using namespace std;

void Graph::print(bool direction) const {
    cout << "Nodes: " << nodeIndex.size() << " Edges: " << edgeBuffer.size() / 2 << "\n";
    for (uint32_t node = 0 ; node < nodeIndex.size() ; node++) {
        for (uint32_t offset = nodeIndex[node].getOffset() ; offset < nodeIndex[node].getOffset() + nodeIndex[node].getEdges() ; offset++) {
            if (direction || !direction && node < edgeBuffer[offset]) {
                cout << node << "\t" << edgeBuffer[offset] << "\n";
            }
        }
    }
}
