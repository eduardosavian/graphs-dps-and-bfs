#include <iostream>
#include <string>

#include "utils/reader.hpp"
#include "utils/graph.hpp"
#include "utils/plot.hpp"
#include "algorithms/searcher.hpp"

// get the name from the termnail
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    std::string file = argv[1];
    //sinput = readFile(file);
    createGraph("<{1,2,3,4},{1,2},{1,3},{2,3},{2,4},{3,4}>");
    plotGraphMatrix();

    return 0;
}