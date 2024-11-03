#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <set>

class Graph {
public:
    Graph(int vertices);
    void addEdge(int u, int v);
    std::vector<int> greedyDominatingSet();

private:
    int vertices;                       // Number of vertices in the graph
    std::vector<std::vector<int>> adj;  // Adjacency list representation
};

#endif // GRAPH_H
