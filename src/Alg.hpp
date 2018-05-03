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
    struct SearchNode {
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
        std::vector<uint32_t> *finalMis; // Final mis of children search nodes, no hypernodes
    };

    struct BranchingRule {
    public:
        enum class Type {
            MAX_DEGREE, DONE
        };

        Type type;
        uint32_t node1;
        uint32_t node2;

        BranchingRule() : type(Type::MAX_DEGREE), node1(NONE), node2(NONE) {}
        void findOptimalShortEdge(const Graph &graph);

        void choose(const Graph &graph, uint32_t &theta) {
            uint32_t maxDegree;
            graph.getMaxNodeDegree(node1, maxDegree);
            bool run = true;
            while (run) {
                run = false;
                switch (theta) {
                    case 8:
                    case 7:
                    case 6:
                    case 5:
                    case 4:
                    case 3:
                    case 2:
                    case 1:
                        if (node1 == NONE) {
                            type = Type::DONE;
                        }
                        else if (maxDegree >= theta) {
                            type = Type::MAX_DEGREE;
                        } else if (maxDegree < theta) {
                            theta--;
                            run = true;
                        }
                        break;
                    default:
                        assert(false);
                }
            }
        }
    };

    void chooseMaxMis(SearchNode *parent) {
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

    void branchLeft(const BranchingRule &branchingRule, SearchNode *searchNode) {
        //std::cout << "left\n";
        switch (branchingRule.type) {
            case BranchingRule::Type::MAX_DEGREE: {
                searchNode->graph.remove(branchingRule.node1, searchNode->reductions->getReduceInfo());
                break;
            }
            default:
                assert(false);
        }
    }

    void branchRight(const BranchingRule &branchingRule, SearchNode *searchNode) {
        //std::cout << "right\n";
        switch (branchingRule.type) {
            case BranchingRule::Type::MAX_DEGREE: {
                std::unordered_set<uint32_t> extendedGrandchildren;
                Graph::GraphTraversal graphTraversal(searchNode->graph, branchingRule.node1);
                searchNode->graph.getExtendedGrandchildren(graphTraversal, extendedGrandchildren);
                extendedGrandchildren.insert(branchingRule.node1);
                std::vector<uint32_t> &mis = searchNode->mis.getMis();
                mis.insert(mis.end(), extendedGrandchildren.begin(), extendedGrandchildren.end());
                std::unordered_set<uint32_t> neighbors;
                searchNode->graph.gatherNeighbors(extendedGrandchildren, neighbors);

                std::unordered_set<uint32_t> *smaller = &neighbors;
                std::unordered_set<uint32_t> *bigger = &extendedGrandchildren;
                if (smaller->size() > bigger->size()) {
                    std::unordered_set<uint32_t> *tmp = smaller;
                    smaller = bigger;
                    bigger = tmp;
                }
                smaller->insert(bigger->begin(), bigger->end());
                searchNode->graph.remove(std::vector<uint32_t>(smaller->begin(), smaller->end()), searchNode->reductions->getReduceInfo());
                break;
            }
            default:
                assert(false);
        }
    }

    std::vector<SearchNode *> searchTree;
};

#endif
