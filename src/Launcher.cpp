#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include "ControlUnit.hpp"

using namespace std;

int main(int argc, char **argv) {
    string inputFile;
    for (int i=1 ; i < argc ; i++) {
        if (!strcmp(argv[i], "-f")) {
           if (++i < argc) {
               inputFile = argv[i];
           }
       }
   }
   if (!inputFile.compare("")) {
       cerr << "Error: No input file specified" << endl;
       exit(EXIT_FAILURE);
   }
   ControlUnit controlUnit(inputFile);
   controlUnit.run();
}
