#!/usr/bin/env sh

cxx='g++ -std=c++20'
cxxflags='-Wall -Wextra -O2 -static'

Run(){ echo "$@"; $@; }

set -e

Run $cxx $cxxflags graph.cpp -o graph
Run ./graph
