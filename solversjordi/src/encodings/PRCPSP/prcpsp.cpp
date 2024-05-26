#include "prcpsp.h"
#include "assert.h"
#include "limits.h"
#include <list>
#include <algorithm>
#include <cstdlib>
#include "util.h"
#include "bipgraph.h"
#include "disjointset.h"
#include <math.h>

using namespace std;

PRCPSP::PRCPSP(int nactivities, int nrenewable, int nnonrenewable){

	//Prepare instance data
	this->nactivities = nactivities;
	this->nrenewable = nrenewable;
	this->nnonrenewable = nnonrenewable;
	this->nresources = nrenewable + nnonrenewable;

	succs = new vector<int> [nactivities+2];
	demand = new vector<int> [nactivities+2];

	duration = new int[nactivities+2];
	capacity = new int[nresources];

	for(int i = 0; i < nactivities+2; i++)
		demand[i] = vector<int>(nresources);

	//Prepare preprocessed data
	extPrecs = new int * [nactivities+2];
	nSteps = new int * [nactivities+2];
	resource_incompatibles = new bool * [nactivities+2];
	tw_incompatibles = new bool * [nactivities+2];
	resource_disjoints = new bool * [nactivities+2];

	for(int i = 0; i < nactivities+2; i++){
		extPrecs[i] = new int[nactivities+2];
		nSteps[i] = new int[nactivities+2];
		resource_incompatibles[i] = new bool[nactivities+2];
		tw_incompatibles[i] = new bool[nactivities+2];
		resource_disjoints[i] = new bool[nactivities+2];
	}

	for(int i = 0; i < nactivities+2; i++){
		for(int j = 0; j < nactivities+2; j++){
			extPrecs[i][j] = INT_MIN;
			nSteps[i][j] = INT_MIN;
			resource_incompatibles[i][j] = false;
			tw_incompatibles[i][j] = false;
			resource_disjoints[i][j] = false;
		}
	}

	ntwincompatibilities = 0;
	nresincomps = 0;
	nenergyprecs = 0;
	ndisjoints = 0;
	nreducednrdemands = 0;
}

PRCPSP::~PRCPSP(){

	//Delete instance data
	delete [] succs;
	delete [] duration;
	delete [] capacity;
	delete [] demand;

	//Delete preprocessed Data
	for(int i = 0; i < nactivities+2; i++){
		delete [] extPrecs[i];
		delete [] nSteps[i];
		delete [] resource_incompatibles[i];
		delete [] tw_incompatibles[i];
		delete [] resource_disjoints[i];
	}

	delete [] extPrecs;
	delete [] nSteps;
	delete [] resource_incompatibles;
	delete [] tw_incompatibles;
	delete [] resource_disjoints;
}

int PRCPSP::getNActivities() const{
	return nactivities;
}

int PRCPSP::getNResources() const{
	return nresources;
}

int PRCPSP::getNRenewable() const{
	return nrenewable;
}

int PRCPSP::getNNonRenewable() const{
	return nnonrenewable;
}

void PRCPSP::setDuration(int i, int p){
	duration[i] = p;
}

void PRCPSP::setCapacity(int r, int c){
	capacity[r]=c;
}

int PRCPSP::getCapacity(int r) const{
	return capacity[r];
}

int PRCPSP::getDuration(int i) const{
	return duration[i];
}

void PRCPSP::setDemand(int i, int r, int d){
	demand[i][r] = d;
}

int PRCPSP::getDemand(int i, int r) const{
	return demand[i][r];
}

void PRCPSP::addSuccessor(int i, int j){
	succs[i].push_back(j);
}

const vector<int> & PRCPSP::getSuccessors(int i) const {
	return succs[i];
}

void PRCPSP::ignoreNR(){
	this->nnonrenewable = 0;
	this->nresources = this->nrenewable;
}

int PRCPSP::getExtPrec(int i, int j) const{
	return extPrecs[i][j];
}

int PRCPSP::getNSteps(int i, int j) const{
	return nSteps[i][j];
}

bool PRCPSP::isPred(int i, int j) const{
	return nSteps[i][j] > 0;
}

bool PRCPSP::inPath(int i, int j) const{
	return isPred(i,j) || isPred(j,i);
}

bool PRCPSP::isResourceIncompatibles(int i, int j) const {
	return resource_incompatibles[i][j];
}

int PRCPSP::trivialUB() const{
	int ub = 0;
	for(int i = 0; i < nactivities+2; i++)
		ub+=getDuration(i);
	return ub;
}

int PRCPSP::trivialLB() const{
	return ES(nactivities+1);
}

int PRCPSP::ES(int i) const{
	return extPrecs[0][i] > 0 ? extPrecs[0][i] : 0;
}

