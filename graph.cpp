#include "graph.h"

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

double Graph::computeEfficiencyLowerBound(){
    double lower_bound = 0.0;

    // Iterate over each edge in the graph
    for (int u = 0; u < vertices; ++u) {
        for (int v : adj[u]) {
            if (u < v) {  // To avoid counting the edge twice (u, v) and (v, u)
                int deg_u = adj[u].size();  
                int deg_v = adj[v].size(); 
                double max_deg = std::max(deg_u, deg_v);

                lower_bound += 1.0 / max_deg;
            }
        }
    }

    return lower_bound;
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

// DFS helper function to traverse and collect nodes in a connected component
void Graph::dfs(int node, std::vector<bool>& visited, std::vector<int>& component) const {
    visited[node] = true;
    component.push_back(node);

    for (int neighbor : adj[node]) {
        if (!visited[neighbor]) {
            dfs(neighbor, visited, component);
        }
    }
}

// Function to extract connected components and return them as separate subgraphs
std::pair<std::vector<std::vector<std::vector<int>>>, std::vector<std::vector<int>>> Graph::getConnectedComponents() const {
    std::vector<bool> visited(vertices, false);
    std::vector<std::vector<std::vector<int>>> connectedComponents;
    std::vector<std::vector<int>> reverseMappings;

    for (int i = 0; i < vertices; ++i) {
        if (!visited[i]) {
            std::vector<int> componentNodes;  // Nodes in the current connected component
            dfs(i, visited, componentNodes);

            // Create a subgraph for the current connected component
            std::vector<std::vector<int>> subgraph(componentNodes.size());
            std::map<int, int> oldToNew;  // Map old indices to new indices in the subgraph
            std::vector<int> newToOld(componentNodes.size()); // Reverse map: subgraph -> old graph
            //TODO: Change map here?

            for (int j = 0; j < componentNodes.size(); ++j) {
                oldToNew[componentNodes[j]] = j;  // Assign new indices
                newToOld[j] = componentNodes[j];
            }

            // Populate adjacency list for the subgraph
            for (int node : componentNodes) {
                std::vector<int> neighborsInSubgraph;
                for (int neighbor : adj[node]) {
                    if (oldToNew.find(neighbor) != oldToNew.end()) {
                        neighborsInSubgraph.push_back(oldToNew[neighbor]);
                    }
                }
                subgraph[oldToNew[node]] = neighborsInSubgraph;
            }

            connectedComponents.push_back(subgraph);
            reverseMappings.push_back(newToOld);
        }
    }

    return std::make_pair(connectedComponents, reverseMappings);
}

void Graph::writeHittingSetILP(const std::string &outputFile) const {
    std::ofstream file(outputFile);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + outputFile);
    }

    // Write the objective function
    file << "Minimize\n obj: ";
    for (int i = 0; i < vertices; ++i) {
        file << "x" << i;
        if (i < vertices - 1) {
            file << " + ";
        }
    }
    file << "\n\nSubject To\n";

    // Write the constraints (one per closed neighborhood)
    for (int u = 0; u < vertices; ++u) {
        file << " c" << u + 1 << ": ";
        std::set<int> neighborhood;
        neighborhood.insert(u); // Include the vertex itself
        for (int v : adj[u]) {
            neighborhood.insert(v); // Include its neighbors
        }
        int count = 0;
        for (int v : neighborhood) {
            file << "x" << v;
            if (++count < neighborhood.size()) {
                file << " + ";
            }
        }
        file << " >= 1\n";
    }

    // Write bounds and variable types
    file << "\nBounds\n";
    for (int i = 0; i < vertices; ++i) {
        file << " 0 <= x" << i << " <= 1\n";
    }

    file << "\nBinary\n";
    for (int i = 0; i < vertices; ++i) {
        file << " x" << i << "\n";
    }

    file << "End\n";
    file.close();
}