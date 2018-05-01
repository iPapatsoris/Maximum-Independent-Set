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

/* Mark selected nodes as removed and reduce their neighbors' neighbor count */
void Graph::remove(const std::vector<uint32_t> &nodes, ReduceInfo &reduceInfo, const bool &sameComponent, unordered_set<uint32_t> *candidateNodes) {
    for (auto it = nodes.begin() ; it != nodes.end() ; it++) {
        cout << "removing " << *it << endl;
        uint32_t pos = (!mapping ? *it : idToPos->at(*it));
        if (!nodeIndex[pos].removed) {
            reduceInfo.nodesRemoved++;
            if (!sameComponent) {
                uint32_t nextNodeOffset = (pos == nodeIndex.size()-1 ? edgeBuffer->size() : nodeIndex[pos+1].offset);
                for (uint32_t offset = nodeIndex[pos].offset ; offset < nextNodeOffset ; offset++) {
                    uint32_t neighbor = (*edgeBuffer)[offset];
                    uint32_t nPos = (!mapping ? neighbor : idToPos->at(neighbor));
                    if (!nodeIndex[nPos].removed) {
                        nodeIndex[nPos].edges--;
                        reduceInfo.edgesRemoved++;
                        if (find(it+1, nodes.end(), neighbor) == nodes.end()) {
                            if(nodeIndex[nPos].edges == 0) {
                                zeroDegreeNodes.push_back(neighbor);
                                nodeIndex[nPos].removed = true;
                                reduceInfo.nodesRemoved++;
                            }
                            if (candidateNodes != NULL && (nodeIndex[nPos].edges == 2 || nodeIndex[nPos].edges == 3) && nPos < pos) {
                                candidateNodes->insert(neighbor);
                            }
                        }
                    }
                }
            }
            nodeIndex[pos].edges = 0;
            nodeIndex[pos].removed = true;
        }
    }
}

void Graph::remove(const uint32_t &node, ReduceInfo &reduceInfo) {
    remove(std::vector<uint32_t>(1, node), reduceInfo);
}

/* Rebuild structures, completely removing nodes that are marked as removed */
void Graph::rebuild(const ReduceInfo &reduceInfo) {
    if (!reduceInfo.nodesRemoved) {
        return;
    }
    cout << "\nRebuilding: nodes removed " << reduceInfo.nodesRemoved << ", edges removed " << reduceInfo.edgesRemoved << endl;
    vector<NodeInfo> nodeIndex;
    nodeIndex.reserve(this->nodeIndex.size() - reduceInfo.nodesRemoved);
    vector<uint32_t> *edgeBuffer = new vector<uint32_t>();
    edgeBuffer->reserve(this->edgeBuffer->size() - reduceInfo.edgesRemoved);
    unordered_map<uint32_t, uint32_t> *idToPos = new unordered_map<uint32_t, uint32_t>();
    vector<uint32_t> *posToId = new vector<uint32_t>();
    posToId->reserve(this->nodeIndex.size() - reduceInfo.nodesRemoved);
    uint32_t offset = 0;

    for (uint32_t pos = 0 ; pos < this->nodeIndex.size() ; pos++) {
        if (this->nodeIndex[pos].removed) {
            continue;
        }
        uint32_t node = (!mapping ? pos : (*this->posToId)[pos]);
        if (!this->nodeIndex[pos].edges) {
            cout << "Found node " << node << " with no edges at rebuilding\n";
            assert(false);
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
                    reduceInfo.edgesRemoved--;
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
    cout << "hypernode " << newNode << endl;
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

/* Gather node's neighbors in a vector. Useful when there are removed neighbors
 * in between non-removed ones, and access is frequent */
void Graph::gatherNeighbors(const uint32_t &node, vector<uint32_t> &neighbors) const {
    uint32_t pos = (!mapping ? node : idToPos->at(node));
    uint32_t neighborCount = nodeIndex[pos].edges;
    uint32_t nextNodeOffset = (pos == nodeIndex.size()-1 ? edgeBuffer->size() : nodeIndex[pos+1].offset);
    for (uint32_t offset = nodeIndex[pos].offset ; offset  < nextNodeOffset && neighborCount; offset++) {
        uint32_t nPos = (!mapping ? (*edgeBuffer)[offset] : idToPos->at((*edgeBuffer)[offset]));
        if (!nodeIndex[nPos].removed) {
            neighbors.push_back((*edgeBuffer)[offset]);
            neighborCount--;
        }
    }
}

uint32_t Graph::getNextNodeWithIdenticalNeighbors(const uint32_t &previousNode, const vector<uint32_t> &neighbors) const {
    uint32_t pos = (!mapping ? previousNode : idToPos->at(previousNode));
    for (pos = pos+1 ; pos < nodeIndex.size() ; pos++) {
        if (nodeIndex[pos].edges == neighbors.size()) {
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
            uint32_t nextNodeOffset = (pos == nodeIndex.size()-1 ? edgeBuffer->size() : nodeIndex[pos+1].offset);
            for (uint32_t offset = nodeIndex[pos].offset ; offset < nextNodeOffset ; offset++) {
                uint32_t node = (!mapping ? pos : (*posToId)[pos]);
                if (mapping && idToPos->find((*edgeBuffer)[offset]) == idToPos->end()) { // For printing subgraphs generated by buildNDegreeSubgraph
                    cout << node << "\t" << (*edgeBuffer)[offset] << "\n";
                    continue;
                }
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
