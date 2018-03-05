OBJS =  Graph.o Launcher.o ControlUnit.o ExactAlg.o
HEADER = src/Graph.hpp src/ControlUnit.hpp src/exact/ExactAlg.hpp
all: mis

CC 	= g++
FLAGS	= -O2 -c

mis: $(OBJS) $(HEADER)
	$(CC) -g3 -O2 -o mis $(OBJS)

Graph.o: src/Graph.cpp
	$(CC) $(FLAGS) src/Graph.cpp

Launcher.o: src/Launcher.cpp
	$(CC) $(FLAGS) src/Launcher.cpp

ControlUnit.o: src/ControlUnit.cpp
	$(CC) $(FLAGS) src/ControlUnit.cpp

ExactAlg.o: src/exact/ExactAlg.cpp
	$(CC) -std=c++11 $(FLAGS) src/exact/ExactAlg.cpp

clean:
	rm -f mis $(OBJS)

count:
	wc -l src/*.cpp $(HEADER)
