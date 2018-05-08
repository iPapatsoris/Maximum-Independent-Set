#include <iostream>
#include <vector>
#include <stack>
#include <unordered_map>
#include <algorithm>
#include <unordered_set>
#include <iomanip>
#include "Reductions.hpp"

using namespace std;

void Reductions::run(const uint32_t &theta) {
    //cout << " \nReductions\n";
    //printCC();
    //graph.print(true);
    switch(theta) {
        case 8:
        case 7:
        case 6:
            reduce6(theta);
            break;
        case 5:
        case 4:
        case 3:
        case 2:
        case 1:
            reduce5(theta);
            break;
        default:
            assert(false);
    }
    //graph.printEdgeCounts();
    //graph.printWithGraphTraversal(true);
    //graph.print(true);
    //printCCSizes();
}

void Reductions::reduce6(const uint32_t &theta) {
    bool firstTime = true;
    unordered_set<uint32_t> *oldCandidateNodes = new unordered_set<uint32_t>();
    unordered_set<uint32_t> *newCandidateNodes = new unordered_set<uint32_t>();
    while (removeUnconfinedNodes() || firstTime) {
        if (!foldCompleteKIndependentSets(firstTime, &oldCandidateNodes, &newCandidateNodes)) {
            break;
        }
    }
    delete oldCandidateNodes;
    delete newCandidateNodes;
    buildCC();
    removeLineGraphs(theta);
    graph.rebuild(reduceInfo);
}

void Reductions::reduce5(const uint32_t &theta) {
    while (removeUnconfinedNodes()) {
        ;
    }
    removeEasyInstances(theta);
    graph.rebuild(reduceInfo);
}

void Reductions::removeEasyInstances(const uint32_t &theta) {
    buildCC();
    //printCCSizes();
    vector<unordered_map<uint32_t, vector<uint32_t>* >::iterator> removedCCs;
    for (auto it = ccToNodes.begin() ; it != ccToNodes.end() ; it++) {
        vector<uint32_t> *cc = it->second;
            if (cc->size() == 28) {
                //cout << "Removing easy instance component " << it->first << "\n";
                //findMis(*cc);
                graph.remove(*cc, reduceInfo, true);
                removedCCs.push_back(it);
            }
        }
    if (removedCCs.empty()) {
        //cout << "No nodes removed" << endl;
    } else {
        for (auto cc : removedCCs) {
            delete cc->second;
            ccToNodes.erase(cc);
        }
    }
}

/* Find mis via brute force. For easy instances */
void Reductions::findMis(const vector<uint32_t> &cc) {
    vector<uint32_t> independentSet;
    vector<uint32_t> maximumIndependentSet;
    vector<uint32_t> frontier; // cc array indexes
    vector<unordered_set<uint32_t> > removedNodes;
    frontier.push_back(0);
    removedNodes.push_back(unordered_set<uint32_t>());
    while (frontier.size()) {
        uint32_t node = cc[frontier.back()];
        Graph::GraphTraversal graphTraversal(graph, node);
        while (graphTraversal.curEdgeOffset != NONE) {
            removedNodes.back().insert((*graph.edgeBuffer)[graphTraversal.curEdgeOffset]);
            graph.getNextEdge(graphTraversal);
        }
        uint32_t newNode = NONE;
        uint32_t newIndex;
        bool found = false;
        for (newIndex = frontier.back()+1 ; newIndex < cc.size() ; newIndex++) {
            if (cc.size() - newIndex + frontier.size() <= maximumIndependentSet.size()) {
                break;
            }
            newNode = cc[newIndex];
            if (removedNodes.back().find(newNode) == removedNodes.back().end()) {
                found = true;
                break;
            }
        }
        if (found) {
            frontier.push_back(newIndex);
            removedNodes.push_back(removedNodes.back());
        } else {
            /*for (auto &i: frontier) {
                cout << i << " ";
            }
            cout << "\n";*/
            if (frontier.size() > maximumIndependentSet.size()) {
                maximumIndependentSet = frontier;
            }
            while (frontier.size()) {
                newIndex = frontier.back() + 1;
                frontier.pop_back();
                removedNodes.pop_back();
                found = false;
                for (; newIndex < cc.size() ; newIndex++) {
                    if (cc.size() - newIndex + frontier.size() <= maximumIndependentSet.size()) {
                        break;
                    }
                    newNode = cc[newIndex];
                    if (!removedNodes.size() || removedNodes.size() && removedNodes.back().find(newNode) == removedNodes.back().end()) {
                        found = true;
                        break;
                    }
                }
                if (found) {
                    frontier.push_back(newIndex);
                    removedNodes.push_back((removedNodes.size() ? removedNodes.back() : unordered_set<uint32_t>()));
                    break;
                }
            }
        }
    }
    /*for (auto &i: maximumIndependentSet) {
        cout << i << " ";
    }
    cout << "\n";*/
    vector<uint32_t> &mis = this->mis.getMis();
    for (auto &i: maximumIndependentSet) {
        mis.push_back(cc[i]);
    }
}

