#include "rcpspt.h"
#include "limits.h"
#include <list>
#include <algorithm>
#include "util.h"
#include "bipgraph.h"
#include "disjointset.h"
#include <math.h>

using namespace std;

RCPSPT::RCPSPT(int nactivities, int nresources, int timeHorizon){

	//Prepare instance data
	this->nactivities = nactivities;
	this->nresources = nresources;
	this->timeHorizon = timeHorizon;
	succs = new vector<int> [nactivities+2];

	duration = new int [nactivities+2];
	demand = new vector<int> * [nresources];
	capacity = new int * [nresources];

	for(int i = 0; i < nresources; i++){
		capacity[i] = new int[timeHorizon];
		demand[i] = new vector<int> [nactivities+2];
	}

	//Prepare preprocessed data
	extPrecs = new int * [nactivities+2];
	nSteps = new int * [nactivities+2];


	for(int i = 0; i < nactivities+2; i++){
		extPrecs[i] = new int[nactivities+2];
		nSteps[i] = new int[nactivities+2];
	}

	for(int i = 0; i < nactivities+2; i++){
		for(int j = 0; j < nactivities+2; j++){
			extPrecs[i][j] = INT_MIN;
			nSteps[i][j] = INT_MIN;
		}
	}

}

RCPSPT::~RCPSPT(){

	//Delete instance data
	delete [] succs;
	delete [] duration;

	delete [] extPrecs;

	for(int i = 0; i < nresources; i++){
		delete [] capacity[i];
		delete [] demand[i];
	}
	delete [] capacity;
	delete [] demand;

	//Delete preprocessed Data
	for(int i = 0; i < nactivities+2; i++){
		delete [] extPrecs[i];
		delete [] nSteps[i];
	}

	delete [] extPrecs;
	delete [] nSteps;
}

int RCPSPT::getTimeHorizon() const{
	return timeHorizon;
}

int RCPSPT::getNActivities() const{
	return nactivities;
}

int RCPSPT::getNResources() const{
	return nresources;
}

void RCPSPT::setCapacity(int r, int t, int c){
	capacity[r][t]=c;
}

int RCPSPT::getCapacity(int r, int t) const{
	return capacity[r][t];
}

void RCPSPT::setDuration(int i, int p){
	duration[i] = p;
	for(int r = 0; r < nresources; r++)
		demand[r][i] = vector<int>(p);
}

int RCPSPT::getDuration(int i) const{
	return duration[i];
}

void RCPSPT::setDemand(int i, int r, int t, int d){
	demand[r][i][t] = d;
}

int RCPSPT::getDemand(int i, int r, int t) const{
	return demand[r][i][t];
}

void RCPSPT::addSuccessor(int i, int j){
	succs[i].push_back(j);
}

const vector<int> & RCPSPT::getSuccessors(int i) const {
	return succs[i];
}


int RCPSPT::getExtPrec(int i, int j) const{
	return extPrecs[i][j];
}

int RCPSPT::getNSteps(int i, int j) const{
	return nSteps[i][j];
}

bool RCPSPT::isPred(int i, int j) const{
	return nSteps[i][j] > 0;
}

bool RCPSPT::inPath(int i, int j) const{
	return isPred(i,j) || isPred(j,i);
}

int RCPSPT::trivialUB() const{
	return timeHorizon;
}

int RCPSPT::trivialLB() const{
	return ES(nactivities+1);
}

int RCPSPT::ES(int i) const{
	return extPrecs[0][i] > 0 ? extPrecs[0][i] : 0;
}

int RCPSPT::LS(int i, int UB) const{
	return extPrecs[i][nactivities+1] > 0 ? UB - extPrecs[i][nactivities+1] : UB;
}

int RCPSPT::EC(int i) const{
	return ES(i) + getDuration(i);
}

int RCPSPT::LC(int i, int UB) const{
	return LS(i,UB) + getDuration(i);
}


void RCPSPT::computeExtPrecs(){
	for (int i=0;i<nactivities+2;i++)
		for(int j : succs[i])
			extPrecs[i][j] = getDuration(i);

	util::floydWarshall(extPrecs,nactivities+2);
}

