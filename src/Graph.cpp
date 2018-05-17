#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <set>
#include "Graph.hpp"

using namespace std;

Graph::Graph(const Graph &graph) {
    nodeIndex = graph.nodeIndex;
    zeroDegreeNodes = graph.zeroDegreeNodes;
    nextUnusedId = graph.nextUnusedId;
    mapping = graph.mapping;
    edgeBuffer = new vector<uint32_t>(*(graph.edgeBuffer));
    if (graph.mapping) {
        idToPos = new unordered_map<uint32_t, uint32_t>(*(graph.idToPos));
        posToId = new vector<uint32_t>(*(graph.posToId));
    }
}

Graph& Graph::operator=(const Graph &graph) {
    if (this != &graph) {
        nodeIndex = graph.nodeIndex;
        zeroDegreeNodes = graph.zeroDegreeNodes;
        nextUnusedId = graph.nextUnusedId;
        mapping = graph.mapping;
        edgeBuffer = new vector<uint32_t>(*(graph.edgeBuffer));
        if (graph.mapping) {
            idToPos = new unordered_map<uint32_t, uint32_t>(*(graph.idToPos));
            posToId = new vector<uint32_t>(*(graph.posToId));
        }
    }
    return *this;
}

Graph::~Graph() {
    delete edgeBuffer;
    if (mapping) {
        delete idToPos;
        delete posToId;
    }
}

Graph::GraphTraversal::GraphTraversal(const Graph &graph) {
    curNode = NONE;
    curEdgeOffset = NONE;
    graph.getNextNode(*this);
}

Graph::GraphTraversal::GraphTraversal(const Graph &graph, const uint32_t &node) {
    graph.goToNode(node, *this);
}

/* Could be potentially replaced by structures that are updated on the fly as degrees change,
 * but is pretty fast and memory efficient nonetheless, even on huge graphs */
void Graph::getMaxNodeDegree(uint32_t &node, uint32_t &maxDegree) const {
    //cout << "Finding max node degree" << endl;
    node = NONE;
    maxDegree = 0;
    for (uint32_t i = 0 ; i < nodeIndex.size() ; i++) {
        if (!nodeIndex[i].removed && nodeIndex[i].edges > maxDegree) {
            node = (!mapping ? i : posToId->at(i));
            maxDegree = nodeIndex[i].edges;
        }
    }
    //cout << "node " << node << " with max degree " << maxDegree << endl;
}

/* Optimized for short funnel detection to not necesssarily return the min degree,
 * if it is less than 3 */
void Graph::getMinDegree(uint32_t &minDegree) const {
    //cout << "Finding min degree" << endl;
    minDegree = NONE;
    for (uint32_t i = 0 ; i < nodeIndex.size() ; i++) {
        if (!nodeIndex[i].removed && nodeIndex[i].edges && nodeIndex[i].edges < minDegree) {
            minDegree = nodeIndex[i].edges;
            if (minDegree < 3) {
                return;
            }
        }
    }
}

uint32_t Graph::getTotalEdges() const {
    uint32_t count = 0;
    for (uint32_t pos = 0 ; pos < nodeIndex.size() ; pos++) {
        if (!nodeIndex[pos].removed) {
            count += nodeIndex[pos].edges;
        }
    }
    return count;
}

uint32_t Graph::getNumberOfDegreeNeighbors(const uint32_t &node, const uint32_t &degree) const {
    uint32_t count = 0;
    uint32_t pos = (!mapping ? node : idToPos->at(node));
    uint32_t neighborCount = nodeIndex[pos].edges;
    uint32_t nextNodeOffset = (pos == nodeIndex.size()-1 ? edgeBuffer->size() : nodeIndex[pos+1].offset);
    for (uint32_t i = nodeIndex[pos].offset ; i < nextNodeOffset && neighborCount ; i++) {
        uint32_t nPos = (!mapping ? (*edgeBuffer)[i] : idToPos->at((*edgeBuffer)[i]));
        if (!nodeIndex[nPos].removed) {
            neighborCount--;
            if (nodeIndex[nPos].edges == degree) {
                count++;
            }
        }
    }
    return count;
}

/* Does not include neighbors at distance 2 that can also be found at distance 1.
 * If the last 2 optional arguments are given, it also counts the number of neighbors
 * at distance 2 with degree less than 'degree' */
