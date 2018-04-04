#ifndef UTIL_H
#define UTIL_H
#include <stdint.h>

#define NONE UINT32_MAX

struct ReduceInfo {
public:
    ReduceInfo() : nodesRemoved(0), edgesRemoved(0) {}
    uint32_t nodesRemoved;
    uint32_t edgesRemoved;

    void print(ReduceInfo *old = NULL) const {
        std::cout << "Nodes removed " << nodesRemoved - (old == NULL ? 0 : old->nodesRemoved) << ", edges removed " << edgesRemoved - (old == NULL ? 0 : old->edgesRemoved) << std::endl;
    }
};

#endif
