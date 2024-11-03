#include <iostream>
#include "graph.h"

int main() {
    int numVertices = 6;
    Graph g(numVertices);

    // Adding edges to the graph
    g.addEdge(0, 1);
    g.addEdge(0, 2);
    g.addEdge(1, 3);
    g.addEdge(2, 3);
    g.addEdge(3, 4);
    g.addEdge(4, 5);

    // Get the dominating set using the greedy algorithm
    std::vector<int> dominatingSet = g.greedyDominatingSet();

    // Output the dominating set
    std::cout << "Approximate Dominating Set: ";
    for (int v : dominatingSet) {
        std::cout << v << " ";
    }
    std::cout << std::endl;

    return 0;
}