void Graph::getNeighborsAtDistance2(const uint32_t &node, unordered_set<uint32_t> &neighbors, const uint32_t &degree, uint32_t *count) const {
    if (degree != NONE && count != NULL) {
        *count = 0;
    }
    uint32_t pos = (!mapping ? node : idToPos->at(node));
    uint32_t neighborCount = nodeIndex[pos].edges;
    uint32_t nextNodeOffset = (pos == nodeIndex.size()-1 ? edgeBuffer->size() : nodeIndex[pos+1].offset);
    for (uint32_t i = nodeIndex[pos].offset ; i < nextNodeOffset && neighborCount ; i++) {
        uint32_t nPos = (!mapping ? (*edgeBuffer)[i] : idToPos->at((*edgeBuffer)[i]));
        if (!nodeIndex[nPos].removed) {
            neighborCount--;
            uint32_t neighborCount2 = nodeIndex[nPos].edges;
            uint32_t nextNodeOffset2 = (nPos == nodeIndex.size()-1 ? edgeBuffer->size() : nodeIndex[nPos+1].offset);
            for (uint32_t j = nodeIndex[nPos].offset ; j < nextNodeOffset2 && neighborCount2 ; j++) {
                uint32_t id = (*edgeBuffer)[j];
                uint32_t nPos2 = (!mapping ? id : idToPos->at(id));
                if (!nodeIndex[nPos2].removed) {
                    neighborCount2--;
                    if (id != node && !edgeExists(id, node)) {
                        neighbors.insert(id);
                        if (degree != NONE && count != NULL && nodeIndex[nPos2].edges < degree) {
                            (*count)++;
                        }
                    }
                }
            }
        }
    }
}


void Graph::remove(const uint32_t &node, ReduceInfo &reduceInfo) {
    remove(std::vector<uint32_t>(1, node), reduceInfo);
}

/* Rebuild structures, completely removing nodes that are marked as removed
 * and collecting zero degree nodes */
void Graph::rebuild(ReduceInfo &reduceInfo) {
    if (!reduceInfo.nodesRemoved) {
        return;
    }
    vector<NodeInfo> nodeIndex;
    uint32_t newNodes = this->nodeIndex.size() - reduceInfo.nodesRemoved;
    uint32_t newEdges = this->getTotalEdges();
    //cout << "\nRebuilding: nodes removed " << reduceInfo.nodesRemoved << ", edges removed " << reduceInfo.edgesRemoved << endl;
    nodeIndex.reserve(newNodes);
    vector<uint32_t> *edgeBuffer = new vector<uint32_t>();
    edgeBuffer->reserve(newEdges);
    unordered_map<uint32_t, uint32_t> *idToPos = new unordered_map<uint32_t, uint32_t>();
    vector<uint32_t> *posToId = new vector<uint32_t>();
    posToId->reserve(newNodes);
    uint32_t offset = 0;

    for (uint32_t pos = 0 ; pos < this->nodeIndex.size() ; pos++) {
        if (this->nodeIndex[pos].removed) {
            continue;
        }
        uint32_t node = (!mapping ? pos : (*this->posToId)[pos]);
        if (!this->nodeIndex[pos].edges) {
            //cout << "Found node " << node << " with no edges at rebuilding\n";
            zeroDegreeNodes.push_back(node);
            continue;
        }
        uint32_t edges = 0;
        uint32_t nextNodeOffset = (pos == this->nodeIndex.size()-1 ? this->edgeBuffer->size() : this->nodeIndex[pos+1].offset);
        /* Don't add neighbors that are marked as removed */
        for (uint32_t i = this->nodeIndex[pos].offset ; i < nextNodeOffset ; i++) {
            uint32_t nPos = (!this->mapping ? (*this->edgeBuffer)[i] : this->idToPos->at((*this->edgeBuffer)[i]));
            if (!this->nodeIndex[nPos].removed) {
                edgeBuffer->push_back((*this->edgeBuffer)[i]);
                edges++;
            }
            if (edges == this->nodeIndex[pos].edges) {
                break;
            }
        }
        assert(edges > 0);
        idToPos->insert({node, nodeIndex.size()});
        posToId->push_back(node);
        nodeIndex.push_back(Graph::NodeInfo(offset, edges));
        offset += edges;
    }
    this->mapping = true;
    if (this->idToPos != NULL) {
        delete this->idToPos;
    }
    if (this->posToId != NULL) {
        delete this->posToId;
    }
    this->idToPos = idToPos;
    this->posToId = posToId;
    //assert(nodeIndex.size() == this->nodeIndex.size() - reduceInfo.nodesRemoved - zeroDegreeNodes.size());
    this->nodeIndex = nodeIndex;
    delete this->edgeBuffer;
    this->edgeBuffer = edgeBuffer;
    //cout << "Rebuilding: nodes removed " << reduceInfo.nodesRemoved << ", edges removed " << reduceInfo.edgesRemoved << endl;
    reduceInfo.nodesRemoved = 0;
}

