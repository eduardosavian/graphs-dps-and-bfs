#ifndef READER_CPP
#define READER_CPP


#include <iostream>
#include <fstream>
#include <string>

#include "reader.hpp"


std::string readFile(const std::string& filepath) {
    std::string input;
    std::ifstream file(filepath);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            input = line;
        }
        file.close();
    } else {
        std::cout << "Unable to open file\n";
    }
    return input;
}

#endif