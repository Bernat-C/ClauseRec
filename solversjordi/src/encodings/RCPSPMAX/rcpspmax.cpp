#include "limits.h"
#include "rcpspmax.h"
#include "util.h"
#include "bipgraph.h"
#include "disjointset.h"

using namespace std;

RCPSPMAX::RCPSPMAX(int nactivities, int nresources){
	this->nactivities = nactivities;

	this->nresources = nresources;
	succs = new vector<int> [nactivities+2];
	duration = new int[nactivities+2];
	timelags = new map<int,int> [nactivities+2];
	extPrecs = new int * [nactivities+2];
	demand = new int * [nactivities+2];
	capacity = new int[nresources];

	for(int i = 0; i < nactivities+2; i++){
		extPrecs[i] = new int [nactivities+2];
		demand[i] = new int [nresources];
		for(int j = 0; j < nactivities+2; j++)
			extPrecs[i][j]=INT_MIN;
	}

	duration[0] = duration[nactivities+1];
	for(int r = 0; r < nresources; r++)
		demand[0][r] = demand[nactivities+1][r] = 0;
}

RCPSPMAX::~RCPSPMAX(){

	for(int i = 0; i < nactivities+2; i++){
		delete [] extPrecs[i];
		delete [] demand[i];
	}
	delete [] timelags;
	delete [] extPrecs;
	delete [] succs;
	delete [] duration;
	delete [] capacity;
	delete [] demand;
}

int RCPSPMAX::getNActivities() const{
	return nactivities;
}

int RCPSPMAX::getNResources() const{
	return nresources;
}

void RCPSPMAX::setDuration(int i, int p){
	duration[i] = p;
}

int RCPSPMAX::getDuration(int i) const{
	return duration[i];
}

void RCPSPMAX::setCapacity(int r, int c){
	capacity[r]=c;
}

int RCPSPMAX::getCapacity(int r) const{
	return capacity[r];
}

void RCPSPMAX::setDemand(int i, int r, int d){
	demand[i][r]= d;
}

int RCPSPMAX::getDemand(int i, int r) const{
	return demand[i][r];
}

void RCPSPMAX::addSuccessor(int i, int j){
	succs[i].push_back(j);
}

void RCPSPMAX::setTimeLag(int i, int j, int l){
	timelags[i][j] =  l;
}

void RCPSPMAX::addTimeLag(int i, int j, int l){
	succs[i].push_back(j);
	timelags[i][j] =  l;
}

const vector<int> & RCPSPMAX::getSuccessors(int i) const {
	return succs[i];
}

void RCPSPMAX::computeExtPrecs(){
	for (int i=0;i<nactivities+2;i++)
		for(int j : succs[i])
			extPrecs[i][j] = getTimeLag(i,j);
	util::floydWarshall(extPrecs,nactivities+2);
}


int RCPSPMAX::getTimeLag(int i, int j) const{
	return timelags[i].find(j)->second;
}

int RCPSPMAX::getExtPrec(int i, int j) const{
	return extPrecs[i][j];
}

bool RCPSPMAX::isPred(int i, int j) const{
	return extPrecs[i][j] >= duration[i];
}

bool RCPSPMAX::inPath(int i, int j) const{
	return isPred(i,j) || isPred(j,i);
}

bool RCPSPMAX::startsBefore(int i, int j) const{
	return timelags[i][j] > 0;
}

int RCPSPMAX::trivialUB() const{
	int ub = 0;
	for(int i = 0; i < nactivities+2; i++){
		int maxtl = duration[i];
		for(int j : succs[i]){
			if(timelags[i][j] > maxtl)
				maxtl=timelags[i][j];
		}
		ub+=maxtl;
	}
	return ub;
}

int RCPSPMAX::trivialLB() const{
	return ES(nactivities+1);
}

int RCPSPMAX::ES(int i) const{
	return timelags[0][i] > 0 ? timelags[0][i] : 0;
}

int RCPSPMAX::LS(int i, int UB) const{
	return timelags[i][nactivities+1] > 0 ? UB - timelags[i][nactivities+1] : UB;
}

int RCPSPMAX::EC(int i) const{
	return ES(i) + duration[i];
}

int RCPSPMAX::LC(int i, int UB) const{
	return LS(i,UB) + duration[i];
}

void RCPSPMAX::computeMinPathCover(const vector<int> & vtasks, vector<set<int> > & groups){

  vector<pair<int,int> > matching;
  BipGraph bg(vtasks.size()+1,vtasks.size()+1);

  matching.clear();

  for(int i = 0; i < vtasks.size(); i++){
    for(int j = 0; j < vtasks.size(); j++){
      if(i!=j){
			if(isPred(vtasks[i],vtasks[j]))
				bg.addEdge(i+1,j+1);
      }
    }
  }

  bg.hopcroftKarp(matching);

  DisjointSet s(vtasks.size());
  for(const pair<int,int> & edge : matching)
    s.join(edge.first-1,edge.second-1);

  s.getSets(groups,vtasks);
}


ostream &operator << (ostream &output, RCPSPMAX &m)
{
	output << m.nactivities << "\t" << m.nresources << "\t" << endl;

	for(int i = 0; i < m.nactivities+2; i++){
		output << i << "\t" << m.succs[i].size();
		for(int j : m.getSuccessors(i))
			output << "\t" << j << "," << m.getTimeLag(i,j);
		output << endl;
	}

	for(int i = 0; i < m.nactivities+2; i++){
		output << i;
		output <<"\t" <<  m.getDuration(i);
		for(int k = 0; k < m.nresources; k++)
			output << "\t" << m.getDemand(i,k);
		output << endl;
	}

	for(int k = 0; k < m.nresources; k++)
		output << m.getCapacity(k) << "\t";
	output << endl;
	return output;
}
