#include "hypergraph.h"

Hypergraph::Hypergraph(int edge_count) : hyperedges(edge_count), active(edge_count, true), covered(edge_count, false) {}

void Hypergraph::addEdge(int u, int v){
    hyperedges[u-1].push_back(v-1);
    hyperedges[v-1].push_back(u-1);
}

void Hypergraph::printHypergraph(){
    std::cout << "Hypergraph:" << std::endl;
    for (size_t i = 0; i < hyperedges.size(); ++i) {
        if (!active[i]) std::cout << "I ";
        if (covered[i]) std::cout << "C ";

        std::cout << "Edge " << i+1 << ": ";
        for (int v : hyperedges[i]) {
            std::cout << v+1 << " ";
        }
        std::cout << std::endl;
    }
}



int Hypergraph::reductionIsolatedVertex(std::vector<int>& dominatingSet, bool verbose){
    int reductionCount = 0;
    for (size_t i = 0; i < hyperedges.size(); ++i) {
        if (hyperedges[i].size() == 0) {
            dominatingSet.push_back(i);
            active[i] = false;
            covered[i] = true;
            reductionCount++;

            if (verbose) std::cout << "Edge "  << i+1 << " was isolated." << std::endl;
        }
    }
    return reductionCount;
}

int Hypergraph::reductionSingleEdgeVertex(std::vector<int>& dominatingSet, bool verbose){
    int reductionCount = 0;
    for (size_t i = 0; i < hyperedges.size(); ++i) {
        if (!active[i]) continue; // Disregard inactive hyperedges

        if (hyperedges[i].size() == 1) {
            int neighbor = hyperedges[i][0];
            if (!active[neighbor]) continue; // Case handled by IsolatedVertex reduction

            dominatingSet.push_back(neighbor);
            active[i] = false;
            covered[i] = true;
            active[neighbor] = false;
            covered[neighbor] = true;
            for (auto v : hyperedges[neighbor]){ // All adjacent vertices' constraints are now inactive
                active[v] = false;
            }
            reductionCount++;

            if (verbose) std::cout << "Edge "  << neighbor+1 << " chosen over " << i+1 << std::endl;
        }
    }
    return reductionCount;
}

int Hypergraph::reductionDominatingEdge(std::vector<int>& dominatingSet, bool verbose){
    int reductionCount = 0;
    for (size_t i = 0; i < hyperedges.size(); ++i) {
        if (!active[i]) continue; // Disregard inactive hyperedges

        std::set<int> edgeSet(hyperedges[i].begin(), hyperedges[i].end());
        edgeSet.insert(i); // Include its own ID

        for (size_t j = 0; j < hyperedges.size(); ++j) {
            if (i == j || !active[j]) continue; // Skip itself and inactive edges

            if (hyperedges[j].size() > hyperedges[i].size()) continue; // If other edge contains more vertices, initial edge can't dominate

            std::set<int> otherEdgeSet(hyperedges[j].begin(), hyperedges[j].end());
            otherEdgeSet.insert(j); // Include its own ID

            // If edge i dominates edge j, deactivate edge j
            if (std::includes(edgeSet.begin(), edgeSet.end(), otherEdgeSet.begin(), otherEdgeSet.end())) {
                //active[j] = false;
                covered[j] = true;
                reductionCount++;

                if (verbose) std::cout << "Edge " << i + 1 << " dominates " << j + 1 << std::endl;
            }
        }
    }
    return reductionCount;
}

//int Hypergraph::reductionDominatingVertex(std::vector<int>& dominatingSet, bool verbose);

int Hypergraph::reductionCountingRule(std::vector<int>& dominatingSet, bool verbose){
    int reductionCount = 0;

    // Frequency count of each element
    std::unordered_map<int, int> elementFrequency;
    for (size_t i = 0; i < hyperedges.size(); ++i) {
        auto& edge = hyperedges[i];
        elementFrequency[i]++; //hyperedge contains its own vertex
        for (int e : edge) {
            elementFrequency[e]++;
        }
    }

    // Iterate over each hyperedge (potential set R)
    for (size_t i = 0; i < hyperedges.size(); ++i) {
        if (!active[i]) continue; // Skip inactive edges

        std::set<int> R(hyperedges[i].begin(), hyperedges[i].end());
        R.insert(i); // Each hyperedge includes its own ID

        // Compute r2: elements in R that appear exactly twice in the hypergraph
        std::vector<int> freqTwoElements;
        for (int e : R) {
            if (elementFrequency[e] == 2) {
                freqTwoElements.push_back(e);
            }
        }
        int r2 = freqTwoElements.size();
        if (r2 == 0) continue; // No frequency-two elements, skip

        // Compute q: elements in sets containing a frequency-two element from R but not in R
        std::set<int> externalElements;
        for (size_t j = 0; j < hyperedges.size(); ++j) {
            if (i == j || !active[j]) continue;

            // Construct the set for hyperedge j, explicitly adding j
            std::set<int> hyperedgeJ(hyperedges[j].begin(), hyperedges[j].end());
            hyperedgeJ.insert(j); // Include j in its own set representation

            for (int e : hyperedgeJ) {
                if (std::find(freqTwoElements.begin(), freqTwoElements.end(), e) != freqTwoElements.end() && R.find(e) == R.end()) {
                    externalElements.insert(e);
                }
            }
        }
        int q = externalElements.size();

        // Apply Reduction Rule 6 if q < r2
        if (q < r2) {
            dominatingSet.push_back(i);
            
            active[i] = false;
            covered[i] = true;

            for (auto& j : hyperedges[i]){
                active[j] = false;
            }

            reductionCount++;

            if (verbose) {
                std::cout << "Reduction Rule 6: Removed hyperedge " << i + 1 << " (r2 = " << r2 << ", q = " << q << ")" << std::endl;
            }
        }
    }

    return reductionCount;
}


void Hypergraph::writeHittingSetLP(const std::string &outputFile, bool ILP) const{
    std::ofstream file(outputFile);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + outputFile);
    }

    // Write the objective function
    file << "Minimize\n obj: ";
    bool first = true;
    for (size_t i = 0; i < hyperedges.size(); ++i) {
        if (covered[i]) continue; // Skip vertices which are already satisfied

        if (!first) {
            file << " + ";
        }
        file << "x" << i + 1;
        first = false;
    }
    file << "\n\nSubject To\n";

    // Write the constraints (one per closed neighborhood)
    for (size_t i = 0; i < hyperedges.size(); ++i) {
        if (!active[i]) continue; // Skip inactive edges

        file << " c" << i + 1 << ": ";
        std::set<int> neighborhood;

        std::set<int> coveredVertices(hyperedges[i].begin(), hyperedges[i].end());
        coveredVertices.insert(i); // Each hyperedge includes its own ID

        int count = 0;
        for (int v : coveredVertices) {
            if (covered[v]) continue; // Skip vertices which are already satisfied

            if (count > 0) {
                file << " + ";
            }
            file << "x" << v + 1;
            count++;
        }
        if (count > 0) file << " >= 1\n";
    }

    // Write bounds and variable types
    file << "\nBounds\n";
    for (size_t i = 0; i < hyperedges.size(); ++i) {
        if (covered[i]) continue; // Skip vertices which are already satisfied
        file << " 0 <= x" << i + 1 << " <= 1\n";
    }

    if (ILP){
        file << "\nBinary\n";
        for (size_t i = 0; i < hyperedges.size(); ++i) {
            if (covered[i]) continue; // Skip vertices which are already satisfied
            file << " x" << i + 1 << "\n";
        }
    }
    

    file << "End\n";
    file.close();

}