int PRCPSP::LS(int i, int UB) const{
	return extPrecs[i][nactivities+1] > 0 ? UB - extPrecs[i][nactivities+1] : UB;
}

int PRCPSP::EC(int i) const{
	return ES(i) + getDuration(i);
}

int PRCPSP::LC(int i, int UB) const{
	return LS(i,UB) + getDuration(i);
}

int PRCPSP::getMostRepDemand(int i, int r) const{
	int max = 0;
	int most_common = INT_MIN;
	map<int,int> m;
	m[demand[i][r]]++;
	if (m[demand[i][r]] > max) {
		max = m[demand[i][r]];
		most_common = demand[i][r];
	}
	return most_common;
}

void PRCPSP::computeExtPrecs(){
	for (int i=0;i<nactivities+2;i++)
		for(int j : succs[i])
			extPrecs[i][j] = getDuration(i);

	util::floydWarshall(extPrecs,nactivities+2);
}

void PRCPSP::recomputeExtPrecs(){
	util::floydWarshall(extPrecs,nactivities+2);
}

void PRCPSP::computeSteps(){
	for (int i=0;i<nactivities+2;i++)
		for(int j : succs[i])
			nSteps[i][j] = 1;

	util::floydWarshall(nSteps,nactivities+2);
}

void PRCPSP::computeResourceIncompatibilities(){
	nresincomps = 0;

	for (int k=0;k<nrenewable;k++) {
		for (int i=0;i<nactivities+1;i++) {
			for (int j=i+1;j<nactivities+2;j++) {
				if (demand[i][k]+demand[j][k]>capacity[k]){
					resource_incompatibles[i][j]=true;
					resource_incompatibles[j][i]=true;
					nresincomps++;
				}
			}
		}
	}
}

//A pair of activities i,j are said to be TWincom
//if i cannot be running during the start time of j
//due to time windows. TWincomp(i,j) is not the same as TWincomp(j,i)
void PRCPSP::computeTWIncompatibilities(int UB){
	ntwincompatibilities = 0;
	for(int i = 0; i<nactivities+2; i++){
		int ESi = ES(i);
		int LCi = LC(i,UB);
		for(int j=0; j<nactivities+2; j++){
			int LSj = LS(j,UB);
			int ESj = ES(j);
			if(i==j)
				tw_incompatibles[i][j] = false;
			else{
				if(LCi <= ESj || LSj < ESi){
					tw_incompatibles[i][j]=true;
					ntwincompatibilities++;
				}
				else
					tw_incompatibles[i][j]=false;
			}
		}
	}
}

void PRCPSP::computeResourceDisjoints(){
	ndisjoints = 0;
	vector<uint32_t> resource_masks(nactivities+2,0);
	for(int i = 0; i < nactivities+2; i++)
		for(int r = 0; r < nrenewable; r++)
			if(demand[i][r]>0)
				resource_masks[i]|=1<<r;

	for(int i = 0; i < nactivities+1; i++){
		for(int j = i+1; j <= nactivities+2; j++){
			if((resource_masks[i] & resource_masks[j]) == 0){
				resource_disjoints[i][j] = resource_disjoints[j][i] = true;
				ndisjoints++;
			}
		}
	}
}


void PRCPSP::computeEnergyPrecedences(){
	nenergyprecs = 0;
	list<int> q;
	bool * visited = new bool[nactivities+2];
	for(int i = 0; i < nactivities+1; i++){
		for(int j = 0; j < nactivities+2; j++){
			if(i!=j && extPrecs[i][j]>INT_MIN){

				vector<int> dems(nrenewable,0);
				for(int k = 0; k < nactivities+2; k++)
					visited[k]=false;

				q.insert(q.end(),succs[i].begin(),succs[i].end());
				while(!q.empty()){
					int k = q.front();
					q.pop_front();
					if(!visited[k] && extPrecs[k][j] > INT_MIN){
						visited[k] = true;
						q.insert(q.end(),succs[k].begin(),succs[k].end());
						for(int r = 0; r < nrenewable;r++)
							dems[r]+=demand[k][r]*duration[k];
					}
				}
				double max = 0;
				for(int r = 0; r < nrenewable;r++)
					if(dems[r]/((double)capacity[r]) > max)
						max = dems[r]/((double)capacity[r]);
				int rl = (int) ceil(max) + getDuration(i);
				if(rl>extPrecs[i][j]){
					nenergyprecs++;
					extPrecs[i][j]=rl;
				}
			}
		}
	}

	delete [] visited;
}

void PRCPSP::reduceNRDemandMin(){
	for(int r = nrenewable; r < nresources; r++){
		for(int i = 0; i < nactivities+2; i++){
			int mindem = getDemand(i,r);
			capacity[r]-=mindem;
			demand[i][r]-=mindem;
			if(demand[i][r]==0)
				nreducednrdemands++;
		}
	}
}

