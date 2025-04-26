#ifndef HYPERGRAPH2_H
#define HYPERGRAPH2_H

#include <vector>
#include <iostream>
#include <fstream>
#include <set>
#include <algorithm>
#include <unordered_map>
#include <sstream>
#include <unordered_set>

class Hypergraph {
private:
    std::vector<std::vector<int>> hyperedges;
    std::vector<std::vector<int>> vertex_to_hyperedges;
    std::vector<bool> useConstraint; // false means this constraint is irrelevant by now
    std::vector<bool> useVariable; // false means this variable isn't needed in at least one optimal solution

public:
    Hypergraph(int num_hyperedges, int num_constraints, int num_variables);
    void initEdge(int vertices);
    void addEdge(int u, int v);
    void setVertexToHyperedges();
    void setHyperedges(const std::vector<std::vector<int>>& sets);
    void setVertexToHyperedges(const std::vector<std::vector<int>>& part_of);
    void printHypergraph();

    int reductionIsolatedVertex(std::set<int>& dominatingSet, bool verbose);
    int reductionSingleEdgeVertex(std::set<int>& dominatingSet, bool verbose);
    int reductionDominatingEdge(std::set<int>& dominatingSet, bool verbose);
    int reductionDominatingVertex(std::set<int>& dominatingSet, bool verbose);
    int reductionCountingRule(std::set<int>& dominatingSet, bool verbose);

    void writeHittingSetLP(const std::string &outputFile, bool ILP) const;
    void hypergraphToSAT(const std::string& outputFile) const;
    void writeMaxSAT(const std::string& outputFile) const;
};
#endif // HYPERGRAPH2_H
