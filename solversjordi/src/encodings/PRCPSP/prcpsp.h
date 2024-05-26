#ifndef PRCPSP_H
#define PRCPSP_H

#include <iostream>
#include <string.h>
#include <fstream>
#include <vector>
#include <map>
#include <set>


using namespace std;

class PRCPSP
{

private:

	//INSTANCE DATA
	int nactivities; //Number of non-dummy activites
	int nrenewable; //Number of renewable resources
	int nnonrenewable; //Number of non-renewable resources
	int nresources; //Number of resources (nresouces = nrenewable + nnonrenewable)

	vector<int> * demand; //Demands of each activity
	vector<int> * succs; //List of successors of each activity
	int *duration; // Duration of each activity
	int *capacity; //Capacities of the resources


	//PREPROCESSED DATA
	int ** extPrecs; //Extended time lags
	int ** nSteps; //Minimal number of edges joining two activities
	bool ** resource_incompatibles;
	bool ** tw_incompatibles;
	bool ** resource_disjoints;


	//Statistics
	int ntwincompatibilities;
	int nresincomps;
	int nenergyprecs;
	int ndisjoints;
	int nreducednrdemands;





	int getMostRepDemand(int i, int r) const; //Most repeated demand of activity i over resource r
	int next_activity(vector<vector<bool> > & predecessors,set<int> & tots,set<int> & C,vector<vector<int> > & pik,int t);

public:

	PRCPSP(int nactivities, int nrenewable, int nnonrenewable);
   ~PRCPSP();

	int getNActivities() const;
	int getNResources() const;
	int getNRenewable() const;
	int getNNonRenewable() const;

	void setCapacity(int r, int c);
	int getCapacity(int r) const;

	void setDuration(int i, int p);
	int getDuration(int i) const;

	void setDemand(int i, int r, int d);
	int getDemand(int i, int r) const;

	void addSuccessor(int i, int j);
	const vector<int> & getSuccessors(int i) const;

	void ignoreNR(); //Make this instance have 0 non-renewable resources

	int getExtPrec(int i, int j) const;
	int getNSteps(int i, int j) const;
	int trivialUB() const;
	int trivialLB() const;
	int ES(int i) const;
	int LS(int i, int ub) const;
	int EC(int i) const;
	int LC(int i, int ub) const;
	bool inPath(int i, int j) const;
	bool isPred(int i, int j) const;
	bool isResourceIncompatibles(int i, int j) const;

	//Preprocesses
	void computeExtPrecs();
	void recomputeExtPrecs();
	void computeSteps();
	void computeTWIncompatibilities(int UB);
	void computeResourceDisjoints();
	void computeResourceIncompatibilities();
	void computeEnergyPrecedences();
	void reduceNRDemandMin();
	void reduceNRDemandMostFrequent();
	int computePSS(vector<int> & starts, vector<vector<bool>> & en_execucio);
	void computeMinPathCover(const vector<int> & vasks, vector<set<int> > & groups);
	void getPossibleParents(int i, int ub, vector<int> & parents);

	//Statistics
	int getNTWIncompatibilities() const;
	int getNResourceIncompatibilities() const;
	int getNPrecedenceIncompatibilities() const;
	int getNEnergyPrecedences() const;
	int getNResourceDisjoints() const;
	int getNReducedNRDemands() const;

	void printSolution(ostream & os, const vector<int> & starts, const vector<vector<bool>> & en_execucio) const;
	friend ostream &operator <<(ostream &, PRCPSP &);

	//Feature selection
	int getMaxDuration();
	int getMaxSuccessors();
	float getMinResourceConsumption(int i);
	float getMaxResourceConsumption(int i);

	void printPractica(ostream & output);
	void printRCP(ostream & output);

};

#endif