bool Reductions::foldCompleteKIndependentSets(bool &firstTime, unordered_set<uint32_t> **oldCandidateNodes, unordered_set<uint32_t> **newCandidateNodes) {
    //cout << "\n**Performing K-Independent set folding reduction**" << endl;
    ReduceInfo old = reduceInfo;
    foldCompleteKIndependentSets2(firstTime, **oldCandidateNodes, **newCandidateNodes);
    swap(oldCandidateNodes, newCandidateNodes);
    if (firstTime) {
        firstTime = false;
    }
    if (old.nodesRemoved == reduceInfo.nodesRemoved) {
        //cout << "No nodes removed." << endl;
        return false;
    }
    do {
        //reduceInfo.print(&old);
        old = reduceInfo;
        foldCompleteKIndependentSets2(firstTime, **oldCandidateNodes, **newCandidateNodes);
        swap(oldCandidateNodes, newCandidateNodes);
    } while (old.nodesRemoved != reduceInfo.nodesRemoved);
    return true;
}


bool Reductions::removeUnconfinedNodes() {
    //cout << "\n**Performing unconfined nodes reduction**" << endl;
    ReduceInfo old = reduceInfo;
    removeUnconfinedNodes2();
    if (old.nodesRemoved == reduceInfo.nodesRemoved) {
        //cout << "No nodes removed." << endl;
        return false;
    }
    do {
        //reduceInfo.print(&old);
        old = reduceInfo;
        removeUnconfinedNodes2();
        //assert(old.nodesRemoved == reduceInfo.nodesRemoved);
    } while (old.nodesRemoved != reduceInfo.nodesRemoved);
    return true;
}

void Reductions::foldCompleteKIndependentSets2(const bool &checkAllNodes, unordered_set<uint32_t> &oldCandidateNodes, unordered_set<uint32_t> &newCandidateNodes) {
    newCandidateNodes.clear();
    Graph::GraphTraversal graphTraversal(graph);
    auto it = oldCandidateNodes.begin();
    uint32_t node = (checkAllNodes ? graphTraversal.curNode : (it == oldCandidateNodes.end() ? NONE : *it));
    uint32_t bound = 5000;
    while (node != NONE) {
        for (uint32_t k = 1 ; k <= 2 ; k++) {
            if ((checkAllNodes || (!checkAllNodes && !graph.nodeIndex[graph.getPos(*it)].removed)) && graph.getNodeDegree(node) == k+1) {
                if (node >= bound) {
                    //cout << fixed << setprecision(0) << ((float) bound / graph.nodeIndex.size()) * 100 << "% done" << endl;
                    bound += 5000;
                }
                vector<uint32_t> nodes;
                vector<uint32_t> neighbors;
                nodes.push_back(node);
                graph.gatherNeighbors(node, neighbors);
                if (k == 2) {
                    uint32_t secondNode = graph.getNextNodeWithIdenticalNeighbors(node, neighbors);
                    if (secondNode != NONE) {
                        nodes.push_back(secondNode);
                    }
                }
                if (k == 1 || k == 2 && nodes.size() == 2) {
                    vector<uint32_t> &mis = this->mis.getMis();
                    if (graph.isIndependentSet(neighbors)) {
                        uint32_t newNode = graph.contractToSingleNode(nodes, neighbors, reduceInfo);
                        this->mis.markHypernode(newNode, nodes, neighbors);
                    } else {
                        mis.insert(mis.end(), nodes.begin(), nodes.end());
                    }
                    neighbors.insert(neighbors.end(), nodes.begin(), nodes.end());
                    graph.remove(neighbors, reduceInfo, false, &newCandidateNodes);
                    //graph.print(true);
                }
                break;
            }
        }
        if (checkAllNodes) {
            graph.getNextNode(graphTraversal);
            node = graphTraversal.curNode;
        } else {
            it++;
            node = (it == oldCandidateNodes.end() ? NONE : *it);
        }
    }
}

