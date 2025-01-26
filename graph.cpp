#include "graph.h"

Graph::Graph(int vertices) : vertices(vertices), adj(vertices) {}

void Graph::addEdge(int u, int v) {
    adj[u-1].edges.push_back(v-1); // Assuming 1-based index in the .gr file, converting to 0-based
    adj[v-1].edges.push_back(u-1);  // Undirected graph, so add edge in both directions
    edges += 1;
}

void Graph::makeNodeInvisible(int u){
    assert(adj[u].active);

    Node* node = &adj[u];

    for (int i = 0; i < node->edges.size(); i++){
        int v = node->edges[i];
        Node* neighbor = &adj[v];

        //make node invisible in all adjacency lists of neighbours
        assert(neighbor->edges.size() > neighbor->offset);
        for (int j = neighbor->offset; j < neighbor->edges.size(); j++) {
            if (neighbor->edges[j] != u) {
                continue;
            }
            //add node to invisible nodes and add visible counter
            std::swap(neighbor->edges[j], neighbor->edges[neighbor->offset]);
            neighbor->offset++;
        }
    }

    node->active = false;
}

void Graph::makeNodeVisible(int u) {
    assert(!adj[u].active);

    Node* node = &adj[u];
    node->offset = 0;
    node->active = true;

    for (int i = 0; i < node->edges.size(); i++) {
        Node* neighbor = &adj[node->edges[i]];

        // Handle inactive neighbors
        if (!neighbor->active){
            std::swap(node->edges[i], node->edges[node->offset]);
            node->offset++;
        }

        //make node visible in all adjacency lists of neighbours
        assert(neighbor->offset > 0);
        for (int j = 0; j < neighbor->offset; j++) {
            if (neighbor->edges[j] != u) {
                continue;
            }
            //add node to visible nodes and reduce visible counter
            std::swap(neighbor->edges[j], neighbor->edges[neighbor->offset - 1]);
            neighbor->offset--;
        }
    }
}

void Graph::printGraph() const {
    std::cout << "Total Vertices: " << vertices << std::endl;
    std::cout << "Total Edges: " << edges << std::endl;

    for (int i = 0; i < vertices; ++i) {
        // Mark nodes that are globally removed
        if (!adj[i].active) {
            std::cout << "// Node " << i+1 << " (Offset: " << adj[i].offset << "): ";
        } else {
            std::cout << "Node " << i+1 << " (Offset: " << adj[i].offset << "): ";
        }

        // Print the neighbors with a visual offset marker '|'
        for (int j = 0; j < adj[i].edges.size(); ++j) {
            if (j == adj[i].offset) std::cout << "| ";  // Mark the offset position
            std::cout << adj[i].edges[j]+1 << " ";
        }

        // If the offset equals the size of the neighborhood, place the '|'
        if (adj[i].offset == adj[i].edges.size()) {
            std::cout << "|";
        }

        std::cout << std::endl;
    }
}

int Graph::reductionIsolatedVertex(std::vector<int>& dominatingSet) {
    int occurence = 0;
    for (int i = 0; i < vertices; i++) {
        if (adj[i].active && (adj[i].edges.size() == adj[i].offset)) {
            dominatingSet.push_back(i);
            makeNodeInvisible(i);
            occurence++;
        }
    }
    return occurence;
}

int Graph::reductionDominatingVertex(std::vector<int>& dominatingSet) {
    //TODO: fix order of looking at
    //TODO: DO WE EVEN NEED ORDERING?
    int occurence = 0;

    // Create a vector of pairs (degree, vertex_index)
    std::vector<std::pair<int, int>> degreeOrder;
    for (int u = 0; u < vertices; u++) {
        if (adj[u].active) {
            degreeOrder.emplace_back(adj[u].edges.size(), u);
        }
    }

    std::sort(degreeOrder.rbegin(), degreeOrder.rend());

    for (const auto& [degree, u] : degreeOrder) {
    //for (int u = 0; u < vertices; u++) {
        if (!adj[u].active) continue;

        for (int i = adj[u].offset; i < adj[u].edges.size(); i++) {
            int v = adj[u].edges[i];
            // u can only dominate v if it has more active edges left
            if (adj[v].edges.size() > adj[u].edges.size()) continue;

            // Check if u dominates v's neighborhood
            bool dominates = true;
            for (int j = 0; j < adj[v].edges.size(); j++) {
                int neighbor = adj[v].edges[j];
                if (neighbor != u && std::find(adj[u].edges.begin(), adj[u].edges.end(), neighbor) == adj[u].edges.end()) {
                    dominates = false;
                    break;
                }
            }

            if (dominates) {
                //std::cout << u << " dominates " << v << std::endl;
                // Remove dominated node v
                makeNodeInvisible(v);
                occurence++;

                // Break to prevent processing invalidated nodes
                break;
            }
        }
    }
    return occurence;
}

