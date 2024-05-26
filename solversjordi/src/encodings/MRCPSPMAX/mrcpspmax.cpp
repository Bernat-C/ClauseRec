#include "limits.h"
#include "mrcpspmax.h"
#include "util.h"
#include "bipgraph.h"
#include "disjointset.h"

using namespace util;
using namespace std;

MRCPSPMAX::MRCPSPMAX(int nactivities, int nrenewable, int nnonrenewable){
	this->nactivities = nactivities;
	this->nrenewable = nrenewable;
	this->nnonrenewable = nnonrenewable;
	this->nresources = nrenewable + nnonrenewable;
	succs = new vector<int> [nactivities+2];
	nmodes = new int[nactivities+2];
	extPrecs = new int * [nactivities+2];
	duration = new vector<int> [nactivities+2];
	timelags = new map<int,int** >[nactivities+2];
	demand = new vector<vector<int> >[nactivities+2];
	capacity = new int[nresources];

	for(int i = 0; i < nactivities+2; i++){
		extPrecs[i] = new int[nactivities+2];
		for(int j = 0; j < nactivities+2;j++)
			extPrecs[i][j] = INT_MIN;
		demand[i].resize(nresources);
	}
}

MRCPSPMAX::~MRCPSPMAX(){

	for(int i = 0; i < nactivities+2; i++){
		for(int j : succs[i]){
			for(int mi = 0; mi < nmodes[i]; mi++)
				delete [] timelags[i][j][mi];
			delete [] timelags[i][j];
		}
		delete [] extPrecs[i];
	}
	delete [] timelags;
	delete [] nmodes;
	delete [] succs;
	delete [] duration;
	delete [] capacity;
	delete [] extPrecs;
	delete [] demand;
}

int MRCPSPMAX::getNActivities() const{
	return nactivities;
}

int MRCPSPMAX::getNResources() const{
	return nresources;
}

int MRCPSPMAX::getNRenewable() const{
	return nrenewable;
}

int MRCPSPMAX::getNNonRenewable() const{
	return nnonrenewable;
}

void MRCPSPMAX::setDuration(int i, int mode, int p){
	duration[i][mode] = p;
}

void MRCPSPMAX::setCapacity(int r, int c){
	capacity[r]=c;
}

int MRCPSPMAX::getCapacity(int r) const{
	return capacity[r];
}

int MRCPSPMAX::getDuration(int i, int mode) const{
	return duration[i][mode];
}

void MRCPSPMAX::setDemand(int i, int r, int mode, int d){
	demand[i][r][mode] = d;
}

int MRCPSPMAX::getDemand(int i, int r, int mode) const{
	return demand[i][r][mode];
}

void MRCPSPMAX::addSuccessor(int i, int j){
	succs[i].push_back(j);
	int modesi = nmodes[i];
	int modesj = nmodes[j];
	timelags[i][j] = new int *[modesi];
	for(int m = 0; m < modesi; m++)
		timelags[i][j][m] = new int[modesj];
}

const vector<int> & MRCPSPMAX::getSuccessors(int i) const {
	return succs[i];
}

void MRCPSPMAX::computeExtPrecs(){
	for (int i=0;i<nactivities+2;i++)
		for(int j : succs[i])
			extPrecs[i][j] = getMinTimeLag(i,j);

	util::floydWarshall(extPrecs,nactivities+2);
}


void MRCPSPMAX::setNModes(int i, int n){
	nmodes[i]=n;
	duration[i]=vector<int>(n,0);
	for(int r = 0; r < nresources; r++)
		demand[i][r]=vector<int>(n,0);
}


int MRCPSPMAX::getNModes(int i) const{
	return nmodes[i];
}

void MRCPSPMAX::setTimeLag(int i, int j, int mi, int mj, int l){
	timelags[i][j][mi][mj]=l;
}

int MRCPSPMAX::getTimeLag(int i, int j,int mi, int mj) const{
	return timelags[i][j][mi][mj];
}

int MRCPSPMAX::getExtPrec(int i, int j) const{
	return extPrecs[i][j];
}

bool MRCPSPMAX::isPred(int i, int j) const{
	if(extPrecs[i][j] >= getMaxDuration(i))
		return true;

	bool found=false, pred=false;
	for(int j2 : succs[i]){
		if(j==j2){
			found = true;
			break;
		}
	}
	if(found){
		pred=true;
		for(int mi = 0; mi < nmodes[i]; mi++){
			if(getMinTimeLag(i,j,mi)<duration[i][mi]){
				pred=false;
				break;
			}
		}
	}
	return pred;
}

bool MRCPSPMAX::inPath(int i, int j) const{
	return isPred(i,j) || isPred(j,i);
}

bool MRCPSPMAX::startsBefore(int i, int j) const{
	return extPrecs[i][j] > 0;
}

int MRCPSPMAX::trivialUB() const{
	int ub = 0;
	for(int i = 0; i < nactivities+2; i++){
		int maxtl = getMaxDuration(i);
		for(int j : succs[i]){
			int tl = getMaxTimeLag(i,j);
			if(tl>maxtl)
				maxtl=tl;
		}
		ub+=maxtl;
	}
	return ub;
}

int MRCPSPMAX::trivialLB() const{
	return ES(nactivities+1);
}

int MRCPSPMAX::ES(int i) const{
	return extPrecs[0][i] > 0 ? extPrecs[0][i] : 0;
}

int MRCPSPMAX::LS(int i, int UB) const{
	return extPrecs[i][nactivities+1] > 0 ? UB - extPrecs[i][nactivities+1] : UB;
}

