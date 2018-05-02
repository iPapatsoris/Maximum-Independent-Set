#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include "ControlUnit.hpp"

using namespace std;

int main(int argc, char **argv) {
    string graphInputFile;
    bool checkIndependentSet = false;
    for (int i=1 ; i < argc ; i++) {
        if (!strcmp(argv[i], "-f")) {
            if (++i < argc) {
                graphInputFile = argv[i];
            }
        } else if (!strcmp(argv[i], "-check")) {
            checkIndependentSet = true;
        }
   }
   if (!graphInputFile.compare("")) {
       cerr << "Error: No graph input file specified" << endl;
       exit(EXIT_FAILURE);
   }
   ControlUnit controlUnit(graphInputFile, checkIndependentSet);
   if (!checkIndependentSet) {
       controlUnit.run();
   } else {
       controlUnit.checkIndependentSet(graphInputFile + ".mis");
   }
}