void Reductions::removeUnconfinedNodes2() {
    Graph::GraphTraversal graphTraversal(graph);
    //uint32_t boolCount = 0;
    //uint32_t independentCount = 0;
    while (graphTraversal.curNode != NONE) {
        ////cout << "node " << graphTraversal.curNode << "\n";
        bool isUnconfined = false;
        unordered_set<uint32_t> extendedGrandchildren;
        graph.getExtendedGrandchildren(graphTraversal, extendedGrandchildren, &isUnconfined);
        if (isUnconfined || !graph.isIndependentSet(extendedGrandchildren)) {
            /*if (isUnconfined) {
                boolCount++;
            } else {
                independentCount++;
            }*/
            //cout << "Unconfined node " << graphTraversal.curNode << "\n";
            graph.remove(graphTraversal.curNode, reduceInfo);
        }
        graph.getNextNode(graphTraversal);
    }
    //cout << "bool " << boolCount << ", independent " << independentCount << endl;
}

void Reductions::removeLineGraphs(const uint32_t &theta) {
    //cout << "\n**Performing line graph reduction**" << endl;
    //printCCSizes();
    vector<unordered_map<uint32_t, vector<uint32_t>* >::iterator> removedCCs;
    for (auto it = ccToNodes.begin() ; it != ccToNodes.end() ; it++) {
        vector<uint32_t> *cc = it->second;
        vector<Graph::GraphTraversal> clique1;
        vector<Graph::GraphTraversal> clique2;
        clique1.reserve(5);
        clique2.reserve(5);
        uint32_t checks;
        if (theta >= 6) {
            checks = 3;
        } else {
            checks = 1;
        }
        for (uint32_t i = 0 ; i < checks ; i++) {
            uint32_t nodes, degree, clique1Size, clique2Size;
            if (theta >= 6) {
                switch (i) {
                    case 0:
                        nodes = 10;
                        degree = 6;
                        clique1Size = clique2Size = 4;
                        break;
                    case 1:
                        nodes = 15;
                        degree = 8;
                        clique1Size = clique2Size = 5;
                        break;
                    case 2:
                        nodes = 20;
                        degree = 7;
                        clique1Size = 5;
                        clique2Size = 4;
                        break;
                }
            } else {
                nodes = 12;
                degree = 5;
                clique1Size = 4;
                clique2Size = 3;
            }
            if (cc->size() == nodes && nodeDegreesEqualTo(*cc, degree, graph)) {
                bool foundCliques = true;
                for (auto node : *cc) {
                    clique1.clear();
                    clique2.clear();
                    clique1.push_back(Graph::GraphTraversal(graph, node));
                    if (!findClique(clique1, NULL, clique1Size)) {
                        foundCliques = false;
                        break;
                    }
                    //cout << "Clique1: " << endl;
                    for (uint32_t i = 0 ; i < clique1.size() ; i++) {
                        //cout << clique1[i].curNode << endl;
                    }
                    clique2.push_back(Graph::GraphTraversal(graph, node));
                    if (!findClique(clique2, &clique1, clique2Size)) {
                        foundCliques = false;
                        break;
                    }
                    //cout << "Clique2: " << endl;
                    for (uint32_t i = 0 ; i < clique2.size() ; i++) {
                        //cout << clique2[i].curNode << endl;
                    }
                }
                if (foundCliques) {
                    //cout << "Removing component " << it->first << "\n";
                    findMisInComponent(*cc);
                    graph.remove(*cc, reduceInfo, true);
                    reduceInfo.edgesRemoved += ((nodes * degree) / 2);
                    removedCCs.push_back(it);
                    break;
                } else {
                    break;
                }
            }
        }
    }
    if (removedCCs.empty()) {
        //cout << "No nodes removed" << endl;
    } else {
        for (auto cc : removedCCs) {
            delete cc->second;
            ccToNodes.erase(cc);
        }
    }
}

