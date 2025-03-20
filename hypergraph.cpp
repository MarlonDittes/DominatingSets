#include "hypergraph.h"

Hypergraph::Hypergraph(int edge_count) : hyperedges(edge_count), useConstraint(edge_count, true), useVariable(edge_count, true) {}

void Hypergraph::addEdge(int u, int v){
    hyperedges[u-1].push_back(v-1);
    hyperedges[v-1].push_back(u-1);
}

void Hypergraph::printHypergraph(){
    std::cout << "Hypergraph:" << std::endl;
    for (size_t i = 0; i < hyperedges.size(); ++i) {
        if (!useConstraint[i]) std::cout << "C ";
        if (!useVariable[i]) std::cout << "V ";

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
            useConstraint[i] = false;
            useVariable[i] = false;
            reductionCount++;

            if (verbose) std::cout << "Edge "  << i+1 << " was isolated." << std::endl;
        }
    }
    return reductionCount;
}

int Hypergraph::reductionSingleEdgeVertex(std::vector<int>& dominatingSet, bool verbose){
    int reductionCount = 0;
    for (size_t i = 0; i < hyperedges.size(); ++i) {
        if (!useConstraint[i]) continue; // Make sure we still need to cover i

        if (hyperedges[i].size() == 1) {
            int neighbor = hyperedges[i][0];
            if (!useVariable[neighbor]) continue;

            dominatingSet.push_back(neighbor);
            useVariable[i] = false;     // Will never need i in optimal solution
            useConstraint[neighbor] = false;
            useVariable[neighbor] = false;
            for (auto v : hyperedges[neighbor]){ // All adjacent vertices' constraints are now inactive
                useConstraint[v] = false;
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
        if (!useVariable[i]) continue; // Make sure i is not yet dominated

        std::set<int> edgeSet(hyperedges[i].begin(), hyperedges[i].end());
        edgeSet.insert(i); // Include its own ID

        for (size_t j = 0; j < hyperedges.size(); ++j) {
            if (i == j || !useVariable[j]) continue; // Skip itself and already dominated variables

            if (hyperedges[j].size() > hyperedges[i].size()) continue; // If other edge contains more vertices, initial edge can't dominate

            std::set<int> otherEdgeSet(hyperedges[j].begin(), hyperedges[j].end());
            otherEdgeSet.insert(j); // Include its own ID

            // If edge i dominates edge j, j is never in optimal solution
            if (std::includes(edgeSet.begin(), edgeSet.end(), otherEdgeSet.begin(), otherEdgeSet.end())) {
                useVariable[j] = false;
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
        if (!useConstraint[i]) continue; // Make sure we still need to cover i

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
            if (i == j) continue; //Skip itself

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

        // Apply Counting Rule if q < r2
        if (q < r2) {
            dominatingSet.push_back(i);
            
            useConstraint[i] = false;
            useVariable[i] = false;

            for (auto& j : hyperedges[i]){
                useConstraint[j] = false;
            }

            reductionCount++;

            if (verbose) {
                std::cout << "Reduction Counting Rule: Removed hyperedge " << i + 1 << " (r2 = " << r2 << ", q = " << q << ")" << std::endl;
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
        if (!useVariable[i]) continue; // Skip vertices which are already satisfied

        if (!first) {
            file << " + ";
        }
        file << "x" << i + 1;
        first = false;
    }
    file << "\n\nSubject To\n";

    // Write the constraints (one per closed neighborhood)
    for (size_t i = 0; i < hyperedges.size(); ++i) {
        if (!useConstraint[i]) continue; // Skip inactive edges

        std::set<int> coveredVertices(hyperedges[i].begin(), hyperedges[i].end());
        coveredVertices.insert(i); // Each hyperedge includes its own ID

        int count = 0;
        std::stringstream constraintStream;
        
        for (int v : coveredVertices) {
            if (!useVariable[v]) continue; // Skip already satisfied vertices

            if (count > 0) {
                constraintStream << " + ";
            }
            constraintStream << "x" << v + 1;
            count++;
        }

        // Only write the constraint if there is at least one valid variable
        if (count > 0) {
            file << " c" << i + 1 << ": " << constraintStream.str() << " >= 1\n";
        }
    }

    // Write bounds and variable types
    file << "\nBounds\n";
    for (size_t i = 0; i < hyperedges.size(); ++i) {
        if (!useVariable[i]) continue; // Skip vertices which are already satisfied
        file << " 0 <= x" << i + 1 << " <= 1\n";
    }

    if (ILP){
        file << "\nBinary\n";
        for (size_t i = 0; i < hyperedges.size(); ++i) {
            if (!useVariable[i]) continue; // Skip vertices which are already satisfied
            file << " x" << i + 1 << "\n";
        }
    }
    

    file << "End\n";
    file.close();

}

/*
void Hypergraph::hypergraphToSAT(const std::string& outputFile) const{
    std::ofstream file(outputFile);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open the output file!");
    }

    // Writing the hypergraph in custom text format described in README.md of https://github.com/Felerius/findminhs
    file << hyperedges.size() << " " << hyperedges.size() << "\n"; // num_vertices num_hyperedges

    for (int u = 0; u < hyperedges.size(); ++u){
        file << 1 << " ";
    }
    file << "\n";

    for (int u = 0; u < hyperedges.size(); ++u) {
        // Insert closed neighborhoods of each vertex as hyperedge
        file << hyperedges[u].size() + 1 << " "; // size of the closed neighborhood
        for (int v : hyperedges[u]) {
            file << v + 1 << " "; // print each node in the neighborhood
        }
        file << u + 1<< "\n"; // Include the vertex itself
    }

    file.close();
}*/

void Hypergraph::hypergraphToSAT(const std::string& outputFile) const{
    std::ofstream file(outputFile);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open the output file!");
    }

    std::vector<int> activeSetIndices(hyperedges.size(), -1);
    std::unordered_map<int, int> setReindexMap;
    
    // What we may pick after reductions (ignore disallowed)
    int setNum = 0;
    for (size_t i = 0; i < hyperedges.size(); ++i) {
        if (useVariable[i]) {       //Need to swap variable and sets when going from dominating set to set cover problem
            activeSetIndices[i] = setNum;
            setReindexMap[i] = setNum++;
        }
    }

    // What still needs to be covered after reductions
    int varNum = 0;
    std::vector<int> activeVarIndices(useVariable.size(), -1);
    for (size_t i = 0; i < useVariable.size(); ++i) {
        if (useConstraint[i]) {
            activeVarIndices[i] = varNum++;
        }
    }
    
    //std::cout << varNum << " " << setNum << std::endl;
    //std::cout << std::endl;
    //for (auto var : activeVarIndices){
    //    std::cout << var << std::endl;
    //}
    //std::cout << std::endl;
    //for (auto var : activeSetIndices){
    //    std::cout << var << std::endl;
    //}

    // Print variable count and set count
    file << varNum << " " << setNum << "\n";

    // Print set weights  =1 for all sets
    for (int i = 0; i < setNum; ++i) {
        file << "1 ";
    }
    file << "\n";
    
    // Print variable-to-set mapping
    for (size_t i = 0; i < useConstraint.size(); ++i) {
        if (!useConstraint[i]) continue; //Skip if this variable is already being covered by a set via previous reductions
        
        int varIndex = activeVarIndices[i];
        std::vector<int> coveredSets;
        
        for (auto elem : hyperedges[i]) {
            if (useVariable[elem]) {
                coveredSets.push_back(setReindexMap[elem]);
            }
        }
        if (useVariable[i]){
            coveredSets.push_back(setReindexMap[i]); //variable itself contained
        }
        
        // Print number of sets covering this variable
        file << coveredSets.size() << " ";
        
        // Print the actual list of sets
        for (int setID : coveredSets) {
            file << setID+1 << " ";
        }
        file << "\n";
    }

    file.close();
}