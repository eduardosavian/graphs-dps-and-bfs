#ifndef GRAPH_CPP
#define GRAPH_CPP

#include "graph.hpp"

void createGraph(const std::string& input) {
    std::vector<std::vector<uint>> matrix;

    // Parse the input string
    std::string inputStr = input.substr(1, input.size() - 2);
    std::cout << "Input: " << inputStr << std::endl;

    // Print the matrix
    for (const auto& row : matrix) {
        for (const auto& value : row) {
            std::cout << value << " ";
        }
        std::cout << std::endl;
    }
}

#endif