void RCPSPT::computeEnergyPrecedences(){
	list<int> q;
	vector<int> maxCapacity(nresources,0);
	for(int t = 0; t < timeHorizon; t++){
		for(int r = 0; r < nresources; r++){
			if(getCapacity(r,t) > maxCapacity[r])
				maxCapacity[r] = getCapacity(r,t);
		}
	}

	bool * visited = new bool[nactivities+2];
	for(int i = 0; i < nactivities+1; i++){
		for(int j = 1; j < nactivities+2; j++){
			if(i!=j && extPrecs[i][j]>INT_MIN){

				vector<int> dems(nresources,0);
				for(int k = 0; k < nactivities+2; k++)
					visited[k]=false;

				q.insert(q.end(),succs[i].begin(),succs[i].end());
				while(!q.empty()){
					int k = q.front();
					q.pop_front();
					if(!visited[k] && extPrecs[k][j] > INT_MIN){
						visited[k] = true;
						q.insert(q.end(),succs[k].begin(),succs[k].end());
						for(int r = 0; r < nresources;r++)
							for(int d = 0; d < duration[k]; d++)
								dems[r] += getDemand(k,r,d);
					}
				}
				double max = 0;
				for(int r = 0; r < nresources;r++)
					if(dems[r]/((double)maxCapacity[r]) > max)
						max = dems[r]/((double)maxCapacity[r]);
				int rl = (int) ceil(max) + duration[i];

				if(rl>extPrecs[i][j]){
					extPrecs[i][j]=rl;
				}
			}
		}
	}


	delete [] visited;
}

void RCPSPT::recomputeExtPrecs(){
	util::floydWarshall(extPrecs,nactivities+2);
}

void RCPSPT::computeSteps(){
	for (int i=0;i<nactivities+2;i++)
		for(int j : succs[i])
			nSteps[i][j] = 1;

	util::floydWarshall(nSteps,nactivities+2);
}

int RCPSPT::next_activity(vector<vector<bool> > & predecessors,set<int> & tots,set<int> & C,vector<vector<int> > & pik, int t) const {
	int d=-1;
	set<int>::iterator it=tots.begin();
	while (it!=tots.end()) {
		bool totpredecessorsfets=true;

		for (int i=1;i<nactivities+2;i++)
			if (predecessors[*it][i])
				if (C.find(i)==C.end())
					totpredecessorsfets=false;

		bool correcte=totpredecessorsfets;
		if (totpredecessorsfets) {
			int i=t;
			while (i<t+duration[*it] && correcte){
				if(i >= timeHorizon) return -2;
				int k=0;
				while (k<nresources && correcte) {
					if (pik[i][k]<demand[k][*it][i-t])
						correcte=false;
					k++;
				}
				i++;
			}
		}
		if (correcte) {
			d=*it;
			tots.erase(it);
			it=tots.end();
		}
		else it++;
	}
	return d;
}

//parallel scheduling scheme afagant el primer a "d". Article Kolish 1996
int RCPSPT::computePSS(vector<int> & starts) const{
	starts.resize(nactivities+2);

	vector<vector<bool> > predecessors;
	predecessors.resize(nactivities+2);

	for (int i=0;i<nactivities+2;i++) {
		predecessors[i].resize(nactivities+2);
		for (int j=0;j<nactivities+2;j++)
			predecessors[i][j]=false;
	}

	for (int i=0;i<nactivities+2;i++)
		for (int j : succs[i])
			predecessors[j][i]=true;



	set<int> A,C,tots;
	vector<vector<int> > pik(timeHorizon);
	for(int i= 0; i < timeHorizon; i++){
		pik[i].resize(nresources);
		for(int j = 0; j < nresources; j++){
			pik[i][j] = capacity[j][i];
		}
	}
	int t=0;
	int d=0;
	for (int i=1; i<nactivities+2; i++) tots.insert(i);

	starts[0]=0; //Afegida
	C.insert(0); //Afegida

	while ((A.size()+C.size())!=nactivities+2) {
		if (!A.empty()) {
			int antt=t;
			t=INT_MAX;
			for (int ii : A)
				if(starts[ii]+duration[ii]<t)
					t=starts[ii]+duration[ii];


				//Modificat
			set<int> aesborrar;
			for (int ii : A){
				if (starts[ii]+duration[ii]==t){
					C.insert(ii);
					aesborrar.insert(ii);
				}
			}

			for(int jj: aesborrar)
				A.erase(jj);

			d=next_activity(predecessors,tots,C,pik,t); //calcular nou d
			if(d==-2) return -1;
		}
		else while(d==-1){
			t++;
			d=next_activity(predecessors,tots,C,pik,t); //calcular nou d
			if(d==-2) return -1;
		}
		while (d!=-1) {
			int j=d;
			starts[j]=t;
			A.insert(j);
			for (int i=t;i<t+duration[j];i++) {
				if(i >= timeHorizon) return -1;
				for (int k=0;k<nresources;k++)
					pik[i][k]-=demand[k][j][i-t];
			}
			d=next_activity(predecessors,tots,C,pik,t); //calcular nou d
			if(d==-2) return -1;
		}
	}
	return t;
}

