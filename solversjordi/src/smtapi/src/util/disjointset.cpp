#include "disjointset.h"
#include <iostream>    

DisjointSet::DisjointSet(int n)
{
  sets.resize(n);
  for(int i = 0; i < n; i++){
    sets[i] = new std::set<int>();
    sets[i]->insert(i);
  }
}

DisjointSet::~DisjointSet()
{
    for(int i = 0; i < sets.size(); i++){
    std::set<int> * group = sets[i]; 
    if(group != NULL){
      for(int j : (*group))
	sets[j]=NULL;       
      delete group;
    }
  }
}
 
void DisjointSet::join(int a, int b)
{
    std::set<int> * first = sets[a];
    std::set<int> * second = sets[b];
        
    if(first->size() < second->size()){
      std::set<int> *aux = first;
      first = second;
      second = aux;
    }
    
    first->insert(second->begin(),second->end());
    
    for(int i : (*second))
      sets[i] = first;
    
    delete second;
}


void DisjointSet::getSets(std::vector<std::set<int> > & groups,const std::vector<int> & mapping)
{
  std::map<std::set<int> *,bool> inserted;
  for(std::set<int> * group : sets){
    if(!inserted[group]){
      std::set<int> aux;
      for(int i : *group)
	aux.insert(mapping[i]);
      groups.push_back(aux);       
      inserted[group]=true;
    }
  }
}
  
 