int Graph::reductionSingleEdgeVertex(std::vector<int>& dominatingSet) {
    int occurence = 0;
    for (int i = 0; i < vertices; i++) {
        if (!adj[i].active || adj[i].edges.size() != adj[i].offset + 1) continue;

        int neighbor = adj[i].edges[adj[i].offset];
        if (adj[neighbor].active) {
            // Add the neighbor to the dominating set
            dominatingSet.push_back(neighbor);
            //std::cout << "Choose " << neighbor << " over " << i << std::endl;
            // Remove the neighbor and initial node and mark all neighbors as covered
            makeNodeInvisible(neighbor);
            makeNodeInvisible(i);
            for (int j = adj[neighbor].offset; j < adj[neighbor].edges.size(); j++) {
                adj[adj[neighbor].edges[j]].covered = true;
            }
            occurence++;
        }
    }
    return occurence;
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
            for (int v : adj[u].edges) {
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
        for (int neighbor : adj[bestVertex].edges) {
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
        int max_deg = adj[u].edges.size() + 1;
        for (int v : adj[u].edges) {
            int current_deg = adj[v].edges.size() + 1;  
            if (current_deg > max_deg){
                max_deg = current_deg;
            }   
        }
        lower_bound += 1.0 / max_deg; 
    }

    return lower_bound;
}

double Graph::computeDensity() const {
    // Since undirected, we have twice the amount of edges
    int num_edges = 2 * edges;

    // Compute the density: num_edges / (V * (V - 1))
    double max_edges = vertices * (vertices - 1);
    return num_edges / max_edges;
}

int Graph::getMaxDegree() const {
    int max_degree = 0;

    // Iterate over all vertices and find the maximum degree
    for (int u = 0; u < adj.size(); ++u) {
        max_degree = std::max(max_degree, static_cast<int>(adj[u].edges.size()));
    }

    return max_degree;
}

int Graph::countTriangles() const{
    int triangleCount = 0;

    // Iterate over all vertices
    for (int u = 0; u < vertices; ++u) {
        // Check neighbors of u
        for (int v : adj[u].edges) {
            if (v > u) { // Ensure u < v to avoid double-counting
                for (int w : adj[v].edges) {
                    if (w > v && std::find(adj[u].edges.begin(), adj[u].edges.end(), w) != adj[u].edges.end()) {
                        // Triangle found: u-v-w
                        ++triangleCount;
                    }
                }
            }
        }
    }

    return triangleCount;
}

std::vector<int> Graph::getVertexDegrees() const{
    std::vector<int> degrees(vertices);
    for (int i = 0; i < vertices; ++i) {
        degrees[i] = adj[i].edges.size(); // Degree is the size of adjacency list
    }
    return degrees;
}

std::pair<double, double> Graph::computeDegreeStats() const{
    auto degrees = getVertexDegrees();
    double sum = std::accumulate(degrees.begin(), degrees.end(), 0);
    double avgDegree = sum / vertices;

    double variance = 0.0;
    for (int degree : degrees) {
        variance += (degree - avgDegree) * (degree - avgDegree);
    }
    variance /= vertices;
    double stdDev = std::sqrt(variance);

    return {avgDegree, stdDev};
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
        file << adj[u].edges.size() + 1 << " "; // size of the closed neighborhood
        for (int v : adj[u].edges) {
            file << v << " "; // print each node in the neighborhood
        }
        file << u << "\n"; // Include the vertex itself
    }

    file.close();
}

