#include <iostream>
#include "ControlUnit.hpp"
#include "exact/ExactAlg.hpp"

using namespace std;

void ControlUnit::run() {
    ExactAlg exactAlg(graph);
    exactAlg.run();
}