/* Contract 'nodes' and 'neighbors' to a single node.
 * It is taken for granted that the only neighbors of 'nodes' are 'neighbors' */
uint32_t Graph::contractToSingleNode(const vector<uint32_t> &nodes, const vector<uint32_t> &neighbors, ReduceInfo &reduceInfo) {
    /*cout << "Contracting node " << nodes[0];
    if (nodes.size() == 2) {
        cout << " and " << nodes[1];
    }
    cout <<" with their neighbors" << endl;*/
    uint32_t newNode = nextUnusedId;
    assert(++nextUnusedId != 0);
    assert(!mapping || mapping && idToPos->find(newNode) == idToPos->end());
    set<uint32_t> newNeighbors;
    for (auto it = neighbors.begin() ; it != neighbors.end() ; it++) {
        GraphTraversal graphTraversal(*this, *it);
        while (graphTraversal.curEdgeOffset != NONE) {
            uint32_t neighbor = (*edgeBuffer)[graphTraversal.curEdgeOffset];
            //cout << "\nneighbor " << neighbor << endl;
            if (find(nodes.begin(), nodes.end(), neighbor) == nodes.end() && find(neighbors.begin(), neighbors.end(), neighbor) == neighbors.end()) {
                if (newNeighbors.insert(neighbor).second) {
                    replaceNeighbor(neighbor, *it, newNode);
                    uint32_t pos = (!mapping ? neighbor : idToPos->at(neighbor));
                    nodeIndex[pos].edges++;
                }
            }
            getNextEdge(graphTraversal);
        }
    }
    reduceInfo.nodesRemoved--;
    if (!newNeighbors.size()) {
        zeroDegreeNodes.push_back(newNode);
    } else {
        uint32_t offset = edgeBuffer->size();
        edgeBuffer->reserve(edgeBuffer->size() + newNeighbors.size());
        copy(newNeighbors.begin(), newNeighbors.end(), back_inserter(*edgeBuffer));
        nodeIndex.push_back(NodeInfo(offset, newNeighbors.size()));
        if (mapping) {
            idToPos->insert({newNode, nodeIndex.size() - 1});
            posToId->push_back(newNode);
        }
    }
    //print(true);
    //cout << "hypernode " << newNode << endl;
    return newNode;
}

void Graph::replaceNeighbor(const uint32_t &node, const uint32_t &oldNeighbor, const uint32_t &newNeighbor) {
    uint32_t offset;
    offset = findEdgeOffset(node, oldNeighbor);
    //print(true);
    //if (offset == NONE) {
    //    cout << "replacing node " << node << " old neighbor " << oldNeighbor << " with " << newNeighbor << endl;
    //    this->print(true);
    //}
    assert(offset != NONE);
    uint32_t pos = (!mapping ? node : idToPos->at(node));
    uint32_t endOffset = (pos == nodeIndex.size()-1 ? edgeBuffer->size() : nodeIndex[pos+1].offset);
    auto it = edgeBuffer->begin();
    move(it + offset + 1, it + endOffset, it + offset);
    (*edgeBuffer)[endOffset-1] = newNeighbor;
}

uint32_t Graph::getNextNodeWithIdenticalNeighbors(const uint32_t &previousNode, const vector<uint32_t> &neighbors) const {
    uint32_t pos = (!mapping ? previousNode : idToPos->at(previousNode));
    for (pos = pos+1 ; pos < nodeIndex.size() ; pos++) {
        if (!nodeIndex[pos].removed && nodeIndex[pos].edges == neighbors.size()) {
            uint32_t neighborCount = neighbors.size();
            uint32_t nextNodeOffset = (pos == nodeIndex.size()-1 ? edgeBuffer->size() : nodeIndex[pos+1].offset);
            for (uint32_t offset = nodeIndex[pos].offset ; offset  < nextNodeOffset && neighborCount; offset++) {
                uint32_t nPos = (!mapping ? (*edgeBuffer)[offset] : idToPos->at((*edgeBuffer)[offset]));
                if (!nodeIndex[nPos].removed && find(neighbors.begin(), neighbors.end(), (*edgeBuffer)[offset]) != neighbors.end()) {
                    neighborCount--;
                }
            }
            if (!neighborCount) {
                return (!mapping ? pos : posToId->at(pos));
            }
        }
    }
    return NONE;
}

