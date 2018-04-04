#include <iostream>
#include "ControlUnit.hpp"

using namespace std;

void ControlUnit::run() {
    //return;
    //graph.print(true); return;
    //graph.printEdgeCounts(); return;
    //graph.printWithGraphTraversal(false); return;
    reductions.run();
}
