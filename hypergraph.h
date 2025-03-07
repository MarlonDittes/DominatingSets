#ifndef HYPERGRAPH_H
#define HYPERGRAPH_H

#include <vector>
#include <iostream>
#include <fstream>
#include <set>
#include <algorithm>
#include <unordered_map>
#include <sstream>

class Hypergraph {
private:
    std::vector<std::vector<int>> hyperedges;
    std::vector<bool> useConstraint; // false means constraint is irrelevant by now
    std::vector<bool> useVariable; // false means dont use in ILP //SWAPPED!!

public:
    Hypergraph(int edge_count);
    void addEdge(int u, int v);
    void printHypergraph();

    int reductionIsolatedVertex(std::vector<int>& dominatingSet, bool verbose);
    int reductionSingleEdgeVertex(std::vector<int>& dominatingSet, bool verbose);
    int reductionDominatingEdge(std::vector<int>& dominatingSet, bool verbose);
    //int reductionDominatingVertex(std::vector<int>& dominatingSet, bool verbose);
    int reductionCountingRule(std::vector<int>& dominatingSet, bool verbose);

    void writeHittingSetLP(const std::string &outputFile, bool ILP) const;
};

#endif // HYPERGRAPH_H
