#ifndef ALG_H
#define ALG_H

#include "Graph.hpp"
#include "Reductions.hpp"
#include "Mis.hpp"


class Alg {
    class SearchNode;

public:
    Alg(const std::string &inputFile, const bool &checkIndependentSet);
    ~Alg();
    void run();
    const std::vector<SearchNode *> &getSearchTree() const {
        return searchTree;
    }
    void print() const;

private:
    class SearchNode {
    public:
        SearchNode(const SearchNode &searchNode, const uint32_t &parent = NONE);
        SearchNode(const std::string &inputFile, const bool &checkIndependentSet) : graph(inputFile, checkIndependentSet), reductions(new Reductions(graph, mis)), parent(NONE), leftChild(NONE), rightChild(NONE), finalMis(NULL) {}
        ~SearchNode();
        const Graph &getGraph() const {
            return graph;
        }
        void print() const;

        Graph graph;
        Mis mis; // Current mis and hypernodes
        Reductions *reductions;
        uint32_t parent;
        uint32_t leftChild;
        uint32_t rightChild;
        std::vector<uint32_t> *finalMis; // Final mis of children, no hypernodes
    };

    void chooseMaxMis(SearchNode *parent, std::vector<SearchNode *> &searchTree) {
        //std::cout << "no next child" << std::endl;
        SearchNode *leftChild = searchTree[parent->leftChild];
        SearchNode *rightChild = searchTree[parent->rightChild];
        assert(parent->rightChild == searchTree.size() - 1 && parent->leftChild == searchTree.size() - 2);
        parent->finalMis = leftChild->finalMis;
        std::vector<uint32_t> *min = rightChild->finalMis;
        if (rightChild->finalMis->size() > parent->finalMis->size()) {
            parent->finalMis = rightChild->finalMis;
            min = leftChild->finalMis;
        }
        delete min;
        delete searchTree.back();
        searchTree.pop_back();
        delete searchTree.back();
        searchTree.pop_back();
    }

    enum class BranchingRule {
        MAX_DEGREE
    };

    void chooseBranchingRule(Graph &graph, BranchingRule &branchingRule, uint32_t &node) {
        branchingRule = BranchingRule::MAX_DEGREE;
        uint32_t maxDegree;
        graph.getMaxNodeDegree(node, maxDegree);
    }

    void branchLeft(const BranchingRule &branchingRule, SearchNode *searchNode, const uint32_t &node) {
        //std::cout << "left\n";
        switch (branchingRule) {
            case BranchingRule::MAX_DEGREE: {
                searchNode->graph.remove(node, searchNode->reductions->getReduceInfo());
                break;
            }
            default:
                assert(false);
        }
    }

    void branchRight(const BranchingRule &branchingRule, SearchNode *searchNode, const uint32_t &node) {
        //std::cout << "right\n";
        switch (branchingRule) {
            case BranchingRule::MAX_DEGREE: {
                std::set<uint32_t> extendedGrandchildren;
                Graph::GraphTraversal graphTraversal(searchNode->graph, node);
                searchNode->graph.getExtendedGrandchildren(graphTraversal, extendedGrandchildren);
                extendedGrandchildren.insert(node);
                std::vector<uint32_t> &mis = searchNode->mis.getMis();
                mis.insert(mis.end(), extendedGrandchildren.begin(), extendedGrandchildren.end());
                std::set<uint32_t> neighbors;
                searchNode->graph.gatherNeighbors(extendedGrandchildren, neighbors);
                neighbors.insert(extendedGrandchildren.begin(), extendedGrandchildren.end());
                searchNode->graph.remove(std::vector<uint32_t>(neighbors.begin(), neighbors.end()), searchNode->reductions->getReduceInfo()); // Todo: generic remove function
                break;
            }
            default:
                assert(false);
        }
    }

    std::vector<SearchNode *> searchTree;
};

#endif
