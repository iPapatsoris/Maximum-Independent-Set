#include <iostream>
#include "ControlUnit.hpp"
#include "exact/ExactAlg.hpp"

using namespace std;

void ControlUnit::run() {
    //return;
    //graph.print(true); return;
    //graph.printEdgeCounts(); return;
    //graph.printWithGraphTraversal(false); return;
    ExactAlg exactAlg(graph);
    exactAlg.run();
}
