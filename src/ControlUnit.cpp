#include <iostream>
#include "ControlUnit.hpp"
#include "exact/ExactAlg.hpp"

using namespace std;

void ControlUnit::run() {
    graph.print(false);
    Graph::GraphTraversal graphTraversal(graph);
    while (graphTraversal.curNode != NONE) {
        while (graphTraversal.curEdgeOffset != NONE) {
            cout << graphTraversal.curNode << "\t" << graph.edgeBuffer[graphTraversal.curEdgeOffset] << "\n";
            graph.getNextEdge(graphTraversal);
        }
        graph.getNextNode(graphTraversal);
    }

    //ExactAlg exactAlg(graph);
    //exactAlg.run();
}
