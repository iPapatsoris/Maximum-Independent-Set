#ifndef UTIL_H
#define UTIL_H
#include <stdint.h>
#include <vector.h>
#include <algorithm.h>

#define NONE UINT32_MAX

/* Vectors should be sorted by default. Optimization check of container sizes should
 * be done from caller (since there might be removed nodes in the container) */
bool isSubsetOf(const std::vector<uint32_t> &container1, std::vector<uint32_t>::iterator &begin, std::vector<uint32_t>::iterator &end) {
    for (auto &i: container1) {
        if (!binary_search(begin, end, i)) {
            return false;
        }
    }
    return true;
}

bool setsHaveKUncommonElements(const std::vector<uint32_t> &container1, const std::vector<uint32_t> &container2, const uint32_t &k, uint32_t &uncommonElement) {
    uint32_t uncommonElements = 0;
    std::vector<uint32_t> *containerPtr = &container1;
    auto begin = container2.begin();
    auto end = container2.end();
    for (uint32_t it = 0 ; it < 2 ; it++) {
        for (auto &i: *containerPtr) {
            if (!binary_search(begin, end, i)) {
                uncommonElements++;
                uncommonElement = i;
                if (uncommonElements > k) {
                    return false;
                }
            }
        }
        containerPtr = &container2;
        begin = container1.begin();
        end = container1.end();
    }
    return (uncommonElements == k);
}

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
