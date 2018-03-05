#include <iostream>
#include "ControlUnit.hpp"
#include "exact/ExactAlg.hpp"

using namespace std;

void ControlUnit::run() {
    graph.print(true);
    ExactAlg exactAlg(graph);
    exactAlg.run();
}
