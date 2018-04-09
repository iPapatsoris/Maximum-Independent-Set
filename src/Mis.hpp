#ifndef MIS_H
#define MIS_H
#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <assert.h>

struct Innernode {
public:
    Innernode(const std::vector<uint32_t> &nodes, const std::vector<uint32_t> &neighbors) : nodes(nodes), neighbors(neighbors) {}

    std::vector<uint32_t> nodes;
    std::vector<uint32_t> neighbors;
};

class Mis {
public:
    std::vector<uint32_t> &getMis() {
        return mis;
    }

    void markHypernode(const uint32_t &hypernode, const std::vector<uint32_t> &nodes, const std::vector<uint32_t> &neighbors) {
        assert(hypernodeToInnernode.insert({hypernode, Innernode(nodes, neighbors)}).second);
    }

    /* Changes structures, should be called only once at the end */
    void print(std::vector<uint32_t> &zeroDegreeNodes) {
        std::cout <<"Mis:\n";
        std::vector<uint32_t> finalMis;
        finalMis.reserve(zeroDegreeNodes.size() + mis.size());
        print(mis, finalMis);
        print(zeroDegreeNodes, finalMis);
        while (!hypernodeToInnernode.empty()) {
            auto hypernode = hypernodeToInnernode.begin();
            std::cout << "Leftover hypernode " << hypernode->first << "\n";
            for (auto node : hypernode->second.nodes) {
                auto nestedHypernode = hypernodeToInnernode.find(node);
                if (nestedHypernode == hypernodeToInnernode.end()) {
                    std::cout << node << "\n";
                    finalMis.push_back(node);
                } else {
                    hypernodeToInnernode.insert(*nestedHypernode);
                }
            }
            hypernodeToInnernode.erase(hypernode);
        }
        std::cout << "\nMis size: " << finalMis.size() << "\n";
        //std::cout << "Zero degree nodes: " << zeroDegreeNodes.size() << "\n";
    }

private:
    std::vector<uint32_t> mis;
    std::unordered_map<uint32_t, Innernode> hypernodeToInnernode;

    void print(std::vector<uint32_t> &set, std::vector<uint32_t> &finalMis) {
        for (uint32_t i = 0 ; i < set.size() ; i++) {
            auto res = hypernodeToInnernode.find(set[i]);
            if (res == hypernodeToInnernode.end()) {
                std::cout << set[i] << "\n";
                finalMis.push_back(set[i]);
            } else {
                for (auto node : res->second.neighbors) {
                    if (hypernodeToInnernode.find(node) == hypernodeToInnernode.end()) {
                        std::cout << node << "\n";
                        finalMis.push_back(node);
                    } else {
                        set.push_back(node);
                    }
                }
                hypernodeToInnernode.erase(res);
            }
        }
    }
};

#endif
