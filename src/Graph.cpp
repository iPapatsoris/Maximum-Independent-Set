#include <iostream>
#include "Graph.hpp"

using namespace std;

void Graph::print() const {
    cout << "Nodes: " << nodeIndex.size() << " Edges: " << edgeBuffer.size() / 2 << "\n";
    for (uint32_t node = 0 ; node < nodeIndex.size() ; node++) {
        for (uint32_t offset = nodeIndex[node].getOffset() ; offset < nodeIndex[node].getOffset() + nodeIndex[node].getEdges() ; offset++) {
            cout << node << "\t" << edgeBuffer[offset] << "\n";
        }
    }
}