uint32_t Graph::getNodeWithOneUncommonNeighbor(const vector<uint32_t> &neighbors, uint32_t &uncommonNeighbor) const {
    uncommonNeighbor = NONE;
    for (uint32_t pos = 0 ; pos < nodeIndex.size() ; pos++) {
        if (!nodeIndex[pos].removed && nodeIndex[pos].edges == 3 || nodeIndex[pos].edges == 4) { // Optimization for 3-4 structures reduction
            uint32_t node = (!mapping ? pos : posToId->at(pos));
            vector<uint32_t> newNeighbors;
            gatherNeighbors(node, newNeighbors);
            if (setsHaveKUncommonElements(neighbors, newNeighbors, 1, uncommonNeighbor)) {
                return node;
            }
        }
    }
    return NONE;
}

void Graph::getExtendedGrandchildren(Graph::GraphTraversal &graphTraversal, unordered_set<uint32_t> &extendedGrandchildren, bool *isUnconfined, const bool &stopAtFirst) const {
    while (graphTraversal.curEdgeOffset != NONE) {
        uint32_t neighbor = (*edgeBuffer)[graphTraversal.curEdgeOffset];
        uint32_t outerNeighbor;
        bool exactlyOne;
        getOuterNeighbor(graphTraversal.curNode, neighbor, outerNeighbor, exactlyOne);
        if (isUnconfined != NULL && outerNeighbor == NONE) {
            ////cout << "none\n";
            *isUnconfined = true;
            break;
        } else if (exactlyOne) {
            ////cout << "exactly one\n";
            extendedGrandchildren.insert(outerNeighbor);
            if (stopAtFirst) {
                return;
            }
        }
        getNextEdge(graphTraversal);
    }
}

bool Graph::get4Cycle(vector<uint32_t> &cycle) const {
    cycle.clear();
    GraphTraversal graphTraversal(*this);
    while (graphTraversal.curNode != NONE) {
        if (getNodeDegree(graphTraversal.curNode) == 4) {
            vector<uint32_t> neighbors;
            gatherNeighbors(graphTraversal.curNode, neighbors);
            uint32_t b, d;
            for (uint32_t i = 0 ; i < 6 ; i++) {
                switch (i) {
                    case 0:
                        b = neighbors[0];
                        d = neighbors[1];
                        break;
                    case 1:
                        b = neighbors[0];
                        d = neighbors[2];
                        break;
                    case 2:
                        b = neighbors[0];
                        d = neighbors[3];
                        break;
                    case 3:
                        b = neighbors[1];
                        d = neighbors[2];
                        break;
                    case 4:
                        b = neighbors[1];
                        d = neighbors[3];
                        break;
                    case 5:
                        b = neighbors[2];
                        d = neighbors[3];
                        break;
                    default:
                        assert(false);
                }
                vector<uint32_t> commonNeighbors;
                getCommonNeighbors(b, d, commonNeighbors, 2);
                if (commonNeighbors.size() == 2) {
                    uint32_t c = (commonNeighbors[0] == graphTraversal.curNode ? commonNeighbors[1] : commonNeighbors[0]);
                    cycle.push_back(graphTraversal.curNode);
                    cycle.push_back(b);
                    cycle.push_back(c);
                    cycle.push_back(d);
                    cout << "branching on cycle " << graphTraversal.curNode << "-" << b << "-" << c << "-" << d << "\n";
                    return true;
                }
            }
        }
        getNextNode(graphTraversal);
    }
    return false;
}

bool Graph::getGoodFunnel(uint32_t &node1, uint32_t &node2) const {
    vector<Funnel> funnels;
    if (getFunnels(funnels)) {
        Funnel &funnel = funnels.back();
        node1 = funnel.a;
        node2 = funnel.v;
        cout << "branching on good funnel ";
        funnel.print();
        return true;
    } else {
        for (uint32_t i = 0 ; i < 2 ; i++) {
            for (auto &funnel: funnels) {
                uint32_t bDegree = getNodeDegree(funnel.b);
                uint32_t cDegree = getNodeDegree(funnel.c);
                if (!i && bDegree == 4 && cDegree == 4 || i && (bDegree == 4 || cDegree == 4)) {
                    node1 = funnel.a;
                    node2 = funnel.v;
                    cout << "branching on good funnel ";
                    funnel.print();
                    return true;
                }
            }
        }
    }
    return false;
}

