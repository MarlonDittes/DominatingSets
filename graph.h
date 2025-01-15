#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <set>
#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>  // For std::max
#include <unordered_map>
#include <set>
#include <cmath>
#include <numeric>
#include <cassert>

struct Node {
    std::vector<int> edges;
    int offset = 0; //offset to visible nodes in neighborhood
    int active = true;
};

class Graph {
private:
    int vertices;
    int edges = 0;
    std::vector<Node> adj;  // Adjacency list representation

    void dfs(int node, std::vector<bool>& visited, std::vector<int>& component) const;
public:
    Graph(int vertices);
    void addEdge(int u, int v);
    void makeNodeInvisible(int u);
    void makeNodeVisible(int u);
    void printGraph() const;

    std::vector<int> greedyDominatingSet();
    double computeEfficiencyLowerBound();
    double computeDensity() const;
    int getMaxDegree() const;

    int getVertices(){return vertices;};
    int getEdges(){return edges;};
    int countTriangles() const;
    std::vector<int> getVertexDegrees() const;
    std::pair<double, double> computeDegreeStats() const;

    void graphToHypergraph(const std::string& outputFile) const;
    void graphToSAT(const std::string& outputFile) const;

    void writeHittingSetILP(const std::string &outputFile) const;
    void writeHittingSetLP(const std::string &outputFile) const;
    void writeHittingSetILP_check(const std::string &outputFile, int k) const;

    std::pair<std::vector<std::vector<std::vector<int>>>, std::vector<std::vector<int>>> getConnectedComponents() const;


};

#endif // GRAPH_H
