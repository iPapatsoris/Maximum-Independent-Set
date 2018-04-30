OBJS =  Graph.o Launcher.o ControlUnit.o Reductions.o Alg.o Mis.o
HEADER = src/Graph.hpp src/ControlUnit.hpp src/Reductions.hpp src/Alg.hpp
all: mis

CC 	= g++
FLAGS	= -g -std=c++11 -O2 -c

mis: $(OBJS) $(HEADER)
	$(CC) -g -O2 -o mis $(OBJS)

Graph.o: src/Graph.cpp
	$(CC) $(FLAGS) src/Graph.cpp

Launcher.o: src/Launcher.cpp
	$(CC) $(FLAGS) src/Launcher.cpp

ControlUnit.o: src/ControlUnit.cpp
	$(CC) $(FLAGS) src/ControlUnit.cpp

Reductions.o: src/Reductions.cpp
	$(CC) $(FLAGS) src/Reductions.cpp

Alg.o: src/Alg.cpp
	$(CC) $(FLAGS) src/Alg.cpp

Mis.o: src/Mis.cpp
	$(CC) $(FLAGS) src/Mis.cpp

clean:
	rm -f mis $(OBJS)

count:
	wc -l src/*.cpp src/*.hpp
