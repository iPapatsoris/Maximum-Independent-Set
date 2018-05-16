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
        SearchNode(const std::string &inputFile, const bool &checkIndependentSet) : id(NONE), theta(8), graph(inputFile, checkIndependentSet), reductions(new Reductions(graph, mis)), parent(NONE), leftChild(NONE), rightChild(NONE), finalMis(NULL) {}
        ~SearchNode();
        const Graph &getGraph() const {
            return graph;
        }
        void print() const;

        uint32_t id;
        uint32_t theta;
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
            MAX_DEGREE, SHORT_EDGE, OPTNODE, GOOD_FUNNEL, DONE
        };

        Type type;
        uint32_t node1;
        uint32_t node2;
        std::vector<uint32_t> container;


        BranchingRule() : type(Type::MAX_DEGREE), node1(NONE), node2(NONE) {}
        void clear() {
            node1 = NONE;
            node2 = NONE;
            container.clear();
        }

        void choose(const Graph &graph, Reductions &reductions, uint32_t &theta) {
            uint32_t maxDegree = 0;
            uint32_t maxDegreeNode = NONE;
            graph.getMaxNodeDegree(maxDegreeNode, maxDegree);
            bool run = true;
            while (run) {
                run = false;
                switch (theta) {
                    case 8:
                    case 7:
                    case 6:
                        if (maxDegree > theta) {
                            type = Type::MAX_DEGREE;
                        } else if (maxDegree == theta) {
                            graph.getOptimalShortEdge(theta, node1, node2, container);
                            if (node1 != NONE) {
                                type = Type::SHORT_EDGE;
                                std::cout << "optimal short edge " << node1 << " " << node2 << " with size " << container.size() << "\n";
                            } else {
                                type = Type::OPTNODE;
                                getOptimalNode(graph, theta);
                            }
                        } else {
                            theta--;
                            if (theta == 5 && graph.nodeIndex.size()) {
                                reductions.run(theta);
                                graph.getMaxNodeDegree(maxDegreeNode, maxDegree);
                            }
                            run = true;
                        }
                        break;
                    case 5:
                        if (!maxDegree) {
                            type = Type::DONE;
                        }
                        else if (maxDegree >= theta) {
                            type = Type::MAX_DEGREE;
                        } else if (maxDegree < theta) {
                            theta--;
                            if (theta == 4 && graph.nodeIndex.size()) {
                                reductions.run(theta);
                                graph.getMaxNodeDegree(maxDegreeNode, maxDegree);
                            }
                            run = true;
                        }
                        break;
                    case 4: {
                        type = Type::MAX_DEGREE;
                        if (maxDegree >= 5) {
                            uint32_t goodNode = graph.getGoodNode(reductions.getCCToNodes());
                            if (goodNode != NONE) {
                                maxDegreeNode = goodNode;
                            }
                        } else if (graph.getGoodFunnel(node1, node2)) {
                            type = Type::GOOD_FUNNEL;
                        }
                        if (maxDegree < theta) {
                            theta--;
                            if (theta == 3 && graph.nodeIndex.size()) {
                                reductions.run(theta);
                                graph.getMaxNodeDegree(maxDegreeNode, maxDegree);
                            }
                            run = true;
                        }
                        break;
                    }
                    case 3:
                    case 2:
                    case 1:
                        if (!maxDegree) {
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
            if (type == Type::MAX_DEGREE) {
                node1 = maxDegreeNode;
            }
        }

        void getOptimalNode(const Graph &graph, const uint32_t &theta) {
            bool found = false;
            uint32_t count = 0;
            Graph::GraphTraversal graphTraversal(graph);
            while (graphTraversal.curNode != NONE) {
                count++;
                //std::cout << "theta " << theta << " count " << count << "\n";
                uint32_t node = graphTraversal.curNode;
                switch (theta) {
                    case 8: {
                        uint32_t degree8Neighbors = graph.getNumberOfDegreeNeighbors(node, 8);
                        if (degree8Neighbors <= 7) {
                            found = true;
                        } else if (degree8Neighbors == 8 && getMeasure(graph, node) >= 36) {
                            found = true;
                        }
                        break;
                    }
                    case 7: {
                        std::unordered_set<uint32_t> extendedGrandchildren;
                        graph.getExtendedGrandchildren(graphTraversal, extendedGrandchildren, NULL, true);
                        if (extendedGrandchildren.size()) {
                            found = true;
                        } else {
                            uint32_t degree7Neighbors = graph.getNumberOfDegreeNeighbors(node, 7);
                            if (degree7Neighbors <= 5) {
                                found = true;
                            } else {
                                uint32_t measure = getMeasure(graph, node);
                                if (degree7Neighbors == 6 && measure >= 22 - 2 * graph.getNumberOfDegreeNeighbors(node, 3) - graph.getNumberOfDegreeNeighbors(node, 4) ||
                                degree7Neighbors == 7 && measure >= 26) {
                                    found = true;
                                }
                            }
                        }
                        break;
                    }
                    case 6: {
                        if (graph.getNumberOfDegreeNeighbors(node, 3) >= 1) {
                            found = true;
                        } else {
                            uint32_t degree6Neighbors = graph.getNumberOfDegreeNeighbors(node, 6);
                            if (degree6Neighbors <= 3) {
                                found = true;
                            } else {
                                uint32_t degree5Neighbors = graph.getNumberOfDegreeNeighbors(node, 5);
                                if (degree6Neighbors == 4 && degree5Neighbors <= 1) {
                                    found = true;
                                } else {
                                    std::unordered_set<uint32_t> neighborsAtDistance2;
                                    uint32_t count = 0;
                                    graph.getNeighborsAtDistance2(node, neighborsAtDistance2, theta, &count);
                                    uint32_t measure = getMeasure(graph, node, &neighborsAtDistance2);
                                    measure += count;
                                    if (degree6Neighbors == 4 && degree5Neighbors == 2 && measure >= 17 ||
                                    degree6Neighbors == 5 && graph.getNumberOfDegreeNeighbors(node, 4) == 1 && measure >= 18 ||
                                    degree6Neighbors == 5 && degree5Neighbors == 1 && measure >= 19 ||
                                    degree6Neighbors == 6 && measure >= 22) {
                                        found = true;
                                    }
                                }
                            }
                        }
                        break;
                    }
                    default:
                        assert(false);
                }
                if (found) {
                    break;
                }
                graph.getNextNode(graphTraversal);
            }
            assert(found);
            node1 = graphTraversal.curNode;
        }

        static uint32_t getMeasure(const Graph &graph, const uint32_t &node, std::unordered_set<uint32_t> *neighborsAtDistance2Ptr = NULL) {
            std::vector<uint32_t> neighbors;
            graph.gatherNeighbors(node, neighbors);
            neighbors.push_back(node);
            std::unordered_set<uint32_t> neighborsAtDistance2;
            if (neighborsAtDistance2Ptr == NULL) {
                graph.getNeighborsAtDistance2(node, neighborsAtDistance2);
                neighborsAtDistance2Ptr = &neighborsAtDistance2;
            }
            uint32_t f = graph.getNumberOfEdgesBetweenSets(neighbors, *neighborsAtDistance2Ptr);
            return 2 * f - neighborsAtDistance2Ptr->size();
        }

    };

    void chooseMaxMis(SearchNode *parent) {
        //std::cout << "no next child" << std::endl;
        SearchNode *leftChild = searchTree[parent->leftChild];
        SearchNode *rightChild = searchTree[parent->rightChild];
        assert(parent->rightChild == searchTree.size() - 1 && parent->leftChild == searchTree.size() - 2);
        parent->id = leftChild->id;
        parent->finalMis = leftChild->finalMis;
        std::vector<uint32_t> *min = rightChild->finalMis;
        if (rightChild->finalMis->size() > parent->finalMis->size()) {
            parent->id = rightChild->id;
            parent->finalMis = rightChild->finalMis;
            min = leftChild->finalMis;
        }
        delete min;
        delete searchTree.back();
        searchTree.pop_back();
        delete searchTree.back();
        searchTree.pop_back();
    }

    void branchLeft(const BranchingRule &branchingRule, SearchNode *searchNode) const {
        //std::cout << "left\n";
        switch (branchingRule.type) {
            case BranchingRule::Type::MAX_DEGREE:
            case BranchingRule::Type::OPTNODE: {
                searchNode->graph.remove(branchingRule.node1, searchNode->reductions->getReduceInfo());
                break;
            }
            case BranchingRule::Type::SHORT_EDGE: {
                std::vector<uint32_t> shortEdge;
                shortEdge.push_back(branchingRule.node1);
                shortEdge.push_back(branchingRule.node2);
                searchNode->graph.remove(shortEdge, searchNode->reductions->getReduceInfo());
                break;
            }
            case BranchingRule::Type::GOOD_FUNNEL: {
                branchOnExtendedGranchildren(branchingRule, searchNode);
                break;
            }
            default:
                assert(false);
        }
    }

    void branchRight(BranchingRule &branchingRule, SearchNode *searchNode) const {
        //std::cout << "right\n";
        switch (branchingRule.type) {
            case BranchingRule::Type::MAX_DEGREE:
            case BranchingRule::Type::OPTNODE: {
                branchOnExtendedGranchildren(branchingRule, searchNode);
                break;
            }
            case BranchingRule::Type::SHORT_EDGE: {
                branchingRule.container.push_back(branchingRule.node1);
                branchingRule.container.push_back(branchingRule.node2);
                searchNode->graph.remove(branchingRule.container, searchNode->reductions->getReduceInfo());
                std::vector<uint32_t> neighbors1, neighbors2;
                searchNode->graph.gatherNeighbors(branchingRule.node1, neighbors1);
                searchNode->graph.gatherNeighbors(branchingRule.node2, neighbors2);
                std::unordered_map<uint32_t, uint32_t> &subsequentNodes = searchNode->mis.getSubsequentNodes();
                for (auto neighbor1: neighbors1) {
                    subsequentNodes.insert({neighbor1, branchingRule.node2});
                    searchNode->graph.addEdges(neighbor1, neighbors2);
                }
                for (auto neighbor2: neighbors2) {
                    subsequentNodes.insert({neighbor2, branchingRule.node1});
                    searchNode->graph.addEdges(neighbor2, neighbors1);
                }
                //searchNode->graph.print(true);
                break;
            }
            case BranchingRule::Type::GOOD_FUNNEL: {
                searchNode->mis.getMis().push_back(branchingRule.node2);
                std::vector<uint32_t> neighbors;
                searchNode->graph.gatherNeighbors(branchingRule.node2, neighbors);
                neighbors.push_back(branchingRule.node2);
                searchNode->graph.remove(neighbors, searchNode->reductions->getReduceInfo());
                break;
            }
            default:
                assert(false);
        }
    }

    void branchOnExtendedGranchildren(const BranchingRule &branchingRule, SearchNode *searchNode) const {
        std::unordered_set<uint32_t> extendedGrandchildren;
        Graph::GraphTraversal graphTraversal(searchNode->graph, branchingRule.node1);
        searchNode->graph.getExtendedGrandchildren(graphTraversal, extendedGrandchildren);
        extendedGrandchildren.insert(branchingRule.node1);
        std::vector<uint32_t> &mis = searchNode->mis.getMis();
        mis.insert(mis.end(), extendedGrandchildren.begin(), extendedGrandchildren.end());
        std::unordered_set<uint32_t> neighbors;
        searchNode->graph.gatherAllNeighbors(extendedGrandchildren, neighbors);

        std::unordered_set<uint32_t> *smaller = &neighbors;
        std::unordered_set<uint32_t> *bigger = &extendedGrandchildren;
        if (smaller->size() > bigger->size()) {
            std::unordered_set<uint32_t> *tmp = smaller;
            smaller = bigger;
            bigger = tmp;
        }
        smaller->insert(bigger->begin(), bigger->end());
        searchNode->graph.remove(std::vector<uint32_t>(smaller->begin(), smaller->end()), searchNode->reductions->getReduceInfo());
    }

    std::vector<SearchNode *> searchTree;
};

#endif
