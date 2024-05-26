#include "mrcpsp.h"
#include "limits.h"
#include "assert.h"
#include <list>
#include <algorithm>
#include <cstdlib>
#include "util.h"
#include "bipgraph.h"
#include "disjointset.h"
#include <math.h>

using namespace std;

MRCPSP::MRCPSP(int nactivities, int nrenewable, int nnonrenewable){

	//Prepare instance data
	this->nactivities = nactivities;
	this->nrenewable = nrenewable;
	this->nnonrenewable = nnonrenewable;
	this->nresources = nrenewable + nnonrenewable;
	succs = new vector<int> [nactivities+2];
	nmodes = new int[nactivities+2];

	duration = new vector<int> [nactivities+2];
	demand = new vector<int> * [nactivities+2];
	capacity = new int[nresources];

	for(int i = 0; i < nactivities+2; i++)
		demand[i] = new vector<int> [nresources];

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

	//Dummies
	setNModes(0,1);
	setNModes(nactivities+1,1);
}

MRCPSP::~MRCPSP(){

	//Delete instance data
	delete [] nmodes;
	delete [] succs;
	delete [] duration;
	delete [] capacity;

	for(int i = 0; i < nactivities+2; i++)
		delete [] demand[i];
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

int MRCPSP::getNActivities() const{
	return nactivities;
}

int MRCPSP::getNResources() const{
	return nresources;
}

int MRCPSP::getNRenewable() const{
	return nrenewable;
}

int MRCPSP::getNNonRenewable() const{
	return nnonrenewable;
}

void MRCPSP::setDuration(int i, int mode, int p){
	duration[i][mode] = p;
}

void MRCPSP::setCapacity(int r, int c){
	capacity[r]=c;
}

int MRCPSP::getCapacity(int r) const{
	return capacity[r];
}

int MRCPSP::getDuration(int i, int mode) const{
	return duration[i][mode];
}

void MRCPSP::setDemand(int i, int r, int mode, int d){
	demand[i][r][mode] = d;
}

int MRCPSP::getDemand(int i, int r, int mode) const{
	return demand[i][r][mode];
}

void MRCPSP::addSuccessor(int i, int j){
	succs[i].push_back(j);
}

const vector<int> & MRCPSP::getSuccessors(int i) const {
	return succs[i];
}

void MRCPSP::setNModes(int i, int n){
	nmodes[i]=n;
	duration[i]=vector<int>(n,0);
	for(int r = 0; r < nresources; r++)
		demand[i][r]=vector<int>(n,0);
}

void MRCPSP::ignoreNR(){
	this->nnonrenewable = 0;
	this->nresources = this->nrenewable;
}

int MRCPSP::getNModes(int i) const{
	return nmodes[i];
}

int MRCPSP::getExtPrec(int i, int j) const{
	return extPrecs[i][j];
}

int MRCPSP::getNSteps(int i, int j) const{
	return nSteps[i][j];
}

bool MRCPSP::isPred(int i, int j) const{
	return nSteps[i][j] > 0;
}

bool MRCPSP::inPath(int i, int j) const{
	return isPred(i,j) || isPred(j,i);
}

int MRCPSP::trivialUB() const{
	int ub = 0;
	for(int i = 0; i < nactivities+2; i++)
		ub+=getMaxDuration(i);
	return ub;
}

int MRCPSP::trivialLB() const{
	return ES(nactivities+1);
}

int MRCPSP::ES(int i) const{
	return extPrecs[0][i] > 0 ? extPrecs[0][i] : 0;
}

int MRCPSP::LS(int i, int UB) const{
	return extPrecs[i][nactivities+1] > 0 ? UB - extPrecs[i][nactivities+1] : UB;
}

int MRCPSP::EC(int i) const{
	return ES(i) + getMinDuration(i);
}

int MRCPSP::LC(int i, int UB) const{
	return LS(i,UB) + getMinDuration(i);
}

int MRCPSP::getMostRepDemand(int i, int r) const{
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

int MRCPSP::getMinDuration(int i) const{
	int min = INT_MAX;
	for(int d : duration[i])
		if(d < min)
			min = d;
	return min;
}

int MRCPSP::getMaxDuration(int i) const{
	int max = INT_MIN;
	for(int d : duration[i])
		if(d > max)
			max = d;
	return max;
}

int MRCPSP::getMinDemand(int i, int r) const{
	int min = INT_MAX;
	for(int m = 0; m < nmodes[i]; m++)
		if(demand[i][r][m] < min)
				min = demand[i][r][m];
	return min;
}

int MRCPSP::getMaxDemand(int i, int r) const{
	int max = INT_MIN;
	for(int m = 0; m < nmodes[i]; m++)
		if(demand[i][r][m] > max)
				max = demand[i][r][m];
	return max;
}

vector<int> MRCPSP::getModesOrdByDur(int act){
	vector<int> res(nmodes[act]);
	for(int i = 0; i < nmodes[act]; i++){
		int j = i;
		bool finish = false;
		while(j > 0 && !finish){
			if(duration[act][res[j-1]]>duration[act][i]){
				res[j]=res[j-1];
				j--;
			}
			else
				finish=true;
		}
		res[j] = i;
	}
	return res;
}

void MRCPSP::computeExtPrecs(){
	for (int i=0;i<nactivities+2;i++)
		for(int j : succs[i])
			extPrecs[i][j] = getMinDuration(i);

	util::floydWarshall(extPrecs,nactivities+2);
}

void MRCPSP::recomputeExtPrecs(){
	util::floydWarshall(extPrecs,nactivities+2);
}

void MRCPSP::computeSteps(){
	for (int i=0;i<nactivities+2;i++)
		for(int j : succs[i])
			nSteps[i][j] = 1;

	util::floydWarshall(nSteps,nactivities+2);
}


void MRCPSP::computeResourceIncompatibilities(){
	nresincomps = 0;

	vector<vector<int> > minunits;
	minunits.resize(nrenewable);
	for(int r = 0; r < nrenewable; r++){
		minunits[r].resize(nactivities+2);
		for (int i=0;i<nactivities+2;i++) {
			int min = INT_MAX;
			for(int m = 0; m < nmodes[i]; m++){
				int dem = demand[i][r][m];
				if(dem < min)
					min = dem;
			}
			minunits[r][i]=min;
		}
	}

	for (int k=0;k<nrenewable;k++) {
		for (int i=0;i<nactivities+1;i++) {
			for (int j=i+1;j<nactivities+2;j++) {
				if (minunits[k][i]+minunits[k][j]>capacity[k]){
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
void MRCPSP::computeTWIncompatibilities(int UB){
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

void MRCPSP::computeResourceDisjoints(){
	ndisjoints = 0;
	vector<uint32_t> resource_masks(nactivities+2,0);
	for(int i = 0; i < nactivities+2; i++)
		for(int r = 0; r < nrenewable; r++)
			for(int m = 0; m < nmodes[i]; m++)
				if(demand[i][r][m]>0)
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


void MRCPSP::computeEnergyPrecedences(){
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
						for(int r = 0; r < nrenewable;r++){
							int min = INT_MAX;
							for(int m = 0; m <nmodes[k]; m++){
								int dem = demand[k][r][m]*duration[k][m];
								if(dem < min)
									min = dem;
							}
							dems[r]+=min;
						}
					}
				}
				double max = 0;
				for(int r = 0; r < nrenewable;r++)
					if(dems[r]/((double)capacity[r]) > max)
						max = dems[r]/((double)capacity[r]);
				int rl = (int) ceil(max) + getMinDuration(i);
				if(rl>extPrecs[i][j]){
					nenergyprecs++;
					extPrecs[i][j]=rl;
				}
			}
		}
	}

	delete [] visited;
}

void MRCPSP::reduceNRDemandMin(){
	for(int r = nrenewable; r < nresources; r++){
		for(int i = 0; i < nactivities+2; i++){
			int mindem = getMinDemand(i,r);
			capacity[r]-=mindem;
			for(int m = 0; m < nmodes[i]; m++){
				demand[i][r][m]-=mindem;
				if(demand[i][r][m]==0)
					nreducednrdemands++;
			}
		}
	}
}

void MRCPSP::reduceNRDemandMostFrequent(){
	for(int r = nrenewable; r < nresources; r++){
		for(int i = 0; i < nactivities+2; i++){
			int mostrep = getMostRepDemand(i,r);
			capacity[r]-=mostrep;
			for(int m = 0; m < nmodes[i]; m++){
				demand[i][r][m]-=mostrep;
				if(demand[i][r][m]==0)
					nreducednrdemands++;
			}
		}
	}
}


int MRCPSP::getNTWIncompatibilities() const{
	return ntwincompatibilities;
}

int MRCPSP::getNResourceIncompatibilities() const{
	return nresincomps;
}

int MRCPSP::getNPrecedenceIncompatibilities() const{
	int npreds = 0;
	for(int i = 0; i < nactivities+2; i++)
		for(int j = 0; j < nactivities+2; j++)
			if(isPred(i,j))
				npreds++;
	return npreds;
}

int MRCPSP::getNEnergyPrecedences() const{
	return nenergyprecs;
}

int MRCPSP::getNResourceDisjoints() const{
	return ndisjoints;
}

int MRCPSP::getNReducedNRDemands() const{
	return nreducednrdemands;
}



int MRCPSP::next_activity(vector<vector<bool> > & predecessors,set<int> & tots,set<int> & C,vector<vector<int> > & pik,int t, const vector<int> & smodes) {
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
			while (i<t+duration[*it][smodes[*it]] && correcte){
				int k=0;
				while (k<nrenewable && correcte) {
					if (pik.size()>i-t)
						if (pik[i-t][k]<demand[*it][k][smodes[*it]])
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
int MRCPSP::computePSS(vector<int> & starts, const vector<int> & smodes){


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
				if(starts[ii]+duration[ii][smodes[ii]]<t)
					t=starts[ii]+duration[ii][smodes[ii]];


				//Modificat
			set<int> aesborrar;
			for (int ii : A){
				if (starts[ii]+duration[ii][smodes[ii]]==t){
					C.insert(ii);
					aesborrar.insert(ii);
				}
			}

			for(int jj: aesborrar)
				A.erase(jj);

			for (int i=antt;i<t;i++)
				pik.erase(pik.begin()); //borrar primer element de pik
			d=next_activity(predecessors,tots,C,pik,t,smodes); //calcular nou d
		}
		while (d!=-1) {
			int j=d;
			starts[j]=t;
			A.insert(j);
			for (int i=t;i<t+duration[j][smodes[j]];i++) {
				if (pik.size()<=i-t) {
					vector<int> auxr;
					auxr.resize(nrenewable);
					for (int k=0;k<nrenewable;k++)
						auxr[k]=capacity[k];
					pik.push_back(auxr);
				}
				for (int k=0;k<nrenewable;k++)
				 pik[i-t][k]=pik[i-t][k]-demand[j][k][smodes[j]];
			}
			d=next_activity(predecessors,tots,C,pik,t,smodes); //calcular nou d
		}
	}

	return t;
}

void MRCPSP::computeMinPathCover(const vector<int> & vtasks, vector<set<int> > & groups){

  vector<pair<int,int> > matching;
  BipGraph bg(vtasks.size()+1,vtasks.size()+1);

  matching.clear();

  for(int i = 0; i < vtasks.size(); i++){
    for(int j = 0; j < vtasks.size(); j++){
      if(i!=j && getMinDuration(vtasks[i]) > 0){
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

void MRCPSP::getPossibleParents(int i, int ub, vector<int> & parents){
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

void MRCPSP::printPRCPSPSolution(ostream & os, const vector<int> & starts, const vector<int> & modes) const{
	
	vector<int> s;
	vector<vector<bool>> x;
	int c = 0;

	if (starts.size() == 0)
		return;

	for(int i=0; i<nactivities+2; i++) {
		s.push_back(starts[c]);
		x.push_back(vector<bool>(starts[starts.size()-1]+1,false));

		int d = this->getDuration(i,0);

		if(d==0) c++;

		for(int t=0; t<d; t++) {
			/* std::cout << i << ": " << starts[c] << std::endl; */
			x[i][starts[c]] = true;
			c++;
		}
	}
	
	int makespan = s[s.size()-1];

	for(int i = 0; i < s.size(); i++)
		os << "S_" << i << ":" << s[i] << "; ";
	os << std::endl;

	/* for (int i = 0; i < x.size(); ++i) {
		os << i << ": ";
		for (int j = 0; j < x[i].size(); ++j)
			os << (s[i]>j ? 0 : 1);

		os << "\n";
	}
	os << "\n"; */

	for (int i = 0; i < x.size(); ++i) {
		os << i << ": ";
		for (int j = 0; j < makespan; ++j)
			os << x[i][j];

		os << "\n";
	}

	/* for(int i = 0; i < modes.size(); i++)
		os << "M_" << i << ":" << modes[i]+1 << "; ";
	os << std::endl; */
}

void MRCPSP::printSolution(ostream & os, const vector<int> & starts, const vector<int> & modes) const{

	for(int i = 0; i < starts.size(); i++)
		os << "S_" << i << ":" << starts[i] << "; ";
	os << std::endl;

	for(int i = 0; i < modes.size(); i++)
		os << "M_" << i << ":" << modes[i]+1 << "; ";
	os << std::endl;
}

ostream &operator << (ostream &output, MRCPSP &m)
{
	output << "N activities: " << m.nactivities << endl;
	output << "N renewable: " << m.nrenewable << endl;
	output << "N non-renewable: " << m.nnonrenewable <<endl;

	for(int i = 0; i < m.nactivities+2; i++){
		output << "Act: " << i << " N modes: " << m.getNModes(i) << " Successors:" ;
		for(int j : m.getSuccessors(i))
			output << " " << j;
		output << endl;
	}

	for(int i = 0; i < m.nactivities+2; i++){
		output << "Act: " << i << " Durations:";
		for(int o = 0; o < m.getNModes(i); o++)
			output << " " << m.getDuration(i,o);
		output << " Requirements:";
		for(int r = 0; r < m.nrenewable+m.nnonrenewable; r++){
			for(int o = 0; o < m.getNModes(i); o++){
				output << " " << m.demand[i][r][o];
			}
		}
		output << endl;
	}

	output << "Capacities: ";
	for(int k = 0; k < m.nresources; k++)
		output << " " << m.getCapacity(k);
	output << endl;

	return output;
}


void MRCPSP::printRCP(ostream & output){

	output << (nactivities+2) << "\t" << nrenewable << endl;

	for(int i = 0; i < nrenewable; i++)
		output << capacity[i] << "\t";
	output << endl;

	for(int i = 0; i < nactivities+2; i++){
		output << getDuration(i,0) << "\t";

		for(int r = 0; r < nrenewable; r++)
			output << getDemand(i,r,0) << "\t";

		output << getSuccessors(i).size() << "\t";

		for(int j : getSuccessors(i))
			output << (j+1) << "\t";

		output << endl;
	}
}


void MRCPSP::printPractica(ostream & output){
	output << "Nproves = " << nactivities << ";" << endl;
	output << "Narbitres = " << capacity[2] << ";" << endl;
	output << "Nseguretat = " << (capacity[3]*2) << ";" << endl;
	output << endl;

	output << "durada = [";
	for(int i = 1; i <= nactivities; i++){
		if(i>1)
			output << ", ";
		output << getDuration(i,0)*10;
	}
	output << "];" << endl;

	output << "estrella = [";
	for(int i = 1; i <= nactivities; i++){
		if(i>1)
			output << ", ";
		output << (getDemand(i,0,0) > 7 ? "true" : "false");
	}
	output << "];" << endl;

	output << "televisio = [";
	for(int i = 1; i <= nactivities; i++){
		if(i>1)
			output << ", ";
		output << (getDemand(i,1,0) > 4 ? "true" : "false");
	}
	output << "];" << endl;

	output << "arbitres = [";
	for(int i = 1; i <= nactivities; i++){
		if(i>1)
			output << ", ";
		output << (getDemand(i,2,0)==0 ? 1 : getDemand(i,2,0));
	}
	output << "];" << endl;

	output << "seguretat = [";
	for(int i = 1; i <= nactivities; i++){
		if(i>1)
			output << ", ";
		output << (getDemand(i,3,0)==0 ? 2 : getDemand(i,3,0)*2);
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