void Reductions::findMisInComponent(const vector<uint32_t> &cc) {
    unordered_set<uint32_t> removedNodes;
    for (auto node : cc) {
        if (removedNodes.find(node) == removedNodes.end()) {
            mis.getMis().push_back(node);
            Graph::GraphTraversal graphTraversal(graph, node);
            while (graphTraversal.curEdgeOffset != NONE) {
                removedNodes.insert((*graph.edgeBuffer)[graphTraversal.curEdgeOffset]);
                graph.getNextEdge(graphTraversal);
            }
        }
    }
}

bool Reductions::findClique(vector<Graph::GraphTraversal> &clique, vector<Graph::GraphTraversal> *previousClique, const uint32_t &cliqueSize) {
    Graph::GraphTraversal graphTraversal = clique[0];
    if (graphTraversal.curNode == NONE) {
        return false;
    }
    while (clique.size() < cliqueSize) {
        //if (clique[0].curNode == 11) {
        ////cout << "Cur clique: \n";
        //for (uint32_t i = 0 ; i < clique.size() ; i++) {
        ////cout << clique[i].curNode << endl;
        //}
        //}
        uint32_t neighbor = (*graph.edgeBuffer)[graphTraversal.curEdgeOffset];
        bool existsInPreviousClique = (previousClique == NULL ? false : find(neighbor, *previousClique));
        ////cout << "node " << graphTraversal.curNode << " neighbor " << neighbor << endl;
        if (!existsInPreviousClique && !find(neighbor, clique) && isSubsetOfNeighbors(clique, neighbor, graph)) {
            ////cout << "going to " << neighbor << endl;
            ////cout << "commonNode is " << commonNode << endl;
            graph.goToNode(neighbor, graphTraversal);
            clique.push_back(graphTraversal);
        } else {
            if (!advance(clique, graphTraversal, graph)) {
                return false;
            }
        }
    }
    return true;
}

void Reductions::buildCC() {
    uint32_t component = 0;
    unordered_set<uint32_t> exploredSet;
    stack<uint32_t> frontier;
    Graph::GraphTraversal graphTraversal(graph);
    while (graphTraversal.curNode != NONE) {
        if (exploredSet.insert(graphTraversal.curNode).second) {
            vector<uint32_t> *componentNodes = new vector<uint32_t>();
            componentNodes->push_back(graphTraversal.curNode);
            frontier.push(graphTraversal.curNode);
            bool big = false;
            while (!frontier.empty()) {
                uint32_t node = frontier.top();
                frontier.pop();
                Graph::GraphTraversal neighbors(graph, node);
                while (neighbors.curEdgeOffset != NONE) {
                    node = (*graph.edgeBuffer)[neighbors.curEdgeOffset];
                    if (exploredSet.insert(node).second) {
                        frontier.push(node);
                        if (!big) {
                            componentNodes->push_back(node);
                        }
                        else if (componentNodes->size() > 20) {
                            big = true;
                        }
                    }
                    graph.getNextEdge(neighbors);
                }
            }
            if (!big) {
                ccToNodes.insert({component, componentNodes});
                component++;
            } else {
                delete componentNodes;
            }
        }
        graph.getNextNode(graphTraversal);
    }
}

Reductions::~Reductions() {
    for (auto &cc : ccToNodes) {
        delete cc.second;
    }
}

void Reductions::printCC() const {
    for (auto &cc : ccToNodes) {
        //cout << "\nCC " << cc.first << ":\n";
    }
}

void Reductions::printCCSizes() const {
    for (auto &cc : ccToNodes) {
        cout << "CC " << cc.first << " size: " << (cc.second)->size() << "\n";
    }
}

void Reductions::swap(unordered_set<uint32_t> **p1, unordered_set<uint32_t> **p2) {
    unordered_set<uint32_t> *tmp = *p1;
    *p1 = *p2;
    *p2 = tmp;
}
