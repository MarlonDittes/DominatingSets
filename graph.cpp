#include "graph.h"
#include <algorithm>  // For std::max
#include <unordered_map>

Graph::Graph(int vertices) : vertices(vertices), adj(vertices) {}

void Graph::addEdge(int u, int v) {
    adj[u].push_back(v);
    adj[v].push_back(u);  // Undirected graph, so add edge in both directions
}

std::vector<int> Graph::greedyDominatingSet() {
    std::vector<int> dominatingSet;
    std::vector<bool> covered(vertices, false);  // To check if a vertex is covered
    std::set<int> uncovered;  // Set of all uncovered vertices

    // Initialize the set of uncovered vertices
    for (int i = 0; i < vertices; ++i) {
        uncovered.insert(i);
    }

    // Greedy selection of dominating set
    while (!uncovered.empty()) {
        // Find the vertex with the maximum number of uncovered neighbors
        int maxCoverage = -1;
        int bestVertex = -1;

        for (int u : uncovered) {
            int coverage = 0;
            for (int v : adj[u]) {
                if (!covered[v]) ++coverage;
            }
            if (coverage > maxCoverage) {
                maxCoverage = coverage;
                bestVertex = u;
            }
        }

        // Add bestVertex to the dominating set
        dominatingSet.push_back(bestVertex);
        covered[bestVertex] = true;
        uncovered.erase(bestVertex);

        // Mark all neighbors of bestVertex as covered
        for (int neighbor : adj[bestVertex]) {
            if (!covered[neighbor]) {
                covered[neighbor] = true;
                uncovered.erase(neighbor);
            }
        }
    }

    return dominatingSet;
}