void PRCPSP::reduceNRDemandMostFrequent(){
	for(int r = nrenewable; r < nresources; r++){
		for(int i = 0; i < nactivities+2; i++){
			int mostrep = getMostRepDemand(i,r);
			capacity[r]-=mostrep;
			demand[i][r]-=mostrep;
			if(demand[i][r]==0)
				nreducednrdemands++;
		}
	}
}


int PRCPSP::getNTWIncompatibilities() const{
	return ntwincompatibilities;
}

int PRCPSP::getNResourceIncompatibilities() const{
	return nresincomps;
}

int PRCPSP::getNPrecedenceIncompatibilities() const{
	int npreds = 0;
	for(int i = 0; i < nactivities+2; i++)
		for(int j = 0; j < nactivities+2; j++)
			if(isPred(i,j))
				npreds++;
	return npreds;
}

int PRCPSP::getNEnergyPrecedences() const{
	return nenergyprecs;
}

int PRCPSP::getNResourceDisjoints() const{
	return ndisjoints;
}

int PRCPSP::getNReducedNRDemands() const{
	return nreducednrdemands;
}



int PRCPSP::next_activity(vector<vector<bool> > & predecessors,set<int> & tots,set<int> & C,vector<vector<int> > & pik,int t) {
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
				int k=0;
				while (k<nrenewable && correcte) {
					if (pik.size()>i-t)
						if (pik[i-t][k]<demand[*it][k])
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
int PRCPSP::computePSS(vector<int> & starts, vector<vector<bool>> & en_execucio){

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
	vector<vector<int> > pik;
	int t=0;
	int d=0;
	for (int i=1; i<nactivities+2; i++) tots.insert(i);
	starts.resize(nactivities+2);
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

			for (int i=antt;i<t;i++)
				pik.erase(pik.begin()); //esborrar primer element de pik
			d=next_activity(predecessors,tots,C,pik,t); //calcular nou d
		}
		while (d!=-1) {
			int j=d;
			starts[j]=t;
			A.insert(j);
			for (int i=t;i<t+duration[j];i++) {
				if (pik.size()<=i-t) {
					vector<int> auxr;
					auxr.resize(nrenewable);
					for (int k=0;k<nrenewable;k++)
						auxr[k]=capacity[k];
					pik.push_back(auxr);
				}
				for (int k=0;k<nrenewable;k++)
				 pik[i-t][k]=pik[i-t][k]-demand[j][k];
			}
			d=next_activity(predecessors,tots,C,pik,t); //calcular nou d
		}
	}

	en_execucio.resize(nactivities+2);
	for(int i=0; i<nactivities+2; i++){
		en_execucio[i].resize(starts[starts.size()-1],false);
		for(int j=starts[i]; j<(starts[i]+this->getDuration(i)); j++) {
			en_execucio[i][j]=true;
		}
	}

	return t;
}

int PRCPSP::getMaxDuration() {
	int max = -1;
	for(int i = 0; i < nactivities; i++)
		if(duration[i]>max)
			max = duration[i];
	
	return max;
}

int PRCPSP::getMaxSuccessors() {
	int max = -1;
	for(int i = 0; i < nactivities; i++) {
		int s = int(succs[i].size());
		if(s>max)
			max = s;
	}
	
	return max;
}

float PRCPSP::getMinResourceConsumption(int i) {
	float min = demand[i][0];
	for(int r = 1; r < nresources; r++) {
		float capacity = getCapacity(r);
		float f = demand[i][r]/capacity;
		if(f<min)
			min = f;
	}
	
	return min;
}

float PRCPSP::getMaxResourceConsumption(int i) {
	float max = -1;
	for(int r = 0; r < nresources; r++) {
		float capacity = getCapacity(r);
		float f = demand[i][r]/capacity;
		if(f>max)
			max = f;
	}
	
	return max;
}

