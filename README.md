⚠️ **WARNING ⚠️  
Repository has been archived due to a significant open bug. Solving it would require a lot of time and effort, as the research papers its based one are very extensive, and a lot of changes need to be made.
For the foreseeable future, please do not use this program.**


# Maximum-Independent-Set
A program for finding an exact solution to the [Maximum Independent Set](https://en.wikipedia.org/wiki/Independent_set_(graph_theory)) problem in Graph Theory. The implementation is based
on the publication [Exact Algorithms for Maximum Independent Set](https://arxiv.org/pdf/1312.6260.pdf), by Mingyu Xiao and Hiroshi Nagamochi.

## Usage
**`./mis <input_graph> `**  
**`./mis <input_graph> -check `**

Replace `<input_graph>` with the input file location. The first run option executes the algorithm, and outputs the maximum independent set to
a new file named `<input_graph>.mis`. The second one verifies whether the vertex set at `<input_graph>.mis` is an independent set or not,
according to the respective graph at `<input_graph>`.

### Input graph format
The program is designed to easily work with graphs generated by the graph processing library [SNAP](http://snap.stanford.edu/snap/index.html),
and so the input format is based on the one used by SNAP.
The first two lines are ignored, and the third one contains the number of nodes x, and the number of edges y, as follows: `# Nodes: x Edges: y`.
Then on the next line follows a list of edges, one edge on each line, according to the format below:  
```
x1 y1  
x2 y2  
...
```

This indicates that there is an edge between nodes x1 and y1, x2 and y2, etc. The nodes on the left column are sorted in ascending order,
the nodes on the right column that correspond to a specific node on the left are also locally sorted in ascending order, and are
larger than the corresponding left column node. Any nodes that belong within the total node count but don't appear on the list, are considered zero degree nodes.

A graph that uses this format can be easily generated with SNAP, by using a command like the following:
`Snap-4.0/examples/graphgen/graphgen -g:e -n:100 -m:500 -o:graph.txt`


### Output

At its core, the algorithm follows a branch and reduce paradigm, typically reducing the
graph to two different smaller instances, and solving those first independently, creating a search tree.
During execution, the program prints on the screen several numbers in descending order.
These correspond to the id of the closest to the root problem instance that has been solved. When the
instance with id 0 (root) is solved, a solution to the initial problem is found, thus the program terminates.
This is a useful indication for predicting the time remaining for instances that may take a long time to solve.

The maximum independent set size is also displayed. As mentioned earlier too, the actual content of the maximum independent set is placed
in `<input_graph>.mis`.
