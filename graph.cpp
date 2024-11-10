#include "graph.h"
#include <algorithm>  // For std::max
#include <unordered_map>

Graph::Graph(int vertices) : vertices(vertices), adj(vertices) {}

void Graph::addEdge(int u, int v) {
    adj[u-1].push_back(v-1); // Assuming 1-based index in the .gr file, converting to 0-based
    adj[v-1].push_back(u-1);  // Undirected graph, so add edge in both directions
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

void Graph::graphToHypergraph(const std::string& outputFile) const{
    std::ofstream file(outputFile);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open the output file!");
    }

    // Writing the hypergraph in custom text format described in README.md of https://github.com/Felerius/findminhs
    file << vertices << " " << vertices << "\n"; // num_vertices num_hyperedges

    for (int u = 0; u < vertices; ++u) {
        // Insert closed neighborhoods of each vertex as hyperedge
        file << adj[u].size() + 1 << " "; // size of the closed neighborhood
        for (int v : adj[u]) {
            file << v << " "; // print each node in the neighborhood
        }
        file << u << "\n"; // Include the vertex itself
    }

    file.close();
}