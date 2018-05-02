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
        MAX_DEGREE, ANY
    };

    void chooseBranchingRule(Graph &graph, BranchingRule &branchingRule, uint32_t &node) {
        branchingRule = BranchingRule::ANY;
        node = Graph::GraphTraversal(graph).curNode;
    }

    void branchLeft(const BranchingRule &branchingRule, SearchNode *searchNode, const uint32_t &node) {
        //std::cout << "left\n";
        switch (branchingRule) {
            case BranchingRule::MAX_DEGREE: {
                break;
            }
            case BranchingRule::ANY: {
                searchNode->mis.getMis().push_back(node);
                std::vector<uint32_t> neighbors;
                neighbors.push_back(node);
                searchNode->graph.gatherNeighbors(node, neighbors);
                searchNode->graph.remove(neighbors, searchNode->reductions->getReduceInfo());
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
                break;
            }
            case BranchingRule::ANY: {
                searchNode->graph.remove(node, searchNode->reductions->getReduceInfo());
                break;
            }
            default:
                assert(false);
        }
    }

    std::vector<SearchNode *> searchTree;
};

#endif
