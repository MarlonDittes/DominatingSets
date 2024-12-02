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

class Graph {
public:
    Graph(int vertices);
    void addEdge(int u, int v);
    std::vector<int> greedyDominatingSet();
    double computeEfficiencyLowerBound();
    void graphToHypergraph(const std::string& outputFile) const;

    std::pair<std::vector<std::vector<std::vector<int>>>, std::vector<std::vector<int>>> getConnectedComponents() const;

    void writeHittingSetILP(const std::string &outputFile) const;

private:
    int vertices;                       // Number of vertices in the graph
    std::vector<std::vector<int>> adj;  // Adjacency list representation

    void dfs(int node, std::vector<bool>& visited, std::vector<int>& component) const;
};

#endif // GRAPH_H
