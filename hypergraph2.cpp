#include "hypergraph2.h"

Hypergraph::Hypergraph(int num_hyperedges, int num_constraints, int num_variables) : hyperedges(num_hyperedges), useConstraint(num_constraints, true), useVariable(num_variables, true) {}

void Hypergraph::initEdge(int vertices){
    for (int i = 0; i < vertices; i++){
        hyperedges[i].push_back(i); // The vertex itself is always included in the closed neighborhood
    }
}

void Hypergraph::addEdge(int u, int v){
    hyperedges[u-1].push_back(v-1);
    hyperedges[v-1].push_back(u-1);
}

void Hypergraph::setVertexToHyperedges(){
    this->vertex_to_hyperedges = hyperedges;
}

void Hypergraph::setHyperedges(const std::vector<std::vector<int>>& sets){
    this->hyperedges = sets;
}

void Hypergraph::setVertexToHyperedges(const std::vector<std::vector<int>>& part_of){
    this->vertex_to_hyperedges = part_of;
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



int Hypergraph::reductionIsolatedVertex(std::set<int>& dominatingSet, bool verbose){
    int reductionCount = 0;
    for (size_t i = 0; i < hyperedges.size(); ++i) {
        if (!useConstraint[i]) continue; // Make sure we still need to cover i

        if (hyperedges[i].size() == 1) {
            auto& vertex = hyperedges[i][0];
            if (!useVariable[vertex]) continue;

            dominatingSet.insert(vertex);
            useVariable[vertex] = false;
            for (auto edge : vertex_to_hyperedges[vertex]){
                useConstraint[edge] = false;
            }
            reductionCount++;

            if (verbose) std::cout << "Edge "  << i+1 << " was isolated." << std::endl;
        }
    }
    return reductionCount;
}

int Hypergraph::reductionSingleEdgeVertex(std::set<int>& dominatingSet, bool verbose){
    int reductionCount = 0;
    for (size_t i = 0; i < hyperedges.size(); ++i) {
        if (!useConstraint[i]) continue; // Make sure we still need to cover i

        if (hyperedges[i].size() == 2) {
            int neighbor = hyperedges[i][1];
            if (!useVariable[neighbor]) continue;

            dominatingSet.insert(neighbor);
            useVariable[i] = false;     // Will never need i in optimal solution
            useVariable[neighbor] = false;
            for (auto v : hyperedges[neighbor]){ // All adjacent vertices' constraints are now inactive/satisfied by neighbor
                useConstraint[v] = false;
            }
            reductionCount++;

            if (verbose) std::cout << "Edge "  << neighbor+1 << " chosen over " << i+1 << std::endl;
        }
    }
    return reductionCount;
}

int Hypergraph::reductionDominatingEdge(std::set<int>& dominatingSet, bool verbose){
    int reductionCount = 0;
    for (size_t i = 0; i < hyperedges.size(); ++i) {
        if (!useConstraint[i]) continue; // Make sure i is not yet dominated

        std::set<int> vertexSet(hyperedges[i].begin(), hyperedges[i].end());

        for (size_t j = 0; j < hyperedges.size(); ++j) {
            if (i == j) continue; // Skip itself; we allow dominating already satisfied constraints, in this case the dominating constraint would also be satisfied
            // TODO: Worth?

            if (hyperedges[j].size() >= hyperedges[i].size()) continue; // If other edge contains more vertices, initial edge can't dominate
            // TODO: delete same hyperedges in extra step somewhere, only letting one remain
        
            std::set<int> otherVertexSet(hyperedges[j].begin(), hyperedges[j].end());

            // If edge i dominates edge j, we only need to satisfy edge j since this will also always satisfy edge i
            if (std::includes(vertexSet.begin(), vertexSet.end(), otherVertexSet.begin(), otherVertexSet.end())) {
                useConstraint[i] = false;
                reductionCount++;

                if (verbose) std::cout << "Edge " << i + 1 << " dominates " << j + 1 << std::endl;
            }
        }
    }
    return reductionCount;
}

int Hypergraph::reductionDominatingVertex(std::set<int>& dominatingSet, bool verbose){
    int reductionCount = 0;
    for (size_t i = 0; i < vertex_to_hyperedges.size(); ++i) {
        if (!useVariable[i]) continue; // Make sure i is not yet dominated

        std::set<int> edgeSet(vertex_to_hyperedges[i].begin(), vertex_to_hyperedges[i].end());

        for (size_t j = 0; j < hyperedges.size(); ++j) {
            if (i == j || !useVariable[j]) continue; // Skip itself and already dominated variables

            if (vertex_to_hyperedges[j].size() > vertex_to_hyperedges[i].size()) continue; // If other vertex contains more edges, initial vertex can't dominate

            std::set<int> otherEdgeSet(vertex_to_hyperedges[j].begin(), vertex_to_hyperedges[j].end());

            // If vertex i dominates vertex j, we may always choose i over j since it can only ever satisfy more constraints
            // This means we may disable j
            if (std::includes(edgeSet.begin(), edgeSet.end(), otherEdgeSet.begin(), otherEdgeSet.end())) {
                useVariable[j] = false;
                reductionCount++;

                if (verbose) std::cout << "Vertex " << i + 1 << " dominates " << j + 1 << std::endl;
            }
        }
    }
    return reductionCount;
}

int Hypergraph::reductionCountingRule(std::set<int>& dominatingSet, bool verbose){
    //TODO: maybe need to turn this around? since it originates from set cover
    int reductionCount = 0;

    // Iterate over each hyperedge (potential set R)
    for (size_t i = 0; i < hyperedges.size(); ++i) {
        if (!useConstraint[i]) continue; // Make sure we still need to cover i

        std::set<int> R(hyperedges[i].begin(), hyperedges[i].end());

        // Compute r2: elements in R that appear exactly twice in the hypergraph
        std::vector<int> freqTwoElements;
        for (int e : R) {
            if (vertex_to_hyperedges[e].size() == 2) {
                freqTwoElements.push_back(e);
            }
        }
        int r2 = freqTwoElements.size();
        if (r2 == 0) continue; // No frequency-two elements, skip

        // Compute q: elements in sets containing a frequency-two element from R but not in R
        std::unordered_set<int> freqTwoSet(freqTwoElements.begin(), freqTwoElements.end());
        std::set<int> externalElements;
        for (size_t j = 0; j < hyperedges.size(); ++j) {
            if (i == j) continue; //Skip itself

            bool hasFreqTwo = false;
            for (int v : hyperedges[j]) {
                if (freqTwoSet.find(v) != freqTwoSet.end()) {
                    hasFreqTwo = true;
                    break;
                }
            }

            if (!hasFreqTwo) continue; // No frequency-two element here, so ignore this hyperedge

            for (int v : hyperedges[j]) {
                if (R.find(v) == R.end()) {
                    externalElements.insert(v);
                }
            }
        }
        int q = externalElements.size();

        // Apply Counting Rule if q < r2
        if (q < r2) {
            dominatingSet.insert(i);
            
            //useConstraint[i] = false;
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
    for (size_t i = 0; i < vertex_to_hyperedges.size(); ++i) {
        if (!useVariable[i]) continue; // Skip disallowed variables

        if (!first) {
            file << " + ";
        }
        file << "x" << i + 1;
        first = false;
    }
    file << "\n\nSubject To\n";

    // Write the constraints (one per closed neighborhood)
    for (size_t i = 0; i < hyperedges.size(); ++i) {
        if (!useConstraint[i]) continue; // Skip inactive constraints

        std::set<int> coveredVertices(hyperedges[i].begin(), hyperedges[i].end());

        int count = 0;
        std::stringstream constraintStream;
        
        for (int v : coveredVertices) {
            if (!useVariable[v]) continue; // Skip disallowed variables

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
    for (size_t i = 0; i < vertex_to_hyperedges.size(); ++i) {
        if (!useVariable[i]) continue; // Skip disallowed variables
        file << " 0 <= x" << i + 1 << " <= 1\n";
    }

    if (ILP){
        file << "\nBinary\n";
        for (size_t i = 0; i < vertex_to_hyperedges.size(); ++i) {
            if (!useVariable[i]) continue; // Skip disallowed variables
            file << " x" << i + 1 << "\n";
        }
    }
    

    file << "End\n";
    file.close();

}

//FUNCTION IS OUTDATED!!!
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

void Hypergraph::writeMaxSAT(const std::string& outputFile) const{
    std::ofstream file(outputFile);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + outputFile);
    }


    // Write hard clauses (one per edge)
    for (size_t i = 0; i < hyperedges.size(); ++i) {
        //there needs to be at least one active variable after h, otherwise unsatisfiable
        if (!useConstraint[i]) continue; // Skip inactive constraints
            
        file << "h";
        for (int neighbor : hyperedges[i]){ //include neighbors
            if (!useVariable[neighbor]) continue; // Skip disallowed variables
            file << " " << neighbor+1;
        }
        file << " 0\n";
    }

    // Write soft clauses (one per vertex)
    for (size_t i = 0; i < hyperedges.size(); ++i) {
        //if (!useVariable[i]) continue; // Skip already satisfied vertices
        file << "1 -" << i+1 << " 0\n";
    }

    file.close();
}