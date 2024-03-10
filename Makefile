CC = clang++
CFLAGS = -Wall -Wextra -std=c++23 -O2
OPATH = objects
CPATH = src

.PHONY: all clean

all: graph

graph: main.o reader.o graph.o plot.o searcher.o
	$(CC) -o graph $(OPATH)/main.o $(OPATH)/reader.o $(OPATH)/graph.o $(OPATH)/plot.o $(OPATH)/searcher.o $(CFLAGS)

main.o: $(CPATH)/main.cpp $(CPATH)/utils/reader.hpp  $(CPATH)/algorithms/searcher.hpp
	$(CC) -c -o $(OPATH)/main.o $(CPATH)/main.cpp $(CFLAGS)

reader.o: $(CPATH)/utils/reader.cpp $(CPATH)/utils/reader.hpp
	$(CC) -c -o $(OPATH)/reader.o $(CPATH)/utils/reader.cpp $(CFLAGS)

graph.o: $(CPATH)/utils/graph.cpp $(CPATH)/utils/graph.hpp
	$(CC) -c -o $(OPATH)/graph.o $(CPATH)/utils/graph.cpp $(CFLAGS)

plot.o: $(CPATH)/utils/plot.cpp $(CPATH)/utils/plot.hpp
	$(CC) -c -o $(OPATH)/plot.o $(CPATH)/utils/plot.cpp $(CFLAGS)

searcher.o: $(CPATH)/algorithms/searcher.cpp $(CPATH)/algorithms/searcher.hpp
	$(CC) -c -o $(OPATH)/searcher.o $(CPATH)/algorithms/searcher.cpp $(CFLAGS)

clean:
	rm -rf $(OPATH)/*.o *~ graph
