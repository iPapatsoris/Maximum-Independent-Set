OBJS =  Graph.o Reductions.o Alg.o Mis.o ControlUnit.o Util.o Jlwrapper.o
SHAREDLIB = libmis.so
all: sharedlib

CC 	= g++
FLAGS	= -std=c++17 -O3 -c -fPIC
JLCXX_DIR = deps/jlcxx
JULIA_DIR = /opt/julia/julia-1.9.3

sharedlib: $(OBJS) $(HEADER)
	$(CC) -O3 $(OBJS) -L$(JLCXX_DIR)/lib -lcxxwrap_julia -shared -o $(SHAREDLIB)

Util.o: src/Util.cpp
	$(CC) $(FLAGS) -I$(JLCXX_DIR)/include src/Util.cpp

Graph.o: src/Graph.cpp
	$(CC) $(FLAGS) -I$(JLCXX_DIR)/include src/Graph.cpp

ControlUnit.o: src/ControlUnit.cpp
	$(CC) $(FLAGS) -I$(JLCXX_DIR)/include src/ControlUnit.cpp

Reductions.o: src/Reductions.cpp
	$(CC) $(FLAGS) -I$(JLCXX_DIR)/include src/Reductions.cpp

Alg.o: src/Alg.cpp
	$(CC) $(FLAGS) -I$(JLCXX_DIR)/include src/Alg.cpp

Mis.o: src/Mis.cpp
	$(CC) $(FLAGS) -I$(JLCXX_DIR)/include src/Mis.cpp

Jlwrapper.o: src/Jlwrapper.cpp
	$(CC) $(FLAGS) -I$(JLCXX_DIR)/include -I$(JULIA_DIR)/include/julia src/Jlwrapper.cpp

clean:
	rm -f mis $(OBJS) $(SHAREDLIB)

count:
	wc -l src/*.cpp src/*.hpp
