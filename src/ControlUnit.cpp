#include <iostream>
#include "ControlUnit.hpp"
#include "exact/ExactAlg.hpp"

using namespace std;

void ControlUnit::run() {
    //graph.print(true); return;
    //graph.printEdgeCounts(); return;
    //graph.printWithGraphTraversal(true); return;
    ExactAlg exactAlg(graph);
    exactAlg.run();
}
