#ifndef MIS_H
#define MIS_H
#include <stdint.h>
#include <vector>
#include <unordered_map>
#include <assert.h>

class Mis {
public:
    std::vector<uint32_t> &getMis() {
        return mis;
    }

    void markHypernode(const uint32_t &hypernode, const uint32_t &node) {
        uint32_t val = node;
        auto it = hypernodeToNode.find(node);
        if (it != hypernodeToNode.end()) {
            val = it->second;
            hypernodeToNode.erase(it);
        }
        auto res = hypernodeToNode.insert({hypernode, val});
        assert(res.second);
    }

    void print(const std::vector<uint32_t> zeroDegreeNodes) const {
        std::cout <<"Mis:\n";
        for (auto it = mis.begin() ; it != mis.end() ; it++) {
            auto res = hypernodeToNode.find(*it);
            std::cout << (res == hypernodeToNode.end() ? *it : res->second) << "\n";
        }
        for (auto it = zeroDegreeNodes.begin() ; it != zeroDegreeNodes.end() ; it++) {
            auto res = hypernodeToNode.find(*it);
            std::cout << (res == hypernodeToNode.end() ? *it : res->second) << "\n";
        }
        std::cout << "\nMis size: " << mis.size() << "\n";
        std::cout << "Zero degree nodes: " << zeroDegreeNodes.size() << "\n";
    }

private:
    std::vector<uint32_t> mis;
    std::unordered_map<uint32_t, uint32_t> hypernodeToNode; // Map hypernodes to their inner node that forms a mis
};

#endif
