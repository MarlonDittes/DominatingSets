#include <iostream>
#include <memory>
#include <stdexcept>
#include <array>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <cmath>
#include <regex>

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

void generateCSVForGraphs(const std::string& folderPath, const std::string& outputCSV) {
    DIR* dir = opendir(folderPath.c_str());
    if (!dir) {
        std::cerr << "Could not open directory: " << folderPath << std::endl;
        return;
    }

    struct dirent* entry;
    std::ofstream csvFile(outputCSV);
    if (!csvFile.is_open()) {
        std::cerr << "Could not open output CSV file: " << outputCSV << std::endl;
        return;
    }

    // Write CSV header
    csvFile << "Name,Vertices,Edges,Density,Max Degree,Lower Bound,Upper Bound,Triangles,Average Degree,Std Dev Degree" << std::endl;

    // Iterate through all files in the directory
    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;

        // Only process .gr files
        if (filename.size() >= 3 && filename.substr(filename.size() - 3) == ".gr") {
            std::string filepath = folderPath + "/" + filename;
            auto graph = readGraphFromFile(filepath);

            //write HittingSet ILP formulation
            //graph.writeHittingSetILP(filepath + ".lp");
            
            int numVertices = graph.getVertices();
            int numEdges = graph.getEdges();
            double density = graph.computeDensity();
            int maxDegree = graph.getMaxDegree();
            double lowerBound = graph.computeEfficiencyLowerBound();
            auto greedySol = graph.greedyDominatingSet();
            int upperBound = greedySol.size();
            int triangles = graph.countTriangles();
            auto [avgDegree, stdDev] = graph.computeDegreeStats();
            

            // Write the results to the CSV
            csvFile << filename << ","
                    << numVertices << ","
                    << numEdges << ","
                    << density << ","
                    << maxDegree << ","
                    << lowerBound << ","
                    << upperBound << ","
                    << triangles << ","
                    << avgDegree << ","
                    << stdDev << std::endl;
        }
    }

    closedir(dir);
    csvFile.close();
    std::cout << "CSV file generated: " << outputCSV << std::endl;
}

void generateReductionCSV(const std::string& folderPath, const std::string& outputCSV) {
    DIR* dir = opendir(folderPath.c_str());
    if (!dir) {
        std::cerr << "Could not open directory: " << folderPath << std::endl;
        return;
    }

    struct dirent* entry;
    std::ofstream csvFile(outputCSV);
    if (!csvFile.is_open()) {
        std::cerr << "Could not open output CSV file: " << outputCSV << std::endl;
        return;
    }

    // Write CSV header
    csvFile << "Name,Isolated,Single Edge,Dominating,Set Size" << std::endl;

    // Iterate through all files in the directory
    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;

        // Only process .gr files
        if (filename.size() >= 3 && filename.substr(filename.size() - 3) == ".gr") {
            std::string filepath = folderPath + "/" + filename;
            auto graph = readGraphFromFile(filepath);
            
            std::vector<int> dominatingSet(0);
            int isolatedVertexUsage = 0;
            int singleEdgeVertexUsage = 0;
            int dominatingVertexUsage = 0;
            bool changed = true;

            while (changed){
                changed = false;
                int currentUsage;
                
                currentUsage = graph.reductionIsolatedVertex(dominatingSet, false);
                if (currentUsage != 0) changed = true;
                isolatedVertexUsage+= currentUsage;
                
                currentUsage = graph.reductionSingleEdgeVertex(dominatingSet, false);
                if (currentUsage != 0) changed = true;
                singleEdgeVertexUsage+= currentUsage;

                currentUsage = graph.reductionDominatingVertex(dominatingSet, false);
                if (currentUsage != 0) changed = true;
                dominatingVertexUsage+= currentUsage;
            }
            

            // Write the results to the CSV
            csvFile << filename << ","
                    << isolatedVertexUsage << ","
                    << singleEdgeVertexUsage << ","
                    << dominatingVertexUsage << ","
                    << dominatingSet.size() << std::endl;
        }
    }

    closedir(dir);
    csvFile.close();
    std::cout << "CSV file generated: " << outputCSV << std::endl;
}

std::pair<double, double> parseReport(const std::string& report, const std::string& solver){
    std::pair<double, double> result = {-1, -1}; // Default values if parsing fails

    if (solver == "highs"){
        std::regex primalBoundRegex(R"(Primal bound\s+(\d+\.?\d*))");
        std::regex timingRegex(R"(Timing\s+([\d\.]+)\s+\(total\))");
        
        std::smatch match;

        // Extract primal bound
        if (std::regex_search(report, match, primalBoundRegex) && match.size() > 1) {
            result.first = std::stod(match[1]);
        }

        // Extract total time
        if (std::regex_search(report, match, timingRegex) && match.size() > 1) {
            result.second = std::stod(match[1]);
        }
    } else if (solver == "scip"){
        std::regex primalBoundRegex(R"(Primal Bound\s+:\s+([+-]?\d+\.?\d*(e[+-]?\d+)?))");
        std::regex timingRegex(R"(Solving Time \(sec\)\s+:\s+([\d\.]+))");
        
        std::smatch match;

        // Extract primal bound
        if (std::regex_search(report, match, primalBoundRegex) && match.size() > 1) {
            result.first = std::stod(match[1]);
        }

        // Extract total time
        if (std::regex_search(report, match, timingRegex) && match.size() > 1) {
            result.second = std::stod(match[1]);
        }

        return result;
    }
    

    return result;
}

using std::cout;
using std::endl;