int MRCPSPMAX::EC(int i) const{
	return ES(i) + getMinDuration(i);
}

int MRCPSPMAX::LC(int i, int UB) const{
	return LS(i,UB) + getMaxDuration(i);
}


int MRCPSPMAX::getMinDuration(int i) const{
	int min = INT_MAX;
	for(int d : duration[i])
		if(d < min)
			min = d;
	return min;
}

int MRCPSPMAX::getMaxDuration(int i) const{
	int max = INT_MIN;
	for(int d : duration[i])
		if(d > max)
			max = d;
	return max;
}

int MRCPSPMAX::getMinTimeLag(int i, int j) const{
	int min = INT_MAX;
	for(int mi = 0; mi < nmodes[i]; mi++)
		for(int mj = 0; mj < nmodes[j]; mj++)
			if(timelags[i][j][mi][mj] < min)
				min = timelags[i][j][mi][mj];
	return min;
}

int MRCPSPMAX::getMaxTimeLag(int i, int j) const{
	int max = INT_MIN;
	for(int mi = 0; mi < nmodes[i]; mi++)
		for(int mj = 0; mj < nmodes[j]; mj++)
			if(timelags[i][j][mi][mj] > max)
				max = timelags[i][j][mi][mj];
	return max;
}

int MRCPSPMAX::getMinTimeLag(int i, int j, int mi) const{
	int min = INT_MAX;
	for(int mj = 0; mj < nmodes[j]; mj++)
		if(timelags[i][j][mi][mj] < min)
			min = timelags[i][j][mi][mj];
	return min;
}

int MRCPSPMAX::getMaxTimeLag(int i, int j, int mi) const{
	int max = INT_MIN;
	for(int mj = 0; mj < nmodes[j]; mj++)
		if(timelags[i][j][mi][mj] > max)
			max = timelags[i][j][mi][mj];
	return max;
}


int MRCPSPMAX::getMinDemand(int i, int r) const{
	int min = INT_MAX;
	for(int m = 0; m < nmodes[i]; m++)
		if(demand[i][r][m] < min)
				min = demand[i][r][m];
	return min;
}

int MRCPSPMAX::getMaxDemand(int i, int r) const{
	int max = INT_MIN;
	for(int m = 0; m < nmodes[i]; m++)
		if(demand[i][r][m] > max)
				max = demand[i][r][m];
	return max;
}

int MRCPSPMAX::getMostRepDemand(int i, int r) const{
	int max = 0;
	int most_common = INT_MIN;
	map<int,int> m;
	for (int x : demand[i][r]) {
		m[x]++;
		if (m[x] > max) {
			max = m[x];
			most_common = x;
		}
	}
	return most_common;
}

RCPSPMAX * MRCPSPMAX::getRCPSPMAX(const vector<int> & modes) const{
	RCPSPMAX * p = new RCPSPMAX(nactivities,nrenewable);

	for(int i = 0; i <= nactivities+1; i++){
		p->setDuration(i,getDuration(i,modes[i]));
		for(int r = 0; r < nrenewable; r++)
			p->setDemand(i,r,getDemand(i,r,modes[i]));
		for(int j : getSuccessors(i))
			p->addTimeLag(i,j,getTimeLag(i,j,modes[i],modes[j]));
	}
	for(int r = 0; r < nrenewable; r++)
		p->setCapacity(r,getCapacity(r));

	return p;
}

//If a capacity becomes negative, it means that the instance is unsat
void MRCPSPMAX::reduceNRDemandMin(){
	for(int r = nrenewable; r < nresources; r++){
		for(int i = 0; i < nactivities+2; i++){
			int mindem = getMinDemand(i,r);
			capacity[r]-=mindem;
			for(int m = 0; m < nmodes[i]; m++)
				demand[i][r][m]-=mindem;
		}
	}
}

void MRCPSPMAX::reduceNRDemandMostFrequent(){
	for(int r = nrenewable; r < nresources; r++){
		for(int i = 0; i < nactivities+2; i++){
			int mostrep = getMostRepDemand(i,r);
			capacity[r]-=mostrep;
			for(int m = 0; m < nmodes[i]; m++)
				demand[i][r][m]-=mostrep;
		}
	}
}

void MRCPSPMAX::computeMinPathCover(const vector<int> & vtasks, vector<set<int> > & groups){

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


ostream &operator << (ostream &output, MRCPSPMAX &m)
{
	output << m.nactivities << "\t" << m.nrenewable << "\t" << m.nnonrenewable << "\t" << 0 <<endl;

	for(int i = 0; i < m.nactivities+2; i++){
		output << i << "\t" << m.getNModes(i) << "\t" << m.succs[i].size();
		for(int j : m.getSuccessors(i))
			output << "\t" << j;
		for(int j : m.getSuccessors(i)){
			output << "\t[";
			for(int k = 0; k < m.getNModes(i); k++)
				for(int l = 0; l < m.getNModes(j); l++)
					output << " " << m.getTimeLag(i,j,k,l);
			output << "]";
		}
		output << endl;
	}

	for(int i = 0; i < m.nactivities+2; i++){
		output << i;
		for(int o = 0; o < m.getNModes(i); o++){
			output << "\t" << o+1 <<"\t" <<  m.getDuration(i,o);
			for(int k = 0; k < m.nresources; k++)
				output << "\t" << m.getDemand(i,k,o);
			output << endl;
		}
	}

	for(int k = 0; k < m.nresources; k++)
		output << m.getCapacity(k) << "\t";
	output << endl;
	return output;
}
