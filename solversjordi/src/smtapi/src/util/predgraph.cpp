#include "predgraph.h"
#include <cstdio>
#include <iostream>

PredGraph::PredGraph(int n)
{
  this->n = n;
  coefficients.resize(n);
  ind.resize(n);
  adj_list.resize(n);
  for(int i = 0; i < n; i++){
    ind[i]=i;
    coefficients[i] = new std::vector<int>();
  }
}

PredGraph::~PredGraph()
{
    for(std::vector<int> * v: coefficients)
      if(v!=NULL)
	delete v;
}
 
 
void PredGraph::addEdge(int a, int b){
  adj_list[a].insert(b);  
}

void PredGraph::join(int a, int b)
{
    //cout << "JOIN: " << a << "," << b << endl;
    std::vector<int> * first = coefficients[ind[a]];
    std::vector<int> * second = coefficients[b];    
    
    first->insert(first->begin(),second->begin(),second->end());
    
    
    adj_list[ind[a]]=adj_list[b];
    for(int i = 0; i < adj_list.size(); i++)
      adj_list[i].erase(b);
    adj_list[b].clear();
    coefficients[ind[b]] = NULL;
    for(int i = 0; i < adj_list.size(); i++)
      if(ind[i] == b) 
	ind[i] = ind[a];
    
    delete second;
}

void PredGraph::addCoefficient(int i, int coef){
  coefficients[ind[i]]->push_back(coef);  
}

int PredGraph::getCoincidences(int a, int b){
  /*int ncoincidences = 0;
  for(int ca : *coefficients[ind[a]])
    for(int cb : *coefficients[ind[b]])
      if(ca==cb)
	ncoincidences++;
  return ncoincidences;*/
  /*std::std::set<int> s;
  s.insert(coefficients[ind[a]]->begin(),coefficients[ind[a]]->end());
  s.insert(coefficients[ind[b]]->begin(),coefficients[ind[b]]->end());
  return - s.size();*/
  
  std::set<int> s;
  s.insert(coefficients[ind[a]]->begin(),coefficients[ind[a]]->end());
  s.insert(coefficients[ind[b]]->begin(),coefficients[ind[b]]->end());
  return coefficients[ind[a]]->size() + coefficients[ind[b]]->size() - s.size();
}

void PredGraph::greedyCoincidencesCover(){
  bool existedge = true;
  while(existedge){
    existedge=false;
    int max = INT_MIN;
    int maxpred, maxsuc;
    
    for(int i = 0; i < n; i++) if(i==ind[i]){
      for(int j : adj_list[i]) {
	existedge = true;
	int ncoincidences = getCoincidences(i,j);
	if(ncoincidences > max){
	  max = ncoincidences;
	  maxpred = i;
	  maxsuc = ind[j];
	}
      }
    }
    
    if(existedge)
      join(maxpred,maxsuc);
    
  }
}

void PredGraph::givenEdgesCover(std::list<std::pair<int,int> >edges){
  
  
  
}

void PredGraph::getSets(std::vector<std::set<int> > & groups,const std::vector<int> & mapping)
{
  std::map<int,std::set<int> > m;
  /*for(int i = 0; i < ind.size(); i++)
    cout  << mapping[i]  << ",";
  */
  //cout << endl;
  for(int i = 0; i < ind.size(); i++){
    m[ind[i]].insert(mapping[i]);
    //cout  << ind[i]  << ",";
  }
  //cout << endl;
  for(int i = 0; i < ind.size(); i++)
    if(!m[i].empty())
      groups.push_back(m[i]);
}
  
 