int main(int argc, char* argv[]) {
    //std::string name = "testset";
    //generateCSVForGraphs("../graphs/" + name, "../graphs/" + name + "/properties.csv");
    //generateReductionCSV("../graphs/" + name, "../graphs/" + name + "/reductions.csv");

    ///*
    // Ensure the correct number of arguments are provided
    if (argc == 1) {
        std::cerr << "Usage: " << argv[0] << " <graphfile> <solver>" << std::endl;
        std::cerr << "Additionally for findminhs: <solutionfile> <settingsfile>" << std::endl;
        return 1;
    }

    // Extract file paths from command line arguments
    std::string graphFile = argv[1];
    std::string solver = argv[2];
    
    // Read graph from file
    auto graph = readGraphFromFile(graphFile);
    bool verbose = false;
    bool reductions = false;

    if (reductions){
        std::vector<int> dominatingSet(0);
        int isolatedVertexUsage = 0;
        int singleEdgeVertexUsage = 0;
        int dominatingVertexUsage = 0;
        bool changed = true;

        while (changed){
            changed = false;
            int currentUsage;
            
            currentUsage = graph.reductionIsolatedVertex(dominatingSet, verbose);
            if (currentUsage != 0) changed = true;
            isolatedVertexUsage+= currentUsage;
            
            currentUsage = graph.reductionSingleEdgeVertex(dominatingSet, verbose);
            if (currentUsage != 0) changed = true;
            singleEdgeVertexUsage+= currentUsage;

            currentUsage = graph.reductionDominatingVertex(dominatingSet, verbose);
            if (currentUsage != 0) changed = true;
            dominatingVertexUsage+= currentUsage;
        }

        if (verbose){
            for (auto node : dominatingSet){
                cout << node+1 << endl;
            }
        }
    }

    graph.writeHittingSetLP("../analyze.lp");

    if (solver == "findminhs"){
        std::string solutionFile = argv[3];
        std::string settingsFile = argv[4];

        // Convert to hypergraph format for findminhs solver by Felerius (https://github.com/Felerius/findminhs)
        std::string hypergraphFile = "temp.hgr"; 
        graph.graphToHypergraph(hypergraphFile);

        // Run findminhs and capture output
        std::string command = "./findminhs-linux64 solve --solution " + solutionFile + " " + hypergraphFile + " " + settingsFile;

        std::string output = exec(command);
        if (verbose){
            std::cout << output;
            std::cout << std::endl;
        }

        // Delete temporary hypergraph file
        std::remove(hypergraphFile.c_str());

        // Output solution
        if (verbose){
            std::cout << "Findminhs solver solution:" << std::endl;
            auto solution = readJsonArray(solutionFile);
            outputSolution(solution);
        }
    }

    if (solver == "highs"){
        std::string lpFile = "temp.lp";
        graph.writeHittingSetILP(lpFile);

        std::string command = "./highs " + lpFile;
        std::string output = exec(command);
        if (verbose){
            std::cout << output;
            std::cout << std::endl;
        }

        auto result = parseReport(output, solver);
        cout << result.first << "," << result.second << endl;

        std::remove(lpFile.c_str());
    }

    if (solver == "scip"){
        std::string lpFile = "temp.lp";
        graph.writeHittingSetILP(lpFile);

        std::string command = "scip -f " + lpFile;
        std::string output = exec(command);
        if (verbose){
            std::cout << output;
            std::cout << std::endl;
        }
        
        auto result = parseReport(output, solver);
        cout << result.first << "," << result.second << endl;

        std::remove(lpFile.c_str());
    }

    if (solver == "domsat"){
        std::string cutoff = argv[3];

        // Convert to SAT format for domsat solver
        std::string SAT_file = "temp.sat"; 
        graph.graphToSAT(SAT_file);
    
        // Run domsat and capture output
        std::string command = "./DomSAT " + SAT_file + " " + cutoff;

        std::string output = exec(command);
        std::cout << output;
        std::cout << std::endl;

        // Delete temporary hypergraph file
        std::remove(SAT_file.c_str());
    }

    if (solver == "nusc"){
        std::string cutoff = argv[3];
        std::string seed = argv[4];

        // Convert to SAT format for NuSC solver
        std::string SAT_file = "temp.sat"; 
        graph.graphToSAT(SAT_file);
    
        // Run NuSC and capture output
        std::string command = "./NuSC " + SAT_file + " " + cutoff + " " + seed;

        std::string output = exec(command);
        std::cout << output;
        std::cout << std::endl;

        // Delete temporary hypergraph file
        std::remove(SAT_file.c_str());
    }

    if (solver == "lp"){
        std::string lpFile = "temp.lp";
        graph.writeHittingSetLP(lpFile);

        std::string command = "scip -f " + lpFile;
        std::string output = exec(command);
        if (verbose){
            std::cout << output;
            std::cout << std::endl;
        }
        
        auto result = parseReport(output, "scip");
        cout << result.first << "," << result.second << endl;

        std::remove(lpFile.c_str());
    }

    if (solver == "ilp_check"){
        int k = std::stoi(argv[3]);

        std::string lpFile = "temp.lp";
        graph.writeHittingSetILP_check(lpFile, k);

        std::string command = "scip -f " + lpFile;
        std::string output = exec(command);
        if (verbose){
            std::cout << output;
            std::cout << std::endl;
        }
        
        std::regex infeasibleRegex(R"(Primal Bound\s*:\s*infeasible|problem infeasible)");
        if (std::regex_search(output, infeasibleRegex)){
            cout << "infeasible" << endl;
        } else{
            cout << "feasible" << endl;
        }

        std::remove(lpFile.c_str());
    }
    //*/
    
    return 0;
}
