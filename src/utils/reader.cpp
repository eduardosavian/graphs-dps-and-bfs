#ifndef READER_CPP
#define READER_CPP


#include <iostream>
#include <fstream>
#include <string>

#include "reader.hpp"


void readFile(const std::string& filename) {
    std::string filepath = "inputs/" + filename;
    std::cout << "File: " << filepath << "\n";

    std::ifstream file(filepath);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::cout << line << '\n';
        }
        file.close();
    } else {
        std::cout << "Unable to open file\n";
    }
}

#endif