bool Graph::getFunnels(vector<Funnel> &funnels) const {
    GraphTraversal graphTraversal(*this);
    while (graphTraversal.curNode != NONE) {
        uint32_t nodeV = graphTraversal.curNode;
        uint32_t vDegree = getNodeDegree(nodeV);
        if (vDegree == 3 || vDegree == 4) {
            //cout << "nodeV " << nodeV << endl;
            vector<uint32_t> neighborsV;
            gatherNeighbors(nodeV, neighborsV);
            for (uint32_t i = 0 ; i < neighborsV.size() ; i++) {
                uint32_t nodeA = neighborsV[i];
                uint32_t nodeB, nodeC, nodeD;
                if (i == 0) {
                    nodeB = neighborsV[1];
                    nodeC = neighborsV[2];
                    nodeD = (neighborsV.size() == 3 ? NONE : neighborsV[3]);
                } else if (i == 1) {
                    nodeB = neighborsV[0];
                    nodeC = neighborsV[2];
                    nodeD = (neighborsV.size() == 3 ? NONE : neighborsV[3]);
                } else if (i == 2) {
                    nodeB = neighborsV[0];
                    nodeC = neighborsV[1];
                    nodeD = (neighborsV.size() == 3 ? NONE : neighborsV[3]);
                } else {
                    nodeB = neighborsV[0];
                    nodeC = neighborsV[1];
                    nodeD = (neighborsV.size() == 3 ? NONE : neighborsV[2]);
                }
                if (edgeExists(nodeB, nodeC) && (vDegree == 3 || vDegree == 4 && edgeExists(nodeB, nodeD) && edgeExists(nodeC, nodeD)))  {
                    funnels.push_back(Funnel(nodeA, nodeB, nodeC, nodeD, nodeV));
                    if (vDegree == 4 || (getNodeDegree(nodeA) == 4 && (getNodeDegree(nodeB) == 4 || getNodeDegree(nodeC) == 4))) {
                        return true;
                    }
                }
            }
        }
        getNextNode(graphTraversal);
    }
    return false;
}

uint32_t Graph::getGoodNode(unordered_map<uint32_t, vector<uint32_t>* > &ccToNodes) const {
    for (auto &cc: ccToNodes) {
        vector<uint32_t> &nodes = *(cc.second);
        if (nodes.size() < 9) {
            continue;
        }
        uint32_t start = 26;
        if (nodes.size() < 29) {
            start = nodes.size() - 3;
        }
        for (uint32_t node: nodes) {
            assert(!nodeIndex[getPos(node)].removed);
            if (getNodeDegree(node) < 5) {
                continue;
            }
            for (uint32_t size = start ; size >= 6 ; size--) {
                vector<Traversal *> frontier;
                unordered_set<uint32_t> set;
                frontier.push_back(new Traversal(node, *this));
                set.insert(node);
                uint32_t goodNode = getGoodNode(frontier, set, nodes, size);
                if (goodNode != NONE) {
                    //cout << "Found good node for good set of size " << size << "\n";
                    return goodNode;
                }
            }
        }
    }
    return NONE;
}

uint32_t Graph::getGoodNode(vector<Traversal *> &frontier, unordered_set<uint32_t> &set, vector<uint32_t> &nodes, const uint32_t &size) const {
    //cout << "Size " << size << endl;
    Traversal *traversal = frontier[0];
    if (traversal->index == NONE) {
        return NONE;
    }
    while (true) {
        auto it = traversal->set.begin();
        std::advance(it, traversal->index);
        uint32_t neighbor = *it;
        //cout << "node " << graphTraversal.curNode << " neighbor " << neighbor << endl;
        if (set.find(neighbor) == set.end()) {
            frontier.push_back(new Traversal(neighbor, *this, frontier[frontier.size()-1]));
            set.insert(neighbor);
            traversal = frontier[frontier.size()-1];
            //for (auto &n: set) {
            //    cout << n << endl;
            //}
            //cout << "\n";
            if (set.size() == size) {
                unordered_set<uint32_t> neighbors;
                gatherAllNeighbors(set, neighbors, 3);
                if (neighbors.size() == 3) {
                    //cout << "FOUND" << endl;
                    return *(neighbors.begin());
                } else {
                    //cout << "NOT FOUND" << endl;
                    delete traversal;
                    frontier.pop_back();
                    set.erase(neighbor);
                    if (!advance(frontier, &traversal, *this, set)) {
                        return NONE;
                    }
                }
            }
        } else {
            if (!advance(frontier, &traversal, *this, set)) {
                return NONE;
            }
        }
    }
    return true;
}

