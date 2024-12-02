#include <iostream>
#include <memory>
#include <stdexcept>
#include <array>
#include <fstream>
#include <sstream>

#include "graph.h"

std::string exec(const std::string& command) {
    std::array<char, 128> buffer;
    std::string result;
    
    // Define the correct type for the deleter
    auto pclose_deleter = [](FILE* f) { pclose(f); };

    // Open a pipe to run the command
    std::unique_ptr<FILE, decltype(pclose_deleter)> pipe(popen(command.c_str(), "r"), pclose_deleter);
    
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    
    // Read the output into the result string
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    
    return result;
}

std::vector<int> readJsonArray(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open the file!");
    }

    std::string line;
    std::getline(file, line); 
    file.close();

    // Check for surrounding brackets
    if (line.front() != '[' || line.back() != ']') {
        throw std::runtime_error("Invalid JSON array format!");
    }

    // Remove the brackets
    line = line.substr(1, line.size() - 2);

    std::vector<int> result;
    std::stringstream ss(line);
    std::string item;

    // Parse each element separated by commas
    while (std::getline(ss, item, ',')) {
        try {
            result.push_back(std::stoi(item));
        } catch (const std::invalid_argument& e) {
            throw std::runtime_error("Invalid number in JSON array!");
        }
    }

    return result;
}

// Function to load a graph from a DIMACS-like .gr file format
Graph readGraphFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open the file!");
    }

    int vertices = 0;
    std::vector<std::pair<int, int>> edges;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == 'c') {
            continue; // Skip comments and empty lines
        }

        std::stringstream ss(line);
        if (line[0] == 'p') {
            std::string descriptor;
            int numEdges;
            ss >> descriptor >> descriptor >> vertices >> numEdges;
            edges.reserve(numEdges); // Reserve space for edges based on the number of edges specified
        } else {
            int u, v;
            ss >> u >> v;
            edges.emplace_back(u, v);
        }
    }

    file.close();

    // Initialize the graph with the specified number of vertices and add edges
    Graph graph(vertices);
    for (const auto& edge : edges) {
        graph.addEdge(edge.first, edge.second);
    }

    return graph;
}

void outputSolution(const std::vector<int>& solution){
    std::cout << solution.size() << std::endl;
    for (auto elem : solution){
        std::cout << elem + 1 << std::endl; // Return to 1-based index from 0-based
    }
}

using std::cout;
using std::endl;

int main(int argc, char* argv[]) {
    // Ensure the correct number of arguments are provided
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <graphfile> <solutionfile> <settingsfile>" << std::endl;
        return 1;
    }

    // Extract file paths from command line arguments
    std::string graphFile = argv[1];
    std::string solutionFile = argv[2];
    std::string settingsFile = argv[3];

    // Read graph from file
    auto graph = readGraphFromFile(graphFile);

    cout << graph.computeEfficiencyLowerBound() << endl;


    //graph.writeHittingSetILP(graphFile + ".lp");
    
    // Run and output Greedy solver
    std::vector<int> solution = graph.greedyDominatingSet();
    cout << solution.size() << endl;
    /*
    std::cout << "Greedy solution:" << std::endl;
    outputSolution(solution);
    std::cout << std::endl;

    // Convert to hypergraph format for findminhs solver by Felerius (https://github.com/Felerius/findminhs)
    std::string hypergraphFile = "../temp.hgr"; 
    graph.graphToHypergraph(hypergraphFile);

    // Run findminhs and capture output
    std::string solverName = "../findminhs-linux64";
    std::string command = solverName + " solve --solution " + solutionFile + " " + hypergraphFile + " " + settingsFile;

    std::string output = exec(command);
    std::cout << output;
    std::cout << std::endl;

    // Delete temporary hypergraph file
    std::remove(hypergraphFile.c_str());

    // Output solution
    std::cout << "Findminhs solver solution:" << std::endl;
    solution = readJsonArray(solutionFile);
    outputSolution(solution);

    // Test connected components
    std::cout << std::endl;
    auto pair = graph.getConnectedComponents();
    auto& components = pair.first;
    auto& reverseMappings = pair.second;

    // Display the connected components
    for (size_t i = 0; i < components.size(); ++i) {
        std::cout << "Component " << i + 1 << " (Original Graph Nodes):\n";
        for (size_t j = 0; j < components[i].size(); ++j) {
            // Print the original graph node corresponding to the subgraph node
            int originalNode = reverseMappings[i][j];
            std::cout << "  Original Node " << originalNode << ": ";

            // Map subgraph neighbors back to the original graph
            for (int neighbor : components[i][j]) {
                int originalNeighbor = reverseMappings[i][neighbor];
                std::cout << originalNeighbor << " ";
            }
            std::cout << "\n";
        }
    }
    */

   

    return 0;
}
