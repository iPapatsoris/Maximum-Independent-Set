#include "Mis.hpp"
#include <iostream>
using namespace std;

string Mis::misOutputFile = ".mis";

/* Insert hypernode, mark any inner hypernodes' outerLevel as false */
void Mis::markHypernode(const uint32_t &hypernode, const vector<uint32_t> &nodes, const vector<uint32_t> &neighbors) {
    const vector<uint32_t> *container = &nodes;
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

void Mis::unfoldHypernodes(vector<uint32_t> &zeroDegreeNodes, vector<uint32_t> &finalMis) {
    //printAll(zeroDegreeNodes);
    finalMis.reserve(zeroDegreeNodes.size() + mis.size());
    expandIncludedNodes(mis, finalMis);
    expandIncludedNodes(zeroDegreeNodes, finalMis);
    expandExcludedNodes(finalMis);
}

void Mis::print(vector<uint32_t> &finalMis) {
    //printAll(zeroDegreeNodes);
    cout <<"\nWriting Maximum Independent Set to file " << Mis::misOutputFile << endl;
    cout << "\nMis size: " << finalMis.size() << "\n";
    FILE *f;
    f = fopen(Mis::misOutputFile.c_str(), "w");
    if (f == NULL) {
        cerr << "Error in opening mis output file " << Mis::misOutputFile << endl;
        exit(EXIT_FAILURE);
    }
    fprintf(f, "Maximum Independent Set size: %ld\n", finalMis.size());
    for (auto node : finalMis) {
        fprintf(f, "%" PRIu32 "\n", node);
    }
    fclose(f);
    //cout << "Zero degree nodes: " << zeroDegreeNodes.size() << "\n";
}

void Mis::expandExcludedNodes(vector<uint32_t> &finalMis) {
    while (!hypernodeToInnernode.empty()) {
        auto hypernode = hypernodeToInnernode.begin();
        while (!hypernode->second.outerLevel) {
            //cout << "Leftover inner hypernode " << hypernode->first << ", skipping for now\n";
            hypernode++;
        }
        //cout << "Leftover outer hypernode " << hypernode->first << ", examining inner nodes\n";
        for (auto node : hypernode->second.nodes) {
            auto nestedHypernode = hypernodeToInnernode.find(node);
            if (nestedHypernode == hypernodeToInnernode.end()) {
                //cout << " adding regular node " << node << "\n";
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

void Mis::printAll(vector<uint32_t> &zeroDegreeNodes) const {
    cout << "\nMis:\n";
    for (auto node: mis) {
        cout << node << "\n";
    }
    cout << "\nZero degree nodes:\n";
    for (auto node: zeroDegreeNodes) {
        cout << node << "\n";
    }
    cout << "\nHypernodes:\n";
    for (auto &hypernode: hypernodeToInnernode) {
        cout << hypernode.first << ", outer " << hypernode.second.outerLevel << "\n";
        for (auto node: hypernode.second.nodes) {
            cout << " node  " << node << "\n";
        }
        for (auto node: hypernode.second.neighbors) {
            cout << " neighbor " << node << "\n";
        }
    }
}
