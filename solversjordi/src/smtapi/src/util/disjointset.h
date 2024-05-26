#ifndef DISJOINTSET_H
#define DISJOINTSET_H

#include <set>
#include <map>
#include <list>
#include <vector>
#include <cstdlib>
#include <limits.h>


class DisjointSet
{

private:
  
    // number of elements of all sets
    int n;
 
    // Set where each element belongs
    std::vector<std::set<int> *> sets;


public:
    DisjointSet(int n); // Constructor
    ~DisjointSet(); // Destructor
    
    //Joins the set containing 'a' and the set containting 'b'.
    //Pre: 'a' and 'b' are in two different sets
    void join(int a, int b);
            
    void getSets(std::vector<std::set<int> > & sets,const std::vector<int> & mapping);


    
};
#endif
