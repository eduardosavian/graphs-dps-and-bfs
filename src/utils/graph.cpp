#ifndef GRAPH_CPP
#define GRAPH_CPP

#include "graph.hpp"

void createGraph(const std::string& input) {
    std::vector<int> vertices;
    std::vector<std::pair<int, int>> edges;

    std::string temp;
    for (uint i = 0; i < input.size(); i++) {
        if (input[i] == '{') {
            i++;
            while (input[i] != '}') {
                if (input[i] == ',') {
                    vertices.push_back(std::stoi(temp));
                    temp = "";
                } else {
                    temp += input[i];
                }
                i++;
            }
            vertices.push_back(std::stoi(temp));
            temp = "";
        }
    }

    for (uint i = 0; i < input.size(); i++) {
        if (input[i] == '{') {
            i++;
            while (input[i] != '}') {
                if (input[i] == ',') {
                    temp = "";
                } else {
                    temp += input[i];
                }
                i++;
            }
            edges.push_back(std::make_pair(std::stoi(temp), std::stoi(temp)));
            temp = "";
        }
    }

    std::cout << "Vertices: ";
    for (uint i = 0; i < vertices.size(); i++) {
        std::cout << vertices[i] << " ";
    }
    std::cout << "\n";

    std::cout << "Edges: ";
    for (uint i = 0; i < edges.size(); i++) {
        std::cout << "(" << edges[i].first << ", " << edges[i].second << ") ";
    }
    std::cout << "\n";

}

#endif