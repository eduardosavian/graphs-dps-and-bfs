#include <iostream>
#include <string>

#include "utils/reader.hpp"
#include "utils/graph.hpp"
#include "utils/plot.hpp"
#include "algorithms/searcher.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    std::string filepath = argv[1];
    std::cout << "Filepath: " << filepath << "\n";
    std::string input = readFile(filepath);
    std::cout << "Input: "<< input << "\n";

    createGraph(input);

    return 0;
}