void Graph::getOptimalShortEdge(const uint32_t &degree, uint32_t &finalNode1, uint32_t &finalNode2, vector<uint32_t> &finalContainer) const {
    finalNode1 = NONE;
    finalNode2 = NONE;
    vector<uint32_t> commonNeighbors;
    bool done = false;
    for (uint32_t pos = 0 ; pos < nodeIndex.size() && !done ; pos++) {
        if (nodeIndex[pos].removed) {
            continue;
        }
        if (nodeIndex[pos].edges == degree) {
            uint32_t neighborCount = degree;
            uint32_t nextNodeOffset = (pos == nodeIndex.size()-1 ? edgeBuffer->size() : nodeIndex[pos+1].offset);
            for (uint32_t offset = nodeIndex[pos].offset ; offset < nextNodeOffset && neighborCount; offset++) {
                uint32_t nPos = (!mapping ? (*edgeBuffer)[offset] : idToPos->at((*edgeBuffer)[offset]));
                if (nodeIndex[nPos].removed) {
                    continue;
                }
                neighborCount--;
                if (nodeIndex[nPos].edges == degree || (degree == 6 && nodeIndex[nPos].edges == 5)) {
                    uint32_t node1 = (!mapping ? pos : posToId->at(pos));
                    uint32_t node2 = (!mapping ? nPos : posToId->at(nPos));
                    if (node1 < node2) {
                        getCommonNeighbors(node1, node2, commonNeighbors);
                        if ((degree == 6 && commonNeighbors.size() >= 3 || (degree == 7 || degree == 8) && commonNeighbors.size() >= 4) &&
                        commonNeighbors.size() > finalContainer.size()) {
                            //cout << node1 << " " << node2 << " common neighbors: " << commonNeighbors.size() << "\n";
                            finalContainer.clear();
                            finalContainer.insert(finalContainer.end(), commonNeighbors.begin(), commonNeighbors.end());
                            finalNode1 = node1;
                            finalNode2 = node2;
                            if (finalContainer.size() == degree - 1) {
                                done = true;
                                break;
                            }
                        }
                        commonNeighbors.clear();
                    }
                }
            }
        }
    }
}

void Graph::getCommonNeighbors(const uint32_t &node1, const uint32_t &node2, vector<uint32_t> &commonNeighbors, const uint32_t &atLeast) const {
    uint32_t pos1 = (!mapping ? node1 : idToPos->at(node1));
    uint32_t pos2 = (!mapping ? node2 : idToPos->at(node2));
    assert(!nodeIndex[pos1].removed && !nodeIndex[pos2].removed);
    uint32_t count = 0;
    vector<uint32_t> neighbors1;
    gatherNeighbors(node1, neighbors1);
    for (auto &neighbor: neighbors1) {
        if (edgeExists(neighbor, node2)) {
            commonNeighbors.push_back(neighbor);
            if (atLeast && ++count == atLeast) {
                return;
            }
        }
    }
}

/* Connect 'node' with 'nodes'. Since moving elements and a vector reallocation is possible,
 * the mirror edges are not added. It is more performant to add them manually with another call
 * to this function, along with any other edges. (Optimization for branching on edges) */
void Graph::addEdges(const uint32_t node, const vector<uint32_t> &nodes) {
    /*cout << "connecting " << node << " with ";
    for (auto &node: nodes) {
        cout << node << " ";
    }
    cout << "\n";*/
    uint32_t pos = (!mapping ? node : idToPos->at(node));
    set<uint32_t> neighbors;
    vector<uint32_t> removedNeighbors;
    gatherNeighborsWithRemoved(node, neighbors, removedNeighbors);
    uint32_t space = neighbors.size() + removedNeighbors.size();
    neighbors.insert(nodes.begin(), nodes.end());
    uint32_t finalNeighborCount = neighbors.size();
    //cout << "space " << space << endl;
    if (neighbors.size() <= space) {
        //cout << "case 1\n";
        while (neighbors.size() < space) {
            uint32_t removed = removedNeighbors.back();
            removedNeighbors.pop_back();
            neighbors.insert(removed);
        }
        copy(neighbors.begin(), neighbors.end(), edgeBuffer->begin() + nodeIndex[pos].offset);
    } else {
        //cout << "case 2\n";
        auto it = neighbors.begin();
        uint32_t insertedElements = 0;
        uint32_t nextNodeOffset = (pos == nodeIndex.size()-1 ? edgeBuffer->size() : nodeIndex[pos+1].offset);
        for (uint32_t offset = nodeIndex[pos].offset ; offset < nextNodeOffset ; offset++) {
            (*edgeBuffer)[offset] = *it;
            it++;
            insertedElements++;
        }
        uint32_t addition = (neighbors.size() - insertedElements);
        //cout << "neighbors " << neighbors.size() << ", insertedElements " << insertedElements << endl;
        edgeBuffer->reserve(edgeBuffer->size() + addition);
        edgeBuffer->insert(edgeBuffer->begin() + nextNodeOffset, it, neighbors.end());
        for (uint32_t i = pos + 1 ; i < nodeIndex.size() ; i++) {
            nodeIndex[i].offset += addition;
        }
    }
    nodeIndex[pos].edges = finalNeighborCount;
    //print(true);
}

