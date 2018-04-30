#ifndef CONTROLUNIT_H
#define CONTROLUNIT_H

#include <string>
#include "Graph.hpp"
#include "Reductions.hpp"
#include "Alg.hpp"
#include "Mis.hpp"

class ControlUnit {
public:
    ControlUnit(const std::string &inputFile, const bool &checkIndependentSet) : alg(inputFile, checkIndependentSet) {}
    void run();
    void checkIndependentSet(const std::string &misInputFile) const;

private:
    Alg alg;
};

#endif
