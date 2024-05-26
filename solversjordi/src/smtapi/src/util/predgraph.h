#ifndef PREDGRAPH_H
#define PREDGRAPH_H

#include <set>
#include <map>
#include <list>
#include <vector>
#include <cstdlib>
#include <limits.h>


class PredGraph
{

private:
  
    // number of nodes
    int n;
    
    //Indirection to join the nodes
    std::vector<int> ind;
 
    // Set where each element belongs
    std::vector<std::set<int> > adj_list;
    
    
    //Coefficients involved in each std::set
    std::vector<std::vector<int> *> coefficients;

public:
    PredGraph(int n); // Constructor
    ~PredGraph(); // Destructor
    
    void addEdge(int a, int b);
    
    //Joins the set containing 'a' and the set containting 'b'.
    //Pre: 'a' and 'b' are in two different sets
    void join(int a, int b);
    
    int getCoincidences(int a, int b);
    
    void addCoefficient(int i, int coef);
        
    void greedyCoincidencesCover();
    
    void givenEdgesCover(std::list<std::pair<int,int> >edges);
    
    void getSets(std::vector<std::set<int> > & sets,const std::vector<int> & mapping);
    
};
 
#endif