void PRCPSP::computeMinPathCover(const vector<int> & vtasks, vector<set<int> > & groups){

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

void PRCPSP::getPossibleParents(int i, int ub, vector<int> & parents){
	if(i==0)
		return;

	for(int j = 0; j <= nactivities; j++){
		if(i!=j
			&& !isPred(i,j)
			&& (!isPred(j,i) || find(succs[j].begin(),succs[j].end(),i) != succs[j].end())
			&& !(ES(i) > LC(j,ub) || LS(i,ub) < EC(j))){

				parents.push_back(j);
		}
	}
}

void PRCPSP::printSolution(ostream & os, const vector<int> & starts, const vector<vector<bool>> & en_execucio) const{

	os << std::endl;
	for(int i = 0; i < starts.size(); i++)
		os << "S_" << i << ":" << starts[i] << "; ";
	os << std::endl;
	
	/* for (int i = 0; i < en_execucio.size(); ++i) {
		os << i << ": ";
		for (int j = 0; j < en_execucio[i].size(); ++j)
			os << (starts[i]>j ? 0 : 1);

		os << "\n";
	}
	os << "\n"; */

	for (int i = 0; i < en_execucio.size(); ++i) {
		os << i << ": ";
		for (int j = 0; j < en_execucio[i].size(); ++j)
			os << en_execucio[i][j];

		os << "\n";
	}
	/*for(int i = 0; i < en_execucio.size()-1; i++){
		int last = 1;	
		for(int j = 0; j < en_execucio[i].size(); j++)
			last = en_execucio[i][j]==1 ? j : last;
		os << "F_" << i << ":" << last-1 << "; ";
	}
	os << "F_" << en_execucio.size()-1 << ":" << starts[starts.size()-1] << "; ";
	os << std::endl;
	for (int i = 0; i < en_execucio.size(); ++i) {
		os << i << ": ";
		for (int j = 0; j < en_execucio[i].size(); ++j)
			os << en_execucio[i][j];

		os << "\n";
	}*/
}

ostream &operator << (ostream &output, PRCPSP &m)
{
	output << "N activities: " << m.nactivities << endl;
	output << "N renewable: " << m.nrenewable << endl;
	output << "N non-renewable: " << m.nnonrenewable <<endl;

	for(int i = 0; i < m.nactivities+2; i++){
		output << "Act: " << i << " Successors:" ;
		for(int j : m.getSuccessors(i))
			output << " " << j;
		output << endl;
	}

	for(int i = 0; i < m.nactivities+2; i++){
		output << "Act: " << i << " Durations:";
		output << " " << m.getDuration(i);
		output << " Requirements:";
		for(int r = 0; r < m.nrenewable+m.nnonrenewable; r++){
			output << " " << m.demand[i][r];
		}
		output << endl;
	}

	output << "Capacities: ";
	for(int k = 0; k < m.nresources; k++)
		output << " " << m.getCapacity(k);
	output << endl;

	return output;
}


void PRCPSP::printRCP(ostream & output){

	output << (nactivities+2) << "\t" << nrenewable << endl;

	for(int i = 0; i < nrenewable; i++)
		output << capacity[i] << "\t";
	output << endl;

	for(int i = 0; i < nactivities+2; i++){
		output << getDuration(i) << "\t";

		for(int r = 0; r < nrenewable; r++)
			output << getDemand(i,r) << "\t";

		output << getSuccessors(i).size() << "\t";

		for(int j : getSuccessors(i))
			output << (j+1) << "\t";

		output << endl;
	}
}


void PRCPSP::printPractica(ostream & output){
	output << "Nproves = " << nactivities << ";" << endl;
	output << "Narbitres = " << capacity[2] << ";" << endl;
	output << "Nseguretat = " << (capacity[3]*2) << ";" << endl;
	output << endl;

	output << "durada = [";
	for(int i = 1; i <= nactivities; i++){
		if(i>1)
			output << ", ";
		output << getDuration(i)*10;
	}
	output << "];" << endl;

	output << "estrella = [";
	for(int i = 1; i <= nactivities; i++){
		if(i>1)
			output << ", ";
		output << (getDemand(i,0) > 7 ? "true" : "false");
	}
	output << "];" << endl;

	output << "televisio = [";
	for(int i = 1; i <= nactivities; i++){
		if(i>1)
			output << ", ";
		output << (getDemand(i,1) > 4 ? "true" : "false");
	}
	output << "];" << endl;

	output << "arbitres = [";
	for(int i = 1; i <= nactivities; i++){
		if(i>1)
			output << ", ";
		output << (getDemand(i,2)==0 ? 1 : getDemand(i,2));
	}
	output << "];" << endl;

	output << "seguretat = [";
	for(int i = 1; i <= nactivities; i++){
		if(i>1)
			output << ", ";
		output << (getDemand(i,3)==0 ? 2 : getDemand(i,3)*2);
	}
	output << "];" << endl;

	output << endl;

	output << "abans = [";
	for(int i = 1; i <= nactivities; i++){
		bool comma = false;
		output << "{";
		for(int j : succs[i]) if(j!=nactivities+1){
			if(comma)
				output << ",";
			else
				comma=true;
			output << j;
		}
		output << "}";
		if(i < nactivities)
			output << "," << endl;
		else
			output << "];" << endl;
	}

	output << endl;

	output << "maxdif = [";
	for(int i = 1; i <= nactivities; i++){
		bool comma = true;
		output << "|";
		for(int j = 1; j <= nactivities; j++){
			if(extPrecs[i][j] > 0 && rand()%30==0)
				output << extPrecs[i][j]*10*5;
			else 
				output << 0;

			if(!(i==nactivities && j==nactivities))
				output << ",";
			else
				output << "|];";
		}
		output << endl;
	}
}
