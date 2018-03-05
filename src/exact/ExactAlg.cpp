#include <iostream>
#include <vector>
#include <unordered_map>
#include "ExactAlg.hpp"

using namespace std;

void ExactAlg::run() {
    this->reduce(6, 4);
}

void ExactAlg::reduce(const int &degree, const int &cliqueSize) {
    Graph newGraph;
    unordered_map<uint32_t, uint32_t> idToPos;
    vector<uint32_t> *posToId = new vector<uint32_t>();
    vector<Graph::NodeInfo> &newNodeIndex = newGraph.getNodeIndex();
    vector<uint32_t> &newEdgeBuffer = newGraph.getEdgeBuffer();
    vector<Graph::NodeInfo> &nodeIndex = graph.getNodeIndex();
    vector<uint32_t> &edgeBuffer = graph.getEdgeBuffer();
    for (uint32_t node = 0 ; node < nodeIndex.size() ; node++) {
        if (nodeIndex[node].getEdges() == degree) {
            uint32_t newOffset = newEdgeBuffer.size();
            uint32_t newEdges = 0;
            for (uint32_t offset = nodeIndex[node].getOffset() ; offset < nodeIndex[node].getOffset() + nodeIndex[node].getEdges() ; offset++) {
                if (nodeIndex[edgeBuffer[offset]].getEdges() == degree) {
                    newEdgeBuffer.push_back(edgeBuffer[offset]);
                    newEdges++;
                }
            }
            if (newEdges) {
                idToPos.insert({node, newNodeIndex.size()});
                posToId->push_back(node);
                newNodeIndex.push_back(Graph::NodeInfo(newOffset, newEdges));
            }
        }
    }
    newGraph.setPosToId(posToId);
    cout << " \n\nExactAlg\n\n";
    graph.printEdgeCounts();
    newGraph.print(true);
}