void RCPSPT::computeMinPathCover(const vector<int> & vtasks, vector<set<int> > & groups) const{

  vector<pair<int,int> > matching;
  BipGraph bg(vtasks.size()+1,vtasks.size()+1);

  matching.clear();

  for(int i = 0; i < vtasks.size(); i++){
    for(int j = 0; j < vtasks.size(); j++){
      if(i!=j && getDuration(vtasks[i]) > 0){
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

void RCPSPT::printSolution(ostream & os, const vector<int> & starts){
	for(int i = 0; i < starts.size(); i++)
		os << "S_" << i << ":" << starts[i] << "; ";
}

void RCPSPT::generateParam(){
	cout << "language ESSENCE' 1.0" << endl;
	cout << "letting jobs = " << nactivities+2 << endl;
	cout << "letting horizon = " << timeHorizon-1 << endl;
	cout << "letting resourcesRenew = " << nresources << endl; 

	cout << "letting successors = [";
	for(int i = 0; i < nactivities+2; ++i){
		if(i > 0) cout << ",";
		cout << "[";
		for(int j = 0; j < succs[i].size(); ++j){
			if(j > 0) cout << ",";
			cout << succs[i][j]+1;
		}
		cout << "]";
	}
	cout << "]" << endl;

	cout << "letting durations = [";
	for(int i = 0; i < nactivities+2; ++i){
		if(i > 0) cout << ",";
		cout << duration[i];
	}
	cout << "]" << endl;

	cout << "letting resourceUsage = [";
	for(int i = 0; i < nactivities+2; ++i){
		if(i > 0) cout << ",";
		cout << "[";
		if(i==0 || i == nactivities+1){
			cout << "[";
			for(int j = 0; j < nresources; ++j){
				if(j>0) cout << ",";
				cout << 0;
			}
			cout  << "]";
		}
		else
		for(int k = 0; k < duration[i]; ++k){
			if(k>0) cout << ",";
			cout << "[";
			for(int j = 0; j < nresources; ++j){
				if(j > 0) cout << ",";
				cout << demand[j][i][k];
			}
			cout << "]";
		}
		cout << "]";
	}
	cout << "]" << endl;

	cout << "letting resourceLimits = [";
	for(int i = 0; i < nresources; ++i){
		if(i>0) cout << ",";
		cout << "[";
		for(int j = 0; j < timeHorizon; ++j){
			if(j>0) cout << ",";
			cout << capacity[i][j];
		}
		cout << "]";
	}
	cout << "]" << endl;	

}


ostream &operator << (ostream &output, RCPSPT &instance)
{
	output << "N activities: " << instance.nactivities << endl;
	output << "N resources: " << instance.nresources << endl;
	output << "Time Horizon: " << instance.timeHorizon <<endl;

	for(int i = 0; i < instance.nactivities+2; i++){
		output << "Act: " << i << " Successors:" ;
		for(int j : instance.getSuccessors(i))
			output << " " << j;
		output << endl;
	}

	for(int i = 0; i < instance.nactivities+2; i++){
		output << "Act: " << i << " Duration:" << " " << instance.getDuration(i) << endl;
		output << " Requirements:" << endl;
		for(int r = 0; r < instance.nresources; r++){
			output << "  Resource " << r << ":";
			for(int o = 0; o < instance.getDuration(i); o++){
				output << " " << instance.demand[r][i][o];
			}
			output << endl;
		}
		output << endl;
	}

	for(int k = 0; k < instance.nresources; k++){
		output << "Resource " << k << ":";
		for(int t = 0; t < instance.timeHorizon; t++)
			output << " " << instance.getCapacity(k,t);
		output << endl;
	}

	return output;
}