void Graph::graphToSAT(const std::string& outputFile) const{
    std::ofstream file(outputFile);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open the output file!");
    }

    // Writing the hypergraph in custom text format described in README.md of https://github.com/Felerius/findminhs
    file << vertices << " " << vertices << "\n"; // num_vertices num_hyperedges

    for (int u = 0; u < vertices; ++u){
        file << 1 << " ";
    }
    file << "\n";

    for (int u = 0; u < vertices; ++u) {
        // Insert closed neighborhoods of each vertex as hyperedge
        file << adj[u].edges.size() + 1 << " "; // size of the closed neighborhood
        for (int v : adj[u].edges) {
            file << v + 1 << " "; // print each node in the neighborhood
        }
        file << u + 1<< "\n"; // Include the vertex itself
    }

    file.close();
}

void Graph::writeHittingSetILP(const std::string &outputFile) const {
    std::ofstream file(outputFile);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + outputFile);
    }

    // Write the objective function
    file << "Minimize\n obj: ";
    bool first = true;
    for (int i = 0; i < vertices; ++i) {
        if (!adj[i].active) continue; // Skip inactive vertices
        if (!first) {
            file << " + ";
        }
        file << "x" << i;
        first = false;
    }
    file << "\n\nSubject To\n";

    // Write the constraints (one per closed neighborhood)
    for (int u = 0; u < vertices; ++u) {
        if (!adj[u].active) continue; // Skip inactive vertices or vertices that are already covered
        file << " c" << u + 1 << ": ";
        std::set<int> neighborhood;
        neighborhood.insert(u); // Include the vertex itself
        for (int j = adj[u].offset; j < adj[u].edges.size(); j++){
            int v = adj[u].edges[j];
            neighborhood.insert(v);
        }
        int count = 0;
        for (int v : neighborhood) {
            if (count > 0) {
                file << " + ";
            }
            file << "x" << v;
            count++;
        }
        file << " >= 1\n";
    }

    // Write bounds and variable types
    file << "\nBounds\n";
    for (int i = 0; i < vertices; ++i) {
        if (!adj[i].active) continue; // Skip inactive vertices
        file << " 0 <= x" << i << " <= 1\n";
    }

    file << "\nBinary\n";
    for (int i = 0; i < vertices; ++i) {
        if (!adj[i].active) continue; // Skip inactive vertices
        file << " x" << i << "\n";
    }

    file << "End\n";
    file.close();
}

void Graph::writeHittingSetLP(const std::string &outputFile) const {
    std::ofstream file(outputFile);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + outputFile);
    }

    // Write the objective function
    file << "Minimize\n obj: ";
    bool first = true;
    for (int i = 0; i < vertices; ++i) {
        if (!adj[i].active) continue; // Skip inactive vertices
        if (!first) {
            file << " + ";
        }
        file << "x" << i;
        first = false;
    }
    file << "\n\nSubject To\n";

    // Write the constraints (one per closed neighborhood)
    for (int u = 0; u < vertices; ++u) {
        if (!adj[u].active || adj[u].covered) continue; // Skip inactive vertices or vertices that are already covered
        file << " c" << u + 1 << ": ";
        std::set<int> neighborhood;
        neighborhood.insert(u); // Include the vertex itself
        for (int j = adj[u].offset; j < adj[u].edges.size(); j++){
            int v = adj[u].edges[j];
            neighborhood.insert(v);
        }
        int count = 0;
        for (int v : neighborhood) {
            if (count > 0) {
                file << " + ";
            }
            file << "x" << v;
            count++;
        }
        file << " >= 1\n";
    }

    // Write bounds and variable types
    file << "\nBounds\n";
    for (int i = 0; i < vertices; ++i) {
        if (!adj[i].active) continue; // Skip inactive vertices
        file << " 0 <= x" << i << " <= 1\n";
    }

    file << "End\n";
    file.close();
}

void Graph::writeHittingSetILP_check(const std::string &outputFile, int k) const {
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
        for (int v : adj[u].edges) {
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

    // Add the constraint to limit the number of selected elements to k
    file << " c_total: ";
    for (int i = 0; i < vertices; ++i) {
        file << "x" << i;
        if (i < vertices - 1) {
            file << " + ";
        }
    }
    file << " <= " << k << "\n";

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

// DFS helper function to traverse and collect nodes in a connected component
void Graph::dfs(int node, std::vector<bool>& visited, std::vector<int>& component) const {
    visited[node] = true;
    component.push_back(node);

    for (int neighbor : adj[node].edges) {
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
                for (int neighbor : adj[node].edges) {
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