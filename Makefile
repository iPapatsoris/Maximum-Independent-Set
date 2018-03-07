OBJS =  Graph.o Launcher.o ControlUnit.o ExactAlg.o
HEADER = src/Graph.hpp src/ControlUnit.hpp src/exact/ExactAlg.hpp
all: mis

CC 	= g++
FLAGS	= -std=c++11 -O2 -c

mis: $(OBJS) $(HEADER)
	$(CC) -O2 -o mis $(OBJS)

Graph.o: src/Graph.cpp
	$(CC) $(FLAGS) src/Graph.cpp

Launcher.o: src/Launcher.cpp
	$(CC) $(FLAGS) src/Launcher.cpp

ControlUnit.o: src/ControlUnit.cpp
	$(CC) $(FLAGS) src/ControlUnit.cpp

ExactAlg.o: src/exact/ExactAlg.cpp
	$(CC) $(FLAGS) src/exact/ExactAlg.cpp

clean:
	rm -f mis $(OBJS)

count:
	wc -l src/*.cpp src/*.hpp src/exact/*.cpp src/exact/*.hpp $(HEADER)
