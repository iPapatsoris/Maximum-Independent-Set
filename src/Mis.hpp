#ifndef MIS_H
#define MIS_H
#include <stdint.h>
#include <inttypes.h>
#include <vector>
#include <unordered_map>
#include <assert.h>

struct Innernode {
public:
    Innernode(const std::vector<uint32_t> &nodes, const std::vector<uint32_t> &neighbors) : nodes(nodes), neighbors(neighbors), outerLevel(true) {}

    std::vector<uint32_t> nodes;
    std::vector<uint32_t> neighbors;
    bool outerLevel;
};

class Mis {
public:
    Mis(const std::string &misOutputFile) : misOutputFile(misOutputFile) {}

    std::vector<uint32_t> &getMis() {
        return mis;
    }

    /* Insert hypernode, mark any inner hypernodes' outerLevel as false */
    void markHypernode(const uint32_t &hypernode, const std::vector<uint32_t> &nodes, const std::vector<uint32_t> &neighbors) {
        const std::vector<uint32_t> *container = &nodes;
        for (uint32_t i = 0 ; i < 2 ; i++) {
            for (auto node: *container) {
                auto it = hypernodeToInnernode.find(node);
                if (it != hypernodeToInnernode.end()) {
                    it->second.outerLevel = false;
                }
            }
            container = &neighbors;
        }
        assert(hypernodeToInnernode.insert({hypernode, Innernode(nodes, neighbors)}).second);
    }

    /* Changes structures, should be called only once at the end */
        void print(std::vector<uint32_t> &zeroDegreeNodes) {
        //printAll(zeroDegreeNodes);
        std::cout <<"\nWriting Maximum Independent Set to file " << misOutputFile << std::endl;
        std::vector<uint32_t> finalMis;
        finalMis.reserve(zeroDegreeNodes.size() + mis.size());
        expandIncludedNodes(mis, finalMis);
        expandIncludedNodes(zeroDegreeNodes, finalMis);
        expandExcludedNodes(finalMis);

        std::cout << "\nMis size: " << finalMis.size() << "\n";
        FILE *f;
        f = fopen(misOutputFile.c_str(), "w");
        if (f == NULL) {
            std::cerr << "Error in opening mis output file " << misOutputFile << std::endl;
            exit(EXIT_FAILURE);
        }
        fprintf(f, "Maximum Independent Set size: %ld\n", finalMis.size());
        for (auto node : finalMis) {
            fprintf(f, "%" PRIu32 "\n", node);
        }
        fclose(f);
        //std::cout << "Zero degree nodes: " << zeroDegreeNodes.size() << "\n";
    }

private:
    std::vector<uint32_t> mis;
    std::unordered_map<uint32_t, Innernode> hypernodeToInnernode;
    std::string misOutputFile;

    void expandIncludedNodes(std::vector<uint32_t> &set, std::vector<uint32_t> &finalMis) {
        for (uint32_t i = 0 ; i < set.size() ; i++) {
            auto res = hypernodeToInnernode.find(set[i]);
            if (res == hypernodeToInnernode.end()) {
                //std::cout << "Adding regular node " << set[i] << "\n";
                //fprintf(f, "%ld\n", set[i]);
                finalMis.push_back(set[i]);
            } else {
                //std::cout << "Examining inner nodes of " << set[i] << "\n";
                assert(res->second.outerLevel);
                for (auto node : res->second.neighbors) {
                    auto innerHypernode = hypernodeToInnernode.find(node);
                    if (innerHypernode == hypernodeToInnernode.end()) {
                        //std::cout << " adding regular node " << node << "\n";
                        //fprintf(f, "%ld\n", node);
                        finalMis.push_back(node);
                    } else {
                        //std::cout << "queing " << node << " for later examination\n";
                        assert(!innerHypernode->second.outerLevel);
                        innerHypernode->second.outerLevel = true;
                        set.push_back(node);
                    }
                }
                for (auto node : res->second.nodes) {
                    auto innerHypernode = hypernodeToInnernode.find(node);
                    if (innerHypernode != hypernodeToInnernode.end()) {
                        assert(!innerHypernode->second.outerLevel);
                        innerHypernode->second.outerLevel = true;
                    }
                }
                hypernodeToInnernode.erase(res);
            }
        }
        set.clear();
    }

    void expandExcludedNodes(std::vector<uint32_t> &finalMis) {
        while (!hypernodeToInnernode.empty()) {
            auto hypernode = hypernodeToInnernode.begin();
            while (!hypernode->second.outerLevel) {
                //std::cout << "Leftover inner hypernode " << hypernode->first << ", skipping for now\n";
                hypernode++;
            }
            //std::cout << "Leftover outer hypernode " << hypernode->first << ", examining inner nodes\n";
            for (auto node : hypernode->second.nodes) {
                auto nestedHypernode = hypernodeToInnernode.find(node);
                if (nestedHypernode == hypernodeToInnernode.end()) {
                    //std::cout << " adding regular node " << node << "\n";
                    //fprintf(f, "%ld\n", node);
                    finalMis.push_back(node);
                } else {
                    assert(!nestedHypernode->second.outerLevel);
                    nestedHypernode->second.outerLevel = true;
                    mis.push_back(nestedHypernode->first);
                }
            }
            for (auto node : hypernode->second.neighbors) {
                auto innerHypernode = hypernodeToInnernode.find(node);
                if (innerHypernode != hypernodeToInnernode.end()) {
                    assert(!innerHypernode->second.outerLevel);
                    innerHypernode->second.outerLevel = true;
                }
            }
            hypernodeToInnernode.erase(hypernode);
            expandIncludedNodes(mis, finalMis);
        }
    }

    void printAll(std::vector<uint32_t> &zeroDegreeNodes) const {
        std::cout << "\nMis:\n";
        for (auto node: mis) {
            std::cout << node << "\n";
        }
        std::cout << "\nZero degree nodes:\n";
        for (auto node: zeroDegreeNodes) {
            std::cout << node << "\n";
        }
        std::cout << "\nHypernodes:\n";
        for (auto &hypernode: hypernodeToInnernode) {
            std::cout << hypernode.first << ", outer " << hypernode.second.outerLevel << "\n";
            for (auto node: hypernode.second.nodes) {
                std::cout << " node  " << node << "\n";
            }
            for (auto node: hypernode.second.neighbors) {
                std::cout << " neighbor " << node << "\n";
            }
        }
    }
};

#endif