void Graph::collectZeroDegreeNodes() {
    for (uint32_t pos = 0 ; pos < nodeIndex.size() ; pos++) {
        if (!nodeIndex[pos].removed && !nodeIndex[pos].edges) {
            zeroDegreeNodes.push_back((!mapping ? pos : posToId->at(pos)));
            nodeIndex[pos].removed = true;
        }
    }
}



/* Only for debugging. If needed in actual algorithm,
 * make it a class field */
uint32_t Graph::getNodeCount() const {
    uint32_t count = 0;
    for (auto &node: nodeIndex) {
        if (!node.removed) {
            count++;
        }
    }
    return count;
}

void Graph::print(bool direction) const {
    cout << "\nNodes: " << nodeIndex.size() << " Edges: " << edgeBuffer->size() / 2 << "\n";
    for (uint32_t pos = 0 ; pos < nodeIndex.size() ; pos++) {
        if (!nodeIndex[pos].removed) {
            uint32_t node = (!mapping ? pos : (*posToId)[pos]);
            if (!nodeIndex[pos].edges) {
                cout << node << "\n";
            }
            uint32_t nextNodeOffset = (pos == nodeIndex.size()-1 ? edgeBuffer->size() : nodeIndex[pos+1].offset);
            for (uint32_t offset = nodeIndex[pos].offset ; offset < nextNodeOffset ; offset++) {
                uint32_t nPos = (!mapping ? (*edgeBuffer)[offset] : idToPos->at((*edgeBuffer)[offset]));
                if (!nodeIndex[nPos].removed && (direction || !direction && node < (*edgeBuffer)[offset])) {
                    cout << node << "\t" << (*edgeBuffer)[offset] << "\n";
                }
            }
        }
    }
    /*cout << "Zero degree nodes: \n";
    for (uint32_t i = 0 ; i < zeroDegreeNodes.size() ; i++) {
        cout << zeroDegreeNodes[i] << "\n";
    }*/
}

void Graph::printWithGraphTraversal(bool direction) const {
    GraphTraversal graphTraversal(*this);
    while (graphTraversal.curNode != NONE) {
        while (graphTraversal.curEdgeOffset != NONE) {
            if (direction || !direction && graphTraversal.curNode < (*edgeBuffer)[graphTraversal.curEdgeOffset]) {
                cout << graphTraversal.curNode << "\t" << (*edgeBuffer)[graphTraversal.curEdgeOffset] << "\n";
            }
            getNextEdge(graphTraversal);
        }
        getNextNode(graphTraversal);
    }
    cout << "Zero degree nodes: \n";
    for (uint32_t i = 0 ; i < zeroDegreeNodes.size() ; i++) {
        cout << zeroDegreeNodes[i] << "\n";
    }
}

void Graph::printEdgeCounts() const {
    cout << "Nodes: " << nodeIndex.size() << " Edges: " << edgeBuffer->size() / 2 << "\n";
    for (uint32_t pos = 0 ; pos < nodeIndex.size() ; pos++) {
        uint32_t node = (!mapping ? pos : (*posToId)[pos]);
        cout << "Node " << node << " has " << nodeIndex[pos].edges << " edges\n";
    }
}

/* Build graph from file, include both edge directions, keep them sorted.
 * Zero degree nodes are included in nodeIndex, but are makred as removed.
 * They are also held on zeroDegreeNodes seperate structure */
