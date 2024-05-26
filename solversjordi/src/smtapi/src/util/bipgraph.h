#ifndef BIPGRAPH_H
#define BIPGRAPH_H

// C++ implementation of Hopcroft Karp algorithm for
// maximum matching
// Source: http://www.geeksforgeeks.org/hopcroft-karp-algorithm-for-maximum-matching-set-2-implementation/


#include <queue>
#include <list>
#include <vector>
#include <cstdlib>
#include <limits.h>

#define NIL 0
#define INF INT_MAX
 
// A class to represent Bipartite graph for Hopcroft
// Karp implementation
class BipGraph
{

private:
  
    // m and n are number of vertices on left
    // and right sides of Bipartite Graph
    int m, n;
 
    // adj[u] stores adjacents of left side
    // vertex 'u'. The value of u ranges from 1 to m.
    // 0 is used for dummy vertex
    std::list<int> *adj;
 
    // These are basically pointers to arrays needed
    // for hopcroftKarp()
    int *pairU, *pairV, *dist;
    
    // Returns true if there is an augmenting path
    bool bfs();
 
    // Adds augmenting path if there is one beginning
    // with u
    bool dfs(int u);
public:
    BipGraph(int m, int n); // Constructor
    ~BipGraph(); // Destructor
    
    void addEdge(int u, int v); // To add edge

    // Returns size of maximum matcing
    int hopcroftKarp(std::vector<std::pair<int,int> >&matching);
    
};
 
#endif
