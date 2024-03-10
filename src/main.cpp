#include <iostream>

#include <string>

#include "utils/reader.hpp"
#include "algorithms/searcher.hpp"

// get the name from the termnail
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    std::string file = argv[1];
    readFile(file);

    return 0;
}