Graph::Graph(const string &inputFile, const bool &checkIndependentSet) : mapping(false), idToPos(NULL), posToId(NULL) {
    /* Open graph input file */
    FILE *f;
    f = fopen(inputFile.c_str(), "r");
    if (f == NULL) {
        cerr << "Error in opening input file " << inputFile << endl;
        exit(EXIT_FAILURE);
    }
    char buf[MAXLINE];

    /* Skip info lines */
    for (int i = 0 ; i < 3 ; i++) {
        if (fgets(buf, MAXLINE, f) == NULL) {
            cerr << "Error in parsing input file " << inputFile << endl;
            exit(EXIT_FAILURE);
        }
    }

    /* Get total node and edge numbers */
    char *ptr = strstr(buf, "Nodes:");
    uint32_t nodes = atoi(ptr + 7);
    ptr = strstr(buf, "Edges:");
    uint32_t edges = atoi(ptr + 7);
    if (fgets(buf, MAXLINE, f) == NULL) {
        cerr << "Error in parsing input file " << inputFile << endl;
        exit(EXIT_FAILURE);
    }

    try {
        nodeIndex.reserve(nodes);
    }
    catch (const length_error &le) {
        cerr << "NodeIndex length error: " << le.what() << endl;
        exit(EXIT_FAILURE);
    }

    edgeBuffer = new vector<uint32_t>();
    try {
        edgeBuffer->reserve(edges * 2);
    }
    catch (const length_error &le) {
        cerr << "EdgeBuffer length error: " << le.what() << endl;
        exit(EXIT_FAILURE);
    }

    /* Hold reverse direction edges temporarily */
    vector<vector<uint32_t> > reverseEdges(nodes);

    /* Build graph with both edge directions, keep them sorted */
    uint32_t sourceNode, targetNode, previousNode, straightEdges, offset;
    previousNode = NONE;
    straightEdges = 0;
    offset = 0;
    while(fgets(buf, MAXLINE, f) != NULL) {
        parseNodeIDs(buf, &sourceNode, &targetNode);
        if (sourceNode >= nodes || targetNode >= nodes) {
            cerr << "Error: received a node id equal or larger than the total nodes number specified at the beginning of the file" << endl;
            exit(EXIT_FAILURE);
        }
        /* Add smaller nodes than first source node */
        if (previousNode == NONE && sourceNode) {
            this->fill(sourceNode, checkIndependentSet);
        }
        if (previousNode != sourceNode && previousNode != NONE) {
            /* At new source node, add previous nodes' (including those who don't appear as source nodes) smaller neighbors */
            for (uint32_t node = previousNode + 1 ; node <= sourceNode ; node++) {
                uint32_t previousEdges = straightEdges + reverseEdges[node - 1].size();
                nodeIndex.push_back(Graph::NodeInfo(offset, previousEdges));
                if (!previousEdges && !checkIndependentSet) {
                    zeroDegreeNodes.push_back(nodeIndex.size()-1);
                    nodeIndex[nodeIndex.size()-1].removed = true;
                }
                offset += previousEdges;
                edgeBuffer->insert(edgeBuffer->end(), reverseEdges[node].begin(), reverseEdges[node].end());
                straightEdges = 0;
                if (node < sourceNode) {
                    //cout << "in between missing node " << node << endl;
                }
            }
        }
        /* Add source node's bigger neighbors */
        edgeBuffer->push_back(targetNode);
        straightEdges++;
        reverseEdges[targetNode].push_back(sourceNode);
        previousNode = sourceNode;
    }
    /* Add final source node's info */
    uint32_t previousEdges = straightEdges + reverseEdges[previousNode].size();
    nodeIndex.push_back(Graph::NodeInfo(offset, previousEdges));
    if (!previousEdges && !checkIndependentSet) {
        zeroDegreeNodes.push_back(nodeIndex.size()-1);
        nodeIndex[nodeIndex.size()-1].removed = true;
    }
    offset += previousEdges;
    /* Add smaller neighbors of the nodes left that don't appear as source nodes */
    for (uint32_t missingNode = nodeIndex.size() ; missingNode < nodes ; missingNode++) {
        //cout << "end missing node " << missingNode << endl;
        previousEdges = reverseEdges[missingNode].size();
        nodeIndex.push_back(Graph::NodeInfo(offset, previousEdges));
        if (!previousEdges && !checkIndependentSet) {
            zeroDegreeNodes.push_back(nodeIndex.size()-1);
            nodeIndex[nodeIndex.size()-1].removed = true;
        }
        offset += previousEdges;
        edgeBuffer->insert(edgeBuffer->end(), reverseEdges[missingNode].begin(), reverseEdges[missingNode].end());
    }
    nextUnusedId = nodeIndex.size();
    fclose(f);
}

void Graph::fill(const uint32_t &size, const bool &checkIndependentSet) {
    //cout << "start missing nodes from " << nodeIndex.size() << " to " << size << endl;
    while (nodeIndex.size() < size) {
        nodeIndex.push_back(NodeInfo(edgeBuffer->size(), 0));
        if (!checkIndependentSet) {
            zeroDegreeNodes.push_back(nodeIndex.size()-1);
            nodeIndex[nodeIndex.size()-1].removed = true;
        }
    }
}

void Graph::parseNodeIDs(char *buf, uint32_t *sourceNode, uint32_t *targetNode) {
    uint32_t *node = sourceNode;
    for (int i = 0 ; i < 2 ; i++) {
        uint32_t id = 0;
        for (; *buf != ' ' && *buf != '\t' && *buf != '\n' ; buf++) {
            id = id * 10 + (*buf - '0');
        }
        *node = id;
        node = targetNode;
        for (; *buf == ' ' || *buf == '\t' ; buf++);
    }
}
